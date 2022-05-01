#ifndef __BUFFER_H_65571FD0_EE2F_4B4F_BCC3_101E419E9DDD__
#define __BUFFER_H_65571FD0_EE2F_4B4F_BCC3_101E419E9DDD__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

struct sBuffer {
  tPtr mpData;
  tU32 mnSize;
  tU32 mnCapacity;
  tBool mbAdopted;
};

niExportFunc(void) Buffer_ZeroMembers(sBuffer* apBuffer);
niExportFunc(tBool) Buffer_IsEmpty(const sBuffer* apBuffer);
niExportFunc(tU32) Buffer_GetCapacity(const sBuffer* apBuffer);
niExportFunc(tU32) Buffer_GetSize(const sBuffer* apBuffer);
niExportFunc(tPtr) Buffer_GetData(const sBuffer* apBuffer);
niExportFunc(tBool) Buffer_IsAdopted(const sBuffer* apBuffer);
niExportFunc(tBool) Buffer_SetCapacity(sBuffer* apBuffer, tU32 anNewCapacity);
niExportFunc(void) Buffer_Clear(sBuffer* apBuffer);
niExportFunc(tBool) Buffer_Adopt(sBuffer* apBuffer, tPtr apData,
                                 tU32 anInitialSize, tU32 anInitialCapacity);
niExportFunc(tBool) Buffer_Reserve(sBuffer* apBuffer, tU32 anSize);
niExportFunc(void) Buffer_Compact(sBuffer* apBuffer);
niExportFunc(tBool) Buffer_Resize(sBuffer* apBuffer, tU32 anSize);
niExportFunc(tBool) Buffer_Append(sBuffer* apBuffer, tPtr apData, tU32 anSize);

niExportFunc(tBool) Buffer_AppendUTF8(sBuffer* apBuffer, const cchar* apData, tU32 anSizeInBytes, tBool abAppendZero);
niExportFunc(tBool) Buffer_AppendUTF8ToUTF16(sBuffer* apBuffer, const cchar* apData, tU32 anSizeInBytes, tBool abAppendZero);
niExportFunc(tBool) Buffer_AppendUTF8ToUTF32(sBuffer* apBuffer, const cchar* apData, tU32 anSizeInBytes, tBool abAppendZero);

niExportFunc(tBool) Buffer_AppendUTF16(sBuffer* apBuffer, const gchar* apData, tU32 anSizeInBytes, tBool abAppendZero);
niExportFunc(tBool) Buffer_AppendUTF16ToUTF8(sBuffer* apBuffer, const gchar* apData, tU32 anSizeInBytes, tBool abAppendZero);
niExportFunc(tBool) Buffer_AppendUTF16ToUTF32(sBuffer* apBuffer, const gchar* apData, tU32 anSizeInBytes, tBool abAppendZero);

niExportFunc(tBool) Buffer_AppendUTF32(sBuffer* apBuffer, const xchar* apData, tU32 anSizeInBytes, tBool abAppendZero);
niExportFunc(tBool) Buffer_AppendUTF32ToUTF8(sBuffer* apBuffer, const xchar* apData, tU32 anSizeInBytes, tBool abAppendZero);
niExportFunc(tBool) Buffer_AppendUTF32ToUTF16(sBuffer* apBuffer, const xchar* apData, tU32 anSizeInBytes, tBool abAppendZero);

