#pragma once

namespace trencher {

class Thread;

class State {
	std::string name_;
	std::vector<Transition *> in_;
	std::vector<Transition *> out_;

	public:

	State(const std::string &name): name_(name) {}

	const std::string &name() const { return name_; }

	const std::vector<const Transition *> &in() const { return in_; }
	const std::vector<const Transition *> &out() const { return out_; }

	friend class Thread;
};

} // namespace trencher
