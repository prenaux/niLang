#pragma once
#ifndef __EXPECTED_H_B43C5B80_D5A3_154C_AC04_03564BD031EA__
#define __EXPECTED_H_B43C5B80_D5A3_154C_AC04_03564BD031EA__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

//
// Original reference doc at:
// https://tl.tartanllama.xyz/en/latest/api/expected.html
//
// expected - An implementation of std::expected with extensions
// Written in 2017 by Simon Brand (simonrbrand@gmail.com, @TartanLlama)
//
// Documentation available at http://tl.tartanllama.xyz/
//
// To the extent possible under law, the author(s) have dedicated all
// copyright and related and neighboring rights to this software to the
// public domain worldwide. This software is distributed without any warranty.
//
// You should have received a copy of the CC0 Public Domain Dedication
// along with this software. If not, see
// <http://creativecommons.org/publicdomain/zero/1.0/>.
//

#define ASTL_EXPECTED_VERSION_MAJOR 1
#define ASTL_EXPECTED_VERSION_MINOR 0
#define ASTL_EXPECTED_VERSION_PATCH 1

#include "EASTL/functional.h"
#include "EASTL/type_traits.h"
#include "EASTL/utility.h"
#include "function_ref.h"

#define ASTL_EXPECTED_DEFAULT_CONSTRUCTOR_IS_ERROR

#if EASTL_EXCEPTIONS_ENABLED
#define ASTL_EXPECTED_EXCEPTIONS_ENABLED
#endif

#if (defined(_MSC_VER) && _MSC_VER == 1900)
#define ASTL_EXPECTED_MSVC2015
#define ASTL_EXPECTED_MSVC2015_CONSTEXPR
#else
#define ASTL_EXPECTED_MSVC2015_CONSTEXPR constexpr
#endif

#if (defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ <= 9 &&              \
     !defined(__clang__))
#define ASTL_EXPECTED_GCC49
#endif

#if (defined(__GNUC__) && __GNUC__ == 5 && __GNUC_MINOR__ <= 4 &&              \
     !defined(__clang__))
#define ASTL_EXPECTED_GCC54
#endif

#if (defined(__GNUC__) && __GNUC__ == 5 && __GNUC_MINOR__ <= 5 &&              \
     !defined(__clang__))
#define ASTL_EXPECTED_GCC55
#endif

#if (defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ <= 9 &&              \
     !defined(__clang__))
// GCC < 5 doesn't support overloading on const&& for member functions

#include <type_traits>

#define ASTL_EXPECTED_NO_CONSTRR
// GCC < 5 doesn't support some standard C++11 type traits
#define ASTL_EXPECTED_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(T)                         \
  std::has_trivial_copy_constructor<T>
#define ASTL_EXPECTED_IS_TRIVIALLY_COPY_ASSIGNABLE(T)                            \
  std::has_trivial_copy_assign<T>

// This one will be different for GCC 5.7 if it's ever supported
#define ASTL_EXPECTED_IS_TRIVIALLY_DESTRUCTIBLE(T)                               \
  eastl::is_trivially_destructible<T>

// GCC 5 < v < 8 has a bug in is_trivially_copy_constructible which breaks std::vector
// for non-copyable types
#elif (defined(__GNUC__) && __GNUC__ < 8 &&                                                \
     !defined(__clang__))
#ifndef TL_GCC_LESS_8_TRIVIALLY_COPY_CONSTRUCTIBLE_MUTEX
#define TL_GCC_LESS_8_TRIVIALLY_COPY_CONSTRUCTIBLE_MUTEX
namespace astl {
  namespace detail {
      template<class T>
      struct is_trivially_copy_constructible : eastl::is_trivially_copy_constructible<T>{};
#ifdef _GLIBCXX_VECTOR
      template<class T, class A>
      struct is_trivially_copy_constructible<eastl::vector<T,A>>
          : eastl::false_type{};
#endif
  }
}
#endif

#define ASTL_EXPECTED_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(T)                                     \
  tl::detail::is_trivially_copy_constructible<T>
#define ASTL_EXPECTED_IS_TRIVIALLY_COPY_ASSIGNABLE(T)                                        \
  eastl::is_trivially_copy_assignable<T>
#define ASTL_EXPECTED_IS_TRIVIALLY_DESTRUCTIBLE(T) eastl::is_trivially_destructible<T>
#else
#define ASTL_EXPECTED_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(T)                         \
  eastl::is_trivially_copy_constructible<T>
#define ASTL_EXPECTED_IS_TRIVIALLY_COPY_ASSIGNABLE(T)                            \
  eastl::is_trivially_copy_assignable<T>
#define ASTL_EXPECTED_IS_TRIVIALLY_DESTRUCTIBLE(T)                               \
  eastl::is_trivially_destructible<T>
#endif

#if __cplusplus > 201103L
#define ASTL_EXPECTED_CXX14
#endif

#ifdef ASTL_EXPECTED_GCC49
#define ASTL_EXPECTED_GCC49_CONSTEXPR
#else
#define ASTL_EXPECTED_GCC49_CONSTEXPR constexpr
#endif

#if (__cplusplus == 201103L || defined(ASTL_EXPECTED_MSVC2015) ||                \
     defined(ASTL_EXPECTED_GCC49))
#define ASTL_EXPECTED_11_CONSTEXPR
#else
#define ASTL_EXPECTED_11_CONSTEXPR constexpr
#endif

namespace astl {
template <class T, class E> class expected;

#ifndef TL_MONOSTATE_INPLACE_MUTEX
#define TL_MONOSTATE_INPLACE_MUTEX
class monostate {};

struct in_place_t {
  explicit in_place_t() = default;
};
static constexpr in_place_t in_place{};
#endif

template <class E> class unexpected {
public:
  static_assert(!eastl::is_same<E, void>::value, "E must not be void");

  unexpected() = delete;
  constexpr explicit unexpected(const E &e) : m_val(e) {}

  constexpr explicit unexpected(E &&e) : m_val(eastl::move(e)) {}

  constexpr const E &value() const & { return m_val; }
  ASTL_EXPECTED_11_CONSTEXPR E &value() & { return m_val; }
  ASTL_EXPECTED_11_CONSTEXPR E &&value() && { return eastl::move(m_val); }
  constexpr const E &&value() const && { return eastl::move(m_val); }

private:
  E m_val;
};

template <class E>
constexpr bool operator==(const unexpected<E> &lhs, const unexpected<E> &rhs) {
  return lhs.value() == rhs.value();
}
template <class E>
constexpr bool operator!=(const unexpected<E> &lhs, const unexpected<E> &rhs) {
  return lhs.value() != rhs.value();
}
template <class E>
constexpr bool operator<(const unexpected<E> &lhs, const unexpected<E> &rhs) {
  return lhs.value() < rhs.value();
}
template <class E>
constexpr bool operator<=(const unexpected<E> &lhs, const unexpected<E> &rhs) {
  return lhs.value() <= rhs.value();
}
template <class E>
constexpr bool operator>(const unexpected<E> &lhs, const unexpected<E> &rhs) {
  return lhs.value() > rhs.value();
}
template <class E>
constexpr bool operator>=(const unexpected<E> &lhs, const unexpected<E> &rhs) {
  return lhs.value() >= rhs.value();
}

template <class E>
unexpected<typename eastl::decay<E>::type> make_unexpected(E &&e) {
  return unexpected<typename eastl::decay<E>::type>(eastl::forward<E>(e));
}

struct unexpect_t {
  unexpect_t() = default;
};
static constexpr unexpect_t unexpect{};

namespace detail {
template<typename E>
[[noreturn]] ASTL_EXPECTED_11_CONSTEXPR void throw_exception(E &&e) {
#ifdef ASTL_EXPECTED_EXCEPTIONS_ENABLED
    throw eastl::forward<E>(e);
#else
  #ifdef _MSC_VER
    __assume(0);
  #else
    __builtin_unreachable();
  #endif
#endif
}

#ifndef TL_TRAITS_MUTEX
#define TL_TRAITS_MUTEX
// C++14-style aliases for brevity
template <class T> using remove_const_t = typename eastl::remove_const<T>::type;
template <class T>
using remove_reference_t = typename eastl::remove_reference<T>::type;
template <class T> using decay_t = typename eastl::decay<T>::type;
template <bool E, class T = void>
using enable_if_t = typename eastl::enable_if<E, T>::type;
template <bool B, class T, class F>
using conditional_t = typename eastl::conditional<B, T, F>::type;

// eastl::conjunction from C++17
template <class...> struct conjunction : eastl::true_type {};
template <class B> struct conjunction<B> : B {};
template <class B, class... Bs>
struct conjunction<B, Bs...>
  : eastl::conditional<bool(B::value), conjunction<Bs...>, B>::type {};

#if defined(_LIBCPP_VERSION) && __cplusplus == 201103L
#define TL_TRAITS_LIBCXX_MEM_FN_WORKAROUND
#endif

// In C++11 mode, there's an issue in libc++'s eastl::mem_fn
// which results in a hard-error when using it in a noexcept expression
// in some cases. This is a check to workaround the common failing case.
#ifdef TL_TRAITS_LIBCXX_MEM_FN_WORKAROUND
template <class T> struct is_pointer_to_non_const_member_func : eastl::false_type {};
template <class T, class Ret, class... Args>
struct is_pointer_to_non_const_member_func<Ret(T::*) (Args...)> : eastl::true_type {};
template <class T, class Ret, class... Args>
struct is_pointer_to_non_const_member_func<Ret(T::*) (Args...)&> : eastl::true_type {};
template <class T, class Ret, class... Args>
struct is_pointer_to_non_const_member_func<Ret(T::*) (Args...) &&> : eastl::true_type {};
template <class T, class Ret, class... Args>
struct is_pointer_to_non_const_member_func<Ret(T::*) (Args...) volatile> : eastl::true_type {};
template <class T, class Ret, class... Args>
struct is_pointer_to_non_const_member_func<Ret(T::*) (Args...) volatile &> : eastl::true_type {};
template <class T, class Ret, class... Args>
struct is_pointer_to_non_const_member_func<Ret(T::*) (Args...) volatile &&> : eastl::true_type {};

template <class T> struct is_const_or_const_ref : eastl::false_type {};
template <class T> struct is_const_or_const_ref<T const&> : eastl::true_type {};
template <class T> struct is_const_or_const_ref<T const> : eastl::true_type {};
#endif

// eastl::invoke from C++17
// https://stackoverflow.com/questions/38288042/c11-14-invoke-workaround
template <typename Fn, typename... Args,
#ifdef TL_TRAITS_LIBCXX_MEM_FN_WORKAROUND
  typename = enable_if_t<!(is_pointer_to_non_const_member_func<Fn>::value
    && is_const_or_const_ref<Args...>::value)>,
#endif
  typename = enable_if_t<eastl::is_member_pointer<decay_t<Fn>>::value>,
  int = 0>
  constexpr auto invoke(Fn && f, Args && ... args) noexcept(
    noexcept(eastl::mem_fn(f)(eastl::forward<Args>(args)...)))
  -> decltype(eastl::mem_fn(f)(eastl::forward<Args>(args)...)) {
  return eastl::mem_fn(f)(eastl::forward<Args>(args)...);
}

template <typename Fn, typename... Args,
  typename = enable_if_t<!eastl::is_member_pointer<decay_t<Fn>>::value>>
  constexpr auto invoke(Fn && f, Args && ... args) noexcept(
    noexcept(eastl::forward<Fn>(f)(eastl::forward<Args>(args)...)))
  -> decltype(eastl::forward<Fn>(f)(eastl::forward<Args>(args)...)) {
  return eastl::forward<Fn>(f)(eastl::forward<Args>(args)...);
}

// eastl::invoke_result from C++17
template <class F, class, class... Us> struct invoke_result_impl;

template <class F, class... Us>
struct invoke_result_impl<
  F, decltype(detail::invoke(eastl::declval<F>(), eastl::declval<Us>()...), void()),
  Us...> {
  using type = decltype(detail::invoke(eastl::declval<F>(), eastl::declval<Us>()...));
};

template <class F, class... Us>
using invoke_result = invoke_result_impl<F, void, Us...>;

template <class F, class... Us>
using invoke_result_t = typename invoke_result<F, Us...>::type;

#if defined(_MSC_VER) && _MSC_VER <= 1900
// TODO make a version which works with MSVC 2015
template <class T, class U = T> struct is_swappable : eastl::true_type {};

template <class T, class U = T> struct is_nothrow_swappable : eastl::true_type {};
#else
// https://stackoverflow.com/questions/26744589/what-is-a-proper-way-to-implement-is-swappable-to-test-for-the-swappable-concept
namespace swap_adl_tests {
  // if swap ADL finds this then it would call eastl::swap otherwise (same
  // signature)
  struct tag {};

