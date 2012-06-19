#pragma once

#include "config.h"

namespace trench {

class Program;
class Thread;

void reduce(const Program &program, Thread *attacker, Program &result);

} // namespace trench
