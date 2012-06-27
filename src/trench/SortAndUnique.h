#pragma once

#include <algorithm>

namespace trench {

template<class T>
void sort_and_unique(T &container) {
	std::sort(container.begin(), container.end());
	container.erase(std::unique(container.begin(), container.end()), container.end());
}

} // namespace trench
