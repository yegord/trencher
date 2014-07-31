#pragma once

#include <functional>

namespace ia {

template<class T>
std::size_t compute_hash(const T &obj) {
	return std::hash<T>()(obj);
}

} // namespace ia

namespace std {

template<class First, class Second>
struct hash<pair<First, Second>> {
	typedef pair<First, Second> argument_type;
	typedef size_t result_type;

	result_type operator()(const argument_type &value) const {
		return ia::compute_hash(value.first) ^ ia::compute_hash(value.second);
	}
};

} // namespace std
