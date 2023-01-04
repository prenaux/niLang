#ifndef __SCRIPTVM_2181373_H__
#define __SCRIPTVM_2181373_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Types.h>
#include "sqconfig.h"
#include "ScriptAutomation.h"

class cScriptObject;

#define SCRIPTOBJECT_INVALIDATEPOLICY

#define GUARD 0

#define niSqGuardEx(v)                                                  \
  int __sq_top = sq_gettop(v); cString __sq_stack_str; sqGetStackDump(__sq_stack_str,v); const achar* __sq_stack = __sq_stack_str.Chars();
#define niSqGuardEx_(v,op)                                              \
  int __sq_top = sq_gettop(v) op; cString __sq_stack_str; sqGetStackDump(__sq_stack_str,v); const achar* __sq_stack = __sq_stack_str.Chars();
#define niSqUnGuardEx(v)                                                \
  if (sq_gettop(v) != __sq_top)                                         \
  {                                                                     \
  cString __sq_cur_stack_str; sqGetStackDump(__sq_cur_stack_str,v); const achar* __sq_cur_stack = __sq_cur_stack_str.Chars(); \
  niAssertMsg((sq_gettop(v) == __sq_top),niFmt(_A("VM stack corrupted, expected %d, have %d.\n--- START STACK ---\n%s\n--- CURRENT STACK ---\n%s)",__sq_top,sq_gettop(v),__sq_stack,__sq_cur_stack_str.Chars())); \
              }

#if GUARD == 2
#define niSqGuard(v)    niSqGuardEx(v)
#define niSqGuard_(v,op)  niSqGuardEx_(v,op)
#define niSqUnGuard(v)    niSqUnGuardEx(v)
#pragma message("==================================================================")
#pragma message("WARNING: GUARD 2 Actived, the vm speed will be greatly decreased.")
#pragma message("WARNING: GUARD 2 Actived, the vm speed will be greatly decreased.")
#pragma message("WARNING: GUARD 2 Actived, the vm speed will be greatly decreased.")
#pragma message("==================================================================")
#elif GUARD == 1
#define niSqGuard(v)    niSqGuardSimple(v)
#define niSqGuard_(v,op)  niSqGuardSimple_(v,op)
#define niSqUnGuard(v)    niSqUnGuardSimple(v)
#else
#define niSqGuard(v)
#define niSqGuard_(v,op)
#define niSqUnGuard(v)
#endif

ASTL_RAW_ALLOCATOR_IMPL(scriptobjectlist);

//////////////////////////////////////////////////////////////////////////////////////////////
// cScriptVM declaration.

//! Script VM.
class cScriptVM : public cIUnknownImpl<iScriptVM,eIUnknownImplFlags_DontInherit1,iScriptingHost>
{
  niBeginClass(cScriptVM);

 public:
  cScriptVM(cScriptVM* apParentVM);
  ~cScriptVM();

  //! Zeros all the class members.
  void ZeroMembers();

  //! Sanity check.
  tBool __stdcall IsOK() const;
  //! Invalidate.
  void __stdcall Invalidate();

  //// iScriptVM ////////////////////////////////
  tIntPtr __stdcall GetHandle() const { return (tIntPtr)mptrVM.ptr(); }
  iScriptVM* __stdcall GetParentVM() const { return mptrParentVM; }
  tBool __stdcall InitializeMT() { return eFalse; }
  tBool __stdcall ShutdownMT() { return eFalse; }

  iScriptObject* __stdcall Compile(iFile* apFile, const achar* aaszName = NULL);
  iScriptObject* __stdcall CompileString(const achar* aaszCode, const achar* aaszName = NULL);
  iUnknown* __stdcall CompileGetIUnknownObject(iScriptObject* apThisTable, iFile* apFile, const achar* aaszObjectName, const tUUID& aIID);
  void __stdcall EnableDebugInfos(tBool abEnabled);
  tBool __stdcall AreDebugInfosEnabled() const;
  void __stdcall SetErrorLineOffset(tI32 anOffset);
  tI32 __stdcall GetErrorLineOffset() const;
  eScriptRaiseErrorMode __stdcall SetRaiseErrorMode(eScriptRaiseErrorMode aMode);
  eScriptRaiseErrorMode __stdcall GetRaiseErrorMode() const;
  void __stdcall SetLogRaiseError(tLogFlags aLog);
  tLogFlags __stdcall GetLogRaiseError() const;

  tBool __stdcall Run(iScriptObject* apThis, const achar* aaszCommand);
  tI32 __stdcall CollectGarbage();

  tBool __stdcall RegisterFunction(const sMethodDef* apFunction, const achar* aaszName);

  tBool __stdcall PushRootTable();
  tBool __stdcall PushString(const iHString* ahspValue);
  tBool __stdcall PushInteger(tI32 anValue);
  tBool __stdcall PushFloat(tF64 afValue);
  tBool __stdcall PushInterface(iUnknown* apInterface);
  tBool __stdcall PushVariant(const Var& aVar);
  tBool __stdcall PushObject(iScriptObject* apObject);
  tBool __stdcall Pop(tU32 anNumPop);
  tI32 __stdcall GetStackSize() const;
  tBool __stdcall SetStackSize(tU32 anSize);
  tBool __stdcall CreateSlot(tI32 idx);

  tInterfaceCVec<iFileSystem>* __stdcall GetImportFileSystems() const;
  iFile* __stdcall ImportFileOpen(const achar* aaszFile);
  tBool __stdcall _DoImport(tBool abNew, iUnknown* apPathOrFile, iScriptObject* apDestTable);
  tBool __stdcall Import(iUnknown* apPathOrFile, iScriptObject* apDestTable);
  tBool __stdcall NewImport(iUnknown* apPathOrFile, iScriptObject* apDestTable);

  iScriptObject* __stdcall CreateTable(iScriptObject* apDelegate, tI32 anNumPop);
  iScriptObject* __stdcall CreateObject(tI32 anIndex, tI32 anNumPop);
  iScriptObject* __stdcall CreateObjectGet(const achar* aaszKey, eScriptObjectType aRequiredType = eScriptObjectType_Last, tI32 anTablePos = -2);

  tBool __stdcall Call(tU32 anNumParams, tBool abPushRet);

  tBool __stdcall WriteClosure(iFile* apFile, iScriptObject* apObject);
  iScriptObject* __stdcall ReadClosure(iFile* apFile);

  tBool __stdcall ScriptCall(const achar* aaszModule, const achar* aaszFunc, const Var* apParams, tU32 anNumParams, Var* apRet);
  iScriptObject* __stdcall ScriptVar(const achar* aaszModule, const achar* aaszVar);
  //// iScriptVM ////////////////////////////////

  //// iScriptingHost ////////////////////////////////
  tBool __stdcall EvalString(iHString* ahspContext, const ni::achar* aaszCode);
  tBool __stdcall CanEvalImpl(iHString* ahspContext, iHString* ahspCodeResource);
  iUnknown* __stdcall EvalImpl(iHString* ahspContext, iHString* ahspCodeResource, const tUUID& aIID);
  void __stdcall Service(tBool abForceGC);
  //// iScriptingHost ////////////////////////////////

  inline operator HSQUIRRELVM () { return (HSQUIRRELVM)mptrVM.ptr(); }
  inline cScriptAutomation* GetAutomation() const { return mptrScriptAutomation; }

#ifdef SCRIPTOBJECT_INVALIDATEPOLICY
  void RegisterScriptObject(cScriptObject* pObj);
  void UnregisterScriptObject(cScriptObject* pObj);
#endif

 public:
  Ptr<cScriptVM> mptrParentVM;
  Ptr<cScriptAutomation> mptrScriptAutomation;
  Ptr<SQVM> mptrVM;
  tI32 mnLineOffset;
  tBool mbDebug;

#ifdef SCRIPTOBJECT_INVALIDATEPOLICY
  typedef astl::list<cScriptObject*, ASTL_ALLOCATOR(cScriptObject*,scriptobjectlist)> tScriptObjectPLst;
  typedef tScriptObjectPLst::iterator tScriptObjectPLstIt;
  typedef tScriptObjectPLst::const_iterator tScriptObjectPLstCIt;
  tScriptObjectPLst mlstScriptObjects;
#endif

  niEndClass(cScriptVM);
};

cString& sqGetStackDump(cString& strOut, HSQUIRRELVM v);
cString& sqGetCallstack(cString& astrOut, HSQUIRRELVM v, int aLevel);

#ifdef _DEBUG
#define niTraceSqCallStack(VM)  { cString str; sqGetStackDump(str,VM); niPrintln(str.Chars()); }
#else
#define niTraceSqCallStack(VM)
#endif

void sqFormatError(HSQUIRRELVM v, const achar* aszErr, cString& strErr, int level);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __SCRIPTVM_2181373_H__
