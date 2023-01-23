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

///////////////////////////////////////////////
// Squirrel api :
//
//    - CreateInstance(objecttype[,vara][,varb])
//    - Import(ModuleName)  - if module name refers to a AOM module
//    - NewImport
//    - GetIUnknown(object)
//    - QueryInterface(object,IID)
//
///////////////////////////////////////////////

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

///////////////////////////////////////////////
static int std_createinstance(HSQUIRRELVM v)
{
  int top = sq_gettop(v);
  cScriptVM* pVM = niUnsafeCast(cScriptVM*,sq_getforeignptr(v));
  const achar* aszObjectType = NULL;
  Var varA, varB;

  top -= 1; // remove this out of the equation

  if (top <= 0 || top > 3)
    return sq_throwerror(v, _A("std_createinstance, Invalid number of parameters."));

  if (!SQ_SUCCEEDED(sq_getstring(v,2,&aszObjectType)))
    return sq_throwerror(v, _A("std_createinstance, First parameter (object type) must be string containing the object type."));

  if (top > 1)
  {
    if (!SQ_SUCCEEDED(sqa_getvar(v,3,&varA)))
      return sq_throwerror(v, _A("std_createinstance, Second parameter (vara) must be a valid variant."));
  }

  if (top > 2)
  {
    if (!SQ_SUCCEEDED(sqa_getvar(v,4,&varB)))
      return sq_throwerror(v, _A("std_createinstance, Third parameter (varb) must be a valid variant."));
  }

  iUnknown* pI = GetLang()->CreateInstance(aszObjectType, varA, varB);
  if (!niIsOK(pI)) {
    niSafeRelease(pI);
    return sq_throwerror(v, niFmt(_A("std_createinstance, Can't create an instance of '%s'."), aszObjectType));
  }

  cString strModule = aszObjectType;
  strModule = strModule.substr(0,strModule.find(_A(".")));
  if (!pVM->GetAutomation()->Import(v,strModule.Chars())) {
    return sq_throwerror(v, niFmt(_A("std_createinstance, Can't import the module '%s'."),strModule.Chars()));
  }

  if (!SQ_SUCCEEDED(sqa_pushIUnknown(v,pI))) {
    niSafeRelease(pI);
    return sq_throwerror(v, niFmt(_A("std_createinstance, Can't push the new instance of '%s'."), aszObjectType));
  }

  return 1;
}

///////////////////////////////////////////////
// The import table is local to each table when it's imported, because a same source file
// can be imported in different table.
static int std_pushimports(HSQUIRRELVM v)
{
  sq_pushstring(v,_HC(__imports));
  if (SQ_FAILED(sq_get(v,-2)))
  {
    sq_pushstring(v,_HC(__imports));
    sq_newtable(v);
    sq_createslot(v,-3);

    sq_pushstring(v,_HC(__imports));
    sq_get(v,-2);

    sqa_setdebugname(v,-1,_A("__imports"));
  }

  return SQ_OK;
}

//static int __call_count = 0;

