#ifndef __QPTR_776684_H__
#define __QPTR_776684_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "SmartPtr.h"
#include "WeakPtr.h"
#include "../Var.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Ptr
 * @{
 */

#if !defined niNoUnsafePtr
#define niQPtr_HasUnsafeAPI
#else
#define niQPtr_NoUnsafeAPI
#endif

/**
 * A strong pointer to an iUnknown instance that may query the underlying
 * object before assigning it. Can be null.
 *
 * - If the object assigned is of exactly the same type it behaves like a regular Ptr<>.
 * - If the object is of a different type then QueryInterface is performed before assignement.
 * - If a WeakPtr is passed it'll be dereferenced and a QueryInterface may be
 *   performed if the underlying type is different.
 *
 */
template <typename T>
class QPtr
{
  niClassNoHeapAlloc(QPtr);
  template<class U> friend struct Ptr;
  template<class U> friend struct QPtr;
  template<class U> friend struct Nonnull;

  typedef typename T::IUnknownBaseType tInterface;

 public:
  QPtr() : mPtr(NULL) {
  }
  QPtr(const T* aP) {
    mPtr = niConstCast(T*,aP);
    if (mPtr)
      ni::AddRef(mPtr);
  }
  QPtr(const Ptr<T>& aP) {
    mPtr = niConstCast(T*,aP.raw_ptr());
    if (mPtr)
      ni::AddRef(mPtr);
  }
  QPtr(const Nonnull<T>& aP) {
    mPtr = niConstCast(T*,aP.raw_ptr());
    ni::AddRef(mPtr);
  }

  QPtr(const QPtr<T>& aP) {
    mPtr = aP.mPtr;
    if (mPtr)
      ni::AddRef(mPtr);
  }
  QPtr(QPtr<T>&& aP) {
    mPtr = aP.mPtr;
    aP.mPtr = NULL;
  }

  QPtr(const Var& aV) {
    mPtr = VarQueryInterface<T>(aV);
    if (mPtr)
      ni::AddRef(mPtr);
  }

  template <typename S>
  QPtr(const S* aP) {
    mPtr = ni::QueryInterface<tInterface>(aP);
    if (mPtr)
      ni::AddRef(mPtr);
  }
  template <typename S>
  QPtr(const Ptr<S>& aP) {
    mPtr = (T*)ni::QueryInterface<tInterface>(aP.ptr());
    if (mPtr)
      ni::AddRef(mPtr);
  }
  template <typename S>
  QPtr(const Nonnull<S>& aP) {
    mPtr = (T*)ni::QueryInterface<tInterface>(aP.raw_ptr());
    if (mPtr)
      ni::AddRef(mPtr);
  }

  QPtr(const WeakPtr<T>& aP) {
    mPtr = aP._DerefAndAddRef();
  }
  template <typename S>
  QPtr(const WeakPtr<S>& aP) {
    S* sp = aP._DerefAndAddRef();
    if (sp) {
      mPtr = (T*)ni::QueryInterface<tInterface>(sp);
      // No AddRef, its already added by _DerefAndAddRef
      if (mPtr == NULL) {
        // If QueryInterface failed we have to release the object since we
        // just added a reference to it with _DerefAndAddRef
        ni::Release(sp);
      }
      // If QueryInterface returns a different object we need add a reference
      // to it and release the original one
      else if ((tIntPtr)sp != (tIntPtr)mPtr) {
        ni::AddRef(mPtr);
        ni::Release(sp);
      }
    }
    else {
      mPtr = NULL;
    }
  }

  ~QPtr() {
    if (mPtr)
      ni::Release(mPtr);
  }

  // Assignment operator
  QPtr&  operator = (const QPtr& newp) {
    this->Swap(newp.mPtr);
    return *this;
  }
  QPtr& operator = (const T* newp) {
    this->Swap(newp);
    return *this;
  }
  QPtr& operator = (const astl::non_null<T*> &newp) {
    this->Swap(newp.raw_ptr());
    return *this;
  }
  QPtr& operator = (const Nonnull<T> &newp) {
    this->Swap(newp.raw_ptr());
    return *this;
  }

  // Check whether the pointer holds a valid object.
  bool IsOK() const {
    return mPtr != NULL && mPtr->IsOK();
  }

#if defined niQPtr_HasUnsafeAPI
  // Casting to a normal pointer.
  operator T* () const {
    return mPtr;
  }

  // Dereference operator
  T& operator * () const {
    niAssert(IsOK());
    return *mPtr;
  }

  // Arrow operator, allow to use regular C syntax to access members of class.
  T* operator -> (void) const {
    niAssert(IsOK());
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

#if defined niQPtr_HasUnsafeAPI
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
  T* ptr() const { return mPtr;  }
  T** ptrptr() const { return &mPtr;  }

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

 private:
  // Prevent if (PTR), if (!PTR), if (PTR == 0/NULL), if (PTR != 0/NULL)
  operator bool () const;
  bool operator !() const;
  bool operator == (int) const;
  bool operator != (int) const;

  // To confuse the compiler if someone tries to delete the smart pointer
  operator void* () const;

  // unwanted operators...pointers only point to single objects!
  QPtr& operator++() = delete;
  QPtr& operator--() = delete;
  QPtr operator++(int) = delete;
  QPtr operator--(int) = delete;
  QPtr& operator+=(std::ptrdiff_t) = delete;
  QPtr& operator-=(std::ptrdiff_t) = delete;
  void operator[](std::ptrdiff_t) const = delete;

  T* mPtr;
};

// more unwanted operators
template <class T, class U>
std::ptrdiff_t operator-(const QPtr<T>&, const QPtr<U>&) = delete;
template <class T>
QPtr<T> operator-(const QPtr<T>&, std::ptrdiff_t) = delete;
template <class T>
QPtr<T> operator+(const QPtr<T>&, std::ptrdiff_t) = delete;
template <class T>
QPtr<T> operator+(std::ptrdiff_t, const QPtr<T>&) = delete;

///
template<class T, class U> inline bool operator==(QPtr<T> const& a, QPtr<U> const& b) {
  return a.raw_ptr() == b.raw_ptr();
}
template<class T, class U> inline bool operator!=(QPtr<T> const& a, QPtr<U> const& b) {
  return a.raw_ptr() != b.raw_ptr();
}
template<class T, class U> inline bool operator<(QPtr<T> const& a, QPtr<U> const& b) {
  return a.raw_ptr() < b.raw_ptr();
}
template<class T, class U> inline bool operator>(QPtr<T> const& a, QPtr<U> const& b) {
  return a.raw_ptr() > b.raw_ptr();
}
template<class T, class U> inline bool operator<=(QPtr<T> const& a, QPtr<U> const& b) {
  return a.raw_ptr() <= b.raw_ptr();
}
template<class T, class U> inline bool operator>=(QPtr<T> const& a, QPtr<U> const& b) {
  return a.raw_ptr() >= b.raw_ptr();
}

template <typename T>
__forceinline bool IsOK(QPtr<T> const& a) {
  return a.IsOK();
}

template <typename T>
__forceinline bool IsNullPtr(QPtr<T> const& a) {
  return (a.raw_ptr() == nullptr);
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __QPTR_776684_H__
