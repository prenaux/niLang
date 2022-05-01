// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "Lang.h"
#ifdef niJNI
#include "API/niLang/Utils/JNIUtils.h"
#endif
#include "CppScriptingHost.h"

using namespace ni;

#if niMinFeatures(15)

///////////////////////////////////////////////
void __stdcall cLang::_InitializeScriptingHosts() {
#if !defined niNoRTCpp
  Ptr<iScriptingHost> ptrScriptingHost = RTCpp_CreateScriptingHost();
  AddScriptingHost(_H("cpp"),ptrScriptingHost.ptr());
  AddScriptingHost(_H("cni"),ptrScriptingHost.ptr());
#endif
}

///////////////////////////////////////////////
tBool __stdcall cLang::AddScriptingHost(iHString* ahspName, iScriptingHost* apHost) {
  niCheckIsOK(apHost,eFalse);
  niCheck(HStringIsNotEmpty(ahspName),eFalse);
  if (GetScriptingHostFromName(ahspName))
    return eTrue; // already added

  __sync_lock_(ScriptingHosts);
  sSH sh;
  sh.hspName = ahspName;
  sh.ptrHost = apHost;
  mvSH.push_back(sh);
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cLang::RemoveScriptingHost(iHString* ahspName) {
  tU32 index = GetScriptingHostIndexFromName(ahspName);
  if (index == eInvalidHandle)
    return eFalse;

  __sync_lock_(ScriptingHosts);
  mvSH.erase(mvSH.begin()+index);
  return eTrue;
}

///////////////////////////////////////////////
tU32 __stdcall cLang::GetNumScriptingHosts() const {
  __sync_lock_(ScriptingHosts);
  return (tU32)mvSH.size();
}

///////////////////////////////////////////////
iHString* __stdcall cLang::GetScriptingHostName(tU32 anIndex) const {
  __sync_lock_(ScriptingHosts);
  niCheckSilent(anIndex < mvSH.size(),NULL);
  return mvSH[anIndex].hspName;
}

///////////////////////////////////////////////
iScriptingHost* __stdcall cLang::GetScriptingHost(tU32 anIndex) {
  __sync_lock_(ScriptingHosts);
  niCheckSilent(anIndex < mvSH.size(),NULL);
  return mvSH[anIndex].ptrHost;
}

///////////////////////////////////////////////
tU32 __stdcall cLang::GetScriptingHostIndexFromName(iHString* ahspName) {
  __sync_lock_(ScriptingHosts);
  niLoop(i,mvSH.size()) {
    if (mvSH[i].hspName == ahspName)
      return i;
  }
  return eInvalidHandle;
}

///////////////////////////////////////////////
iScriptingHost* __stdcall cLang::GetScriptingHostFromName(iHString* ahspName) {
  return GetScriptingHost(GetScriptingHostIndexFromName(ahspName));
}

///////////////////////////////////////////////
tU32 __stdcall cLang::GetScriptingHostIndex(iScriptingHost* apHost) const {
  __sync_lock_(ScriptingHosts);
  niLoop(i,mvSH.size()) {
    if (mvSH[i].ptrHost == apHost)
      return i;
  }
  return eInvalidHandle;
}

///////////////////////////////////////////////
void __stdcall cLang::ServiceAllScriptingHosts(tBool abForceGC) {
  __sync_lock_(ScriptingHosts);
#ifdef niJNI
  if (niJVM_HasVM()) {
    niJVM_Service(niJVM_GetCurrentEnv());
  }
#endif
  niLoop(i,mvSH.size()) {
    mvSH[i].ptrHost->Service(abForceGC);
  }
}

///////////////////////////////////////////////
iScriptingHost* __stdcall cLang::FindScriptingHost(iHString* ahspContext, iHString* ahspCodeResource) {
  __sync_lock_(ScriptingHosts);
  niLoopr(ri,mvSH.size()) {
    if (mvSH[ri].ptrHost->CanEvalImpl(ahspContext,ahspCodeResource)) {
      return mvSH[ri].ptrHost;
    }
  }
  return NULL;
}
#endif
