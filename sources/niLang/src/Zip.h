#ifndef __ZIP_4375745_H__
#define __ZIP_4375745_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/IZip.h"
#include "API/niLang/Utils/UnknownImpl.h"
#include "API/niLang/ObjModel.h"

namespace ni {

//////////////////////////////////////////////////////////////////////////////////////////////
class cZip : public cIUnknownImpl<iZip,eIUnknownImplFlags_Local>
{
 public:
  cZip();
  ~cZip();

  tBool __stdcall IsOK() const;

  tBool __stdcall ZipUncompressBuffer(tPtr apDest, tU32 anDestSize, tPtr apSrc, tU32 anSrcSize);
  tBool __stdcall ZipUncompressBufferInFile(iFile* apDest, tU32 anDestSize, tPtr apSrc, tU32 anSrcSize);
  tBool __stdcall ZipUncompressFile(iFile* apDest, tU32 anDestSize, iFile* apSrc, tU32 anSrcSize);
  tBool __stdcall ZipUncompressFileInBuffer(tPtr apDest, tU32 anDestSize, iFile* apSrc, tU32 anSrcSize);
  tBool __stdcall ZipInflateFileInBuffer(tPtr apDest, tU32 anDestSize, iFile* apSrc);
  tU32  __stdcall ZipCompressBuffer(tPtr apDest, tPtr apSrc, tU32 anSrcSize, tU32 anLevel);
  tU32  __stdcall ZipCompressBufferInFile(iFile* apDest, tPtr apSrc, tU32 anSrcSize, tU32 anLevel);
  tU32  __stdcall ZipCompressFile(iFile* apDest, iFile* apSrc, tU32 anSrcSize, tU32 anLevel);

  tHandle __stdcall UnzOpen(ni::iFile* apFile);
  tI32    __stdcall UnzClose(tHandle file);
  tI32 __stdcall UnzGetGlobalInfo(tHandle file, unz_global_info *pglobal_info);
  tI32 __stdcall UnzGetGlobalComment(tHandle file, cString& strComment);
  tI32 __stdcall UnzGoToFirstFile(tHandle file);
  tI32 __stdcall UnzGoToNextFile(tHandle file);
  tI32 __stdcall UnzLocateFile(tHandle file, const achar *szFileName, tBool bCaseSensitivity);
  tI32 __stdcall UnzLocateFileInc(tHandle file, tU32 num, tU32 pos);
  tI32 __stdcall UnzGetCurrentFileInfo( tHandle file,
                                        unz_file_info *pfile_info,
                                        cString& strFilename,
                                        cString& strExtraField,
                                        cString& strComment);
  tI32 __stdcall UnzOpenCurrentFile(tHandle file, const char* pwd);
  tI32 __stdcall UnzCloseCurrentFile(tHandle file);
  tI32 __stdcall UnzReadCurrentFile(tHandle file, tPtr buf, tU32 len);
  tI32 __stdcall Unztell(tHandle file);
  tI32 __stdcall Unzeof(tHandle file);
  tI32 __stdcall UnzGetLocalExtrafield(tHandle file, tPtr buf, tU32 len);

