#pragma once
#ifndef __REF_H_E82A7EBA_7FCF_2845_98FF_920CC296C3CF__
#define __REF_H_E82A7EBA_7FCF_2845_98FF_920CC296C3CF__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"
#include "../STL/maybe.h"
#include "../STL/non_null.h"
#include "SmartPtr.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Ptr
 * @{
 */

//! A strong reference pointer to an iUnknown object that can never be null.
template <typename T>
struct Nonnull
{
  niClassNoHeapAlloc(Nonnull);

 public:
  typedef astl::non_null<T*> non_null_t;
  typedef astl::non_null<const T*> const_non_null_t;

  // Explicit so that its clear at callsites that it will enforce it to be
  // non-null.
  explicit Nonnull(const T* aPtr) {
    mRefPtr = niConstCast(T*,aPtr);
    niPanicAssert(niIsOK(mRefPtr));
    ni::AddRef(mRefPtr);
  }

  Nonnull(const Nonnull<T>& aRight) {
    // Call the copy operator
    *this = aRight;
  }

  Nonnull(Nonnull<T>&& aRight) {
    // Call the move operator
    *this = astl::move(aRight);
  }

  ~Nonnull() {
    // mRefPtr can be nulled by the move operator
    if (mRefPtr != NULL) {
      ni::Release(mRefPtr);
    }
  }

  // Copy operator
  Nonnull& operator = (const Nonnull<T>& aRight) {
    niPanicAssert(niIsOK(aRight.mRefPtr));
    mRefPtr = aRight.mRefPtr;
    ni::AddRef(mRefPtr);
    return *this;
  }

  // Move operator
  Nonnull& operator = (Nonnull<T>&& aRight) {
    niPanicAssert(niIsOK(aRight.mRefPtr));
    mRefPtr = aRight.mRefPtr;
    aRight.mRefPtr = NULL;
    return *this;
  }

  // Casting to a non_null pointer.
  operator non_null_t () const {
    return ptr();
  }

  // Casting to a T* pointer.
  operator T* () const {
    return mRefPtr;
  }

  // Casting to a non_null<const T*> pointer
  template <
    typename U = T,
    typename eastl::enable_if<!std::is_const<U>::value,int>::type* = nullptr>
  operator const_non_null_t () const {
    return c_ptr();
  }

  // Casting to a const T* pointer.
  template <
    typename U = T,
    typename eastl::enable_if<!std::is_const<U>::value,int>::type* = nullptr>
  operator const T* () const {
    return const_cast<const T*>(mRefPtr);
  }

  // Dereference operator
  T& operator * () const {
    niDebugAssert(niIsOK(mRefPtr));
    return *mRefPtr;
  }

  // Arrow operator, allow to use regular C syntax to access members of class.
  T* operator -> (void) const {
    niDebugAssert(niIsOK(mRefPtr));
    return mRefPtr;
  }

  // shared_ptr like accessors
  non_null_t ptr() const {
    niDebugAssert(niIsOK(mRefPtr));
    return niCCast(astl::non_null<T*>&,mRefPtr);
  }
  const_non_null_t c_ptr() const {
    niDebugAssert(niIsOK(mRefPtr));
    return niCCast(astl::non_null<const T*>&,mRefPtr);
  }

 private:
  Nonnull() = delete;
  Nonnull& operator = (T* newp);

  // To confuse the compiler if someone tries to delete the object
  operator void* () const;

  // Cannot be non_null because we allow mRefPtr to be set to NULL on move
  T* mRefPtr;
};

niCAssert(sizeof(Nonnull<iUnknown>) == sizeof(Ptr<iUnknown>));

template<class T, class U> inline bool operator==(Nonnull<T> const& a, Nonnull<U> const& b) {
  return a.ptr() == b.ptr();
}
template<class T, class U> inline bool operator!=(Nonnull<T> const& a, Nonnull<U> const& b) {
  return a.ptr() != b.ptr();
}
template<class T, class U> inline bool operator<(Nonnull<T> const& a, Nonnull<U> const& b) {
  return a.ptr() < b.ptr();
}
template<class T, class U> inline bool operator>(Nonnull<T> const& a, Nonnull<U> const& b) {
  return a.ptr() > b.ptr();
}
template<class T, class U> inline bool operator<=(Nonnull<T> const& a, Nonnull<U> const& b) {
  return a.ptr() <= b.ptr();
}
template<class T, class U> inline bool operator>=(Nonnull<T> const& a, Nonnull<U> const& b) {
  return a.ptr() >= b.ptr();
}

template <typename T>
using MaybePtr = astl::maybe<ni::Nonnull<T> >;

template <typename T, typename... Args>
inline EA_CONSTEXPR ni::Nonnull<T> NewNonnull(Args&&... args) {
  return ni::Nonnull<T>(niNew T(eastl::forward<Args>(args)...));
}

template <typename T>
inline EA_CONSTEXPR Nonnull<T> MakeNonnull(T* v) {
  return Nonnull<T>(v);
}

template <typename T>
inline EA_CONSTEXPR MaybePtr<T> JustNonnull(T* v) {
  return astl::just(Nonnull<T>(v));
}
template <typename T>
inline EA_CONSTEXPR MaybePtr<T> JustNonnull(const Ptr<T>& v) {
  return JustNonnull(v.ptr());
}

template <typename T, typename... Args>
inline EA_CONSTEXPR MaybePtr<T> NewMaybePtr(Args&&... args) {
  return JustNonnull(niNew T(eastl::forward<Args>(args)...));
}

template <typename T>
inline EA_CONSTEXPR MaybePtr<T> MakeMaybePtr(T* v) {
  if (v == nullptr) {
    return ASTL_NOTHING;
  }
  else {
    return JustNonnull<T>(v);
  }
}
template <typename T>
inline EA_CONSTEXPR MaybePtr<T> MakeMaybePtr(const Ptr<T>& v) {
  return MakeMaybePtr(v.ptr());
}

template <typename T>
inline EA_CONSTEXPR astl::maybe<ni::Nonnull<const T> >& ConstMaybePtr(const MaybePtr<T>& v) {
  return niCCast(astl::maybe<ni::Nonnull<const T> >&,v);
}

/**@}*/
/**@}*/
}; // End of ni

namespace eastl {

template <typename T>
struct hash<ni::Nonnull<T> > {
  size_t operator()(const ni::Nonnull<T>& v) const {
    return eastl::hash<typename ni::Nonnull<T>::non_null_t>{}(v.ptr());
  }
};

}
/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __REF_H_E82A7EBA_7FCF_2845_98FF_920CC296C3CF__
