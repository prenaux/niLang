// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Types.h"
#include "API/niLang/IFile.h"
#include "API/niLang/IFileSystem.h"
#include "API/niLang/Utils/UnknownImpl.h"
#include "API/niLang/Utils/FileEnum.h"
#include "API/niLang/Utils/Path.h"
#include "API/niLang/Utils/Buffer.h"
#include "API/niLang/Utils/CryptoUtils.h"
#include "FileFd.h"

#ifdef niWindows
#include <sys/utime.h>
#endif
#ifdef niPosix
#include <utime.h>
#endif

using namespace ni;

// #ifdef niEmbedded
/// This create a problem to open some files on the TC8900 board... so well disabled for now
/// re-enable only when proper testing can be done...
// Low-level file io is faster on Android (TC8900 board...), so we use it as default for Embedded systems
// #define niCore_FileStdIO_UseOpen
// #endif

#ifndef niCore_FileStdIO_UseOpen
#define niCore_FileStdIO_UseFILE // doesnt support more than 2GB file in Windows...
#endif

#ifdef niCore_FileStdIO_UseFILE
#if defined ni64 || (defined niWin32 && (_MSC_VER > 1310))
#define niCore_FileStdIO_64
#endif
#define CheckValid(X) if (mpFP == NULL) return X;
#else
#define CheckValid(X) if (mnFile == -1) return X;
#endif

#undef stdio

niExportFuncCPP(ni::cString) agetcwd() {
#ifdef niWindows
  return ni::Windows::utf8_getcwd();
#else
  char buf[niStackBufferSize] = {0};
  getcwd(buf,sizeof(buf));
  return buf;
#endif
}

#define DEVICE_SEPARATOR ':'

// Returns the canonical form of the specified filename, i.e. the minimal
// absolute filename describing the same file.
niExportFuncCPP(cString) CanonicalizeFilename(const char *filename, const tBool forceDirectory = eFalse)
{
  cString o;
  const tBool isDirectory = forceDirectory || StrEndsWith(filename,"/");
  if (!niStringIsOK(filename)) {
    return AZEROSTR;
  }

  /* if the filename starts with ~ then it's relative to a home directory */
  if (StrStartsWithHomeExpansion(filename)) {
    const char *tail = filename + StrCharWidth(StrGetNext(filename));
    o = ni::GetLang()->GetProperty("ni.dirs.home");
    if (!o.empty() && !StrIsPathSep(o.back())) {
      o.appendChar('/');
    }
    filename = tail;
  }
  /* if the filename is relative, make it absolute */
  else if (!StrIsAbsolutePath(filename) && (StrGetNext(filename) != '#')) {
    cString strCwd = agetcwd();
    if ((StrToLower(strCwd[0]) >= 'a') && (StrToLower(strCwd[0]) <= 'z') && (strCwd[1] == DEVICE_SEPARATOR)) {
      // append, skip the drive letter since its already added at the begining
      o.append(strCwd.Chars()+2);
    }
    else {
      o.append(strCwd);
    }
    if (!StrIsPathSep(o.back())) {
      o.appendChar('/');
    }
  }

#ifdef niWindows
  if (o.empty()) {
    int drive = -1;
    /* check whether we have a drive letter */
    int c1 = StrToLower(StrGetNext(filename));
    if ((c1 >= 'a') && (c1 <= 'z')) {
      int c2 = StrGetAt(filename, 1);
      if (c2 == DEVICE_SEPARATOR) {
        drive = c1 - 'a';
        filename += StrCharWidth(StrGetNext(filename));
        filename += StrCharWidth(StrGetNext(filename));
      }
    }
    /* if not, use the current drive */
    if (drive < 0)
      drive = _getdrive();
    o.appendChar(drive+'a');
    o.appendChar(DEVICE_SEPARATOR);
  }
#endif

  o.append(filename);
  StrMakeStdPath(o.data());

  /* remove duplicate slashes */
  {
    char* p = NULL;
    const char t[] = {'/','/',0};
    while ((p = (char*)StrStr(o.data(), t)) != NULL) {
      StrRemove(p, 0);
    }
    if (p) {
      o.resize(StrSize(o.data()));
    }
  }

  /* remove /./ patterns */
  {
    char* p = NULL;
    const char t[] = {'/','.','/',0};
    while ((p = (char*)StrStr(o.data(), t)) != NULL) {
      StrRemove(p, 0);
      StrRemove(p, 0);
    }
    if (p) {
      o.resize(StrSize(o.data()));
    }
  }

  /* collapse /../ patterns */
  {
    char* p = NULL;
    const char t[] = {'/','.','.','/',0};
    char* buf = o.data();
    while ((p = (char*)StrStr(buf,t)) != NULL) {
      int i;
      for (i=0; buf+StrOffset(buf, i) < p; i++)
        ;

      while (--i > 0) {
        tU32 c1 = StrGetAt(buf, i);
        if (c1 == '/')
          break;

        if (c1 == DEVICE_SEPARATOR) {
          i++;
          break;
        }
      }

      if (i < 0)
        i = 0;

      p += StrSize(t);
      memmove(buf+StrOffset(buf, i+1), p, StrSizeZ(p));
    }
    if (p) {
      o.resize(StrSize(o.data()));
    }
  }

  if (isDirectory && !StrIsPathSep(o.back())) {
    o.appendChar('/');
  }
  o.compact();
  return o;
}

