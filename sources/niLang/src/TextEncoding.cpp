// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "API/niLang/Types.h"
#include "API/niLang/IFile.h"
#include "API/niLang/StringDef.h"
#include "API/niLang/StringLibIt.h"
#include "API/niLang/Utils/UTFImpl.h"
#include "TextEncoding.h"

using namespace ni;

// const ni::tU32 knUTFReplacementMarker = 0xfffd;
static const tU32 INVALID_CHAR = '?';
// static const tU32 INVALID_CHAR = 0xfffd;

//////////////////////////////////////////////////////////////////////////////////////////////
// Bit stream string

// Space table :
// 0 - end of space stm
// 1 - space
// 2 - \n
// 3 - \r
// 4 - \t
// 5 - end of the string

static void BitsStringWriteCloseMode(iFile* apFile, tI8 mode, tBool bExtIs32) {
  switch (mode) {
    case 0: { apFile->WriteBits8(0,3); break; }
    case 1: { apFile->WriteBits8(0,4); break; }
    case 2: { apFile->WriteBits8(0,6); break; }
    case 3: { apFile->WriteBits16(0,bExtIs32?10:9); break; }
    default: return;
  }
}

static void BitsStringWrite(iFile* apFile, const achar* aaszString, tBool abExtIs32) {
  tI8  currentMode = -1;  // 0 - space, 1 - eng chars, 2 - numbers, 3 - ext
  if (niIsStringOK(aaszString)) {
    ni::StrCharIt it(aaszString);
    for (;;) {
      tU32 c = it.next();
      if (c == 0) {
        break;
      }
      else if (c == 32 || c == '\n' || c == '\r' || c == '\t')
      { // mode 0 (00b) - space
        if (currentMode != 0) {
          BitsStringWriteCloseMode(apFile,currentMode,abExtIs32);
          apFile->WriteBits8(0,2);
          currentMode = 0;
        }
        switch (c) {
          case   32: apFile->WriteBits8(1,3); break;
          case '\n': apFile->WriteBits8(2,3); break;
          case '\r': apFile->WriteBits8(3,3); break;
          case '\t': apFile->WriteBits8(4,3); break;
        }
      }
      else if (c >= 43 && c <= 57)
      { // mode 1 (10b) - numbers
        if (currentMode != 1) {
          BitsStringWriteCloseMode(apFile,currentMode,abExtIs32);
          apFile->WriteBits8(1,2);
          currentMode = 1;
        }
        c -= 42;
        apFile->WriteBits8((tU8)c,4);
      }
      else if (c >= 60 && c <= 122)
      { // mode 2 (01b) - eng chars
        if (currentMode != 2) {
          BitsStringWriteCloseMode(apFile,currentMode,abExtIs32);
          apFile->WriteBits8(2,2);
          currentMode = 2;
        }
        c -= 59;
        apFile->WriteBits8((tU8)c,6);
      }
      else
      { // mode 3 (11b) - ext unicode
        if (currentMode != 3) {
          BitsStringWriteCloseMode(apFile,currentMode,abExtIs32);
          apFile->WriteBits8(3,2);
          apFile->WriteBit(abExtIs32);
          currentMode = 3;
        }
        if (abExtIs32) {
          if (c <= 0xFF) {
            apFile->WriteBits8(0,2);
            apFile->WriteBits8((tU8)c,8);
          }
          else if (c <= 0xFFFF) {
            apFile->WriteBits8(1,2);
            apFile->WriteBits16((tU16)c,16);
          }
          else if (c <= 0x00FFFFFF) {
            apFile->WriteBits8(2,2);
            apFile->WriteBits32(c,24);
          }
          else {
            apFile->WriteBits8(3,2);
            apFile->WriteBits32(c,32);
          }
        }
        else {
          if (c <= 0xFF) {
            apFile->WriteBit(eFalse);
            apFile->WriteBits8((tU8)c,8);
          }
          else {
            apFile->WriteBit(eTrue);
            apFile->WriteBits16((tU16)c,16);
          }
        }
      }
      ++aaszString;
    }
  }
  if (currentMode != 0) {
    BitsStringWriteCloseMode(apFile,currentMode,abExtIs32);
    apFile->WriteBits8(0,2);
    currentMode = 0;
    niUnused(currentMode);
  }
  apFile->WriteBits8(5,3);
}

