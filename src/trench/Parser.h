#pragma once

#include <istream>

namespace trench {

class Program;

class Parser {
	public:

	virtual ~Parser() {}

	virtual void parse(std::istream &in, Program &program) const = 0;
};

} // namespace trench
