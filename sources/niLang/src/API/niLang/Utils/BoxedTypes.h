#ifndef __BOXEDTYPES_11915672_H__
#define __BOXEDTYPES_11915672_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

//! Atomic type box
template <typename T>
struct BoxAtomic
{
  BoxAtomic() {}
  BoxAtomic(T v) : _value(v) {}
  operator T& () { return _value; }
  operator const T& () const { return _value; }
  T& operator = (const T& aRight) {
    _value = aRight;
    return *this;
  }
  operator bool()       { return _value != 0; };
  operator bool() const   { return _value != 0; };
  bool  operator !()      { return _value == 0; };
  bool  operator !() const  { return _value == 0; };

  T* Ptr() const { return (T*)&_value; }
  T* Ptr() const niSync { return (T*)&_value; }

 private:
  T _value;
};


//! Atomic pointer type box
template<class T>
class BoxPtrAtomic
{
 public:
  BoxPtrAtomic() {}
  BoxPtrAtomic(T* _p) : mPtr(_p) {}
  BoxPtrAtomic(const BoxPtrAtomic<T>& _p) : mPtr(_p.mPtr) {}
  ~BoxPtrAtomic() {}

  // Casting to normal pointer.
  operator     T* ()    { return mPtr; }
  // Casting to normal constant pointer.
  operator const T* () const  { return mPtr; }

  // Dereference operator, allow dereferencing boxed pointer just like regular pointer.
  T& operator * () { return *mPtr; }
  const T& operator * () const { return *mPtr; }

  // Arrow operator, allow to use regular C syntax to access members of class.
  T* operator -> (void) { return mPtr; }
  const T* operator -> (void) const { return mPtr; }

  // Replace pointer.
  BoxPtrAtomic&  operator = (T* newp) {
    mPtr = newp;
    return *this;
  }

  // Replace pointer.
  BoxPtrAtomic&  operator = (const BoxPtrAtomic<T> &newp) {
    mPtr = newp.mPtr;
    return *this;
  }

  T* Ptr() const { return (T*)mPtr; }
  T* Ptr() const niSync { return (T*)mPtr; }
  T** PtrAddr() const { return const_cast<T**>(&mPtr); }
  T** PtrAddr() const niSync { return const_cast<T**>(&mPtr); }

  // Cast to boolean, simplify if statements with smart pointers.
  operator bool()       { return mPtr != NULL; };
  operator bool() const   { return mPtr != NULL; };
  bool  operator !()      { return mPtr == NULL; };
  bool  operator !() const  { return mPtr == NULL; };

 private:
  T* mPtr;
};

typedef BoxAtomic<tI8>    tBoxedI8;
typedef BoxAtomic<tU8>    tBoxedU8;
typedef BoxAtomic<tI16>   tBoxedI16;
typedef BoxAtomic<tU16>   tBoxedU16;
typedef BoxAtomic<tI32>   tBoxedI32;
typedef BoxAtomic<tU32>   tBoxedU32;
typedef BoxAtomic<tI64>   tBoxedI64;
typedef BoxAtomic<tU64>   tBoxedU64;
typedef BoxAtomic<tPtr>   tBoxedPtr;
typedef BoxAtomic<tSize>  tBoxedSize;
typedef BoxAtomic<tOffset>  tBoxedOffset;
typedef BoxAtomic<tIntPtr>  tBoxedIntPtr;
typedef BoxAtomic<tF32>   tBoxedF32;
typedef BoxAtomic<tF64>   tBoxedF64;
typedef BoxAtomic<tVersion> tBoxedVersion;
typedef BoxAtomic<tEnum>  tBoxedEnum;
typedef BoxAtomic<tHandle>  tBoxedHandle;
typedef BoxAtomic<tFloat> tBoxedFloat;
typedef BoxAtomic<tUUID>  tBoxedUUID;
typedef BoxAtomic<tBool>  tBoxedBool;
typedef BoxAtomic<tInt>   tBoxedInt;
typedef BoxAtomic<tUInt>  tBoxedUInt;
typedef BoxAtomic<tIndex> tBoxedIndex;
typedef BoxAtomic<cchar>  tBoxedCChar;
typedef BoxAtomic<gchar>  tBoxedGChar;
typedef BoxAtomic<xchar>  tBoxedXChar;
typedef BoxAtomic<uchar>  tBoxedUChar;
typedef BoxAtomic<achar>  tBoxedAChar;
typedef BoxAtomic<tType>  tBoxedType;

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __BOXEDTYPES_11915672_H__
