#pragma once
#ifndef __DRAWOPHUD_H_68D709F5_2081_4157_AABF_1AE5649D4C47__
#define __DRAWOPHUD_H_68D709F5_2081_4157_AABF_1AE5649D4C47__

#if defined niEmbedded

#define niInitializeVMForDrawOpHud() ;
#else

#include <niLang.h>
#include <niScript.h>
#include <niLang/Utils/URLFileHandler.h>

namespace ni {

static ni::tBool InitializeVMForDrawOpHud(ni::iScriptVM* apVM) {
  ni::QPtr<ni::iScriptVM> vm = apVM;
  if (!vm.IsOK()) {
    vm = ni::CreateConcurrentScriptVM(ni::GetConcurrent());
  }
  if (vm.IsOK()) {
    if (!ni::GetLang()->GetScriptingHostFromName(_H("ni"))) {
      cString path = GetToolkitDir("niLang","scripts");
      if (ni::GetRootFS()->FileExists(path.Chars(),eFileAttrFlags_AllDirectories)) {
        Ptr<iFileSystem> scriptsDirFS = ni::GetLang()->CreateFileSystemDir(
            path.Chars(),ni::eFileSystemRightsFlags_ReadOnly);
        vm->GetImportFileSystems()->Add(scriptsDirFS.ptr());
      }
      else {
        niWarning(niFmt("Can't add include directory '%s'.",path));
      }
    }
    if (!vm->Import(_H("gui.ni"),NULL))
      return eFalse;
    ni::GetLang()->AddScriptingHost(_H("ni"),vm);
  }
  return vm.IsOK();
}

#define niInitializeVMForDrawOpHud() ni::InitializeVMForDrawOpHud(NULL)
}

#endif

#endif // __DRAWOPHUD_H_68D709F5_2081_4157_AABF_1AE5649D4C47__
