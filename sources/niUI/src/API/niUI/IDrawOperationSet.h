#ifndef __DRAWOPERATIONSET_842634_H__
#define __DRAWOPERATIONSET_842634_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */

//! Draw operation set.
struct iDrawOperationSet : public iUnknown
{
  niDeclareInterfaceUUID(iDrawOperationSet,0xc5ce498a,0x186a,0x49ed,0x99,0x2c,0xc0,0x0b,0x3b,0x65,0xfb,0x58)

  //! Clear the operation set.
  //! \remark Invalidates the iterator
  virtual void __stdcall Clear() = 0;
  //! Insert a draw operation in the set.
  //! \remark Invalidates the iterator
  //! \return The draw operation inserted
  virtual iDrawOperation* __stdcall Insert(iDrawOperation* apDO) = 0;
  //! Insert another set in the set.
  //! \remark Invalidates the iterator
  virtual tBool __stdcall InsertSet(const iDrawOperationSet* apSet) = 0;

  //! Get the number of elements in the set.
  //! {Property}
  virtual tU32 __stdcall GetNumDrawOperations() const = 0;

  //! Go to the first iterator and returns it.
  virtual iDrawOperation* __stdcall Begin() = 0;
  //! Move to the next draw op and returns it.
  virtual iDrawOperation* __stdcall Next() = 0;
  //! Return eTrue if the iterator is the end of the set.
  virtual tBool __stdcall IsEnd() const = 0;
  //! Get the current iterator.
  //! {Property}
  virtual iDrawOperation* __stdcall GetCurrent() const = 0;

  //! Check whether the draw operation set is empty.
  //! {Property}
  virtual tBool __stdcall GetIsEmpty() const = 0;

  //! Draw all draw operation of the set with specified graphics context.
  //! \param apContext the context to draw to
  //! \param apFrustum if specified frustum culling is applied to the draw operation that have a valid bounding volume
  virtual tU32 __stdcall Draw(iGraphicsContext* apContext, iFrustum* apFrustum) = 0;

  //! Draw all draw operation of the set with specified graphics context and matrix set on each draw op.
  //! \param aMatrix the matrix to set on the draw ops
  //! \param apContext the context to draw to
  //! \param apFrustum if specified frustum culling is applied to the draw operation that have a valid bounding volume
  virtual tU32 __stdcall XDraw(const sMatrixf& aMatrix, iGraphicsContext* apContext, iFrustum* apFrustum) = 0;
};

//! Draw operation set smart pointer.
typedef Ptr<iDrawOperationSet>  tDrawOperationSetPtr;

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __DRAWOPERATIONSET_842634_H__
