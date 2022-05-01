#pragma once
#ifndef __IZIP_19759546_H__
#define __IZIP_19759546_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "Types.h"

struct zlib_internal_state;

#ifdef __cplusplus
#include "Utils/CollectionImpl.h"

namespace ni {
struct iFile;
struct iFileBase;
struct iBufferEncoder;
struct iBufferDecoder;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// ZLib
/** \addtogroup niLang
 * @{
 */

/* Maximum value for windowBits in deflateInit2 and inflateInit2.
 * WARNING: reducing ZIP_MAX_WBITS makes minigzip unable to extract .gz files
 * created by gzip. (Files created by minigzip can still be extracted by
 * gzip.)
 */
#ifndef ZIP_MAX_WBITS
#  define ZIP_MAX_WBITS   15 /* 32K LZ77 window */
#endif

/* Maximum value for memLevel in deflateInit2 */
#ifndef ZIP_MAX_MEM_LEVEL
#  ifdef ZIP_MAXSEG_64K
#    define ZIP_MAX_MEM_LEVEL 8
#  else
#    define ZIP_MAX_MEM_LEVEL 9
#  endif
#endif

typedef void* (__cdecl *zlib_alloc_func) (void* opaque, niNamespace(ni,tU32) items, niNamespace(ni,tU32) size);
typedef void  (__cdecl *zlib_free_func)  (void* opaque, void* address);
typedef niNamespace(ni,tU32) (__cdecl *zlib_in_func) (void*, niNamespace(ni,tU8)**);
typedef niNamespace(ni,tI32) (__cdecl *zlib_out_func) (void*, niNamespace(ni,tU8)*, niNamespace(ni,tU32));

typedef struct z_stream_s {
  niNamespace(ni,tU8)  *next_in;                    /* next input byte */
  niNamespace(ni,tI32)  avail_in;                   /* number of bytes available at next_in */
  niNamespace(ni,tU32)  total_in;                   /* total nb of input bytes read so far */

  niNamespace(ni,tU8)  *next_out;                   /* next output byte should be put there */
  niNamespace(ni,tI32)  avail_out;                  /* remaining free space at next_out */
  niNamespace(ni,tU32)  total_out;                  /* total nb of bytes output so far */

  char                       *msg;      /* last error message, NULL if no error */
  struct zlib_internal_state *state;    /* not visible by applications */

  zlib_alloc_func zalloc;               /* used to allocate the internal state */
  zlib_free_func  zfree;                /* used to free the internal state */
  void*           opaque;               /* private data object passed to zalloc and zfree */

  niNamespace(ni,tI32) data_type;                   /* best guess about the data type: binary or text */
  niNamespace(ni,tU32) adler;                       /* adler32 value of the uncompressed data */
  niNamespace(ni,tU32) reserved;                    /* reserved for future use */
} z_stream;

typedef z_stream  *z_streamp;

/*
  gzip header information passed to and from zlib routines.  See RFC 1952
  for more details on the meanings of these fields.
*/
typedef struct gz_header_s {
  niNamespace(ni,tI32)  text;                       /* true if compressed data believed to be text */
  niNamespace(ni,tU32)  time;                       /* modification time */
  niNamespace(ni,tI32)  xflags;                     /* extra flags (not used when writing a gzip file) */
  niNamespace(ni,tI32)  os;                         /* operating system */
  niNamespace(ni,tU8)  *extra;                      /* pointer to extra field or Z_NULL if none */
  niNamespace(ni,tI32)  extra_len;                  /* extra field length (valid if extra != Z_NULL) */
  niNamespace(ni,tI32)  extra_max;                  /* space at extra (only when reading header) */
  niNamespace(ni,tU8)  *name;                       /* pointer to zero-terminated file name or Z_NULL */
  niNamespace(ni,tI32)  name_max;                   /* space at name (only when reading header) */
  niNamespace(ni,tU8)  *comment;                    /* pointer to zero-terminated comment or Z_NULL */
  niNamespace(ni,tI32)  comm_max;                   /* space at comment (only when reading header) */
  niNamespace(ni,tI32)  hcrc;                       /* true if there was or will be a header crc */
  niNamespace(ni,tI32)  done;       /* true when done reading gzip header (not used
                                       when writing a gzip file) */
} gz_header;

typedef gz_header  *gz_headerp;

#define Z_NO_FLUSH      0
#define Z_PARTIAL_FLUSH 1 /* will be removed, use Z_SYNC_FLUSH instead */
#define Z_SYNC_FLUSH    2
#define Z_FULL_FLUSH    3
#define Z_FINISH        4
#define Z_BLOCK         5

#define Z_OK            0
#define Z_STREAM_END    1
#define Z_NEED_DICT     2
#define Z_ERRNO        (-1)
#define Z_STREAM_ERROR (-2)
#define Z_DATA_ERROR   (-3)
#define Z_MEM_ERROR    (-4)
#define Z_BUF_ERROR    (-5)
#define Z_VERSION_ERROR (-6)

#define Z_NO_COMPRESSION         0
#define Z_BEST_SPEED             1
#define Z_BEST_COMPRESSION       9
#define Z_DEFAULT_COMPRESSION  (-1)

#define Z_FILTERED            1
#define Z_HUFFMAN_ONLY        2
#define Z_RLE                 3
#define Z_FIXED               4
#define Z_DEFAULT_STRATEGY    0

#define Z_BINARY   0
#define Z_TEXT     1
#define Z_ASCII    Z_TEXT   /* for compatibility with 1.2.2 and earlier */
#define Z_UNKNOWN  2

#define Z_DEFLATED   8

#define Z_NULL  0  /* for initializing zalloc, zfree, opaque */

// Unzip defines
#define UNZ_OK                  (0)
#define UNZ_END_OF_LIST_OF_FILE (-100)
#define UNZ_ERRNO               (Z_ERRNO)
#define UNZ_EOF                 (0)
#define UNZ_PARAMERROR          (-102)
#define UNZ_BADZIPFILE          (-103)
#define UNZ_INTERNALERROR       (-104)
#define UNZ_CRCERROR            (-105)
#define UNZ_PASSWORD            (-106)

// unz_global_info structure contain global data about the ZIPfile
// These data comes from the end of central dir
typedef struct unz_global_info_s
{
  unsigned long number_entry;         /* total number of entries in the central dir on this disk */
  unsigned long size_comment;         /* size of the global comment of the zipfile */
} unz_global_info;

// unz_file_info contain information about a file in the zipfile
#if niPragmaPack
#pragma pack(push, 1)
#endif
typedef struct unz_file_info_s
{
  niNamespace(ni,tU16) version;                     /* version made by                 2 bytes */
  niNamespace(ni,tU16) version_needed;              /* version needed to extract       2 bytes */
  niNamespace(ni,tU16) flag;                        /* general purpose bit flag        2 bytes */
  niNamespace(ni,tU16) compression_method;          /* compression method              2 bytes */
  niNamespace(ni,tU32) dosDate;                     /* last mod file date in Dos fmt   4 bytes */
  niNamespace(ni,tU32) crc;                         /* crc-32                          4 bytes */
  niNamespace(ni,tU32) compressed_size;             /* compressed size                 4 bytes */
  niNamespace(ni,tU32) uncompressed_size;           /* uncompressed size               4 bytes */
  niNamespace(ni,tU16) size_filename;               /* filename length                 2 bytes */
  niNamespace(ni,tU16) size_file_extra;             /* extra field length              2 bytes */
  niNamespace(ni,tU16) size_file_comment;           /* file comment length             2 bytes */
  niNamespace(ni,tU16) disk_num_start;              /* disk number start               2 bytes */
  niNamespace(ni,tU16) internal_fa;                 /* internal file attributes        2 bytes */
  niNamespace(ni,tU32) ZEXTERNal_fa;                /* ZEXTERNal file attributes        4 bytes */
  //tm_unz tmu_date;
  niNamespace(ni,tU32) offset, c_offset;
} niPacked(1) unz_file_info;
#if niPragmaPack
#pragma pack(pop)
#endif

/**@}*/

#ifdef __cplusplus
namespace ni {
/** \addtogroup niLang
 * @{
 */

//! Zip archive writer
struct iZipArchWrite : public iUnknown
{
  niDeclareInterfaceUUID(iZipArchWrite,0x50cf0914,0x0ffc,0x4085,0xbc,0xe0,0xa3,0x61,0xa6,0x01,0x74,0x5d);

