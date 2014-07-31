/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include "Liveness.h"

#include <boost/range/adaptor/reversed.hpp>

#include "Program.h"
#include "SortAndUnique.h"
#include "State.h"
#include "Thread.h"
#include "Transition.h"

namespace trench {

namespace {

void computeKilledRegisters(const Instruction &instruction, std::vector<const Register *> &result) {
	switch (instruction.mnemonic()) {
		case Instruction::READ: {
			auto read = instruction.as<Read>();
			result.push_back(read->reg().get());
			break;
		}
		case Instruction::LOCAL: {
			auto local = instruction.as<Local>();
			result.push_back(local->reg().get());
			break;
		}
		case Instruction::ATOMIC: {
			auto atomic = instruction.as<Atomic>();
			for (const auto &instruction : atomic->instructions()) {
				computeKilledRegisters(*instruction, result);
			}
			break;
		}
		default: {
			break;
		}
	}
}

void computeUsedRegisters(const Expression &expression, std::vector<const Register *> &result) {
	switch (expression.kind()) {
		case Expression::REGISTER: {
			result.push_back(expression.as<Register>());
			break;
		}
		case Expression::UNARY: {
			auto unary = expression.as<UnaryOperator>();
			computeUsedRegisters(*unary->operand(), result);
			break;
		}
		case Expression::BINARY: {
			auto binary = expression.as<BinaryOperator>();
			computeUsedRegisters(*binary->left(), result);
			computeUsedRegisters(*binary->right(), result);
			break;
		}
		default: {
			break;
		}
	}
}

void computeUsedRegisters(const Instruction &instruction, std::vector<const Register *> &result) {
	switch (instruction.mnemonic()) {
		case Instruction::READ: {
			auto read = instruction.as<Read>();
			computeUsedRegisters(*read->address(), result);
			break;
		}
		case Instruction::WRITE: {
			auto write = instruction.as<Write>();
			computeUsedRegisters(*write->address(), result);
			computeUsedRegisters(*write->value(), result);
			break;
		}
		case Instruction::LOCAL: {
			auto local = instruction.as<Local>();
			computeUsedRegisters(*local->value(), result);
			break;
		}
		case Instruction::CONDITION: {
			auto condition = instruction.as<Condition>();
			computeUsedRegisters(*condition->expression(), result);
			break;
		}
		case Instruction::ATOMIC: {
			auto atomic = instruction.as<Atomic>();

			std::vector<const Register *> usedAndNotKilled;
			for (const auto &transition : boost::adaptors::reverse(atomic->instructions())) {
				std::vector<const Register *> killed;
				computeKilledRegisters(*transition, killed);

				usedAndNotKilled.erase(
					std::remove_if(
						usedAndNotKilled.begin(),
						usedAndNotKilled.end(),
						[&](const Register *reg){ return std::find(killed.begin(), killed.end(), reg) != killed.end(); }),
					usedAndNotKilled.end());

				computeUsedRegisters(*transition, usedAndNotKilled);
			}

			result.insert(result.end(), usedAndNotKilled.begin(), usedAndNotKilled.end());
			break;
		}
		default: {
			break;
		}
	}
}

void computeLiveness(const Thread &thread, Liveness &liveness) {
	bool changed;
	do {
		changed = false;

		for (const State *state : thread.states()) {
			std::vector<const Register *> liveAtState;

			for (const Transition *transition : state->out()) {
				const auto &liveAtDestination = liveness.getLiveRegisters(transition->to());

				std::vector<const Register *> killed;
				computeKilledRegisters(*transition->instruction(), killed);

				std::copy_if(
					liveAtDestination.begin(),
					liveAtDestination.end(),
					std::back_inserter(liveAtState),
					[&](const Register *reg){
						return std::find(killed.begin(), killed.end(), reg) == killed.end();
					}
				);

				computeUsedRegisters(*transition->instruction(), liveAtState);
			}

			sortAndUnique(liveAtState);

			auto &oldLiveAtState = liveness.getLiveRegisters(state);
			if (oldLiveAtState != liveAtState) {
				changed = true;
				oldLiveAtState = std::move(liveAtState);
			}
		}
	} while (changed);
}

} // anonymous namespace

Liveness computeLiveness(const Program &program) {
	Liveness liveness;
	for (const Thread *thread : program.threads()) {
		computeLiveness(*thread, liveness);
	}
	return liveness;
}

} // namespace trench
