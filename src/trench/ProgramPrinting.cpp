#include "ProgramPrinting.h"

#include <cassert>
#include <ostream>

#include "Instruction.h"
#include "Program.h"
#include "State.h"
#include "Transition.h"

namespace trench {

void printExpression(const Expression &expression, std::ostream &out) {
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
			printExpression(*unary->operand(), out);
			out << ')';
			break;
		}
		case Expression::BINARY: {
			auto binary = expression.as<BinaryOperator>();
			out << '(';
			printExpression(*binary->left(), out);
			out << ' ' << binary->getOperatorSign() << ' ';
			printExpression(*binary->right(), out);
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

void printInstruction(const Instruction &instruction, std::ostream &out) {
	switch (instruction.mnemonic()) {
		case Instruction::READ: {
			auto read = instruction.as<Read>();
			out << read->reg()->name() << ":= mem";
			if (read->space()) {
				out << read->space();
			}
			out << '[';
			printExpression(*read->address(), out);
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
			printExpression(*write->address(), out);
			out << "]:=";
			printExpression(*write->value(), out);
			out << ';';
			break;
		}
		case Instruction::MFENCE: {
			out << "mfence;";
			break;
		}
		case Instruction::LOCAL: {
			auto local = instruction.as<Local>();
			printExpression(*local->reg(), out);
			out << ":=";
			printExpression(*local->value(), out);
			out << ';';
			break;
		}
		case Instruction::CONDITION: {
			auto condition = instruction.as<Condition>();
			out << "check ";
			printExpression(*condition->expression(), out);
			out << ';';
			break;
		}
		case Instruction::ATOMIC: {
			auto atomic = instruction.as<Atomic>();
			out << "atomic {\\n";
			for (const auto &instr : atomic->instructions()) {
				printInstruction(*instr, out);
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

void printProgramAsDot(const Program &program, std::ostream &out) {
	out << "digraph threads {" << std::endl;
	for (const Thread *thread : program.threads()) {
		out << "subgraph cluster" << thread << " {" << std::endl;
		for (const State *state : thread->states()) {
			out << "state" << state << "[shape=\"ellipse\",label=\"" << state->name() << "\"];" << std::endl;
		}
		for (const Transition *transition : thread->transitions()) {
			out << "state" << transition->from() << "->state" << transition->to() << "[label=\"";
			printInstruction(*transition->instruction(), out);
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
