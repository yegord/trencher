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

#include <memory>

#include <boost/unordered_map.hpp>

#include "Expression.h"

namespace trench {

class ExpressionsCache {
	boost::unordered_map<std::string, std::shared_ptr<Register>> name2register_;
	boost::unordered_map<Domain, std::shared_ptr<Constant>> value2constant_;

public:
	const std::shared_ptr<Register> &makeRegister(const std::string &name);
	const std::shared_ptr<Constant> &makeConstant(Domain value);
};

} // namespace trench
