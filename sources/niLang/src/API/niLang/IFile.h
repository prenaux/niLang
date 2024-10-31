#pragma once
#ifndef __NIIFILE_H__
#define __NIIFILE_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "Types.h"
#include "ITime.h"
#include "IToString.h"
#include "Utils/SmartPtr.h"
#include "Utils/Nonnull.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */

//! Text encoding formats.
enum eTextEncodingFormat
{
  //! Text encoding format header is unknown/invalid.
  eTextEncodingFormat_Unknown = eInvalidHandle,
  //! UTF8 encoding.
  eTextEncodingFormat_UTF8 = 0,
  //! UTF8 encoding, with explicit BOM.
  eTextEncodingFormat_UTF8BOM = 1,
  //! UTF16 Little Endian.
  eTextEncodingFormat_UTF16LE = 2,
  //! UTF16 Big Endian.
  eTextEncodingFormat_UTF16BE = 3,
  //! UTF32 Little Endian.
  eTextEncodingFormat_UTF32LE = 4,
  //! UTF32 Big Endian.
  eTextEncodingFormat_UTF32BE = 5,
  //! Bit stream encoding.
  //! \remark Read/WriteChar(Ex) is not supported for this format.
  eTextEncodingFormat_BitStream = 6,

#if niUCharSize == 4
  //! Platform Unicode/WChar encoding.
#  ifdef niLittleEndian
  eTextEncodingFormat_Unicode = eTextEncodingFormat_UTF32LE,
#  else
  eTextEncodingFormat_Unicode = eTextEncodingFormat_UTF32BE,
#  endif
#elif niUCharSize == 2
  //! Platform Unicode/WChar encoding.
#  ifdef niLittleEndian
  eTextEncodingFormat_Unicode = eTextEncodingFormat_UTF16LE,
#  else
  eTextEncodingFormat_Unicode = eTextEncodingFormat_UTF16BE,
#  endif
#elif niUCharSize == 1
  eTextEncodingFormat_Unicode = eTextEncodingFormat_UTF8,
#endif

  //! Native encoding.
  eTextEncodingFormat_Native = eTextEncodingFormat_UTF8,
  //! \internal
  eTextEncodingFormat_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Callback interface used by the FileEnum() method of the iLang interface.
//{DispatchWrapper}
struct iFileEnumSink : public iUnknown
{
  niDeclareInterfaceUUID(iFileEnumSink,0x0298d5b5,0xb4e5,0x4975,0x91,0xbf,0x50,0x14,0x83,0x4a,0xc9,0x5c);
  virtual tBool __stdcall OnFound(const achar* aszFile, tU32 aFileAttrs, tI64 anFileSize) = 0;
};

//! Serialization ID fourCC
const tU32 kfccSerializedObject = niFourCC('S','E','Z','O');

//! File flags.
enum eFileFlags
{
  //! File can be read.
  eFileFlags_Read = niBit(0),
  //! File can be written.
  eFileFlags_Write = niBit(1),
  //! File opened in append mode.
  eFileFlags_Append = niBit(2),
  //! The previous read operation read past the end of the file (could not read all bytes).
  eFileFlags_PartialRead = niBit(16),
  //! The previous write operation could not write all bytes to the file.
  eFileFlags_PartialWrite = niBit(17),
  //! The file is an encoder/decoder (compressor/decompressor/encryptor/decryptor).
  //! \remark Encoders might not move the file cursor by as many
  //!         bytes as passed to the read/write methods since they
  //!         usually compress or uncompress data. That is to say that Tell() might
  //!         not match the the straight number of byte read or written to the file.
  eFileFlags_Encoder = niBit(18),
  //! Seek, SeekSet and SeekEnd are not supported and will always fail.
  eFileFlags_NoSeek = niBit(19),
  //! The file is a dummy, it wont really read or write anything.
  //! \remark Used mainly to compute size of future serialization.
  eFileFlags_Dummy = niBit(20),
  //! The file is based on a stream.
  //! \remark A stream is a 'connection' that can be
  //!         interrupted/paused/restarted at any time, mainly it
  //!         means that Read/Write could block for an undefined
  //!         amount of time waiting for more data to come or a
  //!         write queue to be consumed. In general seeking is not
  //!         supported for streams.
  eFileFlags_Stream = niBit(21),
  //! \internal
  eFileFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};
//! File flags.
typedef tU32  tFileFlags;

//! Base file interface.
struct iFileBase : public iUnknown
{
  niDeclareInterfaceUUID(iFileBase,0x49c35546,0x8a90,0x4650,0x98,0xaa,0x2e,0x7c,0x68,0x45,0xab,0xc0)

  //! Get the file flags.
  //! {Property}
  virtual tFileFlags __stdcall GetFileFlags() const = 0;
  //! Get the path from where this file has been opened.
  //! \return the path from where this file has been opened. If it has not been opened
  //!     from a file, by example it's a system memory file, return NULL.
  //! {Property}
  virtual const achar* __stdcall GetSourcePath() const = 0;

