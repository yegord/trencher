#include "FenceInsertion.h"

#include <atomic>

#include <boost/threadpool.hpp>
#include <boost/unordered_set.hpp>

#include "Foreach.h"
#include "Instruction.h"
#include "Program.h"
#include "Reduction.h"
#include "RobustnessChecking.h"
#include "SpinModelChecker.h"
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

	return FenceSet();
}

} // namespace trench
