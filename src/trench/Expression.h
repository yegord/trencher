/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <trench/config.h>

#include <memory>
#include <string>

#include "Kinds.h"

namespace trench {

class Expression {
	TRENCH_CLASS_WITH_KINDS(Expression, kind)

public:
	enum Kind {
		CONSTANT,
		REGISTER,
		UNARY,
		BINARY,
		CAN_ACCESS_MEMORY,
	};

public:
	Expression(Kind kind): kind_(kind) {}
};

typedef int Domain;
typedef Domain Address;

class Constant: public Expression {
	Domain value_;

public:
	Constant(Domain value): Expression(CONSTANT), value_(value) {}

	Domain value() const { return value_; }
};

class Register: public Expression {
	std::string name_;

public:
	Register(std::string name):
		Expression(REGISTER), name_(std::move(name))
	{}

	const std::string &name() const { return name_; }
};

class UnaryOperator: public Expression {
public:
	enum Kind {
		NOT
	};

private:
	Kind kind_;
	std::shared_ptr<Expression> operand_;

public:
	UnaryOperator(Kind kind, std::shared_ptr<Expression> operand):
		Expression(UNARY), kind_(kind), operand_(std::move(operand))
	{}

	Kind kind() const { return kind_; }
	const std::shared_ptr<Expression> &operand() const { return operand_; }

	const char *getOperatorSign() const;
};

class BinaryOperator: public Expression {
public:
	enum Kind {
		EQ,
		NEQ,
		LT,
		LEQ,
		GT,
		GEQ,
		AND,
		OR,
		ADD,
		SUB,
		MUL,
		BIN_AND
	};

private:
	Kind kind_;
	std::shared_ptr<Expression> left_;
	std::shared_ptr<Expression> right_;

public:
	BinaryOperator(Kind kind, std::shared_ptr<Expression> left, std::shared_ptr<Expression> right):
		Expression(BINARY), kind_(kind), left_(std::move(left)), right_(std::move(right))
	{}

	Kind kind() const { return kind_; }
	const std::shared_ptr<Expression> &left() const { return left_; }
	const std::shared_ptr<Expression> &right() const { return right_; }

	const char *getOperatorSign() const;
};

class CanAccessMemory: public Expression {
public:
	CanAccessMemory():
		Expression(CAN_ACCESS_MEMORY)
	{}
};

} // namespace trench

TRENCH_REGISTER_CLASS_KIND(Expression, Constant, Expression::CONSTANT)
TRENCH_REGISTER_CLASS_KIND(Expression, Register, Expression::REGISTER)
TRENCH_REGISTER_CLASS_KIND(Expression, UnaryOperator, Expression::UNARY)
TRENCH_REGISTER_CLASS_KIND(Expression, BinaryOperator, Expression::BINARY)
TRENCH_REGISTER_CLASS_KIND(Expression, CanAccessMemory, Expression::CAN_ACCESS_MEMORY)
