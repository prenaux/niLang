#ifndef __ISCRIPTVM_826431_H__
#define __ISCRIPTVM_826431_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Types.h>

#if niMinFeatures(15)

#include <niLang/IFileSystem.h>
#include <niLang/IScriptingHost.h>
#include <niLang/ILang.h>
#include <niLang/Utils/QPtr.h>
#include <niLang/Utils/URLFileHandler.h>

#include "IScriptObject.h"

struct SQVM;
struct SQObject;

namespace ni {

struct iScriptVM;

/** \addtogroup niLang
 * @{
 */

//! Script object enumeration sink.
struct iScriptObjectEnumSink : public iUnknown
{
  niDeclareInterfaceUUID(iScriptObjectEnumSink,0xc8e07068,0xb84a,0x4adc,0xa2,0x98,0x2b,0x5d,0xa2,0x1c,0x69,0x00)

  virtual tBool __stdcall OnScriptObjectEnumSinkFound(iScriptVM* apVM, iScriptObject* apKey, iScriptObject* apValue) = 0;
};

//! Script object enumeration sink smart pointer.
typedef Ptr<iScriptObjectEnumSink>  tScriptObjectEnumSinkPtr;

//! Script raise error mode
enum eScriptRaiseErrorMode
{
  //! Never automatically generate an error message
  eScriptRaiseErrorMode_Silent = 0,
  //! Generates a simple error message
  eScriptRaiseErrorMode_Error = 1,
  //! Generates a short callstack (default)
  eScriptRaiseErrorMode_ShortCallstack = 2,
  //! Generates a verbose callstack
  eScriptRaiseErrorMode_Callstack = 3,
  //! Generates a short callstack with locals
  eScriptRaiseErrorMode_ShortLocals = 4,
  //! Generates a verbose callstack with locals
  eScriptRaiseErrorMode_Locals = 5,
  //! \internal
  eScriptRaiseErrorMode_ForceDWORD = 0xFFFFFFFF
};

//! Script Virtual Machine interface.
struct iScriptVM : public iScriptingHost
{
  niDeclareInterfaceUUID(iScriptVM,0x50bc7ff5,0x0e32,0x4390,0x8c,0x5e,0xb9,0x60,0xa2,0xbe,0xe9,0x02)

  //########################################################################################
  //! \name System
  //########################################################################################
  //! @{

  //! Get the VM's handle.
  //! {Property}
  virtual tIntPtr __stdcall GetHandle() const = 0;
  //! Get the parent VM.
  //! {Property}
  virtual iScriptVM* __stdcall GetParentVM() const = 0;

  //! @}

  //########################################################################################
  //! \name Compilation & Execution
  //########################################################################################
  //! @{

  //! Compile the given file.
  //! \param  apFile containes the code to compile.
  //! \param  aaszName is the name to give to the closure. If NULL the file source path is used as name.
  //! \return An object that contains the compiled code.
  //! \remark This does not run the compiled code.
  virtual iScriptObject* __stdcall Compile(iFile* apFile, const achar* aaszName = NULL) = 0;
  //! Compile the given string.
  //! \param  aaszCode containes the code to compile.
  //! \param  aaszName is the name to give to the closure. If NULL [VM::String] is used.
  //! \return A closure that contains the compiled code.
  //! \remark This does not run the compiled code.
  virtual iScriptObject* __stdcall CompileString(const achar* aaszCode, const achar* aaszName = NULL) = 0;
  //! Compile the given file and get the specified IUnknown object.
  virtual iUnknown* __stdcall CompileGetIUnknownObject(iScriptObject* apThisTable, iFile* apFile, const achar* aaszObjectName, const tUUID& aIID) = 0;
  //! Run the specified text.
  //! \param  apThis is the 'this' table to use for the call, if null use the root table.
  //! \param  aaszCommand is the text to run.
  //! \return eFalse on error, else eTrue.
  virtual tBool __stdcall Run(iScriptObject* apThis, const achar* aaszCommand) = 0;
  //! Collect garbage.
  virtual tI32 __stdcall CollectGarbage() = 0;
  //! @}

