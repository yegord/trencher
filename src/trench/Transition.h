/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

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
