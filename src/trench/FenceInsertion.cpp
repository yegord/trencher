/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include "FenceInsertion.h"

#include <boost/range/adaptor/map.hpp>
#include <boost/threadpool.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "Benchmarking.h"
#include "Foreach.h"
#include "Instruction.h"
#include "Program.h"
#include "RobustnessChecking.h"
#include "SortAndUnique.h"
#include "State.h"
#include "Thread.h"
#include "Transition.h"

namespace trench {

namespace {

class Attack {
	const Program &program_;
	Thread *attacker_;
	Transition *write_;
	Transition *read_;

	bool feasible_;
	std::vector<State *> intermediary_;

	public:

	Attack(const Program &program, Thread *attacker, Transition *write, Transition *read):
		program_(program), attacker_(attacker), write_(write), read_(read), feasible_(false)
	{
	}

	const Program &program() const { return program_; }
	Thread *attacker() const { return attacker_; }
	Transition *write() const { return write_; }
	Transition *read() const { return read_; }

	bool feasible() const { return feasible_; }
	void setFeasible(bool value) { feasible_ = value; }

	const std::vector<State *> &intermediary() const { return intermediary_; }

	template<class T>
	void setIntermediary(const T &container) {
		intermediary_.clear();
		intermediary_.insert(intermediary_.end(), container.begin(), container.end());
	}
};

class AttackChecker {
	Attack &attack_;
	bool searchForTdrOnly_;

	public:

	AttackChecker(Attack &attack, bool searchForTdrOnly):
		attack_(attack), searchForTdrOnly_(searchForTdrOnly)
	{}

	void operator()() {
		if (isAttackFeasible(attack_.program(), searchForTdrOnly_, attack_.attacker(), attack_.write(), attack_.read())) {
			attack_.setFeasible(true);

			boost::unordered_set<State *> visited;
			boost::unordered_set<State *> intermediary;

			dfs(attack_.write()->to(), attack_.read()->from(), visited, intermediary);

			attack_.setIntermediary(intermediary);
		}
	}

	private:

	static void dfs(State *state, State *target, boost::unordered_set<State *> &visited, boost::unordered_set<State *> &intermediary) {
		if (state == target) {
			intermediary.insert(state);
			return;
		}

		if (visited.find(state) != visited.end()) {
			return;
		}

		visited.insert(state);

		foreach (Transition *transition, state->out()) {
			switch (transition->instruction()->mnemonic()) {
				case Instruction::READ:
				case Instruction::WRITE:
				case Instruction::LOCAL:
				case Instruction::CONDITION:
				case Instruction::NOOP:
					dfs(transition->to(), target, visited, intermediary);

					if (intermediary.find(transition->to()) != intermediary.end()) {
						intermediary.insert(state);
					}
					break;
				case Instruction::MFENCE:
					break;
				default: {
					assert(!"NEVER REACHED");
				}
			}
		}
	}
};

class Attacker {
	std::vector<const Attack *> attacks_;
	std::vector<State *> fences_;

	public:

	const std::vector<const Attack *> &attacks() const { return attacks_; }
	void addAttack(const Attack *attack) { assert(attack->feasible()); attacks_.push_back(attack); }

	const std::vector<State *> &fences() const { return fences_; }

	template<class T>
	void setFences(const T &container) {
		fences_.clear();
		fences_.insert(fences_.end(), container.begin(), container.end());
	}
};

class AttackerNeutralizer {
	Attacker &attacker_;
	bool searchForTdrOnly_;

	public:

	AttackerNeutralizer(Attacker &attacker, bool searchForTdrOnly):
		attacker_(attacker), searchForTdrOnly_(searchForTdrOnly)
	{}

	void operator()() {
		/*
		 * It is always safe to insert fences after each attacker's write.
		 */
		std::vector<State *> fences;
		foreach (const Attack *attack, attacker_.attacks()) {
			fences.push_back(attack->write()->to());
		}
		sort_and_unique(fences);
		attacker_.setFences(fences);

		/*
		 * Can we improve the result?
		 */

		boost::unordered_map<State *, std::size_t> occurrences;

		/* For each state, compute the number of attacks it belongs to. */
		foreach (const Attack *attack, attacker_.attacks()) {
			foreach (State *state, attack->intermediary()) {
				++occurrences[state];
			}
		}

		/* Compute the set of potential fences. */
		std::vector<State *> potentialFences(fences);
		foreach (const auto &item, occurrences) {
			if (item.second >= 2) {
				potentialFences.push_back(item.first);
			}
		}
		sort_and_unique(potentialFences);

		/*
		 * Iterate all the subsets of potential fences of size up to fences.size() - 1.
		 *
		 * This can be optimized by considering independent attacks independently and
		 * even concurrently.
		 */

		boost::unordered_set<State *> usedFences;
		for (std::size_t nfences = 1; nfences < fences.size(); ++nfences) {
			if (tryFence(potentialFences, usedFences, nfences, 0)) {
				break;
			}
		}
	}

	private:

	bool tryFence(const std::vector<State *> &potentialFences, boost::unordered_set<State *> &usedFences, std::size_t nfences, std::size_t i) {
		usedFences.insert(potentialFences[i]);
		if (usedFences.size() == nfences) {
			bool success = true;
			foreach (const Attack *attack, attacker_.attacks()) {
				if (isAttackFeasible(attack->program(), searchForTdrOnly_,
				    attack->attacker(), attack->write(), attack->read(), usedFences)) {
					success = false;
					break;
				}
			}
			if (success) {
				attacker_.setFences(usedFences);
				return true;
			}
		} else {
			if (tryFence(potentialFences, usedFences, nfences, i + 1)) {
				return true;
			}
		}
		usedFences.erase(potentialFences[i]);

		if (nfences - usedFences.size() < potentialFences.size() - i) {
			if (tryFence(potentialFences, usedFences, nfences, i + 1)) {
				return true;
			}
		}

		return false;
	}
};

} // anonymous namespace

FenceSet computeFences(const Program &program, bool searchForTdrOnly) {
	std::vector<Attack> attacks;

	foreach (Thread *thread, program.threads()) {
		std::vector<Transition *> reads;
		std::vector<Transition *> writes;

		foreach (Transition *transition, thread->transitions()) {
			if (transition->instruction()->is<Read>()) {
				reads.push_back(transition);
			} else if (transition->instruction()->as<Write>()) {
				writes.push_back(transition);
			}
		}

		foreach (Transition *write, writes) {
			foreach (Transition *read, reads) {
				attacks.push_back(Attack(program, thread, write, read));
			}
		}
	}

	boost::threadpool::pool pool(boost::thread::hardware_concurrency());

	foreach (Attack &attack, attacks) {
		pool.schedule(AttackChecker(attack, searchForTdrOnly));
	}

	pool.wait();

	boost::unordered_map<Thread *, Attacker> thread2attacker;

	foreach (Attack &attack, attacks) {
		if (attack.feasible()) {
			thread2attacker[attack.attacker()].addAttack(&attack);
		}
	}

	foreach (Attacker &attacker, thread2attacker | boost::adaptors::map_values) {
		pool.schedule(AttackerNeutralizer(attacker, searchForTdrOnly));
	}

	pool.wait();

	FenceSet result;
	foreach (const auto &item, thread2attacker) {
		foreach (State *state, item.second.fences()) {
			result.push_back(Fence(item.first, state));
		}
	}

	Statistics::instance().incFencesCount(result.size());

	return result;
}

} // namespace trench
