#pragma once

#include <trench/config.h>

#include <iosfwd>

namespace trench {

class Expression;
class Instruction;
class Program;

void printExpression(std::ostream &out, const Expression &expression);
void printInstruction(std::ostream &out, const Instruction &instruction);
void printAsDot(std::ostream &out, const Program &program);

} // namespace trench