static void BitsStringRead(iFile* apFile, ni::cString& strOut, tBool abEndOnNewLine) {
  tU32 t;
  tI8 currentMode = -1;
  tBool bExtIs32 = eFalse;
  while (!apFile->GetPartialRead()) {
    if (currentMode >= 0) {
      switch (currentMode) {
        case 0: { // mode 0 - spaces
          t = apFile->ReadBitsU8(3);
          switch (t) {
            case 0: currentMode = -1; break;
            case 1: strOut.appendChar(' ');  break;
            case 2: strOut.appendChar('\n'); if (abEndOnNewLine) goto read_end_of_string; break;
            case 3: strOut.appendChar('\r'); break;
            case 4: strOut.appendChar('\t'); break;
            case 5: goto read_end_of_string;
          }
          break;
        }
        case 1: { // mode 1 - numbers
          t = apFile->ReadBitsU8(4);
          if (t == 0) {
            currentMode = -1;
          }
          else {
            t += 42;
            strOut.appendChar(t);
          }
          break;
        }
        case 2: { // mode 2 - english chars
          t = apFile->ReadBitsU8(6);
          if (t == 0) {
            currentMode = -1;
          }
          else {
            t += 59;
            strOut.appendChar(t);
          }
          break;
        }
        case 3: { // mode 3 - extended
          if (bExtIs32) {
            t = apFile->ReadBitsU8(2);
            switch (t) {
              case 0: t = apFile->ReadBitsU8 (8); break;
              case 1: t = apFile->ReadBitsU16(16); break;
              case 2: t = apFile->ReadBitsU32(24); break;
              case 3: t = apFile->ReadBitsU32(32); break;
            }
            if (t == 0)
              currentMode = -1;
            else {
              strOut.appendChar(t);
            }
          }
          else {
            if (apFile->ReadBit() == 0) t = apFile->ReadBitsU8(8);
            else            t = apFile->ReadBitsU16(16);
            if (t == 0) currentMode = -1;
            else {
              strOut.appendChar(t);
            }
          }
          break;
        }
      }
    }
    else {
      currentMode = 0;
      apFile->ReadBits((tPtr)&currentMode,2,eFalse);
      if (currentMode == 3) bExtIs32 = apFile->ReadBit();
    }
  }
read_end_of_string:;
}

static const tU8 _kBOM_UTF8[3]      = {0xEF,0xBB,0xBF};
static const tU8 _kBOM_UTF16LE[2]   = {0xFF,0xFE};
static const tU8 _kBOM_UTF16BE[2]   = {0xFE,0xFF};
static const tU8 _kBOM_UTF32LE[4]   = {0xFF,0xFE,0x00,0x00};
static const tU8 _kBOM_UTF32BE[4]   = {0x00,0x00,0xFE,0xFF};
static const tU8 _kBOM_BITSTREAM[2] = {0xBE,0xBE};

