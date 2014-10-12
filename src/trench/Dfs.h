/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <trench/config.h>

#include <boost/unordered_set.hpp>

#include "Benchmarking.h"

namespace trench {

template<class Automaton, class Visitor, class StateSet = boost::unordered_set<typename Automaton::State>>
class Dfs {
	const Automaton &automaton_;
	StateSet visited_;
	Visitor visitor_;

public:
	Dfs(const Automaton &automaton, Visitor visitor): automaton_(automaton), visitor_(visitor) {}

	~Dfs() {
		Statistics::instance().incVisitedStatesCount(visited_.size());
	}

	bool visit(const typename Automaton::State &state) {
		if (!visited_.insert(state).second) {
			return false;
		}
		if (visitor_.onStateEnter(state)) {
			return true;
		}
		for (const auto &transition : automaton_.getTransitionsFrom(state)) {
			if (visitor_.onTransition(transition)) {
				return true;
			}
			if (visit(automaton_.getDestinationState(transition))) {
				return true;
			}
		}
		if (visitor_.onStateExit(state)) {
			return true;
		}
		return false;
	}
};

template<class Automaton>
class EmptyDfsVisitor {
public:
	bool onStateEnter(const typename Automaton::State &) const { return false; }
	bool onStateExit(const typename Automaton::State &) const { return false; }
	bool onTransition(const typename Automaton::Transition &) const { return false; }
};

template<class Automaton, class Visitor = EmptyDfsVisitor<Automaton>>
bool dfs(const Automaton &automaton, Visitor visitor = Visitor()) {
	return Dfs<Automaton, Visitor>(automaton, visitor).visit(automaton.initialState());
}

} // namespace trench
