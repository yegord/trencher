#include "CPrinter.h"

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
//      out << "    auxreg = "; printExpression(out, read->address()); out << ";" << std::endl;
//      out << "    " << read->reg()->name() << " = x[auxreg];";
      out << "    " << read->reg()->name() << " = x"; printExpression(out, read->address()); out << ";";
			break;
		}
		case Instruction::WRITE: {
			Write *write = instruction->as<Write>();
//			out << "    auxreg = "; printExpression(out, write->address()); out << ";" << std::endl;
//      out << "    x[auxreg] = "; printExpression(out, write->value()); out << ';';
      out << "    x"; printExpression(out, write->address()); out << " = "; printExpression(out, write->value()); out << ";";
			break;
		}
		case Instruction::MFENCE: {
			out << "    /* mfence */";
			break;
		}
		case Instruction::LOCAL: {
			Local *local = instruction->as<Local>();
			out << "    ";
      printExpression(out, local->reg());
			out << " = ";
			printExpression(out, local->value());
			out << ';';
			break;
		}
		case Instruction::CONDITION: {
			Condition *condition = instruction->as<Condition>();
			out << "    if ";
			printExpression(out, condition->expression());
			break;
		}
		case Instruction::NOOP: {
      out << "    /* noop */ ";
			break;
		}
		case Instruction::LOCK: {
			out << "    pthread_mutex_lock(&mutex);";
			break;
		}
		case Instruction::UNLOCK: {
      out << "    pthread_mutex_unlock(&mutex);"; 
			break;
		}
		default: {
			assert(!"NEVER REACHED");
		}
	}
}

} // namespace

void CPrinter::print(std::ostream &out, const Program &program) {
  Census programCensus;
  programCensus.visit(program);

	out << "#include <pthread.h>" << std::endl << "int ";
  for (int iter=0; iter < program.memorySize(); iter++) {
    out << "x" << iter << ", ";
  }
  out << "tid;" << std::endl 
     // << "pthread_mutex_t mutex;" << std::endl 
      << std::endl;
	   // << "#define assert(e) if (!(e)) ERROR: goto ERROR;" << std::endl
     // << "int x[" << program.memorySize() << "], tid;" << std::endl

  /* Threads */
  int iter = 0;
	foreach (const Thread *thread, program.threads()) {
    out << "void *thr" << iter << "() {" << std::endl;
    
    Census threadCensus;
    threadCensus.visit(thread);

    out << "  int ";
    /* Registers */
    foreach (Expression *expression, threadCensus.expressions()) {
      if (Register *reg = expression->as<Register>()) {
        out << reg->name() << " = 0, ";
      }
    }
    // out << "auxreg, ";
    out << "thread_id = " << iter << ";" << std::endl;
    /* Go to initial state */
    out << "  goto l" << thread->initialState()->name() << ";" << std::endl;
    /* Transitions */
    foreach (const State *state, thread->states()) {
      out << "  l" << state->name() << ":" << std::endl;
      /* if final state then ERROR on non-mutex */
      if (state == thread->finalState()) {
        out << "    /* critical section */" << std::endl
            << "    tid = thread_id;" << std::endl
            << "    if (tid!=thread_id) ERROR: goto ERROR;;" << std::endl;
      }
      if (state->out().empty()) {
        out << "    goto done;" << std::endl;
      } else {
        /* we put conditions first: nondeterminism is an issue */
        foreach (const Transition *transition, state->out()) {
          if (transition->instruction()->is<Condition>()) {
            printInstruction(out,transition->instruction());
            out << " goto l" << transition->to()->name() << ";" << std::endl;
          }
        }
        foreach (const Transition *transition, state->out()) {
          if (!transition->instruction()->is<Condition>()) {
            printInstruction(out,transition->instruction());
            out << " goto l" << transition->to()->name() << ";" << std::endl;
          }
        }
      }
    }
    out << "  done: return 0;" << std::endl << '}' << std::endl;
    iter++;
	}

  int threadCount = (int) program.threads().size();
  
  out << "int main() {" << std::endl << "  pthread_t";
	for (int iter = 0; iter < threadCount; iter++) {
    out << " t" << iter;
    if (iter != threadCount - 1) {
      out << ",";
    } else {
      out << ";";
    }
  }
  out << std::endl << std::endl;

  for (int iter = 0; iter < threadCount; iter++) 
    out << "  pthread_create(&t" << iter << ", 0, thr" << iter << ", 0);" << std::endl;
  for (int iter = 0; iter < threadCount; iter++) 
    out << "  pthread_join(&t" << iter << ", 0);" << std::endl;
	
  out << std::endl << "  return 0;" << std::endl << "}" << std::endl;
}

} // namespace trench
