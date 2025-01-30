#pragma once
#ifndef __STL_TYPE_TRAITS_H_7A014C0E_F03E_0345_80CD_27E2F05EAAFB__
#define __STL_TYPE_TRAITS_H_7A014C0E_F03E_0345_80CD_27E2F05EAAFB__

#include "EASTL/type_traits.h"

namespace astl {

using eastl::is_same;
using eastl::is_same_v;
using eastl::is_base_of_v;
using eastl::remove_pointer_t;
using eastl::remove_cv_t;
using eastl::remove_cvref_t;
using eastl::remove_reference_t;
using eastl::tuple_element_t;
using eastl::get;
using eastl::is_void_v;

template<typename T, typename U>
concept same_as = is_same_v<T, U> && is_same_v<U, T>;

}
#endif // __STL_TYPE_TRAITS_H_7A014C0E_F03E_0345_80CD_27E2F05EAAFB__
