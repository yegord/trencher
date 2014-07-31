#pragma once

#include <unordered_set>

namespace trench {

template<class Automaton, class Visitor, class StateSet = std::unordered_set<typename Automaton::State>>
class Dfs {
	const Automaton &automaton_;
	StateSet visited_;
	Visitor visitor_;

public:
	Dfs(const Automaton &automaton, Visitor visitor): automaton_(automaton), visitor_(visitor) {}

	bool visit(const typename Automaton::State &state) {
		if (visited_.find(state) != visited_.end()) {
			return false;
		}
		visited_.insert(state);

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
	bool onTransition(const typename Automaton::Transition &transition) const { return false; }
};

template<class Automaton, class Visitor = EmptyDfsVisitor<Automaton>>
bool dfs(const Automaton &automaton, Visitor visitor = Visitor()) {
	return Dfs<Automaton, Visitor>(automaton, visitor).visit(automaton.initialState());
}

} // namespace trench
