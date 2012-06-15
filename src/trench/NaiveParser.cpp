#include "NaiveParser.h"

#include <sstream>
#include <stdexcept>

#include "Instruction.h"
#include "Program.h"

namespace trench {

namespace {

std::shared_ptr<Expression> parseExpression(std::istream &in, Program &program) {
	std::string token;

	if (!(in >> token)) {
		throw std::runtime_error("expected an expression term, got EOF");
	}

	std::istringstream ss(token);
	int value;
	if (ss >> value) {
		return program.makeConstant(value);
	} else {
		return program.makeRegister(token);
	}

	return std::shared_ptr<Expression>();
}

} // anonymous namespace

void NaiveParser::parse(std::istream &in, Program &program) const {
	in >> std::skipws;

	std::string token;

	while (in >> token) {
		if (token == "memory_size") {
			int value;
			if (!(in >> value)) {
				throw std::runtime_error("expected a memory size value (integer), got EOF");
			}
			program.setMemorySize(value);
		} else if (token == "thread") {
			if (!(in >> token)) {
				throw std::runtime_error("expected thread name after `thread'");
			}
			if (program.getThread(token)) {
				throw std::runtime_error("redefinition of thread `" + token + "'");
			}
			Thread *thread = program.makeThread(token);

			while (true) {
				if (!(in >> token)) {
					std::runtime_error("expected `initial', `transition', or `end', got EOF");
				} else if (token == "end") {
					break;
				} else if (token == "initial") {
					if (!(in >> token)) {
						std::runtime_error("expected initial state, got EOF");
					}
					State *initial = thread->makeState(token);
					thread->setInitialState(initial);
				} else if (token == "transition") {
					if (!(in >> token)) {
						std::runtime_error("expected source state, got EOF");
					}
					State *from = thread->makeState(token);

					if (!(in >> token)) {
						std::runtime_error("expected destination state, got EOF");
					}
					State *to = thread->makeState(token);

					std::unique_ptr<Instruction> instruction = NULL;
					if (!(in >> token)) {
						throw std::runtime_error("expected an instruction name, got EOF");
					} else if (token == "read") {
						if (!(in >> token)) {
							throw std::runtime_error("expected a register name, got EOF");
						}
						auto reg = program.makeRegister(token);
						auto expression = parseExpression(in, program);
						
						instruction.reset(new Read(reg, expression));
					} else if (token == "write") {
						auto value = parseExpression(in, program);
						auto address = parseExpression(in, program);

						instruction.reset(new Write(value, address));
					} else if (token == "mfence") {
						instruction.reset(new Mfence());
					} else if (token == "local") {
						if (!(in >> token)) {
							throw std::runtime_error("expected a register name, got EOF");
						}
						auto reg = program.makeRegister(token);
						auto value = parseExpression(in, program);
						instruction.reset(new Local(reg, value));
					} else if (token == "check") {
						auto expression = parseExpression(in, program);
						instruction.reset(new Condition(expression));
					} else {
						throw std::runtime_error("unknown instruction `" + token + "'");
					}

					thread->makeTransition(from, to, instruction.release());
				} else {
					throw std::runtime_error("expected `initial', `transition' or `end', got `" + token +"'");
				}
			}
		} else {
			throw std::runtime_error("expected `memory_size' or `thread', got `" + token + "'");
		}
	}
}

} // namespace trench
