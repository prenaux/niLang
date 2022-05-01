#ifndef __FILEENUM_755321_H__
#define __FILEENUM_755321_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"
#include "../IFile.h"
#include "Path.h"
#include "../STL/map.h"
#include "../IRegex.h"
#include "../Utils/UnknownImpl.h"

#include <stdio.h>

#ifdef niWin32
#include <io.h>
#else
#include "../Platforms/Unix/UnixFindFirst.h"
#endif

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

//! Find file struct
typedef struct sFindFile
{
#ifdef niWin32
  struct _wfinddata_t mFD;
  achar mDirectory[AMAX_PATH];
  void* mpFileName;
#else
  sUnixFFBlk mFD;
#endif
  tIntPtr mHandle;
} sFindFile;

//! Initialize the find file structure.
niExportFunc(void) FindFile_Init(sFindFile* apFF);
//! Close the current find.
niExportFunc(tBool) FindFile_Close(sFindFile* apFF);
//! Reset all file infos.
niExportFunc(void) FindFile_ResetFileInfos(sFindFile* apFF);
//! Find the first file matching the specified path.
niExportFunc(tBool) FindFile_First(sFindFile* apFF, const achar* aaszPath);
//! Find the next file matching the path.
niExportFunc(tBool) FindFile_Next(sFindFile* apFF);
//! Get whether there's a file info available.
niExportFunc(tBool) FindFile_FileExists(const sFindFile* apFF);
//! Get the current directory name.
niExportFunc(const achar*) FindFile_DirName(const sFindFile* apFF);
//! Get the current file name.
niExportFunc(const achar*) FindFile_FileName(const sFindFile* apFF);
//! Get the current file time.
niExportFunc(tI64) FindFile_FileUnixTimeSecs(const sFindFile* apFF);
//! Get the current file size.
niExportFunc(tI64) FindFile_FileSize(const sFindFile* apFF);
//! Get the current file attribs.
niExportFunc(tFileAttrFlags) FindFile_FileAttribs(const sFindFile* apFF);

//! Find file utility class
struct FindFile
{
  //! Constructor
  FindFile() {
    FindFile_Init(&mFF);
  }

  //! Destructor
  ~FindFile() {
    Close();
  }

  //! Reset all file infos.
  void __stdcall ResetFileInfos() {
    FindFile_ResetFileInfos(&mFF);
  }

  //! Get the handle.
  inline tIntPtr __stdcall GetHandle() const {
    return mFF.mHandle;
  }

  //! Find the first file matching the specified path.
  tBool First(const achar* aaszPath) {
    return FindFile_First(&mFF,aaszPath);
  }
  //! Find the next file matching the path.
  tBool Next() {
    return FindFile_Next(&mFF);
  }
  //! Close the current find.
  tBool Close () {
    return FindFile_Close(&mFF);
  }

  //! Get the directory name.
  inline const achar* __stdcall DirName() const {
    return FindFile_DirName(&mFF);
  }

  //! Get whether there's a file info available.
  inline tBool __stdcall FileExists() const {
    return FindFile_FileExists(&mFF);
  }
  //! Get the current file name.
  inline const achar* __stdcall FileName() const {
    return FindFile_FileName(&mFF);
  }
  //! Get the current file time.
  inline tI64 __stdcall FileTime() const {
    return FindFile_FileUnixTimeSecs(&mFF);
  }
  //! Get the current file size.
  inline tI64 __stdcall FileSize() const {
    return FindFile_FileSize(&mFF);
  }
  //! Get the current file attribs.
  inline tFileAttrFlags __stdcall FileAttribs() const {
    return FindFile_FileAttribs(&mFF);
  }

 private:
  sFindFile mFF;
};

//! File info structure
struct sFileInfo
{
  tU32    count;
  cString   name;
  tI64    size;
  time_t    time;
  tU32    attribs;
  sFileInfo() {
    count = 0;
    size = 0;
    memset(&time,0,sizeof(time));
    attribs = 0;
  }
};

//! Enumerate the files found in the specified folder.
//! \param  aszFile is the path with filter of what to enumerate.
//! \param  flAttribs is a filter that defines the attributes that will be enumerated.
//! \param  apSink is the callback interface that will be called when a file is found.
//! \param  aFiles is a vector that will contain the list of files found.
//! \param  anMax if > 0, is the maximum number of file returned.
//! \return The number of files found. eInvalidHandle indicates an error.
//! \remark You can specify a list of file patterns to further filer which
//!         files are enumerated. Example: "/Users/foo/|*.txt|*.png|!_*".
//!         Use '!' to negate the pattern and act as an exclusion filter.
niExportFunc(tU32) FileEnum(const achar* aszFile, tU32 flAttribs, iFileEnumSink* apSink, astl::vector<sFileInfo>* aFiles, tU32 anMax);

static inline astl::vector<sFileInfo> FileEnum(const achar* aszFile) {
  astl::vector<sFileInfo> found;
  ni::FileEnum(aszFile, eFileAttrFlags_AllFiles, NULL, &found, 0);
  return found;
}

//! Get the first matching file path.
//! \remark One of the primary use of this is to get the case sensitive
//!         filename of a case insensitive path.
//! \remark We return a cPath object because we want to preserve the path if
//!         we're going to use it directly and we have to use one anyway since
//!         the FindFile API returns only the filename.
static inline cPath FindFirstFilePath(const achar* aPath) {
  FindFile ff;
  tBool found = ff.First(aPath);
  if (found) {
    cPath path(ff.DirName(),ff.FileName());
    return path;
  }
  else {
    return cPath();
  }
}

//! Get the first matching directory path.
//! \remark One of the primary use of this is to get the case sensitive
//!         filename of a case insensitive path.
//! \remark We return a cPath object because we want to preserve the path if
//!         we're going to use it directly and we have to use one anyway since
//!         the FindFile API returns only the filename.
static inline cPath FindFirstDirPath(const achar* aPath) {
  FindFile ff;
  tBool found = ff.First(aPath);
  if (found) {
    cPath path(ff.DirName(),NULL);
    path.AddDirectoryBack(ff.FileName());
    return path;
  }
  else {
    return cPath();
  }
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
} // end of namespace ni
#endif // __FILEENUM_755321_H__
