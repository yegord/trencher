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
#include <vector>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "Instruction.h"
#include "Expression.h"
#include "Transition.h"
#include "Thread.h"
#include "State.h"

namespace trench {

class Program {
	boost::unordered_map<std::string, std::unique_ptr<Thread>> name2thread_;
	std::vector<Thread *> threads_;
	Domain interestingAddress_;
	Space interestingSpace_;
  bool tsoreach_; // is set to TRUE iff the final state is provably TSO reachable
  
public:
	Program():
		interestingAddress_(0),
		interestingSpace_(INVALID_SPACE)
	{}

	const std::vector<Thread *> &threads() const { return threads_; }
	Thread *getThread(const std::string &name);
	Thread *makeThread(const std::string &name);

	void setInterestingAddress(Domain address, Space space = Space()) {
		interestingAddress_ = address;
		interestingSpace_ = space;
	}

	Domain interestingAddress() const { return interestingAddress_; }
	Space interestingSpace() const { return interestingSpace_; }

  bool reachable() const { return tsoreach_; }
  void setReachable(bool value) { tsoreach_ = value; }
};

class Attack {
  const Program &program_;
  Thread *attacker_;
  Transition *write_;
  Transition *read_;

  bool feasible_;
  boost::unordered_set<State *> intermediary_;
  Program *instrumented_; // this is the only added field wrt. `master' Attack

  public:

  Attack(const Program& program, Thread *attacker, Transition *write, Transition *read):
    program_(program), attacker_(attacker), write_(write), read_(read), feasible_(false) { }

  const Program &program() const { return program_; }
  Thread *attacker() const { return attacker_; }
  Transition *write() const { return write_; }
  Transition *read() const { return read_; }
  
  bool feasible() const { return feasible_; }
  void setFeasible(bool value) { feasible_ = value; }

  const boost::unordered_set<State *> &intermediary() const { return intermediary_; }
  template<class T> void setIntermediary(const T &container) { intermediary_.clear(); intermediary_.insert(container.begin(), container.end()); }

  Program *instrumented() { return instrumented_; }
  void setInstrumented(Program* program) { instrumented_ = program; }
};

} // namespace trench
