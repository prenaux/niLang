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

#ifndef TRACE_ASTL_NON_NULL
#define TRACE_ASTL_NON_NULL(X)
#endif

namespace astl {

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
 public:
  static_assert(details::is_comparable_to_nullptr<T>::value, "T cannot be compared to nullptr.");

  template <typename U, typename = eastl::enable_if_t<eastl::is_convertible<U, T>::value>>
  constexpr explicit non_null(U&& u) : ptr_(eastl::forward<U>(u))
  {
    niPanicAssert(ptr_ != nullptr);
  }

  template <typename = eastl::enable_if_t<!eastl::is_same<std::nullptr_t, T>::value>>
  constexpr explicit non_null(T u) : ptr_(eastl::move(u))
  {
    niPanicAssert(ptr_ != nullptr);
  }

  template <typename U, typename = eastl::enable_if_t<eastl::is_convertible<U, T>::value>>
  constexpr non_null(const non_null<U>& other) : non_null(other.raw_ptr())
  {}

  non_null(non_null&& other) = default;
  non_null(const non_null& other) = default;
  non_null& operator=(const non_null& other) = default;
  constexpr eastl::conditional_t<eastl::is_copy_constructible<T>::value, T, const T&> raw_ptr() const
  {
    // No null check here, can be nulled by tUnsafeCheckNonnullInitForMacro.
    return ptr_;
  }

  constexpr operator T() const {
    EASTL_ASSERT(ptr_ != nullptr);
    return raw_ptr();
  }
  constexpr decltype(auto) operator->() const {
    EASTL_ASSERT(ptr_ != nullptr);
    return raw_ptr();
  }
  constexpr decltype(auto) operator*() const {
    EASTL_ASSERT(ptr_ != nullptr);
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

  struct tUnsafeCheckNonnullInitForMacro {
    explicit tUnsafeCheckNonnullInitForMacro(T aPointer)
        : _maybe_null_ptr(aPointer) {
    }
    ~tUnsafeCheckNonnullInitForMacro() {
    }

    T _maybe_null_ptr;

   private:
    tUnsafeCheckNonnullInitForMacro() = delete;
    tUnsafeCheckNonnullInitForMacro(const tUnsafeCheckNonnullInitForMacro&) = delete;
    tUnsafeCheckNonnullInitForMacro& operator = (const tUnsafeCheckNonnullInitForMacro&) = delete;
    tUnsafeCheckNonnullInitForMacro(const tUnsafeCheckNonnullInitForMacro&&) = delete;
    tUnsafeCheckNonnullInitForMacro& operator = (const tUnsafeCheckNonnullInitForMacro&&) = delete;
  };
  non_null(tUnsafeCheckNonnullInitForMacro&& aRight) {
    TRACE_ASTL_NON_NULL("explicit tUnsafeCheckNonnullInitForMacro MOVE constructor")
    ptr_ = aRight._maybe_null_ptr;
  }
  non_null& operator = (tUnsafeCheckNonnullInitForMacro&& aRight) {
    TRACE_ASTL_NON_NULL("tUnsafeCheckNonnullInitForMacro MOVE operator=")
    ptr_ = aRight._maybe_null_ptr;
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
auto make_non_null(T&& t) noexcept
{
  return non_null<eastl::remove_cv_t<eastl::remove_reference_t<T>>>{eastl::forward<T>(t)};
}

template<typename DestT, typename SrcT>
astl::non_null<DestT> const_cast_non_null(astl::non_null<SrcT> p) {
  return astl::non_null<DestT>(const_cast<DestT>(p.raw_ptr()));
}

#if (defined(__cpp_deduction_guides) && (__cpp_deduction_guides >= 201611L))

// deduction guides to prevent the ctad-maybe-unsupported warning
template <class T>
non_null(T) -> non_null<T>;

#endif // ( defined(__cpp_deduction_guides) && (__cpp_deduction_guides >= 201611L) )

#define niCheckNonnull_(V,EXPR,RET,ERRLOG)                              \
  decltype(V)::tUnsafeCheckNonnullInitForMacro{EXPR};                   \
  EA_ENABLE_GCC_WARNING_AS_ERROR(-Wshadow);                             \
  EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wshadow);                           \
  int V##_DontDeclareSameNonnullCheckTwice; niUnused(V##_DontDeclareSameNonnullCheckTwice); \
  EA_DISABLE_CLANG_WARNING_AS_ERROR();                                  \
  EA_DISABLE_GCC_WARNING_AS_ERROR();                                    \
  if ((V).raw_ptr() == nullptr) {                                       \
    ERRLOG;                                                             \
    return RET;                                                         \
  }

#define niCheckNonnull(V,EXPR,RET) niCheckNonnull_(V,EXPR,RET,niError("CheckNonnull '" #EXPR "' failed."))
#define niCheckNonnullMsg(V,EXPR,MSG,RET) niCheckNonnull_(V,EXPR,RET,niError(MSG))
#define niCheckNonnullSilent(V,EXPR,RET) niCheckNonnull_(V,EXPR,RET,;)

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