  //! Moves the file pointer from the current position.
  //! \param offset is the number of bytes to add to the current position.
  //! \return eTrue if successful, else eFalse.
  virtual tBool __stdcall Seek(tI64 offset) = 0;
  //! Set the file pointer position from the begining of the file.
  //! \param offset is the position, in bytes, from the begining of the file.
  //! \return eTrue if successful, else eFalse.
  virtual tBool __stdcall SeekSet(tI64 offset) = 0;
  //! Read data from the file.
  //! \param apOut is the buffer where the data will be copied.
  //! \param anSize is the number of bytes to read from the file.
  //! \return the number of bytes really read.
  virtual tSize __stdcall ReadRaw(void* apOut, tSize anSize) = 0;
  //! Write data in the file.
  //! \param apIn is the source buffer which contain the data to write.
  //! \param anSize is the number of bytes to write in the file. Usually the size of the pIn buffer.
  //! \return the number of bytes really written.
  virtual tSize __stdcall WriteRaw(const void* apIn, tSize anSize) = 0;

  //! Tell the current cursor position in the file in bytes.
  virtual tI64 __stdcall Tell() = 0;
  //! Get the size of the file.
  //! \return the size of the file.
  //! \remark If the file is write only, this method can return (0).
  //! {Property}
  virtual tI64 __stdcall GetSize() const = 0;

  //! Moves the file pointer from the end of the file.
  //! \param  offset is the number of bytes to move from the end of the file.
  //!     By example if you pass 4, the cursor will be passed four bytes before the end of the file.
  //! \return eTrue if successful, else eFalse.
  virtual tBool __stdcall SeekEnd(tI64 offset) = 0;
  //! Flush the file content.
  virtual tBool __stdcall Flush() = 0;
  //! Get the file time.
  virtual tBool __stdcall GetTime(eFileTime aFileTime, iTime* apTime) const = 0;
  //! Set the file time.
  virtual tBool __stdcall SetTime(eFileTime aFileTime, const iTime* apTime) = 0;
  //! Resize the file.
  virtual tBool __stdcall Resize(tI64 newSize) = 0;
};

//! File interface.
struct iFile : public iUnknown
{
  niDeclareInterfaceUUID(iFile,0xac326eae,0x8870,0x4f15,0xb8,0x93,0x30,0xe8,0x32,0x60,0x0f,0x3c)

  //########################################################################################
  //! \name Base
  //########################################################################################
  //! @{

  //! Get the file base.
  //! {Property}
  //! \remark This shouldnt be used directly unless you really know what you are doing.
  virtual iFileBase* __stdcall GetFileBase() const = 0;
  //! Get the path from where this file has been opened.
  //! \return the path from where this file has been opened. If it has not been opened
  //!     from a file, by example it's a system memory file, return NULL.
  //! {Property}
  virtual const achar* __stdcall GetSourcePath() const = 0;

  //! Moves the file pointer from the current position.
  //! \param offset is the number of bytes to add to the current position.
  //! \return eTrue if successful, else eFalse.
  virtual tBool __stdcall Seek(tI64 offset) = 0;
  //! Set the file pointer position from the begining of the file.
  //! \param offset is the position, in bytes, from the begining of the file.
  //! \return eTrue if successful, else eFalse.
  virtual tBool __stdcall SeekSet(tI64 offset) = 0;
  //! Read data from the file.
  //! \param pOut is the buffer where the data will be copied.
  //! \param nSize is the number of bytes to read from the file.
  //! \return the number of bytes really read.
  virtual tSize __stdcall ReadRaw(void* pOut, tSize nSize) = 0;
  //! Write data in the file.
  //! \param pIn is the source buffer which contain the data to write.
  //! \param nSize is the number of bytes to write in the file. Usually the size of the pIn buffer.
  //! \return the number of bytes really written.
  virtual tSize __stdcall WriteRaw(const void* pIn, tSize nSize) = 0;
  //! Tell the current cursor position in the file in bytes.
  virtual tI64 __stdcall Tell() = 0;
  //! Get the size of the file.
  //! \return the size of the file.
  //! {Property}
  virtual tI64 __stdcall GetSize() const = 0;

  //! Moves the file pointer from the end of the file.
  //! \param  offset is the number of bytes to move from the end of the file.
  //!     By example if you pass 4, the cursor will be passed four bytes before the end of the file.
  //! \return eTrue if successful, else eFalse.
  virtual tBool __stdcall SeekEnd(tI64 offset) = 0;
  //! Flush the file content.
  virtual tBool __stdcall Flush() = 0;

  //! Get the file time.
  //! \return eFalse if the file time can't be retrieved
  virtual tBool __stdcall GetTime(eFileTime aFileTime, iTime* apTime) const = 0;
  //! Set the file time.
  //! \return eFalse if the file time can't be changed
  virtual tBool __stdcall SetTime(eFileTime aFileTime, const iTime* apTime) = 0;

