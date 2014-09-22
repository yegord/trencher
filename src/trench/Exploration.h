/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <calin@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <boost/unordered_set.hpp>

namespace trench {

template<class Automaton> class DefaultVisitor {
public:
	bool onStateEnter(const typename Automaton::State &) const { return false; }
	void onTransition(const typename Automaton::Transition &) const { }
};

// Dfs class and visiting method
template<class Automaton, class Visitor, class StateSet = boost::unordered_set<typename Automaton::State>> class Dfs {
	const Automaton &automaton_;
	StateSet visited_;
	Visitor visitor_;

public:
	Dfs(const Automaton &automaton, Visitor visitor): automaton_(automaton), visitor_(visitor) {}

	bool visit(const typename Automaton::State &state) {
    if (visited_.insert(state).second) {
		  if (visitor_.onStateEnter(state)) return true;
		  for (const auto &transition : automaton_.getTransitionsFrom(state)) {
			  visitor_.onTransition(transition);
			  if (visit(automaton_.getDestinationState(transition))) return true;
		  }
    }
		return false;
	}
};
template<class Automaton, class Visitor = DefaultVisitor<Automaton>> bool dfs(const Automaton &automaton, Visitor visitor = Visitor()) {
	return Dfs<Automaton, Visitor>(automaton, visitor).visit(automaton.initialState());
}

// Explore class and visiting method: usefull for finding reachability witnesses
template<class Automaton, class Visitor, class StateSet = boost::unordered_set<typename Automaton::State>> class Explore {
	const Automaton &automaton_;
	StateSet visited_;
	Visitor visitor_;

public:
	Explore(const Automaton &automaton, Visitor visitor): automaton_(automaton), visitor_(visitor) {}

	std::pair<bool, std::vector<typename Automaton::Transition>> visit(const typename Automaton::State &state) {
    std::vector<typename Automaton::Transition> empty;
    if (visited_.insert(state).second) {
		  if (visitor_.onStateEnter(state)) return std::make_pair(true, empty);
		  for (const auto &transition : automaton_.getTransitionsFrom(state)) {
			  visitor_.onTransition(transition);
        auto result = visit(automaton_.getDestinationState(transition));
        if (result.first) {
          result.second.push_back(transition);
          return std::make_pair(true, result.second);
		    }
      }
    }
		return std::make_pair(false,empty);
	}
};
template<class Automaton, class Visitor = DefaultVisitor<Automaton>> 
std::pair<bool, std::vector<typename Automaton::Transition>> explore(const Automaton &automaton, Visitor visitor = Visitor()) {
  return Explore<Automaton, Visitor>(automaton, visitor).visit(automaton.initialState());
}

} // namespace trench