///////////////////////////////////////////////
//! Get the text encoding format of the given file.
eTextEncodingFormat __stdcall TextEncodingFormatGet(iFile* apFile, tBool abSkipHeader, eTextEncodingFormat aDefault)
{
  if (!niIsOK(apFile))
    return eTextEncodingFormat_Unknown;
  tI64 nPos = apFile->Tell();
  tU8 header[4] = {0,0,0,0};
  if (apFile->ReadRaw(header,2) == 2) {
    if (header[0] == 0x00) {
      // can be UTF32BE only, or invalid... (not text)
      if ((apFile->ReadRaw(header+2,2) == 2) &&
          (::memcmp(header,_kBOM_UTF32BE,4) == 0))
      {
        if (!abSkipHeader) apFile->SeekSet(nPos);
        return eTextEncodingFormat_UTF32BE;
      }
      else {
        apFile->SeekSet(nPos);
        return eTextEncodingFormat_Unknown;
      }
    }
    // BitStream ?
    else if (::memcmp(header,_kBOM_BITSTREAM,2) == 0) {
      if (!abSkipHeader) apFile->SeekSet(nPos);
      return eTextEncodingFormat_BitStream;
    }
    // UTF16BE
    else if (::memcmp(header,_kBOM_UTF16BE,2) == 0) {
      if (!abSkipHeader) apFile->SeekSet(nPos);
      return eTextEncodingFormat_UTF16BE;
    }

    header[2] = apFile->Read8();
    if (::memcmp(header,_kBOM_UTF32LE,3) == 0) {
      // can be UTF32LE only, or invalid... (not text)
      header[3] = apFile->Read8();
      if (::memcmp(header,_kBOM_UTF32LE,4) == 0) {
        if (!abSkipHeader) apFile->SeekSet(nPos);
        return eTextEncodingFormat_UTF32LE;
      }
      else {
        apFile->SeekSet(nPos);
        return eTextEncodingFormat_Unknown;
      }
    }

    // UTF8
    if (::memcmp(header,_kBOM_UTF8,3) == 0) {
      if (!abSkipHeader) apFile->SeekSet(nPos);
      return eTextEncodingFormat_UTF8BOM;
    }
    // UTF16LE
    else if (::memcmp(header,_kBOM_UTF16LE,2) == 0) {
      if (!abSkipHeader) apFile->SeekSet(nPos);
      else   apFile->SeekSet(nPos+2); // we have to 'move back' one byte because we read a third byte
      // necessary to make sure its not UTF32LE
      return eTextEncodingFormat_UTF16LE;
    }
  }

  apFile->SeekSet(nPos);
  return aDefault;
}

