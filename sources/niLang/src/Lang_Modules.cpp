// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "Lang.h"
#include "API/niLang/Utils/ModuleUtils.h"
#include "API/niLang/Utils/Path.h"
#include "API/niLang/Utils/ConcurrentImpl.h"
#include "API/niLang/ILang.h"
#include "API/niLang.h"

using namespace ni;

static sPropertyBool _traceLoadModule("niLang.TraceLoadModule", eFalse);
#define TRACE_LOAD_MODULE(FMT) if (_traceLoadModule.get()) { niDebugFmt(FMT); }

static const sInterfaceDef IDef_iUnknown = {
  niGetInterfaceID(ni::iUnknown),
  &niGetInterfaceUUID(ni::iUnknown),
  0,
  NULL,
  0,
  NULL,
  NULL
};
static const sInterfaceDef IDef_iDispatch = {
  niGetInterfaceID(ni::iDispatch),
  &niGetInterfaceUUID(ni::iDispatch),
  0,
  NULL,
  0,
  NULL,
  NULL
};

struct sObjectTypeDefCallback : public ni::cIUnknownImpl<iCallback,eIUnknownImplFlags_DontInherit1,iRunnable> {
  Ptr<iObjectTypeDef> _odef;
  sObjectTypeDefCallback(const iObjectTypeDef* aObjectTypeDef) : _odef(aObjectTypeDef) {}
  virtual Var __stdcall Run() { return RunCallback(niVarNull, niVarNull); }
  virtual Var __stdcall RunCallback(const Var& avarA, const Var& avarB) {
    return _odef->CreateInstance(avarA,avarB);
  }
};

///////////////////////////////////////////////
void cLang::_StartupModules() {
  mbmapUUIDNames.insert(niGetInterfaceUUID(ni::iUnknown), _H("iUnknown"));
  mbmapUUIDNames.insert(niGetInterfaceUUID(ni::iDispatch), _H("iDispatch"));
  mmapUUIDDef[niGetInterfaceUUID(ni::iUnknown)] = &IDef_iUnknown;
  mmapUUIDDef[niGetInterfaceUUID(ni::iDispatch)] = &IDef_iDispatch;
}

