/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include "Thread.h"

#include "Foreach.h"
#include "State.h"
#include "Transition.h"

namespace trench {

Thread::Thread(const std::string &name): name_(name), initialState_(NULL) {}

Thread::~Thread() {
	foreach (Transition *transition, transitions_) {
		delete transition;
	}
}

State *Thread::makeState(const std::string &name) {
	auto &result = name2state_[name];
	if (!result) {
		result.reset(new State(name));
		states_.push_back(result.get());
	}
	return result.get();
}

Transition *Thread::makeTransition(State *from, State *to, const std::shared_ptr<Instruction> &instruction) {
	std::unique_ptr<Transition> result(new Transition(from, to, instruction));
	transitions_.push_back(result.get());

	from->out_.push_back(result.get());
	to->in_.push_back(result.get());

	return result.release();
}

} // namespace trench
