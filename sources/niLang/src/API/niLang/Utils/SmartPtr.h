#ifndef __SMARTPTR_24481621_H__
#define __SMARTPTR_24481621_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Ptr
 * @{
 */

template <typename T>
struct WeakPtr;

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
      mPtr->AddRef();
  }
  Ptr(const Ptr<T>& _p) : mPtr(_p.mPtr) {
    if (mPtr)
      mPtr->AddRef();
  }
  Ptr(const WeakPtr<T>& aP);

  ~Ptr()  {
    if (mPtr)
      mPtr->Release();
  }

  // Assignment operator
  Ptr& operator = (T* newp) {
    Swap(newp);
    return *this;
  }
  Ptr&  operator = (const Ptr<T> &newp) {
    Swap(newp.mPtr);
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
    niAssert(mPtr != NULL);
    return *mPtr;
  }

  // Arrow operator, allow to use regular C syntax to access members of class.
  T* operator -> (void) const {
    niAssert(mPtr != NULL);
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
  T* ptr() const { return const_cast<T*>(mPtr);  }
  T** ptrptr() const { return const_cast<T**>(&mPtr);  }

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

template<class T, class U> inline bool operator==(Ptr<T> const& a, Ptr<U> const& b) {
  return a.ptr() == b.ptr();
}
template<class T, class U> inline bool operator!=(Ptr<T> const& a, Ptr<U> const& b) {
  return a.ptr() != b.ptr();
}
template<class T, class U> inline bool operator<(Ptr<T> const& a, Ptr<U> const& b) {
  return a.ptr() < b.ptr();
}
template<class T, class U> inline bool operator>(Ptr<T> const& a, Ptr<U> const& b) {
  return a.ptr() > b.ptr();
}
template<class T, class U> inline bool operator<=(Ptr<T> const& a, Ptr<U> const& b) {
  return a.ptr() <= b.ptr();
}
template<class T, class U> inline bool operator>=(Ptr<T> const& a, Ptr<U> const& b) {
  return a.ptr() >= b.ptr();
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

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}; // End of ni
#endif // __SMARTPTR_24481621_H__
