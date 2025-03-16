#pragma once
#ifndef __STL_STRING_VIEW_H_BF6A2F8E_6076_6449_9E5C_EE29E79813FC__
#define __STL_STRING_VIEW_H_BF6A2F8E_6076_6449_9E5C_EE29E79813FC__

#include "../Types.h"
#include "EASTL/string_view.h"

namespace astl {

template <typename T>
using basic_string_view = eastl::basic_string_view<T>;

using string_view = basic_string_view<ni::achar>;
using gstring_view = basic_string_view<ni::gchar>;
using xstring_view = basic_string_view<ni::xchar>;

#if niUCharSize == 4
using ustring_view = xstring_view;
#elif niUCharSize == 2
using ustring_view = gstring_view;
#elif niUCharSize == 1
using ustring_view = string_view;
#else
#error "Invalid niUCharSize"
#endif

inline auto to_sv(const char* aStr, size_t aLen) {
  return string_view(aStr, aLen);
}
inline auto to_sv(const char* aStr) {
  return string_view(aStr);
}

inline auto to_sv(const ni::gchar* aStr, size_t aLen) {
  return gstring_view(aStr, aLen);
}
inline auto to_sv(const ni::gchar* aStr) {
  return gstring_view(aStr);
}

inline auto to_sv(const ni::xchar* aStr, size_t aLen) {
  return xstring_view(aStr, aLen);
}
inline auto to_sv(const ni::xchar* aStr) {
  return xstring_view(aStr);
}

}
#endif // __STL_STRING_VIEW_H_BF6A2F8E_6076_6449_9E5C_EE29E79813FC__
