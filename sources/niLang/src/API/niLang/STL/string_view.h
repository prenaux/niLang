#pragma once
#ifndef __STL_STRING_VIEW_H_BF6A2F8E_6076_6449_9E5C_EE29E79813FC__
#define __STL_STRING_VIEW_H_BF6A2F8E_6076_6449_9E5C_EE29E79813FC__

#include "../Types.h"
#include "EASTL/string_view.h"

namespace astl {

template <typename T>
using basic_string_view = eastl::basic_string_view<T>;

using string_view = basic_string_view<ni::achar>;
using wstring_view = basic_string_view<wchar_t>;

using u8string_view = basic_string_view<char8_t>;
using u16string_view = basic_string_view<char16_t>;
using u32string_view = basic_string_view<char32_t>;

inline auto to_sv(const char* aStr, size_t aLen) {
  return string_view(aStr, aLen);
}
inline auto to_sv(const char* aStr) {
  return string_view(aStr);
}

inline auto to_sv(const wchar_t* aStr, size_t aLen) {
  return wstring_view(aStr, aLen);
}
inline auto to_sv(const wchar_t* aStr) {
  return wstring_view(aStr);
}

}
#endif // __STL_STRING_VIEW_H_BF6A2F8E_6076_6449_9E5C_EE29E79813FC__
