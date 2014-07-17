#include "MPrinter.h"

#include <cassert>

#include "Census.h"
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
			out << "$" << reg->name();
			break;
		}
		case Expression::UNARY: {
			UnaryOperator *unary = expression->as<UnaryOperator>();
			out << unary->getOperatorSign();// << '(';
			printExpression(out, unary->operand());
			//out << ')';
			break;
		}
		case Expression::BINARY: {
			BinaryOperator *binary = expression->as<BinaryOperator>();
			//out << '(';
			printExpression(out, binary->left());
			out << ' ' << binary->getOperatorSign() << ' ';
			printExpression(out, binary->right());
			//out << ')';
			break;
		}
    case Expression::NOT_BLOCKED: {
      out << "/* not_blocked */";
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
      out << " read: $" << read->reg()->name() << " := x"; printExpression(out, read->address()); out << ";";
			break;
		}
		case Instruction::WRITE: {
			Write *write = instruction->as<Write>();
      out << " write: x"; printExpression(out, write->address()); out << " := "; printExpression(out, write->value()); out << ";";
			break;
		}
		case Instruction::MFENCE: {
			out << " /* mfence */";
			break;
		}
		case Instruction::LOCAL: {
			Local *local = instruction->as<Local>();
			out << " ";
      printExpression(out, local->reg());
			out << " = ";
			printExpression(out, local->value());
			out << ';';
			break;
		}
		case Instruction::CONDITION: {
			Condition *condition = instruction->as<Condition>();
			out << " if ";
			printExpression(out, condition->expression());
      out << " then";
			break;
		}
		case Instruction::NOOP: {
      out << " /* noop */ ";
			break;
		}
		case Instruction::LOCK: {
			out << " pthread_mutex_lock(&mutex);"; // has to be changed
			break;
		}
		case Instruction::UNLOCK: {
      out << " pthread_mutex_unlock(&mutex);"; // has to be changed
			break;
		}
		default: {
			assert(!"NEVER REACHED");
		}
	}
}

} // namespace

void MPrinter::print(std::ostream &out, const Program &program) {
  Census programCensus;
  programCensus.visit(program);
  int threadCount = (int) program.threads().size();

  out << "forbidden" << std::endl << " ";
  for (int i = 0; i < threadCount; i++)
    out << " ASSERT";
  out << std::endl << "data" << std::endl;

  for (int i = 0; i < program.memorySize(); i++) 
    out << "x" << i << " = * : [0:1] " << std::endl;
  // out << "giant_lock = * : [0:" << threadCount << "]" << std::endl;
  out << std::endl;

  /* Threads */
	foreach (const Thread *thread, program.threads()) {
    out << "process" << std::endl;
    
    Census threadCensus;
    threadCensus.visit(thread);

    out << "  registers" << std::endl;
    /* Registers */
    foreach (Expression *expression, threadCensus.expressions()) {
      if (Register *reg = expression->as<Register>()) {
        out << "    $" << reg->name() << " = * : [0:1]" << std::endl;
      }
    }
    out << "  text" << std::endl;
    /* Go to initial state */
    out << "    goto l" << thread->initialState()->name() << ";" << std::endl;
    /* Transitions */
    foreach (const State *state, thread->states()) {
      /* if final state then go to ASSERT */
      if (state == thread->finalState()) {
        out << "  l" << state->name() << ":" << std::endl;
        out << "    nop; goto ASSERT;" << std::endl;
      }
      foreach (const Transition *transition, state->out()) {
        out << "  l" << state->name() << ":";
        printInstruction(out,transition->instruction());
        out << " goto l" << transition->to()->name() << ";";
  //        if (transition->instruction()->is<Condition>())
  //          out << " else goto l" << transition->from()->name() << ";"; 
        out << std::endl;
      }
    }
    out << "  ASSERT: nop" << std::endl << std::endl;
	}
}

} // namespace trench
