#pragma once
#ifndef __URLHANDLERMANIFEST_H_717267A8_D672_40A4_ADBB_CD27296AA55C__
#define __URLHANDLERMANIFEST_H_717267A8_D672_40A4_ADBB_CD27296AA55C__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../ILang.h"
#include "../IFileSystem.h"
#include "../../niLang_ModuleDef.h"

namespace ni {

static inline Ptr<iURLFileHandler> CreateURLFileHandlerFileSystem(iFileSystem* apFS, tBool abManifest = eFalse) {
  Ptr<iURLFileHandler> fileHandler = (iURLFileHandler*)(abManifest ?
      niCreateInstance(niLang,URLFileHandlerManifestFileSystem,apFS,niVarNull) :
      niCreateInstance(niLang,URLFileHandlerFileSystem,apFS,niVarNull));
  niCheckIsOK(fileHandler,NULL);
  return fileHandler;
}

static inline Ptr<iURLFileHandler> CreateURLFileHandlerDir(const achar* aDir, tBool abManifest = eFalse, tFileSystemRightsFlags aFlags = eFileSystemRightsFlags_IOOnly) {
  cString absDir = ni::GetRootFS()->GetAbsolutePath(aDir);
  if (!ni::GetRootFS()->FileExists(absDir.Chars(), eFileAttrFlags_Directory)) {
    niError(niFmt("Can't find directory '%s' at abs path '%s'.", aDir, aDir));
    return NULL;
  }

  Ptr<iFileSystem> fileSystem = ni::GetLang()->CreateFileSystemDir(absDir.Chars(), aFlags);
  niCheckIsOK(fileSystem,NULL);

  Ptr<iURLFileHandler> fileHandler = (iURLFileHandler*)(abManifest ?
      niCreateInstance(niLang,URLFileHandlerManifestFileSystem,fileSystem,niVarNull) :
      niCreateInstance(niLang,URLFileHandlerFileSystem,fileSystem,niVarNull));
  niCheckIsOK(fileHandler,NULL);
  return fileHandler;
}

static inline Ptr<iURLFileHandler> CreateURLFileHandlerZip(iFile* apZipFile) {
  Ptr<iURLFileHandler> fileHandler = (iURLFileHandler*)niCreateInstance(niLang,URLFileHandlerZip,apZipFile,niVarNull);
  niCheckIsOK(fileHandler,NULL);
  return fileHandler;
}

static inline Ptr<iURLFileHandler> CreateURLFileHandlerPrefixed(iURLFileHandler* apFileHandler, const achar* aPrefix) {
  Ptr<iURLFileHandler> fileHandler = (iURLFileHandler*)niCreateInstance(niLang,URLFileHandlerPrefixed,apFileHandler,aPrefix);
  niCheckIsOK(fileHandler,NULL);
  return fileHandler;
}

static inline tBool RegisterModuleDataDirDefaultURLFileHandler(const achar* aToolkitName, const achar* aModuleName) {
  const cString instanceName = niFmt("URLFileHandler.%s", aModuleName);
  if (ni::GetLang()->GetGlobalInstance(instanceName.Chars())) {
    return eTrue;
  }
  cString dir = GetModuleDataDir(aToolkitName, aModuleName);
  Ptr<iURLFileHandler> fileHandler = CreateURLFileHandlerDir(dir.Chars());
  if (!fileHandler.IsOK())
    return eFalse;
  ni::GetLang()->SetGlobalInstance(instanceName.Chars(), fileHandler);
  return eTrue;
}

} // namespace ni
#endif // __URLHANDLERMANIFEST_H_717267A8_D672_40A4_ADBB_CD27296AA55C__
