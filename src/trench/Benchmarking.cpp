#include "Benchmarking.h"

namespace trench {

std::ostream &operator<<(std::ostream &out, const Statistics &statistics) {
	return out
		<< " threadsCount " << statistics.threadsCount()
		<< " statesCount " << statistics.statesCount()
		<< " transitionsCount " << statistics.transitionsCount()
		<< " potentialAttacksCount " << statistics.potentialAttacksCount()
		<< " infeasibleAttacksCount1 " << statistics.infeasibleAttacksCount1()
		<< " infeasibleAttacksCount2 " << statistics.infeasibleAttacksCount2()
		<< " feasibleAttacksCount " << statistics.feasibleAttacksCount()
		<< " fencesCount " << statistics.fencesCount()
		<< " spinTime " << statistics.spinTime()
		<< " compilerTime " << statistics.compilerTime()
		<< " verifierTime " << statistics.verifierTime()
		<< " realTime " << statistics.realTime()
	;
}

} // namespace trench
