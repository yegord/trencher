/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include "printAsPromela.h"

#include <cassert>
#ifndef TRENCH_FRIENDLY_SPIN_NAMES
#include <sstream>
#endif

#include "Census.h"
#include "Foreach.h"
#include "Instruction.h"
#include "Program.h"
#include "State.h"
#include "Transition.h"

namespace trench {

namespace {

template<class T>
std::string ident(const T *object) {
#ifdef TRENCH_FRIENDLY_SPIN_NAMES
	return object->name();
#else
	std::stringstream ss;
	ss << "_0x" << object;
	return ss.str();
#endif
}

template<class T>
std::string ident(const std::shared_ptr<T> &ptr) {
	return ident(ptr.get());
}

void printExpression(std::ostream &out, const std::shared_ptr<Expression> &expression) {
	switch (expression->kind()) {
		case Expression::CONSTANT: {
			Constant *constant = expression->as<Constant>();
			out << constant->value();
			break;
		}
		case Expression::REGISTER: {
			Register *reg = expression->as<Register>();
			out << ident(reg);
			break;
		}
		case Expression::UNARY: {
			UnaryOperator *unary = expression->as<UnaryOperator>();
			out << unary->getOperatorSign() << '(';
			printExpression(out, unary->operand());
			out << ')';
			break;
		}
		case Expression::BINARY: {
			BinaryOperator *binary = expression->as<BinaryOperator>();
			out << '(';
			printExpression(out, binary->left());
			out << ' ' << binary->getOperatorSign() << ' ';
			printExpression(out, binary->right());
			out << ')';
			break;
		}
		case Expression::NOT_BLOCKED: {
			out << "(giant_lock == 0 || giant_lock == THREAD_ID)";
			break;
		}
		default: {
			assert(!"NEVER REACHED");
		}
	}
}

void printInstruction(std::ostream &out, const std::shared_ptr<Instruction> &instruction) {
	/* Guard. */
	if (Condition *condition = instruction->as<Condition>()) {
		out << ' ';
		printExpression(out, condition->expression());
		out << " ->";
	}

	out << ' ';

	/* Instruction itself. */
	switch (instruction->mnemonic()) {
		case Instruction::READ: {
			Read *read = instruction->as<Read>();
			out << ident(read->reg()) << " = mem" << read->space() << "[";
			printExpression(out, read->address());
			out << "];";
			break;
		}
		case Instruction::WRITE: {
			Write *write = instruction->as<Write>();
			out << "mem" << write->space() << "[";
			printExpression(out, write->address());
			out << "] = ";
			printExpression(out, write->value());
			out << ';';
			break;
		}
		case Instruction::MFENCE: {
			out << "skip /*mfence*/;" << std::endl;
			break;
		}
		case Instruction::LOCAL: {
			Local *local = instruction->as<Local>();
			out << ident(local->reg()) << " = ";
			printExpression(out, local->value());
			out << ";";
			break;
		}
		case Instruction::CONDITION: {
			out << "skip /*condition*/;";
			break;
		}
		case Instruction::ATOMIC: {
			Atomic *atomic = instruction->as<Atomic>();

			out << "atomic {";
			foreach (const auto &instr, atomic->instructions()) {
				printInstruction(out, instr);
			}
			out << " }";
			break;
		}
		case Instruction::NOOP: {
			out << "skip /*no-op*/;";
			break;
		}
		case Instruction::LOCK: {
			out << "atomic { giant_lock == 0 -> giant_lock = THREAD_ID; }";
			break;
		}
		case Instruction::UNLOCK: {
			out << "atomic { giant_lock == THREAD_ID -> giant_lock = 0; }";
			break;
		}
		default: {
			assert(!"NEVER REACHED");
		}
	}
}

} // anonymous namespace

void printAsPromela(std::ostream &out, const Program &program) {
	/* Set of used spaces. */
	std::vector<Space> spaces;

	Census programCensus;
	programCensus.visit(program);

	/* Shared memory. */
	foreach (Space space, programCensus.spaces()) {
		out << "int mem" << space << "[" << program.memorySize() << "] = " << Domain() << ';' << std::endl;
	}
	out << "int giant_lock = 0;" << std::endl;

	/* Threads. */
	int thread_id = 0;
	foreach (Thread *thread, program.threads()) {
		out << "active proctype " << ident(thread) << "() {" << std::endl;

		out << "int THREAD_ID = " << ++thread_id << ';' << std::endl;

		Census threadCensus;
		threadCensus.visit(thread);

		/* Register declarations. */
		foreach (Expression *expression, threadCensus.expressions()) {
			if (Register *reg = expression->as<Register>()) {
				out << "int " << ident(reg) << ';' << std::endl;
			}
		}

		/* Goto initial state. */
		if (thread->initialState()) {
			out << "goto " << ident(thread->initialState()) << ";" << std::endl;
		}

		/* Transitions. */
		foreach (State *state, thread->states()) {
			out << ident(state) << ": ";
			if (state->out().empty()) {
				out << "goto __done;" << std::endl;
			} else {
				out << "if" << std::endl;
				foreach (Transition *transition, state->out()) {
					out << "::";
					printInstruction(out, transition->instruction());
					out << " goto " << ident(transition->to()) << ';' << std::endl;
				}
				out << "fi;" << std::endl;
			}
		}
		out << "__done: skip;" << std::endl;
		out << "}" << std::endl;
	}

	/* Never claim. */
	if (program.interestingSpace() != INVALID_SPACE) {
		out << "never {" << std::endl;
		out << "T0_init:" << std::endl;
		out << "if" << std::endl;
		out << ":: (mem" << program.interestingSpace() << '[' << program.interestingAddress() << "]) -> goto accept_all" << std::endl;
		out << ":: (1) -> goto T0_init" << std::endl;
		out << "fi;" << std::endl;
		out << "accept_all: skip;" << std::endl;
		out << "}" << std::endl;
	}
}

} // namespace trench