  const char * __stdcall ZlibVersion (void);
  tI32 __stdcall Deflate (z_streamp strm, tI32 flush);
  tI32 __stdcall DeflateEnd (z_streamp strm);
  tI32 __stdcall Inflate (z_streamp strm, tI32 flush);
  tI32 __stdcall InflateEnd (z_streamp strm);
  tI32 __stdcall DeflateSetDictionary (z_streamp strm, const tU8 *dictionary, tI32  dictLength);
  tI32 __stdcall DeflateCopy (z_streamp dest, z_streamp source);
  tI32 __stdcall DeflateReset (z_streamp strm);
  tI32 __stdcall DeflateParams (z_streamp strm, tI32 level, tI32 strategy);
  tI32 __stdcall DeflateTune (z_streamp strm, tI32 good_length, tI32 max_lazy, tI32 nice_length, tI32 max_chain);
  tU32 __stdcall DeflateBound (z_streamp strm, tU32 sourceLen);
  tI32 __stdcall DeflatePrime (z_streamp strm, tI32 bits, tI32 value);
  tI32 __stdcall DeflateSetHeader (z_streamp strm, gz_headerp head);
  tI32 __stdcall InflateSetDictionary (z_streamp strm, const tU8 *dictionary, tI32  dictLength);
  tI32 __stdcall InflateSync (z_streamp strm);
  tI32 __stdcall InflateCopy (z_streamp dest, z_streamp source);
  tI32 __stdcall InflateReset (z_streamp strm);
  tI32 __stdcall InflatePrime (z_streamp strm, tI32 bits, tI32 value);
  tI32 __stdcall InflateGetHeader (z_streamp strm, gz_headerp head);

  tI32 __stdcall InflateBack (z_streamp strm, zlib_in_func aIn, void  *in_desc, zlib_out_func aOut, void  *out_desc);
  tI32 __stdcall InflateBackEnd (z_streamp strm);
  tU32 __stdcall ZlibCompileFlags (void);
  tI32 __stdcall Compress (tU8 *dest, tU32 *destLen, const tU8 *source, tU32 sourceLen);
  tI32 __stdcall Compress2 (tU8 *dest, tU32 *destLen, const tU8 *source, tU32 sourceLen, tI32 level);
  tU32 __stdcall CompressBound (tU32 sourceLen);
  tI32 __stdcall Uncompress (tU8 *dest, tU32 *destLen, const tU8 *source, tU32 sourceLen);
  tU32 __stdcall Adler32 (tU32 adler, const tU8 *buf, tI32 len);
  tU32 __stdcall Adler32Combine (tU32 adler1, tU32 adler2, tSize len2);
  tU32 __stdcall Crc32(tU32 crc, const tU8 *buf, tI32 len);
  tU32 __stdcall Crc32Combine(tU32 crc1, tU32 crc2, tSize len2);
  tI32 __stdcall DeflateInit(z_streamp strm, tI32 level);
  tI32 __stdcall InflateInit(z_streamp strm);
  tI32 __stdcall DeflateInit2(z_streamp strm, tI32  level, tI32  method, tI32 windowBits, tI32 memLevel, tI32 strategy);
  tI32 __stdcall InflateInit2(z_streamp strm, tI32  windowBits);
  tI32 __stdcall InflateBackInit(z_streamp strm, tI32 windowBits, unsigned char  *window);
  const char* __stdcall ZError(tI32);
  tI32         __stdcall InflateSyncPoint(z_streamp z);
  const tU32* __stdcall GetCRCTable(void);

  iBufferEncoder* __stdcall CreateZipBufferEncoder(tU32 anCLevel);
  iBufferDecoder* __stdcall CreateZipBufferDecoder();
  iFile* __stdcall CreateFileZipBufferEncoder(iFileBase* apBaseFile, tU32 anCLevel);
  iFile* __stdcall CreateFileZipBufferDecoder(iFileBase* apBaseFile, tSize aDecSize);

  iBufferEncoder* __stdcall CreateRawBufferEncoder();
  iBufferDecoder* __stdcall CreateRawBufferDecoder();
  iFile* __stdcall CreateFileRawBufferEncoder(iFileBase* apBaseFile);
  iFile* __stdcall CreateFileRawBufferDecoder(iFileBase* apBaseFile, tSize aDecSize);

#if niMinFeatures(15)
  iZipArchWrite* __stdcall CreateZipArchive(iFile* apDest, const char* aaszPwd, const tStringCVec* apStoredExtensions);
  iFile* __stdcall GZipOpen(iFileBase* apFile, tU32 aulCompressionMode);
  iFile* __stdcall ZipOpen(iFileBase* apFile, tU32 aulCompressionMode);
#endif
};

} // end of namespace ni
/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __ZIP_4375745_H__
