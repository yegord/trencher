/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include "SCSemantics.h"

#include <cassert>
#include <sstream>

#include <boost/optional.hpp>

#include "ProgramPrinting.h"
#include "Transition.h"

namespace trench {

std::ostream &operator<<(std::ostream &out, const SCState &state) {
	for (const auto &threadAndState : state.controlStates()) {
		out << "cs(" << threadAndState.first->name() << ")=" << threadAndState.second->name() << "\\n";
	}
	for (const auto &addrAndValue : state.memoryValuation()) {
		if (addrAndValue.second != 0) {
			out << "mem";
			if (addrAndValue.first.first != 0) {
				out << addrAndValue.first.first;
			}
			out << "[" << addrAndValue.first.second << "]=" << addrAndValue.second << "\\n";
		}
	}
	for (const auto &regAndValue : state.registerValuation()) {
		if (regAndValue.second != 0) {
			out << "reg[" << regAndValue.first.first->name() << "," << regAndValue.first.second->name() << "]=" << regAndValue.second << "\\n";
		}
	}
	return out;
}

SCSemantics::State SCSemantics::initialState() const {
	SCState result;

	for (auto thread : program_.threads()) {
		result.setControlState(thread, thread->initialState());
	}

	return result;
};

std::string SCSemantics::getName(const State &state) const {
	std::stringstream out;
	out << state;
	return out.str();
}

namespace {

Domain evaluate(const SCState &state, const Thread *thread, const Expression &expression) {
	switch (expression.kind()) {
		case Expression::CONSTANT:
			return expression.as<Constant>()->value();
		case Expression::REGISTER:
			return state.getRegisterValue(thread, expression.as<Register>());
		case Expression::UNARY: {
			auto unary = expression.as<UnaryOperator>();
			auto operandValue = evaluate(state, thread, *unary->operand());
			switch (unary->kind()) {
				case UnaryOperator::NOT:
					return !operandValue;
			}
			assert(!"NEVER REACHED");
		}
		case Expression::BINARY: {
			auto binary = expression.as<BinaryOperator>();
			auto leftValue = evaluate(state, thread, *binary->left());
			auto rightValue = evaluate(state, thread, *binary->right());
			switch (binary->kind()) {
				case BinaryOperator::EQ:
					return leftValue == rightValue;
				case BinaryOperator::NEQ:
					return leftValue != rightValue;
				case BinaryOperator::LT:
					return leftValue < rightValue;
				case BinaryOperator::LEQ:
					return leftValue <= rightValue;
				case BinaryOperator::GT:
					return leftValue > rightValue;
				case BinaryOperator::GEQ:
					return leftValue >= rightValue;
				case BinaryOperator::AND:
					return leftValue && rightValue;
				case BinaryOperator::OR:
					return leftValue || rightValue;
				case BinaryOperator::ADD:
					return leftValue + rightValue;
				case BinaryOperator::SUB:
					return leftValue - rightValue;
				case BinaryOperator::MUL:
					return leftValue * rightValue;
				case BinaryOperator::BIN_AND:
					return leftValue & rightValue;
			}
			assert(!"NEVER REACHED");
		}
		case Expression::CAN_ACCESS_MEMORY: {
			return state.memoryLockOwner() == NULL || state.memoryLockOwner() == thread;
		}
	}
	assert(!"NEVER REACHED");
}

boost::optional<SCState> execute(const SCState &state, const Thread *thread, const Instruction &instruction) {
	switch (instruction.mnemonic()) {
		case Instruction::READ: {
			auto read = instruction.as<Read>();
			auto result = state;
			result.setRegisterValue(
				thread,
				read->reg().get(),
				state.getMemoryValue(
					read->space(),
					evaluate(state, thread, *read->address())
				));
			result.setFavourite(NULL);
			return result;
		}
		case Instruction::WRITE: {
			auto write = instruction.as<Write>();
			auto result = state;
			result.setMemoryValue(
				write->space(),
				evaluate(state, thread, *write->address()),
				evaluate(state, thread, *write->value())
			);
			result.setFavourite(NULL);
			return result;
		}
		case Instruction::MFENCE: /* FALLTHROUGH */
		case Instruction::NOOP: {
			/* No-op under SC. */
			auto result = state;
			result.setFavourite(thread);
			return result;
		}
		case Instruction::LOCAL: {
			auto local = instruction.as<Local>();
			auto result = state;
			result.setRegisterValue(
				thread,
				local->reg().get(),
				evaluate(state, thread, *local->value())
			);
			result.setFavourite(thread);
			return result;
		}
		case Instruction::CONDITION: {
			auto condition = instruction.as<Condition>();
			if (!evaluate(state, thread, *condition->expression())) {
				return boost::none;
			}
			auto result = state;
			result.setFavourite(thread);
			return result;
		}
		case Instruction::ATOMIC: {
			auto atomic = instruction.as<Atomic>();
			auto result = state;
			auto newFavourite = thread;
			for (const auto &instr : atomic->instructions()) {
				if (auto destination = execute(result, thread, *instr)) {
					result = *destination;
					if (result.favourite() == NULL) {
						newFavourite = NULL;
					}
				} else {
					return boost::none;
				}
			}
			result.setFavourite(newFavourite);
			return result;
		}
		case Instruction::LOCK: {
			if (state.memoryLockOwner() == NULL) {
				auto result = state;
				result.setMemoryLockOwner(thread);
				return result;
			} else {
				return boost::none;
			}
		}
		case Instruction::UNLOCK: {
			if (state.memoryLockOwner() == thread) {
				auto result = state;
				result.setMemoryLockOwner(NULL);
				return state;
			} else {
				return boost::none;
			}
		}
	}
	assert(!"NEVER REACHED");
}

} // anonymous namespace

std::vector<SCSemantics::Transition> SCSemantics::getTransitionsFrom(const State &state) const {
	std::vector<Transition> result;

	for (const auto &threadAndState : state.controlStates()) {
		auto thread = threadAndState.first;
		if ((state.memoryLockOwner() == NULL || state.memoryLockOwner() == thread) &&
		    (state.favourite() == NULL || state.favourite() == thread)) {
			auto controlState = threadAndState.second;
			for (auto transition : controlState->out()) {
				if (auto destination = execute(state, thread, *transition->instruction())) {
					destination->setControlState(thread, transition->to());

					const auto &live = liveness_.getLiveRegisters(transition->to());
					destination->registerValuation().filterOut(
						[&](const std::pair<const Thread *, const Register *> &threadAndRegister){
							return threadAndRegister.first == thread &&
							       std::find(live.begin(), live.end(), threadAndRegister.second) == live.end();
						}
					);

					result.push_back(SCTransition(state, std::move(*destination), transition->instruction().get()));
				}
			}
		}
	}

	return result;
}

SCSemantics::Label SCSemantics::getLabel(const Transition &transition) const {
	std::stringstream out;
	printInstruction(*transition.instruction(), out);
	return out.str();
}

} // namespace trench
