#pragma once

#include "config.h"

#include <vector>

#include "Expression.h"
#include "Kinds.h"

namespace trench {

typedef int Mnemonic;

class Read;
class Write;
class Mfence;
class Local;
class Condition;

typedef int Space;
const Space INVALID_SPACE = -1;

class Instruction {
	TRENCH_CLASS_WITH_KINDS(Instruction, mnemonic)

	public:

	enum Mnemonic {
		READ,
		WRITE,
		CAS,
		MFENCE,
		LOCAL,
		CONDITION,
		ATOMIC,
		NOOP
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
	Space space_;

	public:

	Read(const std::shared_ptr<Register> &reg, const std::shared_ptr<Expression> &address, Space space = Space()):
		Instruction(READ), reg_(reg), address_(address), space_(space)
	{}

	const std::shared_ptr<Register> &reg() const { return reg_; }
	const std::shared_ptr<Expression> &address() const { return address_; }
	Space space() const { return space_; }
};

class Write: public Instruction {
	std::shared_ptr<Expression> value_;
	std::shared_ptr<Expression> address_;
	Space space_;

	public:

	Write(const std::shared_ptr<Expression> &value, const std::shared_ptr<Expression> &address, Space space = Space()):
		Instruction(WRITE), value_(value), address_(address), space_(space)
	{}

	const std::shared_ptr<Expression> &value() const { return value_; }
	const std::shared_ptr<Expression> &address() const { return address_; }
	Space space() const { return space_; }
};

class CompareAndSwap: public Instruction {
	std::shared_ptr<Expression> address_;
	std::shared_ptr<Expression> oldValue_;
	std::shared_ptr<Expression> newValue_;
	std::shared_ptr<Register> success_;
	Space space_;

	public:

	CompareAndSwap(const std::shared_ptr<Expression> &address, const std::shared_ptr<Expression> &oldValue,
	               const std::shared_ptr<Expression> &newValue, const std::shared_ptr<Register> &success,
		       Space space = Space()):
		Instruction(CAS), address_(address), oldValue_(oldValue), newValue_(newValue), success_(success), space_(space)
	{}

	const std::shared_ptr<Expression> &address() const { return address_; }
	const std::shared_ptr<Expression> &oldValue() const { return oldValue_; }
	const std::shared_ptr<Expression> &newValue() const { return newValue_; }
	const std::shared_ptr<Register> &success() const { return success_; }
	Space space() const { return space_; }
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

class Atomic: public Instruction {
	std::vector<std::shared_ptr<Instruction>> instructions_;

	public:

	Atomic(const std::shared_ptr<Instruction> &a): Instruction(ATOMIC) {
		instructions_.push_back(a);
	}

	Atomic(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b): Instruction(ATOMIC) {
		instructions_.reserve(2);
		instructions_.push_back(a);
		instructions_.push_back(b);
	}

	Atomic(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b,
	       const std::shared_ptr<Instruction> &c): Instruction(ATOMIC) {
		instructions_.reserve(3);
		instructions_.push_back(a);
		instructions_.push_back(b);
		instructions_.push_back(c);
	}

	Atomic(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b,
	       const std::shared_ptr<Instruction> &c, const std::shared_ptr<Instruction> &d): Instruction(ATOMIC) {
		instructions_.reserve(4);
		instructions_.push_back(a);
		instructions_.push_back(b);
		instructions_.push_back(c);
		instructions_.push_back(d);
	}

	Atomic(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b,
	       const std::shared_ptr<Instruction> &c, const std::shared_ptr<Instruction> &d,
	       const std::shared_ptr<Instruction> &e): Instruction(ATOMIC) {
		instructions_.reserve(5);
		instructions_.push_back(a);
		instructions_.push_back(b);
		instructions_.push_back(c);
		instructions_.push_back(d);
		instructions_.push_back(e);
	}

	Atomic(const std::shared_ptr<Instruction> &a, const std::shared_ptr<Instruction> &b,
	       const std::shared_ptr<Instruction> &c, const std::shared_ptr<Instruction> &d,
	       const std::shared_ptr<Instruction> &e, const std::shared_ptr<Instruction> &f): Instruction(ATOMIC) {
		instructions_.reserve(6);
		instructions_.push_back(a);
		instructions_.push_back(b);
		instructions_.push_back(c);
		instructions_.push_back(d);
		instructions_.push_back(e);
		instructions_.push_back(f);
	}

	const std::vector<std::shared_ptr<Instruction>> instructions() const { return instructions_; }
};

class Noop: public Instruction {
	public:

	Noop(): Instruction(NOOP) {}
};

} // namespace trench

TRENCH_REGISTER_CLASS_KIND(Instruction, Read, Instruction::READ)
TRENCH_REGISTER_CLASS_KIND(Instruction, Write, Instruction::WRITE)
TRENCH_REGISTER_CLASS_KIND(Instruction, CompareAndSwap, Instruction::CAS)
TRENCH_REGISTER_CLASS_KIND(Instruction, Mfence, Instruction::MFENCE)
TRENCH_REGISTER_CLASS_KIND(Instruction, Local, Instruction::LOCAL)
TRENCH_REGISTER_CLASS_KIND(Instruction, Condition, Instruction::CONDITION)
TRENCH_REGISTER_CLASS_KIND(Instruction, Atomic, Instruction::ATOMIC)
TRENCH_REGISTER_CLASS_KIND(Instruction, Noop, Instruction::NOOP)
