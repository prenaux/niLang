#ifndef __DATATABLEPATH_81D5B3B6_B043_443B_8400_3903FE2804C7_H__
#define __DATATABLEPATH_81D5B3B6_B043_443B_8400_3903FE2804C7_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/ILang.h>
#include <niLang/IRegex.h>
#include <niLang/Utils/SmartPtr.h>
#include <niLang/STL/list.h>
#include "API/niLang/IDataTable.h"

namespace ni {

class cDataTablePathOp;
class cDataTablePath;

//! Data table path operation pointer.
typedef Ptr<cDataTablePathOp> tDataTablePathOpPtr;
//! Data table path operation list.
typedef astl::list<tDataTablePathOpPtr> tDataTablePathOpPtrCLst;
//! Data table path operation list iterator.
typedef tDataTablePathOpPtrCLst::iterator     tDataTablePathOpPtrCLstIt;
//! Data table path operation list const iterator.
typedef tDataTablePathOpPtrCLst::const_iterator tDataTablePathOpPtrCLstCIt;
//! Data table path operation list reverse iterator.
typedef tDataTablePathOpPtrCLst::reverse_iterator tDataTablePathOpPtrCLstRIt;
//! Data table path operation list const reverse iterator.
typedef tDataTablePathOpPtrCLst::const_reverse_iterator tDataTablePathOpPtrCLstCRIt;

//! DataTable path interface smart pointer.
typedef Ptr<cDataTablePath> tDataTablePathPtr;

//////////////////////////////////////////////////////////////////////////////////////////////
// cDataTablePathOp declaration.

//! DataTable path result type.
enum eDataTablePathResultType
{
  //! Data table path result type, no result.
  eDataTablePathResultType_None = 0,
  //! Data table path result type, data table.
  eDataTablePathResultType_DataTable = 1,
  //! Data table path result type, no result.
  eDataTablePathResultType_Property = 2,
  //! \internal
  eDataTablePathResultType_ForceDWORD = 0xFFFFFFFF
};

//! DataTable path operations.
enum eDataTablePathOp
{
  //! Path operation, root.
  eDataTablePathOp_Root = 0,
  //! Path operation, current.
  eDataTablePathOp_Current = 1,
  //! Path operation, parent.
  eDataTablePathOp_Parent = 2,
  //! Path operation, datatable.
  eDataTablePathOp_DataTable = 3,
  //! Path operation, property.
  eDataTablePathOp_Property = 4,
  //! \internal
  eDataTablePathOp_Last = 5,
  //! \internal
  eDataTablePathOp_ForceDWORD = 0xFFFFFFFF
};

//! DataTable path operation interface.
class cDataTablePathOp : public cIUnknownImpl<iUnknown>
{
  niBeginClass(cDataTablePathOp);

 public:
  //! Constructor.
  cDataTablePathOp(eDataTablePathOp aPathOp, const achar* aaszValue);
  //! Destructor.
  ~cDataTablePathOp();

  //! Zeros all the class members.
  void ZeroMembers();

  //! Sanity check.
  tBool __stdcall IsOK() const;

  //// iDataTablePathOp ///////////////////////////
  eDataTablePathOp __stdcall GetOp() const;
  iRegex* __stdcall GetRegex() const;
  //// iDataTablePathOp ///////////////////////////

  const cString& __stdcall GetValue() const;

 public:
  eDataTablePathOp  mOp;
  cString             mstrValue;
  Ptr<iRegex> mptrRegex;

  niEndClass(cDataTablePathOp);
};

//////////////////////////////////////////////////////////////////////////////////////////////
// cDataTablePath declaration.

//! DataTable path implementation.
class cDataTablePath : public cIUnknownImpl<iUnknown>
{
  niBeginClass(cDataTablePath);

 public:
  //! Constructor.
  cDataTablePath(iHString* ahspPath);
  //! Destructor.
  ~cDataTablePath();

  //! Zeros all the class members.
  void ZeroMembers();

  //! Sanity check.
  tBool __stdcall IsOK() const;

  //// iDataTablePath /////////////////////////////
  void __stdcall SetPath(iHString* ahspPath);
  iHString* __stdcall GetPath() const;
  tBool __stdcall Evaluate(iDataTable* apDT);
  tBool __stdcall EvaluateEx(iHString* ahspPath, iDataTable* apDT);
  eDataTablePathResultType __stdcall GetResultType() const;
  iDataTable* __stdcall GetResultDataTable() const;
  tU32 __stdcall GetResultPropertyIndex() const;
  void __stdcall ClearResult();
  cString __stdcall GetRootPathToDataTable(iDataTable* apDT, tU32 anPropIndex) const;
  tBool __stdcall CreatePathInDataTable(iDataTable* apDT, tBool abCompletlyNew);
  //// iDataTablePath /////////////////////////////

 private:
  Ptr<iHString>     mhspPath;
  eDataTablePathResultType  mResultType;
  Ptr<iDataTable>   mptrResultDataTable;
  tU32            mnResultIndex;
  tDataTablePathOpPtrCLst     mlstOps;

  niEndClass(cDataTablePath);
};

} // end of namespace ni
/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __DATATABLEPATH_81D5B3B6_B043_443B_8400_3903FE2804C7_H__
