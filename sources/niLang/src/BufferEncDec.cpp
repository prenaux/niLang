// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Types.h"
#include "API/niLang/IFile.h"
#include "API/niLang/Utils/UnknownImpl.h"
#include "API/niLang/StringDef.h"

using namespace ni;

//! File wrapper around a buffer encoder.
struct FileBufferEncoder : public ImplRC<iFileBase>
{
  FileBufferEncoder(iFileBase* apFile, iBufferEncoder* apEncoder, tSize anBufferSize) {
    niAssert(niIsOK(apFile));
    niAssert(niIsOK(apEncoder));
    _fp = apFile;
    _encoder = apEncoder;
    _encoderMarker = _encoder->EncodeMarker();
    _fpBuffer = ni::CreateFileBaseDynamicMemory(anBufferSize,AZEROSTR);
    _fpBufferMem = ni::QueryInterface<iFileMemory>(_fpBuffer);
    _bufferSize = anBufferSize;
  }
  ~FileBufferEncoder() {
    Invalidate();
  }

  void __stdcall Invalidate() {
    Flush();
    _fp = NULL;
    _encoder = NULL;
    _fpBuffer = NULL;
  }

  tBool __stdcall IsOK() const {
    return _fp.IsOK() && _encoder.IsOK();
  }

  tSize __stdcall WriteRaw(const void* apData, tSize anSize) {
    if (!_fp.IsOK())
      return 0;
    tSize r = _fpBuffer->WriteRaw(apData,anSize);
    if (_fpBuffer->GetSize() >= (tI64)_bufferSize) {
      if (!_FlushBufferToFile())
        return 0;
    }
    return r;
  }

  tBool _FlushBufferToFile() {
    if (!_fp.IsOK())
      return eFalse;
    tU32 bufSize = (tU32)_fpBuffer->GetSize();
    if (!bufSize) return eFalse;
    tU32 outputSize = (tU32)_encoder->EncodeMaxDestSize(bufSize);
    _outputBuffer.resize(outputSize);
    outputSize = (tU32)_encoder->EncodeBuffer(&_outputBuffer[0],outputSize,
                                              _fpBufferMem->GetBase(),bufSize);
    if (!outputSize) return eFalse;
    tU32 t;
    t = niSwapLE32(_encoderMarker); _fp->WriteRaw(&t,4);
    t = niSwapLE32(bufSize); _fp->WriteRaw(&t,4);
    t = niSwapLE32(outputSize); _fp->WriteRaw(&t,4);
    _fp->WriteRaw(&_outputBuffer[0],outputSize);
    _fpBufferMem->Reset();
    //    niDebugFmt((_A("[%s] Flushed %d on %d bytes, %d -> %d\n"),
    //          _fp->GetSourcePath(),
    //          outputSize,
    //          _fp->Tell(),
    //          bufSize,
    //          outputSize
    //      ));
    return eTrue;
  }

  virtual tFileFlags __stdcall GetFileFlags() const {
    return eFileFlags_Write|eFileFlags_Encoder;
  }
  virtual const achar* __stdcall GetSourcePath() const {
    return _fp.IsOK() ? _fp->GetSourcePath() : AZEROSTR;
  }
  virtual tI64 __stdcall Tell() {
    return _fp.IsOK() ? _fp->Tell() : 0;
  }
  virtual tI64 __stdcall GetSize() const {
    return _fp.IsOK() ? _fp->GetSize() : 0;
  }
  virtual tBool __stdcall Seek(tI64 offset) {
    return _fp.IsOK() ? _fp->Seek(offset) : eFalse;
  }
  virtual tBool __stdcall SeekSet(tI64 offset) {
    return _fp.IsOK() ? _fp->SeekSet(offset) : eFalse;
  }
  virtual tBool __stdcall SeekEnd(tI64 offset) {
    return _fp.IsOK() ? _fp->SeekEnd(offset) : eFalse;
  }
  inline tBool __stdcall Flush()  {
    if (!_fp.IsOK()) return eFalse;
    _FlushBufferToFile();
    return _fp->Flush();
  }

  inline tSize __stdcall ReadRaw(void* pOut, tSize nSize) {
    return 0;
  }
  inline tBool __stdcall GetTime(eFileTime aFileTime, iTime* apTime) const {
    niUnused(aFileTime);niUnused(apTime);
    return eFalse;
  }
  inline tBool __stdcall SetTime(eFileTime aFileTime, const iTime* apTime) {
    niUnused(aFileTime);niUnused(apTime);
    return eFalse;
  }
  inline tBool __stdcall Resize(tI64) { return eFalse; }

