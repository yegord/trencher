#pragma once

namespace trench {

template<class T>
T ror(T x, unsigned int moves) {
  return (x >> moves) | (x << (sizeof(T)*8 - moves));
}

} // namespace trench
