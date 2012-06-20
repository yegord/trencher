#include "RobustnessChecking.h"

#include "Foreach.h"
#include "Program.h"
#include "Reduction.h"
#include "SpinModelChecker.h"

namespace trench {

bool checkIsRobust(Program &program) {
	trench::Program augmentedProgram;
	trench::reduce(program, augmentedProgram);

	trench::SpinModelChecker checker;
	return !checker.check(augmentedProgram);
}

bool checkIsRobustParallel(Program &program) {
	foreach (Thread *attacker, program.threads()) {
		trench::Program augmentedProgram;
		trench::reduce(program, augmentedProgram, attacker);

		trench::SpinModelChecker checker;
		if (checker.check(augmentedProgram)) {
			return false;
		}
	}
	return true;
}

} // namespace trench
