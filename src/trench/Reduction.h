#pragma once

#include "config.h"

namespace trench {

class Program;
class Thread;
class Transition;

void reduce(const Program &program, Program &result, Thread *attacker = NULL, Transition *attackWrite = NULL, Transition *attackRead = NULL);

} // namespace trench
