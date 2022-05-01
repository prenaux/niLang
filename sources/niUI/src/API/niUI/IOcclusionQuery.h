#ifndef __IOCCLUSIONQUERY_84092360_H__
#define __IOCCLUSIONQUERY_84092360_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */

//! Occlusion query status.
enum eOcclusionQueryStatus
{
  //! No query has been issued yet.
  eOcclusionQueryStatus_NotIssued = 0,
  //! The query has began but is not ended yet.
  eOcclusionQueryStatus_Began = 1,
  //! Waiting for the query to return.
  eOcclusionQueryStatus_Pending = 2,
  //! The query status is available.
  eOcclusionQueryStatus_Successful = 3,
  //! The query failed.
  eOcclusionQueryStatus_Failed = 4,
  //! \internal
  eOcclusionQueryStatus_ForceDWORD = 0xFFFFFFFF
};

//! Occlusion query object.
struct iOcclusionQuery : public iUnknown
{
  niDeclareInterfaceUUID(iOcclusionQuery,0x9bda9d89,0x23e7,0x4cf7,0xaa,0xa7,0xc9,0x5b,0xa7,0xed,0xae,0x2a)
  //! Begin the query.
  virtual tBool __stdcall Begin() = 0;
  //! End the query.
  virtual tBool __stdcall End() = 0;
  //! Get the status of the query.
  //! \param abWait if eTrue the function will wait until the query is successful or fails.
  virtual eOcclusionQueryStatus __stdcall GetStatus(tBool abWait) = 0;
  //! Get the result of the query.
  //! \return the number of pixels drawn between begin/end. eInvalidHandle
  //!         if the result is not yet available.
  //! {Property}
  virtual tU32 __stdcall GetResult() const = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IOCCLUSIONQUERY_84092360_H__
