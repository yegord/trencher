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

#include <boost/unordered_map.hpp>

namespace trench {

class Program;
class State;
class Register;

class Liveness {
	boost::unordered_map<const State *, std::vector<const Register *>> state2liveRegisters_;
public:
	const std::vector<const Register *> &getLiveRegisters(const State *state) const {
		auto i = state2liveRegisters_.find(state);
		if (i != state2liveRegisters_.end()) {
			return i->second;
		} else {
			static const std::vector<const Register *> result;
			return result;
		}
	}

	std::vector<const Register *> &getLiveRegisters(const State *state) {
		return state2liveRegisters_[state];
	}

	void setLiveRegisters(const State *state, std::vector<const Register *> liveRegisters) {
		state2liveRegisters_[state] = std::move(liveRegisters);
	}
};

Liveness computeLiveness(const Program &program);

} // namespace trench
