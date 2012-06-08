#pragma once

#include <map>
#include <memory>
#include <vector>

#include "Expression.h"

namespace trench {

class Thread;
class Register;
class Constant;

class Program {
	std::map<std::string, std::unique_ptr<Thread>> name2thread_;
	std::vector<Thread *> threads_;

	std::map<std::string, std::shared_ptr<Register>> name2register_;
	std::vector<Register *> registers_;

	std::map<Domain, std::shared_ptr<Constant>> value2constant_;

	public:

	~Program();

	const std::vector<Thread *> &threads() const { return threads_; }
	Thread *makeThread(const std::string &name);

	const std::shared_ptr<Register> &makeRegister(const std::string &name);
	const std::vector<Register *> &registers() const { return registers_; }

	const std::shared_ptr<Constant> &makeConstant(Domain value);
};

} // namespace trench
