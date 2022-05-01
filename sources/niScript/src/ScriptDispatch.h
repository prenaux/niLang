#ifndef __SCRIPTDISPATCH_82111440_H__
#define __SCRIPTDISPATCH_82111440_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "sqobject.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// cScriptDispatch declaration.

//! Script engine iDispatch implementation.
class cScriptDispatch : public cIUnknownImpl<iDispatch,eIUnknownImplFlags_Aggregatable|eIUnknownImplFlags_NoRefCount>
{
  niBeginClass(cScriptDispatch);

 protected:
  //! Constructor.
  cScriptDispatch(SQTable* apTable);
  //! Destructor.
  ~cScriptDispatch();

 public:
  //! Sanity check.
  tBool __stdcall IsOK() const;

  //! Query an interface.
  iUnknown* __stdcall QueryInterface(const tUUID& aIID) niImpl;
  //! Return a new list containing the implemented interfaces.
  void __stdcall ListInterfaces(iMutableCollection* apLst, tU32) const niImpl;

  //// iDispatch ////////////////////////////////
  tBool __stdcall InitializeMethods(const sMethodDef* const* apMethods, ni::tU32 anNumMethods);
  tBool __stdcall CallMethod(const sMethodDef* const apMethodDef, ni::tU32 anMethodIndex, const Var* apParameters, tU32 anNumParameters, Var* apRet);
  //// iDispatch ////////////////////////////////

  virtual tI32 __stdcall AddRef();
  virtual tI32 __stdcall Release();
  virtual tI32 __stdcall SetNumRefs(tI32 anNumRefs);
  virtual tI32 __stdcall GetNumRefs() const;

  void _Unregister(iUnknown* apWrapper);

  SQTable* _GetTable() { return down_cast<SQTable*>(mprotected_pAggregateParent); }
  const SQTable* _GetTable() const { return down_cast<const SQTable*>(mprotected_pAggregateParent); }

  iUnknown* __stdcall _CreateInterfaceDispatchWrapper(const sInterfaceDef* apDef);

 private:
  friend class cScriptAutomation;

  __sync_mutex();
  typedef astl::map<const sMethodDef*, SQObjectPtr> tMethodMap;
  typedef astl::map<tUUID,iUnknown*> tInterfaceMap;
  tMethodMap    mmapMethods;
  tInterfaceMap mmapInterfaces;
  Ptr<iScriptObject> mptrObj;
  HSQUIRRELVM   mOwnerVM;
  tBool         mbDidPrintMultiThreadedWarning;

  niEndClass(cScriptDispatch);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __SCRIPTDISPATCH_82111440_H__