struct Buffer : public sBuffer {
  Buffer() {
    Buffer_ZeroMembers(this);
  }
  Buffer(Buffer&& other) {
    *(sBuffer*)this = *(sBuffer*)&other;
    Buffer_ZeroMembers(&other);
  }
  ~Buffer() {
    this->Clear();
  }
  tBool IsEmpty() const {
    return Buffer_IsEmpty(this);
  }
  tU32 GetCapacity() const {
    return Buffer_GetCapacity(this);
  }
  tU32 GetSize() const {
    return Buffer_GetSize(this);
  }
  tPtr GetData() const {
    return Buffer_GetData(this);
  }
  tBool IsAdopted() const {
    return Buffer_IsAdopted(this);
  }
  tBool SetCapacity(tU32 anNewCapacity) {
    return Buffer_SetCapacity(this,anNewCapacity);
  }
  void Clear() {
    Buffer_Clear(this);
  }
  tBool Adopt(tPtr apData, tU32 anInitialSize, tU32 anInitialCapacity) {
    return Buffer_Adopt(this,apData,anInitialSize,anInitialCapacity);
  }
  tBool Reserve(tU32 anSize) {
    return Buffer_Reserve(this,anSize);
  }
  void Compact() {
    Buffer_Compact(this);
  }
  tBool Resize(tU32 anSize) {
    return Buffer_Resize(this,anSize);
  }
  tBool Append(tPtr apData, tU32 anSize) {
    return Buffer_Append(this,apData,anSize);
  }
  tBool AppendUTF8(const cchar* apData, tU32 anSizeInBytes, tBool abAppendZero) {
    return Buffer_AppendUTF8(this,apData,anSizeInBytes,abAppendZero);
  }
  tBool AppendUTF8ToUTF16(const cchar* apData, tU32 anSizeInBytes, tBool abAppendZero) {
    return Buffer_AppendUTF8ToUTF16(this,apData,anSizeInBytes,abAppendZero);
  }
  tBool AppendUTF8ToUTF32(const cchar* apData, tU32 anSizeInBytes, tBool abAppendZero) {
    return Buffer_AppendUTF8ToUTF32(this,apData,anSizeInBytes,abAppendZero);
  }
  tBool AppendUTF16(const gchar* apData, tU32 anSizeInBytes, tBool abAppendZero) {
    return Buffer_AppendUTF16(this,apData,anSizeInBytes,abAppendZero);
  }
  tBool AppendUTF16ToUTF8(const gchar* apData, tU32 anSizeInBytes, tBool abAppendZero) {
    return Buffer_AppendUTF16ToUTF8(this,apData,anSizeInBytes,abAppendZero);
  }
  tBool AppendUTF16ToUTF32(const gchar* apData, tU32 anSizeInBytes, tBool abAppendZero) {
    return Buffer_AppendUTF16ToUTF32(this,apData,anSizeInBytes,abAppendZero);
  }
  tBool AppendUTF32(const xchar* apData, tU32 anSizeInBytes, tBool abAppendZero) {
    return Buffer_AppendUTF32(this,apData,anSizeInBytes,abAppendZero);
  }
  tBool AppendUTF32ToUTF8(const xchar* apData, tU32 anSizeInBytes, tBool abAppendZero) {
    return Buffer_AppendUTF32ToUTF8(this,apData,anSizeInBytes,abAppendZero);
  }
  tBool AppendUTF32ToUTF16(const xchar* apData, tU32 anSizeInBytes, tBool abAppendZero) {
    return Buffer_AppendUTF32ToUTF16(this,apData,anSizeInBytes,abAppendZero);
  }

  niClassNoCopyAssign(Buffer);
};

struct BufferUTF8 : public Buffer {
 private:
  // BufferUTF8 should be used only mostly to do a conversion, the meaning of
  // the Buffer::Append* methods don't change
  BufferUTF8() {
  }
 public:
  BufferUTF8(const cchar* apData, tU32 anSizeInBytes = 0) {
    this->AppendUTF8(apData,anSizeInBytes,eTrue);
  }
  BufferUTF8(const gchar* apData, tU32 anSizeInBytes = 0) {
    this->AppendUTF16ToUTF8(apData,anSizeInBytes,eTrue);
  }
  BufferUTF8(const xchar* apData, tU32 anSizeInBytes = 0) {
    this->AppendUTF32ToUTF8(apData,anSizeInBytes,eTrue);
  }
  const cchar* __stdcall Chars() const {
    return (const cchar*)mpData;
  }
};

struct BufferUTF16 : public Buffer {
 private:
  // BufferUTF16 should be used only mostly to do a conversion, the meaning of
  // the Buffer::Append* methods don't change
  BufferUTF16() {
  }
 public:
  BufferUTF16(const cchar* apData, tU32 anSizeInBytes = 0) {
    this->AppendUTF8ToUTF16(apData,anSizeInBytes,eTrue);
  }
  BufferUTF16(const gchar* apData, tU32 anSizeInBytes = 0) {
    this->AppendUTF16(apData,anSizeInBytes,eTrue);
  }
  BufferUTF16(const xchar* apData, tU32 anSizeInBytes = 0) {
    this->AppendUTF32ToUTF16(apData,anSizeInBytes,eTrue);
  }
  const gchar* __stdcall Chars() const {
    return (const gchar*)mpData;
  }
};

struct BufferUTF32 : public Buffer {
 private:
  // BufferUTF32 should be used only mostly to do a conversion, the meaning of
  // the Buffer::Append* methods don't change
  BufferUTF32() {
  }
 public:
  BufferUTF32(const cchar* apData, tU32 anSizeInBytes = 0) {
    this->AppendUTF8ToUTF32(apData,anSizeInBytes,eTrue);
  }
  BufferUTF32(const gchar* apData, tU32 anSizeInBytes = 0) {
    this->AppendUTF16ToUTF32(apData,anSizeInBytes,eTrue);
  }
  BufferUTF32(const xchar* apData, tU32 anSizeInBytes = 0) {
    this->AppendUTF32(apData,anSizeInBytes,eTrue);
  }
  const xchar* __stdcall Chars() const {
    return (const xchar*)mpData;
  }
};

#define niToCChars(X) ni::BufferUTF8(X).Chars()
#define niToGChars(X) ni::BufferUTF16(X).Chars()
#define niToXChars(X) ni::BufferUTF32(X).Chars()

#if niUCharSize == 1
#define niToUChars niToCChars
#elif niUCharSize == 2
#define niToUChars niToGChars
#else
#define niToUChars niToXChars
#endif

#define niToAChars niToCChars

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __BUFFER_H_65571FD0_EE2F_4B4F_BCC3_101E419E9DDD__
