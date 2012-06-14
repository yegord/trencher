#include "Transition.h"

#include <cassert>

#include "Instruction.h"

namespace trench {

Transition::Transition(State *from, State *to, Instruction *instruction):
	from_(from), to_(to), instruction_(instruction)
{
	assert(from);
	assert(to);
	assert(instruction);
}

Transition::~Transition() {
}

} // namespace trench
