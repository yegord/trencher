#pragma once

#include <ostream>

namespace trench {

class Program;

class Printer {
	public:

	virtual ~Printer() {}

	virtual void print(std::ostream &out, const Program &program) const = 0;
};

} // namespace trench