  template <class T> tag swap(T&, T&);
  template <class T, size_t N> tag swap(T(&a)[N], T(&b)[N]);

  // helper functions to test if an unqualified swap is possible, and if it
  // becomes eastl::swap
  template <class, class> eastl::false_type can_swap(...) noexcept(false);
  template <class T, class U,
    class = decltype(swap(eastl::declval<T&>(), eastl::declval<U&>()))>
    eastl::true_type can_swap(int) noexcept(noexcept(swap(eastl::declval<T&>(),
      eastl::declval<U&>())));

  template <class, class> eastl::false_type uses_std(...);
  template <class T, class U>
  eastl::is_same<decltype(swap(eastl::declval<T&>(), eastl::declval<U&>())), tag>
    uses_std(int);

  template <class T>
  struct is_std_swap_noexcept
    : eastl::integral_constant<bool,
    eastl::is_nothrow_move_constructible<T>::value&&
    eastl::is_nothrow_move_assignable<T>::value> {};

  template <class T, size_t N>
  struct is_std_swap_noexcept<T[N]> : is_std_swap_noexcept<T> {};

  template <class T, class U>
  struct is_adl_swap_noexcept
    : eastl::integral_constant<bool, noexcept(can_swap<T, U>(0))> {};
} // namespace swap_adl_tests

template <class T, class U = T>
struct is_swappable
  : eastl::integral_constant<
  bool,
  decltype(detail::swap_adl_tests::can_swap<T, U>(0))::value &&
  (!decltype(detail::swap_adl_tests::uses_std<T, U>(0))::value ||
  (eastl::is_move_assignable<T>::value &&
    eastl::is_move_constructible<T>::value))> {};

template <class T, size_t N>
struct is_swappable<T[N], T[N]>
  : eastl::integral_constant<
  bool,
  decltype(detail::swap_adl_tests::can_swap<T[N], T[N]>(0))::value &&
  (!decltype(
    detail::swap_adl_tests::uses_std<T[N], T[N]>(0))::value ||
    is_swappable<T, T>::value)> {};

template <class T, class U = T>
struct is_nothrow_swappable
  : eastl::integral_constant<
  bool,
  is_swappable<T, U>::value &&
  ((decltype(detail::swap_adl_tests::uses_std<T, U>(0))::value
    && detail::swap_adl_tests::is_std_swap_noexcept<T>::value) ||
    (!decltype(detail::swap_adl_tests::uses_std<T, U>(0))::value &&
      detail::swap_adl_tests::is_adl_swap_noexcept<T,
      U>::value))> {
};
#endif
#endif

// Trait for checking if a type is a tl::expected
template <class T> struct is_expected_impl : eastl::false_type {};
template <class T, class E>
struct is_expected_impl<expected<T, E>> : eastl::true_type {};
template <class T> using is_expected = is_expected_impl<decay_t<T>>;

template <class T, class E, class U>
using expected_enable_forward_value = detail::enable_if_t<
    eastl::is_constructible<T, U &&>::value &&
    !eastl::is_same<detail::decay_t<U>, in_place_t>::value &&
    !eastl::is_same<expected<T, E>, detail::decay_t<U>>::value &&
    !eastl::is_same<unexpected<E>, detail::decay_t<U>>::value>;

template <class T, class E, class U, class G, class UR, class GR>
using expected_enable_from_other = detail::enable_if_t<
    eastl::is_constructible<T, UR>::value &&
    eastl::is_constructible<E, GR>::value &&
    !eastl::is_constructible<T, expected<U, G> &>::value &&
    !eastl::is_constructible<T, expected<U, G> &&>::value &&
    !eastl::is_constructible<T, const expected<U, G> &>::value &&
    !eastl::is_constructible<T, const expected<U, G> &&>::value &&
    !eastl::is_convertible<expected<U, G> &, T>::value &&
    !eastl::is_convertible<expected<U, G> &&, T>::value &&
    !eastl::is_convertible<const expected<U, G> &, T>::value &&
    !eastl::is_convertible<const expected<U, G> &&, T>::value>;

template <class T, class U>
using is_void_or = conditional_t<eastl::is_void<T>::value, eastl::true_type, U>;

template <class T>
using is_copy_constructible_or_void =
    is_void_or<T, eastl::is_copy_constructible<T>>;

template <class T>
using is_move_constructible_or_void =
    is_void_or<T, eastl::is_move_constructible<T>>;

template <class T>
using is_copy_assignable_or_void =
    is_void_or<T, eastl::is_copy_assignable<T>>;


template <class T>
using is_move_assignable_or_void =
    is_void_or<T, eastl::is_move_assignable<T>>;


} // namespace detail

namespace detail {

struct no_init_t {};
static constexpr no_init_t no_init{};

// Implements the storage of the values, and ensures that the destructor is
// trivial if it can be.
//
// This specialization is for where neither `T` or `E` is trivially
// destructible, so the destructors must be called on destruction of the
// `expected`
template <class T, class E, bool = eastl::is_trivially_destructible<T>::value,
          bool = eastl::is_trivially_destructible<E>::value>
struct expected_storage_base {
#ifdef ASTL_EXPECTED_DEFAULT_CONSTRUCTOR_IS_ERROR
  constexpr expected_storage_base() : m_unexpect(E{}), m_has_val(false) {}
  constexpr expected_storage_base(no_init_t) : m_no_init(), m_has_val(false) {}
#else
  constexpr expected_storage_base() : m_val(T{}), m_has_val(true) {}
  constexpr expected_storage_base(no_init_t) : m_no_init(), m_has_val(false) {}
#endif

  template <class... Args,
            detail::enable_if_t<eastl::is_constructible<T, Args &&...>::value> * =
                nullptr>
  constexpr expected_storage_base(in_place_t, Args &&... args)
      : m_val(eastl::forward<Args>(args)...), m_has_val(true) {}

  template <class U, class... Args,
            detail::enable_if_t<eastl::is_constructible<
                T, std::initializer_list<U> &, Args &&...>::value> * = nullptr>
  constexpr expected_storage_base(in_place_t, std::initializer_list<U> il,
                                  Args &&... args)
      : m_val(il, eastl::forward<Args>(args)...), m_has_val(true) {}
  template <class... Args,
            detail::enable_if_t<eastl::is_constructible<E, Args &&...>::value> * =
                nullptr>
  constexpr explicit expected_storage_base(unexpect_t, Args &&... args)
      : m_unexpect(eastl::forward<Args>(args)...), m_has_val(false) {}

  template <class U, class... Args,
            detail::enable_if_t<eastl::is_constructible<
                E, std::initializer_list<U> &, Args &&...>::value> * = nullptr>
  constexpr explicit expected_storage_base(unexpect_t,
                                           std::initializer_list<U> il,
                                           Args &&... args)
      : m_unexpect(il, eastl::forward<Args>(args)...), m_has_val(false) {}

  ~expected_storage_base() {
    if (m_has_val) {
      m_val.~T();
    } else {
      m_unexpect.~unexpected<E>();
    }
  }
  union {
    T m_val;
    unexpected<E> m_unexpect;
    char m_no_init;
  };
  bool m_has_val;
};

// This specialization is for when both `T` and `E` are trivially-destructible,
// so the destructor of the `expected` can be trivial.
template <class T, class E> struct expected_storage_base<T, E, true, true> {
#ifdef ASTL_EXPECTED_DEFAULT_CONSTRUCTOR_IS_ERROR
  constexpr expected_storage_base() : m_unexpect(E{}), m_has_val(false) {}
  constexpr expected_storage_base(no_init_t) : m_no_init(), m_has_val(false) {}
#else
  constexpr expected_storage_base() : m_val(T{}), m_has_val(true) {}
  constexpr expected_storage_base(no_init_t) : m_no_init(), m_has_val(false) {}
#endif

  template <class... Args,
            detail::enable_if_t<eastl::is_constructible<T, Args &&...>::value> * =
                nullptr>
  constexpr expected_storage_base(in_place_t, Args &&... args)
      : m_val(eastl::forward<Args>(args)...), m_has_val(true) {}

  template <class U, class... Args,
            detail::enable_if_t<eastl::is_constructible<
                T, std::initializer_list<U> &, Args &&...>::value> * = nullptr>
  constexpr expected_storage_base(in_place_t, std::initializer_list<U> il,
                                  Args &&... args)
      : m_val(il, eastl::forward<Args>(args)...), m_has_val(true) {}
  template <class... Args,
            detail::enable_if_t<eastl::is_constructible<E, Args &&...>::value> * =
                nullptr>
  constexpr explicit expected_storage_base(unexpect_t, Args &&... args)
      : m_unexpect(eastl::forward<Args>(args)...), m_has_val(false) {}

  template <class U, class... Args,
            detail::enable_if_t<eastl::is_constructible<
                E, std::initializer_list<U> &, Args &&...>::value> * = nullptr>
  constexpr explicit expected_storage_base(unexpect_t,
                                           std::initializer_list<U> il,
                                           Args &&... args)
      : m_unexpect(il, eastl::forward<Args>(args)...), m_has_val(false) {}

  ~expected_storage_base() = default;
  union {
    T m_val;
    unexpected<E> m_unexpect;
    char m_no_init;
  };
  bool m_has_val;
};

// T is trivial, E is not.
template <class T, class E> struct expected_storage_base<T, E, true, false> {
#ifdef ASTL_EXPECTED_DEFAULT_CONSTRUCTOR_IS_ERROR
  constexpr expected_storage_base() : m_unexpect(E{}), m_has_val(false) {}
  ASTL_EXPECTED_MSVC2015_CONSTEXPR expected_storage_base(no_init_t)
      : m_no_init(), m_has_val(false) {}
#else
  constexpr expected_storage_base() : m_val(T{}), m_has_val(true) {}
  ASTL_EXPECTED_MSVC2015_CONSTEXPR expected_storage_base(no_init_t)
      : m_no_init(), m_has_val(false) {}
#endif

  template <class... Args,
            detail::enable_if_t<eastl::is_constructible<T, Args &&...>::value> * =
                nullptr>
  constexpr expected_storage_base(in_place_t, Args &&... args)
      : m_val(eastl::forward<Args>(args)...), m_has_val(true) {}

  template <class U, class... Args,
            detail::enable_if_t<eastl::is_constructible<
                T, std::initializer_list<U> &, Args &&...>::value> * = nullptr>
  constexpr expected_storage_base(in_place_t, std::initializer_list<U> il,
                                  Args &&... args)
      : m_val(il, eastl::forward<Args>(args)...), m_has_val(true) {}
  template <class... Args,
            detail::enable_if_t<eastl::is_constructible<E, Args &&...>::value> * =
                nullptr>
  constexpr explicit expected_storage_base(unexpect_t, Args &&... args)
      : m_unexpect(eastl::forward<Args>(args)...), m_has_val(false) {}

  template <class U, class... Args,
            detail::enable_if_t<eastl::is_constructible<
                E, std::initializer_list<U> &, Args &&...>::value> * = nullptr>
  constexpr explicit expected_storage_base(unexpect_t,
                                           std::initializer_list<U> il,
                                           Args &&... args)
      : m_unexpect(il, eastl::forward<Args>(args)...), m_has_val(false) {}

  ~expected_storage_base() {
    if (!m_has_val) {
      m_unexpect.~unexpected<E>();
    }
  }

  union {
    T m_val;
    unexpected<E> m_unexpect;
    char m_no_init;
  };
  bool m_has_val;
};

// E is trivial, T is not.
template <class T, class E> struct expected_storage_base<T, E, false, true> {
#ifdef ASTL_EXPECTED_DEFAULT_CONSTRUCTOR_IS_ERROR
  constexpr expected_storage_base() : m_unexpect(E{}), m_has_val(false) {}
  constexpr expected_storage_base(no_init_t) : m_no_init(), m_has_val(false) {}
#else
  constexpr expected_storage_base() : m_val(T{}), m_has_val(true) {}
  constexpr expected_storage_base(no_init_t) : m_no_init(), m_has_val(false) {}
#endif

  template <class... Args,
            detail::enable_if_t<eastl::is_constructible<T, Args &&...>::value> * =
                nullptr>
  constexpr expected_storage_base(in_place_t, Args &&... args)
      : m_val(eastl::forward<Args>(args)...), m_has_val(true) {}

  template <class U, class... Args,
            detail::enable_if_t<eastl::is_constructible<
                T, std::initializer_list<U> &, Args &&...>::value> * = nullptr>
  constexpr expected_storage_base(in_place_t, std::initializer_list<U> il,
                                  Args &&... args)
      : m_val(il, eastl::forward<Args>(args)...), m_has_val(true) {}
  template <class... Args,
            detail::enable_if_t<eastl::is_constructible<E, Args &&...>::value> * =
                nullptr>
  constexpr explicit expected_storage_base(unexpect_t, Args &&... args)
      : m_unexpect(eastl::forward<Args>(args)...), m_has_val(false) {}

  template <class U, class... Args,
            detail::enable_if_t<eastl::is_constructible<
                E, std::initializer_list<U> &, Args &&...>::value> * = nullptr>
  constexpr explicit expected_storage_base(unexpect_t,
                                           std::initializer_list<U> il,
                                           Args &&... args)
      : m_unexpect(il, eastl::forward<Args>(args)...), m_has_val(false) {}

  ~expected_storage_base() {
    if (m_has_val) {
      m_val.~T();
    }
  }
  union {
    T m_val;
    unexpected<E> m_unexpect;
    char m_no_init;
  };
  bool m_has_val;
};

// `T` is `void`, `E` is trivially-destructible
template <class E> struct expected_storage_base<void, E, false, true> {
#ifdef ASTL_EXPECTED_DEFAULT_CONSTRUCTOR_IS_ERROR
  constexpr expected_storage_base() : m_unexpect(E{}), m_has_val(false) {}
  constexpr expected_storage_base(no_init_t) : m_val(), m_has_val(false) {}
#else
  ASTL_EXPECTED_MSVC2015_CONSTEXPR expected_storage_base() : m_has_val(true) {}
  constexpr expected_storage_base(no_init_t) : m_val(), m_has_val(false) {}
#endif

  constexpr expected_storage_base(in_place_t) : m_has_val(true) {}

  template <class... Args,
            detail::enable_if_t<eastl::is_constructible<E, Args &&...>::value> * =
                nullptr>
  constexpr explicit expected_storage_base(unexpect_t, Args &&... args)
      : m_unexpect(eastl::forward<Args>(args)...), m_has_val(false) {}

  template <class U, class... Args,
            detail::enable_if_t<eastl::is_constructible<
                E, std::initializer_list<U> &, Args &&...>::value> * = nullptr>
  constexpr explicit expected_storage_base(unexpect_t,
                                           std::initializer_list<U> il,
                                           Args &&... args)
      : m_unexpect(il, eastl::forward<Args>(args)...), m_has_val(false) {}

  ~expected_storage_base() = default;
  struct dummy {};
  union {
    unexpected<E> m_unexpect;
    dummy m_val;
  };
  bool m_has_val;
};

// `T` is `void`, `E` is not trivially-destructible
template <class E> struct expected_storage_base<void, E, false, false> {
#ifdef ASTL_EXPECTED_DEFAULT_CONSTRUCTOR_IS_ERROR
  constexpr expected_storage_base() : m_unexpect(E{}), m_has_val(false) {}
  constexpr expected_storage_base(no_init_t) : m_dummy(), m_has_val(false) {}
#else
  constexpr expected_storage_base() : m_dummy(), m_has_val(true) {}
  constexpr expected_storage_base(no_init_t) : m_dummy(), m_has_val(false) {}
#endif

  constexpr expected_storage_base(in_place_t) : m_dummy(), m_has_val(true) {}

  template <class... Args,
            detail::enable_if_t<eastl::is_constructible<E, Args &&...>::value> * =
                nullptr>
  constexpr explicit expected_storage_base(unexpect_t, Args &&... args)
      : m_unexpect(eastl::forward<Args>(args)...), m_has_val(false) {}

  template <class U, class... Args,
            detail::enable_if_t<eastl::is_constructible<
                E, std::initializer_list<U> &, Args &&...>::value> * = nullptr>
  constexpr explicit expected_storage_base(unexpect_t,
                                           std::initializer_list<U> il,
                                           Args &&... args)
      : m_unexpect(il, eastl::forward<Args>(args)...), m_has_val(false) {}

  ~expected_storage_base() {
    if (!m_has_val) {
      m_unexpect.~unexpected<E>();
    }
  }

  union {
    unexpected<E> m_unexpect;
    char m_dummy;
  };
  bool m_has_val;
};

// This base class provides some handy member functions which can be used in
// further derived classes
template <class T, class E>
struct expected_operations_base : expected_storage_base<T, E> {
  using expected_storage_base<T, E>::expected_storage_base;

