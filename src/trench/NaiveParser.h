#pragma once

#include "Parser.h"

namespace trench {

class NaiveParser: public Parser {
	public:

	virtual void parse(std::istream &in, Program &program) const override;
};

} // namespace trench