  //########################################################################################
  //! \name Debug
  //########################################################################################
  //! @{

  //! Set the line error reporting offset.
  //! {Property}
  virtual void __stdcall SetErrorLineOffset(tI32 anOffset) = 0;
  //! Get the line error reporting offset.
  //! {Property}
  virtual tI32 __stdcall GetErrorLineOffset() const = 0;
  //! Set the current raise error mode.
  //! {Property}
  //! \return The previous raise error mode.
  virtual eScriptRaiseErrorMode __stdcall SetRaiseErrorMode(eScriptRaiseErrorMode aMode) = 0;
  //! Get the current raise error mode.
  //! {Property}
  virtual eScriptRaiseErrorMode __stdcall GetRaiseErrorMode() const = 0;
  //! Set whether each error raised should be logged.
  //! {Property}
  //! \remark The default is eLogFlags_Debug in debug builds and 0 (disabled) in release builds.
  virtual void __stdcall SetLogRaiseError(tLogFlags aLog) = 0;
  //! Get whether each error raised should be logged.
  //! {Property}
  virtual tLogFlags __stdcall GetLogRaiseError() const = 0;
  //! @}

  //########################################################################################
  //! \name Registration
  //########################################################################################
  //! @{

  //! Register a function in the active table.
  virtual tBool __stdcall RegisterFunction(const sMethodDef* apFunction, const achar* aaszName = NULL) = 0;
  //! @}

  //########################################################################################
  //! \name Stack manipulation
  //########################################################################################
  //! @{

  //! Push the root table on the stack.
  virtual tBool __stdcall PushRootTable() = 0;
  //! Push a string on the stack.
  virtual tBool __stdcall PushString(const iHString* ahspValue) = 0;
  //! Push an integer on the stack.
  virtual tBool __stdcall PushInteger(tI32 anValue) = 0;
  //! Push a float on the stack.
  virtual tBool __stdcall PushFloat(tF64 afValue) = 0;
  //! Push an interface on the stack.
  virtual tBool __stdcall PushInterface(iUnknown* apInterface) = 0;
  //! Push a variant on the stack.
  virtual tBool __stdcall PushVariant(const Var& aVar) = 0;
  //! Push a script object on the stack.
  virtual tBool __stdcall PushObject(iScriptObject* apObject) = 0;
  //! Pop n object of the stack.
  virtual tBool __stdcall Pop(tU32 anNumPop) = 0;
  //! Get the stack size.
  virtual tI32 __stdcall GetStackSize() const = 0;
  //! Set the stack size.
  virtual tBool __stdcall SetStackSize(tU32 anSize) = 0;
  //! Create a new slot.
  virtual tBool __stdcall CreateSlot(tI32 idx) = 0;
  //! @}

  //########################################################################################
  //! \name Imports
  //########################################################################################
  //! @{

  //! Get the list of file systems used by the script protocol URLFile handler.
  //! {Property}
  //! \remark By default the root file system and ni.dir.scripts folder are added.
  virtual tInterfaceCVec<iFileSystem>* __stdcall GetImportFileSystems() const = 0;
  //! Open a file using the script protocol by default if no procotol is specified.
  virtual iFile* __stdcall ImportFileOpen(const achar* aaszFile) = 0;
  //! Call the system Import function.
  virtual Ptr<iScriptObject> __stdcall Import(iUnknown* apPathOrFile) = 0;
  //! Call the system NewImport function.
  virtual Ptr<iScriptObject> __stdcall NewImport(iUnknown* apPathOrFile) = 0;
  //! @ }

  //########################################################################################
  //! \name Script objects
  //########################################################################################
  //! @{

