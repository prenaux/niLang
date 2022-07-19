#pragma once
#ifndef __STL_OPTIONAL_H_952B67BE_0BC9_4048_9CFE_09A45072E613__
#define __STL_OPTIONAL_H_952B67BE_0BC9_4048_9CFE_09A45072E613__

#include "expected.h"
#include "utils.h"

namespace astl {

struct nullopt_tag_t {};

struct nullopt_t {
  EA_CONSTEXPR nullopt_t(nullopt_tag_t) {}
};

EA_CONSTEXPR nullopt_t nullopt{nullopt_tag_t{}};

template <typename T>
using optional = astl::expected<T,nullopt_t>;

template <class T>
inline EA_CONSTEXPR optional<eastl::decay_t<T>> make_optional(T&& value)
{
  return optional<eastl::decay_t<T>>(eastl::forward<T>(value));
}

template <class T, class... Args>
inline EA_CONSTEXPR optional<T> make_optional(Args&&... args)
{
  return optional<T>(astl::in_place, eastl::forward<Args>(args)...);
}

template <class T, class U, class... Args>
inline EA_CONSTEXPR optional<T> make_optional(std::initializer_list<U> il, Args&&... args)
{
  return optional<T>(eastl::in_place, il, eastl::forward<Args>(args)...);
}

#define ASTL_NULLOPT astl::make_unexpected(astl::nullopt)

} // namespace astl
#endif // __STL_OPTIONAL_H_952B67BE_0BC9_4048_9CFE_09A45072E613__
