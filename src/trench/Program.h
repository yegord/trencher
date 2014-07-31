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

#include "Instruction.h"
#include "Expression.h"
#include "Thread.h"

namespace trench {

class Program {
	boost::unordered_map<std::string, std::unique_ptr<Thread>> name2thread_;
	std::vector<Thread *> threads_;

	Domain interestingAddress_;
	Space interestingSpace_;

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
};

} // namespace trench
