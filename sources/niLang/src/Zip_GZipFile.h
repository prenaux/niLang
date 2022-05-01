#ifndef __GZIPFILE_201142678_H__
#define __GZIPFILE_201142678_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Types.h"

#ifndef niEmbedded

#include "Zip_Gzio.h"
#include "API/niLang/IFile.h"
#include "API/niLang/Utils/UnknownImpl.h"

namespace ni {

///////////////////////////////////////////////
class cGZipFile : public cIUnknownImpl<iFileBase>
{
 public:
  cGZipFile(iFileBase* apFile, tU32 aulCompressionMode, bool abA25Z);
  ~cGZipFile();

  //// iFile ////////////////////////////////
  void  ZeroMembers();
  tBool __stdcall IsOK() const;

  tFileFlags __stdcall GetFileFlags() const { return eFileFlags_Read; }

  tBool __stdcall Seek(tI64 offset);
  tBool __stdcall SeekSet(tI64 offset);
  tSize __stdcall ReadRaw(void *pOut, tSize nSize);
  tSize __stdcall WriteRaw(const void *pOut, tSize nSize);
  tI64  __stdcall Tell();
  tI64  __stdcall GetSize() const;
  tBool __stdcall Eof();
  const achar* __stdcall GetSourcePath() const;

  tBool __stdcall SeekEnd(tI64 offset) {
    return SeekSet((offset<GetSize())?(GetSize()-offset):0);
  }
  tBool __stdcall Flush() { return eTrue; }
  tBool __stdcall GetTime(eFileTime, iTime*) const { return eFalse; }
  tBool __stdcall SetTime(eFileTime, const iTime*) { return eFalse; }
  tBool __stdcall Resize(tI64) { return eFalse; }
  //// iFile ////////////////////////////////

 private:
  gzFile    mpFP;
  cString   mstrPath;
  tI64    mnSize;
  tBool   mbWrite;
};

} // end of namespace ni
#endif

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __ZFILE_201142678_H__
