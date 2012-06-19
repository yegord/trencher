#pragma once

#include <memory>

namespace trench {

class State;
class Instruction;

class Transition {
	State *from_;
	State *to_;
	std::shared_ptr<Instruction> instruction_;

	public:

	Transition(State *from, State *to, const std::shared_ptr<Instruction> &instruction);
	~Transition();

	State *from() const { return from_; }
	State *to() const { return to_; }
	const std::shared_ptr<Instruction> &instruction() const { return instruction_; }
};

} // namespace trench