  //! Resize the file.
  //! \return eFalse if the file can't be resized
  virtual tBool __stdcall Resize(tI64 newSize) = 0;

  //! Get the base pointer.
  //! \remark Only for base file that implement iFileMemory.
  virtual tPtr __stdcall GetBase() const = 0;
  //! Get the here/current position pointer.
  //! \remark Only for base file that implement iFileMemory.
  virtual tPtr __stdcall GetHere() const = 0;
  //! Get the stop/end pointer.
  //! \remark Only for base file that implement iFileMemory.
  virtual tPtr __stdcall GetStop() const = 0;
  //! Set the memory buffer to use with this file.
  //! \return eFalse if the file doesnt support this operation, else eTrue.
  //! \remark Only for base file that implement iFileMemory.
  virtual tBool __stdcall SetMemPtr(tPtr apMem, tSize anSize, tBool abFree, tBool abKeepHere) = 0;
  //! Reset the file. Put it at the begining, and set the memory size to 0
  //! if it's a dynamic memory file.
  //! \remark Only for base file that implement iFileMemory.
  virtual void __stdcall Reset() = 0;

  //! Write another file in this file
  virtual tI64 __stdcall WriteFile(iFileBase* apFile, tI64 anSize = 0) = 0;
  //! @}

  //########################################################################################
  //! \name Flags
  //########################################################################################
  //! @{

  //! Get the file flags.
  //! {Property}
  virtual tFileFlags __stdcall GetFileFlags() const = 0;
  //! Check whether file can be read.
  //! {Property}
  virtual tBool __stdcall GetCanRead() const = 0;
  //! Check whether File can be written.
  //! {Property}
  virtual tBool __stdcall GetCanWrite() const = 0;
  //! Check whether the file supports seeking.
  //! {Property}
  virtual tBool __stdcall GetCanSeek() const = 0;
  //! Check whether the previous read operation could not read all bytes.
  //! {Property}
  virtual tBool __stdcall GetPartialRead() const = 0;
  //! Check whether the previous write operation could not write all bytes to the file.
  virtual tBool __stdcall GetPartialWrite() const = 0;
  //! Check whether the file is an encoder/decoder.
  //! {Property}
  virtual tBool __stdcall GetIsEncoder() const = 0;
  //! Check whether the file is a dummy.
  //! {Property}
  virtual tBool __stdcall GetIsDummy() const = 0;
  //! Check if the file is a memory file (implements iFileMemory).
  //! {Property}
  virtual tBool __stdcall GetIsMemory() const = 0;
  //! Check whether the file is a stream.
  //! {Property}
  virtual tBool __stdcall GetIsStream() const = 0;
  //! @}

  //########################################################################################
  //! \name Reading functions.
  //########################################################################################
  //! @{

  //! Read a 32 bits float and move the file pointer forward of 4 bytes.
  //! \return a 32 bits float.
  virtual tF32  __stdcall ReadF32() = 0;
  //! Read a 64 bits float and move the file pointer forward of 8 bytes.
  //! \return a 64 bits float.
  virtual tF64  __stdcall ReadF64() = 0;
  //! Read a single byte (8 bits integer) and move the file pointer forward of 1 byte.
  //! \return a 8 bits integer.
  virtual tU8 __stdcall Read8() = 0;
  //! Read a 16 bits little endian interger and move the file pointer forward of 2 bytes.
  //! \return a 16 bits little endian interger.
  virtual tU16  __stdcall ReadLE16() = 0;
  //! Read a 16 bits big endian interger and move the file pointer forward of 2 bytes.
  //! \return a big endian 16 bits integer.
  virtual tU16  __stdcall ReadBE16() = 0;
  //! Read a 32 bits little endian interger and move the file pointer forward of 4 bytes.
  //! \return a 32 bits little endian interger.
  virtual tU32  __stdcall ReadLE32() = 0;
  //! Read a 32 bits big endian interger and move the file pointer forward of 4 bytes.
  //! \return a big endian 32 bits integer.
  virtual tU32  __stdcall ReadBE32() = 0;
  //! Read a 64 bits little endian interger and move the file pointer forward of 8 bytes.
  //! \return a 64 bits little endian interger.
  virtual tU64  __stdcall ReadLE64() = 0;
  //! Read a 64 bits big endian interger and move the file pointer forward of 8 bytes.
  //! \return a big endian 64 bits integer.
  virtual tU64  __stdcall ReadBE64() = 0;
  //! Read a 32 bits float array and move the file pointer forward of 4 bytes * anNumElements.
  //! \return the size read in bytes.
  //! {NoAutomation}
  virtual tSize __stdcall ReadF32Array(tF32* apOut, tSize anNumElements) = 0;
  //! Read a 64 bits float array and move the file pointer forward of 8 bytes * anNumElements.
  //! \return the size read in bytes.
  //! {NoAutomation}
  virtual tSize __stdcall ReadF64Array(tF64* apOut, tSize anNumElements) = 0;
  //! Read a 16 bits little endian interger array and move the file pointer forward of 2 bytes * anNumElements.
  //! \return the size read in bytes.
  //! {NoAutomation}
  virtual tSize __stdcall ReadLE16Array(tU16* apOut, tSize anNumElements) = 0;
  //! Read a 16 bits big endian interger array and move the file pointer forward of 2 bytes * anNumElements.
  //! \return the size read in bytes.
  //! {NoAutomation}
  virtual tSize __stdcall ReadBE16Array(tU16* apOut, tSize anNumElements) = 0;
  //! Read a 32 bits little endian interger array and move the file pointer forward of 4 bytes * anNumElements.
  //! \return the size read in bytes.
  //! {NoAutomation}
  virtual tSize __stdcall ReadLE32Array(tU32* apOut, tSize anNumElements) = 0;
  //! Read a 32 bits big endian interger array and move the file pointer forward of 4 bytes * anNumElements.
  //! \return the size read in bytes.
  //! {NoAutomation}
  virtual tSize __stdcall ReadBE32Array(tU32* apOut, tSize anNumElements) = 0;
  //! Read a 64 bits little endian interger array and move the file pointer forward of 8 bytes * anNumElements.
  //! \return the size read in bytes.
  //! {NoAutomation}
  virtual tSize __stdcall ReadLE64Array(tU64* apOut, tSize anNumElements) = 0;
  //! Read a 64 bits big endian interger array and move the file pointer forward of 8 bytes * anNumElements.
  //! \return the size read in bytes.
  //! {NoAutomation}
  virtual tSize __stdcall ReadBE64Array(tU64* apOut, tSize anNumElements) = 0;
  //! @}

