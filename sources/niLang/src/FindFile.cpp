// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Types.h"
#include "API/niLang/Utils/FileEnum.h"
#if defined niWin32
#include "API/niLang/Platforms/Win32/Win32_UTF.h"
#endif
#include "FileFd.h"
#include "API/niLang/Utils/Path.h"

namespace ni {

niExportFunc(void) FindFile_Init(sFindFile* apFF) {
  apFF->mHandle = -1;
#if defined niWin32
  apFF->mpFileName = NULL;
  *((tU32*)apFF->mDirectory) = 0;
#endif
  FindFile_ResetFileInfos(apFF);
}

niExportFunc(tBool) FindFile_Close(sFindFile* apFF) {
  if (apFF->mHandle >= 0) {
#ifdef niWin32
    _findclose(apFF->mHandle);
#else
    unix_findclose(&apFF->mFD);
#endif
    apFF->mHandle = -1;
  }
#if defined niWin32
  if (apFF->mpFileName) {
    ni::Windows::UTF8Buffer* pBuf = (ni::Windows::UTF8Buffer*)apFF->mpFileName;
    niDelete pBuf;
    apFF->mpFileName = NULL;
  }
#endif
  FindFile_ResetFileInfos(apFF);
  return niTrue;
}

niExportFunc(void) FindFile_ResetFileInfos(sFindFile* apFF) {
  memset(&apFF->mFD,0,sizeof(apFF->mFD));
}

niExportFunc(tBool) FindFile_First(sFindFile* apFF, const achar* aaszPath) {
  FindFile_Close(apFF);
  if (!niIsStringOK(aaszPath))
    return niFalse;

  const cPath path(aaszPath);
#ifdef niWin32
  ni::Windows::UTF16Buffer wTmpPath;
  if (path.IsDirectory()) {
    // remove the trailing slash that findfirst doesnt handle as we want
    cString& dir = const_cast<cString&>(path.GetDirectory());
    dir.Remove();
    niWin32_UTF8ToUTF16(wTmpPath,dir.Chars());
    // mDirectory should be the parent directory of what we're searching
    StrCpy(apFF->mDirectory,dir.RBefore("/").Chars());
    StrCat(apFF->mDirectory,"/");
  }
  else {
    StrCpy(apFF->mDirectory,path.GetDirectory().Chars());
    niWin32_UTF8ToUTF16(wTmpPath,path.GetPath().Chars());
  }
  apFF->mHandle = _wfindfirst(wTmpPath.begin(),&apFF->mFD);
#else
  if (path.IsDirectory()) {
    cString& dir = const_cast<cString&>(path.GetDirectory());
    dir.Remove(); // remove the trailing slash that findfirst doesnt handle as we want
    apFF->mHandle = (unix_findfirst(dir.Chars(),&apFF->mFD) == 0) ? 0 : -1;
  }
  else {
    apFF->mHandle = (unix_findfirst(path.GetPath().Chars(),&apFF->mFD) == 0) ? 0 : -1;
  }
#endif
  return apFF->mHandle >= 0;
}

niExportFunc(tBool) FindFile_Next(sFindFile* apFF) {
  if (apFF->mHandle < 0) return niFalse;
#ifdef niUnix
  return unix_findnext(&apFF->mFD) == 0;
#else
  return _wfindnext(apFF->mHandle, &apFF->mFD) == 0;
#endif
}

niExportFunc(tBool) FindFile_FileExists(const sFindFile* apFF) {
  return apFF->mHandle >= 0;
}

niExportFunc(const achar*) FindFile_DirName(const sFindFile* apFF) {
#if defined niWin32
  return apFF->mDirectory;
#else
  return apFF->mFD.info.dirname;
#endif
}

niExportFunc(const achar*) FindFile_FileName(const sFindFile* apFF) {
#if defined niWin32
  ni::Windows::UTF8Buffer* pBuf = (ni::Windows::UTF8Buffer*)apFF->mpFileName;
  if (!pBuf) {
    pBuf = new ni::Windows::UTF8Buffer();
    const_cast<sFindFile*>(apFF)->mpFileName = (void*)pBuf;
  }
  niWin32_UTF16ToUTF8(*pBuf,apFF->mFD.name);
  return pBuf->begin();
#else
  return apFF->mFD.name;
#endif
}

niExportFunc(tI64) FindFile_FileUnixTimeSecs(const sFindFile* apFF) {
  return apFF->mFD.time_write;
}

niExportFunc(tI64) FindFile_FileSize(const sFindFile* apFF) {
  return apFF->mFD.size;
}

niExportFunc(tFileAttrFlags) FindFile_FileAttribs(const sFindFile* apFF) {
  if (apFF->mHandle < 0) return 0;
  const tU32 attrs = apFF->mFD.attrib;
  tFileAttrFlags nFlags = 0;
  if (attrs & _A_SUBDIR)  nFlags |= eFileAttrFlags_Directory;
  else          nFlags |= eFileAttrFlags_File;
  if (attrs & _A_ARCH)  nFlags |= eFileAttrFlags_Archive;
  if (attrs & _A_HIDDEN)  nFlags |= eFileAttrFlags_Hidden;
  if (attrs & _A_RDONLY)  nFlags |= eFileAttrFlags_ReadOnly;
  if (attrs & _A_SYSTEM)  nFlags |= eFileAttrFlags_System;
  return nFlags;
}

}