///////////////////////////////////////////////
class cFileStdIO : public cIUnknownImpl<iFileBase,eIUnknownImplFlags_Default>
{
 public:
  ///////////////////////////////////////////////
  inline cFileStdIO(const achar* aaszFileName, eFileOpenMode aMode) {
    ZeroMembers();

    if (ni::StrICmp(aaszFileName,_A(":stdin:")) == 0) {
      mnFlags = eFileOpenMode_Read;
#ifdef niCore_FileStdIO_UseFILE
      mpFP = stdin;
#else
      mnFile = 0;
#endif
    }
    else if (ni::StrICmp(aaszFileName,_A(":stdout:")) == 0) {
      mnFlags = eFileOpenMode_Write;
#ifdef niCore_FileStdIO_UseFILE
      mpFP = stdout;
#else
      mnFile = 1;
#endif
    }
    else if (ni::StrICmp(aaszFileName,_A(":stderr:")) == 0) {
      mnFlags = eFileOpenMode_Write;
#ifdef niCore_FileStdIO_UseFILE
      mpFP = stderr;
#else
      mnFile = 2;
#endif
    }
    else {
      mnFlags = aMode;
#ifdef niCore_FileStdIO_UseFILE
      /////// FILE base file implementation ///////

      while (1) {
        achar aszMode[16] = {0};
        if (niFlagTest(aMode,eFileOpenMode_Append)) {
          ni::StrCat(aszMode,_A("ab"));
        }
        else if (niFlagTest(aMode,eFileOpenMode_Read) && niFlagTest(aMode,eFileOpenMode_Write)) {
          ni::StrCat(aszMode,_A("r+b")); // read and write mode (DONT ERASE existing content)
        }
        else if (niFlagTest(aMode,eFileOpenMode_Read))  {
          ni::StrCat(aszMode,_A("rb"));
        }
        else if (niFlagTest(aMode,eFileOpenMode_Write)) {
          ni::StrCat(aszMode,_A("wb"));
        }
        if (niFlagTest(aMode,eFileOpenMode_Random)) {
          ni::StrCat(aszMode,_A("R"));
        }

        mpFP = afopen(aaszFileName, aszMode, &mstrPath);
        if (mpFP) {
          // Opened the file, break out of the loop
          break;
        }
        if (niFlagTest(aMode,eFileOpenMode_Read) && niFlagTest(aMode,eFileOpenMode_Write)) {
          // Read-write mode, will fail if the file hasn't be created yet, so we're going
          // to try to create it this time, in w+ mode
          aszMode[0] = 0;
          // read and write mode (ERASE existing content)
          StrCat(aszMode,_A("w+b"));
          // Apply the random flag...
          if (niFlagTest(aMode,eFileOpenMode_Random)) {
            StrCat(aszMode,_A("R"));
          }
          aMode = (eFileOpenMode)0; // zero the mode, we won't retry after this...
          // continue, will retry opening the file with the new mode
          continue;
        }
        else {
          // File not opened and can't switch to another mode from here, return
          // niError(niFmt(_A("Can't open the file [%s] with mode [%s], errno: %d '%s'."), aaszFileName, aszMode, errno, strerror(errno)));
          return;
        }
      }

#else
      /////// _open base file implementation ///////
      int mode = _O_BINARY;
      int pmode = 0;
      if (niFlagTest(aMode,eFileOpenMode_Append)) {
        mode |= _O_CREAT|_O_APPEND;
        pmode |= _S_IWRITE;
        if (niFlagTest(aMode,eFileOpenMode_Read)) {
          mode |= _O_RDWR;
          pmode |= _S_IREAD;
        }
      }
      else if (niFlagTest(aMode,eFileOpenMode_Write)) {
        mode |= _O_CREAT;
        pmode |= _S_IWRITE;
        if (niFlagTest(aMode,eFileOpenMode_Read)) {
          mode |= _O_RDWR;
          pmode |= _S_IREAD;
        }
        else {
          mode |= _O_WRONLY;
        }
      }
      else if (niFlagTest(aMode,eFileOpenMode_Read))  {
        mode |= _O_RDONLY;
        pmode |= _S_IREAD;
      }
      if (niFlagTest(aMode,eFileOpenMode_Random)) {
        mode |= _O_RANDOM;
      }
      mnFile = FdOpen(aaszFileName,&mstrPath,mode,pmode);
      if (mnFile == -1) {
        // perror("Can't open file.");
        return;
      }
#endif
    }
  }

  ///////////////////////////////////////////////
  inline ~cFileStdIO() {
    Invalidate();
  }


  ///////////////////////////////////////////////
  inline void ZeroMembers() {
#ifdef niCore_FileStdIO_UseFILE
    mpFP = NULL;
#else
    mnFile = -1;
#endif
    mnSize = 0;
    mnFlags = 0;
  }


  ///////////////////////////////////////////////
  inline tBool  __stdcall IsOK() const {
#ifdef niCore_FileStdIO_UseFILE
    return (mpFP != NULL);
#else
    return mnFile != -1;
#endif
  }

  ///////////////////////////////////////////////
  inline void __stdcall Invalidate() {
#ifdef niCore_FileStdIO_UseFILE
    if (mpFP) {
      if (mpFP != stdin && mpFP != stdout && mpFP != stderr) {
        fclose((FILE*)mpFP);
      }
      mpFP = NULL;
    }
#else
    if (mnFile != -1) {
      FdClose(mnFile);
      mnFile = -1;
    }
#endif
  }


  ///////////////////////////////////////////////
  inline tBool  __stdcall Seek(tI64 offset) {
    CheckValid(eFalse);
#ifdef niCore_FileStdIO_UseFILE
#ifdef niCore_FileStdIO_64
    return _fseeki64(mpFP, offset, SEEK_CUR) ? eFalse : eTrue;
#else
    return fseek(mpFP, (long)offset, SEEK_CUR) ? eFalse : eTrue;
#endif
#else
    return FdSeek(mnFile,offset,SEEK_CUR) == -1LL ? eFalse : eTrue;
#endif
  }

  ///////////////////////////////////////////////
  inline tBool  __stdcall SeekSet(tI64 offset) {
    CheckValid(eFalse);
#ifdef niCore_FileStdIO_UseFILE
#ifdef niCore_FileStdIO_64
    return _fseeki64(mpFP, (long)offset, SEEK_SET) ? eFalse : eTrue;
#else
    return fseek(mpFP, (long)offset, SEEK_SET) ? eFalse : eTrue;
#endif
#else
    return FdSeek(mnFile,offset,SEEK_SET) == -1LL ? eFalse : eTrue;
#endif
  }