///////////////////////////////////////////////
tBool cLang::_FinalizeRegisterModuleDef(const cLang::sModuleDef& mod) {
  const iModuleDef* mdef = mod.mptrModuleDef;
  niLoop(i,mdef->GetNumEnums()) {
    const sEnumDef* edef = mdef->GetEnum(i);
    RegisterEnumDef(edef);
  }
  niLoop(i,mdef->GetNumObjectTypes()) {
    const iObjectTypeDef* odef = mdef->GetObjectType(i);
    astl::upsert(*mmapCreateInstance, odef->GetName(),niNew sObjectTypeDefCallback(odef));
  }
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cLang::RegisterModuleDef(const iModuleDef* apDef) {
  niCheckIsOK(apDef,eFalse);
  const tU32 nModuleIndex = GetModuleDefIndex(apDef->GetName());
  if (nModuleIndex != eInvalidHandle) {
    niWarning(niFmt("Module '%s' already registered.", apDef->GetName()));
    return eFalse;
  }
  sModuleDef mod;
  mod.mptrModuleDef = apDef;
  mvModuleDefs.push_back(mod);
  return _FinalizeRegisterModuleDef(mvModuleDefs.back());
}

///////////////////////////////////////////////
tU32 __stdcall cLang::GetNumModuleDefs() const {
  return (tU32)mvModuleDefs.size();
}

///////////////////////////////////////////////
const iModuleDef* __stdcall cLang::GetModuleDef(tU32 anIndex) const {
  if (anIndex >= mvModuleDefs.size()) return NULL;
  return mvModuleDefs[anIndex].mptrModuleDef;
}

///////////////////////////////////////////////
tU32 __stdcall cLang::GetModuleDefIndex(const achar* aaszName) const {
  niLoop(i,mvModuleDefs.size()) {
    if (StrEq(mvModuleDefs[i].mptrModuleDef->GetName(),aaszName))
      return i;
  }
  return eInvalidHandle;
}

///////////////////////////////////////////////
const iModuleDef* __stdcall cLang::LoadModuleDef(const achar* aName, const achar* aaszFile) {
  TRACE_LOAD_MODULE(("LoadModuleDef: name: '%s', file: '%s'.",aName,aaszFile));

  // Check if the module has already been loaded
  {
    const tU32 nModuleIndex = GetModuleDefIndex(aName);
    if (nModuleIndex != eInvalidHandle)
      return mvModuleDefs[nModuleIndex].mptrModuleDef;
  }

#ifndef niNoDLL
  sModuleDef mod;

  // Try to load from the specified file path
  if (niStringIsOK(aaszFile)) {
    TRACE_LOAD_MODULE(("LoadModuleDef: trying from explicit path: %s.",aaszFile));
    mod.mhDLL = ni_dll_load(aaszFile);
    if (mod.mhDLL) {
      TRACE_LOAD_MODULE(("Loaded module DLL '%s' from explicit path '%s'.",aName,aaszFile));
    }
  }

  // Try to load from the binary folder
  if (!mod.mhDLL) {
    cPath path;
    path.SetDirectory(this->GetProperty("ni.dirs.bin").Chars());
    path.SetFile(ni::GetModuleFileName(aName).Chars());
    TRACE_LOAD_MODULE(("LoadModuleDef: trying from binary folder: %s.",path.GetPath().Chars()));
    mod.mhDLL = ni_dll_load(path.GetPath().Chars());
    if (mod.mhDLL) {
      TRACE_LOAD_MODULE(("Loaded module DLL '%s' from bin dir '%s'.",aName,path.GetPath()));
    }
    else {
      TRACE_LOAD_MODULE(("LoadModuleDef: trying from file: %s.",path.GetFile()));
      mod.mhDLL = ni_dll_load(path.GetFile().Chars());
      if (mod.mhDLL) {
        TRACE_LOAD_MODULE(("Loaded module DLL '%s' from system PATH.",aName,path.GetPath()));
      }
    }
  }

  // Load the module definition
  if (mod.mhDLL)
  {
    cString strEntryPoint = "GetModuleDef_";
    strEntryPoint += aName;
    tpfnGetModuleDef pfnGetModuleDef = (tpfnGetModuleDef)ni_dll_get_proc(mod.mhDLL,strEntryPoint.Chars());
    if (pfnGetModuleDef == NULL) {
      niError(niFmt(_A("Module '%s' doesn't have '%s'."),aName,strEntryPoint));
      ni_dll_free(mod.mhDLL);
      return NULL;
    }

    mod.mptrModuleDef = pfnGetModuleDef();
    if (!mod.mptrModuleDef.IsOK()) {
      niError(niFmt(_A("Module '%s' can't get module def."),aName));
      ni_dll_free(mod.mhDLL);
      return NULL;
    }

    if (!_FinalizeRegisterModuleDef(mod)) {
      niError(niFmt(_A("Module '%s' can't finalize registration."),aName));
      ni_dll_free(mod.mhDLL);
      return NULL;
    }

    mvModuleDefs.push_back(mod);
    return mod.mptrModuleDef;
  }
#endif

  niError(niFmt("Can't load module '%s'.", aName));
  return NULL;
}

///////////////////////////////////////////////
iUnknown* __stdcall cLang::CreateInstance(
    const achar* aOID,
    const Var& aVarA, const Var& aVarB)
{
  tCreateInstanceCMap::const_iterator it = mmapCreateInstance->find(aOID);
  if (it == mmapCreateInstance->end()) {
    niError(niFmt("Can't find a callback to create '%s'.", aOID));
    return NULL;
  }

  Ptr<iCallback> ptrCreateCallback = it->second;
  if (!ptrCreateCallback.IsOK()) {
    niError(niFmt("Invalid callback to create '%s'.", aOID));
    return NULL;
  }

  Var r = ptrCreateCallback->RunCallback(aVarA,aVarB);
  if (!r.IsIUnknownPointer()) {
    niError(niFmt("Create '%s' failed.", aOID));
    return NULL;
  }

  return r.GetRawIUnknownPointerAndSetNull();
}

///////////////////////////////////////////////
tBool __stdcall cLang::SetGlobalInstance(const achar* aaszName, iUnknown* apInstance) {
  niCheckIsOK(apInstance,eFalse);
  niCheck(niStringIsOK(aaszName),eFalse);
  astl::upsert(*mmapGlobalInstance, aaszName, apInstance);
  niAssert(apInstance == mmapGlobalInstance->find(aaszName)->second);
  return eTrue;
}
iUnknown* __stdcall cLang::GetGlobalInstance(const achar* aaszName) const {
  tGlobalInstanceCMap::const_iterator it = mmapGlobalInstance->find(aaszName);
  if (it == mmapGlobalInstance->end())
    return NULL;
  return it->second;
}

///////////////////////////////////////////////
iHString* cLang::GetInterfaceName(const tUUID& aUUID) const
{
  tUUIDNameBMap::const_iterator_from itFind = mbmapUUIDNames.findFrom(aUUID);
  if (itFind == mbmapUUIDNames.endFrom()) {
    niLoopit(astl::vector<sModuleDef>::const_iterator,it,mvModuleDefs) {
      const iModuleDef* pDef = it->mptrModuleDef;
      if (pDef) {
        niLoop(i,pDef->GetNumInterfaces()) {
          const sInterfaceDef* pIDef = pDef->GetInterface(i);
          if (*pIDef->mUUID == aUUID) {
            tHStringPtr hspName = _H(pIDef->maszName);
            niThis(cLang)->mbmapUUIDNames.insert(aUUID,hspName);
            return hspName;
          }
        }
      }
    }
    return NULL;
  }
  else {
    return tUUIDNameBMap::secondFrom(itFind);
  }
}

///////////////////////////////////////////////
const tUUID& cLang::GetInterfaceUUID(iHString* ahspName) const
{
  tHStringPtr hspName = ahspName;
  tUUIDNameBMap::const_iterator_to itFind = mbmapUUIDNames.findTo(hspName);
  if (itFind == mbmapUUIDNames.endTo()) {
    if (!HStringIsEmpty(hspName)) {
      niLoopit(astl::vector<sModuleDef>::const_iterator,it,mvModuleDefs) {
        const iModuleDef* pDef = it->mptrModuleDef;
        if (pDef) {
          niLoop(i,pDef->GetNumInterfaces()) {
            const sInterfaceDef* pIDef = pDef->GetInterface(i);
            if (ni::StrEq(niHStr(hspName),pIDef->maszName)) {
              niThis(cLang)->mbmapUUIDNames.insert(*pIDef->mUUID,hspName);
              return *pIDef->mUUID;
            }
          }
        }
      }
    }
    return kuuidZero;
  }
  else {
    return tUUIDNameBMap::firstTo(itFind);
  }
}

///////////////////////////////////////////////
const sInterfaceDef* cLang::GetInterfaceDefFromUUID(const tUUID& aUUID) const
{
  tUUIDDefMap::const_iterator itFind = mmapUUIDDef.find(aUUID);
  if (itFind == mmapUUIDDef.end()) {
    niLoopit(astl::vector<sModuleDef>::const_iterator,it,mvModuleDefs) {
      const iModuleDef* pDef = it->mptrModuleDef;
      if (pDef) {
        niLoop(i,pDef->GetNumInterfaces()) {
          const sInterfaceDef* pIDef = pDef->GetInterface(i);
          if (*pIDef->mUUID == aUUID) {
            niThis(cLang)->mmapUUIDDef[aUUID] = pIDef;
            return pIDef;
          }
        }
      }
    }
    return NULL;
  }
  else {
    return itFind->second;
  }
}

///////////////////////////////////////////////
tBool __stdcall cLang::RegisterEnumDef(const sEnumDef* apEnumDef) {
  tEnumDefMap::const_iterator itEnumDef = mmapEnumDefs.find(apEnumDef->maszName);
  if (itEnumDef == mmapEnumDefs.end()) {
    mmapEnumDefs.insert(eastl::make_pair(apEnumDef->maszName,apEnumDef));
  }
  return eTrue;
}

///////////////////////////////////////////////
const sEnumDef* __stdcall cLang::GetEnumDef(const achar* aEID)
{
  tEnumDefMap::const_iterator itEnumDef = mmapEnumDefs.find(aEID);
  if (itEnumDef == mmapEnumDefs.end())
    return NULL;
  return itEnumDef->second;
}
