#pragma once
#ifndef __REF_H_E82A7EBA_7FCF_2845_98FF_920CC296C3CF__
#define __REF_H_E82A7EBA_7FCF_2845_98FF_920CC296C3CF__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"
#include "../STL/non_null.h"
#include "../STL/EASTL/utility.h"

#ifndef TRACE_NI_NONNULL
#define TRACE_NI_NONNULL(X)
#endif

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Ptr
 * @{
 */

template<typename T> struct Ptr;
template<typename T> struct QPtr;
template<typename T> struct WeakPtr;

/**
 * A strong reference pointer for iUnknown instances that can never be null.
 */
template <typename T>
struct Nonnull
{
  niClassNoHeapAlloc(Nonnull);
  template<typename U>
  friend struct Nonnull;

 public:
  typedef T tNonnullIType;
  typedef T* tRawPtr;
  typedef const T* tConstRawPtr;
  typedef astl::non_null<tRawPtr> non_null_t;
  typedef astl::non_null<tConstRawPtr> const_non_null_t;

  // Explicit so that its clear at callsites that it will enforce it to be
  // non-null.
  explicit Nonnull(const T* aPtr) {
    TRACE_NI_NONNULL("COPY explicit constructor T*")
    niPanicAssertMsg(aPtr != nullptr,
                     "Nonnull explicit constructor, T* can't be null.");
    mRefPtr = niConstCast(T*,aPtr);
    ni::AddRef(mRefPtr);
  }

  Nonnull(const astl::non_null<T*> aRight) {
    TRACE_NI_NONNULL("COPY constructor astl::non_null<T>")
    niAssertMsg(aRight.raw_ptr() != nullptr,
                "Nonnull copy constructor, astl::non_null<T> can't be null.");
    mRefPtr = aRight.raw_ptr();
    ni::AddRef(mRefPtr);
  }
  template <typename U,
            typename = eastl::enable_if_t<
              eastl::is_convertible<U*, T*>::value>>
  Nonnull(const astl::non_null<U*>& aRight) {
    TRACE_NI_NONNULL("COPY constructor astl::non_null<U>")
    niAssertMsg(aRight.raw_ptr() != nullptr,
                "Nonnull copy constructor, astl::non_null<U> can't be null.");
    mRefPtr = (T*)aRight.raw_ptr();
    ni::AddRef(mRefPtr);
  }

  Nonnull(const Nonnull<T>& aRight) {
    TRACE_NI_NONNULL("COPY constructor<T>")
    niAssertMsg(aRight.mRefPtr != nullptr,
                "Nonnull copy constructor, Nonnull<T> can't be null.");
    mRefPtr = aRight.mRefPtr;
    ni::AddRef(mRefPtr);
  }
  template <typename U,
            typename = eastl::enable_if_t<
              eastl::is_convertible<U*, T*>::value>>
  Nonnull(const Nonnull<U>& aRight) {
    TRACE_NI_NONNULL("COPY constructor<U>");
    niAssertMsg(aRight.mRefPtr != nullptr,
                "Nonnull copy constructor, Nonnull<U> can't be null.");
    mRefPtr = aRight.mRefPtr;
    ni::AddRef(mRefPtr);
  }

  Nonnull(Nonnull<T>&& aRight) {
    TRACE_NI_NONNULL("MOVE constructor<T>");
    niAssertMsg(aRight.mRefPtr != nullptr,
                "Nonnull move constructor, Nonnull<T> can't be null.");
    mRefPtr = aRight.mRefPtr;
    aRight.mRefPtr = NULL;
  }
  template <typename U,
            typename = eastl::enable_if_t<
              eastl::is_convertible<U*, T*>::value>>
  Nonnull(Nonnull<U>&& aRight) {
    TRACE_NI_NONNULL("MOVE constructor<U>");
    niAssertMsg(aRight.mRefPtr != nullptr,
                "Nonnull copy constructor, Nonnull<U> can't be null.");
    mRefPtr = aRight.mRefPtr;
    aRight.mRefPtr = NULL;
  }

  ~Nonnull() {
    // mRefPtr can be nulled by the move operator
    if (mRefPtr != NULL) {
      ni::Release(mRefPtr);
    }
  }

  // Copy operator
  Nonnull& operator = (const Nonnull<T>& aRight) {
    TRACE_NI_NONNULL("COPY operator=");
    if (mRefPtr != aRight.mRefPtr) {
      if (mRefPtr) {
        ni::Release(mRefPtr);
      }
      niAssertMsg(aRight.mRefPtr != nullptr,
                  "Nonnull<T>& operator= copy, aRight can't be null.");
      mRefPtr = aRight.mRefPtr;
      ni::AddRef(mRefPtr);
    }
    return *this;
  }

  // Move operator
  Nonnull& operator = (Nonnull<T>&& aRight) {
    TRACE_NI_NONNULL("MOVE operator=");
    if (mRefPtr != aRight.mRefPtr) {
      if (mRefPtr) {
        ni::Release(mRefPtr);
      }
      niAssertMsg(aRight.mRefPtr != nullptr,
                  "Nonnull<T>&& operator= move, aRight can't be null.");
      mRefPtr = aRight.mRefPtr;
      aRight.mRefPtr = NULL;
    }
    return *this;
  }

  // Casting to a non_null pointer.
  operator non_null_t () const {
    return non_null();
  }

  // Casting to a T* pointer.
  operator T* () const {
    return mRefPtr;
  }

  // Casting to a non_null<const T*> pointer
  template <
    typename U = T,
    typename eastl::enable_if<!eastl::is_const<U>::value,int>::type* = nullptr>
  operator const_non_null_t () const {
    return c_non_null();
  }

  // Casting to a const T* pointer.
  template <
    typename U = T,
    typename eastl::enable_if<!eastl::is_const<U>::value,int>::type* = nullptr>
  operator const T* () const {
    return const_cast<const T*>(mRefPtr);
  }

  // Dereference operator
  T& operator * () const {
    niDebugAssert(mRefPtr != nullptr);
    return *mRefPtr;
  }

  // Arrow operator, allow to use regular C syntax to access members of class.
  T* operator -> (void) const {
    niDebugAssert(mRefPtr != nullptr);
    return mRefPtr;
  }

  non_null_t non_null() const {
    niDebugAssert(mRefPtr != nullptr);
    return niCCast(astl::non_null<T*>&,mRefPtr);
  }
  const_non_null_t c_non_null() const {
    niDebugAssert(mRefPtr != nullptr);
    return niCCast(astl::non_null<const T*>&,mRefPtr);
  }

  T* raw_ptr() const {
    return const_cast<T*>(mRefPtr);
  }

  struct tUnsafeCheckNonnullInitForMacro {
    explicit tUnsafeCheckNonnullInitForMacro(T* aPointer)
        : _maybe_null_ptr(aPointer) {
      if (_maybe_null_ptr) {
        ni::AddRef(_maybe_null_ptr);
#ifdef _DEBUG
        _initialNumRef = _maybe_null_ptr->GetNumRefs();
#endif
      }
    }

    template <typename U>
    explicit tUnsafeCheckNonnullInitForMacro(Ptr<U>& aPtr)
        : _maybe_null_ptr(aPtr.raw_ptr()) {
      if (_maybe_null_ptr) {
        ni::AddRef(_maybe_null_ptr);
#ifdef _DEBUG
        _initialNumRef = _maybe_null_ptr->GetNumRefs();
#endif
      }
    }
    template <typename U>
    explicit tUnsafeCheckNonnullInitForMacro(Ptr<U>&& aPtr)
        : _maybe_null_ptr(aPtr.raw_ptr()) {
      aPtr.mPtr = NULL;
#ifdef _DEBUG
      if (_maybe_null_ptr) {
        _initialNumRef = _maybe_null_ptr->GetNumRefs();
      }
#endif
    }

    template <typename U>
    explicit tUnsafeCheckNonnullInitForMacro(QPtr<U>& aPtr)
        : _maybe_null_ptr(aPtr.raw_ptr()) {
      if (_maybe_null_ptr) {
        ni::AddRef(_maybe_null_ptr);
#ifdef _DEBUG
        _initialNumRef = _maybe_null_ptr->GetNumRefs();
#endif
      }
    }
    template <typename U>
    explicit tUnsafeCheckNonnullInitForMacro(QPtr<U>&& aPtr)
        : _maybe_null_ptr(aPtr.raw_ptr()) {
      aPtr.mPtr = NULL;
#ifdef _DEBUG
      if (_maybe_null_ptr) {
        _initialNumRef = _maybe_null_ptr->GetNumRefs();
      }
#endif
    }

    template <typename U>
    explicit tUnsafeCheckNonnullInitForMacro(const WeakPtr<U>& aPtr)
        : _maybe_null_ptr(aPtr._DerefAndAddRef()) {
#ifdef _DEBUG
      if (_maybe_null_ptr) {
        _initialNumRef = _maybe_null_ptr->GetNumRefs();
      }
#endif
    }

    ~tUnsafeCheckNonnullInitForMacro() {
#ifdef _DEBUG
      if (_maybe_null_ptr) {
        niDebugAssertMsg(
          _maybe_null_ptr->GetNumRefs() == _initialNumRef,
          "Invalid NumRef");
      }
#endif
    }

    T* _maybe_null_ptr;
#ifdef _DEBUG
    tI32 _initialNumRef = -1;
#endif

   private:
    tUnsafeCheckNonnullInitForMacro() = delete;
    tUnsafeCheckNonnullInitForMacro(const tUnsafeCheckNonnullInitForMacro&) = delete;
    tUnsafeCheckNonnullInitForMacro& operator = (const tUnsafeCheckNonnullInitForMacro&) = delete;
    tUnsafeCheckNonnullInitForMacro(const tUnsafeCheckNonnullInitForMacro&&) = delete;
    tUnsafeCheckNonnullInitForMacro& operator = (const tUnsafeCheckNonnullInitForMacro&&) = delete;
  };
  Nonnull(tUnsafeCheckNonnullInitForMacro&& aRight) {
    TRACE_NI_NONNULL("explicit tUnsafeCheckNonnullInitForMacro MOVE constructor")
    mRefPtr = aRight._maybe_null_ptr;
  }
  Nonnull& operator = (tUnsafeCheckNonnullInitForMacro&& aRight) {
    TRACE_NI_NONNULL("tUnsafeCheckNonnullInitForMacro MOVE operator=")
    if (mRefPtr) {
      ni::Release(mRefPtr);
    }
    mRefPtr = aRight._maybe_null_ptr;
    return *this;
  }

 private:
  Nonnull() = delete;
  Nonnull& operator = (T* newp) = delete;

  // To confuse the compiler if someone tries to delete the object
  operator void* () const;

  // Cannot be non_null because we allow mRefPtr to be set to NULL on move
  T* mRefPtr;
};

niCAssert(sizeof(Nonnull<iUnknown>) == sizeof(iUnknown*));

template<class T, class U> inline bool operator==(Nonnull<T> const& a, Nonnull<U> const& b) {
  return a.raw_ptr() == b.raw_ptr();
}
template<class T, class U> inline bool operator!=(Nonnull<T> const& a, Nonnull<U> const& b) {
  return a.raw_ptr() != b.raw_ptr();
}
template<class T, class U> inline bool operator<(Nonnull<T> const& a, Nonnull<U> const& b) {
  return a.raw_ptr() < b.raw_ptr();
}
template<class T, class U> inline bool operator>(Nonnull<T> const& a, Nonnull<U> const& b) {
  return a.raw_ptr() > b.raw_ptr();
}
template<class T, class U> inline bool operator<=(Nonnull<T> const& a, Nonnull<U> const& b) {
  return a.raw_ptr() <= b.raw_ptr();
}
template<class T, class U> inline bool operator>=(Nonnull<T> const& a, Nonnull<U> const& b) {
  return a.raw_ptr() >= b.raw_ptr();
}

template <typename T, typename... Args>
inline EA_CONSTEXPR ni::Nonnull<T> NewNonnull(Args&&... args) {
  return ni::Nonnull<T>(niNew T(eastl::forward<Args>(args)...));
}

template <typename T>
inline EA_CONSTEXPR Nonnull<T> MakeNonnull(T* v) {
  return Nonnull<T>(v);
}

/**@}*/
/**@}*/
}; // End of ni

namespace astl {

template<typename DestT, typename SrcT>
astl::non_null<DestT> const_cast_non_null(const ni::Nonnull<SrcT>& p) {
  return astl::non_null<DestT>(const_cast<DestT>(p.raw_ptr()));
}

}

namespace eastl {

template <typename T>
struct hash<ni::Nonnull<T> > {
  size_t operator()(const ni::Nonnull<T>& v) const {
    return eastl::hash<T*>{}(v.raw_ptr());
  }
};

}
/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __REF_H_E82A7EBA_7FCF_2845_98FF_920CC296C3CF__