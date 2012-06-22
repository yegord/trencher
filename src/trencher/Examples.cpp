#include "Examples.h"

#include <boost/format.hpp>

#include <trench/Program.h>

void dekker(trench::Program &program, int n, bool fenced) {
	using namespace trench;

	program.setMemorySize(n);

	for (int i = 0; i < n; ++i) {
		Thread *thread = program.makeThread((boost::format("dekker%d") % i).str());

		auto q0 = thread->makeState("q0");
		auto q1 = thread->makeState("q1");
		auto q2 = thread->makeState("q2");
		auto q3 = thread->makeState("q2");
		auto q4 = thread->makeState("q2");

		auto flag = program.makeRegister("flag");

		thread->setInitialState(q0);
		thread->makeTransition(q0, q1, std::make_shared<Write>(program.makeConstant(1), program.makeConstant((i + 1) % n)));
		if (fenced) {
			thread->makeTransition(q1, q2, std::make_shared<Mfence>());
		} else {
			thread->makeTransition(q1, q2, std::make_shared<Noop>());
		}
		thread->makeTransition(q2, q3, std::make_shared<Read>(flag, program.makeConstant(i)));
		thread->makeTransition(q3, q4, std::make_shared<Condition>(flag));
	}
}
