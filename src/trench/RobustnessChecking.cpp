/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include "RobustnessChecking.h"

#include "Benchmarking.h"
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

bool isAttackFeasible(const Program &program, bool searchForTdrOnly, Thread *attacker, Transition *attackWrite, Transition *attackRead, const boost::unordered_set<State *> &fenced) {

	Statistics::instance().incPotentialAttacksCount();

	if (attackWrite && attackRead) {
		boost::unordered_set<State *> visited(fenced);
		if (!isReachable(attackWrite->to(), attackRead->from(), visited)) {
			Statistics::instance().incInfeasibleAttacksCount1();
			return false;
		}
	}

	trench::Program augmentedProgram;
	trench::reduce(program, augmentedProgram, searchForTdrOnly, attacker, attackWrite, attackRead, fenced);

	trench::SpinModelChecker checker;
	bool feasible = checker.check(augmentedProgram);

	if (feasible) {
		Statistics::instance().incFeasibleAttacksCount();
	} else {
		Statistics::instance().incInfeasibleAttacksCount2();
	}

	return feasible;
}

} // namespace trench