  ///////////////////////////////////////////////
  inline tBool    __stdcall SeekEnd(tI64 offset) {
    if (offset == 0) {
      CheckValid(eFalse);
#ifdef niCore_FileStdIO_UseFILE
#ifdef niCore_FileStdIO_64
      return _fseeki64(mpFP,0,SEEK_END) ? eFalse : eTrue;
#else
      return fseek(mpFP,0,SEEK_END) ? eFalse : eTrue;
#endif
#else
      return FdSeek(mnFile,offset,SEEK_END) == -1LL ? eFalse : eTrue;
#endif
    }
    else {
      const tI64 size = GetSize();
      if (offset < size) {
        return SeekSet(size-offset);

      }
      else {
        return SeekSet(0);
      }
    }
  }

  ///////////////////////////////////////////////
  inline tSize  __stdcall ReadRaw(void* pOut, tSize nSize) {
    CheckValid(0);
#ifdef niCore_FileStdIO_UseFILE
    return fread(pOut, 1, nSize, mpFP);
#else
    return FdRead(mnFile,pOut,nSize);
#endif
  }

  ///////////////////////////////////////////////
  inline tSize  __stdcall WriteRaw(const void* pOut, tSize nSize) {
    CheckValid(0);
#ifdef niCore_FileStdIO_UseFILE
    return fwrite(pOut, 1, nSize, mpFP);
#else
    return FdWrite(mnFile,pOut,nSize);
#endif
  }

  ///////////////////////////////////////////////
  inline tI64 __stdcall Tell() {
    CheckValid(0);
    tI64 pos;
#ifdef niCore_FileStdIO_UseFILE
#ifdef niCore_FileStdIO_64
    pos = _ftelli64(mpFP);
#else
    pos = ftell(mpFP);
#endif
#else
    pos = FdTell(mnFile);
#endif
    return pos;
  }

  ///////////////////////////////////////////////
  inline tI64 __stdcall GetSize() const {
    CheckValid(0);
    return FdSize(_fd());
  }

  ///////////////////////////////////////////////
  inline tBool __stdcall Flush() {
    CheckValid(eFalse);
    return _flush();
  }

  ///////////////////////////////////////////////
  inline const achar* __stdcall GetSourcePath() const {
    CheckValid(AZEROSTR);
    return mstrPath.Chars();
  }

  ///////////////////////////////////////////////
  inline tBool __stdcall GetTime(eFileTime aFileTime, iTime* apTime) const {
    CheckValid(eFalse);
#if defined niWinDesktop
    FILETIME  utc_time, local_time;
    SYSTEMTIME  system_time;
    BOOL    retval = FALSE;

    ni::Windows::UTF16Buffer wPath;
    niWin32_UTF8ToUTF16(wPath,mstrPath.Chars());
    HANDLE hFile = ::CreateFileW(wPath.begin(), FILE_READ_ATTRIBUTES, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
      return eFalse;

    // Get the current time in UTC format
    switch (aFileTime) {
      case eFileTime_Creation:
        retval = GetFileTime(hFile, &utc_time, NULL, NULL);
        break;
      case eFileTime_LastAccess:
        retval = GetFileTime(hFile, NULL, &utc_time, NULL);
        break;
      case eFileTime_LastWrite:
        retval = GetFileTime(hFile, NULL, NULL, &utc_time);
        break;
    }

    CloseHandle(hFile);

    // Failed?
    if (retval == FALSE)
      return eFalse;

    // Convert from UTC to local time (daylight savings and things)
    if (!FileTimeToLocalFileTime(&utc_time, &local_time))
      return eFalse;

    // Now convert into the needed system time
    FileTimeToSystemTime(&local_time, &system_time);

    // Transfer these into the time class
    apTime->SetYear(system_time.wYear);
    apTime->SetMonth((tU8)system_time.wMonth);
    apTime->SetDay((tU8)system_time.wDay);
    apTime->SetHour((tU8)system_time.wHour);
    apTime->SetMinute((tU8)system_time.wMinute);
    apTime->SetSecond((tU8)system_time.wSecond);
    return eTrue;
#else
    time_t atime,mtime,ctime;
    if (!unix_filetime(mstrPath.Chars(),&atime,&mtime,&ctime))
      return eFalse;
    switch (aFileTime) {
      case eFileTime_Creation:
        SetTimeFromTimeT(apTime, (void*)&ctime);
        break;
      case eFileTime_LastAccess:
        SetTimeFromTimeT(apTime, (void*)&atime);
        break;
      case eFileTime_LastWrite:
        SetTimeFromTimeT(apTime, (void*)&mtime);
        break;
    }
    return eTrue;
#endif
  }

  ///////////////////////////////////////////////
  inline tBool __stdcall SetTime(eFileTime aFileTime, const iTime* apTime) {
    CheckValid(eFalse);
#if defined niWinDesktop
    if (aFileTime == eFileTime_Creation)
      return eFalse;

    struct stat path_stat;
    if (::stat(mstrPath.c_str(), &path_stat) != 0)
      return eFalse;

    const time_t newTime = (time_t)apTime->GetUnixTimeSecs();
    __utimbuf64 buf;
    if (aFileTime == eFileTime_LastAccess) {
      buf.actime = newTime;
      buf.modtime = path_stat.st_mtime;
    }
    else if (aFileTime == eFileTime_LastWrite) {
      buf.actime = path_stat.st_atime;
      buf.modtime = newTime;
    }
    else {
      return eFalse;
    }

    ni::Windows::UTF16Buffer wPath;
    niWin32_UTF8ToUTF16(wPath,mstrPath.Chars());
    if (_wutime64(wPath.begin(), &buf) != 0)
      return eFalse;

    return eTrue;
#elif defined niPosix
    if (aFileTime == eFileTime_Creation)
      return eFalse;

    struct stat path_stat;
    if (::stat(mstrPath.c_str(), &path_stat) != 0)
      return eFalse;

    const time_t newTime = (time_t)apTime->GetUnixTimeSecs();
    ::utimbuf buf;
    if (aFileTime == eFileTime_LastAccess) {
      buf.actime = newTime;
      buf.modtime = path_stat.st_mtime;
    }
    else if (aFileTime == eFileTime_LastWrite) {
      buf.actime = path_stat.st_atime;
      buf.modtime = newTime;
    }
    else {
      return eFalse;
    }

    if (utime(mstrPath.c_str(), &buf) != 0)
      return eFalse;

    return eTrue;
#else
#pragma niTodo("IMPLEMENT cFileStdIO::SetTime.")
    return eFalse;
#endif
  }

  ///////////////////////////////////////////////
  inline tFileFlags __stdcall GetFileFlags() const {
    return mnFlags;
  }

  ///////////////////////////////////////////////
  inline tBool  __stdcall Resize(tI64 newSize) {
    int fd = _fd();
#if defined niWin32
    if (_chsize(fd, (long)newSize) != -1) {
      return eTrue;
    }
    else {
      return eFalse;
    }
#else
    if (!ftruncate(fd, (off_t)newSize)) {
      return eTrue;
    }
    else {
      return eFalse;
    }
#endif
  }

 private:
  inline tBool _flush() const {
#ifdef niCore_FileStdIO_UseFILE
    fflush((FILE*)mpFP);
    return eTrue;
#else
    FdFlush(_fd());
    return eTrue;
#endif
  }
  inline int _fd() const {
#ifdef niCore_FileStdIO_UseFILE
    return mpFP ? _fileno(mpFP) : -1;
#else
    return mnFile;
#endif
  }

#ifdef niCore_FileStdIO_UseFILE
  FILE*  mpFP;
#else
  int      mnFile;
#endif
  tI64   mnSize;
  tU32   mnFlags;
  cString  mstrPath;
};

//--------------------------------------------------------------------------------------------
//
//
//
//--------------------------------------------------------------------------------------------
//! File system enumerator implementation
class cFileSystemEnumerator : public ni::cIUnknownImpl<ni::iFileSystemEnumerator,ni::eIUnknownImplFlags_Default>
{
  niBeginClass(cFileSystemEnumerator);

