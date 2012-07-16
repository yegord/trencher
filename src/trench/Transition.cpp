/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include "Transition.h"

#include <cassert>

#include "Instruction.h"

namespace trench {

Transition::Transition(State *from, State *to, const std::shared_ptr<Instruction> &instruction):
	from_(from), to_(to), instruction_(instruction)
{
	assert(from);
	assert(to);
	assert(instruction);
}

Transition::~Transition() {
}

} // namespace trench