  Ptr<iFileBase>      _fp;
  Ptr<iBufferEncoder> _encoder;
  tU32                _encoderMarker;
  Ptr<iFileBase>      _fpBuffer;
  Ptr<iFileMemory>    _fpBufferMem;
  tSize               _bufferSize;
  astl::vector<tU8>   _outputBuffer;
};

//! File wrapper around a buffer decoder.
struct FileBufferDecoder : public ImplRC<iFileBase>
{
  static const tU32 MAX_VALID_PACKED_SIZE = 1024*1024*16;
  static const tU32 MAX_VALID_UNPACKED_SIZE = 1024*1024*256;

  FileBufferDecoder(iFileBase* apFile, iBufferDecoder* apDecoder, tSize aDecodedSize) {
    niAssert(niIsOK(apFile));
    niAssert(niIsOK(apDecoder));
    _basePos = apFile->Tell();
    _size = aDecodedSize;
    _fpSrc = apFile;
    _decoder = apDecoder;
    _decoderMarker = _decoder->DecodeMarker();
    _ResetPos();
  }

  void __stdcall _ResetPos() {
    _pos = 0;
    _cur = 0;
    _buffer.clear();
    _fpSrc->SeekSet(_basePos);
  }

  tSize __stdcall ReadRaw(void* apData, tSize anSize) {
    tSize r = 0;
    tSize toread = 0;
    tPtr d = (tPtr)apData;
    while (1) {
      if (_cur < (tI64)_buffer.size()) {
        toread = ni::Min(_buffer.size()-_cur,anSize-r);
        if (toread) {
          if (apData) // allow NULL destination, for seeking
            ni::MemCopy(d,&_buffer[_cur],toread);
          _cur += toread;
          _pos += toread;
          r += toread;
          d += toread;
        }
        if (r >= anSize)
          break;
      }
      tSize read = _ReadBufferFromFile();
      if (!read)
        break;
    }
    return r;
  }

  tSize _ReadBufferFromFile() {
    tU32 marker = 0;
    _fpSrc->ReadRaw(&marker,4);
    marker = niSwapLE32(marker);
    if (marker != _decoderMarker)
      // No error here since we could be simply in a part of the
      // base file where data haven't been encoded.
      return 0;

    tU32 unpackedSize = 0;
    _fpSrc->ReadRaw(&unpackedSize,4);
    unpackedSize = niSwapLE32(unpackedSize);
    niCheck(unpackedSize < MAX_VALID_UNPACKED_SIZE,0);

    tU32 packedSize = 0;
    _fpSrc->ReadRaw(&packedSize,4);
    packedSize = niSwapLE32(packedSize);
    niCheck(packedSize < MAX_VALID_PACKED_SIZE,0);

    _cur = 0;
    _packedBuffer.resize(packedSize);
    _buffer.resize(_decoder->DecodeMaxDestSize(unpackedSize));
    //    niDebugFmt((_A("[%s] Read %d -> %d"),_fpSrc->GetSourcePath(),packedSize,unpackedSize));
    tSize read = _fpSrc->ReadRaw(&_packedBuffer[0],packedSize);
    if (read != packedSize) {
      niWarning(niFmt(_A("Can't read complete packed buffer, read %d on %d."),
                        read,packedSize));
      return 0;
    }
    tSize d = _decoder->DecodeBuffer(&_buffer[0],_buffer.size(),&_packedBuffer[0],packedSize);
    _buffer.resize(d);
    return d;
  }

  virtual tFileFlags __stdcall GetFileFlags() const {
    return eFileFlags_Read|eFileFlags_Encoder;
  }
  virtual const achar* __stdcall GetSourcePath() const {
    return _fpSrc->GetSourcePath();
  }
  virtual tI64 __stdcall GetSize() const {
    if (!_size) {
#ifdef niDebug
      niWarning("Reading the whole file to determin its size.");
#endif
      tU8 t[4096];
      const tI64 nPos = _pos;
      niThis(FileBufferDecoder)->SeekSet(0);
      while (1) {
        tSize r = niThis(FileBufferDecoder)->ReadRaw(t,sizeof(t));
        if (r == 0) break;
        niThis(FileBufferDecoder)->_size += r;
      }
      niThis(FileBufferDecoder)->SeekSet(nPos);
    }
    return _size;
  }
  virtual tBool __stdcall Seek(tI64 offset) {
    if (!offset) return eTrue;
    if (offset < 0) {
      return SeekSet(_pos+offset);
    }
    else {
      return (tI64)ReadRaw(NULL,(tSize)offset) == offset;
    }
  }
  virtual tBool __stdcall SeekSet(tI64 offset) {
    offset = ni::Max(0,offset);
    if (offset == _pos)
      return eTrue; // already at the requested position
    if (offset > _pos) {
      // seek forward...
      return Seek(offset-_pos);
    }
    else {
      tI64 diff = _pos-offset;
      if (diff <= _cur) {
        _cur -= diff;
        _pos -= diff;
        return eTrue;
      }
      else
      {
        // seek backward, slow...
#ifdef _DEBUG
        niWarning("Full Backward Seek is slow....");
#endif
        _ResetPos();
        return Seek(offset);
      }
    }
  }
  inline tBool __stdcall SeekEnd(tI64 offset) {
    tI64 size = GetSize();
    return SeekSet((offset<size)?(size-offset):0);
  }

