#pragma once

#include <trench/config.h>

#include "Printer.h"

namespace trench {

class DotPrinter: public Printer {
	public:

	virtual void print(std::ostream &out, const Program &program) const override;
};

} // namespace trench
