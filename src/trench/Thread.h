#pragma once

#include "config.h"

#include <memory>

namespace trench {

class Thread {
	const std::string &name_;

	std::map<std::string, std::unique_ptr<State>> name2state_;
	std::vector<State *> states_;

	std::vector<std::unique_ptr<Transition>> transitions_;

	public:

	Thread(const std::string &name);

	const std::string &name() const { return name_; }

	const std::vector<State *> &states() const { return states_; }
	State *makeState(const std::string &name);

	Transition *makeTransition(State *from, State *to, Instruction *instruction);
};

} // namespace trench
