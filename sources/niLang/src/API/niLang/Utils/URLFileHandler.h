#pragma once
#ifndef __URLFILEHANDLER_H_015E7601_79FC_F74F_9B1C_99F4E5A2DF23__
#define __URLFILEHANDLER_H_015E7601_79FC_F74F_9B1C_99F4E5A2DF23__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../ILang.h"
#include "../IFileSystem.h"
#include "../../niLang_ModuleDef.h"

namespace ni {

#if defined niIOS
#define niNoToolkitDir
#endif

#if defined niNoToolkitDir
//
// iOS only has a hellish way to copy/reference data at build time, the only
// sane way I found is to reference the module's data folder at the root of
// the Xcode project. That means no explicit toolkit directory so we don't
// expose those on iOS to avoid using them inadvertently there.
//

static inline cString GetModuleDataDir(const achar* /*aToolkitName*/, const achar* aModuleName) {
  cString dir = ni::GetRootFS()->GetAbsolutePath(
    ni::GetLang()->GetProperty("ni.dirs.data").Chars());
  dir += "/";
  dir += aModuleName;
  dir += "/";
  return dir;
}

#else
///////////////////////////////////////////////
static inline cString GetToolkitBinDir(const achar* aToolkitName, const achar* aSubDir) {
  cString r = ni::GetRootFS()->GetAbsolutePath(
    (ni::GetLang()->GetProperty("ni.dirs.bin") +
     "../../../" + aToolkitName + "/").Chars());
  if (niStringIsOK(aSubDir)) {
    r += aSubDir;
    r += "/";
  }
  return r;
}

///////////////////////////////////////////////
static inline cString GetToolkitDir(const achar* aToolkitName, const achar* aSubDir) {
  cString r = ni::GetRootFS()->GetAbsolutePath(
    (ni::GetLang()->GetProperty("ni.dirs.data") +
     "../../").Chars());
  if (niStringIsOK(aToolkitName)) {
    r += aToolkitName;
    r += "/";
  }
  if (niStringIsOK(aSubDir)) {
    r += aSubDir;
    r += "/";
  }
  return r;
}

///////////////////////////////////////////////
static inline cString GetModuleDataDir(const achar* aToolkitName, const achar* aModuleName) {
  return GetToolkitDir(aToolkitName,"data") + aModuleName + "/";
}
#endif

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

static inline void RegisterModuleDataDirDefaultURLFileHandler(const achar* aToolkitName, const achar* aModuleName) {
  const cString instanceName = niFmt("URLFileHandler.%s", aModuleName);
  if (ni::GetLang()->GetGlobalInstance(instanceName.Chars())) {
    return;
  }
  cString dir = GetModuleDataDir(aToolkitName, aModuleName);
  Ptr<iURLFileHandler> fileHandler = CreateURLFileHandlerDir(dir.Chars());
  niPanicAssertMsg(
    fileHandler.IsOK(),
    niFmt("Can't create the file handler for the data directory '%s' of the module '%s::%s', check that the '$WORK/%s/data/%s' folder exists.",
          dir,
          aToolkitName, aModuleName,
          aToolkitName, aModuleName));
  ni::GetLang()->SetGlobalInstance(instanceName.Chars(), fileHandler);
  return;
}

} // namespace ni
#endif // __URLFILEHANDLER_H_015E7601_79FC_F74F_9B1C_99F4E5A2DF23__
