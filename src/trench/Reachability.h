/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "Dfs.h"

namespace trench {

template<class Automaton>
class ReachabilityVisitor: public EmptyDfsVisitor<Automaton> {
	const Automaton &automaton_;

public:
	explicit
	ReachabilityVisitor(const Automaton &automaton): automaton_(automaton) {}

	bool onStateEnter(const typename Automaton::State &state) const {
		return automaton_.isFinal(state);
	}
};

template<class Automaton>
bool isFinalStateReachable(const Automaton &automaton) {
	return dfs<Automaton, const ReachabilityVisitor<Automaton> &>(automaton, ReachabilityVisitor<Automaton>(automaton));
}

} // namespace trench