///////////////////////////////////////////////
static inline tU32 _TextEncodingFormatReadCharUTF8(iFile* apFile) {
  cchar buf[4] = {0,0,0,0};
  buf[0] = apFile->Read8();
  int charLen = utf8::sequence_length(buf); // squence_length only needs the first byte
  if (charLen == 0 || charLen > 4)
    return INVALID_CHAR;

  if (charLen > 1) {
    niAssert(charLen > 1 && charLen <= 4);
    apFile->ReadRaw(buf+1,charLen-1);
  }

  const cchar* it = buf;
  tU32 cp = 0;
  eUTFError err = eUTFError_OK;
  switch (charLen) {
    case 1:
      err = utf8::get_sequence_1(it, buf+1, &cp);
      break;
    case 2:
      err = utf8::get_sequence_2(it, buf+2, &cp);
      break;
    case 3:
      err = utf8::get_sequence_3(it, buf+3, &cp);
      break;
    case 4:
      err = utf8::get_sequence_4(it, buf+4, &cp);
      break;
  }

  if (err == eUTFError_OK) {
    // Decoding succeeded. Now, security checks...
    if (utf_is_code_point_valid(cp)) {
      if (!utf8::is_overlong_sequence(cp,charLen)){
        // Passed! Return here.
        return cp;
      }
      else {
        err = eUTFError_OverlongSequence;
      }
    }
    else {
      err = eUTFError_InvalidCodePoint;
    }
    niUnused(err);
  }

  return INVALID_CHAR;
}
static inline tU32 _TextEncodingFormatReadCharUTF16LE(iFile* apFile) {
  tU32 cp = utf_mask16(apFile->ReadLE16());
  if (utf_is_lead_surrogate(cp)) {
    ni::tU32 trail_surrogate = utf_mask16(apFile->ReadLE16());
    cp = (cp << 10) + trail_surrogate + knUTFSurrogateOffset;
  }
  return (utf_is_code_point_valid(cp)) ? cp : INVALID_CHAR;
}
static inline tU32 _TextEncodingFormatReadCharUTF16BE(iFile* apFile) {
  tU32 cp = utf_mask16(apFile->ReadBE16());
  if (utf_is_lead_surrogate(cp)) {
    ni::tU32 trail_surrogate = utf_mask16(apFile->ReadBE16());
    cp = (cp << 10) + trail_surrogate + knUTFSurrogateOffset;
  }
  return (utf_is_code_point_valid(cp)) ? cp : INVALID_CHAR;
}
static inline tU32 _TextEncodingFormatReadCharUTF32LE(iFile* apFile) {
  tU32 cp = apFile->ReadLE32();
  return (utf_is_code_point_valid(cp)) ? cp : INVALID_CHAR;
}
static inline tU32 _TextEncodingFormatReadCharUTF32BE(iFile* apFile) {
  tU32 cp = apFile->ReadBE32();
  return (utf_is_code_point_valid(cp)) ? cp : INVALID_CHAR;
}
tU32 __stdcall TextEncodingFormatReadChar(iFile* apFile, eTextEncodingFormat aFormat) {
  if (!niIsOK(apFile)) return 0;
  switch (aFormat) {
    case eTextEncodingFormat_UTF8BOM:
    case eTextEncodingFormat_UTF8:    return _TextEncodingFormatReadCharUTF8(apFile);
    case eTextEncodingFormat_UTF16LE: return _TextEncodingFormatReadCharUTF16LE(apFile);
    case eTextEncodingFormat_UTF16BE: return _TextEncodingFormatReadCharUTF16BE(apFile);
    case eTextEncodingFormat_UTF32LE: return _TextEncodingFormatReadCharUTF32LE(apFile);
    case eTextEncodingFormat_UTF32BE: return _TextEncodingFormatReadCharUTF32BE(apFile);
    default: break;
  }
  return 0;
}

///////////////////////////////////////////////
static inline tSize _TextEncodingFormatWriteCharUTF8(iFile* apFile, tU32 ch) {
  if (!utf_is_code_point_valid(ch)) { ch = INVALID_CHAR; }
  cchar buf[4] = {0,0,0,0};
  tSize sz = utf8::append(ch,buf)-buf;
  apFile->WriteRaw(buf,sz);
  return sz;
}
static inline tSize _TextEncodingFormatWriteCharUTF16LE(iFile* apFile, tU32 ch) {
  if (!utf_is_code_point_valid(ch)) { ch = INVALID_CHAR; }
  gchar buf[2] = {0,0};
  tSize numCP = utf16::append(ch,buf)-buf;
  return apFile->WriteLE16Array((tU16*)buf,numCP);
}
static inline tSize _TextEncodingFormatWriteCharUTF16BE(iFile* apFile, tU32 ch) {
  if (!utf_is_code_point_valid(ch)) { ch = INVALID_CHAR; }
  gchar buf[2] = {0,0};
  tSize numCP = utf16::append(ch,buf)-buf;
  return apFile->WriteBE16Array((tU16*)buf,numCP);
}
static inline tSize _TextEncodingFormatWriteCharUTF32LE(iFile* apFile, tU32 ch) {
  if (!utf_is_code_point_valid(ch)) { ch = INVALID_CHAR; }
  return apFile->WriteLE32(ch);
}
static inline tSize _TextEncodingFormatWriteCharUTF32BE(iFile* apFile, tU32 ch) {
  if (!utf_is_code_point_valid(ch)) { ch = INVALID_CHAR; }
  return apFile->WriteBE32(ch);
}
tSize __stdcall TextEncodingFormatWriteChar(iFile* apFile, eTextEncodingFormat aFormat, tU32 ch) {
  if (!apFile) return 0;
  switch (aFormat) {
    case eTextEncodingFormat_UTF8BOM:
    case eTextEncodingFormat_UTF8:    return _TextEncodingFormatWriteCharUTF8(apFile,ch);
    case eTextEncodingFormat_UTF16LE: return _TextEncodingFormatWriteCharUTF16LE(apFile,ch);
    case eTextEncodingFormat_UTF16BE: return _TextEncodingFormatWriteCharUTF16BE(apFile,ch);
    case eTextEncodingFormat_UTF32LE: return _TextEncodingFormatWriteCharUTF32LE(apFile,ch);
    case eTextEncodingFormat_UTF32BE: return _TextEncodingFormatWriteCharUTF32BE(apFile,ch);
    default: break;
  }
  return 0;
}

