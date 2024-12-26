#ifndef __IVERTEXARRAY_10143733_H__
#define __IVERTEXARRAY_10143733_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/IDeviceResource.h>
#include "GraphicsEnum.h"
#include "FVF.h"

namespace ni {

struct iVertexArray;

/** \addtogroup niUI
 * @{
 */

//! Vertex Array interface.
//! \remark Bindable
struct iVertexArray : public iDeviceResource
{
  niDeclareInterfaceUUID(iVertexArray,0x1c847f71,0x312f,0x41ea,0x97,0xb9,0xab,0x29,0xb0,0xc3,0x81,0x21)

  //########################################################################################
  //! \name General
  //########################################################################################
  //! @{

  //! Get the FVF of the vertex array.
  //! {Property}
  virtual tFVF __stdcall GetFVF() const = 0;
  //! Get the number of vertices in the vertex array.
  //! {Property}
  virtual tU32 __stdcall GetNumVertices() const = 0;
  //! Get the usage of the given vertex array.
  //! {Property}
  virtual eArrayUsage __stdcall GetUsage() const = 0;
  //! @}

  //########################################################################################
  //! \name Locking
  //########################################################################################
  //! @{

  //! Lock vertex array memory to enable writting and reading in it.
  //! \param  ulFirstVertex is the offset in the vertex data to lock.
  //! \param  ulNumVertex is the number of vertex to lock.
  //! \param  aLock specify to locking mode.
  //! \return a pointer to the vertex data.
  //! \remark pass 0 for ulFirstVertex and ulNumVertex to lock the entire array.
  virtual tPtr __stdcall Lock(tU32 ulFirstVertex, tU32 ulNumVertex, eLock aLock) = 0;
  //! Unlock vertex array memory.
  virtual tBool __stdcall Unlock() = 0;
  //! Return true if the array is locked.
  //! {Property}
  virtual tBool __stdcall GetIsLocked() const = 0;
  //! @}
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IVERTEXARRAY_10143733_H__
