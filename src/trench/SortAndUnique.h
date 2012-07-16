/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <algorithm>

namespace trench {

template<class T>
void sort_and_unique(T &container) {
	std::sort(container.begin(), container.end());
	container.erase(std::unique(container.begin(), container.end()), container.end());
}

} // namespace trench
