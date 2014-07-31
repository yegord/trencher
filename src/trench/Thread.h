/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <trench/config.h>

#include <memory>
#include <string>
#include <vector>

#include <boost/unordered_map.hpp>

namespace trench {

class Instruction;
class State;
class Transition;

class Thread {
	const std::string name_;

	boost::unordered_map<std::string, std::unique_ptr<State>> name2state_;
	std::vector<State *> states_;

	State *initialState_;

	std::vector<std::unique_ptr<Transition>> transitions_;

public:
	explicit
	Thread(std::string name);
	~Thread();

	const std::string &name() const { return name_; }

	const std::vector<State *> &states() const { return states_; }
	State *makeState(const std::string &name);

	State *initialState() const { return initialState_; }
	void setInitialState(State *state) { initialState_ = state; }

	const std::vector<Transition *> &transitions() const { return reinterpret_cast<const std::vector<Transition *> &>(transitions_); }
	Transition *makeTransition(State *from, State *to, std::shared_ptr<Instruction> instruction);
};

} // namespace trench
