// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Types.h"
#include "API/niLang/IFile.h"
#include "API/niLang/Utils/SmartPtr.h"
#include "API/niLang/Var.h"
#include "API/niLang/Utils/UnknownImpl.h"
#include "API/niLang/ICrypto.h"
#include "API/niLang/STL/vector.h"
#include "TextEncoding.h"

using namespace ni;

// 128MB... avoid morons allocating crap load of memory in script or whatever... if you need
// that much memory use C++ and the low level allocations...
static const ni::tU32 knMaxSensibleFileMemSize = 128*1024*1024;

//! File interface implementation.
class cFileImpl : public ImplRC<iFile,eImplFlags_Default>
{
  niBeginClass(cFileImpl);

  typedef ImplRC<iFile,eImplFlags_Default> BaseImpl;

 public:
  ///////////////////////////////////////////////
  cFileImpl(iFileBase* apBase) {
    //    niAssert(niIsOK(apBase));
    mptrBase = apBase;
    mptrMem = ni::QueryInterface<iFileMemory>(apBase);
    mFormat = eTextEncodingFormat_UTF8;
    mnReadLastByte = 0;
    mnReadNumBytes = eInvalidHandle;
    mnReadMask = 0x80;
    mnWriteLastByte = 0;
    mnWriteNumBytes = eInvalidHandle;
    mnWriteMask = 0x80;
    mnFlags = 0;
  }

  tBool __stdcall IsOK() const {
    niClassIsOK(cFileImpl);
    return niIsOK(mptrBase);
  }

