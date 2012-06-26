#pragma once

#include <vector>

namespace trench {

class Program;
class Thread;
class State;

typedef std::vector<std::pair<Thread *, State *>> FenceSet;

FenceSet computeFences(const Program &program);

} // namespace trench
