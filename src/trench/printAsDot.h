#pragma once

#include <trench/config.h>

#include <iosfwd>

namespace trench {

class Program;

void printAsDot(std::ostream &out, const Program &program);

} // namespace trench
