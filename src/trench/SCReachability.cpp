#include "SCReachability.h"

#include "Reachability.h"
#include "SCSemantics.h"

namespace trench {

bool isInterestingStateSCReachable(const Program &program) {
	return isFinalStateReachable(SCSemantics(program));
}

} // namespace trench
