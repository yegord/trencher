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

#include <map>
#include <memory>
#include <vector>

#include <boost/unordered_set.hpp>

#include "Instruction.h"
#include "Expression.h"
#include "Thread.h"
#include "Transition.h"
#include "State.h"
#include "Foreach.h"

namespace trench {

class Program {
	int memorySize_;
//  std::string globalLock_; // when not empty, it holds the name of the thread holding the lock
	Domain interestingAddress_;
	Space interestingSpace_;
  bool tsoreach_; // is set to true when the final state is provably TSO reachable

	std::vector<Thread *> threads_;

	std::map<std::string, std::unique_ptr<Thread>> name2thread_;
	std::map<std::string, std::shared_ptr<Register>> name2register_;
	std::map<Domain, std::shared_ptr<Constant>> value2constant_;

	public:

	Program(int memorySize = 10): // generous bound, just to be safe
		memorySize_(memorySize), interestingAddress_(0), interestingSpace_(INVALID_SPACE), tsoreach_(false) {}

	int memorySize() const { return memorySize_; }
	void setMemorySize(int size) { memorySize_ = size; }

//  std::string globalLock() const { return globalLock_; }
//  void setGlobalLock(std::string name) { globalLock_ = name; }
  
	void setInteresting(Domain address, Space space = Space()) {
		interestingAddress_ = address;
		interestingSpace_ = space;
	}
	Domain interestingAddress() const { return interestingAddress_;}
	Space interestingSpace() const { return interestingSpace_;}

	const std::shared_ptr<Register> &makeRegister(const std::string &name);
	const std::shared_ptr<Constant> &makeConstant(Domain value);

  bool reachable() const { return tsoreach_; }
  void setReachable(bool value) { tsoreach_ = value; }
 
	const std::vector<Thread *> &threads() const { return threads_; }
	Thread *getThread(const std::string &name);
	Thread *makeThread(const std::string &name);
};

class Attack {
  const Program &program_;
  Thread *attacker_;
  Transition *write_;
  Transition *read_;

  bool feasible_;
  boost::unordered_set<State *> intermediary_;
  Program *instrumented_;
  // ? boost::unordered_set<Program *> instrumented_;
  
  public:

  Attack(const Program &program, Thread *attacker, Transition *write, Transition *read):
    program_(program), attacker_(attacker), write_(write), read_(read), feasible_(false)
  {}

  const Program &program() const { return program_; }
  Thread *attacker() const { return attacker_; }
  Transition *write() const { return write_; }
  Transition *read() const { return read_; }

  bool feasible() const { return feasible_; }
  void setFeasible(bool value) { feasible_ = value; }

  const boost::unordered_set<State *> &intermediary() const { return intermediary_; }
  template<class T>
  void setIntermediary(const T &container) {
    intermediary_.clear();
    intermediary_.insert(container.begin(), container.end());
  }

  Program *instrumented() const { return instrumented_; }
  void setInstrumented(Program* program) { instrumented_ = program; }
};

} // namespace trench