 public:
  ///////////////////////////////////////////////
  cFileSystemEnumerator(iFileSystem* apFS) {
    ZeroMembers();
    mptrFS = apFS;
  }

  ///////////////////////////////////////////////
  ~cFileSystemEnumerator() {
    Invalidate();
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
  }

  ///////////////////////////////////////////////
  void __stdcall ZeroMembers() {
  }

  ///////////////////////////////////////////////
  ni::tBool __stdcall IsOK() const {
    niClassIsOK(cFileSystemEnumerator);
    return ni::eTrue;
  }

  ///////////////////////////////////////////////
  virtual iFileSystem* __stdcall GetFileSystem() const {
    return mptrFS;
  }
  ///////////////////////////////////////////////
  virtual tBool __stdcall FindFirst(const achar* aaszPattern) {
    if (!niStringIsOK(aaszPattern)) return eFalse;
    return mFF.First(aaszPattern);
  }
  ///////////////////////////////////////////////
  virtual tBool __stdcall FindNext() {
    return mFF.Next();
  }
  ///////////////////////////////////////////////
  virtual const achar* __stdcall GetFileName() const {
    return mFF.FileName();
  }
  ///////////////////////////////////////////////
  virtual tI64 __stdcall GetFileSize() const {
    return mFF.FileSize();
  }
  ///////////////////////////////////////////////
  virtual tFileAttrFlags __stdcall GetFileAttributes() const {
    return mFF.FileAttribs();
  }
  ///////////////////////////////////////////////
  virtual iTime* __stdcall GetFileTime() const {
    if (!mptrTime.IsOK()) {
      niThis(cFileSystemEnumerator)->mptrTime = ni::CreateTimeZero();
    }
    niThis(cFileSystemEnumerator)->mptrTime->SetUnixTimeSecs(mFF.FileTime());
    return mptrTime;
  }

 private:
  Ptr<iFileSystem>  mptrFS;
  cString         mstrPattern;
  Ptr<iTime>    mptrTime;
  ni::FindFile      mFF;

