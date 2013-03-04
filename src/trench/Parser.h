/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <trench/config.h>

#include <istream>

namespace trench {

class Program;

class Parser {
	public:

	virtual ~Parser() {}

	virtual void parse(std::istream &in, Program &program) const = 0;
};

} // namespace trench
