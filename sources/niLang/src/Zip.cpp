// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Types.h"
#include "API/niLang/IFile.h"
#include "API/niLang/StringDef.h"
#include "API/niLang/Utils/QPtr.h"

// #define TRACE_URLHANDLER_MANIFEST(X) niDebugFmt(X)
#include "API/niLang/Utils/URLFileHandlerManifest.h"

#include "Zip.h"
#include "Zip_Unzip.h"
#include "zlib/zlib.h"

#if niMinFeatures(15) && !defined niEmbedded
#include "Zip_GZipFile.h"
#include "Zip_File.h"
#include "Zip_Arch.h"
#endif

//--------------------------------------------------------------------------------------------
//
//  Main
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
cZip::cZip()
{
}

///////////////////////////////////////////////
cZip::~cZip()
{
}

///////////////////////////////////////////////
tBool cZip::IsOK() const
{
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cZip::ZipUncompressBuffer(tPtr apDest, tU32 anDestSize, tPtr apSrc, tU32 anSrcSize)
{
  uLongf destSize = anDestSize;
  int err = uncompress(apDest, &destSize, apSrc, anSrcSize);
  if (err != Z_OK) {
    niError(niFmt(_A("Uncompression error : %d."),err));
    return eFalse;
  }
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cZip::ZipUncompressBufferInFile(iFile* apDest, tU32 anDestSize, tPtr apSrc, tU32 anSrcSize)
{
  tU8* pDestBuffer = (tU8*)niMalloc(anDestSize);
  if (!pDestBuffer) {
    niError(_A("Can't allocate dest buffer."));
    return eFalse;
  }
  uLongf destSize = anDestSize;
  int err = uncompress(pDestBuffer, &destSize, apSrc, anSrcSize);
  if (err != Z_OK) {
    niFree(pDestBuffer);
    niError(niFmt(_A("Uncompression error : %d."),err));
    return eFalse;
  }
  apDest->WriteRaw((void*)pDestBuffer,destSize);
  niFree(pDestBuffer);
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cZip::ZipUncompressFile(iFile* apDest, tU32 anDestSize, iFile* apSrc, tU32 anSrcSize)
{
  tU8* pDestBuffer = (tU8*)niMalloc(anDestSize);
  if (!pDestBuffer) {
    niError(_A("Can't allocate dest buffer."));
    return eFalse;
  }
  tU8* pSrcBuffer = (tU8*)niMalloc(anSrcSize);
  if (!pSrcBuffer) {
    niFree(pDestBuffer);
    niError(_A("Can't allocate source buffer."));
    return eFalse;
  }
  apSrc->ReadRaw((void*)pSrcBuffer,anSrcSize);
  uLongf destSize = anDestSize;
  int err = uncompress(pDestBuffer, &destSize, pSrcBuffer, anSrcSize);
  if (err != Z_OK) {
    niFree(pDestBuffer);
    niFree(pSrcBuffer);
    niError(niFmt(_A("Uncompression error : %d."),err));
    return eFalse;
  }
  apDest->WriteRaw((void*)pDestBuffer,destSize);
  niFree(pDestBuffer);
  niFree(pSrcBuffer);
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cZip::ZipUncompressFileInBuffer(tPtr apDest, tU32 anDestSize, iFile* apSrc, tU32 anSrcSize)
{
  tU8* pSrcBuffer = (tU8*)niMalloc(anSrcSize);
  if (!pSrcBuffer) {
    niError(_A("Can't allocate source buffer."));
    return eFalse;
  }
  if (apSrc->ReadRaw((void*)pSrcBuffer,anSrcSize) != anSrcSize) {
    niFree(pSrcBuffer);
    niError(_A("Can't read source file."));
    return eFalse;
  }
  uLongf destSize = anDestSize;
  int err = uncompress(apDest, &destSize, pSrcBuffer, anSrcSize);
  if (err != Z_OK) {
    niFree(pSrcBuffer);
    niError(niFmt(_A("Uncompression error : %d."),err));
    return eFalse;
  }
  niFree(pSrcBuffer);
  return eTrue;
}

///////////////////////////////////////////////
tU32  __stdcall cZip::ZipCompressBuffer(tPtr apDest, tPtr apSrc, tU32 anSrcSize, tU32 anLevel)
{
  uLongf destLen = compressBound(anSrcSize);
  int err = compress2(apDest, &destLen, apSrc, anSrcSize, anLevel);
  if (err != Z_OK) {
    niError(niFmt(_A("Compression error : %d."),err));
    return eInvalidHandle;
  }
  return destLen;
}

///////////////////////////////////////////////
tU32  __stdcall cZip::ZipCompressBufferInFile(iFile* apDest, tPtr apSrc, tU32 anSrcSize, tU32 anLevel)
{
  uLongf destLen = compressBound(anSrcSize);
  tU8* pDestBuffer = (tU8*)niMalloc(destLen);
  int err = compress2(pDestBuffer, &destLen, apSrc, anSrcSize, anLevel);
  if (err != Z_OK) {
    niFree(pDestBuffer);
    niError(niFmt(_A("Compression error : %d."),err));
    return eInvalidHandle;
  }
  apDest->WriteRaw((void*)pDestBuffer,destLen);
  niFree(pDestBuffer);
  return destLen;
}

///////////////////////////////////////////////
tU32  __stdcall cZip::ZipCompressFile(iFile* apDest, iFile* apSrc, tU32 anSrcSize, tU32 anLevel)
{
  uLongf destLen = compressBound(anSrcSize);
  tU8* pDestBuffer = (tU8*)niMalloc(destLen);
  tU8* pSrcBuffer = (tU8*)niMalloc(anSrcSize);
  apSrc->ReadRaw((void*)pSrcBuffer, anSrcSize);
  int err = compress2(pDestBuffer, &destLen, pSrcBuffer, anSrcSize, anLevel);
  if (err != Z_OK) {
    niFree(pDestBuffer);
    niFree(pSrcBuffer);
    niError(niFmt(_A("Compression error : %d."),err));
    return eInvalidHandle;
  }
  apDest->WriteRaw((void*)pDestBuffer,destLen);
  niFree(pDestBuffer);
  niFree(pSrcBuffer);
  return destLen;
}

///////////////////////////////////////////////
tBool __stdcall cZip::ZipInflateFileInBuffer(tPtr apDest, tU32 anDestSize, iFile* apSrc)
{
  if (apDest == NULL || anDestSize <= 0) {
    niError(_A("Invalid destination buffer."));
    return eFalse;
  }

  if (!niIsOK(apSrc)) {
    niError(_A("Invalid source buffer."));
    return eFalse;
  }

  tI32 err;
  z_stream d_stream; // decompression stream

  d_stream.zalloc = (zlib_alloc_func)0;
  d_stream.zfree = (zlib_free_func)0;
  d_stream.opaque = (voidpf)0;

  d_stream.next_in  = 0;
  d_stream.avail_in = 0;

  d_stream.next_out = (Byte*)apDest;
  d_stream.avail_out = (uInt)anDestSize;

  err = inflateInit(&d_stream);
  if (err != Z_OK) {
    niError(niFmt(_A("inflateInit failed %d."), err));
    return eFalse;
  }

  tBool bRet = eTrue;
  tU8   buf[1];
  while (1) {
    // Fill a one-byte (!) buffer.
    buf[0] = apSrc->Read8();
    d_stream.next_in = (Bytef*)&buf[0];
    d_stream.avail_in = 1;

    err = inflate(&d_stream, Z_SYNC_FLUSH);
    if (err == Z_STREAM_END) break;
    if (err != Z_OK) {
      niError(niFmt(_A("inflate() failed %d."), err));
      bRet = eFalse;
      break;
    }
  }

  err = inflateEnd(&d_stream);
  if (err != Z_OK)
  {
    niError(niFmt(_A("inflateEnd() failed %d."), err));
    bRet = eFalse;
  }

  return bRet;
}

///////////////////////////////////////////////
tHandle cZip::UnzOpen(ni::iFile* apFile)
{
  niCheckIsOK(apFile,eInvalidHandle);
  unzFile uzFile = niUnzip_Open(apFile);
  if (!uzFile)
    return eInvalidHandle;
  return tHandle(uzFile);
}

///////////////////////////////////////////////
tI32 cZip::UnzClose(tHandle file)
{
  return niUnzip_Close(unzFile(file));
}

///////////////////////////////////////////////
tI32 cZip::UnzGetGlobalInfo(tHandle file, unz_global_info *pglobal_info)
{
  return niUnzip_GetGlobalInfo(unzFile(file), pglobal_info);
}

///////////////////////////////////////////////
tI32 cZip::UnzGetGlobalComment(tHandle file, cString& strComment)
{
  char cszComment[AMAX_SIZE];
  tI32 ret = niUnzip_GetGlobalComment(unzFile(file), cszComment, AMAX_SIZE);
  strComment = cszComment;
  return ret;
}

///////////////////////////////////////////////
tI32 cZip::UnzGoToFirstFile(tHandle file)
{
  return niUnzip_GoToFirstFile(unzFile(file));
}

///////////////////////////////////////////////
tI32 cZip::UnzGoToNextFile(tHandle file)
{
  return niUnzip_GoToNextFile(unzFile(file));
}

///////////////////////////////////////////////
tI32 cZip::UnzLocateFile(tHandle file, const achar *szFileName, tBool bCaseSensitivity)
{
  return niUnzip_LocateFile(unzFile(file), cString(szFileName).Chars(), bCaseSensitivity);
}

///////////////////////////////////////////////
tI32 cZip::UnzLocateFileInc(tHandle file, tU32 num, tU32 pos)
{
  return niUnzip_LocateFileMy(unzFile(file), num, pos);
}

///////////////////////////////////////////////
tI32 cZip::UnzGetCurrentFileInfo(tHandle file,
                                 unz_file_info *pfile_info,
                                 cString& strFilename,
                                 cString& strExtraField,
                                 cString& strComment)
{
  char cszFilename[AMAX_SIZE];
  char cszExtraField[AMAX_SIZE];
  char cszComment[AMAX_SIZE];
  tI32 ret = niUnzip_GetCurrentFileInfo(unzFile(file),
                                     pfile_info,
                                     cszFilename,
                                     AMAX_SIZE,
                                     cszExtraField,
                                     AMAX_SIZE,
                                     cszComment,
                                     AMAX_SIZE);
  strFilename = cszFilename;
  strExtraField = cszExtraField;
  strComment = cszComment;
  return ret;
}

///////////////////////////////////////////////
tI32 cZip::UnzOpenCurrentFile(tHandle file, const char* pwd)
{
  return niUnzip_OpenCurrentFile(unzFile(file),pwd);
}

///////////////////////////////////////////////
tI32 cZip::UnzCloseCurrentFile(tHandle file)
{
  return niUnzip_CloseCurrentFile(unzFile(file));
}

///////////////////////////////////////////////
tI32 cZip::UnzReadCurrentFile(tHandle file, tPtr buf, tU32 len)
{
  return niUnzip_ReadCurrentFile(unzFile(file), voidp(buf), len);
}

///////////////////////////////////////////////
tI32 cZip::Unztell(tHandle file)
{
  return niUnzip_tell(unzFile(file));
}

///////////////////////////////////////////////
tI32 cZip::Unzeof(tHandle file)
{
  return niUnzip_eof(unzFile(file));
}

///////////////////////////////////////////////
tI32 cZip::UnzGetLocalExtrafield(tHandle file, tPtr buf, tU32 len)
{
  return niUnzip_GetLocalExtrafield(unzFile(file), voidp(buf), len);
}

///////////////////////////////////////////////
const char * __stdcall cZip::ZlibVersion (void) {
  return ::zlibVersion();
}

///////////////////////////////////////////////
tI32 __stdcall cZip::Deflate(z_streamp strm, tI32 flush) {
  return ::deflate(strm,flush);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::DeflateEnd (z_streamp strm) {
  return ::deflateEnd(strm);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::Inflate (z_streamp strm, tI32 flush) {
  return ::inflate(strm,flush);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::InflateEnd (z_streamp strm) {
  return ::inflateEnd(strm);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::DeflateSetDictionary (z_streamp strm, const tU8 *dictionary, tI32  dictLength) {
  return ::deflateSetDictionary(strm,dictionary,dictLength);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::DeflateCopy (z_streamp dest, z_streamp source) {
  return ::deflateCopy(dest,source);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::DeflateReset (z_streamp strm) {
  return ::deflateReset(strm);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::DeflateParams (z_streamp strm, tI32 level, tI32 strategy) {
  return ::deflateParams(strm,level,strategy);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::DeflateTune (z_streamp strm, tI32 good_length, tI32 max_lazy, tI32 nice_length, tI32 max_chain) {
  return ::deflateTune(strm,good_length,max_lazy,nice_length,max_chain);
}

///////////////////////////////////////////////
tU32 __stdcall cZip::DeflateBound (z_streamp strm, tU32 sourceLen) {
  return ::deflateBound(strm,sourceLen);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::DeflatePrime (z_streamp strm, tI32 bits, tI32 value) {
  return ::deflatePrime(strm,bits,value);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::DeflateSetHeader (z_streamp strm, gz_headerp head) {
  return ::deflateSetHeader(strm,head);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::InflateSetDictionary (z_streamp strm, const tU8 *dictionary, tI32  dictLength) {
  return ::inflateSetDictionary(strm,dictionary,dictLength);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::InflateSync (z_streamp strm) {
  return ::inflateSync(strm);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::InflateCopy (z_streamp dest, z_streamp source) {
  return ::inflateCopy(dest,source);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::InflateReset (z_streamp strm) {
  return ::inflateReset(strm);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::InflatePrime (z_streamp strm, tI32 bits, tI32 value) {
  return ::inflatePrime(strm,bits,value);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::InflateGetHeader (z_streamp strm, gz_headerp head) {
  return ::inflateGetHeader(strm,head);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::InflateBack (z_streamp strm, zlib_in_func aIn, void  *in_desc, zlib_out_func aOut, void  *out_desc) {
  return ::inflateBack(strm,aIn,in_desc,aOut,out_desc);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::InflateBackEnd (z_streamp strm) {
  return ::inflateBackEnd(strm);
}

///////////////////////////////////////////////
tU32 __stdcall cZip::ZlibCompileFlags (void) {
  return ::zlibCompileFlags();
}

///////////////////////////////////////////////
tI32 __stdcall cZip::Compress (tU8 *dest, tU32 *apDestLen, const tU8 *source, tU32 sourceLen) {
  uLongf destLen = apDestLen ? *apDestLen : 0;
  tI32 r = ::compress(dest,&destLen,source,sourceLen);
  if (apDestLen) *apDestLen = destLen;
  return r;
}

///////////////////////////////////////////////
tI32 __stdcall cZip::Compress2 (tU8 *dest, tU32 *apDestLen, const tU8 *source, tU32 sourceLen, tI32 level) {
  uLongf destLen = apDestLen ? *apDestLen : 0;
  tI32 r = ::compress2(dest,&destLen,source,sourceLen,level);
  if (apDestLen) *apDestLen = destLen;
  return r;
}

///////////////////////////////////////////////
tU32 __stdcall cZip::CompressBound (tU32 sourceLen) {
  return ::compressBound(sourceLen);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::Uncompress (tU8 *dest, tU32 *apDestLen, const tU8 *source, tU32 sourceLen) {
  uLongf destLen = apDestLen ? *apDestLen : 0;
  tI32 r = ::uncompress(dest,&destLen,source,sourceLen);
  if (apDestLen) *apDestLen = destLen;
  return r;
}

///////////////////////////////////////////////
tU32 __stdcall cZip::Adler32 (tU32 adler, const tU8 *buf, tI32 len) {
  return ::adler32(adler,buf,len);
}

///////////////////////////////////////////////
tU32 __stdcall cZip::Adler32Combine (tU32 adler1, tU32 adler2, tSize len2) {
  return ::adler32_combine(adler1,adler2,len2);
}

///////////////////////////////////////////////
tU32 __stdcall cZip::Crc32   (tU32 crc, const tU8 *buf, tI32 len) {
  return ::crc32(crc,buf,len);
}

///////////////////////////////////////////////
tU32 __stdcall cZip::Crc32Combine(tU32 crc1, tU32 crc2, tSize len2) {
  return ::crc32_combine(crc1,crc2,len2);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::DeflateInit(z_streamp strm, tI32 level) {
  return ::deflateInit(strm,level);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::InflateInit(z_streamp strm) {
  return ::inflateInit(strm);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::DeflateInit2(z_streamp strm, tI32  level, tI32  method, tI32 windowBits, tI32 memLevel, tI32 strategy) {
  return ::deflateInit2(strm,level,method,windowBits,memLevel,strategy);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::InflateInit2(z_streamp strm, tI32  windowBits) {
  return ::inflateInit2(strm,windowBits);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::InflateBackInit(z_streamp strm, tI32 windowBits, unsigned char  *window) {
  return ::inflateBackInit(strm,windowBits,window);
}

///////////////////////////////////////////////
const char   * __stdcall cZip::ZError(tI32 err) {
  return ::zError(err);
}

///////////////////////////////////////////////
tI32 __stdcall cZip::InflateSyncPoint (z_streamp z) {
  return ::inflateSyncPoint(z);
}

///////////////////////////////////////////////
const tU32 * __stdcall cZip::GetCRCTable(void) {
  return ::get_crc_table();
}

#if niMinFeatures(15)
///////////////////////////////////////////////
iFile* __stdcall cZip::GZipOpen(iFileBase* apFile, tU32 aulCompressionMode)
{
#ifdef niEmbedded
  return NULL;
#else
  Ptr<iFile> ptrFile = ni::CreateFile(niNew cGZipFile(apFile,aulCompressionMode,false));
  if (!niIsOK(ptrFile)) {
    return NULL;
  }
  return ptrFile.GetRawAndSetNull();
#endif
}

///////////////////////////////////////////////
iFile* __stdcall cZip::ZipOpen(iFileBase* apFile, tU32 aulCompressionMode)
{
#ifdef niEmbedded
  return NULL;
#else
  Ptr<iFile> ptrFile = ni::CreateFile(niNew cZipFile(apFile,aulCompressionMode));
  if (!niIsOK(ptrFile)) {
    return NULL;
  }
  return ptrFile.GetRawAndSetNull();
#endif
}
#endif


//--------------------------------------------------------------------------------------------
//
//  Zip encoder
//
//--------------------------------------------------------------------------------------------

//! File buffer Zip marker.
const tU32 kfccFileBufferZipMarker = niFourCC('E','Z','I','P');

//! Zip buffer encoder.
struct ZipBufferEncoder : public ImplRC<iBufferEncoder>
{
  ni::Ptr<iZip> _zip;
  const tU32           _level;
  ZipBufferEncoder(iZip* apZip, tU32 anCompressionLevel) : _level(anCompressionLevel) {
    niAssert(niIsOK(apZip));
    _zip = apZip;
  }
  tBool __stdcall IsOK() const {
    return _zip.IsOK();
  }

  virtual tU32 __stdcall EncodeMarker() const {
    return kfccFileBufferZipMarker;
  }
  virtual tSize __stdcall EncodeMaxDestSize(tU32 anSrcSize) {
    return _zip->CompressBound(anSrcSize);
  }
  virtual tSize __stdcall EncodeBuffer(tPtr apDest, tSize anDestSize,
                                       tPtr apSrc, tSize anSrcSize) {
    tU32 destLen = anDestSize;
    if (_zip->Compress2(apDest,&destLen,apSrc,anSrcSize,_level) != Z_OK)
      return 0;
    return destLen;
  }
};

//! Zip buffer decoder.
struct ZipBufferDecoder : public ImplRC<iBufferDecoder>
{
  ni::Ptr<iZip> _zip;
  ZipBufferDecoder(iZip* apZip) {
    niAssert(niIsOK(apZip));
    _zip = apZip;
  }
  virtual tU32 __stdcall DecodeMarker() const {
    return kfccFileBufferZipMarker;
  }
  virtual tSize __stdcall DecodeMaxDestSize(tU32 anSrcSize) {
    return anSrcSize;
  }
  virtual tSize __stdcall DecodeBuffer(tPtr apDest, tSize anDestSize, tPtr apSrc, tSize anSrcSize) {
    tU32 destLen = anDestSize;
    if (_zip->Uncompress(apDest,&destLen,apSrc,anSrcSize) != Z_OK)
      return 0;
    return destLen;
  }
};

iBufferEncoder* __stdcall cZip::CreateZipBufferEncoder(tU32 anCLevel) {
  return niNew ZipBufferEncoder(this,anCLevel);
}
iBufferDecoder* __stdcall cZip::CreateZipBufferDecoder() {
  return niNew ZipBufferDecoder(this);
}
iFile* __stdcall cZip::CreateFileZipBufferEncoder(iFileBase* apBase, tU32 anCLevel) {
  niCheck(niIsOK(apBase),NULL);
  Ptr<iBufferEncoder> enc = niNew ZipBufferEncoder(this,anCLevel);
  return ni::CreateFileBufferEncoder(apBase,enc);
}
iFile* __stdcall cZip::CreateFileZipBufferDecoder(iFileBase* apBase, tSize aDecSize) {
  niCheck(niIsOK(apBase),NULL);
  Ptr<iBufferDecoder> dec = niNew ZipBufferDecoder(this);
  return ni::CreateFileBufferDecoder(apBase,dec,aDecSize);
}

//--------------------------------------------------------------------------------------------
//
//  Raw encoder
//
//--------------------------------------------------------------------------------------------

//! File buffer Raw marker.
const tU32 kfccFileBufferRawMarker = niFourCC('E','R','A','W');

//! Raw buffer encoder.
struct RawBufferEncoder : public ImplRC<iBufferEncoder>
{
  RawBufferEncoder() {
  }
  tBool __stdcall IsOK() const {
    return eTrue;
  }

  virtual tU32 __stdcall EncodeMarker() const {
    return kfccFileBufferRawMarker;
  }
  virtual tSize __stdcall EncodeMaxDestSize(tU32 anSrcSize) {
    return anSrcSize;
  }
  virtual tSize __stdcall EncodeBuffer(tPtr apDest, tSize anDestSize,
                                       tPtr apSrc, tSize anSrcSize) {
    memcpy(apDest,apSrc,anSrcSize);
    return anSrcSize;
  }
};

//! Raw buffer decoder.
struct RawBufferDecoder : public ImplRC<iBufferDecoder>
{
  RawBufferDecoder() {
  }
  virtual tU32 __stdcall DecodeMarker() const {
    return kfccFileBufferRawMarker;
  }
  virtual tSize __stdcall DecodeMaxDestSize(tU32 anSrcSize) {
    return anSrcSize;
  }
  virtual tSize __stdcall DecodeBuffer(tPtr apDest, tSize anDestSize, tPtr apSrc, tSize anSrcSize) {
    memcpy(apDest,apSrc,anSrcSize);
    return anSrcSize;
  }
};

iBufferEncoder* __stdcall cZip::CreateRawBufferEncoder() {
  return niNew RawBufferEncoder();
}
iBufferDecoder* __stdcall cZip::CreateRawBufferDecoder() {
  return niNew RawBufferDecoder();
}
iFile* __stdcall cZip::CreateFileRawBufferEncoder(iFileBase* apBase) {
  niCheck(niIsOK(apBase),NULL);
  Ptr<iBufferEncoder> enc = niNew RawBufferEncoder();
  return ni::CreateFileBufferEncoder(apBase,enc);
}
iFile* __stdcall cZip::CreateFileRawBufferDecoder(iFileBase* apBase, tSize aDecSize) {
  niCheck(niIsOK(apBase),NULL);
  Ptr<iBufferDecoder> dec = niNew RawBufferDecoder();
  return ni::CreateFileBufferDecoder(apBase,dec,aDecSize);
}

//--------------------------------------------------------------------------------------------
//
//  Zip writter
//
//--------------------------------------------------------------------------------------------
#if niMinFeatures(15) && !defined niEmbedded
class cZipArchWrite : public ni::ImplRC<iZipArchWrite>
{
  niBeginClass(cZipArchWrite);
 public:
  cZipArchWrite(iFile* apFile, const char* pwd, const tStringCVec* apStoredExtensions) {
    niAssert(niIsOK(apFile));
    mhZip = CreateZipHandle(apFile,pwd,
                            apStoredExtensions?apStoredExtensions->data():NULL,
                            apStoredExtensions?apStoredExtensions->size():0);
    niCheck(mhZip != NULL,;);
  }
  ~cZipArchWrite() {
    Invalidate();
  }

  virtual void __stdcall Invalidate() {
    if (mhZip) {
      CloseZip(mhZip);
      mhZip = NULL;
    }
  }

  virtual tBool __stdcall IsOK() const {
    niClassIsOK(cZipArchWrite);
    return mhZip != NULL;
  }

  /////////////////////////////////////////////////////////////////
  virtual tBool __stdcall AddFileBlock(const achar* aaszName, iFile* apFile, tI64 aSize) {
    niCheck(mhZip != NULL,eFalse);
    niCheck(niIsStringOK(aaszName),eFalse);
    niCheckIsOK(apFile, eFalse);
    cString dstName = aaszName;
    if (aSize <= 0) {
      aSize = apFile->GetSize()-apFile->Tell();
    }
    ZRESULT zr = ZipAddHandle(mhZip,dstName.Chars(),apFile,(tU32)aSize);
    if (zr != ZR_OK) {
      niError(niFmt(_A("Zip error (%d): %s."),zr,GetZipMessage(zr)));
      return eFalse;
    }
    return eTrue;
  }

  /////////////////////////////////////////////////////////////////
  virtual tBool __stdcall AddRawBlock(const achar* aaszName, tPtr apData, tSize aSize) {
    niCheck(mhZip != NULL,eFalse);
    niCheck(niIsStringOK(aaszName),eFalse);
    niCheck(apData != NULL,eFalse);
    cString dstName = aaszName;
    ZRESULT zr = ZipAdd(mhZip,dstName.Chars(),apData,(tU32)aSize);
    if (zr != ZR_OK) {
      niError(niFmt(_A("Zip error (%d): %s."),zr,GetZipMessage(zr)));
      return eFalse;
    }
    return eTrue;
  }

 private:
  HZIP mhZip;
  niEndClass(cZipArchWrite);
};
#endif

#if niMinFeatures(15)
iZipArchWrite* __stdcall cZip::CreateZipArchive(ni::iFile* apDest, const char* aaszPwd, const tStringCVec* apStoredExtensions) {
  niCheckIsOK(apDest,NULL);
#ifdef niEmbedded
  return NULL;
#else
  return niNew cZipArchWrite(apDest,aaszPwd,apStoredExtensions);
#endif
}
#endif

namespace ni {

niExportFunc(ni::iZip*) GetZip() {
  static cZip _Zip;
  return &_Zip;
}

niExportFunc(iUnknown*) New_niLang_Zip(const Var& avarA, const Var& avarB) {
  return ni::GetZip();
}

struct URLFileHandler_Zip : public URLFileHandler_Manifest {
  Ptr<iFile> mptrFile;
  tHandle mhFile;

  URLFileHandler_Zip(iFile* apFile, tHandle aZipHandler) {
    mptrFile = apFile;
    mhFile = aZipHandler;
  }

  ~URLFileHandler_Zip() {
    if (IsValidHandle(mhFile)) {
      ni::GetZip()->UnzClose(mhFile);
      mhFile = eInvalidHandle;
    }
  }

  virtual iFile* __stdcall URLOpen(const achar* aURL) {
    cString path = this->GetPathFromURL(aURL);

    if (ni::GetZip()->UnzLocateFile(mhFile, path.Chars(), 2) == UNZ_END_OF_LIST_OF_FILE) {
      return NULL;
    }

    if (ni::GetZip()->UnzOpenCurrentFile(mhFile,NULL) != UNZ_OK) {
      niError(niFmt(_A("Can't open url %s."), aURL));
      return NULL;
    }

    unz_file_info fileinfo; cString fn,extra,comment;
    ni::GetZip()->UnzGetCurrentFileInfo(mhFile, &fileinfo, fn, extra, comment);

    Ptr<iFile> ptrFile = ni::CreateFileMemoryAlloc(fileinfo.uncompressed_size, path.Chars());
    if (!niIsOK(ptrFile)) {
      niError(niFmt(_A("Can't open mapped file for url %s."), aURL));
      return NULL;
    }

    ni::GetZip()->UnzReadCurrentFile(mhFile, ptrFile->GetBase(), fileinfo.uncompressed_size);

    if (ni::GetZip()->UnzCloseCurrentFile(mhFile) == UNZ_CRCERROR) {
      niError(niFmt(_A("CRC error in file %s."), aURL));
      return NULL;
    }

    return ptrFile.GetRawAndSetNull();
  }

  virtual tBool __stdcall URLExists(const achar* aURL) {
    cString path = this->GetPathFromURL(aURL);
    if (ni::GetZip()->UnzLocateFile(mhFile, path.Chars(), 2) == UNZ_END_OF_LIST_OF_FILE) {
      return eFalse;
    }
    return eTrue;
  }
};

niExportFunc(iUnknown*) New_niLang_URLFileHandlerZip(const Var& avarA, const Var& avarB) {
  QPtr<iFile> pZipFile = avarA;
  if (!pZipFile.IsOK()) {
    niError("VarA is not a valid file object.");
    return NULL;
  }

  tHandle hZipFileHandle = ni::GetZip()->UnzOpen(pZipFile);
  if (!IsValidHandle(hZipFileHandle)) {
    niError("Specified file object isn't a valid zip archive.");
    return NULL;
  }

  return niNew URLFileHandler_Zip(pZipFile, hZipFileHandle);
}

}
