#ifndef __WEAKPTR_H_2C1983A5_FD14_47C4_BE9C_92D3D12B0F88__
#define __WEAKPTR_H_2C1983A5_FD14_47C4_BE9C_92D3D12B0F88__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "SmartPtr.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Ptr
 * @{
 */

niExportFunc(tBool) ni_object_has_weak_ptr(iUnknown* apObject);
niExportFunc(iUnknown*) ni_object_get_weak_ptr(iUnknown* apObjectPtr);
niExportFunc(iUnknown*) ni_object_deref_and_add_ref_weak_ptr(iUnknown* apWeakPtr);

template <typename T>
struct WeakPtr {
  WeakPtr() {
  }
  WeakPtr(const T* apObjectPtr) {
    this->Swap(apObjectPtr);
  }
  WeakPtr(const WeakPtr<T>& aWeakPtr) {
    this->Swap(aWeakPtr);
  }
  WeakPtr(const Ptr<T>& aPtr) {
    this->Swap(aPtr);
  }

  tBool IsOK() const {
    return mWeakPtrObject.IsOK();
  }

  tBool IsSet() const {
    return mWeakPtrObject.ptr() != NULL;
  }

  void Swap(const T* apObjectPtr) {
    mWeakPtrObject = apObjectPtr ? ni_object_get_weak_ptr(
        (niTypename T::IUnknownBaseType*)apObjectPtr) : NULL;
  }

  void Swap(const WeakPtr<T>& aWeakPtr) {
    mWeakPtrObject = aWeakPtr.mWeakPtrObject;
  }

  void Swap(const Ptr<T>& aPtr) {
    Swap(aPtr.ptr());
  }

  void SetNull() {
    mWeakPtrObject = NULL;
  }

  T* DerefAndAddRef() const {
    return (T*)(niTypename T::IUnknownBaseType*)ni_object_deref_and_add_ref_weak_ptr(mWeakPtrObject.ptr());
  }

  // needed so that WeakPtr<> can be used in a container
  WeakPtr& operator = (const WeakPtr<T>& aWeakPtr) {
    mWeakPtrObject = aWeakPtr.mWeakPtrObject;
    return *this;
  }

  // For comparison operators
  tIntPtr __stdcall __GetWeakObjecIntPtr() const {
    return (tIntPtr)mWeakPtrObject.ptr();
  }

 private:
  Ptr<iUnknown> mWeakPtrObject;

  // should use Swap & Reset() to change the underlying pointer
  WeakPtr(void*);
  WeakPtr(int);
  WeakPtr& operator = (const T* apObjectPtr);
  WeakPtr& operator = (const Ptr<T>& apObjectPtr);

  // Prevent if (PTR), if (!PTR), if (PTR == 0/NULL), if (PTR != 0/NULL)
  operator bool () const;
  bool operator !() const;
  bool operator == (int) const;
  bool operator != (int) const;

  // To confuse the compiler if someone tries to delete the smart pointer
  operator void* () const;
};

template <typename T>
__forceinline bool IsNullPtr(WeakPtr<T> const& a) {
  return (!a.IsSet());
}

template<class T, class U> inline bool operator==(WeakPtr<T> const& a, WeakPtr<U> const& b) {
  return a.__GetWeakObjecIntPtr() == b.__GetWeakObjecIntPtr();
}
template<class T, class U> inline bool operator!=(WeakPtr<T> const& a, WeakPtr<U> const& b) {
  return a.__GetWeakObjecIntPtr() != b.__GetWeakObjecIntPtr();
}
template<class T, class U> inline bool operator<(WeakPtr<T> const& a, WeakPtr<U> const& b) {
  return a.__GetWeakObjecIntPtr() < b.__GetWeakObjecIntPtr();
}
template<class T, class U> inline bool operator>(WeakPtr<T> const& a, WeakPtr<U> const& b) {
  return a.__GetWeakObjecIntPtr() > b.__GetWeakObjecIntPtr();
}
template<class T, class U> inline bool operator<=(WeakPtr<T> const& a, WeakPtr<U> const& b) {
  return a.__GetWeakObjecIntPtr() <= b.__GetWeakObjecIntPtr();
}
template<class T, class U> inline bool operator>=(WeakPtr<T> const& a, WeakPtr<U> const& b) {
  return a.__GetWeakObjecIntPtr() >= b.__GetWeakObjecIntPtr();
}

//! MemberWeakPtr is a weak pointer that requires to be initialized when constructed.
template <typename T>
struct MemberWeakPtr : public WeakPtr<T> {
  MemberWeakPtr(T* apM) : WeakPtr<T>(apM) {}
};

template<class T>
inline Ptr<T>::Ptr(const WeakPtr<T>& aP) {
  mPtr = aP.DerefAndAddRef();
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}; // End of ni
#endif // __WEAKPTR_H_2C1983A5_FD14_47C4_BE9C_92D3D12B0F88__