  //! Create a new table.
  //! \param  apDelegate is an optional delegate table.
  //! \param  anNumPop
  //! \return NULL if the table's creation failed.
  virtual iScriptObject* __stdcall CreateTable(iScriptObject* apDelegate = NULL, tI32 anNumPop = 1) = 0;
  //! Create a new script object.
  virtual iScriptObject* __stdcall CreateObject(tI32 anIndex = eInvalidHandle, tI32 anNumPop = 1) = 0;
  //! Create an object the pushed table.
  //! \param  aaszKey is the name of the object to get.
  //! \param  aRequiredType is the type that the object should have. If eScriptObjectType_Last this is ignored.
  //! \param  anNumPop is the number of pop, by defaul the object is pushed on the stack.
  //! \param  abTry When set to true, no error log will be output if the object can't be found.
  //! \return NULL if the object can't be found or that the type doesn't match aRequiredType.
  virtual iScriptObject* __stdcall CreateObjectGet(const achar* aaszKey, eScriptObjectType aRequiredType = eScriptObjectType_Last, tI32 anNumPop = 1, tBool abTry = eFalse) = 0;
  //! @}

  //########################################################################################
  //! \name Functions
  //########################################################################################
  //! @{

  //! Call a function.
  //! \param  anNumParams number of parameters to pop of the stack, should be at least one, aka the this table.
  //! \param  abPushRet if eTrue the return value will be pushed in the stack when the function returns.
  //! \return eFalse if the call fails else eTrue.
  //! \remark Pops anNumParameters number of parameters of the stack, and call the closure in the stack.
  virtual tBool __stdcall Call(tU32 anNumParams, tBool abPushRet) = 0;
  //! @}

  //########################################################################################
  //! \name Serialization
  //########################################################################################
  //! @{

  //! Write a closure.
  virtual tBool __stdcall WriteClosure(iFile* apFile, iScriptObject* apObject) = 0;
  //! Read a closure.
  virtual iScriptObject* __stdcall ReadClosure(iFile* apFile) = 0;
  //! @}

  //########################################################################################
  //! \name High level API
  //########################################################################################
  //! @{

  //! {NoAutomation}
  virtual tBool __stdcall ScriptCall(iScriptObject* apThis, const achar* aaszFunc, const Var* apParams, tU32 anNumParams, Var* apRet) = 0;

  //! Get a script object by path starting from the root table or the specified this object.
  //! \param apThis Pointer to the script object to start the search from. If nullptr, the root table is used.
  //! \param aaszVar A string representing the path of the variable to search for, for example 'foo.bar'.
  //! \param abTry When set to true, no error log will be output if the object can't be found.
  //! \return The iScriptObject found at the specified path, or nullptr if not found.
  virtual Ptr<iScriptObject> __stdcall ScriptVar(iScriptObject* apThis, const achar* aaszVar, tBool abTry) = 0;

