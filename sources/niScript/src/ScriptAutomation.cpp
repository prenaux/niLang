// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"

#if niMinFeatures(15)

#include <niLang/Utils/ObjectInterfaceCast.h>
#include <niLang/ILang.h>
#include "ScriptVM.h"
#include "ScriptObject.h"
#include "ScriptDispatch.h"
#include "ScriptAutomation.h"
#include "ScriptTypes.h"
#include "sqvm.h"
#include "sqtable.h"
#include <niLang/Utils/Trace.h>
#include "sq_hstring.h"

#define UUID_ASZ(UUID) ni::sUUID(UUID).ToString().Chars()

niDeclareModuleTrace_(niScript,TraceRegisterInterface);
#define TRACE_REGISTER_INTERFACE(FMT) niModuleTrace_(niScript,TraceRegisterInterface,FMT)
#define SHOULD_TRACE_REGISTER_INTERFACE() niModuleShouldTrace_(niScript,TraceRegisterInterface)

niDeclareModuleTrace_(niScript,TraceScriptImport);
#define TRACE_SCRIPT_IMPORT(FMT) niModuleTrace_(niScript,TraceScriptImport,FMT)

niDeclareModuleProperty(niScript,RegisterProperties,ni::eTrue);
#define SHOULD_REGISTER_PROPERTIES() niModuleShouldTrace_(niScript,RegisterProperties)

#pragma niTodo("Seems that when an exception is throw the stack is not properly popped, might be because of the active {} statement, check it")
#pragma niTodo("Test when properties are in several interfaces that one object implements --- It works the first interface is used --- BUT WE SHOULD ISSUE A WARNING, or throw an exceptions so that ambiguities are resolvable")
#pragma niTodo("Add op_Add, op_Sub, etc... automatic binding to metha-methods")
#pragma niTodo("Cleanup the code")
#pragma niTodo("Remove all code for Weak references")
#pragma niTodo("Try to lighten sIUnknown, it's a bit heavy now, and used quite a lot...")
#pragma niTodo("abHoldRef is forced to true all the time now, hopefully it wont cause any problem - it shouldnt...")

