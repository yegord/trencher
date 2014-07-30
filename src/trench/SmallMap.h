#pragma once

#include <algorithm>
#include <utility>
#include <vector>

#include <ia/utility.h>

namespace trench {

template<class Key, class Value, class Vector = std::vector<std::pair<Key, Value>>>
class SmallMap {
	Vector vector_;
	std::size_t hash_;
public:
	typedef typename Vector::iterator iterator;
	typedef typename Vector::const_iterator const_iterator;
	typedef typename Vector::value_type value_type;

	iterator begin() { return vector_.begin(); }
	iterator end() { return vector_.end(); }

	const_iterator cbegin() const { return vector_.cbegin(); }
	const_iterator cend() const { return vector_.cend(); }

	const_iterator begin() const { return vector_.begin(); }
	const_iterator end() const { return vector_.end(); }

	SmallMap(): hash_(0) {}

	const_iterator lower_bound(const Key &key) const {
		return std::lower_bound(
			vector_.begin(),
			vector_.end(),
			key,
			[](const value_type &item, const Key &key){
				return item.first < key;
			}
		);
	}

	void set(const Key &key, Value value) {
		hash_ ^= ia::compute_hash(value);

		auto i = lower_bound(key);
		if (i == vector_.end() || i->first != key) {
			if (value != Value()) {
				vector_.insert(i, std::make_pair(key, std::move(value)));
			}
		} else {
			hash_ ^= ia::compute_hash(i->second);
			if (value != Value()) {
				const_cast<Value &>(i->second) = std::move(value);
			} else {
				vector_.erase(i);
			}
		}
	}

	Value get(const Key &key) const {
		auto i = lower_bound(key);
		if (i != vector_.end() && i->first == key) {
			return i->second;
		} else {
			return Value();
		}
	}

	std::size_t hash() const {
		return hash_;
	}

	const Vector &vector() const { return vector_; }
};

template<class Key, class Value, class Vector>
inline bool operator==(const SmallMap<Key, Value, Vector> &a, const SmallMap<Key, Value, Vector> &b) {
	return a.hash() == b.hash() && a.vector() == b.vector();
}

} // namespace trench

namespace std {

template<class Key, class Value, class Vector>
struct hash<trench::SmallMap<Key, Value, Vector>> {
	typedef trench::SmallMap<Key, Value, Vector> argument_type;
	typedef size_t result_type;

	result_type operator()(const argument_type &value) const {
		return value.hash();
	}
};

} // namespace std
