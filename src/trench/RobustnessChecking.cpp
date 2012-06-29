#include "RobustnessChecking.h"

#include "Foreach.h"
#include "Program.h"
#include "Reduction.h"
#include "SpinModelChecker.h"
#include "State.h"
#include "Transition.h"

namespace trench {

namespace {

bool isReachable(State *state, State *target, boost::unordered_set<State *> &visited) {
	if (state == target) {
		return true;
	}

	if (visited.find(state) != visited.end()) {
		return false;
	}

	visited.insert(state);

	foreach (Transition *transition, state->out()) {
		switch (transition->instruction()->mnemonic()) {
			case Instruction::READ:
			case Instruction::WRITE:
			case Instruction::LOCAL:
			case Instruction::CONDITION:
			case Instruction::NOOP:
				if (isReachable(transition->to(), target, visited)) {
					return true;
				}
				break;
			case Instruction::MFENCE:
			case Instruction::LOCK:
			case Instruction::UNLOCK:
				break;
			default: {
				assert(!"NEVER REACHED");
			}
		}
	}

	return false;
}

} // anonymous namespace

bool isAttackFeasible(const Program &program, Thread *attacker, Transition *attackWrite, Transition *attackRead, const boost::unordered_set<State *> &fenced) {
	if (attackWrite && attackRead) {
		boost::unordered_set<State *> visited(fenced);
		if (!isReachable(attackWrite->to(), attackRead->from(), visited)) {
			return false;
		}
	}

	trench::Program augmentedProgram;
	trench::reduce(program, augmentedProgram, attacker, attackWrite, attackRead, fenced);

	trench::SpinModelChecker checker;
	return checker.check(augmentedProgram);
}

} // namespace trench
