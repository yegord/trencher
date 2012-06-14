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
		default: {
			assert(!"NEVER REACHED");
		}
	}
}

} // anonymous namespace

void NaivePrinter::print(std::ostream &out, const Program &program) const {
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