  //########################################################################################
  //! \name Writting functions.
  //########################################################################################
  //! @{

  //! Write a 32 bits float and move the file pointer forward of 4 bytes.
  //! \param  v is the 32 bits float to write.
  //! \return the number of bytes effectivly written in the file.
  virtual tSize __stdcall WriteF32(tF32 v) = 0;
  //! Write a 64 bits float and move the file pointer forward of 8 bytes.
  //! \param  v is the 64 bits float to write.
  //! \return the number of bytes effectivly written in the file.
  virtual tSize __stdcall WriteF64(tF64 v) = 0;
  //! Write a single byte (8 bits integer) and move the file pointer forward of one byte.
  //! \param  v is the byte to write.
  //! \return the number of bytes effectibly written in the file.
  virtual tSize __stdcall Write8(tU8 v) = 0;
  //! Write a little endian 16 bits integer and move the file pointer forward of 2 bytes.
  //! \param  v is the little endian 16 bits integer to write.
  //! \return the number of bytes effectivly written.
  virtual tSize __stdcall WriteLE16(tU16 v) = 0;
  //! Write a big endian 16 bits integer and move the file pointer forward of 2 bytes.
  //! \param  v is the big endian 16 bits integer to write.
  //! \return the number of bytes effectivly written.
  virtual tSize __stdcall WriteBE16(tU16 v) = 0;
  //! Write a little endian 32 bits integer and move the file pointer forward of 4 bytes.
  //! \param  v is the little endian 32 bits integer to write.
  //! \return the number of bytes effectivly written.
  virtual tSize __stdcall WriteLE32(tU32 v) = 0;
  //! Write a big endian 32 bits integer and move the file pointer forward of 4 bytes.
  //! \param  v is the big endian 32 bits integer to write.
  //! \return the number of bytes effectivly written.
  virtual tSize __stdcall WriteBE32(tU32 v) = 0;
  //! Write a little endian 64 bits integer and move the file pointer forward of 8 bytes.
  //! \param  v is the little endian 64 bits integer to write.
  //! \return the number of bytes effectivly written.
  virtual tSize __stdcall WriteLE64(tU64 v) = 0;
  //! Write a big endian 64 bits integer and move the file pointer forward of 8 bytes.
  //! \param  v is the big endian 64 bits integer to write.
  //! \return the number of bytes effectivly written.
  virtual tSize __stdcall WriteBE64(tU64 v) = 0;
  //! Write a 32 bits float array and move the file pointer forward of 4 bytes * anNumElements.
  //! \return the number of bytes effectivly written in the file.
  //! {NoAutomation}
  virtual tSize __stdcall WriteF32Array(const tF32* apIn, tU32 anNumElements) = 0;
  //! Write a 64 bits float and move the file pointer forward of 8 bytes * anNumElements.
  //! \return the number of bytes effectivly written in the file.
  //! {NoAutomation}
  virtual tSize __stdcall WriteF64Array(const tF64* apIn, tU32 anNumElements) = 0;
  //! Write a little endian 16 bits integer and move the file pointer forward of 2 bytes * anNumElements.
  //! \return the number of bytes effectivly written.
  //! {NoAutomation}
  virtual tSize __stdcall WriteLE16Array(const tU16* apIn, tU32 anNumElements) = 0;
  //! Write a big endian 16 bits integer and move the file pointer forward of 2 bytes * anNumElements.
  //! \return the number of bytes effectivly written.
  //! {NoAutomation}
  virtual tSize __stdcall WriteBE16Array(const tU16* apIn, tU32 anNumElements) = 0;
  //! Write a little endian 32 bits integer and move the file pointer forward of 4 bytes * anNumElements.
  //! \return the number of bytes effectivly written.
  //! {NoAutomation}
  virtual tSize __stdcall WriteLE32Array(const tU32* apIn, tU32 anNumElements) = 0;
  //! Write a big endian 32 bits integer and move the file pointer forward of 4 bytes * anNumElements.
  //! \return the number of bytes effectivly written.
  //! {NoAutomation}
  virtual tSize __stdcall WriteBE32Array(const tU32* apIn, tU32 anNumElements) = 0;
  //! Write a little endian 64 bits integer and move the file pointer forward of 8 bytes * anNumElements.
  //! \return the number of bytes effectivly written.
  //! {NoAutomation}
  virtual tSize __stdcall WriteLE64Array(const tU64* apIn, tU32 anNumElements) = 0;
  //! Write a big endian 64 bits integer and move the file pointer forward of 8 bytes * anNumElements.
  //! \return the number of bytes effectivly written.
  //! {NoAutomation}
  virtual tSize __stdcall WriteBE64Array(const tU64* apIn, tU32 anNumElements) = 0;
  //! @}

