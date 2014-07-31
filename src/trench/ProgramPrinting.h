#pragma once

#include <trench/config.h>

#include <iosfwd>

namespace trench {

class Expression;
class Instruction;
class Program;

void printExpression(const Expression &expression, std::ostream &out);
void printInstruction(const Instruction &instruction, std::ostream &out);
void printProgramAsDot(const Program &program, std::ostream &out);

} // namespace trench
