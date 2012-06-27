#include "FenceInsertion.h"

#include <boost/range/adaptor/map.hpp>
#include <boost/threadpool.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "Foreach.h"
#include "Instruction.h"
#include "Program.h"
#include "Reduction.h"
#include "RobustnessChecking.h"
#include "SpinModelChecker.h"
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
	std::vector<State *> intermediary_;
	bool feasible_;

	public:

	Attack(const Program &program, Thread *attacker, Transition *write, Transition *read):
		program_(program), attacker_(attacker), write_(write), read_(read), feasible_(false)
	{
	}

	const Program &program() const { return program_; }
	Thread *attacker() const { return attacker_; }
	Transition *write() const { return write_; }
	Transition *read() const { return read_; }

	const std::vector<State *> &intermediary() const { return intermediary_; }

	template<class T>
	void setIntermediary(const T &container) {
		intermediary_.clear();
		intermediary_.insert(intermediary_.end(), container.begin(), container.end());
	}

	bool feasible() const { return feasible_; }
	void setFeasible(bool value) { feasible_ = value; }
};

class AttackChecker {
	Attack &attack_;

	public:

	AttackChecker(Attack &attack): attack_(attack) {}

	void operator()() {
		{
			boost::unordered_set<State *> visited;
			boost::unordered_set<State *> intermediary;

			dfs(attack_.write()->to(), attack_.read()->from(), visited, intermediary);

			attack_.setIntermediary(intermediary);
		}

		if (!attack_.intermediary().empty()) {
			trench::Program augmentedProgram;
			trench::reduce(attack_.program(), augmentedProgram, attack_.attacker(), attack_.write(), attack_.read());

			trench::SpinModelChecker checker;
			attack_.setFeasible(checker.check(augmentedProgram));
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
				case Instruction::CAS:
				case Instruction::MFENCE:
					/* No fences or compare-and-swap on the way. */
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
	void setFences(const std::vector<State *> &fences) { fences_ = fences; }
};

class AttackerNeutralizer {
	Attacker &attacker_;

	public:

	AttackerNeutralizer(Attacker &attacker): attacker_(attacker) {}

	void operator()() {
#if 0
		// The task is, given certain attacks, find the minimal
		// number of fences killing these attacks.
		//
		// Is it possible that after killing all these attacks
		// there will be more? IMHO, no, because we've checked
		// (and killed) all attacks with a single attacker.
		//
		// If there would be more attacks, there would be more
		// attacks with a single attacked, but we've killed them
		// all.
		// 
		boost::unordered_map<State *, std::size_t> occurrence;

		foreach (const Attack *attack, attacker_.attacks()) {
			/* State immediately after write is preferred. */
			++occurrence[attack->write()->to()];

			foreach (State *state, attack->intermediary()) {
				++occurrence[state];
			}
		}

		std::vector<State *> potentialFences;
		foreach (const auto &pair, occurrence) {
			if (pair.second >= 2) {
				potentialFences.push_back(pair.first);
			}
		}

		// TODO: For each subset of potentialFences, check whether it restores robustness.
		// WTF???
#endif
		std::vector<State *> fences;
		foreach (const Attack *attack, attacker_.attacks()) {
			fences.push_back(attack->write()->to());
		}
		sort_and_unique(fences);
		attacker_.setFences(fences);
	}
};

} // anonymous namespace

FenceSet computeFences(const Program &program) {
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
		pool.schedule(AttackChecker(attack));
	}

	pool.wait();

	boost::unordered_map<Thread *, Attacker> thread2attacker;

	foreach (Attack &attack, attacks) {
		if (attack.feasible()) {
			thread2attacker[attack.attacker()].addAttack(&attack);
		}
	}

	foreach (Attacker &attacker, thread2attacker | boost::adaptors::map_values) {
		pool.schedule(AttackerNeutralizer(attacker));
	}

	pool.wait();

	FenceSet result;
	foreach (const auto &item, thread2attacker) {
		foreach (State *state, item.second.fences()) {
			result.push_back(Fence(item.first, state));
		}
	}
	return result;
}

} // namespace trench