  virtual tSize __stdcall WriteRaw(const void* apIn, tSize nSize) {
    niAssert(0 && "Not Supported.");
    return 0;
  }
  virtual tI64 __stdcall Tell() {
    return _pos;
  }
  inline tBool __stdcall Flush()  { return eTrue; }
  inline tBool __stdcall GetTime(eFileTime aFileTime, iTime* apTime) const {
    niUnused(aFileTime);niUnused(apTime); return eFalse;
  }
  inline tBool __stdcall SetTime(eFileTime aFileTime, const iTime* apTime) {
    niUnused(aFileTime);niUnused(apTime); return eFalse;
  }
  inline tBool __stdcall Resize(tI64) { return eFalse; }

  Ptr<iFileBase>      _fpSrc;
  Ptr<iBufferDecoder> _decoder;
  tU32                _decoderMarker;
  astl::vector<tU8>   _packedBuffer;
  astl::vector<tU8>   _buffer;
  tI64                _pos;
  tI64                _cur;
  tI64                _size;
  tI64                _basePos;
};

namespace ni {

static const ni::tU32 _knEncoderBufferSize = 0xFFFF;

niExportFunc(iFileBase*) CreateFileBaseBufferEncoder(iFileBase* apBase, iBufferEncoder* apEnc)
{
  niCheck(niIsOK(apBase),NULL);
  Ptr<iBufferEncoder> enc = apEnc;
  niCheck(enc.IsOK(),NULL);
  Ptr<iFileBase> fpBase = niNew FileBufferEncoder(apBase,enc,_knEncoderBufferSize);
  return fpBase.GetRawAndSetNull();
}

niExportFunc(iFileBase*) CreateFileBaseBufferDecoder(iFileBase* apBase, iBufferDecoder* apDec, tSize aDecSize)
{
  niCheck(niIsOK(apBase),NULL);
  Ptr<iBufferDecoder> dec = apDec;
  niCheck(dec.IsOK(),NULL);
  Ptr<iFileBase> fpBase = niNew FileBufferDecoder(apBase,dec,aDecSize);
  return fpBase.GetRawAndSetNull();
}

niExportFunc(iFile*) CreateFileBufferEncoder(iFileBase* apBase, iBufferEncoder* apEnc)
{
  niCheck(niIsOK(apBase),NULL);
  Ptr<iBufferEncoder> enc = apEnc;
  niCheck(enc.IsOK(),NULL);
  Ptr<iFileBase> fpBase = CreateFileBaseBufferEncoder(apBase,enc);
  return ni::CreateFile(fpBase);
}

niExportFunc(iFile*) CreateFileBufferDecoder(iFileBase* apBase, iBufferDecoder* apDec, tSize aDecSize)
{
  niCheck(niIsOK(apBase),NULL);
  Ptr<iBufferDecoder> dec = apDec;
  niCheck(dec.IsOK(),NULL);
  Ptr<iFileBase> fpBase = CreateFileBaseBufferDecoder(apBase,dec,aDecSize);
  return ni::CreateFile(fpBase);
}

niExportFunc(tSize) FileBufferEncode(iBufferEncoder* apEnc, iFile* apSrc, tSize anSrcSize, iFile* apDest)
{
  if (anSrcSize == 0) return 0;
  niCheckIsOK(apEnc,0);
  niCheckIsOK(apSrc,0);
  niCheckIsOK(apDest,0);
  Ptr<iFile> enc = CreateFileBufferEncoder(apDest->GetFileBase(),apEnc);
  return (tSize)enc->WriteFile(apSrc->GetFileBase(),anSrcSize);
}

niExportFunc(tSize) FileBufferDecode(iBufferDecoder* apDec, iFile* apSrc, tSize anSrcSize, iFile* apDest, tSize anDestSize)
{
  if (anSrcSize == 0) return 0;
  niCheckIsOK(apDec,0);
  niCheckIsOK(apSrc,0);
  niCheckIsOK(apDest,0);
  Ptr<iFileBase> dec = niNew FileBufferDecoder(apSrc->GetFileBase(),apDec,anSrcSize);
  return (tSize)apDest->WriteFile(dec,anSrcSize);
}

}