  //########################################################################################
  //! \name Text
  //########################################################################################
  //! @{

  //! Get the text encoding format.
  virtual eTextEncodingFormat __stdcall GetTextEncodingFormat() = 0;
  //! Set the text encoding format.
  virtual void __stdcall SetTextEncodingFormat(eTextEncodingFormat aFormat) = 0;
  //! Read text file header.
  virtual tBool __stdcall BeginTextFileRead(tBool abSeekSetZero = eTrue) = 0;
  //! Write text file header.
  virtual tBool __stdcall BeginTextFileWrite(eTextEncodingFormat aFormat = eTextEncodingFormat_UTF8, tBool abSeekSetZero = eTrue) = 0;

  //! Read a string.
  virtual cString __stdcall ReadString() = 0;
  //! Read a string line.
  virtual cString __stdcall ReadStringLine() = 0;
  //! Write a string.
  virtual tSize __stdcall WriteString(const achar* aaszString) = 0;
  //! Write a string with a end zero.
  virtual tSize __stdcall WriteStringZ(const achar* aaszString) = 0;
  //! Read a character with the current encoding.
  virtual tU32 __stdcall ReadChar() = 0;
  //! Write a character with the current encoding.
  virtual tSize __stdcall WriteChar(tU32 anChar) = 0;

  //! Read a string.
  virtual cString __stdcall ReadStringEx(eTextEncodingFormat aFmt) = 0;
  //! Read a string line.
  virtual cString __stdcall ReadStringLineEx(eTextEncodingFormat aFmt) = 0;
  //! Write a string.
  virtual tSize __stdcall WriteStringEx(eTextEncodingFormat aFmt, const achar* aaszString) = 0;
  //! Write a string with a end zero.
  virtual tSize __stdcall WriteStringZEx(eTextEncodingFormat aFmt, const achar* aaszString) = 0;
  //! Read a character with the current encoding.
  virtual tU32 __stdcall ReadCharEx(eTextEncodingFormat aFmt) = 0;
  //! Write a character with the current encoding.
  virtual tSize __stdcall WriteCharEx(eTextEncodingFormat aFmt, tU32 anChar) = 0;
  //! @}

  //########################################################################################
  //! \name Bit stream
  //########################################################################################
  //! @{

