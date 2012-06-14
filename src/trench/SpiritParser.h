#pragma once

#include <istream>

namespace trench {

class Program;

class SpiritParser {
	public:

	virtual void parse(std::istream &in, Program &program);
};

} // namespace trench
