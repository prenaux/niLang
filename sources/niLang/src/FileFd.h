#ifndef __FILEFD_H_690EA275_CAAB_41E2_98E8_42C0078B429A__
#define __FILEFD_H_690EA275_CAAB_41E2_98E8_42C0078B429A__

#include "API/niLang/Types.h"
#include "API/niLang/IFile.h"
#include "API/niLang/StringBase.h"

#if defined niWin32

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include "API/niLang/Platforms/Win32/Win32_UTF.h"
#include "API/niLang/Utils/Buffer.h"

#elif defined niUnix

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#define _fileno fileno
#ifndef _fseeki64
#define _fseeki64 fseek
#define _ftelli64 ftell
#define _lseeki64 lseek
#define _telli64  tell
#endif

#endif

#ifndef _A_RDONLY
#ifdef FA_RDONLY
#define _A_RDONLY       FA_RDONLY
#define _A_HIDDEN       FA_HIDDEN
#define _A_SYSTEM       FA_SYSTEM
#define _A_LABEL        FA_LABEL
#define _A_SUBDIR       FA_DIRECT
#define _A_ARCH         FA_ARCH
#else
#define _A_RDONLY       1
#define _A_HIDDEN       2
#define _A_SYSTEM       4
#define _A_LABEL        8
#define _A_SUBDIR       16
#define _A_ARCH         32
#endif
#endif

niExportFunc(FILE*) afopen(const char* file, const char* mode, ni::cString* apPathOnDisk);
niExportFunc(int)   amkdir(const char* dir);
niExportFunc(int)   armdir(const char* dir);
niExportFunc(int)   aunlink(const char* file);

niExportFuncCPP(ni::cString) agetenv(const char* env);
niExportFunc(int)            aputenv(const char* envString);

niExportFuncCPP(ni::cString) agetcwd();

namespace ni {

#ifndef _O_BINARY
#define _O_BINARY 0
#define _O_CREAT  O_CREAT
#define _O_APPEND O_APPEND
#define _O_RDWR   O_RDWR
#define _O_RANDOM 0
#define _O_WRONLY O_WRONLY
#define _O_RDONLY O_RDONLY
#endif

#ifndef _S_IWRITE
#define _S_IWRITE 0
#define _S_IREAD  0
#endif

struct sFileStats {
  tI64           size;
  tFileAttrFlags attr;
};

niExportFunc(int) FdOpen(const achar* path, ni::cString* apPathOnDisk, int mode = O_RDONLY, int pmode = 0);

inline int FdClose(int fd) {
  if (fd < 0)
    return -1;
#ifdef niWin32
  return _close(fd);
#else
  return close(fd);
#endif
}

inline tBool FdStats(int fd, sFileStats* s) {
  if (fd < 0)
    return eFalse;
#if defined niUnix && ((defined ni32 || defined niOSX) || (defined ni64 && defined niIOS))
  struct stat st;
  if (fstat(fd, &st)) {
    return eFalse;
  }
#elif defined niUnix && defined ni64
  struct stat64 st;
  if (fstat64(fd, &st)) {
    return eFalse;
  }
#elif defined niCore_FileStdIO_64
  struct __stat64 st;
  if (_fstat64(fd, &st)) {
    return eFalse;
  }
#else
  struct _stat st;
  if (_fstat(fd, &st)) {
    return eFalse;
  }
#endif
  if (s) {
    s->size = st.st_size;
    if (st.st_mode & S_IFDIR) {
      s->attr = eFileAttrFlags_Directory;
    }
    else {
      s->attr = eFileAttrFlags_File;
    }
  }
  return eTrue;
}

inline tU32 FdAttrs(int fd) {
  if (fd < 0)
    return 0;
  sFileStats stats;
  if (!FdStats(fd,&stats))
    return 0;
  return stats.attr;
}

inline tI64 FdSize(int fd) {
  if (fd < 0)
    return 0;
  sFileStats stats;
  if (!FdStats(fd,&stats) || !(stats.attr&eFileAttrFlags_File))
    return 0;
  return stats.size;
}

inline tU32 FdPathAttrs(const achar* path, ni::cString* apPathOnDisk = NULL) {
  int fd = FdOpen(path,apPathOnDisk);
  if (fd < 0) {
#ifdef niWin32
    if (ni::Windows::utf8_access(path,0) == 0)
      return eFileAttrFlags_Directory;
#endif
    return 0;
  }
  tU32 r = FdAttrs(fd);
  FdClose(fd);
  return r;
}

inline tI64 FdPathSize(const achar* path, ni::cString* apPathOnDisk = NULL) {
  int fd = FdOpen(path,apPathOnDisk);
  if (fd < 0)
    return 0;
  tI64 r = FdSize(fd);
  FdClose(fd);
  return r;
}

inline tI64 FdWrite(int fd, const void* data, unsigned int size) {
  if (fd < 0)
    return 0;
#ifdef niWin32
  return _write(fd,data,size);
#else
  return write(fd,data,size);
#endif
}
inline tI64 FdRead(int fd,  void* data, unsigned int size) {
  if (fd < 0)
    return 0;
#ifdef niWin32
  return _read(fd,data,size);
#else
  return read(fd,data,size);
#endif
}
inline tI64 FdSeek(int fd,  tI64 off, int mode) {
  if (fd < 0)
    return -1;
  return _lseeki64(fd,off,mode);
}
inline tI64 FdTell(int fd) {
  if (fd < 0)
    return -1;
  return FdSeek(fd,0,SEEK_CUR);
}
inline void FdFlush(int fd) {
  if (fd < 0) return;
#ifdef niWin32
  _commit(fd);
#else
  // no equivalent on POSIX systems ?
#endif
}


enum eFileFdFlags
{
  eFileFdFlags_DontOwnRead = niBit(0),
  eFileFdFlags_DontOwnWrite = niBit(1),
  eFileFdFlags_DontOwnRW = niBit(0)|niBit(1),
  eFileFdFlags_NoSeek = niBit(2),
  eFileFdFlags_Pipe = niBit(3)|eFileFdFlags_NoSeek,
  //! \internal
  eFileFdFlags_ForceDWORD = 0xFFFFFFFF
};

//! file fd flags type. \see ni::eFileFdFlags
typedef tU32 tFileFdFlags;

niExportFunc(ni::iFileBase*) CreateFileFd(int anRead, int anWrite, tFileFdFlags aFileFlags, const char* aaszFileName);

}
#endif // __FILEFD_H_690EA275_CAAB_41E2_98E8_42C0078B429A__
