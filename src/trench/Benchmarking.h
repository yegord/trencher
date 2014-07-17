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

#include <boost/thread.hpp>

#include <ostream>

namespace trench {

class Statistics {
	boost::mutex mutex_;

	int threadsCount_;
	int statesCount_;
	int transitionsCount_;

	int potentialAttacksCount_;
	int infeasibleAttacksCount1_;
	int infeasibleAttacksCount2_;
	int feasibleAttacksCount_;
	int fencesCount_;

	long spinTime_;
	long compilerTime_;
	long verifierTime_;
  long trailTime_;
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

		spinTime_     = 0;
		compilerTime_ = 0;
		verifierTime_ = 0;
    trailTime_    = 0;
		cpuTime_      = 0;
		realTime_     = 0;
	}

	static Statistics &instance() {
		static Statistics statistics;
		return statistics;
	}

	void incThreadsCount(int value = 1) { mutex_.lock(); threadsCount_ += value; mutex_.unlock(); }
	int threadsCount() const { return threadsCount_; }

	void incStatesCount(int value = 1) { mutex_.lock(); statesCount_ += value; mutex_.unlock(); }
	int statesCount() const { return statesCount_; }

	void incTransitionsCount(int value = 1) { mutex_.lock(); transitionsCount_ += value; mutex_.unlock(); }
	int transitionsCount() const { return transitionsCount_; }

	void incPotentialAttacksCount(int value = 1) { mutex_.lock(); potentialAttacksCount_ += value; mutex_.unlock(); }
	int potentialAttacksCount() const { return potentialAttacksCount_; }

	void incInfeasibleAttacksCount1(int value = 1) { mutex_.lock(); infeasibleAttacksCount1_ += value; mutex_.unlock(); }
	int infeasibleAttacksCount1() const { return infeasibleAttacksCount1_; }

	void incInfeasibleAttacksCount2(int value = 1) { mutex_.lock(); infeasibleAttacksCount2_ += value; mutex_.unlock(); }
	int infeasibleAttacksCount2() const { return infeasibleAttacksCount2_; }

	void incFeasibleAttacksCount(int value = 1) { mutex_.lock(); feasibleAttacksCount_ += value; mutex_.unlock(); }
	int feasibleAttacksCount() const { return feasibleAttacksCount_; }

	void incFencesCount(int value = 1) { mutex_.lock(); fencesCount_ += value; mutex_.unlock(); }
	int fencesCount() const { return fencesCount_; }

	void addSpinTime(long milliseconds) { mutex_.lock(); spinTime_ += milliseconds; mutex_.unlock(); }
	long spinTime() const { return spinTime_; } 

	void addCompilerTime(long milliseconds) { mutex_.lock(); compilerTime_ += milliseconds; mutex_.unlock(); }
	long compilerTime() const { return compilerTime_; }

	void addVerifierTime(double milliseconds) { mutex_.lock(); verifierTime_ += milliseconds; mutex_.unlock(); }
	long verifierTime() const { return verifierTime_; }

	void addTrailTime(double milliseconds) { mutex_.lock(); trailTime_ += milliseconds; mutex_.unlock(); }
	long trailTime() const { return trailTime_; }

	void addCpuTime(double milliseconds) { mutex_.lock(); cpuTime_ += milliseconds; mutex_.unlock(); }
	long cpuTime() const { return cpuTime_; }

	void addRealTime(double milliseconds) { mutex_.lock(); realTime_ += milliseconds; mutex_.unlock(); }
	long realTime() const { return realTime_; }
};

std::ostream &operator<<(std::ostream &out, const Statistics &statistics);

} // namespace trench
