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

#include <atomic>
#include <iosfwd>

namespace trench {

class Statistics {
	std::atomic<std::size_t> threadsCount_;
	std::atomic<std::size_t> statesCount_;
	std::atomic<std::size_t> transitionsCount_;

	std::atomic<std::size_t> potentialAttacksCount_;
	std::atomic<std::size_t> infeasibleAttacksCount1_;
	std::atomic<std::size_t> infeasibleAttacksCount2_;
	std::atomic<std::size_t> feasibleAttacksCount_;
	std::atomic<std::size_t> fencesCount_;

	long cpuTime_;
	long realTime_;

public:
	Statistics() {
		reset();
	}

	void reset() {
		threadsCount_     = 0;
		statesCount_      = 0;
		transitionsCount_ = 0;

		potentialAttacksCount_   = 0;
		infeasibleAttacksCount1_ = 0;
		infeasibleAttacksCount2_ = 0;
		feasibleAttacksCount_    = 0;
		fencesCount_             = 0;
	}

	static Statistics &instance() {
		static Statistics statistics;
		return statistics;
	}

	void incThreadsCount(std::size_t value = 1) { threadsCount_ += value; }
	std::size_t threadsCount() const { return threadsCount_; }

	void incStatesCount(std::size_t value = 1) { statesCount_ += value; }
	std::size_t statesCount() const { return statesCount_; }

	void incTransitionsCount(std::size_t value = 1) { transitionsCount_ += value; }
	std::size_t transitionsCount() const { return transitionsCount_; }

	void incPotentialAttacksCount(std::size_t value = 1) { potentialAttacksCount_ += value; }
	std::size_t potentialAttacksCount() const { return potentialAttacksCount_; }

	void incInfeasibleAttacksCount1(std::size_t value = 1) { infeasibleAttacksCount1_ += value; }
	std::size_t infeasibleAttacksCount1() const { return infeasibleAttacksCount1_; }

	void incInfeasibleAttacksCount2(std::size_t value = 1) { infeasibleAttacksCount2_ += value; }
	std::size_t infeasibleAttacksCount2() const { return infeasibleAttacksCount2_; }

	void incFeasibleAttacksCount(std::size_t value = 1) { feasibleAttacksCount_ += value; }
	std::size_t feasibleAttacksCount() const { return feasibleAttacksCount_; }

	void incFencesCount(std::size_t value = 1) { fencesCount_ += value; }
	std::size_t fencesCount() const { return fencesCount_; }

	void addCpuTime(long milliseconds) { cpuTime_ += milliseconds; }
	long cpuTime() const { return cpuTime_; }

	void addRealTime(long milliseconds) { realTime_ += milliseconds; }
	long realTime() const { return realTime_; }

};

std::ostream &operator<<(std::ostream &out, const Statistics &statistics);

} // namespace trench
