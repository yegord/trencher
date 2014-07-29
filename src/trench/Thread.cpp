/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include "Thread.h"

#include "State.h"
#include "Transition.h"
#include "make_unique.h"

namespace trench {

Thread::Thread(std::string name):
	name_(std::move(name)),
	initialState_(NULL)
{}

Thread::~Thread() {}

State *Thread::makeState(const std::string &name) {
	auto &result = name2state_[name];
	if (!result) {
		result.reset(new State(name));
		states_.push_back(result.get());
	}
	return result.get();
}

Transition *Thread::makeTransition(State *from, State *to, std::shared_ptr<Instruction> instruction) {
	auto transition = make_unique<Transition>(from, to, std::move(instruction));
	auto result = transition.get();

	transitions_.push_back(std::move(transition));
	from->out_.push_back(result);
	to->in_.push_back(result);

	return result;
}

} // namespace trench
