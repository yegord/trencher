#pragma once

#include <trench/config.h>

#include <memory>

namespace trench {

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args && ... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

} // namespace trench
