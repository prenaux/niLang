#pragma once
#ifndef __STL_OPTIONAL_H_952B67BE_0BC9_4048_9CFE_09A45072E613__
#define __STL_OPTIONAL_H_952B67BE_0BC9_4048_9CFE_09A45072E613__

#include "EASTL/optional.h"

namespace astl {

template <typename T>
using optional = eastl::optional<T>;

using nullopt_t = eastl::nullopt_t;

EA_CONSTEXPR nullopt_t nullopt = eastl::nullopt;

template <class T>
inline EA_CONSTEXPR optional<eastl::decay_t<T>> make_optional(T&& value)
{
  return optional<eastl::decay_t<T>>(eastl::forward<T>(value));
}

template <class T, class... Args>
inline EA_CONSTEXPR optional<T> make_optional(Args&&... args)
{
  return optional<T>(eastl::in_place, eastl::forward<Args>(args)...);
}

template <class T, class U, class... Args>
inline EA_CONSTEXPR optional<T> make_optional(std::initializer_list<U> il, Args&&... args)
{
  return eastl::optional<T>(eastl::in_place, il, eastl::forward<Args>(args)...);
}

} // namespace astl
#endif // __STL_OPTIONAL_H_952B67BE_0BC9_4048_9CFE_09A45072E613__
