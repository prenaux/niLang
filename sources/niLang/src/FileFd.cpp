#include "FileFd.h"
#include "API/niLang/Utils/UnknownImpl.h"

using namespace ni;

class cFileFd : public cIUnknownImpl<iFileBase,eIUnknownImplFlags_Default>
{
 public:
  cFileFd(int anRead, int anWrite, tFileFdFlags aFileFlags, const char* aaszFileName)
      : mstrName(aaszFileName)
      , mFileFlags(aFileFlags)
      , mnRead(anRead)
      , mnWrite(anWrite)
  {}

  ~cFileFd() {
    Invalidate();
  }

  void __stdcall Invalidate() {
    const tBool bRWSame = (mnRead == mnWrite);
    if (mnRead != -1) {
      if (niFlagIsNot(mFileFlags,eFileFdFlags_DontOwnRead)) {
        close(mnRead);
      }
      mnRead = -1;
    }
    if (mnWrite != -1) {
      if (niFlagIsNot(mFileFlags,eFileFdFlags_DontOwnWrite) && !bRWSame) {
        close(mnWrite);
      }
      mnWrite = -1;
    }
  }

  tBool  __stdcall IsOK() const {
    return mnRead != -1 || mnWrite != -1;
  }

  tBool __stdcall _CanSeek() const {
    if (niFlagIs(mFileFlags,eFileFdFlags_NoSeek))
      return eFalse;
    if (mnRead != -1 && mnWrite != -1 && mnRead != mnWrite)
      return eFalse;
    return eTrue;
  }

  int _GetSeekHandle() const {
    if (!_CanSeek()) return -1;
    return (mnRead != -1) ? mnRead : mnWrite;
  }

  tBool  __stdcall Seek(tI64 offset) {
    int fd = _GetSeekHandle();
    if (fd < 0) return eFalse;
    return FdSeek(fd,offset,SEEK_CUR) == -1LL ? eFalse : eTrue;
  }

  tBool  __stdcall SeekSet(tI64 offset) {
    int fd = _GetSeekHandle();
    if (fd < 0) return eFalse;
    return FdSeek(fd,offset,SEEK_SET) == -1LL ? eFalse : eTrue;
  }

  tBool __stdcall SeekEnd(tI64 offset) {
    int fd = _GetSeekHandle();
    if (fd < 0) return eFalse;
    if (offset == 0) {
      return FdSeek(fd,offset,SEEK_END) == -1LL ? eFalse : eTrue;
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
  tSize  __stdcall ReadRaw(void* pOut, tSize nSize) {
    if (mnRead < 0) return 0;
    return FdRead(mnRead,pOut,nSize);
  }

  ///////////////////////////////////////////////
  tSize  __stdcall WriteRaw(const void* pOut, tSize nSize) {
    if (mnWrite < 0) return 0;
    return FdWrite(mnWrite,pOut,nSize);
  }

  ///////////////////////////////////////////////
  inline tI64 __stdcall Tell() {
    int fd = _GetSeekHandle();
    if (fd < 0) return 0;
    tI64 pos;
    pos = FdTell(fd);
    return pos;
  }

  ///////////////////////////////////////////////
  inline tI64 __stdcall GetSize() const {
    int fd = _GetSeekHandle();
    if (fd < 0) return 0;
    return FdSize(fd);
  }

  ///////////////////////////////////////////////
  tBool __stdcall Flush() {
    if (mnWrite >= 0) {
      FdFlush(mnWrite);
    }
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall GetTime(eFileTime aFileTime, iTime* apTime) const {
    return eFalse;
  }
  tBool __stdcall SetTime(eFileTime aFileTime, const iTime* apTime) {
    return eFalse;
  }
  tBool __stdcall Resize(tI64 newSize) {
    return eFalse;
  }

  ///////////////////////////////////////////////
  tFileFlags __stdcall GetFileFlags() const {
    tFileFlags ff = 0;
    niFlagOnIf(ff,eFileFlags_Read,mnRead!=-1);
    niFlagOnIf(ff,eFileFlags_Write,mnWrite!=-1);
    niFlagOnIf(ff,eFileFlags_NoSeek,!_CanSeek());
    niFlagOnIf(ff,eFileFlags_Stream,niFlagIs(mFileFlags,eFileFdFlags_NoSeek));
    return ff;
  }
  const achar* __stdcall GetSourcePath() const {
    return mstrName.Chars();
  }

 private:
  int mnRead;
  int mnWrite;
  tFileFdFlags mFileFlags;
  cString mstrName;
};

namespace ni {
niExportFunc(ni::iFileBase*) CreateFileFd(int anRead, int anWrite, tFileFdFlags aFileFlags, const char* aaszFileName) {
  return niNew cFileFd(anRead,anWrite,aFileFlags,aaszFileName);
}
}
