#pragma once

#include <trench/config.h>

#include <sstream>
#include <unordered_map>

#include <ia/utility.h>

#include "Program.h"
#include "State.h"
#include "printAsDot.h"

namespace trench {

class SCState {
	friend bool operator==(const SCState &, const SCState &);

	std::vector<std::pair<const Thread *, const State *>> controlStates_;
	std::unordered_map<std::pair<Space, Domain>, Domain> memoryValuation_;
	std::unordered_map<std::pair<const Thread *, const Register *>, Domain> registerValuation_;
	const Thread *memoryLock_;
	std::size_t hash_;
public:
	SCState(): memoryLock_(NULL), hash_(0) {}

	void setControlState(const Thread *thread, const State *state);
	const std::vector<std::pair<const Thread *, const State *>> controlStates() const { return controlStates_; }
	void setMemoryValue(Space space, Domain address, Domain value);
	Domain getMemoryValue(Space space, Domain address) const;
	const std::unordered_map<std::pair<Space, Domain>, Domain> &memoryValuation() const { return memoryValuation_; }
	void setRegisterValue(const Thread *thread, const Register *reg, Domain value);
	Domain getRegisterValue(const Thread *thread, const Register *reg) const;
	const std::unordered_map<std::pair<const Thread *, const Register *>, Domain> &registerValuation() const { return registerValuation_; }
	const Thread *memoryLock() const { return memoryLock_; }
	void setMemoryLock(const Thread *thread) { memoryLock_ = thread; }
	std::size_t hash() const { return hash_; }
};

inline bool operator==(const SCState &a, const SCState &b) {
	return a.hash() == b.hash() &&
	       a.controlStates() == b.controlStates() &&
	       a.memoryValuation_ == b.memoryValuation_ &&
	       a.registerValuation_ == b.registerValuation_;
}

std::ostream &operator<<(std::ostream &out, const SCState &state);

} // namespace trench

namespace std {

template<>
struct hash<trench::SCState> {
	typedef trench::SCState argument_type;
	typedef std::size_t result_type;

	std::size_t operator()(const trench::SCState &state) const { return state.hash(); }
};

} // namespace std

namespace trench {

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

public:
	typedef SCState State;
	typedef SCTransition Transition;
	typedef std::string Label;

	SCSemantics(const Program &program): program_(program) {}

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
