// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "Zip_GZipFile.h"

#ifndef niEmbedded
#include "API/niLang/StringDef.h"

using namespace ni;

//////////////////////////////////////////////////////////////////////////////////////////////
// File interface implentation

///////////////////////////////////////////////
cGZipFile::cGZipFile(iFileBase* apFile, tU32 aulCompressionMode, bool abA25Z)
{
  ZeroMembers();
  niCheck(niIsOK(apFile),;);

  tFileFlags fileFlags = apFile->GetFileFlags();

  cString strMode;
  if (niFlagTest(fileFlags,eFileFlags_Append))
    strMode.Format("ab%d", aulCompressionMode);
  else if (niFlagTest(fileFlags,eFileFlags_Write))
    strMode.Format("wb%d", (int)aulCompressionMode);
  else if (niFlagTest(fileFlags,eFileFlags_Read))
    strMode = "rb";

  mpFP = gzdopen(apFile, strMode.Chars(), abA25Z);
  if (!mpFP) {
    return;
  }

  if (!niFlagTest(fileFlags,eFileOpenMode_Write)) {
    char tmp[16];
    int r = gzread(mpFP,tmp,16);
    if (r <= 0) {
      int errnum;
      niError(niFmt(_A("Can't validate read the zip stream : %s."),cString(gzerror(mpFP,&errnum)).Chars()));
      gzclose(mpFP);
      mpFP = NULL;
      return;
    }
    gzseek(mpFP,0,SEEK_SET);
  }

  mstrPath = apFile->GetSourcePath();
}

///////////////////////////////////////////////
cGZipFile::~cGZipFile()
{
  if (mpFP) {
    gzclose(mpFP);
    mpFP = NULL;
  }
}

///////////////////////////////////////////////
void cGZipFile::ZeroMembers()
{
  mpFP = NULL;
  mnSize = 0;
  mbWrite = eFalse;
}

///////////////////////////////////////////////
tBool cGZipFile::IsOK() const
{
  return (mpFP != NULL);
}

///////////////////////////////////////////////
tBool cGZipFile::Seek(tI64 offset)
{
  return (gzseek(mpFP, (tI32)offset, SEEK_CUR) >= 0) ? eTrue : eFalse;
}

///////////////////////////////////////////////
tBool cGZipFile::SeekSet(tI64 offset)
{
  return (gzseek(mpFP, (tI32)offset, SEEK_SET) >= 0) ? eTrue : eFalse;
}

///////////////////////////////////////////////
tSize cGZipFile::ReadRaw(void *pOut, tSize nSize)
{
  return gzread(mpFP, pOut, nSize);
}

///////////////////////////////////////////////
tSize cGZipFile::WriteRaw(const void *pOut, tSize nSize)
{
  return gzwrite(mpFP, (voidp)pOut, nSize);
}

///////////////////////////////////////////////
tI64  cGZipFile::Tell()
{
  return gztell(mpFP);
}

///////////////////////////////////////////////
tI64 cGZipFile::GetSize() const
{
  if (!mnSize && !mbWrite) {
    tU8 buffer[2048];
    tI64 nPos = niThis(cGZipFile)->Tell();
    while (1) {
      tU64 read = niThis(cGZipFile)->ReadRaw(buffer,sizeof(buffer));
      niThis(cGZipFile)->mnSize += read;
      if (read != sizeof(buffer))
        break;
    }
    niThis(cGZipFile)->SeekSet((tSize)nPos);
  }
  return mnSize;
}

///////////////////////////////////////////////
tBool __stdcall cGZipFile::Eof()
{
  return gzeof(mpFP);
}

///////////////////////////////////////////////
const achar* cGZipFile::GetSourcePath() const
{
  return mstrPath.Chars();
}

#endif // niEmbedded
