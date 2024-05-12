#pragma once
#ifndef __URLFILEHANDLERDIRECTORY_H_692E9AA7_6504_984D_9D3C_0C906E64D859__
#define __URLFILEHANDLERDIRECTORY_H_692E9AA7_6504_984D_9D3C_0C906E64D859__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "UnknownImpl.h"
#include "../IFile.h"

namespace ni {

struct URLFileHandler_Directories : public ImplRC<iURLFileHandler> {
  astl::vector<cString> _dirs;

  URLFileHandler_Directories(const achar* aAppDir, astl::vector<cString> aToolkits) {
    niLoop(i,aToolkits.size()) {
      _AddToolkitDirs(aAppDir, aToolkits[i].c_str());
    }

    // App/data
    {
      cPath path;
      path.SetDirectory(aAppDir);
      path.AddDirectoryBack("data");
      _dirs.push_back(path.GetPath());
    }
    // App/scripts
    {
      cPath path;
      path.SetDirectory(aAppDir);
      path.AddDirectoryBack("scripts");
      _dirs.push_back(path.GetPath());
    }
    // App
    {
      cPath path;
      path.SetDirectory(aAppDir);
      _dirs.push_back(path.GetPath());
    }
  }

  // Toolkit directories are assumed to be at the same level as the app's directory
  void _AddToolkitDirs(const achar* aAppDir, const achar* aToolkitName) {
    {
      cPath path;
      path.SetDirectory(aAppDir);
      path.RemoveDirectoryBack();
      path.AddDirectoryBack(aToolkitName);
      path.AddDirectoryBack("data");
      _dirs.push_back(path.GetPath());
    }
    {
      cPath path;
      path.SetDirectory(aAppDir);
      path.RemoveDirectoryBack();
      path.AddDirectoryBack(aToolkitName);
      path.AddDirectoryBack("scripts");
      _dirs.push_back(path.GetPath());
    }
  }

  virtual iFile* __stdcall URLOpen(const achar* aURL) {
    const cString path = _FindFile(StringURLGetPath(aURL));
    // niDebugFmt(("... URLFileHandler.dir.URLOpen: %s -> %s", aURL, path));
    if (path.IsEmpty()) return NULL;
    return ni::GetRootFS()->FileOpen(path.Chars(),eFileOpenMode_Read);
  }

  virtual tBool __stdcall URLExists(const achar* aURL) {
    const cString path = _FindFile(StringURLGetPath(aURL));
    if (path.IsEmpty()) return eFalse;
    return eTrue;
  }

  cString __stdcall _FindFile(const cString& basePath) {
    if (ni::GetRootFS()->FileExists(basePath.Chars(),eFileAttrFlags_AllFiles)) {
      return basePath;
    }
    niLoopr(i,_dirs.size()) {
      cString tryPath = _dirs[i] + basePath;
      if (ni::GetRootFS()->FileExists(tryPath.Chars(),eFileAttrFlags_AllFiles)) {
        return tryPath;
      }
    }
    return AZEROSTR;
  }
};

}
#endif // __URLFILEHANDLERDIRECTORY_H_692E9AA7_6504_984D_9D3C_0C906E64D859__
