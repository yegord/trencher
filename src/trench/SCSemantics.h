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

#include "Liveness.h"
#include "Program.h"
#include "SmallMap.h"
#include "State.h"

namespace trench {

class SCState {
	SmallMap<const Thread *, const State *> controlStates_;
	SmallMap<std::pair<Space, Address>, Domain> memoryValuation_;
	SmallMap<std::pair<const Thread *, const Register *>, Domain> registerValuation_;
	const Thread *memoryLockOwner_;
	const Thread *favourite_;
public:
	SCState(): memoryLockOwner_(NULL), favourite_(NULL) {}

	const SmallMap<const Thread *, const State *> &controlStates() const { return controlStates_; }
	void setControlState(const Thread *thread, const State *state) { controlStates_.set(thread, state); }

	const SmallMap<std::pair<Space, Address>, Domain> &memoryValuation() const { return memoryValuation_; }
	void setMemoryValue(Space space, Domain address, Domain value) { memoryValuation_.set(std::make_pair(space, address), value); }
	Domain getMemoryValue(Space space, Domain address) const { return memoryValuation_.get(std::make_pair(space, address)); }

	SmallMap<std::pair<const Thread *, const Register *>, Domain> &registerValuation() { return registerValuation_; }
	const SmallMap<std::pair<const Thread *, const Register *>, Domain> &registerValuation() const { return registerValuation_; }
	void setRegisterValue(const Thread *thread, const Register *reg, Domain value) { registerValuation_.set(std::make_pair(thread, reg), value); }
	Domain getRegisterValue(const Thread *thread, const Register *reg) const { return registerValuation_.get(std::make_pair(thread, reg)); }

	const Thread *memoryLockOwner() const { return memoryLockOwner_; }
	void setMemoryLockOwner(const Thread *thread) { memoryLockOwner_ = thread; }

	const Thread *favourite() const { return favourite_; }
	void setFavourite(const Thread *thread) { favourite_ = thread; }

	std::size_t hash() const {
		return controlStates_.hash() ^
		       memoryValuation_.hash() ^
		       registerValuation_.hash() ^
		       reinterpret_cast<uintptr_t>(memoryLockOwner_) ^
		       (reinterpret_cast<uintptr_t>(favourite_) << 8); }
};

inline bool operator==(const SCState &a, const SCState &b) {
	return a.controlStates() == b.controlStates() &&
	       a.memoryValuation() == b.memoryValuation() &&
	       a.registerValuation() == b.registerValuation() &&
	       a.memoryLockOwner() == b.memoryLockOwner() &&
	       a.favourite() == b.favourite();
}

std::ostream &operator<<(std::ostream &out, const SCState &state);

inline std::size_t hash_value(const trench::SCState &state) {
	return state.hash();
}

class SCTransition {
	SCState source_;
	SCState destination_;
	const Instruction *instruction_;

public:
	SCTransition(SCState source, SCState destination, const Instruction *instruction):
		source_(std::move(source)), destination_(std::move(destination)), instruction_(std::move(instruction))
	{}

	const SCState &source() const { return source_; }
	const SCState &destination() const { return destination_; }
	const Instruction *instruction() const { return instruction_; }
};


class SCSemantics {
	const Program &program_;
	const Liveness liveness_;

public:
	typedef SCState State;
	typedef SCTransition Transition;
	typedef std::string Label;

	SCSemantics(const Program &program): program_(program), liveness_(computeLiveness(program)) {}

	State initialState() const;

	bool isFinal(const State &state) const {
		return state.getMemoryValue(program_.interestingSpace(), program_.interestingAddress()) != 0;
	}

	std::string getName(const State &state) const;

	std::vector<Transition> getTransitionsFrom(const State &state) const;

        const State &getSourceState(const Transition &transition) const { return transition.source(); }
        const State &getDestinationState(const Transition &transition) const { return transition.destination(); }
	Label getLabel(const Transition &transition) const;
};

} // namespace trench
