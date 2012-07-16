/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include "NaivePrinter.h"

#include <cassert>

#include "Expression.h"
#include "Foreach.h"
#include "Instruction.h"
#include "Program.h"
#include "State.h"
#include "Transition.h"

namespace trench {

namespace {

void printExpression(std::ostream &out, const std::shared_ptr<Expression> &expression) {
	switch (expression->kind()) {
		case Expression::CONSTANT: {
			Constant *constant = expression->as<Constant>();
			out << constant->value();
			break;
		}
		case Expression::REGISTER: {
			Register *reg = expression->as<Register>();
			out << reg->name();
			break;
		}
		case Expression::BINARY: {
			BinaryOperator *binary = expression->as<BinaryOperator>();
			switch (binary->kind()) {
				case BinaryOperator::EQ: {
					out << "==";
					break;
				}
				case BinaryOperator::NEQ: {
					out << "!=";
					break;
				}
				case BinaryOperator::LT:
					out << '<';
					break;
				case BinaryOperator::LEQ:
					out << "<=";
					break;
				case BinaryOperator::GT:
					out << '>';
					break;
				case BinaryOperator::GEQ:
					out << ">=";
					break;
				case BinaryOperator::AND:
					out << "&&";
					break;
				case BinaryOperator::OR:
					out << "||";
					break;
				case BinaryOperator::ADD:
					out << '+';
					break;
				case BinaryOperator::SUB:
					out << '-';
					break;
				case BinaryOperator::MUL:
					out << '*';
					break;
				case BinaryOperator::BIN_AND:
					out << '&';
					break;
				default: {
					 assert(!"NEVER REACHED");
				}
			}

			out << ' ';
			printExpression(out, binary->left());
			out << ' ';
			printExpression(out, binary->right());
		}
		default: {
			assert(!"NEVER REACHED");
		}
	}
}

} // anonymous namespace

void NaivePrinter::print(std::ostream &out, const Program &program) const {
	out << "memory_size " << program.memorySize() << std::endl;

	foreach (const Thread *thread, program.threads()) {
		out << "thread " << thread->name() << std::endl;
		if (thread->initialState()) {
			out << "initial " << thread->initialState()->name() << std::endl;
		}
		foreach (Transition *transition, thread->transitions()) {
			out << "transition " << transition->from()->name() << " " << transition->to()->name() << '\t';
			switch (transition->instruction()->mnemonic()) {
				case Instruction::READ: {
					Read *read = transition->instruction()->as<Read>();
					out << "read ";
					printExpression(out, read->reg());
					out << " ";
					printExpression(out, read->address());
					break;
				}
				case Instruction::WRITE: {
					Write *write = transition->instruction()->as<Write>();
					out << "write ";
					printExpression(out, write->value());
					out << '\t';
					printExpression(out, write->address());
					break;
				}
				case Instruction::MFENCE: {
					out << "mfence";
					break;
				}
				case Instruction::LOCAL: {
					Local *local = transition->instruction()->as<Local>();
					out << "local ";
					printExpression(out, local->reg());
					out << '\t';
					printExpression(out, local->value());
					break;
				}
				case Instruction::CONDITION: {
					Condition *condition = transition->instruction()->as<Condition>();
					out << "check ";
					printExpression(out, condition->expression());
					break;
				}
				default: {
					assert(!"NEVER REACHED");
				}

			}
			out << std::endl;
		}
		out << "end" << std::endl;
	}
}

} // namespace trench
