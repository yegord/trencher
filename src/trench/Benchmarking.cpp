/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include "Benchmarking.h"

namespace trench {

std::ostream &operator<<(std::ostream &out, const Statistics &statistics) {
	return out
		<< " threads " << statistics.threadsCount()
		<< " states " << statistics.statesCount()
		<< " transitions " << statistics.transitionsCount()
		<< " potentialAttacksCount " << statistics.potentialAttacksCount()
//		<< " infeasibleAttacksCount1 " << statistics.infeasibleAttacksCount1()
//		<< " infeasibleAttacksCount2 " << statistics.infeasibleAttacksCount2()
		<< " feasibleAttacks " << statistics.feasibleAttacksCount()
//		<< " fencesCount " << statistics.fencesCount()
		<< " spinTime " << statistics.spinTime()
		<< " compilerTime " << statistics.compilerTime()
		<< " verifierTime " << statistics.verifierTime()
		<< " trailTime " << statistics.trailTime()
		<< " cpuTime " << statistics.cpuTime()
		<< " realTime " << statistics.realTime()
	;
}

} // namespace trench