  virtual void  __stdcall BeginWriteBits() = 0;
  virtual tSize __stdcall EndWriteBits() = 0;
  virtual tBool __stdcall IsWriteBitsBegan() = 0;
  virtual void  __stdcall WriteBit(tBool abBit) = 0;
  virtual void  __stdcall WriteBits(tPtr apData, tU32 anBits, tBool abInvertBytesOrder) = 0;
  virtual void  __stdcall WriteBits8(tU8 anData, tU32 anBits) = 0;
  virtual void  __stdcall WriteBits16(tU16 anData, tU32 anBits) = 0;
  virtual void  __stdcall WriteBits32(tU32 anData, tU32 anBits) = 0;
  virtual void  __stdcall WriteBits64(tU64 anData, tU32 anBits) = 0;
  virtual void  __stdcall BeginReadBits() = 0;
  virtual tSize __stdcall EndReadBits() = 0;
  virtual tBool __stdcall IsReadBitsBegan() = 0;
  virtual tBool __stdcall ReadBit() = 0;
  virtual void  __stdcall ReadBits(tPtr apData, tU32 anBits, tBool abInvertBytesOrder) = 0;
  virtual tU8   __stdcall ReadBitsU8(tU32 anBits) = 0;
  virtual tU16  __stdcall ReadBitsU16(tU32 anBits) = 0;
  virtual tU32  __stdcall ReadBitsU32(tU32 anBits) = 0;
  virtual tU64  __stdcall ReadBitsU64(tU32 anBits) = 0;
  virtual tI8   __stdcall ReadBitsI8(tU32 anBits) = 0;
  virtual tI16  __stdcall ReadBitsI16(tU32 anBits) = 0;
  virtual tI32  __stdcall ReadBitsI32(tU32 anBits) = 0;
  virtual tI64  __stdcall ReadBitsI64(tU32 anBits) = 0;
  virtual tU32  __stdcall GetMinNumBitsU32(tU32 anNumber) = 0;
  virtual tU32  __stdcall GetMinNumBitsI32(tI32 anNumber) = 0;
  virtual tU32  __stdcall GetMinNumBitsU64(tU64 anNumber) = 0;
  virtual tU32  __stdcall GetMinNumBitsI64(tI64 anNumber) = 0;
  virtual void  __stdcall WriteBitsPackedU64(tU64 anNumber) = 0;
  virtual void  __stdcall WriteBitsPackedU48(tU64 anNumber) = 0;
  virtual void  __stdcall WriteBitsPackedU32(tU32 anNumber) = 0;
  virtual void  __stdcall WriteBitsPackedU24(tU32 anNumber) = 0;
  virtual void  __stdcall WriteBitsPackedU16(tU16 anNumber) = 0;
  virtual void  __stdcall WriteBitsPackedU8(tU8 anNumber) = 0;
  virtual tU64  __stdcall ReadBitsPackedU64() = 0;
  virtual tU64  __stdcall ReadBitsPackedU48() = 0;
  virtual tU32  __stdcall ReadBitsPackedU32() = 0;
  virtual tU32  __stdcall ReadBitsPackedU24() = 0;
  virtual tU16  __stdcall ReadBitsPackedU16() = 0;
  virtual tU8   __stdcall ReadBitsPackedU8() = 0;
  virtual void  __stdcall WriteBitsPackedI64(tI64 anNumber) = 0;
  virtual void  __stdcall WriteBitsPackedI48(tI64 anNumber) = 0;
  virtual void  __stdcall WriteBitsPackedI32(tI32 anNumber) = 0;
  virtual void  __stdcall WriteBitsPackedI24(tI32 anNumber) = 0;
  virtual void  __stdcall WriteBitsPackedI16(tI16 anNumber) = 0;
  virtual void  __stdcall WriteBitsPackedI8(tI8 anNumber) = 0;
  virtual tI64  __stdcall ReadBitsPackedI64() = 0;
  virtual tI64  __stdcall ReadBitsPackedI48() = 0;
  virtual tI32  __stdcall ReadBitsPackedI32() = 0;
  virtual tI32  __stdcall ReadBitsPackedI24() = 0;
  virtual tI16  __stdcall ReadBitsPackedI16() = 0;
  virtual tI8   __stdcall ReadBitsPackedI8() = 0;
  virtual cString __stdcall ReadBitsString() = 0;
  virtual tSize __stdcall WriteBitsString(const achar* aaszIn) = 0;
  //! @}

  //########################################################################################
  //! \name Variant
  //########################################################################################
  //! @{

  //! Write a variant.
  virtual tBool __stdcall WriteVar(const Var& aVar) = 0;
  //! Read a variant.
  virtual Var __stdcall ReadVar() = 0;
  //! Read a variant.
  //! {NoAutomation}
  virtual tBool __stdcall ReadVarEx(Var& aVar) = 0;

  //! Write a variant.
  virtual tBool __stdcall WriteBitsPackedVar(const Var& aVar) = 0;
  //! Read a variant.
  virtual Var __stdcall ReadBitsPackedVar() = 0;
  //! Read a variant.
  //! {NoAutomation}
  virtual tBool __stdcall ReadBitsPackedVarEx(Var& aVar) = 0;
  //! @}

  //########################################################################################
  //! \name Encoding/Decoding
  //########################################################################################
  //! @{

  //! Reads the specified number of bytes and return them encoded as a string.
  virtual cString __stdcall ReadRawToString(eRawToStringEncoding aFormat, tInt anNumBytes) = 0;
  //! Write bytes encoded in a string.
  virtual tInt __stdcall WriteRawFromString(eRawToStringEncoding aFormat, const achar* aaszString) = 0;