  template <class... Args> void construct(Args &&... args) noexcept {
    new (eastl::addressof(this->m_val)) T(eastl::forward<Args>(args)...);
    this->m_has_val = true;
  }

  template <class Rhs> void construct_with(Rhs &&rhs) noexcept {
    new (eastl::addressof(this->m_val)) T(eastl::forward<Rhs>(rhs).get());
    this->m_has_val = true;
  }

  template <class... Args> void construct_error(Args &&... args) noexcept {
    new (eastl::addressof(this->m_unexpect))
        unexpected<E>(eastl::forward<Args>(args)...);
    this->m_has_val = false;
  }

  #ifdef ASTL_EXPECTED_EXCEPTIONS_ENABLED

  // These assign overloads ensure that the most efficient assignment
  // implementation is used while maintaining the strong exception guarantee.
  // The problematic case is where rhs has a value, but *this does not.
  //
  // This overload handles the case where we can just copy-construct `T`
  // directly into place without throwing.
  template <class U = T,
            detail::enable_if_t<eastl::is_nothrow_copy_constructible<U>::value>
                * = nullptr>
  void assign(const expected_operations_base &rhs) noexcept {
    if (!this->m_has_val && rhs.m_has_val) {
      geterr().~unexpected<E>();
      construct(rhs.get());
    } else {
      assign_common(rhs);
    }
  }

  // This overload handles the case where we can attempt to create a copy of
  // `T`, then no-throw move it into place if the copy was successful.
  template <class U = T,
            detail::enable_if_t<!eastl::is_nothrow_copy_constructible<U>::value &&
                                eastl::is_nothrow_move_constructible<U>::value>
                * = nullptr>
  void assign(const expected_operations_base &rhs) noexcept {
    if (!this->m_has_val && rhs.m_has_val) {
      T tmp = rhs.get();
      geterr().~unexpected<E>();
      construct(eastl::move(tmp));
    } else {
      assign_common(rhs);
    }
  }

  // This overload is the worst-case, where we have to move-construct the
  // unexpected value into temporary storage, then try to copy the T into place.
  // If the construction succeeds, then everything is fine, but if it throws,
  // then we move the old unexpected value back into place before rethrowing the
  // exception.
  template <class U = T,
            detail::enable_if_t<!eastl::is_nothrow_copy_constructible<U>::value &&
                                !eastl::is_nothrow_move_constructible<U>::value>
                * = nullptr>
  void assign(const expected_operations_base &rhs) {
    if (!this->m_has_val && rhs.m_has_val) {
      auto tmp = eastl::move(geterr());
      geterr().~unexpected<E>();

#ifdef ASTL_EXPECTED_EXCEPTIONS_ENABLED
      try {
        construct(rhs.get());
      } catch (...) {
        geterr() = eastl::move(tmp);
        throw;
      }
#else
      construct(rhs.get());
#endif
    } else {
      assign_common(rhs);
    }
  }

  // These overloads do the same as above, but for rvalues
  template <class U = T,
            detail::enable_if_t<eastl::is_nothrow_move_constructible<U>::value>
                * = nullptr>
  void assign(expected_operations_base &&rhs) noexcept {
    if (!this->m_has_val && rhs.m_has_val) {
      geterr().~unexpected<E>();
      construct(eastl::move(rhs).get());
    } else {
      assign_common(eastl::move(rhs));
    }
  }

  template <class U = T,
            detail::enable_if_t<!eastl::is_nothrow_move_constructible<U>::value>
                * = nullptr>
  void assign(expected_operations_base &&rhs) {
    if (!this->m_has_val && rhs.m_has_val) {
      auto tmp = eastl::move(geterr());
      geterr().~unexpected<E>();
#ifdef ASTL_EXPECTED_EXCEPTIONS_ENABLED
      try {
        construct(eastl::move(rhs).get());
      } catch (...) {
        geterr() = eastl::move(tmp);
        throw;
      }
#else
      construct(eastl::move(rhs).get());
#endif
    } else {
      assign_common(eastl::move(rhs));
    }
  }

  #else

  // If exceptions are disabled then we can just copy-construct
  void assign(const expected_operations_base &rhs) noexcept {
    if (!this->m_has_val && rhs.m_has_val) {
      geterr().~unexpected<E>();
      construct(rhs.get());
    } else {
      assign_common(rhs);
    }
  }

  void assign(expected_operations_base &&rhs) noexcept {
    if (!this->m_has_val && rhs.m_has_val) {
      geterr().~unexpected<E>();
      construct(eastl::move(rhs).get());
    } else {
      assign_common(rhs);
    }
  }

  #endif

  // The common part of move/copy assigning
  template <class Rhs> void assign_common(Rhs &&rhs) {
    if (this->m_has_val) {
      if (rhs.m_has_val) {
        get() = eastl::forward<Rhs>(rhs).get();
      } else {
		destroy_val();
        construct_error(eastl::forward<Rhs>(rhs).geterr());
      }
    } else {
      if (!rhs.m_has_val) {
        geterr() = eastl::forward<Rhs>(rhs).geterr();
      }
    }
  }

  bool has_value() const { return this->m_has_val; }

  ASTL_EXPECTED_11_CONSTEXPR T &get() & { return this->m_val; }
  constexpr const T &get() const & { return this->m_val; }
  ASTL_EXPECTED_11_CONSTEXPR T &&get() && { return eastl::move(this->m_val); }
#ifndef ASTL_EXPECTED_NO_CONSTRR
  constexpr const T &&get() const && { return eastl::move(this->m_val); }
#endif

  ASTL_EXPECTED_11_CONSTEXPR unexpected<E> &geterr() & {
    return this->m_unexpect;
  }
  constexpr const unexpected<E> &geterr() const & { return this->m_unexpect; }
  ASTL_EXPECTED_11_CONSTEXPR unexpected<E> &&geterr() && {
    return eastl::move(this->m_unexpect);
  }
#ifndef ASTL_EXPECTED_NO_CONSTRR
  constexpr const unexpected<E> &&geterr() const && {
    return eastl::move(this->m_unexpect);
  }
#endif

  ASTL_EXPECTED_11_CONSTEXPR void destroy_val() {
	get().~T();
  }
};

// This base class provides some handy member functions which can be used in
// further derived classes
template <class E>
struct expected_operations_base<void, E> : expected_storage_base<void, E> {
  using expected_storage_base<void, E>::expected_storage_base;

  template <class... Args> void construct() noexcept { this->m_has_val = true; }

  // This function doesn't use its argument, but needs it so that code in
  // levels above this can work independently of whether T is void
  template <class Rhs> void construct_with(Rhs &&) noexcept {
    this->m_has_val = true;
  }

  template <class... Args> void construct_error(Args &&... args) noexcept {
    new (eastl::addressof(this->m_unexpect))
        unexpected<E>(eastl::forward<Args>(args)...);
    this->m_has_val = false;
  }

  template <class Rhs> void assign(Rhs &&rhs) noexcept {
    if (!this->m_has_val) {
      if (rhs.m_has_val) {
        geterr().~unexpected<E>();
        construct();
      } else {
        geterr() = eastl::forward<Rhs>(rhs).geterr();
      }
    } else {
      if (!rhs.m_has_val) {
        construct_error(eastl::forward<Rhs>(rhs).geterr());
      }
    }
  }

  bool has_value() const { return this->m_has_val; }

  ASTL_EXPECTED_11_CONSTEXPR unexpected<E> &geterr() & {
    return this->m_unexpect;
  }
  constexpr const unexpected<E> &geterr() const & { return this->m_unexpect; }
  ASTL_EXPECTED_11_CONSTEXPR unexpected<E> &&geterr() && {
    return eastl::move(this->m_unexpect);
  }
#ifndef ASTL_EXPECTED_NO_CONSTRR
  constexpr const unexpected<E> &&geterr() const && {
    return eastl::move(this->m_unexpect);
  }
#endif

