/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

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
