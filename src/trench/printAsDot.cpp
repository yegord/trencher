#include "printAsDot.h"

#include <cassert>
#include <ostream>

#include "Instruction.h"
#include "Program.h"
#include "State.h"
#include "Transition.h"

namespace trench {

void printExpression(std::ostream &out, const Expression &expression) {
	switch (expression.kind()) {
		case Expression::CONSTANT: {
			auto constant = expression.as<Constant>();
			out << constant->value();
			break;
		}
		case Expression::REGISTER: {
			auto reg = expression.as<Register>();
			out << reg->name();
			break;
		}
		case Expression::UNARY: {
			auto unary = expression.as<UnaryOperator>();
			out << unary->getOperatorSign() << '(';
			printExpression(out, *unary->operand());
			out << ')';
			break;
		}
		case Expression::BINARY: {
			auto binary = expression.as<BinaryOperator>();
			out << '(';
			printExpression(out, *binary->left());
			out << ' ' << binary->getOperatorSign() << ' ';
			printExpression(out, *binary->right());
			out << ')';
			break;
		}
		case Expression::CAN_ACCESS_MEMORY: {
			out << "can_access_memory";
			break;
		}
		default: {
			assert(!"NEVER REACHED");
		}
	}
}

void printInstruction(std::ostream &out, const Instruction &instruction) {
	switch (instruction.mnemonic()) {
		case Instruction::READ: {
			auto read = instruction.as<Read>();
			out << read->reg()->name() << ":= mem";
			if (read->space()) {
				out << read->space();
			}
			out << '[';
			printExpression(out, *read->address());
			out << "];";
			break;
		}
		case Instruction::WRITE: {
			auto write = instruction.as<Write>();
			out << "mem";
			if (write->space()) {
				out << write->space();
			}
			out << '[';
			printExpression(out, *write->address());
			out << "]:=";
			printExpression(out, *write->value());
			out << ';';
			break;
		}
		case Instruction::MFENCE: {
			out << "mfence;";
			break;
		}
		case Instruction::LOCAL: {
			auto local = instruction.as<Local>();
			printExpression(out, *local->reg());
			out << ":=";
			printExpression(out, *local->value());
			out << ';';
			break;
		}
		case Instruction::CONDITION: {
			auto condition = instruction.as<Condition>();
			out << "check ";
			printExpression(out, *condition->expression());
			out << ';';
			break;
		}
		case Instruction::ATOMIC: {
			auto atomic = instruction.as<Atomic>();
			out << "atomic {\\n";
			for (const auto &instr : atomic->instructions()) {
				printInstruction(out, *instr);
				out << "\\n";
			}
			out << "};";
			break;
		}
		case Instruction::NOOP: {
			break;
		}
		case Instruction::LOCK: {
			out << "lock";
			break;
		}
		case Instruction::UNLOCK: {
			out << "unlock";
			break;
		}
		default: {
			assert(!"NEVER REACHED");
		}
	}
}

void printAsDot(std::ostream &out, const Program &program) {
	out << "digraph threads {" << std::endl;
	for (const Thread *thread : program.threads()) {
		out << "subgraph cluster" << thread << " {" << std::endl;
		for (const State *state : thread->states()) {
			out << "state" << state << "[shape=\"ellipse\",label=\"" << state->name() << "\"];" << std::endl;
		}
		for (const Transition *transition : thread->transitions()) {
			out << "state" << transition->from() << "->state" << transition->to() << "[label=\"";
			printInstruction(out, *transition->instruction());
			out << "\"];" << std::endl;
		}
		if (thread->initialState()) {
			out << "initial [shape=none,label=\"\"];" << std::endl;
			out << "initial -> state" << thread->initialState() << ';' << std::endl;
		}
		out << '}' << std::endl;
	}
	out << "}" << std::endl;
}

} // namespace trench
