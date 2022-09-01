#ifndef __SMARTPTR_24481621_H__
#define __SMARTPTR_24481621_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "Nonnull.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Ptr
 * @{
 */

template <typename T>
struct WeakPtr;

#if !defined niNoUnsafePtr
#define niPtr_HasUnsafeAPI
#else
#define niPtr_NoUnsafeAPI
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct Ptr
{
  niClassNoHeapAlloc(Ptr);

 public:
  Ptr() : mPtr(NULL) {}

  Ptr(const T* _p) {
    mPtr = niConstCast(T*,_p);
    if (mPtr)
      ni::AddRef(mPtr);
  }
  Ptr(const Ptr<T>& _p) : mPtr(_p.mPtr) {
    if (mPtr)
      ni::AddRef(mPtr);
  }
  Ptr(const WeakPtr<T>& aP);

  Ptr(const astl::non_null<T*>& aRight) {
    mPtr = niConstCast(T*,aRight.raw_ptr());
    ni::AddRef(mPtr);
  }
  template <typename U,
            typename = eastl::enable_if_t<
              eastl::is_convertible<U*, T*>::value>>
  Ptr(const astl::non_null<U*>& aU) {
    mPtr = niConstCast(U*,aU.raw_ptr());
    ni::AddRef(mPtr);
  }

  Ptr(const Nonnull<T>& aRight) {
    mPtr = niConstCast(T*,aRight.raw_ptr());
    ni::AddRef(mPtr);
  }
  template <typename U,
            typename = eastl::enable_if_t<
              eastl::is_convertible<U*, T*>::value>>
  Ptr(const Nonnull<U>& aU) {
    mPtr = niConstCast(U*,aU.raw_ptr());
    ni::AddRef(mPtr);
  }

  ~Ptr()  {
    if (mPtr)
      ni::Release(mPtr);
  }

  // Assignment operator
  Ptr& operator = (T* newp) {
    Swap(newp);
    return *this;
  }
  Ptr& operator = (const Ptr<T> &newp) {
    Swap(newp.mPtr);
    return *this;
  }
  Ptr& operator = (const astl::non_null<T*> &newp) {
    Swap(newp.raw_ptr());
    return *this;
  }
  Ptr& operator = (const Nonnull<T> &newp) {
    Swap(newp.raw_ptr());
    return *this;
  }

  // Check whether the pointer holds a valid object.
  bool IsOK() const {
    return mPtr != NULL && mPtr->IsOK();
  }

#if defined niPtr_HasUnsafeAPI
  // Casting to a normal pointer.
  operator T* () const {
    return mPtr;
  }

  // Dereference operator
  T& operator * () const {
    niAssert(mPtr != NULL);
    return *mPtr;
  }

  // Arrow operator, allow to use regular C syntax to access members of class.
  T* operator -> (void) const {
    niAssert(mPtr != NULL);
    return mPtr;
  }
#endif

  // Replace pointer.
  void Swap(const T* apPointer) {
    T* newp = (T*)apPointer;
    if (newp != mPtr) {
      if (newp)
        ni::AddRef(newp);
      if (mPtr)
        ni::Release(mPtr);
    }
    mPtr = newp;
  }

#if defined niPtr_HasUnsafeAPI
  //! Null the smart pointer and return it's contained pointer.
  //! \remark This method makes sure that the pointer returned is not released.
  //!     It can return zero reference objects.
  T* GetRawAndSetNull() {
    if (!mPtr) return NULL;
    T* rawPtr = mPtr;
    rawPtr->SetNumRefs(rawPtr->GetNumRefs()+1000);
    *this = NULL;
    rawPtr->SetNumRefs(rawPtr->GetNumRefs()-1000);
    return rawPtr;
  }
#endif

  // shared_ptr like accessors
  T* ptr() const { return const_cast<T*>(mPtr);  }
  T** ptrptr() const { return const_cast<T**>(&mPtr);  }

  void swap(T* newp) { this->Swap(newp); }

  tBool is_null() const {
    return mPtr == nullptr;
  }
  tBool has_value() const {
    return mPtr != nullptr;
  }
  ni::Nonnull<T>& non_null() const {
    niPanicAssert(mPtr != nullptr);
    return niCCast(Nonnull<T>&,*this);
  }
  ni::Nonnull<const T>& c_non_null() const {
    niPanicAssert(mPtr != nullptr);
    return niCCast(Nonnull<const T>&,*this);
  }

  T* raw_ptr() const {
    return const_cast<T*>(mPtr);
  }

  ni::Ptr<const T>& ToConst() const {
    niPanicAssert(mPtr != nullptr);
    return (ni::Ptr<const T>&)*this;
  }

 private:
  // Prevent if (PTR), if (!PTR), if (PTR == 0/NULL), if (PTR != 0/NULL)
  operator bool () const = delete;
  bool operator !() const = delete;
  bool operator == (int) const = delete;
  bool operator != (int) const = delete;

  // To confuse the compiler if someone tries to delete the smart pointer
  operator void* () const = delete;

  // unwanted operators...pointers only point to single objects!
  Ptr& operator++() = delete;
  Ptr& operator--() = delete;
  Ptr operator++(int) = delete;
  Ptr operator--(int) = delete;
  Ptr& operator+=(std::ptrdiff_t) = delete;
  Ptr& operator-=(std::ptrdiff_t) = delete;
  void operator[](std::ptrdiff_t) const = delete;

  T* mPtr;
};

// more unwanted operators
template <class T, class U>
std::ptrdiff_t operator-(const Ptr<T>&, const Ptr<U>&) = delete;
template <class T>
Ptr<T> operator-(const Ptr<T>&, std::ptrdiff_t) = delete;
template <class T>
Ptr<T> operator+(const Ptr<T>&, std::ptrdiff_t) = delete;
template <class T>
Ptr<T> operator+(std::ptrdiff_t, const Ptr<T>&) = delete;

template<class T, class U>
inline bool operator==(Ptr<T> const& a, Ptr<U> const& b) {
  return a.raw_ptr() == b.raw_ptr();
}
template<class T, class U>
inline bool operator!=(Ptr<T> const& a, Ptr<U> const& b) {
  return a.raw_ptr() != b.raw_ptr();
}
template<class T, class U>
inline bool operator<(Ptr<T> const& a, Ptr<U> const& b) {
  return a.raw_ptr() < b.raw_ptr();
}
template<class T, class U>
inline bool operator>(Ptr<T> const& a, Ptr<U> const& b) {
  return a.raw_ptr() > b.raw_ptr();
}
template<class T, class U>
inline bool operator<=(Ptr<T> const& a, Ptr<U> const& b) {
  return a.raw_ptr() <= b.raw_ptr();
}
template<class T, class U>
inline bool operator>=(Ptr<T> const& a, Ptr<U> const& b) {
  return a.raw_ptr() >= b.raw_ptr();
}

template<class T, class U>
inline bool operator==(Ptr<T> const& a, Nonnull<U> const& b) {
  return a.raw_ptr() == b.raw_ptr();
}
template<class T, class U>
inline bool operator!=(Ptr<T> const& a, Nonnull<U> const& b) {
  return a.raw_ptr() != b.raw_ptr();
}
template<class T, class U>
inline bool operator<(Ptr<T> const& a, Nonnull<U> const& b) {
  return a.raw_ptr() < b.raw_ptr();
}
template<class T, class U>
inline bool operator>(Ptr<T> const& a, Nonnull<U> const& b) {
  return a.raw_ptr() > b.raw_ptr();
}
template<class T, class U>
inline bool operator<=(Ptr<T> const& a, Nonnull<U> const& b) {
  return a.raw_ptr() <= b.raw_ptr();
}
template<class T, class U>
inline bool operator>=(Ptr<T> const& a, Nonnull<U> const& b) {
  return a.raw_ptr() >= b.raw_ptr();
}

template<class T, class U>
inline bool operator==(Nonnull<T> const& a, Ptr<U> const& b) {
  return a.raw_ptr() == b.raw_ptr();
}
template<class T, class U>
inline bool operator!=(Nonnull<T> const& a, Ptr<U> const& b) {
  return a.raw_ptr() != b.raw_ptr();
}
template<class T, class U>
inline bool operator<(Nonnull<T> const& a, Ptr<U> const& b) {
  return a.raw_ptr() < b.raw_ptr();
}
template<class T, class U>
inline bool operator>(Nonnull<T> const& a, Ptr<U> const& b) {
  return a.raw_ptr() > b.raw_ptr();
}
template<class T, class U>
inline bool operator<=(Nonnull<T> const& a, Ptr<U> const& b) {
  return a.raw_ptr() <= b.raw_ptr();
}
template<class T, class U>
inline bool operator>=(Nonnull<T> const& a, Ptr<U> const& b) {
  return a.raw_ptr() >= b.raw_ptr();
}

template <typename T>
__forceinline bool IsOK(Ptr<T> const& a) {
  return a.IsOK();
}

template <typename T>
__forceinline bool IsNullPtr(Ptr<T> const& a) {
  return (a.ptr() == NULL);
}

//! Cast a Ptr type to another Ptr type, makes sure that the base types are compatible
template <typename T, typename I>
ni::Ptr<T>& cast_ptr(ni::Ptr<I>& aPtr) {
  T* r = aPtr.ptr();
  (void)r;
  return (ni::Ptr<T>&)aPtr;
}

template <typename T, typename... Args>
inline EA_CONSTEXPR ni::Ptr<T> NewPtr(Args&&... args) {
  return ni::Ptr<T>(niNew T(eastl::forward<Args>(args)...));
}

template <typename T>
inline EA_CONSTEXPR Ptr<T> MakePtr(T* v) {
  return Ptr<T>(v);
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}; // End of ni
#endif // __SMARTPTR_24481621_H__