  //! Adds a file block to the archive.
  //! \return eTrue if successfull, else eFalse.
  virtual tBool __stdcall AddFileBlock(const achar* aaszName, iFile* apFile, tI64 aSize) = 0;
  //! Adds a block of raw data as a file.
  //! {NoAutomation}
  //! \return eTrue if successfull, else eFalse.
  virtual tBool __stdcall AddRawBlock(const achar* aaszName, tPtr apData, tSize aSize) = 0;
};

//! Zip interface.
struct iZip : public iUnknown
{
  niDeclareInterfaceUUID(iZip,0x6573d955,0x27b5,0x4ff0,0x9e,0xb2,0x6b,0xf8,0xbf,0x31,0xc6,0xd5)

  //########################################################################################
  //! \name Zip
  //########################################################################################
  //! @{

  virtual tBool __stdcall ZipUncompressBuffer(tPtr apDest, tU32 anDestSize, tPtr apSrc, tU32 anSrcSize) = 0;
  virtual tBool __stdcall ZipUncompressBufferInFile(iFile* apDest, tU32 anDestSize, tPtr apSrc, tU32 anSrcSize) = 0;
  virtual tBool __stdcall ZipUncompressFile(iFile* apDest, tU32 anDestSize, iFile* apSrc, tU32 anSrcSize) = 0;
  virtual tBool __stdcall ZipUncompressFileInBuffer(tPtr apDest, tU32 anDestSize, iFile* apSrc, tU32 anSrcSize) = 0;
  virtual tBool __stdcall ZipInflateFileInBuffer(tPtr apDest, tU32 anDestSize, iFile* apSrc) = 0;
  //! The destination buffer must be at least 20% bigger than the source.
  virtual tU32  __stdcall ZipCompressBuffer(tPtr apDest, tPtr apSrc, tU32 anSrcSize, tU32 anLevel) = 0;
  virtual tU32  __stdcall ZipCompressBufferInFile(iFile* apDest, tPtr apSrc, tU32 anSrcSize, tU32 anLevel) = 0;
  virtual tU32  __stdcall ZipCompressFile(iFile* apDest, iFile* apSrc, tU32 anSrcSize, tU32 anLevel) = 0;
  //! @}

