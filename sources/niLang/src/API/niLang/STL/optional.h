#pragma once
#ifndef __OPTIONAL_H_CD77D39C_9D47_C14D_924F_2EB3D16EFCCD__
#define __OPTIONAL_H_CD77D39C_9D47_C14D_924F_2EB3D16EFCCD__

#include "function_ref.h"
#include "EASTL/optional.h"

namespace astl {

template <typename T>
using optional = eastl::optional<T>;

template <typename T>
using optional_function_ref = eastl::optional<astl::function_ref<T>>;

using eastl::nullopt;
using nullopt_t = eastl::nullopt_t;

}  // namespace astl
#endif // __OPTIONAL_H_CD77D39C_9D47_C14D_924F_2EB3D16EFCCD__
