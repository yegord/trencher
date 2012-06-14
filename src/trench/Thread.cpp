#include "Thread.h"

#include "State.h"
#include "Transition.h"

namespace trench {

Thread::Thread(const std::string &name): name_(name) {}

Thread::~Thread() {}

State *Thread::makeState(const std::string &name) {
	auto &result = name2state_[name];
	if (!result) {
		result.reset(new State(name));
		states_.push_back(result.get());
	}
	return result.get();
}

Transition *Thread::makeTransition(State *from, State *to, Instruction *instruction) {
	Transition *result = new Transition(from, to, instruction);
	transitions_.push_back(std::unique_ptr<Transition>(result));

	from->out_.push_back(result);
	to->in_.push_back(result);

	return result;
}

} // namespace trench
