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

namespace trench {

class Configuration {
	bool partialOrderReduction_;
	bool livenessOptimization_;

public:
	Configuration(): partialOrderReduction_(true), livenessOptimization_(true) {}

	static Configuration &instance() {
		static Configuration configuration;
		return configuration;
	}

	bool partialOrderReduction() const { return partialOrderReduction_; }
	void setPartialOrderReduction(bool value) { partialOrderReduction_ = value; }

	bool livenessOptimization() const { return livenessOptimization_; }
	void setLivenessOptimization(bool value) { livenessOptimization_ =  value; }
};

} // namespace trench
