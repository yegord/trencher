#pragma once

#include <memory>

namespace trench {

class State;
class Instruction;

class Transition {
	State *from_;
	State *to_;
	std::unique_ptr<Instruction> instruction_;

	public:

	Transition(State *from, State *to, Instruction *instruction);
	~Transition();

	State *from() const { return from_; }
	State *to() const { return to_; }
	Instruction *instruction() const { return instruction_.get(); }
};

} // namespace trench