  //########################################################################################
  //! \name UnZip
  //########################################################################################
  //! @{

  //! {NoAutomation}
  virtual tHandle __stdcall UnzOpen(ni::iFile* apFile) = 0;
  //! {NoAutomation}
  virtual tI32    __stdcall UnzClose(tHandle file) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall UnzGetGlobalInfo(tHandle file, unz_global_info *pglobal_info) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall UnzGetGlobalComment(tHandle file, cString& strComment) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall UnzGoToFirstFile(tHandle file) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall UnzGoToNextFile(tHandle file) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall UnzLocateFile(tHandle file, const achar *szFileName, tBool bCaseSensitivity) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall UnzLocateFileInc(tHandle file, tU32 num, tU32 pos) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall UnzGetCurrentFileInfo( tHandle file,
                                                unz_file_info *pfile_info,
                                                cString& strFileName,
                                                cString& strExtraField,
                                                cString& strComment) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall UnzOpenCurrentFile(tHandle file, const achar* pwd) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall UnzCloseCurrentFile(tHandle file) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall UnzReadCurrentFile(tHandle file, tPtr buf, tU32 len) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall Unztell(tHandle file) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall Unzeof(tHandle file) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall UnzGetLocalExtrafield(tHandle file, tPtr buf, tU32 len) = 0;
  //! @}

  //########################################################################################
  //! \name ZLib
  //########################################################################################
  //! @{

  //! {NoAutomation}
  virtual const achar* __stdcall ZlibVersion (void) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall Deflate (z_streamp strm, tI32 flush) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall DeflateEnd (z_streamp strm) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall Inflate (z_streamp strm, tI32 flush) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall InflateEnd (z_streamp strm) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall DeflateSetDictionary (z_streamp strm, const tU8 *dictionary, tI32  dictLength) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall DeflateCopy (z_streamp dest, z_streamp source) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall DeflateReset (z_streamp strm) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall DeflateParams (z_streamp strm, tI32 level, tI32 strategy) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall DeflateTune (z_streamp strm, tI32 good_length, tI32 max_lazy, tI32 nice_length, tI32 max_chain) = 0;
  //! {NoAutomation}
  virtual tU32 __stdcall DeflateBound (z_streamp strm, tU32 sourceLen) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall DeflatePrime (z_streamp strm, tI32 bits, tI32 value) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall DeflateSetHeader (z_streamp strm, gz_headerp head) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall InflateSetDictionary (z_streamp strm, const tU8 *dictionary, tI32  dictLength) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall InflateSync (z_streamp strm) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall InflateCopy (z_streamp dest, z_streamp source) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall InflateReset (z_streamp strm) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall InflatePrime (z_streamp strm, tI32 bits, tI32 value) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall InflateGetHeader (z_streamp strm, gz_headerp head) = 0;

