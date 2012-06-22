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
	} else if (token == "==") {
		std::shared_ptr<Expression> left = parseExpression(in, program);
		std::shared_ptr<Expression> right = parseExpression(in, program);
		return std::make_shared<BinaryOperator>(BinaryOperator::EQ, left, right);
	} else if (token == "!=") {
		std::shared_ptr<Expression> left = parseExpression(in, program);
		std::shared_ptr<Expression> right = parseExpression(in, program);
		return std::make_shared<BinaryOperator>(BinaryOperator::NEQ, left, right);
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
		if (token == "#") {
			in >> std::noskipws;
			char c;
			while (in >> c) {
				if (c == '\n' || c == '\r') {
					break;
				}
			}
			in >> std::skipws;
		} else if (token == "memory_size") {
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
					if (!thread->initialState()) {
						throw std::runtime_error("No initial state specified for thread " + thread->name());
					}
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

					std::shared_ptr<Instruction> instruction;
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
					} else if (token == "noop") {
						instruction.reset(new Noop());
					} else {
						throw std::runtime_error("unknown instruction `" + token + "'");
					}

					thread->makeTransition(from, to, instruction);
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
