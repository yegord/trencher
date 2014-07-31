/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include "SCReachability.h"

#include "Reachability.h"
#include "SCSemantics.h"

namespace trench {

bool isInterestingStateSCReachable(const Program &program) {
	return isFinalStateReachable(SCSemantics(program));
}

} // namespace trench