  //! {NoAutomation}
  virtual tI32 __stdcall InflateBack (z_streamp strm, zlib_in_func aIn, void  *in_desc, zlib_out_func aOut, void  *out_desc) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall InflateBackEnd (z_streamp strm) = 0;
  //! {NoAutomation}
  virtual tU32 __stdcall ZlibCompileFlags (void) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall Compress (tU8 *dest, tU32 *destLen, const tU8 *source, tU32 sourceLen) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall Compress2 (tU8 *dest, tU32 *destLen, const tU8 *source, tU32 sourceLen, tI32 level) = 0;
  //! {NoAutomation}
  virtual tU32 __stdcall CompressBound (tU32 sourceLen) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall Uncompress (tU8 *dest, tU32 *destLen, const tU8 *source, tU32 sourceLen) = 0;
  //! {NoAutomation}
  virtual tU32 __stdcall Adler32 (tU32 adler, const tU8 *buf, tI32 len) = 0;
  //! {NoAutomation}
  virtual tU32 __stdcall Adler32Combine (tU32 adler1, tU32 adler2, tSize len2) = 0;
  //! {NoAutomation}
  virtual tU32 __stdcall Crc32(tU32 crc, const tU8 *buf, tI32 len) = 0;
  //! {NoAutomation}
  virtual tU32 __stdcall Crc32Combine (tU32 crc1, tU32 crc2, tSize len2) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall DeflateInit(z_streamp strm, tI32 level) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall InflateInit(z_streamp strm) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall DeflateInit2(z_streamp strm, tI32  level, tI32  method, tI32 windowBits, tI32 memLevel, tI32 strategy) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall InflateInit2(z_streamp strm, tI32  windowBits) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall InflateBackInit(z_streamp strm, tI32 windowBits, tU8  *window) = 0;
  //! {NoAutomation}
  virtual const achar* __stdcall ZError(tI32 err) = 0;
  //! {NoAutomation}
  virtual tI32 __stdcall InflateSyncPoint (z_streamp z) = 0;
  //! {NoAutomation}
  virtual const tU32* __stdcall GetCRCTable(void) = 0;
  //! @}

  //########################################################################################
  //! \name Buffer encoder
  //########################################################################################
  //! @{

  //! Create a Zip buffer encoder.
  virtual iBufferEncoder* __stdcall CreateZipBufferEncoder(tU32 anCLevel) = 0;
  //! Create a Zip buffer decoder.
  virtual iBufferDecoder* __stdcall CreateZipBufferDecoder() = 0;
  //! Create a Zip buffer encoder file.
  virtual iFile* __stdcall CreateFileZipBufferEncoder(iFileBase* apBaseFile, tU32 anCLevel) = 0;
  //! Create a Zip buffer decoder file.
  //! \see iLang::CreateFileBufferDecoder
  virtual iFile* __stdcall CreateFileZipBufferDecoder(iFileBase* apBaseFile, tSize aDecodedSize) = 0;
  //! @}

  //########################################################################################
  //! \name RAW encoder (no compression)
  //########################################################################################
  //! @{

  //! Create a Raw buffer encoder.
  virtual iBufferEncoder* __stdcall CreateRawBufferEncoder() = 0;
  //! Create a Raw buffer decoder.
  virtual iBufferDecoder* __stdcall CreateRawBufferDecoder() = 0;
  //! Create a Raw buffer encoder file.
  virtual iFile* __stdcall CreateFileRawBufferEncoder(iFileBase* apBaseFile) = 0;
  //! Create a Raw buffer decoder file.
  //! \see iLang::CreateFileBufferDecoder
  virtual iFile* __stdcall CreateFileRawBufferDecoder(iFileBase* apBaseFile, tSize aDecodedSize) = 0;
  //! @}

  //########################################################################################
  //! \name Zip archive
  //########################################################################################
  //! @{

#if niMinFeatures(15)
  //! Create a new zip archive writter.
  virtual iZipArchWrite* __stdcall CreateZipArchive(iFile* apDest, const achar* aaszPwd, const tStringCVec* apStoredExtensions) = 0;
  //! Open a file which can write or read directly in a zip file.
  virtual iFile* __stdcall GZipOpen(iFileBase* apFile, tU32 aulCompressionMode) = 0;
  //! Open a file which can write or read directly in a zip file.
  virtual iFile* __stdcall ZipOpen(iFileBase* apFile, tU32 aulCompressionMode) = 0;
#endif
  //! @}
};

niExportFunc(iZip*) GetZip();
niExportFunc(ni::iUnknown*) New_niLang_Zip(const ni::Var&,const ni::Var&);

/**@}*/
} // End of ni namespace
#endif

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __IZIP_19759546_H__
