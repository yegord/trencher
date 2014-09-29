/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include "Benchmarking.h"

#include <ostream>

namespace trench {

std::ostream &operator<<(std::ostream &out, const Statistics &statistics) {
	return out
		<< " threads " << statistics.threadsCount()
		<< " states " << statistics.statesCount()
		<< " transitions " << statistics.transitionsCount()
		<< " potentialAttacks " << statistics.potentialAttacksCount()
//		<< " infeasibleAttacksCount1 " << statistics.infeasibleAttacksCount1()
//		<< " infeasibleAttacksCount2 " << statistics.infeasibleAttacksCount2()
		<< " feasibleAttacks " << statistics.feasibleAttacksCount()
//		<< " fencesCount " << statistics.fencesCount()
		<< " cpuTime " << statistics.cpuTime()
		<< " realTime " << statistics.realTime()
	;
}

} // namespace trench
