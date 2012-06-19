#pragma once

#include "config.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace trench {

class Instruction;
class State;
class Transition;

class Thread {
	const std::string name_;

	std::map<std::string, std::unique_ptr<State>> name2state_;
	std::vector<State *> states_;

	State *initialState_;

	std::vector<Transition *> transitions_;

	public:

	Thread(const std::string &name);
	~Thread();

	const std::string &name() const { return name_; }

	const std::vector<State *> &states() const { return states_; }
	State *makeState(const std::string &name);

	State *initialState() const { return initialState_; }
	void setInitialState(State *state) { initialState_ = state; }

	const std::vector<Transition *> &transitions() const { return transitions_; }
	Transition *makeTransition(State *from, State *to, const std::shared_ptr<Instruction> &instruction);
};

} // namespace trench
