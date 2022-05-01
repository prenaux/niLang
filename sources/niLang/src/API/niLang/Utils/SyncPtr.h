#ifndef __SYNCPTR_4994731_H__
#define __SYNCPTR_4994731_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "SmartPtr.h"
#include "../Utils/ThreadImpl.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

//! Synchronized pointer.
template <typename T>
struct SyncPtr
{
  SyncPtr() = delete;

  SyncPtr(volatile const T& aObj, const ThreadMutex& aMutex)
      : mMutex(aMutex)
  {
    mpObj = const_cast<T*>(&aObj);
    mMutex.ThreadLock();
  }
  ~SyncPtr() {
    mMutex.ThreadUnlock();
  }

  //! Pointer dereferencing.
  T& operator * () {
    return *mpObj;
  }
  //! Pointer access.
  T* operator -> () {
    return mpObj;
  }

  //! Pointer casting.
  T* ptr() const { return mpObj; }
  T** ptrptr() const { return &mpObj; }

  // Check whether the pointer holds a valid object.
  bool IsOK() const {
    return mpObj != NULL;
  }

 private:
  T*  mpObj;
  const ThreadMutex& mMutex;

  operator T& ();
  operator const T& () const;
  T& operator = (const T& aRight);
  T& operator = (T& aRight);
  T& operator = (const T* aRight);
  T& operator = (T* aRight);
  operator bool () const;
  operator int () const;
  bool operator == (int) const;
  bool operator != (int) const;

  niClassStrictLocal(SyncPtr);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __SYNCPTR_4994731_H__
