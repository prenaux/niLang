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

//////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
class QPtr
{
  niClassNoHeapAlloc(Ptr);

  typedef typename T::IUnknownBaseType tInterface;

 public:
  QPtr() : mPtr(NULL) {
  }
  QPtr(const T* aP) {
    mPtr = (T*)aP;
    if (mPtr)
      mPtr->AddRef();
  }
  QPtr(const Ptr<T>& aP) {
    mPtr = aP.ptr();
    if (mPtr)
      mPtr->AddRef();
  }
  QPtr(const QPtr<T>& aP) {
    mPtr = aP.ptr();
    if (mPtr)
      mPtr->AddRef();
  }
  QPtr(const Var& aV) {
    mPtr = VarQueryInterface<T>(aV);
    if (mPtr)
      mPtr->AddRef();
  }

  template <typename S>
  QPtr(const S* aP) {
    mPtr = ni::QueryInterface<tInterface>(aP);
    if (mPtr)
      mPtr->AddRef();
  }
  template <typename S>
  QPtr(const Ptr<S>& aP) {
    mPtr = (T*)ni::QueryInterface<tInterface>(aP.ptr());
    if (mPtr)
      mPtr->AddRef();
  }

  QPtr(const WeakPtr<T>& aP) {
    mPtr = aP.DerefAndAddRef();
  }
  template <typename S>
  QPtr(const WeakPtr<S>& aP) {
    S* sp = aP.DerefAndAddRef();
    if (sp) {
      mPtr = (T*)ni::QueryInterface<tInterface>(sp);
      // No AddRef, its already added by DerefAndAddRef
      if (mPtr == NULL) {
        // If QueryInterface failed we have to release the object since we
        // just added a reference to it with DerefAndAddRef
        sp->Release();
      }
      // If QueryInterface returns a different object we need add a reference
      // to it and release the original one
      else if ((tIntPtr)sp != (tIntPtr)mPtr) {
        mPtr->AddRef();
        sp->Release();
      }
    }
    else {
      mPtr = NULL;
    }
  }

  ~QPtr() {
    if (mPtr)
      mPtr->Release();
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

  // Check whether the pointer holds a valid object.
  bool IsOK() const {
    return mPtr != NULL && mPtr->IsOK();
  }

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

  // Replace pointer.
  void Swap(const T* apPointer) {
    T* newp = (T*)apPointer;
    if (newp != mPtr) {
      if (newp)
        newp->AddRef();
      if (mPtr)
        mPtr->Release();
    }
    mPtr = newp;
  }

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

  // shared_ptr like accessors
  T* ptr() const { return mPtr;  }
  T** ptrptr() const { return &mPtr;  }
  void swap(T* newp) { this->Swap(newp); }

 private:
  // Prevent if (PTR), if (!PTR), if (PTR == 0/NULL), if (PTR != 0/NULL)
  operator bool () const;
  bool operator !() const;
  bool operator == (int) const;
  bool operator != (int) const;

  // To confuse the compiler if someone tries to delete the smart pointer
  operator void* () const;

  T* mPtr;
};

///
template<class T, class U> inline bool operator==(QPtr<T> const& a, QPtr<U> const& b) {
  return a.ptr() == b.ptr();
}
template<class T, class U> inline bool operator!=(QPtr<T> const& a, QPtr<U> const& b) {
  return a.ptr() != b.ptr();
}
template<class T, class U> inline bool operator<(QPtr<T> const& a, QPtr<U> const& b) {
  return a.ptr() < b.ptr();
}
template<class T, class U> inline bool operator>(QPtr<T> const& a, QPtr<U> const& b) {
  return a.ptr() > b.ptr();
}
template<class T, class U> inline bool operator<=(QPtr<T> const& a, QPtr<U> const& b) {
  return a.ptr() <= b.ptr();
}
template<class T, class U> inline bool operator>=(QPtr<T> const& a, QPtr<U> const& b) {
  return a.ptr() >= b.ptr();
}

template <typename T>
__forceinline bool IsOK(QPtr<T> const& a) {
  return a.IsOK();
}

template <typename T>
__forceinline bool IsNullPtr(QPtr<T> const& a) {
  return (a.ptr() == NULL);
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __QPTR_776684_H__