///////////////////////////////////////////////
static int std_import_ex(HSQUIRRELVM v, tBool abNew)
{
  int top = sq_gettop(v);
  cScriptVM* pVM = reinterpret_cast<cScriptVM*>(sq_getforeignptr(v));
  const achar* aszModule = NULL;

  niSqGuard(v);

  top -= 1; // remove this out of the equation

  if (top <= 0 || top > 2)
    return sq_throwerror(v, _A("std_import, Invalid number of parameters."));

  eScriptType type = sqa_getscripttype(v,2);
  if (type != eScriptType_String && type != eScriptType_IUnknown)
    return sq_throwerror(v, _A("std_import, the first parameter should be a string or a iFile instance."));

  tBool isScriptFile = eFalse;
  Ptr<iFile> ptrFile;
  if (type == eScriptType_IUnknown) {
    iUnknown* pFile;
    if (!SQ_SUCCEEDED(sqa_getIUnknown(v,2,&pFile,niGetInterfaceUUID(iFile))))
      return sq_throwerror(v,_A("std_import, the given instance doesn't implement iFile."));
    ptrFile = (iFile*)pFile;
  }
  else {
    if (!SQ_SUCCEEDED(sq_getstring(v,2,&aszModule)))
      return sq_throwerror(v, _A("std_import, can't get first parameter's string."));
    isScriptFile =
        StrEndsWithI(aszModule,".ni") ||
        StrEndsWithI(aszModule,".nim") ||
        StrEndsWithI(aszModule,".nio") ||
        StrEndsWithI(aszModule,".nip") ||
        StrEndsWithI(aszModule,".niw");
  }

  if (ptrFile.IsOK() || isScriptFile)
  {
    if (!niIsStringOK(aszModule))
      aszModule = ptrFile->GetSourcePath();

    if (!niIsStringOK(aszModule))
    {
      niSqUnGuard(v);
      return sq_throwerror(v, niFmt(_A("std_import, unnamed module cant be imported."),aszModule));
    }

    Ptr<iScriptObject> ptrTable = NULL;
    if (top > 1)
    {
      ptrTable = pVM->CreateObject(3,0);
      if (!niIsOK(ptrTable) || ptrTable->GetType() != eScriptObjectType_Table)
      {
        niSqUnGuard(v);
        return sq_throwerror(v, niFmt(_A("std_import, Can't open script module '%s', second parameter not a table."),aszModule));
      }
    }

    if (!abNew)
    {
      if (ptrTable.IsOK()) pVM->PushObject(ptrTable);
      else          pVM->PushRootTable();
      std_pushimports(v);
      sq_pushstring(v,_H(aszModule));
      tBool bAlreadyImported = SQ_SUCCEEDED(sq_get(v,-2));
      sq_pop(v,2); // pop the imports and root tables
      if (bAlreadyImported)
      {
        sq_pop(v,1);  // pop an extra item added by the successful get
        TRACE_SCRIPT_IMPORT(("niScript-Import: ni script '%s' already imported.",aszModule));
        niSqUnGuard(v);
        return 0;
      }
    }

    if (!ptrFile.IsOK()) {
      ptrFile = pVM->ImportFileOpen(aszModule);
      if (!ptrFile.IsOK()) {
        return sq_throwerror(v, niFmt(_A("std_import, Can't open script module '%s', can't open source file."),aszModule));
      }
    }

    // register the module
    if (!abNew) {
      if (ptrTable.IsOK()) pVM->PushObject(ptrTable);
      else          pVM->PushRootTable();
      std_pushimports(v);
      sq_pushstring(v,_H(aszModule));
      sq_pushint(v,1);
      sq_createslot(v,-3);
      sq_pop(v,2);  // pop the imports and root tables
      if (ptrTable.IsOK()) {
        TRACE_SCRIPT_IMPORT(("niScript-Import: ni script '%s' (%s) imported in a custom table.",
                             aszModule, ptrFile.IsOK() ? ptrFile->GetSourcePath() : aszModule));
      }
      else {
        TRACE_SCRIPT_IMPORT(("niScript-Import: ni script '%s' (%s) imported in the root table.",
                            aszModule, aszModule, ptrFile.IsOK() ? ptrFile->GetSourcePath() : aszModule));
      }
    }

    Ptr<iScriptObject> ptrScript = pVM->Compile(
        ptrFile,ni::GetRootFS()->GetAbsolutePath(ptrFile->GetSourcePath()).Chars());
    if (!niIsOK(ptrScript))
    {
      niSqUnGuard(v);
      return sq_throwerror(v, niFmt(_A("Can't compile script module '%s'."),aszModule));
    }
    //niTraceSqCallStack(v);
    //++__call_count;
    pVM->PushObject(ptrScript);
    if (ptrTable.IsOK()) pVM->PushObject(ptrTable);
    else          pVM->PushRootTable();
    //niTraceSqCallStack(v);
    if (!pVM->Call(1,eFalse))
    {
      pVM->Pop(1);
      //niTraceSqCallStack(v);
      niSqUnGuard(v);
      return sq_throwerror(v, niFmt(_A("Can't call script module '%s' closure."),aszModule));
    }
    pVM->Pop(1);

    niSqUnGuard(v);
    return 0;
  }
  else
  {
    if (!pVM->GetAutomation()->Import(v,aszModule)) {
      niSqUnGuard(v);
      return sq_throwerror(v, niFmt(_A("std_import, Can't import the module '%s'."),aszModule));
    }

    TRACE_SCRIPT_IMPORT(("niScript-Import: ni module '%s' imported.",aszModule));
    niSqUnGuard(v);
    return 0;
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
static int std_getiunknown(HSQUIRRELVM v)
{
  iUnknown* pIUnk = NULL;
  if (!SQ_SUCCEEDED(sqa_getIUnknown(v,2,&pIUnk,kuuidZero)))
    return sq_throwerror(v,_A("std_getiunknown, the parameter is not a valid iunknown."));
  sqa_pushIUnknown(v,pIUnk);
  return 1;
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

    eScriptType type = sqa_getscripttype(refpos);
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

  // TODO: Task test
#pragma niNote("Use SafeInvalidate here instead ?")
  pV->pObject->Invalidate();
  return SQ_OK;
}

///////////////////////////////////////////////
const SQObjectPtr& SQSharedState::GetInterfaceDelegate(HSQUIRRELVM v, const tUUID& aIID)
{
  niSqGuard(v);

  tDelegateMap::iterator it = mmapDelegates.find(aIID);
  if (it == mmapDelegates.end()) {
    SQObjectPtr objTable = _null_;
    if (aIID == niGetInterfaceUUID(iUnknown)) {
      objTable = SQTable::Create();
      _table(objTable)->SetDebugName(ni::GetLang()->GetInterfaceName(niGetInterfaceUUID(iUnknown)));
      v->Push(objTable);
      sqa_registerfunction(v,_A("_tostring"),iunknown_tostring);
      sqa_registerfunction(v,_A("GetImplementedInterfaces"),iunknown_getimplementedinterfaces);
      sqa_registerfunction(v,_A("isvalid"),iunknown_isvalid);
      sqa_registerfunction(v,_A("IsValid"),iunknown_isvalid);
      sqa_registerfunction(v,_A("tointptr"),iunknown_tointptr);
      sqa_registerfunction(v,_A("ToIntPtr"),iunknown_tointptr);
      sqa_registerfunction(v,_A("GetNumRefs"),iunknown_GetNumRefs);
      sqa_registerfunction(v,_A("QueryInterface"),iunknown_queryinterface);
      sqa_registerfunction(v,_A("Invalidate"),iunknown_invalidate);
      v->Pop(1);
    }
    else {
      const sInterfaceDef* pInterfaceDef = ni::GetLang()->GetInterfaceDefFromUUID(aIID);
      objTable = SQTable::Create();
      _table(objTable)->SetDebugName(ni::GetLang()->GetInterfaceName(aIID));
      v->Push(objTable);
      if (!pInterfaceDef)  {
        niWarning(niFmt(_A("Can't get definition of interface '%s' {%s}."),
                        niHStr(ni::GetLang()->GetInterfaceName(aIID)),
                        UUID_ASZ(aIID)));
      }
      else {
        sqa_setdebugname(v, -1, niFmt(_A("delegate:%s"),pInterfaceDef->maszName));

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
            sq_pushstring(v, _H(pMethodDef->maszName));
            sqa_pushMethodDef(v, pInterfaceDef, pMethodDef);
            sq_createslot(v,-3);
            if (SHOULD_TRACE_REGISTER_INTERFACE()) {
              cString str;
              str << pInterfaceDef->maszName << _A("::") << pMethodDef->maszName
                  << _A("/") << pMethodDef->mnNumParameters;
              niDebugFmt((str.Chars()));
            }
          }
        }

        if (!mapProps.empty())
        {
          // sq_pushstring(v, _A("__properties"), -1);
          // tScriptObjectPtr ptrPropsTable = mpVM->CreateTable(NULL,0);
          // sqa_setdebugname(v, -1, niFmt(_A("__properties:%s"),pInterfaceDef->maszName));
          for (itProp = mapProps.begin(); itProp != mapProps.end(); ++itProp)
          {
            sq_pushstring(v, _H(itProp->first));
            sqa_pushPropertyDef(v, pInterfaceDef, itProp->second.pSet, itProp->second.pGet);
            sq_createslot(v,-3);
          }
          // sq_createslot(v,-3);
        }
      }
      v->Pop(1);
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
void cScriptAutomation::RegisterAPI(HSQUIRRELVM vm) {
  sqa_registerglobalfunction(vm,_A("CreateInstance"),std_createinstance);
  sqa_registerglobalfunction(vm,_A("Import"),std_import);
  sqa_registerglobalfunction(vm,_A("NewImport"),std_newimport);
  sqa_registerglobalfunction(vm,_A("GetIUnknown"),std_getiunknown);
  sqa_registerglobalfunction(vm,_A("QueryInterface"),std_queryinterface);
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
tBool cScriptAutomation::Import(HSQUIRRELVM vm, const achar* aszMod)
{
  if (!mbIsValid) {
    niWarning(_A("Script automation invalidated."));
    return eFalse;
  }
  niAssert(vm != NULL);

  //tHStringPtr hspModule = _H(aszMod);
  tModuleMapIt it = mmapModules.find(aszMod);
  if (it != mmapModules.end())  return eTrue;

  Ptr<iModuleDef> ptrModuleDef = ni::GetLang()->LoadModuleDef(aszMod);
  if (!ptrModuleDef.IsOK()) {
    niError(niFmt(_A("Can't load module '%s'."),aszMod));
    return eFalse;
  }

  tU32 i;

  sq_pushroottable(vm);
  for (i = 0; i < ptrModuleDef->GetNumEnums(); ++i)
  {
    const sEnumDef* pEnumDef = ptrModuleDef->GetEnum(i);
    if (ni::StrCmp(pEnumDef->maszName,_A("Unnamed")) == 0) {
      sq_pushstring(vm, _HC(e));
    }
    else {
      sq_pushstring(vm, _H(pEnumDef->maszName));
    }
    sqa_pushEnumDef(vm,ptrModuleDef->GetEnum(i));
    sq_createslot(vm,-3);
  }
  sq_pop(vm,1);

  astl::upsert(mmapModules, aszMod, ptrModuleDef);

  for (i = 0; i < ptrModuleDef->GetNumDependencies(); ++i)
  {
    // check for invalid dependency
    if (ni::StrCmp(ptrModuleDef->GetDependency(i),ptrModuleDef->GetName()) == 0)
    {
      niWarning(niFmt(_A("Module '%s' loading, self-dependency."),ptrModuleDef->GetName()));
      continue;
    }

    // import the dependency
    if (!Import(vm,ptrModuleDef->GetDependency(i))) {
      niWarning(niFmt(_A("Module '%s' loading, can't import the dependency '%s'."),ptrModuleDef->GetName(),ptrModuleDef->GetDependency(i)));
    }
  }

  sq_pushroottable(vm);
  for (i = 0; i < ptrModuleDef->GetNumConstants(); ++i)
  {
    const sConstantDef* pConstDef = ptrModuleDef->GetConstant(i);
    //    niLog(Info,niFmt(_A("Const reg: %s\n"),pConstDef->GetName()));
    //    niPrintln(niFmt(_A("Const reg: %s\n"),pConstDef->GetName()));
    sq_pushstring(vm,_H(pConstDef->maszName));
    if (SQ_FAILED(sqa_pushvar(vm,pConstDef->mvarValue))) {
      sq_pop(vm,1); // pop the key
      niWarning(niFmt(_A("Can't push the value of constant '%s'."),pConstDef->maszName));
      continue;
    }
    sq_createslot(vm,-3);
  }
  sq_pop(vm,1);

  return eTrue;
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
    SQObjectPtr objTable = vm->_ss->GetInterfaceDelegate(vm,iid);
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

  SQObjectPtr objBase = vm->_ss->GetInterfaceDelegate(vm,iidBaseInterface);
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

#endif // niMinFeatures
