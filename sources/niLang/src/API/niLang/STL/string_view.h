#pragma once
#ifndef __STL_STRING_VIEW_H_BF6A2F8E_6076_6449_9E5C_EE29E79813FC__
#define __STL_STRING_VIEW_H_BF6A2F8E_6076_6449_9E5C_EE29E79813FC__

#include "../Types.h"
#include "EASTL/string_view.h"

namespace astl {

template <typename T>
using basic_string_view = eastl::basic_string_view<T>;

using string_view = basic_string_view<ni::achar>;

}
#endif // __STL_STRING_VIEW_H_BF6A2F8E_6076_6449_9E5C_EE29E79813FC__