///////////////////////////////////////////////
tBool __stdcall TextEncodingFormatWriteHeader(iFile* apFile, eTextEncodingFormat aFormat)
{
  if (!niIsOK(apFile)) return eFalse;
  tBool bRet = eTrue;
  switch (aFormat) {
    case eTextEncodingFormat_UTF8: break;
    case eTextEncodingFormat_UTF8BOM: apFile->WriteRaw(_kBOM_UTF8,3); break;
    case eTextEncodingFormat_UTF16LE: apFile->WriteRaw(_kBOM_UTF16LE,2); break;
    case eTextEncodingFormat_UTF16BE: apFile->WriteRaw(_kBOM_UTF16BE,2); break;
    case eTextEncodingFormat_UTF32LE: apFile->WriteRaw(_kBOM_UTF32LE,4); break;
    case eTextEncodingFormat_UTF32BE: apFile->WriteRaw(_kBOM_UTF32BE,4); break;
    case eTextEncodingFormat_BitStream: apFile->WriteRaw(_kBOM_BITSTREAM,2); break;
    default:
      bRet = eFalse;
      break;
  }
  return bRet;
}

///////////////////////////////////////////////
inline tSize __stdcall _TextEncodingFormatReadString(iFile* apFile, eTextEncodingFormat aFormat, cString& strOut, const tU32 aNewLineStop[2])
{
  niAssert(niIsOK(apFile));
  niCheckIsOK(apFile,0);

#define READ_LOOP(READCHAR) {                                           \
    tI64 wasPos = apFile->Tell();                                       \
    while (1) {                                                         \
      tU32 c = READCHAR(apFile);                                        \
      if (aNewLineStop[0] && c == aNewLineStop[0]) {                    \
        tI64 pos = apFile->Tell();                                      \
        tU32 t = READCHAR(apFile);                                      \
        if (t != 0 && t != aNewLineStop[1]) {                           \
          apFile->SeekSet(pos); /* rewind one character - for '\r' only case */ \
        }                                                               \
        break;                                                          \
      }                                                                 \
      else if (c == 0 || c == aNewLineStop[1] || apFile->GetPartialRead()) { \
        break;                                                          \
      }                                                                 \
      strOut.appendChar(c);                                             \
    }                                                                   \
    nRet = (tSize)(wasPos-apFile->Tell());                              \
  }

  tSize nRet = 0;
  switch (aFormat) {
    // We must have a default and at least try to read some data so that the
    // file's state behave as expected if reading the header fails. For
    // example when trying to read a string from an empty file we want to make
    // sure that the PartialRead flag is set on the file.
    default:
    case eTextEncodingFormat_UTF8:
    case eTextEncodingFormat_UTF8BOM: {
      READ_LOOP(_TextEncodingFormatReadCharUTF8);
      break;
    }
    case eTextEncodingFormat_UTF16LE: {
      READ_LOOP(_TextEncodingFormatReadCharUTF16LE);
      break;
    }
    case eTextEncodingFormat_UTF16BE: {
      READ_LOOP(_TextEncodingFormatReadCharUTF16BE);
      break;
    }
    case eTextEncodingFormat_UTF32LE: {
      READ_LOOP(_TextEncodingFormatReadCharUTF32LE);
      break;
    }
    case eTextEncodingFormat_UTF32BE: {
      READ_LOOP(_TextEncodingFormatReadCharUTF32BE);
      break;
    }
    case eTextEncodingFormat_BitStream: {
      tBool bIsReadingBitStream = !apFile->IsReadBitsBegan();
      tI64 pos = apFile->Tell();
      if (bIsReadingBitStream) apFile->BeginReadBits();
      BitsStringRead(apFile,strOut,aNewLineStop[0]!=0);
      if (bIsReadingBitStream) apFile->EndReadBits();
      nRet = (tSize)(apFile->Tell()-pos);
      break;
    }
  }

