#ifndef __IINDEXARRAY_39242085_H__
#define __IINDEXARRAY_39242085_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/IDeviceResource.h>
#include "GraphicsEnum.h"

namespace ni {

struct iIndexArray;

/** \addtogroup niUI
 * @{
 */

//! Index array interface
//! \remark Bindable
struct iIndexArray : public iDeviceResource
{
  niDeclareInterfaceUUID(iIndexArray,0xa1ab22bf,0x95b1,0x43fe,0x92,0x3c,0x9e,0xef,0x84,0xca,0x83,0x41)

  //########################################################################################
  //! \name General
  //########################################################################################
  //! @{

  //! Get the primitive type of this index array.
  //! {Property}
  virtual eGraphicsPrimitiveType __stdcall GetPrimitiveType() const = 0;
  //! Get the number of indices in the given index array.
  //! {Property}
  virtual tU32 __stdcall GetNumIndices() const = 0;
  //! Get the maximum vertex index that can be used in this index array.
  //! {Property}
  virtual tU32 __stdcall GetMaxVertexIndex() const = 0;
  //! Get the usage of the given vertex array.
  //! {Property}
  virtual eArrayUsage __stdcall GetUsage() const = 0;
  //! @}

  //########################################################################################
  //! \name Locking
  //########################################################################################
  //! @{

  //! Lock index array memory to enable writting and reading in it.
  //! \param  ulFirstIndex is the offset in the index data to lock.
  //! \param  ulNumIndex is the number of index to lock.
  //! \param  aLock specify the locking mode.
  //! \return a pointer to the index data.
  //! \remark pass 0 for ulFirstIndex and ulNumIndex to lock the entire array.
  //! \remark This method should be considered as slow as it may perform some processing to
  //!     adapt the indices data to the renderer's supported format.
  virtual tPtr __stdcall Lock(tU32 ulFirstIndex, tU32 ulNumIndex, eLock aLock) = 0;
  //! Unlock vertex array memory.
  //! \remark This method should be considered as slow as it may perform some processing to
  //!     adapt the indices data to the renderer's supported format.
  virtual tBool __stdcall Unlock() = 0;
  //! Return true if the array is locked.
  //! {Property}
  virtual tBool __stdcall GetIsLocked() const = 0;
  //! @}
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IINDEXARRAY_39242085_H__
