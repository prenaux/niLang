#pragma once
#ifndef __FUNCTION_REF_H_951DF2A9_291D_FF47_89B0_2223AC8B67C8__
#define __FUNCTION_REF_H_951DF2A9_291D_FF47_89B0_2223AC8B67C8__
//
// An implementation of function_ref:
//  "function_ref: a type-erased callable reference" https://wg21.link/P0792
//
// Original reference doc at:
// https://tl.tartanllama.xyz/en/latest/api/function_ref.html
//
// function_ref - A low-overhead non-owning function
// Written in 2017 by Simon Brand (@TartanLlama)
//
// To the extent possible under law, the author(s) have dedicated all
// copyright and related and neighboring rights to this software to the
// public domain worldwide. This software is distributed without any warranty.
//
// You should have received a copy of the CC0 Public Domain Dedication
// along with this software. If not, see
// <http://creativecommons.org/publicdomain/zero/1.0/>.
//

#define ASTL_FUNCTION_REF_VERSION_MAJOR 1
#define ASTL_FUNCTION_REF_VERSION_MINOR 0
#define ASTL_FUNCTION_REF_VERSION_PATCH 0

#if (defined(_MSC_VER) && _MSC_VER == 1900)
/// \exclude
#define ASTL_FUNCTION_REF_MSVC2015
#endif

#if (defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ <= 9 &&              \
     !defined(__clang__))
/// \exclude
#define ASTL_FUNCTION_REF_GCC49
#endif

#if (defined(__GNUC__) && __GNUC__ == 5 && __GNUC_MINOR__ <= 4 &&              \
     !defined(__clang__))
/// \exclude
#define ASTL_FUNCTION_REF_GCC54
#endif

#if (defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ <= 9 &&              \
     !defined(__clang__))
// GCC < 5 doesn't support overloading on const&& for member functions
/// \exclude
#define ASTL_FUNCTION_REF_NO_CONSTRR
#endif

#if __cplusplus > 201103L
/// \exclude
#define ASTL_FUNCTION_REF_CXX14
#endif

// constexpr implies const in C++11, not C++14
#if (__cplusplus == 201103L || defined(ASTL_FUNCTION_REF_MSVC2015) ||            \
     defined(ASTL_FUNCTION_REF_GCC49)) &&                                        \
    !defined(ASTL_FUNCTION_REF_GCC54)
/// \exclude
#define ASTL_FUNCTION_REF_11_CONSTEXPR
#else
/// \exclude
#define ASTL_FUNCTION_REF_11_CONSTEXPR constexpr
#endif

#include "EASTL/type_traits.h"
#include "EASTL/utility.h"

