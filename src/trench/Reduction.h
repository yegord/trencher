#pragma once

#include "config.h"

namespace trench {

class Program;
class Read;
class Thread;
class Write;

void reduce(const Program &program, Program &result, Thread *attacker = NULL, Read *attackRead = NULL, Write *attackWrite = NULL);

} // namespace trench
