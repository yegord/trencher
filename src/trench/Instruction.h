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
	TRENCH_CLASS_WITH_KINDS(Instruction, mnemonic)

	public:

	enum Mnemonic {
		READ,
		WRITE,
		MFENCE,
		LOCAL,
		CONDITION
	};

	public:
	
	Instruction(Mnemonic mnemonic):
		mnemonic_(mnemonic)
	{}

	virtual ~Instruction() {}
};

class Read: public Instruction {
	std::shared_ptr<Register> reg_;
	std::shared_ptr<Expression> address_;
	int region_;

	public:

	Read(const std::shared_ptr<Register> &reg, const std::shared_ptr<Expression> &address, int region = 0):
		Instruction(READ), reg_(reg), address_(address), region_(region)
	{}

	const std::shared_ptr<Register> &reg() const { return reg_; }
	const std::shared_ptr<Expression> &address() const { return address_; }
	int region() const { return region_; }
};

class Write: public Instruction {
	std::shared_ptr<Expression> value_;
	std::shared_ptr<Expression> address_;
	int region_;

	public:

	Write(const std::shared_ptr<Expression> &value, const std::shared_ptr<Expression> &address, int region = 0):
		Instruction(WRITE), value_(value), address_(address), region_(region)
	{}

	const std::shared_ptr<Expression> &value() const { return value_; }
	const std::shared_ptr<Expression> &address() const { return address_; }
	int region() const { return region_; }
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

TRENCH_REGISTER_CLASS_KIND(Instruction, Read, Instruction::READ)
TRENCH_REGISTER_CLASS_KIND(Instruction, Write, Instruction::WRITE)
TRENCH_REGISTER_CLASS_KIND(Instruction, Mfence, Instruction::MFENCE)
TRENCH_REGISTER_CLASS_KIND(Instruction, Local, Instruction::LOCAL)
TRENCH_REGISTER_CLASS_KIND(Instruction, Condition, Instruction::CONDITION)
