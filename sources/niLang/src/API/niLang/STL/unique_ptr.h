#pragma once
#ifndef __UNIQUE_PTR_H_25CE6A74_85F5_8E4C_A8A7_0567E4B5CC9D__
#define __UNIQUE_PTR_H_25CE6A74_85F5_8E4C_A8A7_0567E4B5CC9D__

#include "EASTL/unique_ptr.h"

namespace astl {

template <typename T>
using unique_ptr = eastl::unique_ptr<T>;

// eastl::make_unique is only available in C++14 & C++20, we need it in C++11 aswell
template<typename T, typename... Args>
astl::unique_ptr<T> make_unique(Args&&... args) {
  return astl::unique_ptr<T>(new T(eastl::forward<Args>(args)...));
}

} // namespace astl
#endif // __UNIQUE_PTR_H_25CE6A74_85F5_8E4C_A8A7_0567E4B5CC9D__
