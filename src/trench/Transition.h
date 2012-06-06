#pragma once

namespace trench {

class Transition {
	State *from;
	State *to;
	std::unique_ptr<Instruction> instruction_;

	public:

	Transition(State *from, State *to, const Instruction *instruction):
		from_(from), to_(to), instruction_(instruction)
	{}

	State *from() const { return from_; }
	State *to() const { return to_; }
	Instruction *instruction() const { return instruction_.get(); }
};

} // namespace trench
