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
#include <memory>

namespace trench {

class Program;
class NaiveParserImpl;

class NaiveParser {
	std::unique_ptr<NaiveParserImpl> impl_;
public:
	NaiveParser();
	~NaiveParser();
	void parse(std::istream &in, Program &program);
};

} // namespace trench
