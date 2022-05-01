#pragma once
#ifndef __ICOLLECTION_H_EC13D733_C7E7_4E18_BDDA_F61DFEE721D3__
#define __ICOLLECTION_H_EC13D733_C7E7_4E18_BDDA_F61DFEE721D3__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "Types.h"

namespace ni {

struct iCollection;
struct iIterator;

/** \addtogroup niLang
 * @{
 */

//! Collection iterator interface.
struct iIterator : public iUnknown {
  niDeclareInterfaceUUID(iIterator,0xb33c7b41,0x8edd,0x4699,0x95,0x38,0x0e,0x2f,0x9c,0xf4,0xe5,0x43);

  //! Get the collection in which the iterator operates.
  //! {Property}
  virtual iCollection* __stdcall GetCollection() const = 0;
  //! Check whether their is more elements to iterate.
  virtual tBool __stdcall HasNext() const = 0;
  //! Get the next value.
  virtual const Var& __stdcall Next() = 0;
  //! Get the current key.
  virtual const Var& __stdcall Key() const = 0;
  //! Get the current value.
  virtual const Var& __stdcall Value() const = 0;
};

//! Immutable Collection interface.
struct iCollection : public iUnknown {
  niDeclareInterfaceUUID(iCollection,0x9917d8a6,0xa85f,0x4f30,0xb6,0x68,0xe3,0x17,0x6e,0x33,0x4b,0xf2);

  //! Get the collection's key type.
  //! {Property}
  //! \remark Only Associative collections return a valid type
  //!         here. Although Vector collections can use Put/Get with
  //!         an index, it is not considered a "key" type.
  virtual tType __stdcall GetKeyType() const = 0;
  //! Get the collection's value type.
  //! {Property}
  virtual tType __stdcall GetValueType() const = 0;

  //! Returns true if this collection contains no elements.
  virtual tBool __stdcall IsEmpty() const = 0;
  //! Returns the number of elements in this collection.
  virtual tU32 __stdcall GetSize() const = 0;

  //! Returns true if this collection contains the specified key.
  virtual tBool __stdcall Contains(const Var& aVar) const = 0;
  //! Returns true if this collection contains all of the elements in the specified collection.
  virtual tBool __stdcall ContainsAll(const iCollection* apCollection) const = 0;

  //! Returns an iterator over the elements in this collection.
  virtual iIterator* __stdcall Iterator() const = 0;
  //! Returns an iterator over starting at the specified elements in this collection.
  virtual iIterator* __stdcall Find(const Var& aVar) const = 0;

  //! Returns the value to which the specified key is mapped, or
  //! null if this map contains no mapping for the key.
  virtual Var __stdcall Get(const Var& aKey) const = 0;

  //! Returns the first value in the container.
  //! {Property}
  virtual Var __stdcall GetFirst() const = 0;
  //! Returns the last value in the container.
  //! {Property}
  virtual Var __stdcall GetLast() const = 0;
};

//! Mutable Collection interface.
struct iMutableCollection : public iCollection {
  niDeclareInterfaceUUID(iMutableCollection,0xf88402a3,0xd533,0x426e,0xbe,0x12,0xe6,0x9f,0xdf,0xea,0x0a,0x66);

  //! Removes all of the elements from this collection.
  virtual tBool __stdcall Clear() = 0;

  //! Copy a collection of the same type in this collection.
  //! \remark Invalidates all the iterators.
  //! \return eFalse if the specified collection cannot be copied in this collection.
  virtual tBool __stdcall Copy(const iCollection* apCollection) = 0;

  //! Reserve memory for the specified number of elements.
  virtual tBool __stdcall Reserve(tU32 anSize) = 0;
  //! Resize the collection to the specified number of elements.
  virtual tBool __stdcall Resize(tU32 anSize) = 0;

  //! Ensures that this collection contains the specified key.
  virtual tBool __stdcall Add(const Var& aVar) = 0;
  //! Adds all of the elements in the specified collection to this collection.
  virtual tBool __stdcall AddAll(const iCollection* apCollection) = 0;

  //! Removes a single instance of the specified key from this
  //! collection, if it is present (optional operation).
  virtual tBool __stdcall Remove(const Var& aVar) = 0;
  //! Removes the element pointed by the specified iterator.
  virtual tBool __stdcall RemoveIterator(iIterator* apIterator) = 0;
  //! Removes all of this collection's elements that are also contained in the specified collection.
  virtual tBool __stdcall RemoveAll(const iCollection* apCollection) = 0;

  //! Retrieve and remove the first element of the container.
  virtual tBool __stdcall RemoveFirst() = 0;
  //! Retrieve and remove the last element of the container.
  virtual tBool __stdcall RemoveLast() = 0;

  //! Associates the specified value with the specified key.
  virtual tBool __stdcall Put(const Var& aKey, const Var& aValue) = 0;

  //! Sets the first value in the container.
  //! {Property}
  virtual tBool __stdcall SetFirst(const Var& aValue) = 0;
  //! Sets the last value in the container.
  //! {Property}
  virtual tBool __stdcall SetLast(const Var& aValue) = 0;

  //! Get a pointer to continous memory that can be read/written directly.
  //! \remark Optional
  virtual tPtr __stdcall GetDataPtr() const = 0;
  //! Get the size in bytes of the memory pointed by the data pointer.
  //! \remark Optional
  virtual tSize __stdcall GetDataSize() const = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __ICOLLECTION_H_EC13D733_C7E7_4E18_BDDA_F61DFEE721D3__
