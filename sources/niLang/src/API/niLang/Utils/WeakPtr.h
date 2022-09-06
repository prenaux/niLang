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
niExportFunc(iUnknown*) ni_object_deref_weak_ptr(iUnknown* apWeakPtr);
niExportFunc(iUnknown*) ni_object_deref_and_add_ref_weak_ptr(iUnknown* apWeakPtr);

/**
 * A weak reference to an iUnknown instance.
 *
 * The underlying object can be accessed dereferencing it with QPtr, Deref or niCheckNonnull.
 */
template <typename T>
struct WeakPtr {
  niClassNoHeapAlloc(WeakPtr);
  template<class U> friend struct QPtr;
  template<class U> friend struct Nonnull;

 public:
  WeakPtr() {}
  WeakPtr(std::nullptr_t) { *this = nullptr; }
  WeakPtr(const T* other) { *this = other; }
  WeakPtr(const Ptr<T>& other) { *this = other; }
  WeakPtr(const QPtr<T>& other) { *this = other; }
  WeakPtr(const Nonnull<T>& other) { *this = other; }

  WeakPtr& operator=(std::nullptr_t) {
    this->_Set(nullptr);
    return *this;
  }
  WeakPtr& operator=(const T* other) {
    this->_Set(other);
    return *this;
  }
  WeakPtr& operator=(const Ptr<T>& other) {
    this->_Set(other.raw_ptr());
    return *this;
  }
  WeakPtr& operator=(const QPtr<T> other) {
    this->_Set(other.raw_ptr());
    return *this;
  }
  WeakPtr& operator=(const Nonnull<T> other) {
    this->_Set(other.raw_ptr());
    return *this;
  }

  WeakPtr(WeakPtr&& other) = default;
  WeakPtr(const WeakPtr& other) = default;
  WeakPtr& operator=(const WeakPtr& other) = default;

  tBool IsOK() const {
    return mWeakPtrObject.IsOK();
  }
  tBool IsSet() const {
    return mWeakPtrObject.ptr() != NULL;
  }

  void SetNull() {
    mWeakPtrObject = NULL;
  }

  __forceinline T* Deref() const {
    return (T*)(niTypename T::IUnknownBaseType*)
        ni_object_deref_weak_ptr(mWeakPtrObject.ptr());
  }

  tBool is_null() const {
    return !IsSet();
  }
  tBool has_value() const {
    return IsSet();
  }
  ni::Nonnull<T> non_null() const {
    return ni::Nonnull<T>(this->Deref());
  }
  ni::Nonnull<const T> c_non_null() const {
    return ni::Nonnull<const T>(this->Deref());
  }

  __forceinline iUnknown* weak_object_ptr() const {
    return mWeakPtrObject.raw_ptr();
  }

 private:
  __forceinline void _Set(const T* apObject) {
    mWeakPtrObject = apObject ? ni_object_get_weak_ptr(
      (niTypename T::IUnknownBaseType*)apObject) : NULL;
  }

  __forceinline T* _DerefAndAddRef() const {
    return (T*)(niTypename T::IUnknownBaseType*)
        ni_object_deref_and_add_ref_weak_ptr(mWeakPtrObject.ptr());
  }

  Ptr<iUnknown> mWeakPtrObject;

  // should use Swap & Reset() to change the underlying pointer
  WeakPtr(void*) = delete;
  WeakPtr(int) = delete;

  // Prevent if (PTR), if (!PTR), if (PTR == 0/NULL), if (PTR != 0/NULL)
  operator bool () const = delete;
  bool operator !() const = delete;
  bool operator == (int) const = delete;
  bool operator != (int) const = delete;

  // To confuse the compiler if someone tries to delete the smart pointer
  operator void* () const = delete;

  // unwanted operators...pointers only point to single objects!
  WeakPtr& operator++() = delete;
  WeakPtr& operator--() = delete;
  WeakPtr operator++(int) = delete;
  WeakPtr operator--(int) = delete;
  WeakPtr& operator+=(std::ptrdiff_t) = delete;
  WeakPtr& operator-=(std::ptrdiff_t) = delete;
  void operator[](std::ptrdiff_t) const = delete;
};

// more unwanted operators
template <class T, class U>
std::ptrdiff_t operator-(const WeakPtr<T>&, const WeakPtr<U>&) = delete;
template <class T>
WeakPtr<T> operator-(const WeakPtr<T>&, std::ptrdiff_t) = delete;
template <class T>
WeakPtr<T> operator+(const WeakPtr<T>&, std::ptrdiff_t) = delete;
template <class T>
WeakPtr<T> operator+(std::ptrdiff_t, const WeakPtr<T>&) = delete;

template <typename T>
__forceinline bool IsNullPtr(WeakPtr<T> const& a) {
  return (!a.IsSet());
}

template<class T, class U> inline bool operator==(WeakPtr<T> const& a, WeakPtr<U> const& b) {
  return a.weak_object_ptr() == b.weak_object_ptr();
}
template<class T, class U> inline bool operator!=(WeakPtr<T> const& a, WeakPtr<U> const& b) {
  return a.weak_object_ptr() != b.weak_object_ptr();
}
template<class T, class U> inline bool operator<(WeakPtr<T> const& a, WeakPtr<U> const& b) {
  return a.weak_object_ptr() < b.weak_object_ptr();
}
template<class T, class U> inline bool operator>(WeakPtr<T> const& a, WeakPtr<U> const& b) {
  return a.weak_object_ptr() > b.weak_object_ptr();
}
template<class T, class U> inline bool operator<=(WeakPtr<T> const& a, WeakPtr<U> const& b) {
  return a.weak_object_ptr() <= b.weak_object_ptr();
}
template<class T, class U> inline bool operator>=(WeakPtr<T> const& a, WeakPtr<U> const& b) {
  return a.weak_object_ptr() >= b.weak_object_ptr();
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}; // End of ni
#endif // __WEAKPTR_H_2C1983A5_FD14_47C4_BE9C_92D3D12B0F88__
