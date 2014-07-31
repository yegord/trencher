/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include "ExpressionsCache.h"

namespace trench {

const std::shared_ptr<Register> &ExpressionsCache::makeRegister(const std::string &name) {
	auto &result = name2register_[name];
	if (!result) {
		result = std::make_shared<Register>(name);
	}
	return result;
}

const std::shared_ptr<Constant> &ExpressionsCache::makeConstant(Domain value) {
	auto &result = value2constant_[value];
	if (!result) {
		result = std::make_shared<Constant>(value);
	}
	return result;
}

} // namespace trench
