// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Types.h"
#include "API/niLang/Utils/Buffer.h"
#include "API/niLang/Utils/UTFImpl.h"

namespace ni {

niExportFunc(void) Buffer_ZeroMembers(sBuffer* apBuffer) {
  apBuffer->mpData = (tPtr)niMalloc(0);
  apBuffer->mnSize = 0;
  apBuffer->mnCapacity = 0;
  apBuffer->mbAdopted = eFalse;
}

niExportFunc(tBool) Buffer_IsEmpty(const sBuffer* apBuffer) {
  return apBuffer->mnSize == 0;
}

niExportFunc(tU32) Buffer_GetCapacity(const sBuffer* apBuffer) {
  return apBuffer->mnCapacity;
}

niExportFunc(tU32) Buffer_GetSize(const sBuffer* apBuffer) {
  return apBuffer->mnSize;
}

niExportFunc(tPtr) Buffer_GetData(const sBuffer* apBuffer) {
  return apBuffer->mpData;
}

niExportFunc(tBool) Buffer_IsAdopted(const sBuffer* apBuffer) {
  return apBuffer->mbAdopted;
}

//! Reserves memory for the specified number of bytes.
niExportFunc(tBool) Buffer_SetCapacity(sBuffer* apBuffer, tU32 anNewCapacity) {
  // 256MB is a lot for a buffer, it'll be out of the release build,
  // and allows to catch corrupted buffers faster
  niAssert(anNewCapacity < 1024*1024*256);

  // If the buffer has been 'adopted', the new capacity needs only to fit in the adopted capacity
  if (apBuffer->mbAdopted) {
    return Buffer_GetCapacity(apBuffer) <= anNewCapacity;
  }

  if (anNewCapacity == 0) {
    niFree(apBuffer->mpData);
    // Set the empty/zero string
    Buffer_ZeroMembers(apBuffer);
  }
  else {
    // Allocate a new memory block
    apBuffer->mpData = (tPtr)niRealloc(apBuffer->mpData, anNewCapacity);
    niAssert(apBuffer->mpData != NULL);
    apBuffer->mnCapacity = anNewCapacity;
    // Adjust size if necessary
    apBuffer->mnSize = ni::Min(apBuffer->mnSize,anNewCapacity);
  }

  return eTrue;
}

//! Clear the string
niExportFunc(void) Buffer_Clear(sBuffer* apBuffer) {
  Buffer_SetCapacity(apBuffer,0);
}

niExportFunc(tBool) Buffer_Adopt(sBuffer* apBuffer, tPtr apData,
                                 tU32 anInitialSize, tU32 anInitialCapacity)
{
  Buffer_Clear(apBuffer); // Release previous memory if any was allocated
  apBuffer->mpData = apData;
  apBuffer->mnSize = anInitialSize;
  apBuffer->mnCapacity = anInitialCapacity;
  apBuffer->mbAdopted = eTrue;
  return eTrue;
}

//! Reserves memory for the specified number of characters (not including the end zero).
//! \remark This will allocate exactly the requested amount of memory but not lower
//!         than the currently allocated size/capacity.
niExportFunc(tBool) Buffer_Reserve(sBuffer* apBuffer, tU32 anSize) {
  if (Buffer_GetCapacity(apBuffer) && (anSize <= Buffer_GetCapacity(apBuffer))) {
    return eTrue;
  }
  return Buffer_SetCapacity(apBuffer,anSize);
}

//! Set the capactity to the minimum required to contain the stored data
niExportFunc(void) Buffer_Compact(sBuffer* apBuffer) {
  Buffer_SetCapacity(apBuffer,apBuffer->mnSize);
}

//! Resize the buffer.
//! \remark This will allocate at least the requested amount of memory with alignment.
//! \remark resize never deallocates memory, use Buffer_Clear/SetCapacity/Compact() to release memory.
niExportFunc(tBool) Buffer_Resize(sBuffer* apBuffer, tU32 anSize) {
  // 256MB is a lot for a buffer, it'll be out of the release build,
  // and allows to catch corrupted buffers faster
  niAssert(anSize < 1024*1024*256);

  if (anSize > apBuffer->mnSize) {
    // Grow the buffer
    if (anSize >= Buffer_GetCapacity(apBuffer)) {
      if (apBuffer->mnSize == 0) {
        // 8 bytes min are allocated for each buffer
        if (!Buffer_Reserve(apBuffer,8))
          return eFalse;
      }
      else {
        // up the capacity by at least a 1.5 (*3/2) factor
        if (!Buffer_Reserve(apBuffer,ni::Max(anSize,(Buffer_GetCapacity(apBuffer)*3)>>1)))
          return eFalse;
      }
    }
    apBuffer->mnSize = anSize;
  }
  else if (anSize < apBuffer->mnSize) {
    // Reduce the buffer's size
    apBuffer->mnSize = anSize;
  }
  else {
    // Already the requested size
  }

  return eTrue;
}

niExportFunc(tBool) Buffer_Append(sBuffer* apBuffer, tPtr apData, tU32 anSize) {
  tU32 nPrevSize = apBuffer->mnSize;
  if (!Buffer_Resize(apBuffer,apBuffer->mnSize+anSize))
    return eFalse;
  ni::MemCopy(apBuffer->mpData+nPrevSize,apData,anSize);
  return eTrue;
}

template <typename _UTFD, typename _T, typename _UTFS>
tBool _AppendUTF(sBuffer* apBuffer, const _T* apData, tU32 anSizeInBytes, tBool abAppendZero) {
  tU32 pos = apBuffer->mnSize;
  const _T* start = apData;
  const _T* end = anSizeInBytes?apData+(anSizeInBytes/sizeof(_T)):NULL;
  const _T* p = start;
  while (!end || (p < end)) {
    const tU32 c = _UTFS::next(p);
    if (c == 0) break;
    const tU32 cw = (tU32)_UTFD::char_width(c);
    Buffer_Resize(apBuffer,apBuffer->mnSize+cw);
    _UTFD::append(c,(typename _UTFD::tChar*)(apBuffer->mpData+pos));
    pos += cw;
  }
  if (abAppendZero) {
    const tU32 c = '\0';
    const tU32 cw = (tU32)_UTFD::char_width(c);
    Buffer_Resize(apBuffer,apBuffer->mnSize+cw);
    _UTFD::append(c,(typename _UTFD::tChar*)(apBuffer->mpData+pos));
    pos += cw;
  }
  return eTrue;
}

niExportFunc(tBool) Buffer_AppendUTF8(sBuffer* apBuffer, const cchar* apData, tU32 anSizeInBytes, tBool abAppendZero) {
  return _AppendUTF<ni::utf8,ni::cchar,ni::utf8>(apBuffer,apData,anSizeInBytes,abAppendZero);
}
niExportFunc(tBool) Buffer_AppendUTF8ToUTF16(sBuffer* apBuffer, const cchar* apData, tU32 anSizeInBytes, tBool abAppendZero) {
  return _AppendUTF<ni::utf16,ni::cchar,ni::utf8>(apBuffer,apData,anSizeInBytes,abAppendZero);
}
niExportFunc(tBool) Buffer_AppendUTF8ToUTF32(sBuffer* apBuffer, const cchar* apData, tU32 anSizeInBytes, tBool abAppendZero) {
  return _AppendUTF<ni::utf32,ni::cchar,ni::utf8>(apBuffer,apData,anSizeInBytes,abAppendZero);
}

niExportFunc(tBool) Buffer_AppendUTF16(sBuffer* apBuffer, const gchar* apData, tU32 anSizeInBytes, tBool abAppendZero) {
  return _AppendUTF<ni::utf16,ni::gchar,ni::utf16>(apBuffer,apData,anSizeInBytes,abAppendZero);
}
niExportFunc(tBool) Buffer_AppendUTF16ToUTF8(sBuffer* apBuffer, const gchar* apData, tU32 anSizeInBytes, tBool abAppendZero) {
  return _AppendUTF<ni::utf8,ni::gchar,ni::utf16>(apBuffer,apData,anSizeInBytes,abAppendZero);
}
niExportFunc(tBool) Buffer_AppendUTF16ToUTF32(sBuffer* apBuffer, const gchar* apData, tU32 anSizeInBytes, tBool abAppendZero) {
  return _AppendUTF<ni::utf32,ni::gchar,ni::utf16>(apBuffer,apData,anSizeInBytes,abAppendZero);
}

niExportFunc(tBool) Buffer_AppendUTF32(sBuffer* apBuffer, const xchar* apData, tU32 anSizeInBytes, tBool abAppendZero) {
  return _AppendUTF<ni::utf32,ni::xchar,ni::utf32>(apBuffer,apData,anSizeInBytes,abAppendZero);
}
niExportFunc(tBool) Buffer_AppendUTF32ToUTF8(sBuffer* apBuffer, const xchar* apData, tU32 anSizeInBytes, tBool abAppendZero) {
  return _AppendUTF<ni::utf8,ni::xchar,ni::utf32>(apBuffer,apData,anSizeInBytes,abAppendZero);
}
niExportFunc(tBool) Buffer_AppendUTF32ToUTF16(sBuffer* apBuffer, const xchar* apData, tU32 anSizeInBytes, tBool abAppendZero) {
  return _AppendUTF<ni::utf16,ni::xchar,ni::utf32>(apBuffer,apData,anSizeInBytes,abAppendZero);
}

}