  //! Reads a line of text that potentially use quotes " and escaped quotes ""
  //! & \" to join a single 'line' with embedded newlines inside.
  //! \remark This is how CSV files handle multi-line fields.
  virtual ni::cString __stdcall ReadQuotedLine() = 0;
  //! @}
};

//! Memory file.
struct iFileMemory : public iUnknown
{
  niDeclareInterfaceUUID(iFileMemory,0x8d0af9a2,0x1c6f,0x4f50,0xb0,0x62,0x33,0xf4,0x97,0x40,0x28,0xf2)

  //! Get the base pointer.
  virtual tPtr __stdcall GetBase() const = 0;
  //! Get the here/current position pointer.
  virtual tPtr __stdcall GetHere() const = 0;
  //! Get the stop/end pointer.
  virtual tPtr __stdcall GetStop() const = 0;
  //! Set the memory buffer to use with this file.
  //! \return eFalse if the file doesnt support this operation, else eTrue.
  virtual tBool __stdcall SetMemPtr(tPtr apMem, tSize anSize, tBool abFree, tBool abKeepHere) = 0;
  //! Reset the file. Put it at the begining, and set the memory size to 0 if it's a dynamic
  //! memory file.
  virtual void __stdcall Reset() = 0;
};

//! URL File handler interface.
//! {DispatchWrapper}
struct iURLFileHandler : public iUnknown {
  niDeclareInterfaceUUID(iURLFileHandler,0x443a9e8f,0x8db6,0x4caa,0x80,0x92,0x23,0x92,0x01,0x15,0x21,0x01);