#undef READ_LOOP
  return nRet;
}
tSize __stdcall TextEncodingFormatReadString(iFile* apFile, eTextEncodingFormat aFormat, cString& strOut)
{
  static const tU32 newLineStop[2] = {0,0};
  return _TextEncodingFormatReadString(apFile,aFormat,strOut,newLineStop);
}
tSize __stdcall TextEncodingFormatReadStringLine(iFile* apFile, eTextEncodingFormat aFormat, cString& strOut)
{
  static const tU32 newLineStop[2] = {'\r','\n'};
  return _TextEncodingFormatReadString(apFile,aFormat,strOut,newLineStop);
}

///////////////////////////////////////////////
tSize __stdcall TextEncodingFormatWriteString(iFile* apFile, eTextEncodingFormat aFormat, const achar* aaszStr, tBool abWriteEndZero)
{
  niAssert(niIsOK(apFile));
  niCheckIsOK(apFile,0);

#define WRITE_LOOP(WRITECHAR,WRITEZERO) {           \
    if (!niIsStringOK(aaszStr))                     \
      return abWriteEndZero?apFile->WRITEZERO(0):0; \
    ni::StrCharIt it(aaszStr);                      \
    for(;;) {                                       \
      tU32 c = it.next();                           \
      if (!c) break;                                \
      nSize += WRITECHAR(apFile,c);                 \
    }                                               \
    if (abWriteEndZero)                             \
      nSize += apFile->WRITEZERO(0);                \
  }

  tSize nSize = 0;
  switch (aFormat) {
    case eTextEncodingFormat_UTF8:
    case eTextEncodingFormat_UTF8BOM: {
#if 1
      if (!niIsStringOK(aaszStr))
        return abWriteEndZero?apFile->Write8(0):0;
      auto size = ni::StrSize(aaszStr);
      apFile->WriteRaw((tPtr)aaszStr, size);
      if (abWriteEndZero)
        nSize += apFile->Write8(0);
#else
      WRITE_LOOP(_TextEncodingFormatWriteCharUTF8,Write8);
#endif
      break;
    }
    case eTextEncodingFormat_UTF16LE: WRITE_LOOP(_TextEncodingFormatWriteCharUTF16LE,WriteLE16); break;
    case eTextEncodingFormat_UTF16BE: WRITE_LOOP(_TextEncodingFormatWriteCharUTF16BE,WriteBE16); break;
    case eTextEncodingFormat_UTF32LE: WRITE_LOOP(_TextEncodingFormatWriteCharUTF32LE,WriteLE32); break;
    case eTextEncodingFormat_UTF32BE: WRITE_LOOP(_TextEncodingFormatWriteCharUTF32BE,WriteBE32); break;
    case eTextEncodingFormat_BitStream: {
      tBool bIsWrittingBitStream = !apFile->IsWriteBitsBegan();
      tI64 nBeg = apFile->Tell();
      if (bIsWrittingBitStream) apFile->BeginWriteBits();
      BitsStringWrite(apFile,aaszStr,eFalse);
      if (bIsWrittingBitStream) apFile->EndWriteBits();
      nSize = (tSize)(apFile->Tell()-nBeg);
      break;
    }
    default:
      break;
  }

  return nSize;

#undef WRITE_LOOP
}