  ASTL_EXPECTED_11_CONSTEXPR void destroy_val() {
	  //no-op
  }
};

// This class manages conditionally having a trivial copy constructor
// This specialization is for when T and E are trivially copy constructible
template <class T, class E,
          bool = is_void_or<T, ASTL_EXPECTED_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(T)>::
              value &&ASTL_EXPECTED_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(E)::value>
struct expected_copy_base : expected_operations_base<T, E> {
  using expected_operations_base<T, E>::expected_operations_base;
};

// This specialization is for when T or E are not trivially copy constructible
template <class T, class E>
struct expected_copy_base<T, E, false> : expected_operations_base<T, E> {
  using expected_operations_base<T, E>::expected_operations_base;

  expected_copy_base() = default;
  expected_copy_base(const expected_copy_base &rhs)
      : expected_operations_base<T, E>(no_init) {
    if (rhs.has_value()) {
      this->construct_with(rhs);
    } else {
      this->construct_error(rhs.geterr());
    }
  }

  expected_copy_base(expected_copy_base &&rhs) = default;
  expected_copy_base &operator=(const expected_copy_base &rhs) = default;
  expected_copy_base &operator=(expected_copy_base &&rhs) = default;
};

// This class manages conditionally having a trivial move constructor
// Unfortunately there's no way to achieve this in GCC < 5 AFAIK, since it
// doesn't implement an analogue to eastl::is_trivially_move_constructible. We
// have to make do with a non-trivial move constructor even if T is trivially
// move constructible
#ifndef ASTL_EXPECTED_GCC49
template <class T, class E,
          bool = is_void_or<T, eastl::is_trivially_move_constructible<T>>::value
              &&eastl::is_trivially_move_constructible<E>::value>
struct expected_move_base : expected_copy_base<T, E> {
  using expected_copy_base<T, E>::expected_copy_base;
};
#else
template <class T, class E, bool = false> struct expected_move_base;
#endif
template <class T, class E>
struct expected_move_base<T, E, false> : expected_copy_base<T, E> {
  using expected_copy_base<T, E>::expected_copy_base;

  expected_move_base() = default;
  expected_move_base(const expected_move_base &rhs) = default;

  expected_move_base(expected_move_base &&rhs) noexcept(
      eastl::is_nothrow_move_constructible<T>::value)
      : expected_copy_base<T, E>(no_init) {
    if (rhs.has_value()) {
      this->construct_with(eastl::move(rhs));
    } else {
      this->construct_error(eastl::move(rhs.geterr()));
    }
  }
  expected_move_base &operator=(const expected_move_base &rhs) = default;
  expected_move_base &operator=(expected_move_base &&rhs) = default;
};

// This class manages conditionally having a trivial copy assignment operator
template <class T, class E,
          bool = is_void_or<
              T, conjunction<ASTL_EXPECTED_IS_TRIVIALLY_COPY_ASSIGNABLE(T),
                             ASTL_EXPECTED_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(T),
                             ASTL_EXPECTED_IS_TRIVIALLY_DESTRUCTIBLE(T)>>::value
              &&ASTL_EXPECTED_IS_TRIVIALLY_COPY_ASSIGNABLE(E)::value
                  &&ASTL_EXPECTED_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(E)::value
                      &&ASTL_EXPECTED_IS_TRIVIALLY_DESTRUCTIBLE(E)::value>
struct expected_copy_assign_base : expected_move_base<T, E> {
  using expected_move_base<T, E>::expected_move_base;
};

template <class T, class E>
struct expected_copy_assign_base<T, E, false> : expected_move_base<T, E> {
  using expected_move_base<T, E>::expected_move_base;

  expected_copy_assign_base() = default;
  expected_copy_assign_base(const expected_copy_assign_base &rhs) = default;

  expected_copy_assign_base(expected_copy_assign_base &&rhs) = default;
  expected_copy_assign_base &operator=(const expected_copy_assign_base &rhs) {
    this->assign(rhs);
    return *this;
  }
  expected_copy_assign_base &
  operator=(expected_copy_assign_base &&rhs) = default;
};

// This class manages conditionally having a trivial move assignment operator
// Unfortunately there's no way to achieve this in GCC < 5 AFAIK, since it
// doesn't implement an analogue to eastl::is_trivially_move_assignable. We have
// to make do with a non-trivial move assignment operator even if T is trivially
// move assignable
#ifndef ASTL_EXPECTED_GCC49
template <class T, class E,
          bool =
              is_void_or<T, conjunction<eastl::is_trivially_destructible<T>,
                                        eastl::is_trivially_move_constructible<T>,
                                        eastl::is_trivially_move_assignable<T>>>::
                  value &&eastl::is_trivially_destructible<E>::value
                      &&eastl::is_trivially_move_constructible<E>::value
                          &&eastl::is_trivially_move_assignable<E>::value>
struct expected_move_assign_base : expected_copy_assign_base<T, E> {
  using expected_copy_assign_base<T, E>::expected_copy_assign_base;
};
#else
template <class T, class E, bool = false> struct expected_move_assign_base;
#endif

template <class T, class E>
struct expected_move_assign_base<T, E, false>
    : expected_copy_assign_base<T, E> {
  using expected_copy_assign_base<T, E>::expected_copy_assign_base;

  expected_move_assign_base() = default;
  expected_move_assign_base(const expected_move_assign_base &rhs) = default;

  expected_move_assign_base(expected_move_assign_base &&rhs) = default;

  expected_move_assign_base &
  operator=(const expected_move_assign_base &rhs) = default;

  expected_move_assign_base &
  operator=(expected_move_assign_base &&rhs) noexcept(
      eastl::is_nothrow_move_constructible<T>::value
          &&eastl::is_nothrow_move_assignable<T>::value) {
    this->assign(eastl::move(rhs));
    return *this;
  }
};

// expected_delete_ctor_base will conditionally delete copy and move
// constructors depending on whether T is copy/move constructible
template <class T, class E,
          bool EnableCopy = (is_copy_constructible_or_void<T>::value &&
                             eastl::is_copy_constructible<E>::value),
          bool EnableMove = (is_move_constructible_or_void<T>::value &&
                             eastl::is_move_constructible<E>::value)>
struct expected_delete_ctor_base {
  expected_delete_ctor_base() = default;
  expected_delete_ctor_base(const expected_delete_ctor_base &) = default;
  expected_delete_ctor_base(expected_delete_ctor_base &&) noexcept = default;
  expected_delete_ctor_base &
  operator=(const expected_delete_ctor_base &) = default;
  expected_delete_ctor_base &
  operator=(expected_delete_ctor_base &&) noexcept = default;
};

template <class T, class E>
struct expected_delete_ctor_base<T, E, true, false> {
  expected_delete_ctor_base() = default;
  expected_delete_ctor_base(const expected_delete_ctor_base &) = default;
  expected_delete_ctor_base(expected_delete_ctor_base &&) noexcept = delete;
  expected_delete_ctor_base &
  operator=(const expected_delete_ctor_base &) = default;
  expected_delete_ctor_base &
  operator=(expected_delete_ctor_base &&) noexcept = default;
};

template <class T, class E>
struct expected_delete_ctor_base<T, E, false, true> {
  expected_delete_ctor_base() = default;
  expected_delete_ctor_base(const expected_delete_ctor_base &) = delete;
  expected_delete_ctor_base(expected_delete_ctor_base &&) noexcept = default;
  expected_delete_ctor_base &
  operator=(const expected_delete_ctor_base &) = default;
  expected_delete_ctor_base &
  operator=(expected_delete_ctor_base &&) noexcept = default;
};

template <class T, class E>
struct expected_delete_ctor_base<T, E, false, false> {
  expected_delete_ctor_base() = default;
  expected_delete_ctor_base(const expected_delete_ctor_base &) = delete;
  expected_delete_ctor_base(expected_delete_ctor_base &&) noexcept = delete;
  expected_delete_ctor_base &
  operator=(const expected_delete_ctor_base &) = default;
  expected_delete_ctor_base &
  operator=(expected_delete_ctor_base &&) noexcept = default;
};

// expected_delete_assign_base will conditionally delete copy and move
// constructors depending on whether T and E are copy/move constructible +
// assignable
template <class T, class E,
          bool EnableCopy = (is_copy_constructible_or_void<T>::value &&
                             eastl::is_copy_constructible<E>::value &&
                             is_copy_assignable_or_void<T>::value &&
                             eastl::is_copy_assignable<E>::value),
          bool EnableMove = (is_move_constructible_or_void<T>::value &&
                             eastl::is_move_constructible<E>::value &&
                             is_move_assignable_or_void<T>::value &&
                             eastl::is_move_assignable<E>::value)>
struct expected_delete_assign_base {
  expected_delete_assign_base() = default;
  expected_delete_assign_base(const expected_delete_assign_base &) = default;
  expected_delete_assign_base(expected_delete_assign_base &&) noexcept =
      default;
  expected_delete_assign_base &
  operator=(const expected_delete_assign_base &) = default;
  expected_delete_assign_base &
  operator=(expected_delete_assign_base &&) noexcept = default;
};

template <class T, class E>
struct expected_delete_assign_base<T, E, true, false> {
  expected_delete_assign_base() = default;
  expected_delete_assign_base(const expected_delete_assign_base &) = default;
  expected_delete_assign_base(expected_delete_assign_base &&) noexcept =
      default;
  expected_delete_assign_base &
  operator=(const expected_delete_assign_base &) = default;
  expected_delete_assign_base &
  operator=(expected_delete_assign_base &&) noexcept = delete;
};

template <class T, class E>
struct expected_delete_assign_base<T, E, false, true> {
  expected_delete_assign_base() = default;
  expected_delete_assign_base(const expected_delete_assign_base &) = default;
  expected_delete_assign_base(expected_delete_assign_base &&) noexcept =
      default;
  expected_delete_assign_base &
  operator=(const expected_delete_assign_base &) = delete;
  expected_delete_assign_base &
  operator=(expected_delete_assign_base &&) noexcept = default;
};

template <class T, class E>
struct expected_delete_assign_base<T, E, false, false> {
  expected_delete_assign_base() = default;
  expected_delete_assign_base(const expected_delete_assign_base &) = default;
  expected_delete_assign_base(expected_delete_assign_base &&) noexcept =
      default;
  expected_delete_assign_base &
  operator=(const expected_delete_assign_base &) = delete;
  expected_delete_assign_base &
  operator=(expected_delete_assign_base &&) noexcept = delete;
};

// This is needed to be able to construct the expected_default_ctor_base which
// follows, while still conditionally deleting the default constructor.
struct default_constructor_tag {
  explicit constexpr default_constructor_tag() = default;
};

// expected_default_ctor_base will ensure that expected has a deleted default
// consturctor if T is not default constructible.
// This specialization is for when T is default constructible
template <class T, class E,
#ifdef ASTL_EXPECTED_DEFAULT_CONSTRUCTOR_IS_ERROR
          bool Enable =
              eastl::is_default_constructible<E>::value || eastl::is_void<E>::value
#else
          bool Enable =
              eastl::is_default_constructible<T>::value || eastl::is_void<T>::value
#endif
          >
struct expected_default_ctor_base {
  constexpr expected_default_ctor_base() noexcept = default;
  constexpr expected_default_ctor_base(
      expected_default_ctor_base const &) noexcept = default;
  constexpr expected_default_ctor_base(expected_default_ctor_base &&) noexcept =
      default;
  expected_default_ctor_base &
  operator=(expected_default_ctor_base const &) noexcept = default;
  expected_default_ctor_base &
  operator=(expected_default_ctor_base &&) noexcept = default;

