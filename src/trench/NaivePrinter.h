#pragma once

#include "Printer.h"

namespace trench {

class NaivePrinter: public Printer {
	public:

	virtual void print(std::ostream &out, const Program &program) const override;
};

} // namespace trench