//////////////////////////////////////////////////////////////////////////////////////////////
#if 0
sScriptTypeIUnknown::sScriptTypeIUnknown(cScriptAutomation* apAutomation, iUnknown* apObj)
{
  pAutomation = apAutomation;
  pObject = apObj;
}
sScriptTypeIUnknown::~sScriptTypeIUnknown() {
  if (pAutomation)
    pAutomation->ReleaseIUnknownRef(this);
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////

// in sqlinter.cpp
SQObjectPtr DoImportNative(aout<SQSharedState> aSS, ain<SQObjectPtr> aDestTable, ain<SQObjectPtr> aImports, ain<SQObjectPtr> aModuleName);

///////////////////////////////////////////////
static int std_createinstance(HSQUIRRELVM v)
{
  int top = sq_gettop(v);
  cScriptVM* pVM = niUnsafeCast(cScriptVM*,sq_getforeignptr(v));
  const achar* aszObjectType = NULL;
  Var varA, varB;

  top -= 1; // remove this out of the equation

  if (top <= 0 || top > 3)
    return sq_throwerror(v, _A("std_createinstance: Invalid number of parameters."));

  if (!SQ_SUCCEEDED(sq_getstring(v,2,&aszObjectType)))
    return sq_throwerror(v, _A("std_createinstance: First parameter (object type) must be string containing the object type."));

  if (top > 1)
  {
    if (!SQ_SUCCEEDED(sqa_getvar(v,3,&varA)))
      return sq_throwerror(v, _A("std_createinstance: Second parameter (vara) must be a valid variant."));
  }

  if (top > 2)
  {
    if (!SQ_SUCCEEDED(sqa_getvar(v,4,&varB)))
      return sq_throwerror(v, _A("std_createinstance: Third parameter (varb) must be a valid variant."));
  }

  iUnknown* pI = GetLang()->CreateInstance(aszObjectType, varA, varB);
  if (!niIsOK(pI)) {
    niSafeRelease(pI);
    return sq_throwerror(v, niFmt(_A("std_createinstance: Can't create an instance of '%s'."), aszObjectType));
  }

  cString strModule = aszObjectType;
  strModule = strModule.substr(0,strModule.find(_A(".")));
  SQObjectPtr objModuleDef = DoImportNative(*v->_ss, v->_roottable, v->_ss->_nativeimports_table, _H(strModule));
  if (sqa_getscriptobjtype(objModuleDef) == eScriptType_ErrorCode) {
    return sq_throwerror(v, niFmt(
      _A("std_createinstance: Can't import the module '%s': %s."),
      strModule.Chars(),
      ((sScriptTypeErrorCode*)_userdata(objModuleDef))->_strErrorDesc));
  }

  if (!SQ_SUCCEEDED(sqa_pushIUnknown(v,pI))) {
    niSafeRelease(pI);
    return sq_throwerror(v, niFmt(_A("std_createinstance: Can't push the new instance of '%s'."), aszObjectType));
  }

  return 1;
}

///////////////////////////////////////////////
static int std_import_ex(HSQUIRRELVM v, tBool abNew)
{
  niLet numParams = sq_gettop(v)-1;
  cScriptVM* pVM = reinterpret_cast<cScriptVM*>(sq_getforeignptr(v));
  niSqGuard(v);

  if (numParams != 1) {
    niSqUnGuard(v);
    return sq_throwerror(v, niFmt("std_import: one parameter expected, %d provided.", numParams));
  }

  eScriptType type = sqa_getscripttype(v,2);
  if (type != eScriptType_String && type != eScriptType_IUnknown) {
    niSqUnGuard(v);
    return sq_throwerror(v, _A("std_import: the first parameter should be a string or a iFile instance."));
  }

  const achar* aszModule = NULL;
  tBool isScriptFile = eFalse;
  Ptr<iFile> ptrFile;
  if (type == eScriptType_IUnknown) {
    iUnknown* pFile;
    if (!SQ_SUCCEEDED(sqa_getIUnknown(v,2,&pFile,niGetInterfaceUUID(iFile)))) {
      niSqUnGuard(v);
      return sq_throwerror(v,_A("std_import: the given instance doesn't implement iFile."));
    }
    ptrFile = (iFile*)pFile;
  }
  else {
    if (!SQ_SUCCEEDED(sq_getstring(v,2,&aszModule))) {
      niSqUnGuard(v);
      return sq_throwerror(v, _A("std_import: can't get first parameter's string."));
    }
    isScriptFile =
        StrEndsWithI(aszModule,".ni") ||
        StrEndsWithI(aszModule,".nim") ||
        StrEndsWithI(aszModule,".nio") ||
        StrEndsWithI(aszModule,".nip") ||
        StrEndsWithI(aszModule,".niw");
  }

  // Import a script file
  if (ptrFile.IsOK() || isScriptFile) {
    if (!niIsStringOK(aszModule))
      aszModule = ptrFile->GetSourcePath();

    if (!niIsStringOK(aszModule)) {
      niSqUnGuard(v);
      return sq_throwerror(v, niFmt(_A("std_import: unnamed module cant be imported."),aszModule));
    }

    SQObjectPtr importKey = _H(aszModule);
    if (!abNew) {
      SQObjectPtr foundImported;
      if (_table(v->_ss->_scriptimports_table)->Get(importKey,foundImported)) {
        if (!sq_istable(foundImported)) {
          niSqUnGuard(v);
          return sq_throwerror(v, niFmt("std_import: %s: got an invalid object from the imports table.",aszModule));
        }
        TRACE_SCRIPT_IMPORT(("niScript-Import: ni script '%s' already imported.",aszModule));
        niSqUnGuard(v);
        v->Push(foundImported);
        return 1;
      }
      else {
        TRACE_SCRIPT_IMPORT(("niScript-Import: ni script '%s' not found, importing.",aszModule));
      }
    }
    else {
      TRACE_SCRIPT_IMPORT(("niScript-Import: ni script '%s' NewImport, importing.",aszModule));
    }

    if (!ptrFile.IsOK()) {
      ptrFile = pVM->ImportFileOpen(aszModule);
      if (!ptrFile.IsOK()) {
        niSqUnGuard(v);
        return sq_throwerror(v, niFmt(_A("std_import: %s: cant open script module as a file."),aszModule));
      }
    }

    SQObjectPtr thisTable = SQTable::Create();
    Ptr<iScriptObject> ptrScript = pVM->Compile(
        ptrFile,ni::GetRootFS()->GetAbsolutePath(ptrFile->GetSourcePath()).Chars());
    if (!niIsOK(ptrScript)) {
      niSqUnGuard(v);
      return sq_throwerror(v, niFmt("std_import: %s: cant compile script.",aszModule));
    }

    // register thisTable now, so that we dont try to reimport if there's an import cycle
    if (!abNew) {
      SQObjectPtr importsTable;
      if (!_table(v->_ss->_scriptimports_table)->NewSlot(importKey,thisTable)) {
        niSqUnGuard(v);
        return sq_throwerror(v, niFmt("std_import: %s: cant add to imports table.",aszModule));
      }
    }

    pVM->PushObject(ptrScript);
    v->Push(thisTable);
    if (!pVM->Call(1,eFalse)) {
      pVM->Pop(1);
      niSqUnGuard(v);
      return sq_throwerror(v, niFmt(_A("Can't call script module '%s' closure."),aszModule));
    }
    pVM->Pop(1);

    niSqUnGuard(v);
    v->Push(thisTable);
    return 1;
  }
  // Import a native script module
  else {
    SQObjectPtr objModuleDef = DoImportNative(*v->_ss, v->_roottable, v->_ss->_nativeimports_table, _H(aszModule));
    if (sqa_getscriptobjtype(objModuleDef) == eScriptType_ErrorCode) {
      niSqUnGuard(v);
      return sq_throwerror(v, niFmt(
        _A("std_import: Can't import the module '%s': %s."),
        aszModule,
        ((sScriptTypeErrorCode*)_userdata(objModuleDef))->_strErrorDesc));
    }

    TRACE_SCRIPT_IMPORT(("niScript-Import: ni module '%s' imported.",aszModule));
    niSqUnGuard(v);
    v->Push(objModuleDef);
    return 1;
  }
}

///////////////////////////////////////////////
static int std_newimport(HSQUIRRELVM v)
{
  return std_import_ex(v,eTrue);
}

///////////////////////////////////////////////
static int std_import(HSQUIRRELVM v)
{
  return std_import_ex(v,eFalse);
}

///////////////////////////////////////////////
static int std_queryinterface(HSQUIRRELVM v)
{
  iUnknown* pIUnk = NULL;
  tUUID uuid;
  iHString* hspName = NULL;

  switch (sqa_getscripttype(v,3)) {
    case eScriptType_UUID:
      if (!SQ_SUCCEEDED(sqa_getUUID(v,3,&uuid))) {
        return sq_throwerror(v,"std_queryinterface, can't get interface uuid.");
      }
      break;
    case eScriptType_IUnknown:
    case eScriptType_String:
      if (!SQ_SUCCEEDED(sq_gethstring(v,3,&hspName))) {
        return sq_throwerror(v,"std_queryinterface, can't get interface string id.");
      }
      uuid = ni::GetLang()->GetInterfaceUUID(hspName);
      if (uuid == kuuidZero) {
        return sq_throwerror(v,niFmt("std_queryinterface, can't get uuid of interface '%s'.",hspName));
      }
      break;
    default:
      return sq_throwerror(v,"std_queryinterface, parameter 2 is not a valid interface id");
  }

  if (!SQ_SUCCEEDED(sqa_getIUnknown(v,2,&pIUnk,uuid)))
    return sq_throwerror(v,_A("std_queryinterface, the first parameter is not a valid iunknown."));
  if (!SQ_SUCCEEDED(sqa_pushIUnknown(v,pIUnk)))
    return sq_throwerror(v,niFmt("std_queryinterface, can't query interface '%s' (%s).",
                                 uuid, hspName));

  return 1;
}

///////////////////////////////////////////////
bool iunknown_nexti(HSQUIRRELVM v, iUnknown* apObj, const SQObjectPtr &refpos, SQObjectPtr &outkey, SQObjectPtr &outval, SQObjectPtr &outitr) {

  Ptr<iIterator> it = ni::QueryInterface<iIterator>(apObj);
  if (niIsNullPtr(it)) {
    Ptr<iCollection> pCollection = ni::QueryInterface<iCollection>(apObj);
    if (!pCollection.IsOK())
      return false;

    eScriptType type = sqa_getscriptobjtype(refpos);
    if (type == eScriptType_Null) {
      it = pCollection->Iterator();
    }
    else if (type == eScriptType_IUnknown) {
      it = (iIterator*)_iunknown(refpos);
    }
    else {
      return false;
    }
  }

  if (!it->HasNext())
    return false;

  sqa_pushvar(v,it->Key());
  outkey = stack_get(v,-1);
  sq_pop(v,1);

  sqa_pushvar(v,it->Value());
  outval = stack_get(v,-1);
  sq_pop(v,1);

  sqa_pushIUnknown(v,it.ptr());
  outitr = stack_get(v,-1);
  sq_pop(v,1);

  it->Next();
  return true;
}

///////////////////////////////////////////////
static int iunknown_tostring(HSQUIRRELVM v)
{
  GET_IUNKNOWNUD(V,-1);
  if (!pV->pObject)
    return sq_throwerror(v,_A("iunknown_tostring, [this] is an invalid iunknown."));

  iHString* hsp = niGetIUnknownHString(pV->pObject);
  if (hsp) {
    v->Push(SQObjectPtr(hsp));
  }
  else {
    Ptr<iToString> ptrToString = ni::QueryInterface<iToString>(pV->pObject);
    if (ptrToString.IsOK()) {
      cString str;
      str << ptrToString.ptr();
      sq_pushstring(v,_H(str));
    }
    else {
      sq_pushstring(v,_HC(NOT_ITOSTRING));
    }
  }

  return 1;
}

///////////////////////////////////////////////
void iunknown_gettype_concat(cString& astrOut, HSQUIRRELVM v, const iUnknown* apI)
{
  if (!niIsOK(apI)) {
    astrOut = _A("Invalid");
  }
  else {
    astl::set<cString> setIIDs;

    Nonnull<tUUIDCVec> uuidLst{tUUIDCVec::Create()};
    apI->ListInterfaces(uuidLst,0);
    niLoop(i,uuidLst->size()) {
      const tUUID& IID = uuidLst->at(i);
      if (IID != niGetInterfaceUUID(iUnknown)) {
        const sInterfaceDef* def = ni::GetLang()->GetInterfaceDefFromUUID(IID);
        if (def) {
          setIIDs.insert(def->maszName);
        }
        else {
          setIIDs.insert(cString(IID));
        }
      }
    }
    if (setIIDs.empty()) {
      astrOut << "?NONE?;";
    }
    else {
      for (astl::set<cString>::iterator it = setIIDs.begin(); it != setIIDs.end(); ++it) {
        astrOut << *it;
        if (++it == setIIDs.end())
          break;
        astrOut << _A(";");
      }
    }
  }
}

///////////////////////////////////////////////
static int iunknown_getimplementedinterfaces(HSQUIRRELVM v)
{
  GET_IUNKNOWNUD(V,-1);
  if (!pV->pObject)
    return sq_throwerror(v,_A("iunknown_getimplementedinterfaces, [this] is an invalid iunknown."));

  cString str;
  iunknown_gettype_concat(str,v,pV->pObject);
  sq_pushstring(v,_H(str));
  return 1;
}

///////////////////////////////////////////////
static int iunknown_isvalid(HSQUIRRELVM v)
{
  GET_IUNKNOWNUD(V,-1);
  if (!pV->pObject)
    return sq_throwerror(v,_A("iunknown_isvalid, [this] is an invalid iunknown."));
  sq_pushint(v,niIsOK(pV->pObject));
  return 1;
}

///////////////////////////////////////////////
static int iunknown_tointptr(HSQUIRRELVM v)
{
  GET_IUNKNOWNUD(V,-1);
  if (!pV->pObject)
    return sq_throwerror(v,_A("iunknown_tointptr, [this] is an invalid iunknown."));
  sq_pushint(v,(tIntPtr)pV->pObject);
  return 1;
}

///////////////////////////////////////////////
static int iunknown_GetNumRefs(HSQUIRRELVM v)
{
  GET_IUNKNOWNUD(V,-1);
  if (!pV->pObject)
    return sq_throwerror(v,_A("iunknown_GetNumRefs, [this] is an invalid iunknown."));
  sq_pushint(v,pV->pObject->GetNumRefs());
  return 1;
}

///////////////////////////////////////////////
static int iunknown_queryinterface(HSQUIRRELVM v)
{
  GET_IUNKNOWNUD(V,-2);
  if (!pV->pObject)
    return sq_throwerror(v,_A("iunknown_queryinterface, [this] is an invalid iunknown."));

  iHString* hspName;
  if (!SQ_SUCCEEDED(sq_gethstring(v,-1,&hspName)))
    return sq_throwerror(v,_A("iunknown_queryinterface, the parameter is not a string."));

  iUnknown* p = pV->pObject->QueryInterface(ni::GetLang()->GetInterfaceUUID(hspName));
  if (p == NULL) {
    sq_pushnull(v);
  }
  else {
    if (!SQ_SUCCEEDED(sqa_pushIUnknown(v,p)))
      return sq_throwerror(v,_A("iunknown_queryinterface, can't push the queried interface."));
  }

  return 1;
}

///////////////////////////////////////////////
static int iunknown_invalidate(HSQUIRRELVM v)
{
  GET_IUNKNOWNUD(V,-1);
  if (!pV->pObject)
    return sq_throwerror(v,_A("iunknown_invalidate, [this] is an invalid iunknown."));

  pV->pObject->Invalidate();
  return SQ_OK;
}

///////////////////////////////////////////////
const SQObjectPtr& SQSharedState::GetInterfaceDelegate(const tUUID& aIID)
{
  niSqGuard(v);

  tDelegateMap::iterator it = mmapDelegates.find(aIID);
  if (it == mmapDelegates.end()) {
    SQObjectPtr objTable = _null_;
    if (aIID == niGetInterfaceUUID(iUnknown)) {
      objTable = SQTable::Create();
      niLetMut table = as_nn(_table(objTable));
      table->SetDebugName(ni::GetLang()->GetInterfaceName(niGetInterfaceUUID(iUnknown)));
      niLet regFunc = [&](ain<tChars> aName, ain<SQFUNCTION> aFunc, ain_nn_mut<iHString> ahspRetType) {
        return RegisterSQRegFunction(table, SQRegFunction { .name = aName, .f = aFunc, .rettype = ahspRetType });
      };
      niPanicAssert(regFunc("_tostring", iunknown_tostring, _HC(typestr_string)));
      niPanicAssert(regFunc("GetImplementedInterfaces",iunknown_getimplementedinterfaces, _HC(typestr_string)));
      niPanicAssert(regFunc("isvalid",iunknown_isvalid,_HC(typestr_int)));
      niPanicAssert(regFunc("IsValid",iunknown_isvalid,_HC(typestr_int)));
      niPanicAssert(regFunc("tointptr",iunknown_tointptr,_HC(typestr_int)));
      niPanicAssert(regFunc("ToIntPtr",iunknown_tointptr,_HC(typestr_int)));
      niPanicAssert(regFunc("GetNumRefs",iunknown_GetNumRefs,_HC(typestr_int)));
      niPanicAssert(regFunc("QueryInterface",iunknown_queryinterface,_HC(typestr_iunknown)));
      niPanicAssert(regFunc("Invalidate",iunknown_invalidate,_HC(typestr_void)));
    }
    else {
      const sInterfaceDef* pInterfaceDef = ni::GetLang()->GetInterfaceDefFromUUID(aIID);
      objTable = SQTable::Create();
      niLetMut table = _table(objTable);
      table->SetDebugName(ni::GetLang()->GetInterfaceName(aIID));
      if (!pInterfaceDef)  {
        niWarning(niFmt(_A("Can't get definition of interface '%s' {%s}."),
                        niHStr(ni::GetLang()->GetInterfaceName(aIID)),
                        UUID_ASZ(aIID)));
      }
      else {
        astl::map<cString,sPropertyMethods> mapProps;
        astl::map<cString,sPropertyMethods>::iterator itProp;

        tU32 i;
        for (i = 0; i < pInterfaceDef->mnNumMethods; ++i)
        {
          const sMethodDef* pMethodDef = pInterfaceDef->mpMethods[i];

          if (SHOULD_REGISTER_PROPERTIES()) {
            if (niFlagIs(pMethodDef->mReturnType,eTypeFlags_MethodSetter)) {
              if (pMethodDef->mnNumParameters > 2) {
                niWarning(niFmt(
                    "Method '%s::%s' is a property setter but doesnt take more than two parameters, property setter wont be registered.",
                    pInterfaceDef->maszName,
                    pMethodDef->maszName));
              }
              else {
                const achar* p = pMethodDef->maszName;
                if (StrNCmp(p,_A("Set"),3) != 0) {
                  niWarning(niFmt(
                      "Method '%s::%s' is a property setter but it's name doesnt start with Set, property setter wont be registered.",
                      pInterfaceDef->maszName,
                      pMethodDef->maszName));
                }
                else {
                  cString strName; StringToPropertyName(strName,p);
                  if (SHOULD_TRACE_REGISTER_INTERFACE()) {
                    cString str;
                    str << pInterfaceDef->maszName << _A("::") << p
                        << _A("/") << pMethodDef->mnNumParameters
                        << _A(" -> ") << strName;
                    niDebugFmt((str.Chars()));
                  }
                  if (strName.IsEmpty()) {
                    niWarning(niFmt(
                        "Method '%s::%s' is a property setter but can't get a proper name, property setter wont be registered.",
                        pInterfaceDef->maszName,
                        pMethodDef->maszName));
                  }
                  else {
                    itProp = mapProps.find(strName);
                    if (itProp != mapProps.end()) {
                      itProp->second.pSet = pMethodDef;
                    }
                    else {
                      astl::upsert(mapProps,strName,sPropertyMethods(pMethodDef,NULL));
                    }
                  }
                }
              }
            }
            else if (niFlagIs(pMethodDef->mReturnType,eTypeFlags_MethodGetter)) {
              if (niType(pMethodDef->mReturnType) == eType_Null) {
                niWarning(niFmt(
                    "Method '%s::%s' is a property getter but doesnt return a value, property getter wont be registered.",
                    pInterfaceDef->maszName,
                    pMethodDef->maszName));
              }
              else {
                if (pMethodDef->mnNumParameters > 1) {
                  niWarning(niFmt(
                      "Method '%s::%s' is a property getter but takes more than one parameter, property getter wont be registered.",
                      pInterfaceDef->maszName,
                      pMethodDef->maszName));
                }
                else {
                  const achar* p = pMethodDef->maszName;
                  if (StrNCmp(p,_A("Get"),3) != 0) {
                    niWarning(niFmt(
                        "Method '%s::%s' is a property getter but it's name doesnt start with Get, property getter wont be registered.",
                        pInterfaceDef->maszName,
                        pMethodDef->maszName));
                  }
                  else {
                    cString strName; StringToPropertyName(strName,p);
                    if (SHOULD_TRACE_REGISTER_INTERFACE()) {
                      cString str;
                      str << pInterfaceDef->maszName << _A("::") << p
                          << _A("/") << pMethodDef->mnNumParameters
                          << _A(" -> ") << strName;
                      niDebugFmt((str.Chars()));
                    }
                    if (strName.IsEmpty()) {
                      niWarning(niFmt(
                          "Method '%s::%s' is a property getter but can't get a proper name, property getter wont be registered.",
                          pInterfaceDef->maszName,
                          pMethodDef->maszName));
                    }
                    else {
                      itProp = mapProps.find(strName);
                      if (itProp != mapProps.end()) {
                        itProp->second.pGet = pMethodDef;
                      }
                      else {
                        astl::upsert(mapProps,strName,sPropertyMethods(NULL,pMethodDef));
                      }
                    }
                  }
                }
              }
            }
          }

          {
            table->NewSlot(
              _H(pMethodDef->maszName),
              niNew sScriptTypeMethodDef(*this, pInterfaceDef, pMethodDef));
            if (SHOULD_TRACE_REGISTER_INTERFACE()) {
              cString str;
              str << pInterfaceDef->maszName << _A("::") << pMethodDef->maszName
                  << _A("/") << pMethodDef->mnNumParameters;
              niDebugFmt((str.Chars()));
            }
          }
        }

        if (!mapProps.empty()) {
          for (itProp = mapProps.begin(); itProp != mapProps.end(); ++itProp) {
            table->NewSlot(
              _H(itProp->first),
              niNew sScriptTypePropertyDef(
                pInterfaceDef,
                itProp->second.pSet,
                itProp->second.pGet));
          }
        }
      }
    }

    if (objTable == _null_) {
      niSqUnGuard(v);
      return _null_;
    }

    it = astl::upsert(mmapDelegates,aIID,objTable);
  }

  niSqUnGuard(v);
  return it->second;
}

//////////////////////////////////////////////////////////////////////////////////////////////
const SQObjectPtr& SQSharedState::GetEnumDefTable(const sEnumDef* apEnumDef) {
  // We use a IntPtr as key because each enum def pointer must be
  // unique whithin a process.  That is enforced by the idl
  // generator... the enum definitions are static in nature.
  tEnumDefMap::iterator it = mmapEnumDefs.find((tIntPtr)apEnumDef);
  if (it == mmapEnumDefs.end()) {
    SQObjectPtr objTable = SQTable::Create();
    SQTable* t = _table(objTable);
    for (tU32 j = 0; j < apEnumDef->mnNumValues; ++j) {
      SQObjectPtr key(_H(apEnumDef->mpValues[j].maszName));
      SQObjectPtr val((SQInt)apEnumDef->mpValues[j].mnValue);
      t->NewSlot(key,val);
    }
    it = astl::upsert(mmapEnumDefs,(tIntPtr)apEnumDef,objTable);
  }
  return it->second;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// cScriptAutomation implementation.

///////////////////////////////////////////////
cScriptAutomation::cScriptAutomation() {

  ZeroMembers();
  mbIsValid = eTrue;

#if defined GETIUNKNOWN_INDEX_VTABLE || defined GETIUNKNOWN_OBJECT
  // Setup the Unknown table and array
  niLoop(i,knUnkTableMaxSize) {
    mlstUnk.push_back(&mvUnk[i]);
  }
#endif

#ifdef niDebug
  niDebugFmt(("ScriptAutomation[%p] initialized.",(tIntPtr)this));
#endif
}

///////////////////////////////////////////////
cScriptAutomation::~cScriptAutomation()
{
  Invalidate();
}

///////////////////////////////////////////////
void cScriptAutomation::ZeroMembers()
{
  mbIsValid = eFalse;
}

///////////////////////////////////////////////
void cScriptAutomation::Invalidate()
{
  if (!IsValid())
    return;

#ifdef GETIUNKNOWN_INDEX_VTABLE
  mmapVTable.clear();
#endif

#if defined GETIUNKNOWN_INDEX_VTABLE || defined GETIUNKNOWN_OBJECT
  mlstUnk.clear();
  mmapUnk.clear();
  niLoop(i,knUnkTableMaxSize) {
    mvUnk[i].lstInterfaces.clear();
  }
#endif

  mmapModules.clear();
  mptrFS = NULL;
  mbIsValid = eFalse;

#ifdef niDebug
  niDebugFmt(("ScriptAutomation[%p] invalidated.",(tIntPtr)this));
#endif
}

///////////////////////////////////////////////
iFileSystem* cScriptAutomation::_GetFS()
{
  if (mptrFS.IsOK())
    return mptrFS.ptr();
  mptrFS = ni::GetRootFS();
  return mptrFS.ptr();
}

///////////////////////////////////////////////
tBool __stdcall cScriptAutomation::IsOK() const
{
  niClassIsOK(cScriptAutomation);
  return ni::eTrue;
}

///////////////////////////////////////////////
int cScriptAutomation::PushIUnknown(HSQUIRRELVM v, iUnknown* apClass)
{
  niAssert(v != NULL);

  niSqGuard_(v,+1);
  if (apClass == NULL) {
    sq_pushnull(v);
    niSqUnGuard(v);
    return SQ_OK;
  }

  iHString* hsp = niGetIUnknownHString(apClass);
  if (hsp) {
    v->Push(SQObjectPtr(hsp));
  }
  else {
    v->Push(SQObjectPtr(apClass));
  }
  niSqUnGuard(v);
  return SQ_OK;
}

///////////////////////////////////////////////
int cScriptAutomation::GetIUnknown(HSQUIRRELVM v, int idx, iUnknown** appIUnknown, const tUUID& aIID)
{
  niAssert(v != NULL);

  // cScriptVM* pVM = reinterpret_cast<cScriptVM*>(sq_getforeignptr(v));
  switch (sqa_getscripttype(v,idx)) {
    case eScriptType_Table:
      {
        SQObjectPtr& o = stack_get(v,idx);
        niAssert(_sqtype(o) == OT_TABLE);
        SQTable* table = _table(o);
        cScriptDispatch* pScriptDispatch = down_cast<cScriptDispatch*>(table->mpDispatch);
        if (!pScriptDispatch) {
          pScriptDispatch = niNew cScriptDispatch(table);
          niAssert(niIsOK(pScriptDispatch));
          niAssert(table->mpDispatch == pScriptDispatch);
        }
        if (aIID != kuuidZero) {
          *appIUnknown = pScriptDispatch->QueryInterface(aIID);
          if (!*appIUnknown) {
            niSafeRelease(pScriptDispatch);
            niWarning(
                niFmt(_A("IUnknown Table doesnt implement the required interface '%s' {%s}."),
                      niHStr(ni::GetLang()->GetInterfaceName(aIID)),
                      UUID_ASZ(aIID)));
            return SQ_ERROR;
          }
        }
        else {
          *appIUnknown = pScriptDispatch;
        }
        break;
      }
    case eScriptType_String:
      {
        if (aIID != kuuidZero &&
            aIID != niGetInterfaceUUID(iHString) &&
            aIID != niGetInterfaceUUID(iUnknown))
        {
          niWarning(niFmt(_A("String can't be converted to interface '%s' {%s}."),
                          niHStr(ni::GetLang()->GetInterfaceName(aIID)),
                          UUID_ASZ(aIID)));
          return SQ_ERROR;
        }

        SQObjectPtr& o = stack_get(v,idx);
        niAssert(_sqtype(o) == OT_STRING);
        *appIUnknown = _stringhval(o);
        break;
      }
    case eScriptType_Int:
      {
        SQObjectPtr& o = stack_get(v,idx);
        if (_int(o) == 0) {
          *appIUnknown = NULL;
        }
        else {
          niWarning(niFmt("Only 'NULL' (0) integer can be passed as IUnknown, got '%s'.",(Var&)o._var));
          return SQ_ERROR;
        }
        break;
      };
    case eScriptType_Null:
      {
        *appIUnknown = NULL;
        break;
      }
    case eScriptType_IUnknown: {
      SQObjectPtr& o = stack_get(v,idx);
      if (aIID != kuuidZero) {
        *appIUnknown = _iunknown(o)->QueryInterface(aIID);
        if (!*appIUnknown) {
          cString str;
          iunknown_gettype_concat(str,v,_iunknown(o));

          niWarning(niFmt(_A("IUnknown (%s) doesnt implement interface '%s' {%s}."),
                          str,
                          niHStr(ni::GetLang()->GetInterfaceName(aIID)),
                          UUID_ASZ(aIID)));
          return SQ_ERROR;
        }
      }
      else
      {
        *appIUnknown = _iunknown(o);
      }
      break;
    }
    default:
      {
        GET_IUNKNOWNUD(V,idx);
        if (aIID != kuuidZero) {
          *appIUnknown = pV->pObject->QueryInterface(aIID);
          if (!*appIUnknown) {
            niWarning(niFmt(_A("IUnknown doesnt implement interface '%s' {%s}."),
                            niHStr(ni::GetLang()->GetInterfaceName(aIID)),
                            UUID_ASZ(aIID)));
            return SQ_ERROR;
          }
        }
        else{
          *appIUnknown = pV->pObject;
        }
        break;
      }
  }

  return SQ_OK;
}

///////////////////////////////////////////////
void cScriptAutomation::InitIntfDelegateLst(
    HSQUIRRELVM vm,
    cScriptAutomation::tIntfDelegateLst& alstInterfaces,
    iUnknown* apObj
#ifdef GETIUNKNOWN_INDEX_VTABLE
    , tBool abDynamicOnly
#endif
                                            )
{
  niAssert(niIsOK(apObj));
  niAssert(vm != NULL);

  tUUID iidBaseInterface = niGetInterfaceUUID(iUnknown);
  Nonnull<tUUIDCVec> uuidLst{tUUIDCVec::Create()};
  apObj->ListInterfaces(
      uuidLst,
#ifdef GETIUNKNOWN_INDEX_VTABLE
      abDynamicOnly?eListInterfacesFlags_DynamicOnly:eListInterfacesFlags_NoDynamic
#else
      0
#endif
                        );
  if (uuidLst->empty())
    return;

  Ptr<iScriptObject> ptrDelegateTable = NULL;
  Ptr<iScriptObject> ptrRootDelegateTable = NULL;
  niLoop(i,uuidLst->size()) {
    const tUUID& iid = uuidLst->at(i);
    if (iid == niGetInterfaceUUID(iUnknown))  continue;
    if (iid == niGetInterfaceUUID(iDispatch)) continue;
    SQObjectPtr objTable = vm->_ss->GetInterfaceDelegate(iid);
    if (objTable == _null_) {
      niWarning(niFmt(_A("Can't get delegate of interface '%s' {%s}."),
                      niHStr(ni::GetLang()->GetInterfaceName(iid)),
                      UUID_ASZ(iid)));
    }
    else {
      alstInterfaces.push_back(_table(objTable));
    }
  }

#if 0
  if (!abBaseOnly) {
    // Reverse is used by the VTable indexing mode to reverse the
    // list of dispatch for specific instance. The idea is that
    // the VTable should match most of the time and even for
    // instance specific the vtable's interfaces would be first in
    // the list, so reversing the list should put the 'missed'
    // interfaces first and avoid lookingup the VTable interfaces
    // twice.
    // For example :
    //   iWidget vtable : iWidget;iUnknown
    //   Button interfaces : iWidget;iWidgetButton;iUnknown
    //   -> so for the button the reverse will give us iWidgetButton;iWidget;iUnknown
    //      which will get the lookup to look into iWidgetButton first, yes we do
    //      waste two pointers in that case, but the added complexity to not do so
    //      is not worth it atm.
    alstInterfaces.reverse();
  }
#endif

  SQObjectPtr objBase = vm->_ss->GetInterfaceDelegate(iidBaseInterface);
  if (objBase == _null_) {
    niWarning(niFmt(_A("sqa_pushIUnknown, Can't get delegate of base interface '%s' {%s}."),
                    niHStr(ni::GetLang()->GetInterfaceName(iidBaseInterface)),
                    UUID_ASZ(iidBaseInterface)));
  }
  else {
    alstInterfaces.push_back(_table(objBase));
  }
}

///////////////////////////////////////////////
#ifdef GETIUNKNOWN_INDEX_VTABLE
cScriptAutomation::tIntfDelegateLst* cScriptAutomation::GetIUnknownVTable(iUnknown* apIUnknown) {
  const tIntPtr vt = (tIntPtr)niGetVTable_Object(apIUnknown);
  tVTableMap::iterator it = mmapVTable.find(vt);
  if (it == mmapVTable.end()) {
    it = astl::upsert(mmapVTable,vt,tIntfDelegateLst());
  }
  return &it->second;
}
#endif

///////////////////////////////////////////////
#if defined GETIUNKNOWN_INDEX_VTABLE || defined GETIUNKNOWN_OBJECT
cScriptAutomation::sIUnknownEntry* cScriptAutomation::NewIUnknownEntry(iUnknown* apObj) {
  // get oldest node...
  tUEntryLst::iterator lit = mlstUnk.end();
  --lit;
  sIUnknownEntry* e = *lit;
  // remove the object from the table if necessary
  mlstUnk.erase(lit);
  astl::map_erase(mmapUnk,e->pUnknown);
  // initialize
  e->pUnknown = (tIntPtr)apObj;
  e->pVTable = (tIntPtr)niGetVTable_Object(apObj);
  e->lstInterfaces.clear();
  mlstUnk.push_front(e);
  // put back in the table
  astl::upsert(mmapUnk,(tIntPtr)apObj,mlstUnk.begin());
  return e;
}
cScriptAutomation::sIUnknownEntry* cScriptAutomation::GetIUnknownEntry(iUnknown* apIUnknown) {
  tUEntryMap::iterator it = mmapUnk.find((tIntPtr)apIUnknown);
  sIUnknownEntry* e;
  if (it == mmapUnk.end()) {
    return NewIUnknownEntry(apIUnknown);
  }
  else {
    tUEntryLst::iterator lit = it->second;
    e = *lit;
    if (e->pVTable != (tIntPtr)niGetVTable_Object(apIUnknown)) {
      return NewIUnknownEntry(apIUnknown);
    }
    else if (lit != mlstUnk.begin()) {
      // move the entry as 'most recent'
      mlstUnk.erase(lit);
      mlstUnk.push_front(e);
      it->second = mlstUnk.begin();
    }
  }
  return e;
}
#endif

// in ScriptTypes.cpp
int vec2f_constructor(HSQUIRRELVM v);
int vec3f_constructor(HSQUIRRELVM v);
int vec4f_constructor(HSQUIRRELVM v);
int vec4f_constructor(HSQUIRRELVM v);
int rect_constructor(HSQUIRRELVM v);
int matrixf_constructor(HSQUIRRELVM v);
int uuid_constructor(HSQUIRRELVM v);

SQRegFunction SQSharedState::_automation_funcs[] = {
  {"CreateInstance", std_createinstance, -2, "ts", _HC(typestr_iunknown)},
  {"Import", std_import, -2, "ts", nullptr},
  {"NewImport", std_newimport, -2, "ts", nullptr},
  {"QueryInterface", std_queryinterface, 3, "t..", _HC(typestr_iunknown)},
  {"Vec2", vec2f_constructor, -1, "t", _HC(Vec2)},
  {"Vec3", vec3f_constructor, -1, "t", _HC(Vec3)},
  {"RGB", vec3f_constructor, -1, "t", _HC(Vec3)},
  {"Vec4", vec4f_constructor, -1, "t", _HC(Vec4)},
  {"RGBA", vec4f_constructor, -1, "t", _HC(Vec4)},
  {"Quat", vec4f_constructor, -1, "t", _HC(Vec4)},
  {"Plane", vec4f_constructor, -1, "t", _HC(Vec4)},
  {"Rect", rect_constructor, -1, "t", _HC(Vec4)},
  {"Matrix", matrixf_constructor, -1, "t", _HC(Matrix)},
  {"UUID", uuid_constructor, -1, "t", _HC(UUID)},
  {0,0}
};

#endif // niMinFeatures