  void __stdcall Invalidate() {
    // Base could be referenced elsewhere, don't invalidate...
    mptrBase = NULL;
  }

  ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) {
    if (aIID == niGetInterfaceUUID(iUnknown))
      return static_cast<iFile*>(this);
    if (aIID == niGetInterfaceUUID(iFile))
      return static_cast<iFile*>(this);
    return mptrBase.IsOK() ? mptrBase->QueryInterface(aIID) : NULL;
  }
  void __stdcall ListInterfaces(iMutableCollection* apLst, tU32 anFlags) const {
    if (mptrBase.IsOK()) {
      mptrBase->ListInterfaces(apLst,anFlags);
    }
    BaseImpl::ListInterfaces(apLst,anFlags);
  }

  iFileBase* __stdcall GetFileBase() const  {
    return mptrBase;
  }
  const achar* __stdcall GetSourcePath() const  {
    return mptrBase.IsOK() ? mptrBase->GetSourcePath() : AZEROSTR;
  }

  tBool __stdcall Seek(tI64 offset) {
    if (IsReadBitsBegan()) EndReadBits();
    if (IsWriteBitsBegan()) EndWriteBits();
    niFlagOff(mnFlags,eFileFlags_PartialRead);
    return mptrBase->Seek(offset);
  }
  tBool __stdcall SeekSet(tI64 offset)  {
    if (IsReadBitsBegan()) EndReadBits();
    if (IsWriteBitsBegan()) EndWriteBits();
    niFlagOff(mnFlags,eFileFlags_PartialRead);
    return mptrBase->SeekSet(offset);
  }
  tBool __stdcall SeekEnd(tI64 offset)  {
    if (IsReadBitsBegan()) EndReadBits();
    if (IsWriteBitsBegan()) EndWriteBits();
    niFlagOff(mnFlags,eFileFlags_PartialRead);
    return mptrBase->SeekEnd(offset);
  }
  tI64 __stdcall Tell() {
    return mptrBase->Tell();
  }
  tI64 __stdcall GetSize() const  {
    return mptrBase->GetSize();
  }

  tSize __stdcall ReadRaw(void* apOut, tSize nSize) {
    if (!apOut || !nSize) return 0;
    if (IsReadBitsBegan()) EndReadBits();
    tSize read = mptrBase->ReadRaw(apOut,nSize);
    niFlagOnIf(mnFlags,eFileFlags_PartialRead,(read!=nSize));
    return read;
  }
  tSize __stdcall WriteRaw(const void* apIn, tSize nSize) {
    if (!apIn || !nSize) return 0;
    if (IsWriteBitsBegan()) EndWriteBits();
    tSize wrote = mptrBase->WriteRaw(apIn,nSize);
    niFlagOnIf(mnFlags,eFileFlags_PartialWrite,(wrote!=nSize));
    return wrote;
  }

  tBool __stdcall Flush() {
    return mptrBase->Flush();
  }

  tBool __stdcall GetTime(eFileTime aFileTime, iTime* apTime) const {
    return mptrBase->GetTime(aFileTime,apTime);
  }
  tBool __stdcall SetTime(eFileTime aFileTime, const iTime* apTime) {
    return mptrBase->SetTime(aFileTime,apTime);
  }
  tBool __stdcall Resize(tI64 newSize) {
    return mptrBase->Resize(newSize);
  }

  ///////////////////////////////////////////////
  inline tFileFlags __stdcall GetFileFlags() const  { return mptrBase->GetFileFlags()|mnFlags; }
  inline tBool __stdcall GetCanRead() const { tFileFlags flags = GetFileFlags(); return niFlagIs(flags,eFileFlags_Read); }
  inline tBool __stdcall GetCanWrite() const { tFileFlags flags = GetFileFlags(); return niFlagIs(flags,eFileFlags_Write); }
  inline tBool __stdcall GetCanSeek() const { tFileFlags flags = GetFileFlags(); return niFlagIsNot(flags,eFileFlags_NoSeek); }
  inline tBool __stdcall GetPartialRead() const { tFileFlags flags = GetFileFlags(); return niFlagIs(flags,eFileFlags_PartialRead); }
  inline tBool __stdcall GetPartialWrite() const { tFileFlags flags = GetFileFlags(); return niFlagIs(flags,eFileFlags_PartialWrite); }
  inline tBool __stdcall GetIsEncoder() const { tFileFlags flags = GetFileFlags(); return niFlagIs(flags,eFileFlags_Encoder); }
  inline tBool __stdcall GetIsDummy() const { tFileFlags flags = GetFileFlags(); return niFlagIs(flags,eFileFlags_Dummy); }
  inline tBool __stdcall GetIsMemory() const { return mptrMem.IsOK(); }
  inline tBool __stdcall GetIsStream() const { tFileFlags flags = GetFileFlags(); return niFlagIs(flags,eFileFlags_Stream); }

  ///////////////////////////////////////////////
  inline tPtr __stdcall GetBase() const { return mptrMem.IsOK() ? mptrMem->GetBase() : NULL; }
  inline tPtr __stdcall GetHere() const { return mptrMem.IsOK() ? mptrMem->GetHere() : NULL; }
  inline tPtr __stdcall GetStop() const { return mptrMem.IsOK() ? mptrMem->GetStop() : NULL; }
  inline tBool __stdcall SetMemPtr(tPtr apMem, tSize anSize, tBool abFree, tBool abKeepHere) {
    if (!mptrMem.IsOK()) return eFalse;
    return mptrMem->SetMemPtr(apMem,anSize,abFree,abKeepHere);
  }
  inline void __stdcall Reset() {
    if (!mptrMem.IsOK()) return;
    mptrMem->Reset();
  }

  ///////////////////////////////////////////////
  //! Read a 32 bits float and move the file pointer forward of 4 bytes.
  inline tF32 __stdcall ReadF32()
  {
    tF32 v = 0;
    ReadRaw(&v, sizeof(v));
    return v;
  }

  ///////////////////////////////////////////////
  //! Read a 64 bits float and move the file pointer forward of 8 bytes.
  inline tF64 __stdcall ReadF64()
  {
    tF64 v = 0;
    ReadRaw(&v, sizeof(v));
    return v;
  }

  ///////////////////////////////////////////////
  //! Read a single byte (8 bits integer) and move the file pointer forward of 1 byte.
  inline tU8  __stdcall Read8()
  {
    tU8  v = 0;
    ReadRaw(&v, sizeof(v));
    return v;
  }

  ///////////////////////////////////////////////
  //! Read a 16 bits little endian interger and move the file pointer forward of 2 bytes.
  inline tU16 __stdcall ReadLE16()
  {
    tU16 v = 0;
    ReadRaw(&v, sizeof(v));
    return niSwapLE16(v);
  }

  ///////////////////////////////////////////////
  //! Read a 32 bits little endian interger and move the file pointer forward of 4 bytes.
  inline tU32 __stdcall ReadLE32()
  {
    tU32 v = 0;
    ReadRaw(&v, sizeof(v));
    return niSwapLE32(v);
  }

  ///////////////////////////////////////////////
  //! Read a 64 bits little endian interger and move the file pointer forward of 8 bytes.
  inline tU64 __stdcall ReadLE64()
  {
    tU64 v = 0;
    ReadRaw(&v, sizeof(v));
    return niSwapLE64(v);
  }

  ///////////////////////////////////////////////
  //! Read a 16 bits big endian interger and move the file pointer forward of 2 bytes.
  inline tU16 __stdcall ReadBE16()
  {
    tU16 v = 0;
    ReadRaw(&v, sizeof(v));
    return niSwapBE16(v);
  }

  ///////////////////////////////////////////////
  //! Read a 32 bits big endian interger and move the file pointer forward of 4 bytes.
  inline tU32 __stdcall ReadBE32()
  {
    tU32 v = 0;
    ReadRaw(&v, sizeof(v));
    return niSwapBE32(v);
  }

  ///////////////////////////////////////////////
  //! Read a 64 bits big endian interger and move the file pointer forward of 8 bytes.
  inline tU64 __stdcall ReadBE64()
  {
    tU64 v = 0;
    ReadRaw(&v, sizeof(v));
    return niSwapBE64(v);
  }

  ///////////////////////////////////////////////
  //! Read a 32 bits float array and move the file pointer forward of 4 bytes * anNumElements.
  inline tSize __stdcall ReadF32Array(tF32* apOut, tSize anNumElements)
  {
    if (anNumElements <= 0) return 0;
    return ReadRaw((tPtr)apOut, sizeof(tF32)*anNumElements);
  }

  ///////////////////////////////////////////////
  //! Read a 64 bits float array and move the file pointer forward of 8 bytes * anNumElements.
  inline tSize __stdcall ReadF64Array(tF64* apOut, tSize anNumElements)
  {
    if (anNumElements <= 0) return 0;
    return ReadRaw((tPtr)apOut, sizeof(tF64)*anNumElements);
  }

  ///////////////////////////////////////////////
  //! Read a 16 bits little endian interger array and move the file pointer forward of 2 bytes * anNumElements.
  inline tSize __stdcall ReadLE16Array(tU16* apOut, tSize anNumElements)
  {
    if (anNumElements <= 0) return 0;
    tSize anRet = ReadRaw((tPtr)apOut, sizeof(tU16)*anNumElements);
#ifdef niBigEndian
    anNumElements = anRet/anNumElements;
    for (tU32 i = 0; i < anNumElements; ++i, ++apOut)
      *apOut = niSwapLE16(*apOut);
#endif
    return anRet;
  }

  ///////////////////////////////////////////////
  //! Read a 16 bits big endian interger array and move the file pointer forward of 2 bytes * anNumElements.
  inline tSize __stdcall ReadBE16Array(tU16* apOut, tSize anNumElements)
  {
    if (anNumElements <= 0) return 0;
    tSize anRet = ReadRaw((tPtr)apOut, sizeof(tU16)*anNumElements);
#ifdef niLittleEndian
    anNumElements = anRet/anNumElements;
    for (tU32 i = 0; i < anNumElements; ++i, ++apOut)
      *apOut = niSwapBE16(*apOut);
#endif
    return anRet;
  }

  ///////////////////////////////////////////////
  //! Read a 32 bits little endian interger array and move the file pointer forward of 4 bytes * anNumElements.
  inline tSize __stdcall ReadLE32Array(tU32* apOut, tSize anNumElements)
  {
    if (anNumElements <= 0) return 0;
    tSize anRet = ReadRaw((tPtr)apOut, sizeof(tU32)*anNumElements);
#ifdef niBigEndian
    anNumElements = anRet/anNumElements;
    for (tU32 i = 0; i < anNumElements; ++i, ++apOut)
      *apOut = niSwapLE32(*apOut);
#endif
    return anRet;
  }

  ///////////////////////////////////////////////
  //! Read a 32 bits big endian interger array and move the file pointer forward of 4 bytes * anNumElements.
  inline tSize __stdcall ReadBE32Array(tU32* apOut, tSize anNumElements)
  {
    if (anNumElements <= 0) return 0;
    tSize anRet = ReadRaw((tPtr)apOut, sizeof(tU32)*anNumElements);
#ifdef niLittleEndian
    anNumElements = anRet/anNumElements;
    for (tU32 i = 0; i < anNumElements; ++i, ++apOut)
      *apOut = niSwapBE32(*apOut);
#endif
    return anRet;
  }

  ///////////////////////////////////////////////
  //! Read a 64 bits little endian interger array and move the file pointer forward of 8 bytes * anNumElements.
  inline tSize __stdcall ReadLE64Array(tU64* apOut, tSize anNumElements)
  {
    if (anNumElements <= 0) return 0;
    tSize anRet = ReadRaw((tPtr)apOut, sizeof(tU64)*anNumElements);
#ifdef niBigEndian
    anNumElements = anRet/anNumElements;
    for (tU32 i = 0; i < anNumElements; ++i, ++apOut)
      *apOut = niSwapLE64(*apOut);
#endif
    return anRet;
  }

  ///////////////////////////////////////////////
  //! Read a 64 bits big endian interger array and move the file pointer forward of 8 bytes * anNumElements.
  inline tSize __stdcall ReadBE64Array(tU64* apOut, tSize anNumElements)
  {
    if (anNumElements <= 0) return 0;
    tSize anRet = ReadRaw((tPtr)apOut, sizeof(tU64)*anNumElements);
#ifdef niLittleEndian
    anNumElements = anRet/anNumElements;
    for (tU32 i = 0; i < anNumElements; ++i, ++apOut)
      *apOut = niSwapBE64(*apOut);
#endif
    return anRet;
  }

  ///////////////////////////////////////////////
  //! Write a 32 bits float and move the file pointer forward of 4 bytes.
  inline tSize __stdcall WriteF32(tF32 v)
  {
    return WriteRaw(&v, sizeof(v));
  }

  ///////////////////////////////////////////////
  //! Write a 64 bits float and move the file pointer forward of 8 bytes.
  inline tSize __stdcall WriteF64(tF64 v)
  {
    return WriteRaw(&v, sizeof(v));
  }

  ///////////////////////////////////////////////
  //! Write a single byte (8 bits integer) and move the file pointer forward of one byte.
  inline tSize __stdcall Write8(tU8 v)
  {
    return WriteRaw(&v, sizeof(v));
  }

  ///////////////////////////////////////////////
  //! Write a little endian 16 bits integer and move the file pointer forward of 2 bytes.
  inline tSize __stdcall WriteLE16(tU16 v)
  {
#ifdef niBigEndian
    v = niSwapLE16(v);
#endif
    return WriteRaw(&v, sizeof(v));
  }

  ///////////////////////////////////////////////
  //! Write a little endian 32 bits integer and move the file pointer forward of 4 bytes.
  inline tSize __stdcall WriteLE32(tU32 v)
  {
#ifdef niBigEndian
    v = niSwapLE32(v);
#endif
    return WriteRaw(&v, sizeof(v));
  }

  ///////////////////////////////////////////////
  //! Write a little endian 64 bits integer and move the file pointer forward of 8 bytes.
  inline tSize __stdcall WriteLE64(tU64 v)
  {
#ifdef niBigEndian
    v = niSwapLE64(v);
#endif
    return WriteRaw(&v, sizeof(v));
  }

  ///////////////////////////////////////////////
  //! Write a big endian 16 bits integer and move the file pointer forward of 2 bytes.
  inline tSize __stdcall WriteBE16(tU16 v)
  {
#ifdef niLittleEndian
    v = niSwapBE16(v);
#endif
    return WriteRaw(&v, sizeof(v));
  }

  ///////////////////////////////////////////////
  //! Write a big endian 32 bits integer and move the file pointer forward of 4 bytes.
  inline tSize __stdcall WriteBE32(tU32 v)
  {
#ifdef niLittleEndian
    v = niSwapBE32(v);
#endif
    return WriteRaw(&v, sizeof(v));
  }

  ///////////////////////////////////////////////
  //! Write a big endian 64 bits integer and move the file pointer forward of 8 bytes.
  inline tSize __stdcall WriteBE64(tU64 v)
  {
#ifdef niLittleEndian
    v = niSwapBE64(v);
#endif
    return WriteRaw(&v, sizeof(v));
  }

  ///////////////////////////////////////////////
  //! Write a 32 bits float array and move the file pointer forward of 4 bytes * anNumElements.
  inline tSize __stdcall WriteF32Array(const tF32* apIn, tU32 anNumElements)
  {
    return WriteRaw((tPtr)apIn, sizeof(tF32)*anNumElements);
  }

  ///////////////////////////////////////////////
  //! Write a 64 bits float and move the file pointer forward of 8 bytes * anNumElements.
  inline tSize __stdcall WriteF64Array(const tF64* apIn, tU32 anNumElements)
  {
    return WriteRaw((tPtr)apIn, sizeof(tF64)*anNumElements);
  }

  ///////////////////////////////////////////////
  //! Write a little endian 16 bits integer and move the file pointer forward of 2 bytes * anNumElements.
  inline tSize __stdcall WriteLE16Array(const tU16* apIn, tU32 anNumElements)
  {
#ifdef niBigEndian
    tSize nRet = 0;
    for (tU32 i = 0; i < anNumElements; ++i, ++apIn)
      nRet += WriteLE16(*apIn);
    return nRet;
#else
    return WriteRaw((tPtr)apIn, sizeof(tU16)*anNumElements);
#endif
  }

  ///////////////////////////////////////////////
  //! Write a big endian 16 bits integer and move the file pointer forward of 2 bytes * anNumElements.
  inline tSize __stdcall WriteBE16Array(const tU16* apIn, tU32 anNumElements)
  {
#ifdef niLittleEndian
    tSize nRet = 0;
    for (tU32 i = 0; i < anNumElements; ++i, ++apIn)
      nRet += WriteBE16(*apIn);
    return nRet;
#else
    return WriteRaw((tPtr)apIn, sizeof(tU16)*anNumElements);
#endif
  }

  ///////////////////////////////////////////////
  //! Write a little endian 32 bits integer and move the file pointer forward of 4 bytes * anNumElements.
  inline tSize __stdcall WriteLE32Array(const tU32* apIn, tU32 anNumElements)
  {
#ifdef niBigEndian
    tSize nRet = 0;
    for (tU32 i = 0; i < anNumElements; ++i, ++apIn)
      nRet += WriteLE32(*apIn);
    return nRet;
#else
    return WriteRaw((tPtr)apIn, sizeof(tU32)*anNumElements);
#endif
  }

  ///////////////////////////////////////////////
  //! Write a big endian 32 bits integer and move the file pointer forward of 4 bytes * anNumElements.
  inline tSize __stdcall WriteBE32Array(const tU32* apIn, tU32 anNumElements)
  {
#ifdef niLittleEndian
    tSize nRet = 0;
    for (tU32 i = 0; i < anNumElements; ++i, ++apIn)
      nRet += WriteBE32(*apIn);
    return nRet;
#else
    return WriteRaw((tPtr)apIn, sizeof(tU32)*anNumElements);
#endif
  }

  ///////////////////////////////////////////////
  //! Write a little endian 64 bits integer and move the file pointer forward of 8 bytes * anNumElements.
  inline tSize __stdcall WriteLE64Array(const tU64* apIn, tU32 anNumElements)
  {
#ifdef niBigEndian
    tSize nRet = 0;
    for (tU32 i = 0; i < anNumElements; ++i, ++apIn)
      nRet += WriteLE64(*apIn);
    return nRet;
#else
    return WriteRaw((tPtr)apIn, sizeof(tU64)*anNumElements);
#endif
  }

  ///////////////////////////////////////////////
  //! Write a big endian 64 bits integer and move the file pointer forward of 8 bytes * anNumElements.
  inline tSize __stdcall WriteBE64Array(const tU64* apIn, tU32 anNumElements)
  {
#ifdef niLittleEndian
    tSize nRet = 0;
    for (tU32 i = 0; i < anNumElements; ++i, ++apIn)
      nRet += WriteBE64(*apIn);
    return nRet;
#else
    return WriteRaw((tPtr)apIn, sizeof(tU64)*anNumElements);
#endif
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall WriteVar(const Var& aVar) {
    return VarSerializeWrite(this,(Var&)aVar);
  }
  virtual tBool __stdcall ReadVarEx(Var& aVar) {
    return VarSerializeRead(this,(Var&)aVar);
  }
  virtual Var __stdcall ReadVar() {
    Var v = niVarNull;
    ReadVarEx(v);
    return v;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall WriteBitsPackedVar(const Var& aVar) {
    return VarSerializeWriteBitsPacked(this,aVar);
  }
  virtual tBool __stdcall ReadBitsPackedVarEx(Var& aVar) {
    return VarSerializeReadBitsPacked(this,aVar);
  }
  virtual Var __stdcall ReadBitsPackedVar() {
    Var v = niVarNull;
    ReadBitsPackedVarEx(v);
    return v;
  }

  ///////////////////////////////////////////////
  //! Get the text encoding format.
  inline eTextEncodingFormat __stdcall GetTextEncodingFormat()
  {
    if (mFormat == eTextEncodingFormat_Unknown) {
      mFormat = TextEncodingFormatGet(this,eFalse,eTextEncodingFormat_UTF8);
    }
    return mFormat;
  }

  ///////////////////////////////////////////////
  //! Set the text encoding format.
  inline void __stdcall SetTextEncodingFormat(eTextEncodingFormat aFormat)
  {
    mFormat = aFormat;
  }

  ///////////////////////////////////////////////
  //! Read text file header.
  inline tBool __stdcall BeginTextFileRead(tBool abSeekSetZero)
  {
    if (abSeekSetZero)  SeekSet(0);
    mFormat = TextEncodingFormatGet(
        this,eTrue,(mFormat == eTextEncodingFormat_Unknown ? eTextEncodingFormat_UTF8 : mFormat));
    niFlagOff(mnFlags,eFileFlags_PartialRead);
    return mFormat != eTextEncodingFormat_Unknown;
  }

  ///////////////////////////////////////////////
  //! Write text file header.
  inline tBool __stdcall BeginTextFileWrite(eTextEncodingFormat aFormat, tBool abSeekSetZero)
  {
    if (abSeekSetZero)  SeekSet(0);
    if (aFormat != eTextEncodingFormat_Unknown) mFormat = aFormat;
    return TextEncodingFormatWriteHeader(this,aFormat);
  }

  ///////////////////////////////////////////////
  inline cString __stdcall ReadString() {
    cString strOut;
    TextEncodingFormatReadString(this, GetTextEncodingFormat(), strOut);
    return strOut;
  }
  inline cString __stdcall ReadStringLine() {
    cString strOut;
    TextEncodingFormatReadStringLine(this, GetTextEncodingFormat(), strOut);
    return strOut;
  }
  inline tSize __stdcall WriteString(const achar* aaszString) {
    return TextEncodingFormatWriteString(this, GetTextEncodingFormat(), aaszString, eFalse);
  }
  inline tSize __stdcall WriteStringZ(const achar* aaszString) {
    return TextEncodingFormatWriteString(this, GetTextEncodingFormat(), aaszString, eTrue);
  }
  virtual tU32 __stdcall ReadChar() {
    return TextEncodingFormatReadChar(this, GetTextEncodingFormat());
  }
  virtual tSize __stdcall WriteChar(tU32 anChar) {
    return TextEncodingFormatWriteChar(this, GetTextEncodingFormat(), anChar);
  }

  ///////////////////////////////////////////////
  inline cString __stdcall ReadStringEx(eTextEncodingFormat aFmt) {
    cString strOut;
    TextEncodingFormatReadString(this, aFmt, strOut);
    return strOut;
  }
  inline cString __stdcall ReadStringLineEx(eTextEncodingFormat aFmt) {
    cString strOut;
    TextEncodingFormatReadStringLine(this, aFmt, strOut);
    return strOut;
  }
  inline tSize __stdcall WriteStringEx(eTextEncodingFormat aFmt, const achar* aaszString) {
    return TextEncodingFormatWriteString(this, aFmt, aaszString, eFalse);
  }
  inline tSize __stdcall WriteStringZEx(eTextEncodingFormat aFmt, const achar* aaszString) {
    return TextEncodingFormatWriteString(this, aFmt, aaszString, eTrue);
  }
  virtual tU32 __stdcall ReadCharEx(eTextEncodingFormat aFmt) {
    return TextEncodingFormatReadChar(this, aFmt);
  }
  virtual tSize __stdcall WriteCharEx(eTextEncodingFormat aFmt, tU32 anChar) {
    return TextEncodingFormatWriteChar(this, aFmt, anChar);
  }

  ///////////////////////////////////////////////
  inline void __stdcall BeginWriteBits() {
    mnWriteLastByte = 0;
    mnWriteNumBytes = 0;
    mnWriteMask = 0x80;
  }
  inline tSize __stdcall EndWriteBits() {
    tSize nRet = mnWriteNumBytes;
    if (mnWriteMask != 0x80) {
      mptrBase->WriteRaw(&mnWriteLastByte,sizeof(tU8));
      ++nRet;
    }
    mnWriteLastByte = 0;
    mnWriteMask = 0x80;
    mnWriteNumBytes = eInvalidHandle;
    return nRet;
  }

  ///////////////////////////////////////////////
  inline tBool __stdcall IsWriteBitsBegan() {
    return mnWriteNumBytes != eInvalidHandle;
  }

  ///////////////////////////////////////////////
  inline void __stdcall WriteBits(tPtr apData, tU32 anBits, tBool abInvertBytesOrder)
  {
    if (anBits <= 0) return;
    if (IsReadBitsBegan())
      EndReadBits();
    if (!IsWriteBitsBegan())
      BeginWriteBits();
    if (abInvertBytesOrder)
    {
      tU32 nNumBytes = (anBits+7)>>3;
      tU8* pSrc = ((tU8*)apData)+(nNumBytes-1);
      tU32 nSrcMask = 0x80 >> (8-(anBits-((nNumBytes-1)<<3)));
      for (;;)
      {
        if (*pSrc & nSrcMask)
          mnWriteLastByte |= mnWriteMask;

        if (mnWriteMask == 1) {
          mnWriteMask = 0x80;
          mptrBase->WriteRaw(&mnWriteLastByte,sizeof(tU8));
          ++mnWriteNumBytes;
          mnWriteLastByte = 0;
        }
        else {
          mnWriteMask >>= 1;
        }

        if (--anBits == 0)
          break;

        if (nSrcMask == 1) {
          nSrcMask = 0x80;
          --pSrc;
        }
        else {
          nSrcMask >>= 1;
        }
      }
    }
    else
    {
      tU8* pSrc = (tU8*)apData;
      tU32 nSrcMask = 0x80;
      if (anBits < 8)
        nSrcMask >>= (8-anBits);
      for (;;)
      {
        if (*pSrc & nSrcMask)
          mnWriteLastByte |= mnWriteMask;

        if (mnWriteMask == 1) {
          mnWriteMask = 0x80;
          mptrBase->WriteRaw(&mnWriteLastByte,sizeof(tU8));
          ++mnWriteNumBytes;
          mnWriteLastByte = 0;
        }
        else {
          mnWriteMask >>= 1;
        }

        if (--anBits == 0)
          break;

        if (nSrcMask == 1) {
          nSrcMask = 0x80;
          if (anBits < 8)
            nSrcMask >>= (8-anBits);
          ++pSrc;
        }
        else {
          nSrcMask >>= 1;
        }
      }
    }
  }

  ///////////////////////////////////////////////
  inline void __stdcall WriteBits8(tU8 anData, tU32 anBits)
  {
    WriteBits((tPtr)&anData,anBits,kbIsLittleEndian);
  }

  ///////////////////////////////////////////////
  inline void __stdcall WriteBits16(tU16 anData, tU32 anBits)
  {
    niAssert(anBits <= 16);
    WriteBits((tPtr)&anData,anBits,kbIsLittleEndian);
  }

  ///////////////////////////////////////////////
  inline void __stdcall WriteBits32(tU32 anData, tU32 anBits)
  {
    niAssert(anBits <= 32);
    WriteBits((tPtr)&anData,anBits,kbIsLittleEndian);
  }

  ///////////////////////////////////////////////
  inline void __stdcall WriteBits64(tU64 anData, tU32 anBits)
  {
    niAssert(anBits <= 64);
    WriteBits((tPtr)&anData,anBits,kbIsLittleEndian);
  }

  ///////////////////////////////////////////////
  inline void __stdcall BeginReadBits()
  {
    mnReadLastByte = 0;
    mnReadMask = 0x80;
    mnReadNumBytes = 0;
  }

  ///////////////////////////////////////////////
  inline tSize __stdcall EndReadBits()
  {
    tSize nRet = mnReadNumBytes;
    mnReadLastByte = 0;
    mnReadMask = 0x80;
    mnReadNumBytes = eInvalidHandle;
    return nRet;
  }

  ///////////////////////////////////////////////
  inline tBool __stdcall IsReadBitsBegan()
  {
    return mnReadNumBytes != eInvalidHandle;
  }

  ///////////////////////////////////////////////
  inline void __stdcall ReadBits(tPtr apData, tU32 anBits, tBool abInvertBytesOrder)
  {
    if (anBits <= 0) return;
    if (IsWriteBitsBegan())
      EndWriteBits();
    if (!IsReadBitsBegan())
      BeginReadBits();
    if (abInvertBytesOrder)
    {
      tU32 nNumBytes = (anBits+7)>>3;
      tU8* pDst = ((tU8*)apData)+(nNumBytes-1);
      tU32 nDstMask = 0x80 >> ((8-(anBits-((nNumBytes-1)<<3))));
      for (;;)
      {
        if (mnReadMask == 0x80) {
          mptrBase->ReadRaw(&mnReadLastByte,sizeof(tU8));
          ++mnReadNumBytes;
        }

        if (mnReadLastByte & mnReadMask)
          *pDst |= nDstMask;
        else
          *pDst &= ~nDstMask;

        if (mnReadMask == 1) {
          mnReadMask = 0x80;
        }
        else {
          mnReadMask >>= 1;
        }

        if (--anBits == 0)
          break;

        if (nDstMask == 1) {
          nDstMask = 0x80;
          --pDst;
        }
        else {
          nDstMask >>= 1;
        }
      }
    }
    else
    {
      tU8* pDst = (tU8*)apData;
      tU32 nDstMask = 0x80;
      if (anBits < 8) nDstMask >>= (8-anBits);
      for (;;)
      {
        if (mnReadMask == 0x80) {
          mptrBase->ReadRaw(&mnReadLastByte,sizeof(tU8));
          ++mnReadNumBytes;
        }

        if (mnReadLastByte & mnReadMask)
          *pDst |= nDstMask;
        else
          *pDst &= ~nDstMask;

        if (mnReadMask == 1) {
          mnReadMask = 0x80;
        }
        else {
          mnReadMask >>= 1;
        }

        if (--anBits == 0)
          break;

        if (nDstMask == 1) {
          nDstMask = 0x80;
          if (anBits < 8)
            nDstMask >>= (8-anBits);
          ++pDst;
        }
        else {
          nDstMask >>= 1;
        }
      }
    }
  }

  ///////////////////////////////////////////////
  inline tU8 __stdcall ReadBitsU8(tU32 anBits)
  {
    niAssert(anBits <= 8);
    tU8 nRet = 0;
    ReadBits((tPtr)&nRet,anBits,kbIsLittleEndian);
    return nRet;
  }

  ///////////////////////////////////////////////
  inline tU16 __stdcall ReadBitsU16(tU32 anBits)
  {
    niAssert(anBits <= 16);
    tU16 v = 0;
    ReadBits((tPtr)&v,anBits,kbIsLittleEndian);
    return v;
  }

  ///////////////////////////////////////////////
  inline tU32 __stdcall ReadBitsU32(tU32 anBits)
  {
    niAssert(anBits <= 32);
    tU32 v = 0;
    ReadBits((tPtr)&v,anBits,kbIsLittleEndian);
    return v;
  }

  ///////////////////////////////////////////////
  inline tU64 __stdcall ReadBitsU64(tU32 anBits)
  {
    niAssert(anBits <= 64);
    tU64 v = 0;
    ReadBits((tPtr)&v,anBits,kbIsLittleEndian);
    return v;
  }

  ///////////////////////////////////////////////
  inline tI8 __stdcall ReadBitsI8(tU32 anBits)
  {
    niAssert(anBits <= 8);
    tI8 v = 0;
    ReadBits((tPtr)&v,anBits,kbIsLittleEndian);
    return v;
  }

  ///////////////////////////////////////////////
  inline tI16 __stdcall ReadBitsI16(tU32 anBits)
  {
    niAssert(anBits <= 16);
    tI16 v = 0;
    ReadBits((tPtr)&v,anBits,kbIsLittleEndian);
    return v;
  }

  ///////////////////////////////////////////////
  inline tI32 __stdcall ReadBitsI32(tU32 anBits)
  {
    niAssert(anBits <= 32);
    tI32 v = 0;
    ReadBits((tPtr)&v,anBits,kbIsLittleEndian);
    return v;
  }

  ///////////////////////////////////////////////
  inline tI64 __stdcall ReadBitsI64(tU32 anBits)
  {
    niAssert(anBits <= 64);
    tI64 v = 0;
    ReadBits((tPtr)&v,anBits,kbIsLittleEndian);
    return v;
  }

  ///////////////////////////////////////////////
  inline tU32 __stdcall GetMinNumBitsU32(tU32 anNumber)
  {
    tU32 nNumBits = 32;
    tU32 nMask = 0x80000000;
    while (nNumBits-- > 1)
    {
      if (anNumber & nMask)
        break;
      nMask >>= 1;
    }
    return nNumBits+1;
  }

  ///////////////////////////////////////////////
  inline tU32 __stdcall GetMinNumBitsI32(tI32 anNumber)
  {
    tU32 nNumBits = 31;
    tU32 nMask = 0x40000000;
    anNumber = ni::Abs(anNumber);
    while (nNumBits-- > 1) {
      if (anNumber & nMask)
        break;
      nMask >>= 1;
    }
    return nNumBits+2;
  }

  ///////////////////////////////////////////////
  inline tU32 __stdcall GetMinNumBitsU64(tU64 anNumber)
  {
    tU32 nNumBits = 64;
    tU64 nMask = 0x8000000000000000ULL;
    while (nNumBits-- > 1) {
      if (anNumber & nMask)
        break;
      nMask >>= 1;
    }
    return nNumBits+1;
  }

  ///////////////////////////////////////////////
  inline tU32 __stdcall GetMinNumBitsI64(tI64 anNumber)
  {
    tU32 nNumBits = 63;
    tU64 nMask = 0x4000000000000000ULL;
    anNumber = ni::Abs(anNumber);
    while (nNumBits-- > 1) {
      if (anNumber & nMask)
        break;
      nMask >>= 1;
    }
    return nNumBits+2;
  }

  ///////////////////////////////////////////////
  inline void __stdcall WriteBit(tBool abBool)
  {
    WriteBits8(abBool,1);
  }

  ///////////////////////////////////////////////
  inline tBool __stdcall ReadBit()
  {
    return ReadBitsU8(1);
  }

  ///////////////////////////////////////////////
  inline void __stdcall WriteBitsPackedU64(tU64 anNumber)
  {
    if (anNumber <= 0x0000FFFFFFFFFFFFULL)
    {
      WriteBit(eTrue);
      WriteBitsPackedU48(anNumber);
    }
    else
    {
      WriteBit(eFalse);
      WriteBits64(anNumber,64);
    }
  }

  ///////////////////////////////////////////////
  inline void __stdcall WriteBitsPackedU48(tU64 anNumber)
  {
    if (anNumber <= 0x00000000FFFFFFFF) {
      WriteBit(eTrue);
      WriteBitsPackedU32((tU32)anNumber);
    }
    else {
      WriteBit(eFalse);
      WriteBits64(anNumber,48);
    }
  }

  ///////////////////////////////////////////////
  inline void __stdcall WriteBitsPackedU32(tU32 anNumber)
  {
    if (anNumber <= 0x00FFFFFF) {
      WriteBit(eTrue);
      WriteBitsPackedU24(anNumber);
    }
    else {
      WriteBit(eFalse);
      WriteBits32(anNumber,32);
    }
  }

  ///////////////////////////////////////////////
  inline void __stdcall WriteBitsPackedU24(tU32 anNumber)
  {
    if (anNumber <= 0x0000FFFF) {
      WriteBit(eTrue);
      WriteBitsPackedU16((tU16)anNumber);
    }
    else {
      WriteBit(eFalse);
      WriteBits32(anNumber,24);
    }
  }

  ///////////////////////////////////////////////
  inline void __stdcall WriteBitsPackedU16(tU16 anNumber)
  {
    if (anNumber <= 0x000000FF) {
      WriteBit(eTrue);
      WriteBitsPackedU8((tU8)anNumber);
    }
    else
    {
      WriteBit(eFalse);
      WriteBits16(anNumber,16);
    }
  }

  ///////////////////////////////////////////////
  inline void __stdcall WriteBitsPackedU8(tU8 anNumber)
  {
    if (anNumber <= 0x0000000F) {
      WriteBit(eTrue);
      WriteBits8(anNumber,4);
    }
    else {
      WriteBit(eFalse);
      WriteBits8(anNumber,8);
    }
  }

  ///////////////////////////////////////////////
  inline tU64 __stdcall ReadBitsPackedU64()
  {
    if (ReadBit())  return ReadBitsPackedU48();
    else      return ReadBitsU64(64);
  }

  ///////////////////////////////////////////////
  inline tU64 __stdcall ReadBitsPackedU48()
  {
    if (ReadBit())  return ReadBitsPackedU32();
    else      return ReadBitsU64(48);
  }

  ///////////////////////////////////////////////
  inline tU32 __stdcall ReadBitsPackedU32()
  {
    if (ReadBit())  return ReadBitsPackedU24();
    else      return ReadBitsU32(32);
  }

  ///////////////////////////////////////////////
  inline tU32 __stdcall ReadBitsPackedU24()
  {
    if (ReadBit())  return ReadBitsPackedU16();
    else      return ReadBitsU32(24);
  }

  ///////////////////////////////////////////////
  inline tU16 __stdcall ReadBitsPackedU16()
  {
    if (ReadBit())  return ReadBitsPackedU8();
    else      return ReadBitsU16(16);
  }

  ///////////////////////////////////////////////
  inline tU8 __stdcall ReadBitsPackedU8()
  {
    if (ReadBit())  return ReadBitsU8(4);
    else      return ReadBitsU8(8);
  }

  ///////////////////////////////////////////////
  inline void __stdcall WriteBitsPackedI64(tI64 anNumber)
  {
    if (ni::Abs(anNumber) <= 0x00007FFFFFFFFFFFLL) {
      WriteBit(eTrue);
      WriteBitsPackedI48(anNumber);
    }
    else {
      WriteBit(eFalse);
      WriteBits64(anNumber,64);
    }
  }

  ///////////////////////////////////////////////
  inline void __stdcall WriteBitsPackedI48(tI64 anNumber)
  {
    if (ni::Abs(anNumber) <= 0x000000007FFFFFFF) {
      WriteBit(eTrue);
      WriteBitsPackedI32((tI32)anNumber);
    }
    else {
      WriteBit(eFalse);
      WriteBits64(anNumber,48);
    }
  }

  ///////////////////////////////////////////////
  inline void __stdcall WriteBitsPackedI32(tI32 anNumber)
  {
    if (ni::Abs(anNumber) <= 0x007FFFFF) {
      WriteBit(eTrue);
      WriteBitsPackedI24(anNumber);
    }
    else {
      WriteBit(eFalse);
      WriteBits32(anNumber,32);
    }
  }

  ///////////////////////////////////////////////
  inline void __stdcall WriteBitsPackedI24(tI32 anNumber)
  {
    if (ni::Abs(anNumber) <= 0x00007FFF) {
      WriteBit(eTrue);
      WriteBitsPackedI16((tI16)anNumber);
    }
    else {
      WriteBit(eFalse);
      WriteBits32(anNumber,24);
    }
  }

  ///////////////////////////////////////////////
  inline void __stdcall WriteBitsPackedI16(tI16 anNumber)
  {
    if (ni::Abs(anNumber) <= 0x0000007F) {
      WriteBit(eTrue);
      WriteBitsPackedI8((tI8)anNumber);
    }
    else {
      WriteBit(eFalse);
      WriteBits16(anNumber,16);
    }
  }

  ///////////////////////////////////////////////
  inline void __stdcall WriteBitsPackedI8(tI8 anNumber)
  {
    WriteBits((tPtr)&anNumber,8,eFalse);
  }

  ///////////////////////////////////////////////
  inline tI64 __stdcall ReadBitsPackedI64()
  {
    if (ReadBit())  return ReadBitsPackedI48();
    else      return ReadBitsI64(64);
  }

  ///////////////////////////////////////////////
  inline tI64 __stdcall ReadBitsPackedI48()
  {
    if (ReadBit())  return ReadBitsPackedI32();
    else      return ReadBitsI64(48);
  }

  ///////////////////////////////////////////////
  inline tI32 __stdcall ReadBitsPackedI32()
  {
    if (ReadBit())  return ReadBitsPackedI24();
    else      return ReadBitsI32(32);
  }

  ///////////////////////////////////////////////
  inline tI32 __stdcall ReadBitsPackedI24()
  {
    if (ReadBit())  return ReadBitsPackedI16();
    else      return ReadBitsI32(24);
  }

  ///////////////////////////////////////////////
  inline tI16 __stdcall ReadBitsPackedI16()
  {
    if (ReadBit())  return ReadBitsPackedI8();
    else      return ReadBitsI16(16);
  }

  ///////////////////////////////////////////////
  inline tI8 __stdcall ReadBitsPackedI8()
  {
    tI8 v;
    ReadBits((tPtr)&v,8,eFalse);
    return v;
  }

  ///////////////////////////////////////////////
  inline cString __stdcall ReadBitsString() {
    cString strOut;
    TextEncodingFormatReadString(this, eTextEncodingFormat_BitStream, strOut);
    return strOut;
  }
  inline tSize __stdcall WriteBitsString(const achar* aaszIn) {
    return TextEncodingFormatWriteString(this, eTextEncodingFormat_BitStream, aaszIn, eTrue);
  }

  ///////////////////////////////////////////////
  inline tI64 __stdcall WriteFile(iFileBase* apFile, tI64 anSize)
  {
    niCheckIsOK(apFile,0);
    Ptr<iFileBase> fp = apFile;
    tU8 buff[niStackBufferSize];
    tI64 sz = 0;
    if (anSize <= 0) {
      // write the file until the end
      for (;;) {
        tSize read = apFile->ReadRaw(buff,niCountOf(buff));
        if (read == 0)
          break;
        sz += read;
        this->WriteRaw(buff,read);
      }
    }
    else {
      // write the specified number of bytes
      for (;;) {
        tSize toread = ni::Min((tSize)(anSize-sz),niCountOf(buff));
        tSize read = apFile->ReadRaw(buff,toread);
        if (read == 0)
          break;
        sz += read;
        this->WriteRaw(buff,read);
        if (sz >= anSize)
          break;
      }
    }
    return sz;
  }

  virtual cString __stdcall ReadRawToString(eRawToStringEncoding aFormat, tInt anNumBytes) {
    sReadBufferFromFile buffer(this,anNumBytes);
    if (!buffer.readAll()) {
      return AZEROSTR;
    }
    switch (aFormat) {
      case eRawToStringEncoding_Hex: {
        return HexEncodeToString(buffer.begin(), (tInt)buffer.size(), eFalse);
      }
      case eRawToStringEncoding_Base64: {
        return Base64EncodeToString(buffer.begin(), (tInt)buffer.size());
      }
      case eRawToStringEncoding_Base32: {
        return Base32EncodeToString(buffer.begin(), (tInt)buffer.size(), eFalse);
      }
    }
    return AZEROSTR;
  }

  virtual tInt __stdcall WriteRawFromString(eRawToStringEncoding aFormat, const achar* apSrc) {
    if (!niStringIsOK(apSrc)) {
      return 0;
    }
    const tSize slen = StrLen(apSrc);
    switch (aFormat) {
      case eRawToStringEncoding_Hex: {
        const tInt sz = HexDecodeOutputSize(apSrc,slen);
        if (sz <= 0) { return 0; }
        sWriteBufferToFile buffer(this,sz);
        const tInt ws = HexDecode(buffer.begin(),(tInt)buffer.size(),apSrc,slen);
        return buffer.commit(ws);
      }
      case eRawToStringEncoding_Base64: {
        const tInt sz = Base64DecodeOutputSize(apSrc,slen);
        if (sz <= 0) { return 0; }
        sWriteBufferToFile buffer(this,sz);
        const tInt ws = Base64Decode(buffer.begin(),(tInt)buffer.size(),apSrc,slen);
        return buffer.commit(ws);
      }
      case eRawToStringEncoding_Base32: {
        const tInt sz = Base32DecodeOutputSize(apSrc,slen);
        if (sz <= 0) { return 0; }
        sWriteBufferToFile buffer(this,sz);
        const tInt ws = Base32Decode(buffer.begin(),(tInt)buffer.size(),apSrc,slen);
        return buffer.commit(ws);
      }
    }
    return 0;
  }

  virtual ni::cString __stdcall ReadQuotedLine() {
    ni::cString r;
    while (!this->GetPartialRead()) {
      r += this->ReadStringLine();
      bool openQuote = false;
      const achar* p = r.begin();
      tU32 pc = 0;
      while (*p) {
        const tU32 c = *p;
        if (c == '"' && (*(p+1) != '"') && pc != '\\') {
          openQuote = !openQuote;
        }
        pc = c;
        ++p;
      }
      if (!openQuote) {
        break;
      }
      r += "\n";
    }
    return r;
  }

 private:
  Ptr<iFileBase>    mptrBase;
  Ptr<iFileMemory>    mptrMem;
  enum eTextEncodingFormat  mFormat;
  tSize mnWriteNumBytes;
  mutable tSize mnReadNumBytes;
  tU8   mnWriteLastByte;
  tU8   mnWriteMask;
  mutable tU8   mnReadLastByte;
  mutable tU8   mnReadMask;
  tU32    mnFlags;

  niEndClass(cFileImpl);
};

#undef READV

//////////////////////////////////////////////////////////////////////////////////////////////
//! Dummy iFile implementation.
class cFileWriteDummy : public ImplRC<iFileBase,eImplFlags_Default>
{
 public:
  cFileWriteDummy(tI64 anSize) {
    ZeroMembers();
    mnSize = anSize;
    mnPos = 0;
  }

  ~cFileWriteDummy() {
  }

  void  ZeroMembers() {
    mnSize = mnPos = 0;
  }

  //// iFile ////////////////////////////////
  inline tFileFlags __stdcall GetFileFlags() const {
    return eFileFlags_Write|eFileFlags_Dummy;
  }
  inline tBool  __stdcall Seek(tI64 offset) {
    tI64 nNewPos = mnPos + offset;
    if (nNewPos < 0)    { nNewPos = 0; }
    if (nNewPos > tOffset(mnSize)) { nNewPos = mnSize; }
    mnPos = nNewPos;
    return eTrue;
  }
  inline tBool  __stdcall SeekSet(tI64 offset) {
    tI64 nNewPos = offset;
    if (nNewPos < 0)    { nNewPos = 0; }
    if (nNewPos > tOffset(mnSize)) { nNewPos = mnSize; }
    mnPos = nNewPos;
    return eTrue;
  }
  inline tSize  __stdcall ReadRaw(void* pOut, tSize nSize) {
    niUnused(pOut);
    niUnused(nSize);
    return 0;
  }
  inline tSize  __stdcall WriteRaw(const void* apIn, tSize nSize) {
    niUnused(apIn);
    if ((tI64)(mnPos+nSize) > mnSize) {
      mnSize = mnPos + nSize;
    }
    mnPos += nSize;
    return nSize;
  }
  inline tI64 __stdcall Tell() {
    return mnPos;
  }
  inline tI64 __stdcall GetSize() const {
    return mnSize;
  }
  inline const achar* __stdcall GetSourcePath() const {
    return NULL;
  }

  inline tBool __stdcall SeekEnd(tI64 offset) { return SeekSet((offset<GetSize())?(GetSize()-offset):0); }
  inline tBool __stdcall Flush()  { return eTrue; }
  inline tBool __stdcall GetTime(eFileTime aFileTime, iTime* apTime) const { niUnused(aFileTime);niUnused(apTime); return eFalse; }
  inline tBool __stdcall SetTime(eFileTime aFileTime, const iTime* apTime) { niUnused(aFileTime);niUnused(apTime); return eFalse; }
  inline tBool __stdcall Resize(tI64 newSize) { return eFalse; }
  //// iFile ////////////////////////////////

 protected:
  tI64  mnSize;
  tI64  mnPos;
};

// TODO : FileMem with custom memory will cause a crash when deleted cause the buffer was not allocated in the same memory heap/dll

//////////////////////////////////////////////////////////////////////////////////////////////
//! File interface that can read and write in any preallocated system memory buffer
class cFileMem : public ImplRC<iFileBase,eImplFlags_Default,iFileMemory>
{
 public:
  ///////////////////////////////////////////////
  cFileMem(tPtr pMem, tSize nSize, tBool bFree, const achar* aszPath = NULL) {
    ZeroMembers();
    SetSourcePath(aszPath);
    SetMemPtr(pMem, nSize, bFree, eFalse);
  }

  ///////////////////////////////////////////////
  cFileMem(tSize nSize, const achar* aszPath = NULL) {
    ZeroMembers();
    SetMemPtr((tPtr)niMalloc(nSize), nSize, eTrue, eFalse);
    SetSourcePath(aszPath);
  }

  ///////////////////////////////////////////////
  ~cFileMem() {
    if (mbFree && mpBase) {
      niFree(mpBase);
      mpBase = NULL;
    }
  }

  ///////////////////////////////////////////////
  void SetSourcePath(const achar* aszPath) {
    if (!aszPath)
      mstrSourcePath.Clear();
    else
      mstrSourcePath = aszPath;
  }

  ///////////////////////////////////////////////
  void  ZeroMembers()  {
    mbFree = eFalse;
    mpBase = NULL;
    mpHere = NULL;
    mpStop = NULL;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall IsOK() const {
    return mpBase != NULL;
  }

  ///////////////////////////////////////////////
  tBool __stdcall Seek(tI64 offset) {
    tU8* pNewPos = mpHere + offset;
    if (pNewPos < mpBase) { pNewPos = mpBase; }
    if (pNewPos > mpStop) { pNewPos = mpStop; }
    mpHere = pNewPos;
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall SeekSet(tI64 offset) {
    tU8* pNewPos = mpBase + offset;
    if (pNewPos < mpBase) { pNewPos = mpBase; }
    if (pNewPos > mpStop) { pNewPos = mpStop; }
    mpHere = pNewPos;
    return eTrue;
  }

  ///////////////////////////////////////////////
  tSize __stdcall ReadRaw(void* pOut, tSize nSize) {
    if (mpHere >= mpStop) {
      mpHere = mpStop+1;
      return 0;
    }

    if (mpHere+nSize > mpStop) {
      tSize size = (mpStop-mpHere);
      memcpy(pOut, mpHere, size);
      mpHere = mpStop;
      return size;
    }
    else {
      memcpy(pOut, mpHere, nSize);
      mpHere += nSize;
      return nSize;
    }
  }

  ///////////////////////////////////////////////
  virtual tSize __stdcall WriteRaw(const void* apIn, tSize nSize) {
    if (mpHere >= mpStop) {
      mpHere = mpStop+1;
      return 0;
    }

    if (mpHere+nSize > mpStop) {
      tSize size = (mpStop-mpHere);
      memcpy(mpHere, apIn, size);
      mpHere = mpStop;
      return size;
    }
    else {
      memcpy(mpHere, apIn, nSize);
      mpHere += nSize;
      return nSize;
    }
  }

  ///////////////////////////////////////////////
  tI64  __stdcall Tell() {
    return (mpHere - mpBase);
  }

  ///////////////////////////////////////////////
  tI64  __stdcall GetSize() const {
    return (mpStop - mpBase);
  }

  ///////////////////////////////////////////////
  const achar* __stdcall GetSourcePath() const {
    return mstrSourcePath.empty()?NULL:mstrSourcePath.Chars();
  }

  ///////////////////////////////////////////////
  tFileFlags __stdcall GetFileFlags() const {
    return eFileFlags_Read|eFileFlags_Write;
  }

  ///////////////////////////////////////////////
  tPtr __stdcall GetBase() const {
    return mpBase;
  }

  ///////////////////////////////////////////////
  tPtr __stdcall GetHere() const {
    return mpHere;
  }

  ///////////////////////////////////////////////
  tPtr __stdcall GetStop() const {
    return mpStop;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetMemPtr(tPtr pMem, tSize nSize, tBool bFree, tBool bKeepHere) {
    tU8* pOldBase = mpBase;
    mpBase = pMem;
    mpStop = mpBase + nSize;
    if (bKeepHere) {
      mpHere = ((tU8*)pMem) + (mpHere-pOldBase);
      if (mpHere < mpBase) { mpHere = mpBase; }
      if (mpHere > mpStop) { mpHere = mpStop; }
    }
    else {
      mpHere = mpBase;
    }
    mbFree = bFree;
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall Reset()  {
  }

  inline tBool __stdcall SeekEnd(tI64 offset) { return SeekSet((offset<GetSize())?(GetSize()-offset):0); }
  inline tBool __stdcall Flush()  { return eTrue; }
  inline tBool __stdcall GetTime(eFileTime aFileTime, iTime* apTime) const { niUnused(aFileTime);niUnused(apTime); return eFalse; }
  inline tBool __stdcall SetTime(eFileTime aFileTime, const iTime* apTime) { niUnused(aFileTime);niUnused(apTime); return eFalse; }
  inline tBool __stdcall Resize(tI64) { return eFalse; }

 protected:
  cString mstrSourcePath;
  tBool mbFree;
  tPtr  mpBase; // First byte of the memory
  tPtr  mpHere; // Current position in the stream
  tPtr  mpStop; // Last byte of the memory
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! File interface that grow when you write outside his orginal size.
class cFileDynamicMem : public ImplRC<iFileBase,eImplFlags_Default,iFileMemory>
{
 public:
  cFileDynamicMem(tSize anSize = 0, const achar* aszPath = NULL)
  {
    mstrSourcePath = aszPath;
    if (anSize > 0) {
      mvData.resize(anSize);
    }
    _UpdateMemPtr(NULL,0);
    mnBaseSize = ni::Min(32,anSize);
  }

  ///////////////////////////////////////////////
  tBool __stdcall IsOK() const {
    return eTrue;
  }

  ///////////////////////////////////////////////
  tSize __stdcall ReadRaw(void* pOut, tSize nSize) {
    if (mpHere >= mpStop) {
      mpHere = mpStop+1;
      return 0;
    }

    if (mpHere+nSize > mpStop) {
      tSize size = (mpStop-mpHere);
      memcpy(pOut, mpHere, size);
      mpHere = mpStop;
      return size;
    }
    else {
      memcpy(pOut, mpHere, nSize);
      mpHere += nSize;
      return nSize;
    }
  }

  ///////////////////////////////////////////////
  tSize __stdcall WriteRaw(const void* pIn, tSize nSize) {
    if (this->mpHere+nSize > this->mpStop) {
      tSize newsize = this->mpHere-this->mpBase + nSize;
      if (newsize >= this->mvData.size()) {
        this->mvData.resize(
            ni::Max(tSize(newsize),tSize(this->mvData.size()+this->mnBaseSize)));
      }
      _UpdateMemPtr(&this->mvData[0], newsize);
    }
    memcpy(this->mpHere, pIn, nSize);
    this->mpHere += nSize;
    return nSize;
  }

  ///////////////////////////////////////////////
  void __stdcall Reset()  {
    this->mvData.clear();
    _UpdateMemPtr(NULL,0);
  }
  tBool __stdcall Resize(tI64 anNewSize) {
    niAssert(anNewSize < knMaxSensibleFileMemSize);
    niCheck(anNewSize < knMaxSensibleFileMemSize,eFalse);
    this->mvData.resize((tI32)anNewSize);
    _UpdateMemPtr(&this->mvData[0], this->mvData.size());
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall SetMemPtr(tPtr apMem, tSize anSize, tBool abFree, tBool abKeepHere) {
    niUnused(apMem);
    niUnused(anSize);
    niUnused(abFree);
    niUnused(abKeepHere);
    return eFalse;
  }

  ///////////////////////////////////////////////
  tI64  __stdcall Tell() {
    return (mpHere - mpBase);
  }
  tI64  __stdcall GetSize() const {
    return (mpStop - mpBase);
  }

  ///////////////////////////////////////////////
  const achar* __stdcall GetSourcePath() const {
    return mstrSourcePath.empty()?NULL:mstrSourcePath.Chars();
  }

  ///////////////////////////////////////////////
  tFileFlags __stdcall GetFileFlags() const {
    return eFileFlags_Read|eFileFlags_Write;
  }

  ///////////////////////////////////////////////
  tPtr __stdcall GetBase() const {
    return mpBase;
  }
  tPtr __stdcall GetHere() const {
    return mpHere;
  }
  tPtr __stdcall GetStop() const {
    return mpStop;
  }

  ///////////////////////////////////////////////
  tBool __stdcall Seek(tI64 offset) {
    tU8* pNewPos = mpHere + offset;
    if (pNewPos < mpBase) { pNewPos = mpBase; }
    if (pNewPos > mpStop) { pNewPos = mpStop; }
    mpHere = pNewPos;
    return eTrue;
  }
  tBool __stdcall SeekSet(tI64 offset) {
    tU8* pNewPos = mpBase + offset;
    if (pNewPos < mpBase) { pNewPos = mpBase; }
    if (pNewPos > mpStop) { pNewPos = mpStop; }
    mpHere = pNewPos;
    return eTrue;
  }
  inline tBool __stdcall SeekEnd(tI64 offset) {
    return this->SeekSet((offset<this->GetSize())?(this->GetSize()-offset):0);
  }

  ///////////////////////////////////////////////
  inline tBool __stdcall Flush()  { return eTrue; }
  inline tBool __stdcall GetTime(eFileTime aFileTime, iTime* apTime) const { niUnused(aFileTime);niUnused(apTime); return eFalse; }
  inline tBool __stdcall SetTime(eFileTime aFileTime, const iTime* apTime) { niUnused(aFileTime);niUnused(apTime); return eFalse; }

 private:
  inline tBool __stdcall _UpdateMemPtr(tPtr pMem, tSize nSize) {
    tU8* pOldBase = mpBase;
    mpBase = pMem;
    mpStop = mpBase + nSize;
    mpHere = ((tU8*)pMem) + (mpHere-pOldBase);
    if (mpHere < mpBase) { mpHere = mpBase; }
    if (mpHere > mpStop) { mpHere = mpStop; }
    return eTrue;
  }

  cString mstrSourcePath;
  tSize       mnBaseSize;
  astl::vector<tU8> mvData;
  tPtr  mpBase; // First byte of the memory
  tPtr  mpHere; // Current position in the stream
  tPtr  mpStop; // Last byte of the memory
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! File window
class cFileWindow : public ImplRC<iFileBase,eImplFlags_Default>
{
 public:
  ///////////////////////////////////////////////
  cFileWindow(iFileBase* apBase, tI64 anBase, tI64 anSize, const achar* aaszPath, tBool abAutoSeekSet)
      : mbAutoSeekSet(abAutoSeekSet)
  {
    niCheckSilent(niIsOK(apBase),;);

    ZeroMembers();

    if (!aaszPath) {
      mstrSourcePath = apBase->GetSourcePath();
    }
    else {
      mstrSourcePath = aaszPath;
    }

    if (anSize == 0) {
      anSize = apBase->GetSize();
      if (anBase > anSize)
        return;
      anSize -= anBase;
    }

    mnBase = anBase;
    mnStop = mnBase + anSize;
    mnHere = mnBase;
    mptrBase = apBase;
    if (!mbAutoSeekSet) {
      mptrBase->SeekSet(mnBase);
    }
  }

  ///////////////////////////////////////////////
  ~cFileWindow() {
    Invalidate();
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
  }

  ///////////////////////////////////////////////
  void SetSourcePath(const achar* aszPath) {
    if (!aszPath)
      mstrSourcePath.Clear();
    else
      mstrSourcePath = aszPath;
  }

  ///////////////////////////////////////////////
  void  ZeroMembers()  {
    mnBase = 0;
    mnHere = 0;
    mnStop = 0;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall IsOK() const {
    return mptrBase.IsOK();
  }

  ///////////////////////////////////////////////
  tBool __stdcall Seek(tI64 offset) {
    return SeekSet((mnHere-mnBase) + offset);
  }
  tBool __stdcall SeekSet(tI64 offset) {
    tI64 nNewPos = mnBase + offset;
    if (nNewPos < mnBase) { nNewPos = mnBase; }
    if (nNewPos > mnStop) { nNewPos = mnStop; }
    mnHere = nNewPos;
    if (mbAutoSeekSet) {
      return eTrue;
    }
    else {
      return mptrBase->SeekSet(mnHere);
    }
  }

  ///////////////////////////////////////////////
  tSize __stdcall ReadRaw(void* pOut, tSize nSize) {
    if (mnHere >= mnStop) {
      mnHere = mnStop+1;
      return 0;
    }

    tSize readSize;
    tBool goToEof = eFalse;
    if (((tI64)(mnHere+nSize)) > mnStop) {
      tSize size = (tSize)(mnStop-mnHere);
      if (mbAutoSeekSet) {
        tI64 lPos = mptrBase->Tell();
        mptrBase->SeekSet(mnHere);
        readSize = mptrBase->ReadRaw(pOut,size);
        mptrBase->SeekSet(lPos);
      }
      else {
        readSize = mptrBase->ReadRaw(pOut,size);
      }
      goToEof = eTrue;
    }
    else {
      if (mbAutoSeekSet) {
        tI64 lPos = mptrBase->Tell();
        mptrBase->SeekSet(mnHere);
        readSize = mptrBase->ReadRaw(pOut,nSize);
        mptrBase->SeekSet(lPos);
      }
      else {
        readSize = mptrBase->ReadRaw(pOut,nSize);
      }
      if (readSize != nSize) goToEof = eTrue;
      else      mnHere += nSize;

    }

    if (goToEof) {
      mnHere = mnStop+1;
    }
    return readSize;
  }

  ///////////////////////////////////////////////
  virtual tSize __stdcall WriteRaw(const void* apIn, tSize nSize) {
    if (mnHere >= mnStop) {
      mnHere = mnStop+1;
      return 0;
    }

    tSize writeSize;
    tBool goToEof = eFalse;
    if (((tI64)(mnHere+nSize)) > mnStop) {
      tSize size = (tSize)(mnStop-mnHere);
      if (mbAutoSeekSet) {
        tI64 lPos = mptrBase->Tell();
        mptrBase->SeekSet(mnHere);
        writeSize = mptrBase->WriteRaw(apIn,size);
        mptrBase->SeekSet(lPos);
      }
      else {
        writeSize = mptrBase->WriteRaw(apIn,size);
      }
      goToEof = eTrue;
    }
    else {
      if (mbAutoSeekSet) {
        tI64 lPos = mptrBase->Tell();
        mptrBase->SeekSet(mnHere);
        writeSize = mptrBase->WriteRaw(apIn,nSize);
        mptrBase->SeekSet(lPos);
      }
      else {
        writeSize = mptrBase->WriteRaw(apIn,nSize);
      }
      if (writeSize != nSize) goToEof = eTrue;
      else      mnHere += nSize;
    }

    if (goToEof) {
      mnHere = mnStop+1;
    }
    return writeSize;
  }

  ///////////////////////////////////////////////
  tI64  __stdcall Tell() {
    return (mnHere - mnBase);
  }

  ///////////////////////////////////////////////
  tI64  __stdcall GetSize() const {
    return (mnStop - mnBase);
  }

  ///////////////////////////////////////////////
  const achar* __stdcall GetSourcePath() const {
    return mstrSourcePath.Chars();
  }

  ///////////////////////////////////////////////
  tFileFlags __stdcall GetFileFlags() const {
    return mptrBase->GetFileFlags();
  }

  inline tBool __stdcall SeekEnd(tI64 offset) { return SeekSet((offset<GetSize())?(GetSize()-offset):0); }
  inline tBool __stdcall Flush()  { return eTrue; }
  inline tBool __stdcall GetTime(eFileTime aFileTime, iTime* apTime) const { niUnused(aFileTime);niUnused(apTime); return eFalse; }
  inline tBool __stdcall SetTime(eFileTime aFileTime, const iTime* apTime) { niUnused(aFileTime);niUnused(apTime); return eFalse; }
  inline tBool __stdcall Resize(tI64) { return eFalse; }

 protected:
  const   tBool    mbAutoSeekSet;
  Ptr<iFileBase> mptrBase;
  cString mstrSourcePath;
  tI64  mnBase; // First byte of the memory
  tI64  mnHere; // Current position in the stream
  tI64  mnStop; // Last byte of the memory
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! File chain
class cFileChain : public ImplRC<iFileBase,eImplFlags_Default>
{
 public:
  ///////////////////////////////////////////////
  cFileChain(tI64 anBase, const achar* aaszPath) {
    ZeroMembers();
    mstrSourcePath = aaszPath;
    mnBase = anBase;
    mnStop = mnBase;
    mnHere = mnBase;
  }

  ///////////////////////////////////////////////
  ~cFileChain() {
    Invalidate();
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
  }

  ///////////////////////////////////////////////
  void SetSourcePath(const achar* aszPath) {
    if (!aszPath)
      mstrSourcePath.Clear();
    else
      mstrSourcePath = aszPath;
  }

  ///////////////////////////////////////////////
  void  ZeroMembers()  {
    mnBase = 0;
    mnHere = 0;
    mnStop = 0;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall IsOK() const {
    return ni::eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall Seek(tI64 offset) {
    return SeekSet((mnHere-mnBase) + offset);
  }
  tBool __stdcall SeekSet(tI64 offset) {
    tI64 nNewPos = mnBase + offset;
    if (nNewPos < mnBase) { nNewPos = mnBase; }
    if (nNewPos > mnStop) { nNewPos = mnStop; }
    mnHere = nNewPos;
    tU32 link = GetLinkIndex(mnHere);
    if (link == eInvalidHandle) return ni::eFalse;
    return mvLinks[link].mptrFile->SeekSet(mnHere-mvLinks[link].mnStart);
  }

  ///////////////////////////////////////////////
  tSize __stdcall ReadRaw(void* pOut, tSize nSize) {
    if (mnHere >= mnStop) {
      mnHere = mnStop+1;
      return 0;
    }

    tU32 i = GetLinkIndex(mnHere);
    tSize readSize = 0;
    while (i < mvLinks.size()) {
      mvLinks[i].mptrFile->SeekSet(mnHere-mvLinks[i].mnStart);
      tSize read = mvLinks[i].mptrFile->ReadRaw(pOut,nSize-readSize);
      readSize += read;
      mnHere += read;
      if (!read || readSize == nSize) {
        break;
      }
      ++i; // not read fully in the current link, go to the next one
    }

    if (readSize != nSize) mnHere = mnStop+1;
    return readSize;
  }

  ///////////////////////////////////////////////
  virtual tSize __stdcall WriteRaw(const void* apIn, tSize nSize) {
    return 0;
  }

  ///////////////////////////////////////////////
  tI64  __stdcall Tell() {
    return (mnHere - mnBase);
  }

  ///////////////////////////////////////////////
  tI64  __stdcall GetSize() const {
    return (mnStop - mnBase);
  }

  ///////////////////////////////////////////////
  const achar* __stdcall GetSourcePath() const {
    return mstrSourcePath.Chars();
  }

  ///////////////////////////////////////////////
  tFileFlags __stdcall GetFileFlags() const {
    return eFileFlags_Read;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall AppendLink(iFile* apFile, tI64 anSize) {
    niCheckSilent(niIsOK(apFile),eFalse);
    sLink link;
    link.mptrFile = apFile;
    link.mnStart = (mvLinks.empty()) ? mnBase : mvLinks.back().mnEnd;
    if (anSize <= 0) anSize = apFile->GetSize();
    link.mnEnd = link.mnStart+anSize;
    mnStop = link.mnEnd;
    mvLinks.push_back(link);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tU32 __stdcall GetLinkIndex(tI64 anOffset) {
    niLoop(i,mvLinks.size()) {
      const sLink& l = mvLinks[i];
      if (anOffset >= l.mnStart && anOffset < l.mnEnd)
        return i;
    }
    return eInvalidHandle;
  }

  inline tBool __stdcall SeekEnd(tI64 offset) { return SeekSet((offset<GetSize())?(GetSize()-offset):0); }
  inline tBool __stdcall Flush()  { return eTrue; }
  inline tBool __stdcall GetTime(eFileTime aFileTime, iTime* apTime) const { niUnused(aFileTime);niUnused(apTime); return eFalse; }
  inline tBool __stdcall SetTime(eFileTime aFileTime, const iTime* apTime) { niUnused(aFileTime);niUnused(apTime); return eFalse; }
  inline tBool __stdcall Resize(tI64) { return eFalse; }

 protected:
  struct sLink {
    Ptr<iFile> mptrFile;
    tI64             mnStart;
    tI64             mnEnd;
  };
  astl::vector<sLink> mvLinks;
  cString mstrSourcePath;
  tI64  mnBase; // First byte of the memory
  tI64  mnHere; // Current position in the stream
  tI64  mnStop; // Last byte of the memory
};

namespace ni {

/////////////////////////////////////////////////////////////////
niExportFunc(iFile*) CreateFile(iFileBase* apBase) {
  niCheckIsOK(apBase,NULL);
  return niNew cFileImpl(apBase);
}

/////////////////////////////////////////////////////////////////
niExportFunc(iFileBase*) CreateFileBaseWriteDummy() {
  return niNew cFileWriteDummy(0);
}
niExportFunc(iFile*) CreateFileWriteDummy() {
  return niNew cFileImpl(CreateFileBaseWriteDummy());
}

/////////////////////////////////////////////////////////////////
niExportFunc(iFileBase*) CreateFileBaseMemory(tPtr pMem, tSize nSize, tBool bFree, const achar* aszPath) {
  return niNew cFileMem(pMem, nSize, bFree, aszPath);
}
niExportFunc(iFile*) CreateFileMemory(tPtr pMem, tSize nSize, tBool bFree, const achar* aszPath) {
  return niNew cFileImpl(CreateFileBaseMemory(pMem, nSize, bFree, aszPath));
}

/////////////////////////////////////////////////////////////////
niExportFunc(iFileBase*) CreateFileBaseMemoryAlloc(tSize nSize, const achar* aszPath) {
  niAssert(nSize < knMaxSensibleFileMemSize);
  niCheck(nSize < knMaxSensibleFileMemSize,NULL);
  return niNew cFileMem(nSize, aszPath);
}
niExportFunc(iFile*) CreateFileMemoryAlloc(tSize nSize, const achar* aszPath) {
  niAssert(nSize < knMaxSensibleFileMemSize);
  niCheck(nSize < knMaxSensibleFileMemSize,NULL);
  return niNew cFileImpl(CreateFileBaseMemoryAlloc(nSize, aszPath));
}

/////////////////////////////////////////////////////////////////
niExportFunc(iFileBase*) CreateFileBaseDynamicMemory(tSize anSize, const achar* aszPath) {
  niAssert(anSize < knMaxSensibleFileMemSize);
  niCheck(anSize < knMaxSensibleFileMemSize,NULL);
  return niNew cFileDynamicMem(anSize, aszPath);
}
niExportFunc(iFile*) CreateFileDynamicMemory(tSize anSize, const achar* aszPath) {
  niAssert(anSize < knMaxSensibleFileMemSize);
  niCheck(anSize < knMaxSensibleFileMemSize,NULL);
  return niNew cFileImpl(CreateFileBaseDynamicMemory(anSize, aszPath));
}

/////////////////////////////////////////////////////////////////
niExportFunc(iFileBase*) CreateFileBaseWindow(iFileBase* apBase, tI64 anBase, tI64 anSize, const achar* aaszPath, tBool abAutoSeekSet) {
  niCheckIsOK(apBase,NULL);
  return niNew cFileWindow(apBase,anBase,anSize,aaszPath,abAutoSeekSet);
}
niExportFunc(iFile*) CreateFileWindow(iFileBase* apBase, tI64 anBase, tI64 anSize, const achar* aaszPath, tBool abAutoSeekSet) {
  niCheckIsOK(apBase,NULL);
  return ni::CreateFile(CreateFileBaseWindow(apBase,anBase,anSize,aaszPath,abAutoSeekSet));
}

}
