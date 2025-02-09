// clang-format off

#include <niLang/Types.h>
#include <niLang/Utils/ObjModelImpl.h>
#include <niLang/Utils/ModuleDefImpl_Begin.h>

#ifndef niConfig_OnlyObjectTypesIDL
#endif // #ifndef niConfig_OnlyObjectTypesIDL



#if !defined niConfig_OnlyObjectTypesIDL && !defined niConfig_NoInterfaceDef
#define _niScriptCpp_TestModule_ModuleDef
#include <niLang/Utils/VMCallCImpl.h>
#ifndef niConfig_NoXCALL
#define IDLC_BEGIN_NAMESPACE() namespace  { namespace xcall_cimpl {
#define IDLC_END_NAMESPACE() }}
#ifndef XCALL_CIMPL
#define XCALL_CIMPL(NAME) ::xcall_cimpl::NAME
#endif
#endif // niConfig_NoXCALL
#endif // #if !defined niConfig_OnlyObjectTypesIDL && !defined niConfig_NoInterfaceDef

namespace  {
#if !defined niConfig_OnlyObjectTypesIDL && !defined niConfig_NoInterfaceDef

#endif // #if !defined niConfig_OnlyObjectTypesIDL && !defined niConfig_NoInterfaceDef



//////////////////////////////////////////////////////////////////////////////////////////////
// GetModuleDef
//////////////////////////////////////////////////////////////////////////////////////////////
niExportFunc(const ni::iModuleDef*) GetModuleDef_niScriptCpp_TestModule()
{
  static bool _initialized = false;
  if (!_initialized) {
    _initialized = true;
    niModuleDefImpl_GetModule_Register();
  }

//////////////////////////////////////////////////////////////////////////////////////////////
// Object types definition
//////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////
// Module definition
//////////////////////////////////////////////////////////////////////////////////////////////
static const ni::achar* Dependencies[] = {
  "niLang",
};

#if !defined niConfig_OnlyObjectTypesIDL && !defined niConfig_NoInterfaceDef

#endif // #if !defined niConfig_OnlyObjectTypesIDL && !defined niConfig_NoInterfaceDef

static const ni::sModuleDef ModuleDef(
  "niScriptCpp_TestModule",
  "1,0,0",
  "niScriptCpp test module (Build Date: " __DATE__ ")",
  "",
  "",
  1, Dependencies,
#if !defined niConfig_OnlyObjectTypesIDL && !defined niConfig_NoInterfaceDef
  0, NULL,
#else
  0, NULL,
#endif
#if !defined niConfig_OnlyObjectTypesIDL && !defined niConfig_NoEnumDef
  0, NULL,
#else
  0, NULL,
#endif
#if !defined niConfig_OnlyObjectTypesIDL && !defined niConfig_NoConstDef
  0, NULL,
#else
  0, NULL,
#endif
  0, NULL
);

  return &ModuleDef;
} // End of GetModuleDef_niScriptCpp_TestModule
} // end of namespace

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#include <niLang/Utils/ModuleDefImpl_End.h>
