#include "SpinPrinter.h"

#include <cassert>

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
		default: {
			assert(!"NEVER REACHED");
		}
	}
}

} // anonymous namespace

void SpinPrinter::print(std::ostream &out, const Program &program) const {
	/* Approximate memory size. */
	out << "int mem[10];" << std::endl;

	/* Threads. */
	foreach (Thread *thread, program.threads()) {
		out << "active proctype " << ident(thread) << "() {" << std::endl;

		/* Register declarations. */
		foreach (const auto &reg, program.registers()) {
			out << "int " << ident(reg) << ';' << std::endl;
		}

		/* Goto initial state. */
		if (thread->initialState()) {
			out << "goto " << ident(thread->initialState()) << ";" << std::endl;
		}

		/* Transitions. */
		foreach (State *state, thread->states()) {
			out << ident(state) << ": ";
			if (state->out().empty()) {
				out << "skip;" << std::endl;
			} else {
				out << "if" << std::endl;
				foreach (Transition *transition, state->out()) {
					out << ":: ";

					/* Guard. */
					if (Condition *condition = transition->instruction()->as<Condition>()) {
						out << "(";
						printExpression(out, condition->expression());
						out << " == 0) ->";
					}

					/* Instruction itself. */
					switch (transition->instruction()->mnemonic()) {
						case Instruction::READ: {
							Read *read = transition->instruction()->as<Read>();
							out << ident(read->reg()) << " = mem[";
							printExpression(out, read->address());
							out << "];";
							break;
						}
						case Instruction::WRITE: {
							Write *write = transition->instruction()->as<Write>();
							out << "mem[";
							printExpression(out, write->address());
							out << "] = ";
							printExpression(out, write->value());
							out << ";";
							break;
						}
						case Instruction::MFENCE: {
							out << "/* mfence */";
							break;
						}
						case Instruction::LOCAL: {
							Local *local = transition->instruction()->as<Local>();
							out << ident(local->reg()) << " = ";
							printExpression(out, local->value());
							out << ";";
							break;
						}
						case Instruction::CONDITION: {
							/* Nothing. */
							break;
						}
						default: {
							assert(!"NEVER REACHED");
						}
					}
					out << " goto " << ident(transition->to()) << ';' << std::endl;
				}
				out << "fi;" << std::endl;
			}
		}
		out << "}" << std::endl;
	}
}

} // namespace trench