  constexpr explicit expected_default_ctor_base(default_constructor_tag) {}
};

// This specialization is for when T is not default constructible
template <class T, class E> struct expected_default_ctor_base<T, E, false> {
  constexpr expected_default_ctor_base() noexcept = delete;
  constexpr expected_default_ctor_base(
      expected_default_ctor_base const &) noexcept = default;
  constexpr expected_default_ctor_base(expected_default_ctor_base &&) noexcept =
      default;
  expected_default_ctor_base &
  operator=(expected_default_ctor_base const &) noexcept = default;
  expected_default_ctor_base &
  operator=(expected_default_ctor_base &&) noexcept = default;

  constexpr explicit expected_default_ctor_base(default_constructor_tag) {}
};
} // namespace detail

template <class E> class bad_expected_access : public std::exception {
public:
  explicit bad_expected_access(E e) : m_val(eastl::move(e)) {}

  virtual const char *what() const noexcept override {
    return "Bad expected access";
  }

  const E &error() const & { return m_val; }
  E &error() & { return m_val; }
  const E &&error() const && { return eastl::move(m_val); }
  E &&error() && { return eastl::move(m_val); }

private:
  E m_val;
};

/// An `expected<T, E>` object is an object that contains the storage for
/// another object and manages the lifetime of this contained object `T`.
/// Alternatively it could contain the storage for another unexpected object
/// `E`. The contained object may not be initialized after the expected object
/// has been initialized, and may not be destroyed before the expected object
/// has been destroyed. The initialization state of the contained object is
/// tracked by the expected object.
template <class T, class E>
class expected : private detail::expected_move_assign_base<T, E>,
                 private detail::expected_delete_ctor_base<T, E>,
                 private detail::expected_delete_assign_base<T, E>,
                 private detail::expected_default_ctor_base<T, E> {
  static_assert(!eastl::is_reference<T>::value, "T must not be a reference");
  static_assert(!eastl::is_same<T, eastl::remove_cv<in_place_t>::type>::value,
                "T must not be in_place_t");
  static_assert(!eastl::is_same<T, eastl::remove_cv<unexpect_t>::type>::value,
                "T must not be unexpect_t");
  static_assert(!eastl::is_same<T, typename eastl::remove_cv<unexpected<E>>::type>::value,
                "T must not be unexpected<E>");
  static_assert(!eastl::is_reference<E>::value, "E must not be a reference");

  T *valptr() { return eastl::addressof(this->m_val); }
  const T *valptr() const { return eastl::addressof(this->m_val); }
  unexpected<E> *errptr() { return eastl::addressof(this->m_unexpect); }
  const unexpected<E> *errptr() const { return eastl::addressof(this->m_unexpect); }

  template <class U = T,
            detail::enable_if_t<!eastl::is_void<U>::value> * = nullptr>
  ASTL_EXPECTED_11_CONSTEXPR U &val() {
    return this->m_val;
  }
  ASTL_EXPECTED_11_CONSTEXPR unexpected<E> &err() { return this->m_unexpect; }

  template <class U = T,
            detail::enable_if_t<!eastl::is_void<U>::value> * = nullptr>
  constexpr const U &val() const {
    return this->m_val;
  }
  constexpr const unexpected<E> &err() const { return this->m_unexpect; }

  using impl_base = detail::expected_move_assign_base<T, E>;
  using ctor_base = detail::expected_default_ctor_base<T, E>;

public:
  typedef T value_type;
  typedef E error_type;
  typedef unexpected<E> unexpected_type;

#if defined(ASTL_EXPECTED_CXX14) && !defined(ASTL_EXPECTED_GCC49) &&               \
    !defined(ASTL_EXPECTED_GCC54) && !defined(ASTL_EXPECTED_GCC55)
  template <class F> ASTL_EXPECTED_11_CONSTEXPR auto and_then(F &&f) & {
    return and_then_impl(*this, eastl::forward<F>(f));
  }
  template <class F> ASTL_EXPECTED_11_CONSTEXPR auto and_then(F &&f) && {
    return and_then_impl(eastl::move(*this), eastl::forward<F>(f));
  }
  template <class F> constexpr auto and_then(F &&f) const & {
    return and_then_impl(*this, eastl::forward<F>(f));
  }

#ifndef ASTL_EXPECTED_NO_CONSTRR
  template <class F> constexpr auto and_then(F &&f) const && {
    return and_then_impl(eastl::move(*this), eastl::forward<F>(f));
  }
#endif

#else
  template <class F>
  ASTL_EXPECTED_11_CONSTEXPR auto
  and_then(F &&f) & -> decltype(and_then_impl(eastl::declval<expected&>(), eastl::forward<F>(f))) {
    return and_then_impl(*this, eastl::forward<F>(f));
  }
  template <class F>
  ASTL_EXPECTED_11_CONSTEXPR auto and_then(F &&f) && -> decltype(
      and_then_impl(eastl::declval<expected&&>(), eastl::forward<F>(f))) {
    return and_then_impl(eastl::move(*this), eastl::forward<F>(f));
  }
  template <class F>
  constexpr auto and_then(F &&f) const & -> decltype(
      and_then_impl(eastl::declval<expected const&>(), eastl::forward<F>(f))) {
    return and_then_impl(*this, eastl::forward<F>(f));
  }

#ifndef ASTL_EXPECTED_NO_CONSTRR
  template <class F>
  constexpr auto and_then(F &&f) const && -> decltype(
      and_then_impl(eastl::declval<expected const&&>(), eastl::forward<F>(f))) {
    return and_then_impl(eastl::move(*this), eastl::forward<F>(f));
  }
#endif
#endif

#if defined(ASTL_EXPECTED_CXX14) && !defined(ASTL_EXPECTED_GCC49) &&               \
    !defined(ASTL_EXPECTED_GCC54) && !defined(ASTL_EXPECTED_GCC55)
    template <class F> ASTL_EXPECTED_11_CONSTEXPR auto map(F &&f) & {
    return expected_map_impl(*this, eastl::forward<F>(f));
  }
  template <class F> ASTL_EXPECTED_11_CONSTEXPR auto map(F &&f) && {
    return expected_map_impl(eastl::move(*this), eastl::forward<F>(f));
  }
  template <class F> constexpr auto map(F &&f) const & {
    return expected_map_impl(*this, eastl::forward<F>(f));
  }
  template <class F> constexpr auto map(F &&f) const && {
    return expected_map_impl(eastl::move(*this), eastl::forward<F>(f));
  }
#else
  template <class F>
  ASTL_EXPECTED_11_CONSTEXPR decltype(
      expected_map_impl(eastl::declval<expected &>(), eastl::declval<F &&>()))
  map(F &&f) & {
    return expected_map_impl(*this, eastl::forward<F>(f));
  }
  template <class F>
  ASTL_EXPECTED_11_CONSTEXPR decltype(
      expected_map_impl(eastl::declval<expected>(), eastl::declval<F &&>()))
  map(F &&f) && {
    return expected_map_impl(eastl::move(*this), eastl::forward<F>(f));
  }
  template <class F>
  constexpr decltype(expected_map_impl(eastl::declval<const expected &>(),
                                       eastl::declval<F &&>()))
  map(F &&f) const & {
    return expected_map_impl(*this, eastl::forward<F>(f));
  }

#ifndef ASTL_EXPECTED_NO_CONSTRR
  template <class F>
  constexpr decltype(expected_map_impl(eastl::declval<const expected &&>(),
                                       eastl::declval<F &&>()))
  map(F &&f) const && {
    return expected_map_impl(eastl::move(*this), eastl::forward<F>(f));
  }
#endif
#endif

#if defined(ASTL_EXPECTED_CXX14) && !defined(ASTL_EXPECTED_GCC49) &&               \
    !defined(ASTL_EXPECTED_GCC54) && !defined(ASTL_EXPECTED_GCC55)
    template <class F> ASTL_EXPECTED_11_CONSTEXPR auto transform(F &&f) & {
    return expected_map_impl(*this, eastl::forward<F>(f));
  }
  template <class F> ASTL_EXPECTED_11_CONSTEXPR auto transform(F &&f) && {
    return expected_map_impl(eastl::move(*this), eastl::forward<F>(f));
  }
  template <class F> constexpr auto transform(F &&f) const & {
    return expected_map_impl(*this, eastl::forward<F>(f));
  }
  template <class F> constexpr auto transform(F &&f) const && {
    return expected_map_impl(eastl::move(*this), eastl::forward<F>(f));
  }
#else
    template <class F>
  ASTL_EXPECTED_11_CONSTEXPR decltype(
      expected_map_impl(eastl::declval<expected &>(), eastl::declval<F &&>()))
  transform(F &&f) & {
    return expected_map_impl(*this, eastl::forward<F>(f));
  }
  template <class F>
  ASTL_EXPECTED_11_CONSTEXPR decltype(
      expected_map_impl(eastl::declval<expected>(), eastl::declval<F &&>()))
  transform(F &&f) && {
    return expected_map_impl(eastl::move(*this), eastl::forward<F>(f));
  }
  template <class F>
  constexpr decltype(expected_map_impl(eastl::declval<const expected &>(),
                                       eastl::declval<F &&>()))
  transform(F &&f) const & {
    return expected_map_impl(*this, eastl::forward<F>(f));
  }

#ifndef ASTL_EXPECTED_NO_CONSTRR
  template <class F>
  constexpr decltype(expected_map_impl(eastl::declval<const expected &&>(),
                                       eastl::declval<F &&>()))
  transform(F &&f) const && {
    return expected_map_impl(eastl::move(*this), eastl::forward<F>(f));
  }
#endif
#endif

#if defined(ASTL_EXPECTED_CXX14) && !defined(ASTL_EXPECTED_GCC49) &&               \
    !defined(ASTL_EXPECTED_GCC54) && !defined(ASTL_EXPECTED_GCC55)
  template <class F> ASTL_EXPECTED_11_CONSTEXPR auto map_error(F &&f) & {
    return map_error_impl(*this, eastl::forward<F>(f));
  }
  template <class F> ASTL_EXPECTED_11_CONSTEXPR auto map_error(F &&f) && {
    return map_error_impl(eastl::move(*this), eastl::forward<F>(f));
  }
  template <class F> constexpr auto map_error(F &&f) const & {
    return map_error_impl(*this, eastl::forward<F>(f));
  }
  template <class F> constexpr auto map_error(F &&f) const && {
    return map_error_impl(eastl::move(*this), eastl::forward<F>(f));
  }
#else
  template <class F>
  ASTL_EXPECTED_11_CONSTEXPR decltype(map_error_impl(eastl::declval<expected &>(),
                                                   eastl::declval<F &&>()))
  map_error(F &&f) & {
    return map_error_impl(*this, eastl::forward<F>(f));
  }
  template <class F>
  ASTL_EXPECTED_11_CONSTEXPR decltype(map_error_impl(eastl::declval<expected &&>(),
                                                   eastl::declval<F &&>()))
  map_error(F &&f) && {
    return map_error_impl(eastl::move(*this), eastl::forward<F>(f));
  }
  template <class F>
  constexpr decltype(map_error_impl(eastl::declval<const expected &>(),
                                    eastl::declval<F &&>()))
  map_error(F &&f) const & {
    return map_error_impl(*this, eastl::forward<F>(f));
  }

#ifndef ASTL_EXPECTED_NO_CONSTRR
  template <class F>
  constexpr decltype(map_error_impl(eastl::declval<const expected &&>(),
                                    eastl::declval<F &&>()))
  map_error(F &&f) const && {
    return map_error_impl(eastl::move(*this), eastl::forward<F>(f));
  }
#endif
#endif
  template <class F> expected ASTL_EXPECTED_11_CONSTEXPR or_else(F &&f) & {
    return or_else_impl(*this, eastl::forward<F>(f));
  }

  template <class F> expected ASTL_EXPECTED_11_CONSTEXPR or_else(F &&f) && {
    return or_else_impl(eastl::move(*this), eastl::forward<F>(f));
  }

  template <class F> expected constexpr or_else(F &&f) const & {
    return or_else_impl(*this, eastl::forward<F>(f));
  }

#ifndef ASTL_EXPECTED_NO_CONSTRR
  template <class F> expected constexpr or_else(F &&f) const && {
    return or_else_impl(eastl::move(*this), eastl::forward<F>(f));
  }
#endif
  constexpr expected() = default;
  constexpr expected(const expected &rhs) = default;
  constexpr expected(expected &&rhs) = default;
  expected &operator=(const expected &rhs) = default;
  expected &operator=(expected &&rhs) = default;

  template <class... Args,
            detail::enable_if_t<eastl::is_constructible<T, Args &&...>::value> * =
                nullptr>
  constexpr expected(in_place_t, Args &&... args)
      : impl_base(in_place, eastl::forward<Args>(args)...),
        ctor_base(detail::default_constructor_tag{}) {}

  template <class U, class... Args,
            detail::enable_if_t<eastl::is_constructible<
                T, std::initializer_list<U> &, Args &&...>::value> * = nullptr>
  constexpr expected(in_place_t, std::initializer_list<U> il, Args &&... args)
      : impl_base(in_place, il, eastl::forward<Args>(args)...),
        ctor_base(detail::default_constructor_tag{}) {}

  template <class G = E,
            detail::enable_if_t<eastl::is_constructible<E, const G &>::value> * =
                nullptr,
            detail::enable_if_t<!eastl::is_convertible<const G &, E>::value> * =
                nullptr>
  explicit constexpr expected(const unexpected<G> &e)
      : impl_base(unexpect, e.value()),
        ctor_base(detail::default_constructor_tag{}) {}

    template <
      class G = E,
      detail::enable_if_t<eastl::is_constructible<E, const G &>::value> * =
          nullptr,
      detail::enable_if_t<eastl::is_convertible<const G &, E>::value> * = nullptr>
  constexpr expected(unexpected<G> const &e)
      : impl_base(unexpect, e.value()),
        ctor_base(detail::default_constructor_tag{}) {}

  template <
      class G = E,
      detail::enable_if_t<eastl::is_constructible<E, G &&>::value> * = nullptr,
      detail::enable_if_t<!eastl::is_convertible<G &&, E>::value> * = nullptr>
  explicit constexpr expected(unexpected<G> &&e) noexcept(
      eastl::is_nothrow_constructible<E, G &&>::value)
      : impl_base(unexpect, eastl::move(e.value())),
        ctor_base(detail::default_constructor_tag{}) {}

    template <
      class G = E,
      detail::enable_if_t<eastl::is_constructible<E, G &&>::value> * = nullptr,
      detail::enable_if_t<eastl::is_convertible<G &&, E>::value> * = nullptr>
  constexpr expected(unexpected<G> &&e) noexcept(
      eastl::is_nothrow_constructible<E, G &&>::value)
      : impl_base(unexpect, eastl::move(e.value())),
        ctor_base(detail::default_constructor_tag{}) {}

  template <class... Args,
            detail::enable_if_t<eastl::is_constructible<E, Args &&...>::value> * =
                nullptr>
  constexpr explicit expected(unexpect_t, Args &&... args)
      : impl_base(unexpect, eastl::forward<Args>(args)...),
        ctor_base(detail::default_constructor_tag{}) {}

    template <class U, class... Args,
            detail::enable_if_t<eastl::is_constructible<
                E, std::initializer_list<U> &, Args &&...>::value> * = nullptr>
  constexpr explicit expected(unexpect_t, std::initializer_list<U> il,
                              Args &&... args)
      : impl_base(unexpect, il, eastl::forward<Args>(args)...),
        ctor_base(detail::default_constructor_tag{}) {}

  template <class U, class G,
            detail::enable_if_t<!(eastl::is_convertible<U const &, T>::value &&
                                  eastl::is_convertible<G const &, E>::value)> * =
                nullptr,
            detail::expected_enable_from_other<T, E, U, G, const U &, const G &>
                * = nullptr>
  explicit ASTL_EXPECTED_11_CONSTEXPR expected(const expected<U, G> &rhs)
      : ctor_base(detail::default_constructor_tag{}) {
    if (rhs.has_value()) {
      this->construct(*rhs);
    } else {
      this->construct_error(rhs.error());
    }
  }

    template <class U, class G,
            detail::enable_if_t<(eastl::is_convertible<U const &, T>::value &&
                                 eastl::is_convertible<G const &, E>::value)> * =
                nullptr,
            detail::expected_enable_from_other<T, E, U, G, const U &, const G &>
                * = nullptr>
  ASTL_EXPECTED_11_CONSTEXPR expected(const expected<U, G> &rhs)
      : ctor_base(detail::default_constructor_tag{}) {
    if (rhs.has_value()) {
      this->construct(*rhs);
    } else {
      this->construct_error(rhs.error());
    }
  }

  template <
      class U, class G,
      detail::enable_if_t<!(eastl::is_convertible<U &&, T>::value &&
                            eastl::is_convertible<G &&, E>::value)> * = nullptr,
      detail::expected_enable_from_other<T, E, U, G, U &&, G &&> * = nullptr>
  explicit ASTL_EXPECTED_11_CONSTEXPR expected(expected<U, G> &&rhs)
      : ctor_base(detail::default_constructor_tag{}) {
    if (rhs.has_value()) {
      this->construct(eastl::move(*rhs));
    } else {
      this->construct_error(eastl::move(rhs.error()));
    }
  }

    template <
      class U, class G,
      detail::enable_if_t<(eastl::is_convertible<U &&, T>::value &&
                           eastl::is_convertible<G &&, E>::value)> * = nullptr,
      detail::expected_enable_from_other<T, E, U, G, U &&, G &&> * = nullptr>
  ASTL_EXPECTED_11_CONSTEXPR expected(expected<U, G> &&rhs)
      : ctor_base(detail::default_constructor_tag{}) {
    if (rhs.has_value()) {
      this->construct(eastl::move(*rhs));
    } else {
      this->construct_error(eastl::move(rhs.error()));
    }
  }

  template <
      class U = T,
      detail::enable_if_t<!eastl::is_convertible<U &&, T>::value> * = nullptr,
      detail::expected_enable_forward_value<T, E, U> * = nullptr>
  explicit ASTL_EXPECTED_MSVC2015_CONSTEXPR expected(U &&v)
      : expected(in_place, eastl::forward<U>(v)) {}

    template <
      class U = T,
      detail::enable_if_t<eastl::is_convertible<U &&, T>::value> * = nullptr,
      detail::expected_enable_forward_value<T, E, U> * = nullptr>
  ASTL_EXPECTED_MSVC2015_CONSTEXPR expected(U &&v)
      : expected(in_place, eastl::forward<U>(v)) {}

  template <
      class U = T, class G = T,
      detail::enable_if_t<eastl::is_nothrow_constructible<T, U &&>::value> * =
          nullptr,
      detail::enable_if_t<!eastl::is_void<G>::value> * = nullptr,
      detail::enable_if_t<
          (!eastl::is_same<expected<T, E>, detail::decay_t<U>>::value &&
           !detail::conjunction<eastl::is_scalar<T>,
                                eastl::is_same<T, detail::decay_t<U>>>::value &&
           eastl::is_constructible<T, U>::value &&
           eastl::is_assignable<G &, U>::value &&
           eastl::is_nothrow_move_constructible<E>::value)> * = nullptr>
  expected &operator=(U &&v) {
    if (has_value()) {
      val() = eastl::forward<U>(v);
    } else {
      err().~unexpected<E>();
      ::new (valptr()) T(eastl::forward<U>(v));
      this->m_has_val = true;
    }

    return *this;
  }

    template <
      class U = T, class G = T,
      detail::enable_if_t<!eastl::is_nothrow_constructible<T, U &&>::value> * =
          nullptr,
      detail::enable_if_t<!eastl::is_void<U>::value> * = nullptr,
      detail::enable_if_t<
          (!eastl::is_same<expected<T, E>, detail::decay_t<U>>::value &&
           !detail::conjunction<eastl::is_scalar<T>,
                                eastl::is_same<T, detail::decay_t<U>>>::value &&
           eastl::is_constructible<T, U>::value &&
           eastl::is_assignable<G &, U>::value &&
           eastl::is_nothrow_move_constructible<E>::value)> * = nullptr>
  expected &operator=(U &&v) {
    if (has_value()) {
      val() = eastl::forward<U>(v);
    } else {
      auto tmp = eastl::move(err());
      err().~unexpected<E>();

      #ifdef ASTL_EXPECTED_EXCEPTIONS_ENABLED
      try {
        ::new (valptr()) T(eastl::forward<U>(v));
        this->m_has_val = true;
      } catch (...) {
        err() = eastl::move(tmp);
        throw;
      }
      #else
        ::new (valptr()) T(eastl::forward<U>(v));
        this->m_has_val = true;
      #endif
    }

    return *this;
  }

  template <class G = E,
            detail::enable_if_t<eastl::is_nothrow_copy_constructible<G>::value &&
                                eastl::is_assignable<G &, G>::value> * = nullptr>
  expected &operator=(const unexpected<G> &rhs) {
    if (!has_value()) {
      err() = rhs;
    } else {
      this->destroy_val();
      ::new (errptr()) unexpected<E>(rhs);
      this->m_has_val = false;
    }

    return *this;
  }

  template <class G = E,
            detail::enable_if_t<eastl::is_nothrow_move_constructible<G>::value &&
                                eastl::is_move_assignable<G>::value> * = nullptr>
  expected &operator=(unexpected<G> &&rhs) noexcept {
    if (!has_value()) {
      err() = eastl::move(rhs);
    } else {
      this->destroy_val();
      ::new (errptr()) unexpected<E>(eastl::move(rhs));
      this->m_has_val = false;
    }

    return *this;
  }

  template <class... Args, detail::enable_if_t<eastl::is_nothrow_constructible<
                               T, Args &&...>::value> * = nullptr>
  void emplace(Args &&... args) {
    if (has_value()) {
      val() = T(eastl::forward<Args>(args)...);
    } else {
      err().~unexpected<E>();
      ::new (valptr()) T(eastl::forward<Args>(args)...);
      this->m_has_val = true;
    }
  }

    template <class... Args, detail::enable_if_t<!eastl::is_nothrow_constructible<
                               T, Args &&...>::value> * = nullptr>
  void emplace(Args &&... args) {
    if (has_value()) {
      val() = T(eastl::forward<Args>(args)...);
    } else {
      auto tmp = eastl::move(err());
      err().~unexpected<E>();

      #ifdef ASTL_EXPECTED_EXCEPTIONS_ENABLED
      try {
        ::new (valptr()) T(eastl::forward<Args>(args)...);
        this->m_has_val = true;
      } catch (...) {
        err() = eastl::move(tmp);
        throw;
      }
      #else
      ::new (valptr()) T(eastl::forward<Args>(args)...);
      this->m_has_val = true;
      #endif
    }
  }

  template <class U, class... Args,
            detail::enable_if_t<eastl::is_nothrow_constructible<
                T, std::initializer_list<U> &, Args &&...>::value> * = nullptr>
  void emplace(std::initializer_list<U> il, Args &&... args) {
    if (has_value()) {
      T t(il, eastl::forward<Args>(args)...);
      val() = eastl::move(t);
    } else {
      err().~unexpected<E>();
      ::new (valptr()) T(il, eastl::forward<Args>(args)...);
      this->m_has_val = true;
    }
  }

    template <class U, class... Args,
            detail::enable_if_t<!eastl::is_nothrow_constructible<
                T, std::initializer_list<U> &, Args &&...>::value> * = nullptr>
  void emplace(std::initializer_list<U> il, Args &&... args) {
    if (has_value()) {
      T t(il, eastl::forward<Args>(args)...);
      val() = eastl::move(t);
    } else {
      auto tmp = eastl::move(err());
      err().~unexpected<E>();

      #ifdef ASTL_EXPECTED_EXCEPTIONS_ENABLED
      try {
        ::new (valptr()) T(il, eastl::forward<Args>(args)...);
        this->m_has_val = true;
      } catch (...) {
        err() = eastl::move(tmp);
        throw;
      }
      #else
      ::new (valptr()) T(il, eastl::forward<Args>(args)...);
      this->m_has_val = true;
      #endif
    }
  }

private:
  using t_is_void = eastl::true_type;
  using t_is_not_void = eastl::false_type;
  using t_is_nothrow_move_constructible = eastl::true_type;
  using move_constructing_t_can_throw = eastl::false_type;
  using e_is_nothrow_move_constructible = eastl::true_type;
  using move_constructing_e_can_throw = eastl::false_type;

  void swap_where_both_have_value(expected &/*rhs*/ , t_is_void) noexcept {
    // swapping void is a no-op
  }

  void swap_where_both_have_value(expected &rhs, t_is_not_void) {
    using eastl::swap;
    swap(val(), rhs.val());
  }

  void swap_where_only_one_has_value(expected &rhs, t_is_void) noexcept(
      eastl::is_nothrow_move_constructible<E>::value) {
    ::new (errptr()) unexpected_type(eastl::move(rhs.err()));
    rhs.err().~unexpected_type();
    eastl::swap(this->m_has_val, rhs.m_has_val);
  }

  void swap_where_only_one_has_value(expected &rhs, t_is_not_void) {
    swap_where_only_one_has_value_and_t_is_not_void(
        rhs, typename eastl::is_nothrow_move_constructible<T>::type{},
        typename eastl::is_nothrow_move_constructible<E>::type{});
  }

  void swap_where_only_one_has_value_and_t_is_not_void(
      expected &rhs, t_is_nothrow_move_constructible,
      e_is_nothrow_move_constructible) noexcept {
    auto temp = eastl::move(val());
    val().~T();
    ::new (errptr()) unexpected_type(eastl::move(rhs.err()));
    rhs.err().~unexpected_type();
    ::new (rhs.valptr()) T(eastl::move(temp));
    eastl::swap(this->m_has_val, rhs.m_has_val);
  }

  void swap_where_only_one_has_value_and_t_is_not_void(
      expected &rhs, t_is_nothrow_move_constructible,
      move_constructing_e_can_throw) {
    auto temp = eastl::move(val());
    val().~T();
#ifdef ASTL_EXPECTED_EXCEPTIONS_ENABLED
    try {
      ::new (errptr()) unexpected_type(eastl::move(rhs.err()));
      rhs.err().~unexpected_type();
      ::new (rhs.valptr()) T(eastl::move(temp));
      eastl::swap(this->m_has_val, rhs.m_has_val);
    } catch (...) {
      val() = eastl::move(temp);
      throw;
    }
#else
    ::new (errptr()) unexpected_type(eastl::move(rhs.err()));
    rhs.err().~unexpected_type();
    ::new (rhs.valptr()) T(eastl::move(temp));
    eastl::swap(this->m_has_val, rhs.m_has_val);
#endif
  }

  void swap_where_only_one_has_value_and_t_is_not_void(
      expected &rhs, move_constructing_t_can_throw,
      t_is_nothrow_move_constructible) {
    auto temp = eastl::move(rhs.err());
    rhs.err().~unexpected_type();
#ifdef ASTL_EXPECTED_EXCEPTIONS_ENABLED
    try {
      ::new (rhs.valptr()) T(val());
      val().~T();
      ::new (errptr()) unexpected_type(eastl::move(temp));
      eastl::swap(this->m_has_val, rhs.m_has_val);
    } catch (...) {
      rhs.err() = eastl::move(temp);
      throw;
    }
#else
    ::new (rhs.valptr()) T(val());
    val().~T();
    ::new (errptr()) unexpected_type(eastl::move(temp));
    eastl::swap(this->m_has_val, rhs.m_has_val);
#endif
  }

public:
  template <class OT = T, class OE = E>
  detail::enable_if_t<detail::is_swappable<OT>::value &&
                      detail::is_swappable<OE>::value &&
                      (eastl::is_nothrow_move_constructible<OT>::value ||
                       eastl::is_nothrow_move_constructible<OE>::value)>
  swap(expected &rhs) noexcept(
      eastl::is_nothrow_move_constructible<T>::value
          &&detail::is_nothrow_swappable<T>::value
              &&eastl::is_nothrow_move_constructible<E>::value
                  &&detail::is_nothrow_swappable<E>::value) {
    if (has_value() && rhs.has_value()) {
      swap_where_both_have_value(rhs, typename eastl::is_void<T>::type{});
    } else if (!has_value() && rhs.has_value()) {
      rhs.swap(*this);
    } else if (has_value()) {
      swap_where_only_one_has_value(rhs, typename eastl::is_void<T>::type{});
    } else {
      using eastl::swap;
      swap(err(), rhs.err());
    }
  }

  constexpr const T *operator->() const { return valptr(); }
  ASTL_EXPECTED_11_CONSTEXPR T *operator->() { return valptr(); }

  template <class U = T,
            detail::enable_if_t<!eastl::is_void<U>::value> * = nullptr>
  constexpr const U &operator*() const & {
    return val();
  }
  template <class U = T,
            detail::enable_if_t<!eastl::is_void<U>::value> * = nullptr>
  ASTL_EXPECTED_11_CONSTEXPR U &operator*() & {
    return val();
  }
  template <class U = T,
            detail::enable_if_t<!eastl::is_void<U>::value> * = nullptr>
  constexpr const U &&operator*() const && {
    return eastl::move(val());
  }
  template <class U = T,
            detail::enable_if_t<!eastl::is_void<U>::value> * = nullptr>
  ASTL_EXPECTED_11_CONSTEXPR U &&operator*() && {
    return eastl::move(val());
  }

  constexpr bool has_value() const noexcept { return this->m_has_val; }
  constexpr explicit operator bool() const noexcept { return this->m_has_val; }

  template <class U = T,
            detail::enable_if_t<!eastl::is_void<U>::value> * = nullptr>
  ASTL_EXPECTED_11_CONSTEXPR const U &value() const & {
    if (!has_value())
      detail::throw_exception(bad_expected_access<E>(err().value()));
    return val();
  }
  template <class U = T,
            detail::enable_if_t<!eastl::is_void<U>::value> * = nullptr>
  ASTL_EXPECTED_11_CONSTEXPR U &value() & {
    if (!has_value())
      detail::throw_exception(bad_expected_access<E>(err().value()));
    return val();
  }
  template <class U = T,
            detail::enable_if_t<!eastl::is_void<U>::value> * = nullptr>
  ASTL_EXPECTED_11_CONSTEXPR const U &&value() const && {
    if (!has_value())
      detail::throw_exception(bad_expected_access<E>(eastl::move(err()).value()));
    return eastl::move(val());
  }
  template <class U = T,
            detail::enable_if_t<!eastl::is_void<U>::value> * = nullptr>
  ASTL_EXPECTED_11_CONSTEXPR U &&value() && {
    if (!has_value())
      detail::throw_exception(bad_expected_access<E>(eastl::move(err()).value()));
    return eastl::move(val());
  }

  constexpr const E &error() const & { return err().value(); }
  ASTL_EXPECTED_11_CONSTEXPR E &error() & { return err().value(); }
  constexpr const E &&error() const && { return eastl::move(err().value()); }
  ASTL_EXPECTED_11_CONSTEXPR E &&error() && { return eastl::move(err().value()); }

  template <class U> constexpr T value_or(U &&v) const & {
    static_assert(eastl::is_copy_constructible<T>::value &&
                      eastl::is_convertible<U &&, T>::value,
                  "T must be copy-constructible and convertible to from U&&");
    return bool(*this) ? **this : static_cast<T>(eastl::forward<U>(v));
  }
  template <class U> ASTL_EXPECTED_11_CONSTEXPR T value_or(U &&v) && {
    static_assert(eastl::is_move_constructible<T>::value &&
                      eastl::is_convertible<U &&, T>::value,
                  "T must be move-constructible and convertible to from U&&");
    return bool(*this) ? eastl::move(**this) : static_cast<T>(eastl::forward<U>(v));
  }

  constexpr T value_or_fn(astl::function_ref<T()> fn) const& {
    return bool(*this) ? **this : fn();
  }

  ASTL_EXPECTED_11_CONSTEXPR T value_or_fn(astl::function_ref<T()> fn) && {
    return bool(*this) ? eastl::move(**this) : fn();
  }
};

namespace detail {
template <class Exp> using exp_t = typename detail::decay_t<Exp>::value_type;
template <class Exp> using err_t = typename detail::decay_t<Exp>::error_type;
template <class Exp, class Ret> using ret_t = expected<Ret, err_t<Exp>>;

#ifdef ASTL_EXPECTED_CXX14
template <class Exp, class F,
          detail::enable_if_t<!eastl::is_void<exp_t<Exp>>::value> * = nullptr,
          class Ret = decltype(detail::invoke(eastl::declval<F>(),
                                              *eastl::declval<Exp>()))>
constexpr auto and_then_impl(Exp &&exp, F &&f) {
  static_assert(detail::is_expected<Ret>::value, "F must return an expected");

  return exp.has_value()
             ? detail::invoke(eastl::forward<F>(f), *eastl::forward<Exp>(exp))
             : Ret(unexpect, eastl::forward<Exp>(exp).error());
}

template <class Exp, class F,
          detail::enable_if_t<eastl::is_void<exp_t<Exp>>::value> * = nullptr,
          class Ret = decltype(detail::invoke(eastl::declval<F>()))>
constexpr auto and_then_impl(Exp &&exp, F &&f) {
  static_assert(detail::is_expected<Ret>::value, "F must return an expected");

  return exp.has_value() ? detail::invoke(eastl::forward<F>(f))
                         : Ret(unexpect, eastl::forward<Exp>(exp).error());
}
#else
template <class> struct TC;
template <class Exp, class F,
          class Ret = decltype(detail::invoke(eastl::declval<F>(),
                                              *eastl::declval<Exp>())),
          detail::enable_if_t<!eastl::is_void<exp_t<Exp>>::value> * = nullptr>
auto and_then_impl(Exp &&exp, F &&f) -> Ret {
  static_assert(detail::is_expected<Ret>::value, "F must return an expected");

  return exp.has_value()
             ? detail::invoke(eastl::forward<F>(f), *eastl::forward<Exp>(exp))
             : Ret(unexpect, eastl::forward<Exp>(exp).error());
}

template <class Exp, class F,
          class Ret = decltype(detail::invoke(eastl::declval<F>())),
          detail::enable_if_t<eastl::is_void<exp_t<Exp>>::value> * = nullptr>
constexpr auto and_then_impl(Exp &&exp, F &&f) -> Ret {
  static_assert(detail::is_expected<Ret>::value, "F must return an expected");

  return exp.has_value() ? detail::invoke(eastl::forward<F>(f))
                         : Ret(unexpect, eastl::forward<Exp>(exp).error());
}
#endif

#ifdef ASTL_EXPECTED_CXX14
template <class Exp, class F,
          detail::enable_if_t<!eastl::is_void<exp_t<Exp>>::value> * = nullptr,
          class Ret = decltype(detail::invoke(eastl::declval<F>(),
                                              *eastl::declval<Exp>())),
          detail::enable_if_t<!eastl::is_void<Ret>::value> * = nullptr>
constexpr auto expected_map_impl(Exp &&exp, F &&f) {
  using result = ret_t<Exp, detail::decay_t<Ret>>;
  return exp.has_value() ? result(detail::invoke(eastl::forward<F>(f),
                                                 *eastl::forward<Exp>(exp)))
                         : result(unexpect, eastl::forward<Exp>(exp).error());
}

template <class Exp, class F,
          detail::enable_if_t<!eastl::is_void<exp_t<Exp>>::value> * = nullptr,
          class Ret = decltype(detail::invoke(eastl::declval<F>(),
                                              *eastl::declval<Exp>())),
          detail::enable_if_t<eastl::is_void<Ret>::value> * = nullptr>
auto expected_map_impl(Exp &&exp, F &&f) {
  using result = expected<void, err_t<Exp>>;
  if (exp.has_value()) {
    detail::invoke(eastl::forward<F>(f), *eastl::forward<Exp>(exp));
    return result();
  }

  return result(unexpect, eastl::forward<Exp>(exp).error());
}

template <class Exp, class F,
          detail::enable_if_t<eastl::is_void<exp_t<Exp>>::value> * = nullptr,
          class Ret = decltype(detail::invoke(eastl::declval<F>())),
          detail::enable_if_t<!eastl::is_void<Ret>::value> * = nullptr>
constexpr auto expected_map_impl(Exp &&exp, F &&f) {
  using result = ret_t<Exp, detail::decay_t<Ret>>;
  return exp.has_value() ? result(detail::invoke(eastl::forward<F>(f)))
                         : result(unexpect, eastl::forward<Exp>(exp).error());
}

template <class Exp, class F,
          detail::enable_if_t<eastl::is_void<exp_t<Exp>>::value> * = nullptr,
          class Ret = decltype(detail::invoke(eastl::declval<F>())),
          detail::enable_if_t<eastl::is_void<Ret>::value> * = nullptr>
auto expected_map_impl(Exp &&exp, F &&f) {
  using result = expected<void, err_t<Exp>>;
  if (exp.has_value()) {
    detail::invoke(eastl::forward<F>(f));
    return result();
  }

  return result(unexpect, eastl::forward<Exp>(exp).error());
}
#else
template <class Exp, class F,
          detail::enable_if_t<!eastl::is_void<exp_t<Exp>>::value> * = nullptr,
          class Ret = decltype(detail::invoke(eastl::declval<F>(),
                                              *eastl::declval<Exp>())),
          detail::enable_if_t<!eastl::is_void<Ret>::value> * = nullptr>

constexpr auto expected_map_impl(Exp &&exp, F &&f)
    -> ret_t<Exp, detail::decay_t<Ret>> {
  using result = ret_t<Exp, detail::decay_t<Ret>>;

  return exp.has_value() ? result(detail::invoke(eastl::forward<F>(f),
                                                 *eastl::forward<Exp>(exp)))
                         : result(unexpect, eastl::forward<Exp>(exp).error());
}

template <class Exp, class F,
          detail::enable_if_t<!eastl::is_void<exp_t<Exp>>::value> * = nullptr,
          class Ret = decltype(detail::invoke(eastl::declval<F>(),
                                              *eastl::declval<Exp>())),
          detail::enable_if_t<eastl::is_void<Ret>::value> * = nullptr>

auto expected_map_impl(Exp &&exp, F &&f) -> expected<void, err_t<Exp>> {
  if (exp.has_value()) {
    detail::invoke(eastl::forward<F>(f), *eastl::forward<Exp>(exp));
    return {};
  }

  return unexpected<err_t<Exp>>(eastl::forward<Exp>(exp).error());
}

template <class Exp, class F,
          detail::enable_if_t<eastl::is_void<exp_t<Exp>>::value> * = nullptr,
          class Ret = decltype(detail::invoke(eastl::declval<F>())),
          detail::enable_if_t<!eastl::is_void<Ret>::value> * = nullptr>

constexpr auto expected_map_impl(Exp &&exp, F &&f)
    -> ret_t<Exp, detail::decay_t<Ret>> {
  using result = ret_t<Exp, detail::decay_t<Ret>>;

  return exp.has_value() ? result(detail::invoke(eastl::forward<F>(f)))
                         : result(unexpect, eastl::forward<Exp>(exp).error());
}

template <class Exp, class F,
          detail::enable_if_t<eastl::is_void<exp_t<Exp>>::value> * = nullptr,
          class Ret = decltype(detail::invoke(eastl::declval<F>())),
          detail::enable_if_t<eastl::is_void<Ret>::value> * = nullptr>

auto expected_map_impl(Exp &&exp, F &&f) -> expected<void, err_t<Exp>> {
  if (exp.has_value()) {
    detail::invoke(eastl::forward<F>(f));
    return {};
  }

  return unexpected<err_t<Exp>>(eastl::forward<Exp>(exp).error());
}
#endif

#if defined(ASTL_EXPECTED_CXX14) && !defined(ASTL_EXPECTED_GCC49) &&               \
    !defined(ASTL_EXPECTED_GCC54) && !defined(ASTL_EXPECTED_GCC55)
template <class Exp, class F,
          detail::enable_if_t<!eastl::is_void<exp_t<Exp>>::value> * = nullptr,
          class Ret = decltype(detail::invoke(eastl::declval<F>(),
                                              eastl::declval<Exp>().error())),
          detail::enable_if_t<!eastl::is_void<Ret>::value> * = nullptr>
constexpr auto map_error_impl(Exp &&exp, F &&f) {
  using result = expected<exp_t<Exp>, detail::decay_t<Ret>>;
  return exp.has_value()
             ? result(*eastl::forward<Exp>(exp))
             : result(unexpect, detail::invoke(eastl::forward<F>(f),
                                               eastl::forward<Exp>(exp).error()));
}
template <class Exp, class F,
          detail::enable_if_t<!eastl::is_void<exp_t<Exp>>::value> * = nullptr,
          class Ret = decltype(detail::invoke(eastl::declval<F>(),
                                              eastl::declval<Exp>().error())),
          detail::enable_if_t<eastl::is_void<Ret>::value> * = nullptr>
auto map_error_impl(Exp &&exp, F &&f) {
  using result = expected<exp_t<Exp>, monostate>;
  if (exp.has_value()) {
    return result(*eastl::forward<Exp>(exp));
  }

  detail::invoke(eastl::forward<F>(f), eastl::forward<Exp>(exp).error());
  return result(unexpect, monostate{});
}
template <class Exp, class F,
          detail::enable_if_t<eastl::is_void<exp_t<Exp>>::value> * = nullptr,
          class Ret = decltype(detail::invoke(eastl::declval<F>(),
                                              eastl::declval<Exp>().error())),
          detail::enable_if_t<!eastl::is_void<Ret>::value> * = nullptr>
constexpr auto map_error_impl(Exp &&exp, F &&f) {
  using result = expected<exp_t<Exp>, detail::decay_t<Ret>>;
  return exp.has_value()
             ? result()
             : result(unexpect, detail::invoke(eastl::forward<F>(f),
                                               eastl::forward<Exp>(exp).error()));
}
template <class Exp, class F,
          detail::enable_if_t<eastl::is_void<exp_t<Exp>>::value> * = nullptr,
          class Ret = decltype(detail::invoke(eastl::declval<F>(),
                                              eastl::declval<Exp>().error())),
          detail::enable_if_t<eastl::is_void<Ret>::value> * = nullptr>
auto map_error_impl(Exp &&exp, F &&f) {
  using result = expected<exp_t<Exp>, monostate>;
  if (exp.has_value()) {
    return result();
  }

  detail::invoke(eastl::forward<F>(f), eastl::forward<Exp>(exp).error());
  return result(unexpect, monostate{});
}
#else
template <class Exp, class F,
          detail::enable_if_t<!eastl::is_void<exp_t<Exp>>::value> * = nullptr,
          class Ret = decltype(detail::invoke(eastl::declval<F>(),
                                              eastl::declval<Exp>().error())),
          detail::enable_if_t<!eastl::is_void<Ret>::value> * = nullptr>
constexpr auto map_error_impl(Exp &&exp, F &&f)
    -> expected<exp_t<Exp>, detail::decay_t<Ret>> {
  using result = expected<exp_t<Exp>, detail::decay_t<Ret>>;

  return exp.has_value()
             ? result(*eastl::forward<Exp>(exp))
             : result(unexpect, detail::invoke(eastl::forward<F>(f),
                                               eastl::forward<Exp>(exp).error()));
}

template <class Exp, class F,
          detail::enable_if_t<!eastl::is_void<exp_t<Exp>>::value> * = nullptr,
          class Ret = decltype(detail::invoke(eastl::declval<F>(),
                                              eastl::declval<Exp>().error())),
          detail::enable_if_t<eastl::is_void<Ret>::value> * = nullptr>
auto map_error_impl(Exp &&exp, F &&f) -> expected<exp_t<Exp>, monostate> {
  using result = expected<exp_t<Exp>, monostate>;
  if (exp.has_value()) {
    return result(*eastl::forward<Exp>(exp));
  }

  detail::invoke(eastl::forward<F>(f), eastl::forward<Exp>(exp).error());
  return result(unexpect, monostate{});
}

template <class Exp, class F,
          detail::enable_if_t<eastl::is_void<exp_t<Exp>>::value> * = nullptr,
          class Ret = decltype(detail::invoke(eastl::declval<F>(),
                                              eastl::declval<Exp>().error())),
          detail::enable_if_t<!eastl::is_void<Ret>::value> * = nullptr>
constexpr auto map_error_impl(Exp &&exp, F &&f)
    -> expected<exp_t<Exp>, detail::decay_t<Ret>> {
  using result = expected<exp_t<Exp>, detail::decay_t<Ret>>;

  return exp.has_value()
             ? result()
             : result(unexpect, detail::invoke(eastl::forward<F>(f),
                                               eastl::forward<Exp>(exp).error()));
}

template <class Exp, class F,
          detail::enable_if_t<eastl::is_void<exp_t<Exp>>::value> * = nullptr,
          class Ret = decltype(detail::invoke(eastl::declval<F>(),
                                              eastl::declval<Exp>().error())),
          detail::enable_if_t<eastl::is_void<Ret>::value> * = nullptr>
auto map_error_impl(Exp &&exp, F &&f) -> expected<exp_t<Exp>, monostate> {
  using result = expected<exp_t<Exp>, monostate>;
  if (exp.has_value()) {
    return result();
  }

  detail::invoke(eastl::forward<F>(f), eastl::forward<Exp>(exp).error());
  return result(unexpect, monostate{});
}
#endif

#ifdef ASTL_EXPECTED_CXX14
template <class Exp, class F,
          class Ret = decltype(detail::invoke(eastl::declval<F>(),
                                              eastl::declval<Exp>().error())),
          detail::enable_if_t<!eastl::is_void<Ret>::value> * = nullptr>
constexpr auto or_else_impl(Exp &&exp, F &&f) {
  static_assert(detail::is_expected<Ret>::value, "F must return an expected");
  return exp.has_value()
  ? eastl::forward<Exp>(exp)
  : detail::invoke(eastl::forward<F>(f), eastl::forward<Exp>(exp).error());
}

template <class Exp, class F,
          class Ret = decltype(detail::invoke(eastl::declval<F>(),
                                              eastl::declval<Exp>().error())),
          detail::enable_if_t<eastl::is_void<Ret>::value> * = nullptr>
detail::decay_t<Exp> or_else_impl(Exp &&exp, F &&f) {
  return exp.has_value()
  ? eastl::forward<Exp>(exp)
  : (detail::invoke(eastl::forward<F>(f), eastl::forward<Exp>(exp).error()),
    eastl::forward<Exp>(exp));
}
#else
template <class Exp, class F,
          class Ret = decltype(detail::invoke(eastl::declval<F>(),
                                              eastl::declval<Exp>().error())),
  detail::enable_if_t<!eastl::is_void<Ret>::value> * = nullptr>
auto or_else_impl(Exp &&exp, F &&f) -> Ret {
  static_assert(detail::is_expected<Ret>::value, "F must return an expected");
  return exp.has_value()
         ? eastl::forward<Exp>(exp)
         : detail::invoke(eastl::forward<F>(f), eastl::forward<Exp>(exp).error());
}

template <class Exp, class F,
          class Ret = decltype(detail::invoke(eastl::declval<F>(),
                                              eastl::declval<Exp>().error())),
  detail::enable_if_t<eastl::is_void<Ret>::value> * = nullptr>
detail::decay_t<Exp> or_else_impl(Exp &&exp, F &&f) {
  return exp.has_value()
         ? eastl::forward<Exp>(exp)
         : (detail::invoke(eastl::forward<F>(f), eastl::forward<Exp>(exp).error()),
            eastl::forward<Exp>(exp));
}
#endif
} // namespace detail

template <class T, class E, class U, class F>
constexpr bool operator==(const expected<T, E> &lhs,
                          const expected<U, F> &rhs) {
  return (lhs.has_value() != rhs.has_value())
             ? false
             : (!lhs.has_value() ? lhs.error() == rhs.error() : *lhs == *rhs);
}
template <class T, class E, class U, class F>
constexpr bool operator!=(const expected<T, E> &lhs,
                          const expected<U, F> &rhs) {
  return (lhs.has_value() != rhs.has_value())
             ? true
             : (!lhs.has_value() ? lhs.error() != rhs.error() : *lhs != *rhs);
}

template <class T, class E, class U>
constexpr bool operator==(const expected<T, E> &x, const U &v) {
  return x.has_value() ? *x == v : false;
}
template <class T, class E, class U>
constexpr bool operator==(const U &v, const expected<T, E> &x) {
  return x.has_value() ? *x == v : false;
}
template <class T, class E, class U>
constexpr bool operator!=(const expected<T, E> &x, const U &v) {
  return x.has_value() ? *x != v : true;
}
template <class T, class E, class U>
constexpr bool operator!=(const U &v, const expected<T, E> &x) {
  return x.has_value() ? *x != v : true;
}

template <class T, class E>
constexpr bool operator==(const expected<T, E> &x, const unexpected<E> &e) {
  return x.has_value() ? false : x.error() == e.value();
}
template <class T, class E>
constexpr bool operator==(const unexpected<E> &e, const expected<T, E> &x) {
  return x.has_value() ? false : x.error() == e.value();
}
template <class T, class E>
constexpr bool operator!=(const expected<T, E> &x, const unexpected<E> &e) {
  return x.has_value() ? true : x.error() != e.value();
}
template <class T, class E>
constexpr bool operator!=(const unexpected<E> &e, const expected<T, E> &x) {
  return x.has_value() ? true : x.error() != e.value();
}

template <class T, class E,
          detail::enable_if_t<(eastl::is_void<T>::value ||
                               eastl::is_move_constructible<T>::value) &&
                              detail::is_swappable<T>::value &&
                              eastl::is_move_constructible<E>::value &&
                              detail::is_swappable<E>::value> * = nullptr>
void swap(expected<T, E> &lhs,
          expected<T, E> &rhs) noexcept(noexcept(lhs.swap(rhs))) {
  lhs.swap(rhs);
}
} // namespace astl

#endif // __EXPECTED_H_B43C5B80_D5A3_154C_AC04_03564BD031EA__