  //! Evaluate the specified string.
  virtual tBool __stdcall EvalString(iHString* ahspContext, const ni::achar* aaszCode) = 0;
  //! Check whether the scripting host can evaluate the specified code resource.
  virtual tBool __stdcall CanEvalImpl(iHString* ahspContext, iHString* ahspCodeResource) = 0;
  //! Evaluate the specified implementation from the specified 'code resource'.
  //! \remark The code resource can be a file name, class name, or whathever is relevant as
  //!         an implementation identifier depending on the type of the scripting host.
  virtual iUnknown* __stdcall EvalImpl(iHString* ahspContext, iHString* ahspCodeResource, const tUUID& aIID) = 0;
  //! Service the scripting host.
  virtual void __stdcall Service(tBool abForceGC) = 0;
  //! @}
};

niExportFunc(ni::iUnknown*) New_niScript_ScriptVM(const ni::Var&,const ni::Var&);

///////////////////////////////////////////////
inline iScriptVM* CreateScriptVM() {
  return (iScriptVM*)niCreateInstance(niScript,ScriptVM,0,0);
}

///////////////////////////////////////////////
inline iScriptVM* CreateChildScriptVM(iScriptVM* apParentVM) {
  return (iScriptVM*)niCreateInstance(niScript,ScriptVM,apParentVM,0);
}

///////////////////////////////////////////////
inline iScriptVM* CreateConcurrentScriptVM(iConcurrent* apConcurrent) {
  return (iScriptVM*)niCreateInstance(niScript,ScriptVM,apConcurrent,0);
}

///////////////////////////////////////////////
inline tBool ScriptCall(iScriptVM* apVM, iScriptObject* apThis, const achar* aaszFunc, const Var* apParams, tU32 anNumParams, Var* apRet) {
  if (!apVM) {
    apVM = ni::QueryInterface<iScriptVM>(ni::GetLang()->GetGlobalInstance("niLang.ScriptVM"));
    if (!apVM) {
      niError(_A("No valid ScriptVM instance."));
      return eFalse;
    }
  }
  return apVM->ScriptCall(apThis,aaszFunc,apParams,anNumParams,apRet);
}

///////////////////////////////////////////////
inline Ptr<iScriptObject> ScriptVar(iScriptVM* apVM, iScriptObject* apThis, const achar* aaszVar, tBool abTry) {
  if (!apVM) {
    apVM = ni::QueryInterface<iScriptVM>(ni::GetLang()->GetGlobalInstance("niLang.ScriptVM"));
    if (!apVM) {
      niError(_A("No valid ScriptVM instance."));
      return NULL;
    }
  }
  return apVM->ScriptVar(apThis,aaszVar,abTry);
}

///////////////////////////////////////////////
inline ni::tBool ScriptAddImportDir(
  ni::iScriptVM* apVM,
  const achar* aDirectory,
  ni::iFileSystem* apFS = ni::GetRootFS())
{
  niCheckIsOK(apVM,eFalse);
  niCheckIsOK(apFS,eFalse);
  cString absPath = apFS->GetAbsolutePath(aDirectory);
  if (apFS->FileExists(absPath.Chars(),eFileAttrFlags_AllDirectories)) {
    tInterfaceCVec<iFileSystem>& importFileSystems = *apVM->GetImportFileSystems();
    Ptr<iFileSystem> importDirFS = ni::GetLang()->CreateFileSystemDir(
      absPath.Chars(),ni::eFileSystemRightsFlags_ReadOnly);
    niLoopr(ri,importFileSystems.size()) {
      Ptr<iFileSystem> fs = importFileSystems.at(ri);
      if (fs.IsOK()) {
        if (ni::StrIEq(importDirFS->GetBaseContainer(),fs->GetBaseContainer())) {
          niWarning(niFmt("Include directory '%s' already added.",absPath.Chars()));
          return ni::eTrue;
        }
      }
    }
    importFileSystems.Add(importDirFS.ptr());
    niLog(Info,niFmt("Added script import directory '%s'.",absPath.Chars()));
    return ni::eTrue;
  }
  else {
    niWarning(niFmt("Can't add script import directory '%s', it doesn't exist.",absPath.Chars()));
    return ni::eFalse;
  }
}

///////////////////////////////////////////////
inline ni::tBool ScriptAddToolkitImportDir(
  ni::iScriptVM* apVM,
  const char* aTKName,
  const char* aDirInTK,
  ni::iFileSystem* apFS = ni::GetRootFS())
{
  cString path = GetToolkitDir(aTKName,aDirInTK);
  return ScriptAddImportDir(apVM,path.Chars(),apFS);
}

///////////////////////////////////////////////
inline ni::QPtr<iScriptVM> ScriptGetDefaultVM() {
  return ni::GetLang()->GetGlobalInstance("niLang.ScriptVM");
}

///////////////////////////////////////////////
inline ni::Ptr<iScriptVM> ScriptCreateOrGetDefaultVM(ni::iFileSystem* apFS = ni::GetRootFS()) {
  ni::QPtr<ni::iScriptVM> vm = ScriptGetDefaultVM();
  if (!vm.IsOK()) {
    vm = ni::CreateConcurrentScriptVM(ni::GetConcurrent());
    niCheckIsOK(vm,NULL);
    ni::GetLang()->SetGlobalInstance("niLang.ScriptVM", vm);
  }
  if (vm.IsOK() && !ni::GetLang()->GetScriptingHostFromName(_H("ni"))) {
    ScriptAddToolkitImportDir(vm,"niLang","scripts",apFS);
    niCheck(vm->Import(_H("lang.ni")).IsOK(),nullptr);
    ni::GetLang()->AddScriptingHost(_H("ni"),vm);
  }
  return vm.ptr();
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif
#endif // __ISCRIPTVM_826431_H__
