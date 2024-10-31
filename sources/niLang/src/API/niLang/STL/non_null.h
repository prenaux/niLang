#pragma once
#ifndef __NON_NULL_H_A46F1134_B458_8647_8976_5FCF07AB39DD__
#define __NON_NULL_H_A46F1134_B458_8647_8976_5FCF07AB39DD__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

//
// Based on gsl::not_null, see thirdparty/LICENSE-GSL.txt
//

#include "../Types.h"
#include "EASTL/functional.h"
#include "EASTL/type_traits.h"
#include "source_location.h"

namespace astl {

#ifndef TRACE_ASTL_NON_NULL
#define TRACE_ASTL_NON_NULL(X)
#endif

namespace details {
template <typename T, typename = void>
struct is_comparable_to_nullptr : eastl::false_type
{};

template <typename T>
struct is_comparable_to_nullptr<
  T,
  eastl::enable_if_t<eastl::is_convertible<decltype(eastl::declval<T>() != nullptr), bool>::value>>
    : eastl::true_type
{};

} // namespace details

//
// non_null
//
// Restricts a pointer or smart pointer to only hold non-null values.
//
// Has zero size overhead over T.
//
// If T is a pointer (i.e. T == U*) then
// - allow construction from U*
// - disallow construction from nullptr_t
// - disallow default construction
// - ensure construction from null U* fails
// - allow implicit conversion to U*
//
template <class T>
struct non_null
{
  template<class U>
  friend struct non_null;

 public:
  static_assert(details::is_comparable_to_nullptr<T>::value, "T cannot be compared to nullptr.");

  template <typename U>
  constexpr non_null(U&& u, eastl::enable_if_t<eastl::is_pointer_v<U>>* = nullptr) {
    static_assert(false, "Trying to move a raw pointer to non_null. Use ni::as_nn(ptr) or astl::as_non_null(ptr) to mark the raw pointer as non null.");
  }

  template <typename = eastl::enable_if_t<!eastl::is_same<std::nullptr_t, T>::value>>
  constexpr non_null(T u) {
    static_assert(
      false,
      "Trying to copy a raw pointer to non_null. Use ni::as_nn(ptr) or astl::as_non_null(ptr) to mark the raw pointer as non null.");
  }

  // For non_null conversions - allow
  template <typename U>
  constexpr non_null(const non_null<U>& other,
                  eastl::enable_if_t<!eastl::is_pointer_v<U>>* = nullptr)
      : ptr_(other.raw_ptr()) {}

  template <typename U, typename = eastl::enable_if_t<eastl::is_convertible<U, T>::value>>
  constexpr non_null(const non_null<U>& other) : ptr_(other.raw_ptr())
  {}

  non_null(const non_null& other) : ptr_(other.ptr_) {}
  non_null(non_null&& other) : ptr_(eastl::move(other.ptr_)) {}

  non_null& operator=(const non_null& other) {
    this->ptr_ = other.ptr_;
    return *this;
  }
  template <typename U>
  non_null& operator=(const non_null<U>& other) {
    this->ptr_ = other.ptr_;
    return *this;
  }

  non_null& operator=(non_null&& other) {
    this->ptr_ = eastl::move(other.ptr_);
    return *this;
  }
  template <typename U>
  non_null& operator=(non_null<U>&& other) {
    ptr_ = eastl::move(other.ptr_);
    return *this;
  }

  constexpr eastl::conditional_t<eastl::is_copy_constructible<T>::value, T, const T&> raw_ptr() const
  {
    // No null check here, can be nulled with tUnsafeUncheckedInitializer.
    return ptr_;
  }

  constexpr operator T() const {
    niDebugAssert(ptr_ != nullptr);
    return raw_ptr();
  }
  constexpr decltype(auto) operator->() const {
    niDebugAssert(ptr_ != nullptr);
    return raw_ptr();
  }
  constexpr decltype(auto) operator*() const {
    niDebugAssert(ptr_ != nullptr);
    return *raw_ptr();
  }

  // prevents compilation when someone attempts to assign a null pointer constant
  non_null(std::nullptr_t) = delete;
  non_null& operator=(std::nullptr_t) = delete;

  // unwanted operators...pointers only point to single objects!
  non_null& operator++() = delete;
  non_null& operator--() = delete;
  non_null operator++(int) = delete;
  non_null operator--(int) = delete;
  non_null& operator+=(std::ptrdiff_t) = delete;
  non_null& operator-=(std::ptrdiff_t) = delete;
  void operator[](std::ptrdiff_t) const = delete;

  struct tUnsafeUncheckedInitializer {
    explicit tUnsafeUncheckedInitializer(T&& aPointer)
        : _maybe_null_ptr(eastl::move(aPointer)) {
    }
    explicit tUnsafeUncheckedInitializer(T& aPointer)
        : _maybe_null_ptr(aPointer) {
    }
    ~tUnsafeUncheckedInitializer() {
    }

    T _maybe_null_ptr;

   private:
    tUnsafeUncheckedInitializer() = delete;
    tUnsafeUncheckedInitializer(const tUnsafeUncheckedInitializer&) = delete;
    tUnsafeUncheckedInitializer& operator = (const tUnsafeUncheckedInitializer&) = delete;
    tUnsafeUncheckedInitializer(const tUnsafeUncheckedInitializer&&) = delete;
    tUnsafeUncheckedInitializer& operator = (const tUnsafeUncheckedInitializer&&) = delete;
  };
  non_null(tUnsafeUncheckedInitializer&& aRight) {
    TRACE_ASTL_NON_NULL("explicit tUnsafeUncheckedInitializer MOVE constructor")
    ptr_ = eastl::move(aRight._maybe_null_ptr);
  }
  non_null& operator = (tUnsafeUncheckedInitializer&& aRight) {
    TRACE_ASTL_NON_NULL("tUnsafeUncheckedInitializer MOVE operator=")
    ptr_ = eastl::move(aRight._maybe_null_ptr);
    return *this;
  }

