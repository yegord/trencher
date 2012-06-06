#pragma once

#include "config.h"

#include "Expression.h"
#include "Kinds.h"

namespace trench {

typedef int Mnemonic;

class Read;
class Write;
class Mfence;
class Local;
class Condition;

class Instruction {
	public:

	enum Mnemonic {
		READ,
		WRITE,
		MFENCE,
		LOCAL,
		CONDITION
	};

	private:

	Mnemonic mnemonic_;

	public:
	
	Instruction(Mnemonic mnemonic):
		mnemonic_(mnemonic)
	{}

	virtual ~Instruction {}

	KIND(Read,      READ)
	KIND(Write,     WRITE)
	KIND(Mfence,    MFENCE)
	KIND(Local,     LOCAL)
	KIND(Condition, CONDITION)
};

class Read: public Instruction {
	std::shared_ptr<Register> reg_;
	std::shared_ptr<Expression> address_;

	public:

	Read(const std::shared_ptr<Register> &reg, const std::shared_ptr<Expression> &address):
		Instruction(READ), reg_(reg), address_(address)
	{}

	const std::shared_ptr<Register> &reg() const { return reg_.get(); }
	const std::shared_ptr<Expression> &address() const { return address_.get(); }
};

class Write: public Instruction {
	std::shared_ptr<Expression> value_;
	std::shared_ptr<Expression> address_;

	public:

	Write(const std::shared_ptr<Expression> &value, const std::shared_ptr<Expression> &address):
		Instruction(WRITE), value_(value), address_(address)
	{}

	const std::shared_ptr<Expression> &value() const { return value_.get(); }
	const std::shared_ptr<Expression> &address() const { return address_.get(); }
};

class Mfence: public Instruction {
	public:

	Mfence(): Instruction(MFENCE) {}
};

class Local: public Instruction {
	public:

	std::shared_ptr<Register> reg_;
	std::shared_ptr<Expression> value_;

	public:

	Local(const std::shared_ptr<Register> &reg, const std::shared_ptr<Expression> &value):
		Instruction(LOCAL), reg_(reg), value_(value)
	{}

	const std::shared_ptr<Register> &reg() const { return reg_; }
	const std::shared_ptr<Expression> &value() const { return value_; }
};

class Condition: public Instruction {
	std::shared_ptr<Expression> expression_;

	public:

	Condition(const std::shared_ptr<Expression> &expression):
		Instruction(CONDITION), expression_(expression)
	{}

	const std::shared_ptr<Expression> &expression() const { return expression_; }
};

} // namespace trench
