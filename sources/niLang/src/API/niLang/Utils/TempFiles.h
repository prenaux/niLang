#pragma once
#ifndef __TEMPFILES_H_EAE428FB_5516_4A3D_9C58_B3DB78FEC99E__
#define __TEMPFILES_H_EAE428FB_5516_4A3D_9C58_B3DB78FEC99E__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"
#include "../StringDef.h"
#include "../Utils/Path.h"
#include "../Utils/UUID.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

#include <niLang/Utils/UUID.h>
struct sTempFiles {
  __sync_mutex();
  tBool _deleteTempFilesOnDestruct;
  cString _tempDir;
  astl::vector<cString> _tempFilesCollector;

  sTempFiles(tBool abDeleteTempFilesOnDestruct) : _deleteTempFilesOnDestruct(abDeleteTempFilesOnDestruct) {
  }
  ~sTempFiles() {
    if (_deleteTempFilesOnDestruct) {
      DeleteTempFiles();
    }
  }

  const achar* GetTempDir() {
    __sync_lock();
    if (!_tempDir.empty())
      return _tempDir.Chars();

    {
      cPath pathTemp;
      pathTemp.SetDirectory(GetLang()->GetProperty("ni.dirs.temp").Chars());
      if (GetRootFS()->FileExists(pathTemp.GetPath().Chars(),eFileAttrFlags_AllDirectories)) {
        _tempDir = pathTemp.GetPath();
        return _tempDir.Chars();
      }
    }

    {
      cPath pathTemp;
      pathTemp.SetDirectory(GetLang()->GetEnv("TEMP").Chars());
      if (GetRootFS()->FileExists(pathTemp.GetPath().Chars(),eFileAttrFlags_AllDirectories)) {
        _tempDir = pathTemp.GetPath();
        return _tempDir.Chars();
      }
    }

    {
      cPath pathTemp;
      pathTemp.SetDirectory(GetLang()->GetEnv("TMP").Chars());
      if (GetRootFS()->FileExists(pathTemp.GetPath().Chars(),eFileAttrFlags_AllDirectories)) {
        _tempDir = pathTemp.GetPath();
        return _tempDir.Chars();
      }
    }

    {
      cPath pathTemp;
      pathTemp.SetDirectory(GetLang()->GetProperty("ni.dirs.home").Chars());
      pathTemp.AddDirectoryBack("_ham/_temp");
      if (GetRootFS()->FileMakeDir(pathTemp.GetPath().Chars())) {
        _tempDir = pathTemp.GetPath();
        return _tempDir.Chars();
      }
    }

    {
      cPath pathTemp;
      pathTemp.SetDirectory(GetLang()->GetProperty("ni.dirs.home").Chars());
      _tempDir = pathTemp.GetPath();
      return _tempDir.Chars();
    }
  }

  cString GetNewTempFilePath(const achar* aExt, const achar* aName) {
    __sync_lock();
    cPath pathFile;
    pathFile.SetDirectory(GetTempDir());
    pathFile.SetFile((_ASTR(niStringIsOK(aName) ? aName : "_temp_") + sUUID(GetLang()->CreateGlobalUUID()).ToString()).Chars());
    pathFile.SetExtension(niStringIsOK(aExt) ? aExt : "tmp");
    _tempFilesCollector.push_back(pathFile.GetPath());
    return pathFile.GetPath();
  }

  void DeleteTempFiles() {
    __sync_lock();
    niLoop(i, _tempFilesCollector.size()) {
      GetRootFS()->FileDelete(_tempFilesCollector[i].Chars());
    }
  }
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __TEMPFILES_H_EAE428FB_5516_4A3D_9C58_B3DB78FEC99E__
