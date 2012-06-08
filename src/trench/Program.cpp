#include "Program.h"

#include "Foreach.h"
#include "Thread.h"

namespace trench {

Thread *Program::makeThread(const std::string &name) {
	auto &result = name2thread_[name];
	if (!result) {
		result.reset(new Thread(name));
		threads_.push_back(result.get());
	}
	return result.get();
}

const std::shared_ptr<Register> &Program::makeRegister(const std::string &name) {
	auto &result = name2register_[name];
	if (!result) {
		result.reset(new Register(name));
		registers_.push_back(result.get());
	}
	return result;
}

const std::shared_ptr<Constant> &Program::makeConstant(Domain value) {
	auto &result = value2constant_[value];
	if (!result) {
		result.reset(new Constant(value));
	}
	return result;
}

} // namespace trench
