#pragma once
#ifndef __IFILESYSTEM_8014780_H__
#define __IFILESYSTEM_8014780_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "IFile.h"
#include "Utils/CollectionImpl.h"

namespace ni {

struct iFileSystem;
struct iFileSystemEnumerator;

/** \addtogroup niLang
 * @{
 */

//! File system rights.
enum eFileSystemRightsFlags
{
  //! Right to enumerate files.
  eFileSystemRightsFlags_Enum = niBit(0),
  //! Right to read from files.
  eFileSystemRightsFlags_Read = niBit(1),
  //! Right to write in files, but not to create files.
  eFileSystemRightsFlags_Write = niBit(2),
  //! Right to create files.
  eFileSystemRightsFlags_Create = niBit(3),
  //! Right to delete files.
  eFileSystemRightsFlags_Delete = niBit(4),
  //! Right to execute files.
  eFileSystemRightsFlags_Execute = niBit(5),
  //! Read-only system
  eFileSystemRightsFlags_ReadOnly = eFileSystemRightsFlags_Enum|eFileSystemRightsFlags_Read,
  //! Write-only system
  eFileSystemRightsFlags_WriteOnly = eFileSystemRightsFlags_Write,
  //! Read-Write-only system
  eFileSystemRightsFlags_ReadWriteOnly = eFileSystemRightsFlags_Enum|eFileSystemRightsFlags_Read|eFileSystemRightsFlags_Write,
  //! IO rights only. All excepted execution.
  eFileSystemRightsFlags_IOOnly = eFileSystemRightsFlags_Read|eFileSystemRightsFlags_Enum|eFileSystemRightsFlags_Write|eFileSystemRightsFlags_Create|eFileSystemRightsFlags_Delete,
  //! All rights
  eFileSystemRightsFlags_All = eFileSystemRightsFlags_Read|eFileSystemRightsFlags_Enum|eFileSystemRightsFlags_Write|eFileSystemRightsFlags_Create|eFileSystemRightsFlags_Delete|eFileSystemRightsFlags_Execute,
  //! \internal
  eFileSystemRightsFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! File system rights flags type. \see ni::eFileSystemRightsFlags
typedef tU32 tFileSystemRightsFlags;

//! File system enumerator interface.
struct iFileSystemEnumerator : public iUnknown
{
  niDeclareInterfaceUUID(iFileSystemEnumerator,0x85fb1784,0x1a48,0x4581,0xa3,0xaf,0xa7,0xd9,0x65,0x3f,0x5f,0x77)
  //! Get the owner file system.
  //! {Property}
  virtual iFileSystem* __stdcall GetFileSystem() const = 0;
  //! Find the first file matching the specified pattern.
  virtual tBool __stdcall FindFirst(const achar* aaszFilePattern) = 0;
  //! Find the next file matching the specified pattern.
  //! \return eFalse if there's no more file matching the specified pattern.
  virtual tBool __stdcall FindNext() = 0;
  //! Get the current file name.
  //! {Property}
  virtual const achar* __stdcall GetFileName() const = 0;
  //! Get the current file size.
  //! {Property}
  virtual tI64 __stdcall GetFileSize() const = 0;
  //! Get the current file attributes.
  //! {Property}
  virtual tFileAttrFlags __stdcall GetFileAttributes() const = 0;
  //! Get the current file last write time.
  //! {Property}
  virtual iTime* __stdcall GetFileTime() const = 0;
};

//! File system interface.
struct iFileSystem : public iUnknown
{
  niDeclareInterfaceUUID(iFileSystem,0x0a9c86eb,0xe95e,0x43a1,0xa7,0x24,0x58,0xb5,0x79,0x69,0x71,0x76)

  //! Get the file system's rights.
  //! {Property}
  virtual tFileSystemRightsFlags __stdcall GetRightsFlags() const = 0;
  //! Get the file system's base container.
  //! \remark This depend on the type of file system, it can be a file, a
  //!         folder, a remote location, and so on...
  //! {Property}
  virtual const achar* __stdcall GetBaseContainer() const = 0;

  //! Create a directory.
  virtual tBool __stdcall FileMakeDir(const achar* aszDir) = 0;
  //! Delete a directory.
  virtual tBool __stdcall FileDeleteDir(const achar* aszDir) = 0;
  //! Copy a file.
  virtual tBool __stdcall FileCopy(const achar* aszDest, const achar* aszSrc) = 0;
  //! Move a file.
  virtual tBool __stdcall FileMove(const achar* aszDest, const achar* aszSrc) = 0;
  //! Delete a file.
  virtual tBool __stdcall FileDelete(const achar* aszFile) = 0;
  //! Enumerate files in the specified directory. \see ni::FileEnum
  //! \param  aszFile is the path with filter of what to enumerate.
  //! \param  aAttribs is a filter that defines the attributes that will be enumerated.
  //! \param  pSink is the callback interface that will be called when a file is found.
  //! \return The number of files found. eInvalidHandle indicates an error.
  virtual tU32  __stdcall FileEnum(const achar* aszFile, tU32 aAttribs, iFileEnumSink* pSink) = 0;
  //! Checks if a file exists.
  //! \param  aszFile is the path to check.
  //! \param  aAttribs is a filter that defines the attributes that will be enumerated.
  //! \return 0 if the file doesnt exist else its attributes.
  virtual tU32  __stdcall FileExists(const achar* aszFile, tU32 aAttribs) = 0;
  //! Get the size of a file.
  virtual tI64  __stdcall FileSize(const achar* aszFile) = 0;
  //! Open a file for a read and/or write operation.
  virtual iFile*  __stdcall FileOpen(const achar* aszFile, eFileOpenMode aMode) = 0;
  //! Open a file for a read and/or write operation.
  virtual iFileBase*  __stdcall FileBaseOpen(const achar* aszFile, eFileOpenMode aMode) = 0;

  //! Get the absolute path of the given file or directory.
  virtual cString __stdcall GetAbsolutePath(const achar* aaszFile) const = 0;
  //! Create a file system enumerator.
  virtual iFileSystemEnumerator* __stdcall CreateEnumerator() = 0;
};

//! Get the root file system object.
niExportFunc(ni::iFileSystem*) GetRootFS();
//! Create a file system with access restricted to the specified directory.
niExportFunc(ni::iFileSystem*) CreateFileSystemDir(const ni::achar* aaszDir, ni::tFileSystemRightsFlags aRights);
//! Creates a file system that store the file using its hashed file name.
niExportFunc(ni::iFileSystem*) CreateFileSystemHashed(const iFileSystem* apFS);

inline tBool FileExists(const achar* path, iFileSystem* fs = ni::GetRootFS()) {
  return !!(fs->FileExists(path,eFileAttrFlags_AllFiles) & eFileAttrFlags_File);
}
inline tBool DirExists(const achar* path, iFileSystem* fs = ni::GetRootFS()) {
  return !!(fs->FileExists(path,eFileAttrFlags_AllDirectories) & eFileAttrFlags_Directory);
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IFILESYSTEM_8014780_H__