namespace astl {
namespace detail {
namespace fnref {
// C++14-style aliases for brevity
template <class T> using remove_const_t = typename eastl::remove_const<T>::type;
template <class T>
using remove_reference_t = typename eastl::remove_reference<T>::type;
template <class T> using decay_t = typename eastl::decay<T>::type;
template <bool E, class T = void>
using enable_if_t = typename eastl::enable_if<E, T>::type;
template <bool B, class T, class F>
using conditional_t = typename eastl::conditional<B, T, F>::type;

// eastl::invoke from C++17
// https://stackoverflow.com/questions/38288042/c11-14-invoke-workaround
template <typename Fn, typename... Args,
          typename = enable_if_t<eastl::is_member_pointer<decay_t<Fn>>::value>,
          int = 0>
constexpr auto invoke(Fn &&f, Args &&... args) noexcept(
    noexcept(eastl::mem_fn(f)(eastl::forward<Args>(args)...)))
    -> decltype(eastl::mem_fn(f)(eastl::forward<Args>(args)...)) {
  return eastl::mem_fn(f)(eastl::forward<Args>(args)...);
}

template <typename Fn, typename... Args,
          typename = enable_if_t<!eastl::is_member_pointer<decay_t<Fn>>{}>>
constexpr auto invoke(Fn &&f, Args &&... args) noexcept(
    noexcept(eastl::forward<Fn>(f)(eastl::forward<Args>(args)...)))
    -> decltype(eastl::forward<Fn>(f)(eastl::forward<Args>(args)...)) {
  return eastl::forward<Fn>(f)(eastl::forward<Args>(args)...);
}

// eastl::invoke_result from C++17
template <class F, class, class... Us> struct invoke_result_impl;

template <class F, class... Us>
struct invoke_result_impl<
    F, decltype(astl::detail::fnref::invoke(eastl::declval<F>(), eastl::declval<Us>()...), void()),
    Us...> {
  using type = decltype(astl::detail::fnref::invoke(eastl::declval<F>(), eastl::declval<Us>()...));
};

template <class F, class... Us>
using invoke_result = invoke_result_impl<F, void, Us...>;

template <class F, class... Us>
using invoke_result_t = typename invoke_result<F, Us...>::type;

template <class, class R, class F, class... Args>
struct is_invocable_r_impl : eastl::false_type {};

template <class R, class F, class... Args>
struct is_invocable_r_impl<
    typename eastl::is_convertible<invoke_result_t<F, Args...>, R>::type, R, F, Args...>
    : eastl::true_type {};

template <class R, class F, class... Args>
using is_invocable_r = is_invocable_r_impl<eastl::true_type, R, F, Args...>;

} // namespace detail
} // namespace fnref

/// A lightweight non-owning reference to a callable.
///
/// Example usage:
///
/// ```cpp
/// void foo (function_ref<int(int)> func) {
///     eastl::cout << "Result is " << func(21); //42
/// }
///
/// foo([](int i) { return i*2; });
template <class F> class function_ref;

/// Specialization for function types.
template <class R, class... Args> class function_ref<R(Args...)> {
public:
  constexpr function_ref() noexcept = delete;

  /// Creates a `function_ref` which refers to the same callable as `rhs`.
    constexpr function_ref(const function_ref<R(Args...)> &rhs) noexcept = default;

  /// Constructs a `function_ref` referring to `f`.
  ///
  /// \synopsis template <typename F> constexpr function_ref(F &&f) noexcept
  template <typename F,
            detail::fnref::enable_if_t<
                !eastl::is_same<detail::fnref::decay_t<F>, function_ref>::value &&
                detail::fnref::is_invocable_r<R, F &&, Args...>::value> * = nullptr>
  ASTL_FUNCTION_REF_11_CONSTEXPR function_ref(F &&f) noexcept
      : obj_(const_cast<void*>(reinterpret_cast<const void *>(eastl::addressof(f)))) {
    callback_ = [](void *obj, Args... args) -> R {
      return detail::fnref::invoke(
          *reinterpret_cast<typename eastl::add_pointer<F>::type>(obj),
          eastl::forward<Args>(args)...);
    };
  }

  /// Makes `*this` refer to the same callable as `rhs`.
  ASTL_FUNCTION_REF_11_CONSTEXPR function_ref<R(Args...)> &
  operator=(const function_ref<R(Args...)> &rhs) noexcept = default;

  /// Makes `*this` refer to `f`.
  ///
  /// \synopsis template <typename F> constexpr function_ref &operator=(F &&f) noexcept;
  template <typename F,
            detail::fnref::enable_if_t<detail::fnref::is_invocable_r<R, F &&, Args...>::value>
                * = nullptr>
  ASTL_FUNCTION_REF_11_CONSTEXPR function_ref<R(Args...)> &operator=(F &&f) noexcept {
    obj_ = reinterpret_cast<void *>(eastl::addressof(f));
    callback_ = [](void *obj, Args... args) {
      return detail::fnref::invoke(
          *reinterpret_cast<typename eastl::add_pointer<F>::type>(obj),
          eastl::forward<Args>(args)...);
    };

    return *this;
  }

  /// Swaps the referred callables of `*this` and `rhs`.
  constexpr void swap(function_ref<R(Args...)> &rhs) noexcept {
    eastl::swap(obj_, rhs.obj_);
    eastl::swap(callback_, rhs.callback_);
  }

  /// Call the stored callable with the given arguments.
  R operator()(Args... args) const {
    return callback_(obj_, eastl::forward<Args>(args)...);
  }

private:
  void *obj_ = nullptr;
  R (*callback_)(void *, Args...) = nullptr;
};

/// Swaps the referred callables of `lhs` and `rhs`.
template <typename R, typename... Args>
constexpr void swap(function_ref<R(Args...)> &lhs,
                    function_ref<R(Args...)> &rhs) noexcept {
  lhs.swap(rhs);
}

#if __cplusplus >= 201703L
template <typename R, typename... Args>
function_ref(R (*)(Args...))->function_ref<R(Args...)>;

// TODO, will require some kind of callable traits
// template <typename F>
// function_ref(F) -> function_ref</* deduced if possible */>;
#endif
} // namespace tl
#endif // __FUNCTION_REF_H_951DF2A9_291D_FF47_89B0_2223AC8B67C8__
