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

namespace trench {

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args && ... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

} // namespace trench