  //! Open a synchronous stream to the specified URL.
  //! \remark The operation is blocking, if necessary the whole file is
  //!         downloaded before the function returns.
  virtual iFile* __stdcall URLOpen(const achar* aURL) = 0;
  //! Validates the specified URL and if possible checks whether the resource
  //! the URL points to actually exists.
  virtual tBool __stdcall URLExists(const achar* aURL) = 0;
};

//! Buffer encoder
struct iBufferEncoder : public iUnknown
{
  niDeclareInterfaceUUID(iBufferEncoder,0x5e522b3f,0x32d1,0x4968,0x81,0x62,0xa1,0x04,0x2f,0xa4,0x77,0x86);
  //! Get the encoder's buffer marker.
  virtual tU32 __stdcall EncodeMarker() const = 0;
  //! Get the maximum size that could be required by the specified buffer size.
  virtual tSize __stdcall EncodeMaxDestSize(tU32 anSrcSize) = 0;
  //! Encodes the specified data buffer in the specified output buffer.
  //! \return 0 on encoding error.
  virtual tSize __stdcall EncodeBuffer(tPtr apDest, tSize anDestSize, tPtr apSrc, tSize anSrcSize) = 0;
};

//! Buffer decoder
struct iBufferDecoder : public iUnknown
{
  niDeclareInterfaceUUID(iBufferDecoder,0xfe58c80f,0x8878,0x4537,0x9f,0x70,0x6d,0xb7,0x7f,0x64,0x1a,0x58);
  //! Get the decoder's buffer marker.
  virtual tU32 __stdcall DecodeMarker() const = 0;
  //! Get the maximum size that could be required by the specified buffer size.
  virtual tSize __stdcall DecodeMaxDestSize(tU32 anSrcSize) = 0;
  //! Decodes the specified data buffer in the specified output buffer.
  //! \return 0 on decoding error.
  virtual tSize __stdcall DecodeBuffer(tPtr apDest, tSize anDestSize, tPtr apSrc, tSize anSrcSize) = 0;
};

niExportFunc(ni::iFileBase*) CreateFileBaseBufferEncoder(ni::iFileBase* apBase, ni::iBufferEncoder* apEnc);
niExportFunc(ni::iFileBase*) CreateFileBaseBufferDecoder(ni::iFileBase* apBase, ni::iBufferDecoder* apDec, ni::tSize aDecSize);
niExportFunc(ni::iFile*) CreateFileBufferEncoder(ni::iFileBase* apBase, ni::iBufferEncoder* apEnc);
niExportFunc(ni::iFile*) CreateFileBufferDecoder(ni::iFileBase* apBase, ni::iBufferDecoder* apDec, ni::tSize aDecSize);
niExportFunc(ni::tSize) FileBufferEncode(ni::iBufferEncoder* apEnc, ni::iFile* apSrc, ni::tSize anSrcSize, ni::iFile* apDest);
niExportFunc(ni::tSize) FileBufferDecode(ni::iBufferDecoder* apDec, ni::iFile* apSrc, ni::tSize anSrcSize, ni::iFile* apDest, ni::tSize anDestSize);

niExportFunc(ni::iFile*) CreateFile(ni::iFileBase* apBase);
niExportFunc(ni::iFileBase*) CreateFileBaseWriteDummy();
niExportFunc(ni::iFile*) CreateFileWriteDummy();
niExportFunc(ni::iFileBase*) CreateFileBaseMemory(ni::tPtr pMem, ni::tSize nSize, ni::tBool bFree, const ni::achar* aszPath);
niExportFunc(ni::iFile*) CreateFileMemory(ni::tPtr pMem, ni::tSize nSize, ni::tBool bFree, const ni::achar* aszPath);
niExportFunc(ni::iFileBase*) CreateFileBaseMemoryAlloc(ni::tSize nSize, const ni::achar* aszPath);
niExportFunc(ni::iFile*) CreateFileMemoryAlloc(ni::tSize nSize, const ni::achar* aszPath);
niExportFunc(ni::iFileBase*) CreateFileBaseDynamicMemory(ni::tSize anSize, const ni::achar* aszPath);
niExportFunc(ni::iFile*) CreateFileDynamicMemory(ni::tSize anSize, const ni::achar* aszPath);
niExportFunc(ni::iFileBase*) CreateFileBaseWindow(ni::iFileBase* apBase, ni::tI64 anBase, ni::tI64 anSize, const ni::achar* aaszPath, ni::tBool abAutoSeekSet);
niExportFunc(ni::iFile*) CreateFileWindow(ni::iFileBase* apBase, ni::tI64 anBase, ni::tI64 anSize, const ni::achar* aaszPath, ni::tBool abAutoSeekSet);

/**
 * Provides a buffer which directly access the file's memory if its a memory
 * file, otherwise read to a temporary memory buffer.
 *
 * Example usage:
 *   sReadBufferFromFile buffer(apFile, anSize);
 *   if (!ReadBufferAndDoSomethingWithIt(buffer.begin(),buffer.size())) {
 *     return eFalse;
 *   }
 */
struct sReadBufferFromFile {
  tPtr _ownedMemory;
  tPtr _memory;
  tSize _size;
  tSize _readSize;
  sReadBufferFromFile(iFile* apFile, tSize anSize) {
    niAssert(niIsOK(apFile));
    if (!niIsOK(apFile)) {
      _size = 0;
      _readSize = 0;
      return;
    }
    _size = anSize;
    if (apFile->GetIsMemory() &&
        ((apFile->GetHere()+anSize) < apFile->GetStop()))
    {
      _ownedMemory = NULL;
      _memory = apFile->GetHere();
      apFile->Seek(_size);
      _readSize = _size;
    }
    else {
      _ownedMemory = _memory = niTMalloc(tU8,_size);
      ni::MemSet(_memory, 0, _size);
      _readSize = apFile->ReadRaw(_memory, _size);
    }
    niAssert(_memory);
  }
  ~sReadBufferFromFile() {
    niSafeFree(_ownedMemory);
  }
  const tPtr begin() const { return _memory; }
  const tSize size() const { return _size; }
  const tSize readSize() const { return _readSize; }
  const tBool readAll() const { return _readSize && (_readSize >= _size); }
};

/**
 * Provides a buffer which directly access the file's memory if its a memory
 * file, otherwise return a temporary buffer that is written to the file when
 * commit() is called or the buffer is destructed.
 *
 * Example usage:
 *   sWriteBufferToFile buffer(apFile, olen);
 *   tSize bytesWritten = WriteSomethingToBuffer(buffer.begin(),buffer.size());
 *   if (buffer.commit(bytesWritten) == bytesWritten) {
 *     ... data are now guaranteed to be written to the file ...
 *   }
 */
struct sWriteBufferToFile {
  Ptr<iFile> _file;
  tPtr _ownedMemory;
  tPtr _memory;
  tSize _size;
  sWriteBufferToFile(iFile* apFile, tSize anSize) {
    niAssert(niIsOK(apFile));
    _file = apFile;
    _size = anSize;
    if (apFile->GetIsMemory() &&
        ((apFile->GetHere()+anSize) < apFile->GetStop()))
    {
      _ownedMemory = NULL;
      _memory = apFile->GetHere();
    }
    else {
      _ownedMemory = _memory = niTMalloc(tU8,_size);
    }
    niAssert(_memory);
  }
  ~sWriteBufferToFile() {
    commit(-1);
  }

  //! Commit (write) the data to the file if its not a memory file. Can only
  //! be called successfuly once. Its automatically called by the destructor
  //! but its recommended to commit manually and check that all the data have
  //! been written.
  tSize commit(tInt aCommitSize) {
    if (!_file.IsOK()) {
      return 0;
    }
    Nonnull<iFile> fp = _file.non_null();
    const tSize writeSize = (aCommitSize >= 0) ? aCommitSize : _size;
    tSize r = 0;
    if (writeSize > 0) {
      if (_ownedMemory) {
        r = fp->WriteRaw(_ownedMemory, writeSize);
        niFree(_ownedMemory);
      }
      else {
        fp->Seek(writeSize);
        r = _size;
      }
    }
    _ownedMemory = _memory = NULL;
    _file = NULL;
    return r;
  }
  tPtr begin() { niAssert(_file.IsOK()); return _memory; }
  tSize size() const { niAssert(_file.IsOK()); return _size; }
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}; // End of ni
#endif // __NIIFILE_H__