  niEndClass(cFileSystemEnumerator);
};

//--------------------------------------------------------------------------------------------
//
// FileEnum
//
//--------------------------------------------------------------------------------------------
namespace ni {

///////////////////////////////////////////////
static tU32 _FileEnumCountItemFileInfo(const sFileInfo& fi, iRegex* regex, tU32& count, iFileEnumSink* apSink, astl::vector<sFileInfo>* apFiles, tU32 anMax) {
  if ((fi.attribs&eFileAttrFlags_Directory)
      || !niIsOK(regex)
      || regex->DoesMatch(fi.name.Chars()))
  {
    if (apFiles) {
      apFiles->push_back(fi);
    }
    ++count;
    if (niIsOK(apSink) && !apSink->OnFound(fi.name.Chars(),fi.attribs,fi.size)) {
      return count;
    }
    if (anMax && count >= anMax) {
      return count;
    }
  }
  return eInvalidHandle;
}

///////////////////////////////////////////////
static tU32 _FileEnumCountItem(const ni::FindFile& aFF, iRegex* regex, tU32& count, iFileEnumSink* apSink, astl::vector<sFileInfo>* apFiles, tU32 anMax) {
  sFileInfo fi;
  fi.count = count;
  fi.name = aFF.FileName();
  fi.time = aFF.FileTime();
  fi.attribs = aFF.FileAttribs();
  fi.size = aFF.FileSize();
  return _FileEnumCountItemFileInfo(fi,regex,count,apSink,apFiles,anMax);
}

///////////////////////////////////////////////
niExportFunc(tU32) FileEnum(const achar* aszFile, tU32 flAttribs, iFileEnumSink* apSink, astl::vector<sFileInfo>* apFiles, tU32 anMax)
{
  if (!niIsStringOK(aszFile)) {
    return 0;
  }

  tU32 count = 0;
  FindFile ff;
  Ptr<iFileEnumSink> pSink = apSink;

  tU32 r;
  cString strFile = aszFile;
  cString strRegex;
  tI32 regexPos = strFile.find(_A("|"));
  Ptr<ni::iRegex> regex;
  if (regexPos != cString::npos) {
    strRegex = strFile.substr(regexPos+1);
    strFile = strFile.substr(0,regexPos);
    regex = ni::CreateFilePatternRegex(strRegex.Chars(),_A("|"));
    if (!regex.IsOK()) return eInvalidHandle;
  }

  if (StrEq(strFile.Chars(),_A("//")) ||
      StrEq(strFile.Chars(),_A("//*"))  ||
      StrEq(strFile.Chars(),_A("//*.*")))
  {
#if defined niWinDesktop
    if (niFlagTest(flAttribs,eFileAttrFlags_Directory))
    {
      achar curDrive[4] = { 'A', ':', '\\', 0 };
      niLoop(i,26) {
        curDrive[0] = (achar)('A'+i);
        UINT driveInfo = GetDriveType(curDrive);
        if (driveInfo != DRIVE_UNKNOWN &&
            driveInfo != DRIVE_NO_ROOT_DIR)
        {
          sFileInfo fi;
          fi.count = count;
          fi.name = niFmt(_A("%c:"),i+65);
          fi.attribs = eFileAttrFlags_Directory;
          switch (driveInfo) {
            case DRIVE_REMOVABLE: fi.attribs |= (curDrive[0]==_A('A')||curDrive[0]==_A('B')) ? eFileAttrFlags_DeviceFloppy : eFileAttrFlags_DeviceRemovable; break;
            case DRIVE_REMOTE:    fi.attribs |= eFileAttrFlags_DeviceRemote;
            case DRIVE_CDROM:   fi.attribs |= eFileAttrFlags_DeviceCDRom;
            case DRIVE_RAMDISK:   fi.attribs |= eFileAttrFlags_DeviceRAM;
            case DRIVE_FIXED:   fi.attribs |= eFileAttrFlags_DeviceFixed;
          }
          r = _FileEnumCountItemFileInfo(fi,regex,count,pSink,apFiles,anMax);
          if (r != eInvalidHandle) return r;
        }
      }
    }
    return count;
#else
    strFile = strFile.Chars()+1;
#endif
  }
  // Is it a Windows drive path `//X:` ?
  else if (StrNICmp(strFile.Chars(),_A("//"),2) == 0 &&
           (strFile.Len() >= 4 && strFile[3] == _A(':')))
  {
    // Make this a valid root path
    strFile = strFile.substr(2);
  }
  // Is it a absolute root path `//Something` ?
  else if (StrNICmp(strFile.Chars(),_A("//"),2) == 0)
  {
    // Make this a valid root path
    strFile = strFile.substr(1);
  }

  // Is it a drive only pattern ? --- this is necessary because Windows wont
  // enumerate anything with only a drive name, if it's not the current drive
  if ((strFile.Len() == 2 && strFile[1] == _A(':')) ||
      (strFile.Len() == 3 && strFile[1] == _A(':') && StrIsPathSep(strFile[2])))
  {
    achar tmp[5] = { strFile[0], _A(':'), _A('/'), _A('*'), '\0' };
    strFile = tmp;
  }

  cString strSearch;
  cPath path(strFile.Chars());
  if (path.IsDirectory()) {
    strSearch = path.GetDirectory();
    strSearch.Remove(); // remove the end path separatos
  }
  else {
    strSearch = path.GetPath();
  }

  if (!ff.First(strSearch.Chars()))
    return 0;

  do {
    if (ff.FileAttribs()&flAttribs) {
      if (ni::StrCmp(ff.FileName(),_A("..")) != 0 && ni::StrCmp(ff.FileName(),_A(".")) != 0) {
        r = _FileEnumCountItem(ff,regex,count,pSink,apFiles,anMax);
        if (r != eInvalidHandle) return r;
      }
    }
  } while (ff.Next());

  return count;
}

}

//--------------------------------------------------------------------------------------------
//
//
//
//--------------------------------------------------------------------------------------------

#define FILESYSTEM_REBASE_PATH(NAME)            \
  cString __rebased_##NAME;                     \
  __rebased_##NAME << this->mstrBase;           \
  __rebased_##NAME << _##NAME;                  \
  const char* NAME = __rebased_##NAME.Chars();

//! File system directory implementation
template <eIUnknownImplFlags FLAGS = eIUnknownImplFlags_Default>
class cFileSystemDir : public cIUnknownImpl<ni::iFileSystem,FLAGS>
{
  niBeginClass(cFileSystemDir);
 public:
  ///////////////////////////////////////////////
  cFileSystemDir(const achar* aaszDir, tFileSystemRightsFlags aRights)
  {
    mRights  = aRights;
    if (niStringIsOK(aaszDir)) {
      cPath path;
      path.SetDirectory(aaszDir);
      mstrBase = path.GetPath();
    }
  }

  ///////////////////////////////////////////////
  ~cFileSystemDir() {
    Invalidate();
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
    if (mRights) {
      mRights = 0;
    }
  }

  ///////////////////////////////////////////////
  ni::tBool __stdcall IsOK() const {
    return ni::eTrue;
  }

  ///////////////////////////////////////////////
  virtual tFileSystemRightsFlags __stdcall GetRightsFlags() const {
    return mRights;
  }

  ///////////////////////////////////////////////
  virtual const achar* __stdcall GetBaseContainer() const {
    return mstrBase.Chars();
  }

