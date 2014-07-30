#include "SCSemantics.h"

#include <cassert>

#include <boost/optional.hpp>

#include "Transition.h"

namespace trench {

inline void SCState::setControlState(const Thread *thread, const State *state) {
	hash_ ^= reinterpret_cast<uintptr_t>(state);

	for (auto &threadAndState : controlStates_) {
		if (threadAndState.first == thread) {
			hash_ ^= reinterpret_cast<uintptr_t>(threadAndState.second);
			threadAndState.second = state;
			return;
		}
	}

	controlStates_.push_back(std::make_pair(thread, state));
}

inline void SCState::setMemoryValue(Space space, Domain address, Domain value) {
	auto &v = memoryValuation_[std::make_pair(space, address)];
	hash_ ^= v ^ value;
	if (value) {
		v = value;
	} else {
		memoryValuation_.erase(std::make_pair(space, address));
	}
}

inline Domain SCState::getMemoryValue(Space space, Domain address) const {
	auto i = memoryValuation_.find(std::make_pair(space, address));
	if (i != memoryValuation_.end()) {
		return i->second;
	} else {
		return Domain();
	}
}

inline void SCState::setRegisterValue(const Thread *thread, const Register *reg, Domain value) {
	auto &v = registerValuation_[std::make_pair(thread, reg)];
	hash_ ^= v ^ value;
	if (value) {
		v = value;
	} else {
		registerValuation_.erase(std::make_pair(thread, reg));
	}
}

inline Domain SCState::getRegisterValue(const Thread *thread, const Register *reg) const {
	auto i = registerValuation_.find(std::make_pair(thread, reg));
	if (i != registerValuation_.end()) {
		return i->second;
	} else {
		return Domain();
	}
}

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
			out << "reg[" << regAndValue.first.first->name() << "," << regAndValue.first.second->name() << "]=" << regAndValue.second;
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
			auto rightValue = evaluate(state, thread, *binary->left());
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
			return state.memoryLock() == NULL || state.memoryLock() == thread;
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
			return result;
		}
		case Instruction::MFENCE: /* FALLTHROUGH */
		case Instruction::NOOP: {
			/* No-op under SC. */
			auto result = state;
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
			return result;
		}
		case Instruction::CONDITION: {
			auto condition = instruction.as<Condition>();
			if (!evaluate(state, thread, *condition->expression())) {
				return boost::none;
			}
			auto result = state;
			return result;
		}
		case Instruction::ATOMIC: {
			auto atomic = instruction.as<Atomic>();
			auto result = state;
			for (const auto &instr : atomic->instructions()) {
				if (auto destination = execute(result, thread, *instr)) {
					result = *destination;
				} else {
					return boost::none;
				}
			}
		}
		case Instruction::LOCK: {
			if (state.memoryLock() == NULL) {
				auto result = state;
				result.setMemoryLock(thread);
				return result;
			} else {
				return boost::none;
			}
		}
		case Instruction::UNLOCK: {
			if (state.memoryLock() == thread) {
				auto result = state;
				result.setMemoryLock(NULL);
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
		if (state.memoryLock() == NULL || state.memoryLock() == thread) {
			auto controlState = threadAndState.second;
			for (auto transition : controlState->out()) {
				if (auto destination = execute(state, thread, *transition->instruction())) {
					destination->setControlState(thread, transition->to());
					result.push_back(SCTransition(state, std::move(*destination), transition->instruction().get()));
				}
			}
		}
	}

	return result;
}

SCSemantics::Label SCSemantics::getLabel(const Transition &transition) const {
	std::stringstream out;
	printInstruction(out, *transition.instruction());
	return out.str();
}

} // namespace trench
