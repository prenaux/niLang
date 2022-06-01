#pragma once
#ifndef __DEBUGUI_H_1D08E5C8_8DDF_434F_A92E_7F93CEC742B5__
#define __DEBUGUI_H_1D08E5C8_8DDF_434F_A92E_7F93CEC742B5__

#if defined niEmbedded

#define niInitScriptVMForDebugUI() ;

#else
#include <niUI.h>
#include <niScript.h>

namespace ni {
inline tBool InitScriptVMForDebugUI() {
  ni::Ptr<ni::iScriptVM> vm = ni::ScriptCreateOrGetDefaultVM();
  niCheckIsOK(vm,eFalse);
  vm->Import(_H("gui.ni"),NULL);
  return eTrue;
}

#define niInitScriptVMForDebugUI(UICONTEXT) ni::InitScriptVMForDebugUI()
} // namespace ni

#endif

#endif // __DEBUGUI_H_1D08E5C8_8DDF_434F_A92E_7F93CEC742B5__
