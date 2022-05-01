#ifndef __IDGENERATOR_24666734_H__
#define __IDGENERATOR_24666734_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Types.h>
#include <niLang/STL/stack.h>
#include <niLang/Utils/UnknownImpl.h>
#include <niLang/Utils/Sync.h>

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

//! ID generator template.
template <typename T, typename BASE = cEmpty>
struct IDGeneratorTpl : public BASE
{
 protected:
  IDGeneratorTpl(tSize aFreeListPopSize) : _freelistPopSize(aFreeListPopSize) {
    _counter = 0;
    // reserve ID 0, aka "NULL" sould be considered invalid
    this->AllocID();
  }
  ~IDGeneratorTpl() {
  }

 public:
  T __stdcall AllocID() {
    __sync_lock();
    if (_freelist.size() > _freelistPopSize) {
      T r = _freelist.top();
      _freelist.pop();
      return r;
    }
    else {
      return _counter++;
    }
  }
  tBool __stdcall FreeID(T aID) {
    niAssert(aID != 0);
    __sync_lock();
    //niAssert(aID < _counter);
    if (aID >= _counter) return eFalse;
    _freelist.push(aID);
    return eTrue;
  }

  T __stdcall GetCounter() const {
    __sync_lock();
    return _counter;
  }

 private:
  __sync_mutex();
  const tSize   _freelistPopSize;
  T       _counter;
  astl::stack<T>  _freelist;
};

//! ID generator.
struct IDGenerator : public IDGeneratorTpl<tU32,cIUnknownImpl<iUnknown> >
{
  IDGenerator* Create(tSize aFreeListPopSize = 10) {
    return niNew IDGenerator(aFreeListPopSize);
  }

 private:
  IDGenerator(tSize aFreeListPopSize = 10) : IDGeneratorTpl<tU32,cIUnknownImpl<iUnknown> >(aFreeListPopSize) {}
};

//! Local ID generator.
struct LocalIDGenerator : public IDGeneratorTpl<tU32,cEmpty>
{
  LocalIDGenerator(tSize aFreeListPopSize = 10) : IDGeneratorTpl<tU32,cEmpty>(aFreeListPopSize) {}
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __IDGENERATOR_24666734_H__