 private:
  T ptr_;
};

// more unwanted operators
template <class T, class U>
std::ptrdiff_t operator-(const non_null<T>&, const non_null<U>&) = delete;
template <class T>
non_null<T> operator-(const non_null<T>&, std::ptrdiff_t) = delete;
template <class T>
non_null<T> operator+(const non_null<T>&, std::ptrdiff_t) = delete;
template <class T>
non_null<T> operator+(std::ptrdiff_t, const non_null<T>&) = delete;

template <class T, class U>
auto operator==(const non_null<T>& lhs,
                const non_null<U>& rhs) noexcept(noexcept(lhs.raw_ptr() == rhs.raw_ptr()))
    -> decltype(lhs.raw_ptr() == rhs.raw_ptr())
{
  return lhs.raw_ptr() == rhs.raw_ptr();
}

template <class T, class U>
auto operator!=(const non_null<T>& lhs,
                const non_null<U>& rhs) noexcept(noexcept(lhs.raw_ptr() != rhs.raw_ptr()))
    -> decltype(lhs.raw_ptr() != rhs.raw_ptr())
{
  return lhs.raw_ptr() != rhs.raw_ptr();
}

template <class T, class U>
auto operator<(const non_null<T>& lhs,
               const non_null<U>& rhs) noexcept(noexcept(lhs.raw_ptr() < rhs.raw_ptr()))
    -> decltype(lhs.raw_ptr() < rhs.raw_ptr())
{
  return lhs.raw_ptr() < rhs.raw_ptr();
}

template <class T, class U>
auto operator<=(const non_null<T>& lhs,
                const non_null<U>& rhs) noexcept(noexcept(lhs.raw_ptr() <= rhs.raw_ptr()))
    -> decltype(lhs.raw_ptr() <= rhs.raw_ptr())
{
  return lhs.raw_ptr() <= rhs.raw_ptr();
}

template <class T, class U>
auto operator>(const non_null<T>& lhs,
               const non_null<U>& rhs) noexcept(noexcept(lhs.raw_ptr() > rhs.raw_ptr()))
    -> decltype(lhs.raw_ptr() > rhs.raw_ptr())
{
  return lhs.raw_ptr() > rhs.raw_ptr();
}

template <class T, class U>
auto operator>=(const non_null<T>& lhs,
                const non_null<U>& rhs) noexcept(noexcept(lhs.raw_ptr() >= rhs.raw_ptr()))
    -> decltype(lhs.raw_ptr() >= rhs.raw_ptr())
{
  return lhs.raw_ptr() >= rhs.raw_ptr();
}

static_assert(sizeof(non_null<int*>) == sizeof(int*), "non_null<> must be the same size as a pointer.");

template <class T>
inline auto as_non_null(T&& t, ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT) noexcept
{
  if (!t) {
    ni_throw_panic(
      _HSym(ni,panic),
      "as_non_null(&&) with nullptr",
      ASTL_SOURCE_LOCATION_ARG_CALL);
  }
  typedef non_null<eastl::remove_cv_t<eastl::remove_reference_t<T>>> tNN;
  return tNN{typename tNN::tUnsafeUncheckedInitializer(
    eastl::forward<T>(t))};
}

#if (defined(__cpp_deduction_guides) && (__cpp_deduction_guides >= 201611L))

// deduction guides to prevent the ctad-maybe-unsupported warning
template <class T>
non_null(T) -> non_null<T>;

#endif // ( defined(__cpp_deduction_guides) && (__cpp_deduction_guides >= 201611L) )

#define niNonnullIfNull(V,EXPR)                   \
  decltype(V)::tUnsafeUncheckedInitializer{EXPR}; \
  EA_ENABLE_GCC_WARNING_AS_ERROR(-Wshadow);       \
  EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wshadow);     \
  int V##_DontDeclareSameNonnullCheckTwice;       \
  niUnused(V##_DontDeclareSameNonnullCheckTwice); \
  EA_DISABLE_CLANG_WARNING_AS_ERROR();            \
  EA_DISABLE_GCC_WARNING_AS_ERROR();              \
  if ((V).raw_ptr() == nullptr)

#define niCheckNonnull(V,EXPR,RET)                  \
  niNonnullIfNull(V,EXPR) {                         \
    niError("niCheckNonnull '" #EXPR "' failed.");  \
    return RET;                                     \
  }

#define niCheckNonnullMsg(V,EXPR,MSG,RET)       \
  niNonnullIfNull(V,EXPR) {                     \
    niError(MSG);                               \
    return RET;                                 \
  }

#define niCheckNonnullSilent(V,EXPR,RET)        \
  niNonnullIfNull(V,EXPR) {                     \
    return RET;                                 \
  }

} // end of namespace astl

namespace eastl {

template <typename T>
struct hash<astl::non_null<T> > {
  size_t operator()(const astl::non_null<T>& v) const {
    return eastl::hash<T>{}(v.raw_ptr());
  }
};

} // end of namespace eastl
#endif // __NON_NULL_H_A46F1134_B458_8647_8976_5FCF07AB39DD__