  ///////////////////////////////////////////////
  inline tBool __stdcall _CheckPathRights(const achar* aaszPath) {
    if (!mstrBase.empty()) {
      // dont authorize a constrained directory FS to use ".."
      if (StrZContains(aaszPath, 0, "..", 0, 0))
        return eFalse;
    }
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall FileMakeDir(const achar* _aszDir)
  {
    niCheck(_CheckPathRights(_aszDir),eFalse);
    niCheck(niFlagIs(mRights,eFileSystemRightsFlags_Create),eFalse);

    FILESYSTEM_REBASE_PATH(aszDir);

    cPath curPath;
    cString strDir;
    while (1)
    {
      const tU32 chDir = StrGetNext(aszDir);
      if (*aszDir == '\\')
        strDir.appendChar('/');
      else
        strDir.appendChar(chDir);
      if ((!chDir || chDir == '/' || chDir == '\\') && strDir.IsNotEmpty()) {
        curPath.AddDirectoryBack(strDir.Chars());
        if (ni::StrCmp(curPath.GetPath().Chars(),_A("/")) != 0 &&
            ni::StrCmp(curPath.GetPath().Chars(),_A("./")) != 0 &&
            !curPath.GetPath().EndsWith(_A(":/")) &&
            FileExists(curPath.GetPath().Chars(),eFileAttrFlags_Directory) == 0)
        {
          bool failed;
          failed = amkdir(curPath.GetPath().Chars()) != 0;
          if (failed) {
            if (errno != EEXIST)
              return eFalse;
          }
        }
        strDir.Clear();
      }
      if (!chDir)
        return eTrue;
      StrGetNextX(&aszDir);
    }

    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall FileDeleteDir(const achar* _aszDir)
  {
    niCheck(_CheckPathRights(_aszDir),eFalse);
    niCheck(niFlagIs(mRights,eFileSystemRightsFlags_Delete),eFalse);

    FILESYSTEM_REBASE_PATH(aszDir);
    return (armdir(aszDir) == 0);
  }

  ///////////////////////////////////////////////
  tBool __stdcall FileCopy(const achar* _aszDest, const achar* _aszSrc)
  {
    niCheck(_CheckPathRights(_aszDest),eFalse);
    niCheck(_CheckPathRights(_aszSrc),eFalse);
    niCheck(niFlagIs(mRights,eFileSystemRightsFlags_Read),eFalse);
    niCheck(niFlagIs(mRights,eFileSystemRightsFlags_Write),eFalse);
    niCheck(niFlagIs(mRights,eFileSystemRightsFlags_Create),eFalse);

    FILE *src, *dst;

    FILESYSTEM_REBASE_PATH(aszSrc);
    src = afopen(aszSrc, _A("rb"), NULL);
    if (!src) {
      niError(niFmt(_A("Can't open the source file [%s]"), aszSrc));
      return eFalse;
    }

    FILESYSTEM_REBASE_PATH(aszDest);
    dst = afopen(aszDest, _A("wb"), NULL);
    if (!dst) {
      niError(niFmt(_A("Can't open the destination file [%s]"), aszDest));
      fclose(src);
      return eFalse;
    }

    char buf[4096];
    while (1) {
      size_t r = fread(buf,1,sizeof(buf),src);
      if (r > 0) {
        fwrite(buf,r,1,dst);
      }
      if (feof(src))
        break;
    }

    fclose(src);
    fclose(dst);
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall FileMove(const achar* _aszDest, const achar* _aszSrc)
  {
    niCheck(_CheckPathRights(_aszDest),eFalse);
    niCheck(_CheckPathRights(_aszSrc),eFalse);
    niCheck(niFlagIs(mRights,eFileSystemRightsFlags_Read),eFalse);
    niCheck(niFlagIs(mRights,eFileSystemRightsFlags_Write),eFalse);
    niCheck(niFlagIs(mRights,eFileSystemRightsFlags_Create),eFalse);
    niCheck(niFlagIs(mRights,eFileSystemRightsFlags_Delete),eFalse);
    FILESYSTEM_REBASE_PATH(aszSrc);
    FILESYSTEM_REBASE_PATH(aszDest);
    return ::rename(aszSrc, aszDest) == 0;
  }

  ///////////////////////////////////////////////
  tBool __stdcall FileDelete(const achar* _aszFile)
  {
    niCheck(_CheckPathRights(_aszFile),eFalse);
    niCheck(niFlagIs(mRights,eFileSystemRightsFlags_Delete),eFalse);

    FILESYSTEM_REBASE_PATH(aszFile);
    cString path = aszFile;
#ifdef niWindows
    achar* p = (achar*)path.Chars();
    while (*p) {
      if (*p == '/')
        *p = '\\';
      ++p;
    }
    ni::Windows::UTF16Buffer wPath; niWin32_UTF8ToUTF16(wPath,path.Chars());
    return (::DeleteFileW(wPath.begin())?eTrue:eFalse);
#else
    return (aunlink(path.Chars()) == 0);
#endif
  }

  ///////////////////////////////////////////////
  tU32 __stdcall FileEnum(const achar* _aszFile, tU32 flAttribs, iFileEnumSink* apSink)
  {
    niCheck(_CheckPathRights(_aszFile),eInvalidHandle);
    niCheck(niFlagIs(mRights,eFileSystemRightsFlags_Enum),eInvalidHandle);
    FILESYSTEM_REBASE_PATH(aszFile);
    return ni::FileEnum(aszFile,flAttribs,apSink,NULL,0);
  }

  ///////////////////////////////////////////////
  tU32 __stdcall FileExists(const achar* _aszFile, tU32 flAttribs)
  {
    niCheck(_CheckPathRights(_aszFile),eFalse);
    niCheck(niFlagIs(mRights,eFileSystemRightsFlags_Read) |
            niFlagIs(mRights,eFileSystemRightsFlags_Write) |
            niFlagIs(mRights,eFileSystemRightsFlags_Enum),eFalse);

    FILESYSTEM_REBASE_PATH(aszFile);
#ifdef niWindows
    // This is necessary on Windows because it doesn't handle
    // directories as if they were a file so FdOpen just doesn't work on
    // directories
    if (flAttribs&eFileAttrFlags_Directory) {
      astl::vector<sFileInfo> infos;
      tU32 r = ni::FileEnum(aszFile,flAttribs,NULL,&infos,1);
      if (r == 1) {
        return infos[0].attribs;
      }
      else {
        return 0;
      }
    }
    else
#endif
    {
      tU32 attr = FdPathAttrs(aszFile);
      if ((flAttribs&eFileAttrFlags_Directory) && (attr&eFileAttrFlags_Directory))
        return attr;
      if ((flAttribs&eFileAttrFlags_File) && (attr&eFileAttrFlags_File))
        return attr;
      return 0;
    }
  }

  ///////////////////////////////////////////////
  tI64 __stdcall FileSize(const achar* _aszFile)
  {
    niCheck(_CheckPathRights(_aszFile),eFalse);
    niCheck(niFlagIs(mRights,eFileSystemRightsFlags_Enum),eFalse);
    FILESYSTEM_REBASE_PATH(aszFile);
    return FdPathSize(aszFile);
  }

  ///////////////////////////////////////////////
  iFileBase* __stdcall FileBaseOpen(const achar* _aszFile, eFileOpenMode aMode)
  {
    niCheck(_CheckPathRights(_aszFile),NULL);

    // check rights
    if (niFlagIs(aMode,eFileOpenMode_Read)) {
      niCheck(niFlagIs(mRights,eFileSystemRightsFlags_Read),NULL);
    }

    FILESYSTEM_REBASE_PATH(aszFile);

    if (niFlagIs(aMode,eFileOpenMode_Write) || niFlagIs(aMode,eFileOpenMode_Append)) {
      niCheck(niFlagIs(mRights,eFileSystemRightsFlags_Write),NULL);
      if (!niFlagIs(mRights,eFileSystemRightsFlags_Create)) {
        // no create right so the file must already exists, this is
        // arguable... but that's how it is :)
        if (!FileExists(aszFile,eFileAttrFlags_AllFiles)) {
          niError(niFmt(_A("No create rights assigned to write file '%s' which doesnt already exist."),aszFile));
          return NULL;
        }
      }
    }

    cPath path(aszFile);
    Ptr<iFileBase> fpBase = niNew cFileStdIO(path.GetPath().Chars(), aMode);
    if (!niIsOK(fpBase)) {
      return NULL;
    }

    return fpBase.GetRawAndSetNull();
  }
  iFile* __stdcall FileOpen(const achar* aszFile, eFileOpenMode aMode)
  {
    Ptr<iFileBase> fpBase = FileBaseOpen(aszFile,aMode);
    if (!niIsOK(fpBase)) return NULL;
    return ni::CreateFile(fpBase);
  }

  ///////////////////////////////////////////////
  virtual cString __stdcall GetAbsolutePath(const achar* _aszFile) const {
    FILESYSTEM_REBASE_PATH(aszFile);
    const tBool isDirectory = StrEndsWith(_aszFile,"/");

#if defined niWinDesktop
    if (StrStartsWithHomeExpansion(aszFile)) {
      return CanonicalizeFilename(aszFile,isDirectory);
    }
    else {
      WCHAR buffer[AMAX_PATH];
      WCHAR* bufferFile;
      ni::Windows::UTF16Buffer wFile;
      niWin32_UTF8ToUTF16(wFile,aszFile);
      if (::GetFullPathNameW(wFile.begin(),AMAX_PATH,buffer,&bufferFile)) {
        ni::Windows::UTF8Buffer bufferUTF8;
        niWin32_UTF16ToUTF8(bufferUTF8,buffer);
        return CanonicalizeFilename(bufferUTF8.begin(),isDirectory);
      }
      return AZEROSTR;
    }
#elif defined niPosix
    cString r;
    char* resolvedPath = realpath(aszFile,NULL);
    if (resolvedPath) {
      r = resolvedPath;
      // If resolved_path is specified as NULL, then realpath() uses malloc(3)
      // to allocate a buffer of up to PATH_MAX bytes to hold the resolved
      // pathname, and returns a pointer to this buffer. The caller should
      // deallocate this buffer using free(3).buffer using free(3).
      free(resolvedPath);
    }
    else {
      r = aszFile;
    }
    return CanonicalizeFilename(r.Chars(),isDirectory);
#else
    #error "GetAbsolutePath: Unknown platform."
#endif
  }

  ///////////////////////////////////////////////
  virtual iFileSystemEnumerator* __stdcall CreateEnumerator() {
    return niNew cFileSystemEnumerator(this);
  }

 private:
  cString         mstrBase;
  tFileSystemRightsFlags  mRights;
  niEndClass(cFileSystemDir);
};

//--------------------------------------------------------------------------------------------
//
//
//
//--------------------------------------------------------------------------------------------
niConstValue achar* const kFileSystemHashedAlgo = "SHA1";

struct FileSystemHashed : public cIUnknownImpl<ni::iFileSystem>
{
  ///////////////////////////////////////////////
  FileSystemHashed(const iFileSystem* apFS)
      : _fs(apFS) {}

  const Ptr<iFileSystem> _fs;

  // !!! This must be thread safe
  cString _ComputeFilePath(const achar* aFile, const tBool createCacheSubDir) const {
    const tI32 strLen = StrLen(aFile);
    const tI32 extPos = StrZRFindChar(aFile, 0, '.');
    // const tI32 extLen = strLen - extPos;
    cString hash = HashToString(HashString(CreateHash(kFileSystemHashedAlgo), aFile, extPos));
    niAssert(hash.size() >= 4);
    cString path;
    path.reserve(6 + hash.size() + 1);
    const achar cacheSubDir[7] = { hash[0], hash[1], '/', hash[2], hash[3], '/', 0 };
    path.append(cacheSubDir);
    path.append(hash.Chars());
    // Preserves the extension, this is to keep compatibility with functions
    // that use the file's extension to do different kind of
    // processing/handling.
    if (extPos >= 0) {
      path.append(aFile + extPos);
    }
    if (createCacheSubDir) {
      _fs->FileMakeDir(cacheSubDir);
    }
    return path;
  }

  virtual tFileSystemRightsFlags __stdcall GetRightsFlags() const niImpl {
    return _fs->GetRightsFlags() & ~(eFileSystemRightsFlags_Enum);
  }

  virtual const achar* __stdcall GetBaseContainer() const niImpl {
    return _fs->GetBaseContainer();
  }

  virtual tBool __stdcall FileMakeDir(const achar* aDir) niImpl {
    niError("Not supported.");
    return eFalse;
  }

  virtual tBool __stdcall FileDeleteDir(const achar* aDir) niImpl {
    niError("Not supported.");
    return eFalse;
  }

  virtual tBool __stdcall FileCopy(const achar* aDest, const achar* aSrc) niImpl {
    niCheck(niFlagIs(GetRightsFlags(),eFileSystemRightsFlags_Read),eFalse);
    niCheck(niFlagIs(GetRightsFlags(),eFileSystemRightsFlags_Write),eFalse);
    niCheck(niFlagIs(GetRightsFlags(),eFileSystemRightsFlags_Create),eFalse);

    const cString srcPath = _ComputeFilePath(aSrc,eFalse);
    if (!_fs->FileExists(srcPath.Chars(), eFileAttrFlags_File)) {
      niError(niFmt(_A("Source file '%s' doesn't exist."), aSrc));
      return eFalse;
    }

    const cString dstPath = _ComputeFilePath(aDest,eTrue);
    if (!_fs->FileCopy(dstPath.Chars(), srcPath.Chars())) {
      niError(niFmt(_A("Copying file '%s' to '%s' failed."), aSrc, aDest));
      return eFalse;
    }

    return eFalse;
  }

  virtual tBool __stdcall FileMove(const achar* aDest, const achar* aSrc) niImpl {
    niCheck(niFlagIs(GetRightsFlags(),eFileSystemRightsFlags_Read),eFalse);
    niCheck(niFlagIs(GetRightsFlags(),eFileSystemRightsFlags_Write),eFalse);
    niCheck(niFlagIs(GetRightsFlags(),eFileSystemRightsFlags_Create),eFalse);
    niCheck(niFlagIs(GetRightsFlags(),eFileSystemRightsFlags_Delete),eFalse);

    const cString srcPath = _ComputeFilePath(aSrc,eFalse);
    if (!_fs->FileExists(srcPath.Chars(), eFileAttrFlags_File)) {
      niError(niFmt(_A("Source file '%s' doesn't exist."), aSrc));
      return eFalse;
    }

    const cString dstPath = _ComputeFilePath(aDest,eTrue);
    if (!_fs->FileMove(dstPath.Chars(), srcPath.Chars())) {
      niError(niFmt(_A("Moving file '%s' to '%s' failed."), aSrc, aDest));
      return eFalse;
    }

    return eFalse;
  }

  virtual tBool __stdcall FileDelete(const achar* aFile) niImpl {
    niCheck(niFlagIs(GetRightsFlags(),eFileSystemRightsFlags_Delete),eFalse);
    const cString filePath = _ComputeFilePath(aFile,eFalse);
    return _fs->FileDelete(filePath.Chars());
  }

  virtual tU32  __stdcall FileEnum(const achar* aFile, tU32 aAttribs, iFileEnumSink* aSink) niImpl {
    return 0;
  }

  virtual tU32  __stdcall FileExists(const achar* aFile, tU32 aAttribs) niImpl {
    const cString filePath = _ComputeFilePath(aFile,eFalse);
    return _fs->FileExists(filePath.Chars(), aAttribs);
  }

  virtual tI64  __stdcall FileSize(const achar* aFile) niImpl {
    const cString filePath = _ComputeFilePath(aFile,eFalse);
    return _fs->FileSize(filePath.Chars());
  }

  virtual iFile*  __stdcall FileOpen(const achar* aFile, eFileOpenMode aMode) niImpl {
    Ptr<iFileBase> fpBase = FileBaseOpen(aFile,aMode);
    if (!niIsOK(fpBase)) return NULL;
    return ni::CreateFile(fpBase);
  }

  virtual iFileBase*  __stdcall FileBaseOpen(const achar* aFile, eFileOpenMode aMode) niImpl {
    if (niFlagIs(aMode,eFileOpenMode_Write)) {
      niCheck(niFlagIs(GetRightsFlags(),eFileSystemRightsFlags_Write),NULL);
    }

    const cString filePath = _ComputeFilePath(aFile,niFlagIs(aMode,eFileOpenMode_Write));
    return _fs->FileBaseOpen(filePath.Chars(), aMode);
  }

  virtual cString __stdcall GetAbsolutePath(const achar* aFile) const niImpl {
    const cString filePath = _ComputeFilePath(aFile,eFalse);
    return _fs->GetAbsolutePath(filePath.Chars());
  }

  virtual iFileSystemEnumerator* __stdcall CreateEnumerator() niImpl {
    return NULL;
  }
};

namespace ni {

niExportFunc(iFileSystem*) GetRootFS() {
  static cFileSystemDir<eIUnknownImplFlags_NoRefCount> _rootFS(
      NULL,eFileSystemRightsFlags_All);
  return &_rootFS;
}

niExportFunc(iFileSystem*) CreateFileSystemDir(const achar* aaszDir, tFileSystemRightsFlags aRights)
{
  if (!GetRootFS()->FileExists(aaszDir,eFileAttrFlags_Directory)) {
    niError(niFmt(_A("Directory '%s' doesn't exists."),aaszDir));
    return NULL;
  }
  if (!aRights) {
    niError(niFmt(_A("No rights specified.")));
    return NULL;
  }
  return niNew cFileSystemDir<eIUnknownImplFlags_Default>(aaszDir,aRights);
}


niExportFuncCPP(iFileSystem*) CreateFileSystemHashed(const iFileSystem* apFS) {
  niCheckIsOK(apFS, NULL);
  return niNew FileSystemHashed(apFS);
}

}
