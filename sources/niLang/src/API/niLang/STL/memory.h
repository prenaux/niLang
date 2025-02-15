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

using eastl::make_shared;

template<typename T, typename... Args>
shared_non_null<T> make_shared_non_null(Args&&... args) {
  return as_non_null(make_shared<T>(eastl::forward<Args>(args)...));
}

template <typename T>
using weak_ptr = eastl::weak_ptr<T>;

template <typename T>
using unique_ptr = eastl::unique_ptr<T>;

using eastl::make_unique;

template <typename T>
using unique_non_null = astl::non_null<eastl::unique_ptr<T> >;

template<typename T, typename... Args>
unique_non_null<T> make_unique_non_null(Args&&... args) {
  return as_non_null(make_unique<T>(eastl::forward<Args>(args)...));
}

template<typename T>
astl::non_null<T*> as_non_null(const astl::shared_non_null<T>& v) {
  return as_non_null(v.raw_ptr().get());
}

template<typename T>
astl::non_null<T*> as_non_null(const astl::unique_non_null<T>& v) {
  return as_non_null(v.raw_ptr().get());
}

/**
 * Requires that a variable (usually a struct/class member) be explicitly
 * initialized.
 *
 * Example:
 * ```
 *   struct sFoo {
 *     astl::required<ni::tI32> _bar;
 *   };
 *
 *   sFoo f; // does not compile
 *   sFoo f = {}; // does not compile
 *   sFoo f = { 0 }; // success!
 * ```
 */
template <typename T>
struct required {
  T _value;

#ifdef EA_COMPILER_CPP20_ENABLED
  explicit(false)
#endif
  constexpr required(const T& aValue)
      : _value(eastl::move(aValue)) {}

  operator T& () {
    return _value;
  }

  operator const T& () const {
    return _value;
  }

  const T& operator * () const {
    return _value;
  }
};

}
#endif // __STL_MEMORY_H_0E726DAE_AFEA_0147_943B_BEFCCB8E1077__
