#include "DotPrinter.h"

#include <cassert>

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
      out << "not_blocked";
      break;
    }
		default: {
			assert(!"NEVER REACHED");
		}
	}
}

void printInstruction(std::ostream &out, const std::shared_ptr<Instruction> &instruction) {
	switch (instruction->mnemonic()) {
		case Instruction::READ: {
			Read *read = instruction->as<Read>();
			out << read->reg()->name() << ":= mem";
			if (read->space()) {
				out << read->space();
			}
			out << '[';
			printExpression(out, read->address());
			out << "];";
			break;
		}
		case Instruction::WRITE: {
			Write *write = instruction->as<Write>();
			out << "mem";
			if (write->space()) {
				out << write->space();
			}
			out << '[';
			printExpression(out, write->address());
			out << "]:=";
			printExpression(out, write->value());
			out << ';';
			break;
		}
		case Instruction::MFENCE: {
			out << "mfence;";
			break;
		}
		case Instruction::LOCAL: {
			Local *local = instruction->as<Local>();
			printExpression(out, local->reg());
			out << ":=";
			printExpression(out, local->value());
			out << ';';
			break;
		}
		case Instruction::CONDITION: {
			Condition *condition = instruction->as<Condition>();
			out << "check ";
			printExpression(out, condition->expression());
			out << ';';
			break;
		}
		case Instruction::ATOMIC: {
			Atomic *atomic = instruction->as<Atomic>();
			out << "atomic {\\n";
			foreach (const auto &instr, atomic->instructions()) {
				printInstruction(out, instr);
				out << "\\n";
			}
			out << "};";
			break;
		}
		case Instruction::NOOP: {
      out << "noop";
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

} // namespace

void DotPrinter::print(std::ostream &out, const Program &program) {
	out << "digraph threads {" << std::endl;
	foreach (const Thread *thread, program.threads()) {
		out << "subgraph cluster" << this << " {" << std::endl;
		out << "state" << thread << "[shape=\"point\"];" << std::endl;
    foreach (const State *state, thread->states()) {
      if (state == thread->finalState()) {
			  out << "state" << state << "[shape=\"ellipse\",peripheries=2,label=\"" << state->name() << "\"];" << std::endl;
      } else {
			  out << "state" << state << "[shape=\"ellipse\",label=\"" << state->name() << "\"];" << std::endl;
		  }
      if (state == thread->initialState()) {
        out << "state" << thread << "->state" << state << "[];" << std::endl;
      }
    }
		foreach (const Transition *transition, thread->transitions()) {
			out << "state" << transition->from() << "->state" << transition->to() << "[label=\"";
			printInstruction(out, transition->instruction());
			out << "\"];" << std::endl;
		}
		out << '}' << std::endl;
	}
	out << "}" << std::endl;
}

} // namespace trench
