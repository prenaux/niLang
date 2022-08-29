#pragma once
#ifndef __STL_SPAN_H_0673C68A_2B92_A44D_9F6A_CD2A65967BB0__
#define __STL_SPAN_H_0673C68A_2B92_A44D_9F6A_CD2A65967BB0__

#include "EASTL/span.h"
#include "array.h"

namespace astl {

template <typename T, ni::tSize Extent = eastl::dynamic_extent>
using span = eastl::span<T,Extent>;

namespace detail {
template <class C>
constexpr auto data(C& c) -> decltype(c.data()) {
  return c.data();
}

template <class C>
constexpr auto data(const C& c) -> decltype(c.data()) {
  return c.data();
}

template <class T, ni::tSize N>
constexpr T* data(T (&array)[N]) noexcept {
  return array;
}

template <class E>
constexpr const E* data(std::initializer_list<E> il) noexcept {
  return il.begin();
}
}

template <typename ElementType, ni::tSize Extent>
constexpr span<ElementType, Extent>
make_span(span<ElementType, Extent> s) noexcept {
  return s;
}

template <typename T, ni::tSize N>
constexpr span<T, N> make_span(T (&arr)[N]) noexcept {
  return {arr};
}

template <typename T, ni::tSize N = eastl::dynamic_extent>
constexpr span<T, N> make_span(T* arr, ni::tSize sz) noexcept {
  return {arr,sz};
}

template <typename T, ni::tSize N>
EA_CONSTEXPR span<T, N> make_span(eastl::array<T, N>& arr) noexcept {
  return {arr};
}

template <typename T, ni::tSize N>
EA_CONSTEXPR span<const T, N>
make_span(const eastl::array<T, N>& arr) noexcept {
  return {arr};
}

template <typename Container>
constexpr span<typename eastl::remove_reference<
                 decltype(*detail::data(std::declval<Container&>()))>::type>
make_span(Container& cont) {
  return {cont};
}

template <typename Container>
constexpr span<const typename Container::value_type>
make_span(const Container& cont) {
  return {cont};
}

template <typename ElementType, ni::tSize Extent>
span<const ni::tU8,
     ((Extent == eastl::dynamic_extent) ?
     eastl::dynamic_extent : sizeof(ElementType) * Extent)>
as_bytes(span<ElementType, Extent> s) noexcept {
  return {reinterpret_cast<const ni::tU8*>(s.data()), s.size_bytes()};
}

template <
  class ElementType, size_t Extent,
  typename std::enable_if<!std::is_const<ElementType>::value, int>::type = 0>
span<ni::tU8,
     ((Extent == eastl::dynamic_extent) ?
     eastl::dynamic_extent : sizeof(ElementType) * Extent)>
as_writable_bytes(span<ElementType, Extent> s) noexcept {
  return {reinterpret_cast<ni::tU8*>(s.data()), s.size_bytes()};
}

template <ni::tSize N, typename E, ni::tSize S>
constexpr auto get(span<E, S> s) -> decltype(s[N]) {
  return s[N];
}

template<class T, ni::tSize N, ni::tSize M> EA_NODISCARD
constexpr bool starts_with(span<T,N> data, span<T,M> prefix) {
    return data.size() >= prefix.size()
        && eastl::equal(prefix.begin(), prefix.end(), data.begin());
}

template<class T, ni::tSize N, ni::tSize M> EA_NODISCARD
constexpr bool ends_with(span<T,N> data, span<T,M> suffix) {
    return data.size() >= suffix.size()
        && eastl::equal(data.end() - suffix.size(), data.end(),
                      suffix.end() - suffix.size());
}

template<class T, ni::tSize N, ni::tSize M> EA_NODISCARD
constexpr bool contains(span<T,N> data, span<T,M> sub) {
  return eastl::search(data.begin(), data.end(), sub.begin(), sub.end()) != data.end();
}

} // namespace astl
#endif // __STL_SPAN_H_0673C68A_2B92_A44D_9F6A_CD2A65967BB0__
