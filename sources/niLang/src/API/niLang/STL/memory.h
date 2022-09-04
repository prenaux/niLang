#pragma once
#ifndef __STL_MEMORY_H_0E726DAE_AFEA_0147_943B_BEFCCB8E1077__
#define __STL_MEMORY_H_0E726DAE_AFEA_0147_943B_BEFCCB8E1077__

#include "EASTL/memory.h"
#include "EASTL/shared_ptr.h"
#include "EASTL/weak_ptr.h"
#include "EASTL/unique_ptr.h"
#include "non_null.h"

namespace astl {

template <typename T>
using shared_ptr = eastl::shared_ptr<T>;

template <typename T>
using shared_non_null = astl::non_null<eastl::shared_ptr<T> >;

template<typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args) {
  return shared_ptr<T>(new T(eastl::forward<Args>(args)...));
}

template<typename T, typename... Args>
shared_non_null<T> make_shared_non_null(Args&&... args) {
  return shared_non_null<T>(make_shared<T>(eastl::forward<Args>(args)...));
}

template <typename T>
using weak_ptr = eastl::weak_ptr<T>;

template <typename T>
using unique_ptr = eastl::unique_ptr<T>;

// eastl::make_unique is only available in C++14 & C++20, we need it in C++11 aswell
template<typename T, typename... Args>
astl::unique_ptr<T> make_unique(Args&&... args) {
  return astl::unique_ptr<T>(new T(eastl::forward<Args>(args)...));
}

}
#endif // __STL_MEMORY_H_0E726DAE_AFEA_0147_943B_BEFCCB8E1077__
