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
		MFENCE,
		LOCAL,
		CONDITION,
		ATOMIC,
		NOOP,
		LOCK,
		UNLOCK
	};

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
	Read(std::shared_ptr<Register> reg, std::shared_ptr<Expression> address, Space space = Space()):
		Instruction(READ), reg_(std::move(reg)), address_(std::move(address)), space_(space)
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
	Write(std::shared_ptr<Expression> value, std::shared_ptr<Expression> address, Space space = Space()):
		Instruction(WRITE), value_(std::move(value)), address_(std::move(address)), space_(space)
	{}

	const std::shared_ptr<Expression> &value() const { return value_; }
	const std::shared_ptr<Expression> &address() const { return address_; }
	Space space() const { return space_; }
};

class Mfence: public Instruction {
public:
	Mfence(): Instruction(MFENCE) {}
};

class Local: public Instruction {
	std::shared_ptr<Register> reg_;
	std::shared_ptr<Expression> value_;

public:
	Local(std::shared_ptr<Register> reg, std::shared_ptr<Expression> value):
		Instruction(LOCAL), reg_(std::move(reg)), value_(std::move(value))
	{}

	const std::shared_ptr<Register> &reg() const { return reg_; }
	const std::shared_ptr<Expression> &value() const { return value_; }
};

class Condition: public Instruction {
	std::shared_ptr<Expression> expression_;

public:
	Condition(std::shared_ptr<Expression> expression):
		Instruction(CONDITION), expression_(std::move(expression))
	{}

	const std::shared_ptr<Expression> &expression() const { return expression_; }
};

class Atomic: public Instruction {
	std::vector<std::shared_ptr<Instruction>> instructions_;

public:
	template<class... Ts>
	Atomic(Ts &&...instructions): Instruction(ATOMIC) {
		addInstructions(std::forward<Ts>(instructions)...);
	}

	const std::vector<std::shared_ptr<Instruction>> &instructions() const { return instructions_; }

private:
	template<class T, class... Ts>
	void addInstructions(T &&instruction, Ts &&...instructions) {
		instructions_.push_back(std::forward<T>(instruction));
		addInstructions(std::forward<Ts>(instructions)...);
	}

	void addInstructions() {}
};

class Noop: public Instruction {
public:
	Noop(): Instruction(NOOP) {}
};

class Lock: public Instruction {
public:
	Lock(): Instruction(LOCK) {}
};

class Unlock: public Instruction {
public:
	Unlock(): Instruction(UNLOCK) {}
};

} // namespace trench

TRENCH_REGISTER_CLASS_KIND(Instruction, Read, Instruction::READ)
TRENCH_REGISTER_CLASS_KIND(Instruction, Write, Instruction::WRITE)
TRENCH_REGISTER_CLASS_KIND(Instruction, Mfence, Instruction::MFENCE)
TRENCH_REGISTER_CLASS_KIND(Instruction, Local, Instruction::LOCAL)
TRENCH_REGISTER_CLASS_KIND(Instruction, Condition, Instruction::CONDITION)
TRENCH_REGISTER_CLASS_KIND(Instruction, Atomic, Instruction::ATOMIC)
TRENCH_REGISTER_CLASS_KIND(Instruction, Noop, Instruction::NOOP)
TRENCH_REGISTER_CLASS_KIND(Instruction, Lock, Instruction::LOCK)
TRENCH_REGISTER_CLASS_KIND(Instruction, Unlock, Instruction::UNLOCK)
