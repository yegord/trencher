/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <string>
#include <vector>

namespace trench {

class Thread;
class Transition;

class State {
	std::string name_;
	std::vector<Transition *> in_;
	std::vector<Transition *> out_;

	public:

	State(const std::string &name): name_(name) {}

	const std::string &name() const { return name_; }

	const std::vector<Transition *> &in() const { return in_; }
	const std::vector<Transition *> &out() const { return out_; }

	friend class Thread;
};

} // namespace trencher
