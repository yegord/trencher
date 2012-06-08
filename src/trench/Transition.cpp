#include "Transition.h"

#include "Instruction.h"

namespace trench {

Transition::Transition(State *from, State *to, Instruction *instruction):
	from_(from), to_(to), instruction_(instruction)
{}

Transition::~Transition() {
}

} // namespace trench
