/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include "NaiveParser.h"

#include <sstream>
#include <stdexcept>

#include "Instruction.h"
#include "Program.h"

namespace trench {

namespace {

std::shared_ptr<Register> parseRegister(std::istream &in, Program &program) {
	std::string token;
	if (!(in >> token)) {
		throw std::runtime_error("expected a register name, got EOF");
	}
	return program.makeRegister(token);
}

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
	} else if (token == "<") {
		std::shared_ptr<Expression> left = parseExpression(in, program);
		std::shared_ptr<Expression> right = parseExpression(in, program);
		return std::make_shared<BinaryOperator>(BinaryOperator::LT, left, right);
	} else if (token == "<=") {
		std::shared_ptr<Expression> left = parseExpression(in, program);
		std::shared_ptr<Expression> right = parseExpression(in, program);
		return std::make_shared<BinaryOperator>(BinaryOperator::LEQ, left, right);
	} else if (token == ">") {
		std::shared_ptr<Expression> left = parseExpression(in, program);
		std::shared_ptr<Expression> right = parseExpression(in, program);
		return std::make_shared<BinaryOperator>(BinaryOperator::GT, left, right);
	} else if (token == ">=") {
		std::shared_ptr<Expression> left = parseExpression(in, program);
		std::shared_ptr<Expression> right = parseExpression(in, program);
		return std::make_shared<BinaryOperator>(BinaryOperator::GEQ, left, right);
	} else if (token == "&&") {
		std::shared_ptr<Expression> left = parseExpression(in, program);
		std::shared_ptr<Expression> right = parseExpression(in, program);
		return std::make_shared<BinaryOperator>(BinaryOperator::AND, left, right);
	} else if (token == "||") {
		std::shared_ptr<Expression> left = parseExpression(in, program);
		std::shared_ptr<Expression> right = parseExpression(in, program);
		return std::make_shared<BinaryOperator>(BinaryOperator::OR, left, right);
	} else if (token == "+") {
		std::shared_ptr<Expression> left = parseExpression(in, program);
		std::shared_ptr<Expression> right = parseExpression(in, program);
		return std::make_shared<BinaryOperator>(BinaryOperator::ADD, left, right);
	} else if (token == "-") {
		std::shared_ptr<Expression> left = parseExpression(in, program);
		std::shared_ptr<Expression> right = parseExpression(in, program);
		return std::make_shared<BinaryOperator>(BinaryOperator::SUB, left, right);
	} else if (token == "*") {
		std::shared_ptr<Expression> left = parseExpression(in, program);
		std::shared_ptr<Expression> right = parseExpression(in, program);
		return std::make_shared<BinaryOperator>(BinaryOperator::MUL, left, right);
	} else if (token == "^") {
		std::shared_ptr<Expression> left = parseExpression(in, program);
		std::shared_ptr<Expression> right = parseExpression(in, program);
    
    std::shared_ptr<Expression> result = program.makeConstant(1);
    for (int i=0; i < right->as<Constant>()->value(); i++) {
      std::shared_ptr<Expression> tmp = std::make_shared<BinaryOperator>(BinaryOperator::MUL, left, result);
      result = tmp;
    }
    return result;
  } else if (token == "&") {
		std::shared_ptr<Expression> left = parseExpression(in, program);
		std::shared_ptr<Expression> right = parseExpression(in, program);
		return std::make_shared<BinaryOperator>(BinaryOperator::BIN_AND, left, right);
	} else if (token == "!") {
		std::shared_ptr<Expression> operand = parseExpression(in, program);
		return std::make_shared<UnaryOperator>(UnaryOperator::NOT, operand);
	} else {
		return program.makeRegister(token);
	}

	return std::shared_ptr<Expression>();
}

} // anonymous namespace

void NaiveParser::parse(std::istream &in, Program &program) {
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
					throw std::runtime_error("expected `initial', `transition', or `end', got EOF");
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
				} else if (token == "final") {
          if (!(in >> token)) {
            std::runtime_error("expected final state, got EOF");
          }
          State *final = thread->makeState(token);
          thread->setFinalState(final);
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
						auto reg = parseRegister(in, program);
						auto address = parseExpression(in, program);
						
						instruction.reset(new Read(reg, address));
					} else if (token == "write") {
						auto value = parseExpression(in, program);
						auto address = parseExpression(in, program);

						instruction.reset(new Write(value, address));
					} else if (token == "mfence") {
						instruction.reset(new Mfence());
					} else if (token == "local") {
						auto reg = parseRegister(in, program);
						auto value = parseExpression(in, program);
						instruction.reset(new Local(reg, value));
					} else if (token == "check") {
						auto expression = parseExpression(in, program);
						instruction.reset(new Condition(expression));
					} else if (token == "noop") {
						instruction.reset(new Noop());
					} else if (token == "lock") {
						instruction.reset(new Lock());
					} else if (token == "unlock") {
						instruction.reset(new Unlock());
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
