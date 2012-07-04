#pragma once

#include <vector>

namespace trench {

class Program;
class Thread;
class State;

typedef std::pair<Thread *, State *> Fence;
typedef std::vector<Fence> FenceSet;

FenceSet computeFences(const Program &program, bool searchForTdrOnly);

} // namespace trench
