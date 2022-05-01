#ifndef __WIN32_FILE_1BD2D213_E260_410C_AEDF_F841045A5881_H__
#define __WIN32_FILE_1BD2D213_E260_410C_AEDF_F841045A5881_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../../Types.h"
#include "../../IFile.h"
#include "../../Utils/UnknownImpl.h"
#include "Win32_Redef.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Win32
 * @{
 */

//! Windows file handle flags.
enum eOSWinFileFlags
{
  eOSWinFileFlags_DontOwnRead = niBit(0),
  eOSWinFileFlags_DontOwnWrite = niBit(1),
  eOSWinFileFlags_DontOwnRW = niBit(0)|niBit(1),
  eOSWinFileFlags_NoSeek = niBit(2),
  eOSWinFileFlags_Pipe = niBit(3)|eOSWinFileFlags_NoSeek,
  eOSWinFileFlags_Socket = niBit(4)|eOSWinFileFlags_NoSeek,
  //! Connect the read/write handle with ConnectNamedPipe on the first
  //! read or write.
  eOSWinFileFlags_ConnectNamedPipe = niBit(5)|eOSWinFileFlags_Pipe,
  //! \internal
  eOSWinFileFlags_ForceDWORD = 0xFFFFFFFF
};

//! Windows file handle flags type. \see ni::eOSWinFileFlags
typedef tU32 tOSWinFileFlags;

//! iFileBase implementation wrapping Windows File Handles.
class cOSWinFile : public cIUnknownImpl<iFileBase>
{
  niBeginClass(cOSWinFile);

  enum eInternalFlags {
    eInternalFlags_Connected = niBit(31),
    eInternalFlags_TryConnect = niBit(30),
  };

 public:
  cOSWinFile(HANDLE ahRead, HANDLE ahWrite, tOSWinFileFlags aFileFlags, const ni::achar* aaszName)
      : mstrName(aaszName)
      , mFileFlags(aFileFlags)
      , mhRead(ahRead)
      , mhWrite(ahWrite)
  {
  }
  ~cOSWinFile() {
    Invalidate();
  }

  void __stdcall Invalidate() {
    tBool bRWSame = (mhRead == mhWrite);
    if (niFlagIs(mFileFlags,eOSWinFileFlags_ConnectNamedPipe)) {
      // disconnect the pipe if we connected it
      HANDLE h = mhRead ? mhRead : mhWrite;
      if (h) {
        if (mhWrite) {
          ::FlushFileBuffers(mhWrite);
        }
        ::DisconnectNamedPipe(h);
      }
    }
    if (mhRead != NULL) {
      if (niFlagIsNot(mFileFlags,eOSWinFileFlags_DontOwnRead)) {
        __try {
          CloseHandle(mhRead);
        }
        __finally {
        }
      }
      mhRead = NULL;
    }
    if (mhWrite != NULL) {
      if (niFlagIsNot(mFileFlags,eOSWinFileFlags_DontOwnWrite) && !bRWSame) {
        __try {
          CloseHandle(mhWrite);
        }
        __finally {
        }
      }
      mhWrite = NULL;
    }
  }
  void __stdcall _ProcessReadWriteError() {
    DWORD err = GetLastError();
    switch (err) {
      case ERROR_BROKEN_PIPE: {
        if (mhRead) {
          __try {
            CloseHandle(mhRead);
          }
          __finally {
            mhRead = NULL;
          }
        }
        if (mhWrite) {
          __try {
            CloseHandle(mhWrite);
          }
          __finally {
          }
          mhWrite = NULL;
        }
        break;
      }
    }
  }

  tBool __stdcall IsOK() const {
    return mhRead != NULL || mhWrite != NULL;
  }

  tBool __stdcall _CanReadWrite() const {
    if (niFlagIs(mFileFlags,eOSWinFileFlags_ConnectNamedPipe)) {
      if (niFlagIs(mFileFlags,eInternalFlags_TryConnect)) {
        // already tryed to connect, allow read/write if connection has been successful
        return niFlagIs(mFileFlags,eInternalFlags_Connected);
      }
      niFlagOn(niThis(cOSWinFile)->mFileFlags,eInternalFlags_TryConnect);
      HANDLE h = mhRead ? mhRead : mhWrite;
      if (!h) return eFalse;
      BOOL bConnected = ::ConnectNamedPipe(h,NULL) ?
          TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
      niFlagOnIf(niThis(cOSWinFile)->mFileFlags,eInternalFlags_Connected,bConnected);
    }
    return eTrue;
  }
  tBool __stdcall _CanSeek() const {
    if (mhRead != NULL && mhWrite != NULL && mhRead != mhWrite)
      return eFalse;
    if (niFlagIs(mFileFlags,eOSWinFileFlags_NoSeek))
      return eFalse;
    return eTrue;
  }

  virtual tFileFlags __stdcall GetFileFlags() const {
    tFileFlags ff = 0;
    niFlagOnIf(ff,eFileFlags_Read,mhRead!=NULL);
    niFlagOnIf(ff,eFileFlags_Write,mhWrite!=NULL);
    niFlagOnIf(ff,eFileFlags_NoSeek,!_CanSeek());
    niFlagOnIf(ff,eFileFlags_Stream,
               niFlagIs(mFileFlags,eOSWinFileFlags_NoSeek));
    return ff;
  }
  virtual const achar* __stdcall GetSourcePath() const {
    return mstrName.Chars();
  }

  HANDLE _GetSeekHandle() const {
    if (!_CanSeek()) return NULL;
    return (mhRead!=NULL)?mhRead:mhWrite;
  }

  tBool __stdcall _SeekFileHandle(tI64 offset, DWORD moveMethod, tI64* curPos = NULL) {
    HANDLE h = _GetSeekHandle();
    if (!h) return eFalse;
    LARGE_INTEGER newPos;
    if (!SetFilePointerEx(h,*(LARGE_INTEGER*)&offset,&newPos,moveMethod))
      return eFalse;
    if (curPos)
      *curPos = *(tI64*)&newPos;
    return eTrue;
  }

  virtual tBool __stdcall Seek(tI64 offset) {
    return _SeekFileHandle(offset,FILE_CURRENT);
  }
  virtual tBool __stdcall SeekSet(tI64 offset) {
    return _SeekFileHandle(offset,FILE_BEGIN);
  }
  virtual tBool __stdcall SeekEnd(tI64 offset) {
    return _SeekFileHandle(-offset,FILE_END);
  }
  virtual tSize __stdcall ReadRaw(void* pOut, tSize nSize) {
    if (!mhRead || !_CanReadWrite()) return 0;
    if (nSize <= 0) return 0;
    DWORD read;
    /*
      if (niFlagIs(mFileFlags,eOSWinFileFlags_Pipe)) {
      DWORD peeked, avail, left;
      PeekNamedPipe(mhRead,NULL,nSize,&peeked,&avail,&left);
      if (peeked == 0) {
      _ProcessReadWriteError();
      return 0;
      }
      }
    */
    if (::ReadFile(mhRead,pOut,nSize,&read,NULL) != TRUE || read == 0) {
      _ProcessReadWriteError();
      return 0;
    }
    return read;
  }
  virtual tSize __stdcall WriteRaw(const void* pIn, tSize nSize) {
    if (!mhWrite || !_CanReadWrite()) return 0;
    if (nSize <= 0) return 0;
    DWORD wrote;
    if (::WriteFile(mhWrite,pIn,nSize,&wrote,NULL) != TRUE || wrote == 0) {
      _ProcessReadWriteError();
      return 0;
    }
    return wrote;
  }

  virtual tI64 __stdcall Tell() {
    tI64 pos = 0;
    _SeekFileHandle(0,FILE_CURRENT,&pos);
    return pos;
  }

  virtual tI64 __stdcall GetSize() const {
    HANDLE h = _GetSeekHandle();
    if (!h) return eFalse;
    tI64 size = 0;
    ::GetFileSizeEx(h,(LARGE_INTEGER*)&size);
    return size;
  }

  virtual tBool __stdcall Flush() {
    if (!mhWrite) return eFalse;
    return !!FlushFileBuffers(mhWrite);
  }

  virtual tBool __stdcall GetTime(eFileTime aFileTime, iTime* apTime) const {
    return eFalse;
  }
  virtual tBool __stdcall SetTime(eFileTime aFileTime, const iTime* apTime) {
    return eFalse;
  }
  virtual tBool __stdcall Resize(tI64 newSize) {
    return eFalse;
  }

  HANDLE __stdcall _GetReadHandle() const { return mhRead; }
  HANDLE __stdcall _GetWriteHandle() const { return mhWrite; }

 private:
  cString mstrName;
  tOSWinFileFlags mFileFlags;
  HANDLE mhRead;
  HANDLE mhWrite;
  niEndClass(cOSWinFile);
};


/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
} // namespace ni {
#endif // __WIN32_FILE_1BD2D213_E260_410C_AEDF_F841045A5881_H__
