#pragma once
#ifndef __STRINGLIB_H_8B4B4FF5_B474_47EB_B185_6F69B2E6C016__
#define __STRINGLIB_H_8B4B4FF5_B474_47EB_B185_6F69B2E6C016__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "Types.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_String
 * @{
 */

//! Get the size in bytes of the specified character.
niExportFunc(tU32) StrCharWidth(tU32 c);
//! Get the size in bytes of the next character in the specified string.
//! \remark This function is unsafe, it won't check if the parameter is valid.
niExportFunc(tU32) StrSequenceLength(const achar* s);
//! Get the number of characters between a and b.
//! \remark a and b must be pointers to the same string.
//! \remark This function is unsafe, it won't check if the parameters are valid.
niExportFunc(tU32) StrCharDistance(const achar* a, const achar* b);
//! Return the current character.
//! \remark This function is unsafe, it won't check if the parameter is valid.
niExportFunc(tU32) StrGetNext(const achar* s);
//! Return the current character and set the pointer to the next.
//! \remark This function is unsafe, it won't check if the parameter is valid.
niExportFunc(tU32) StrGetNextX(const achar** s);
//! Return the previous character.
//! \remark This function is unsafe, it won't check if the parameter is valid.
niExportFunc(tU32) StrGetPrior(const achar* s);
//! Return the previous character and set the pointer to it.
//! \remark This function is unsafe, it won't check if the parameter is valid.
niExportFunc(tU32) StrGetPriorX(const achar** s);
//! Set a character in the specified buffer. Buffer should be at least 4 bytes wide.
//! \return Size in bytes, of the character set.
//! \remark This function is unsafe, it won't check if the parameters are valid.
niExportFunc(tU32) StrSetChar(achar* s, tI32 c);
//! Set a character in the specified buffer and move the pointer after
//! it. Buffer should be at least 4 bytes wide.
//! \return Size in bytes, of the character set.
//! \remark This function is unsafe, it won't check if the parameters are valid.
niExportFunc(tU32) StrSetCharX(achar** s, tI32 c);

//! Check whether the specified code point is a valid UTF8 character.
niExportFunc(tBool) StrIsValidCodePoint(tU32 c);
//! Check whether the specified sequence is a valid UTF8 string.
niExportFunc(tBool) StrIsValidSequence(const achar* b, const achar* e);
//! Check whether the specified string is valid UTF8 string.
niExportFunc(tBool) StrIsValidString(const achar* str);

//! Returns the offset in bytes from the start of the string to the
//! character at the specified index. If the index is negative, counts
//! backward from the end of the string (-1 returns an offset to the
//! last character).
niExportFunc(tU32) StrOffset(const achar* s, tI32 idx);
//! Modifies the character at the specified index within the string,
//! handling adjustments for variable width data. Returns how far the
//! rest of the string was moved.
niExportFunc(tI32) StrSetAt(achar* s, tI32 index, tI32 c);
//! Get the character at the specified character index.
niExportFunc(tI32) StrGetAt(const achar* s, tI32 index);
//! Inserts a character at the specified index within a string, sliding
//! following data along to make room. Returns how far the data was moved.
niExportFunc(tI32) StrInsert(achar* s, tI32 idx, tI32 c);
//! Removes the character at the specified index within the string, sliding
//! following data back to make room. Returns how far the data was moved.
niExportFunc(tI32) StrRemove(achar* s, tI32 idx);
//! Utf8-aware version of the ANSI tolower() function.
niExportFunc(tI32) StrToLower(tI32 c);
//! Utf8-aware version of the ANSI toupper() function.
niExportFunc(tI32) StrToUpper(tI32 c);
//! Utf8-aware version of the ANSI islower() function.
niExportFunc(tBool) StrIsUpper(tI32 c);
//! Utf8-aware version of the ANSI isupper() function.
niExportFunc(tBool) StrIsLower(tI32 c);
//! Utf8-aware version of the ANSI isalnum() function.
niExportFunc(tBool) StrIsAlNum(tI32 c);
//! Utf8-aware version of the ANSI isspace() function.
niExportFunc(tBool) StrIsSpace(tI32 c);
//! Utf8-aware version of the ANSI isdigit() function.
niExportFunc(tBool) StrIsDigit(tI32 c);
//! Utf8-aware version of the ANSI isxdigit() function.
niExportFunc(tBool) StrIsXDigit(tI32 c);
//! Utf8-aware version of the ANSI iscntrl() function.
niExportFunc(tBool) StrIsControl(tI32 c);
//! Check whether the specified character should be considered part of a 'word'.
niExportFunc(tBool) StrIsLetter(tI32 c);
//! Check whether the specified character is a letter or a digit.
niExportFunc(tBool) StrIsLetterDigit(tI32 c);
//! Returns the number of characters in the specified utf8 string.
niExportFunc(tI32) StrLen(const achar* s);
//! Returns the size of the specified string in bytes, not including the trailing zero.
niExportFunc(tI32) StrSize(const achar* s);
//! Returns the size of the specified string in bytes, including the trailing zero.
niExportFunc(tI32) StrSizeZ(const achar* s);
//! Enhanced Utf8-aware version of the ANSI strcpy() function
//! that can handle the size (in bytes) of the destination string.
//! The raw Utf8-aware version of ANSI strcpy() is defined as:
niExportFunc(achar*) StrZCpy(achar* dest, tI32 size, const achar* src);
//! Enhanced Utf8-aware version of the ANSI strcat() function
//! that can handle the size (in bytes) of the destination string.
//! The raw Utf8-aware version of ANSI strcat() is defined as:
niExportFunc(achar*) StrZCat(achar* dest, tI32 size, const achar* src);
//! Enhanced Utf8-aware version of the ANSI strncpy() function
//! that can handle the size (in bytes) of the destination string.
//! The raw Utf8-aware version of ANSI strncpy() is defined as:
niExportFunc(achar*) StrZNCpy(achar* dest, tI32 size, const achar* src, tSize n);
//! Enhanced Utf8-aware version of the ANSI strncat() function
//! that can handle the size (in bytes) of the destination string.
//! The raw Utf8-aware version of ANSI strncat() is defined as:
niExportFunc(achar*) StrZNCat(achar* dest, tI32 size, const achar* src, tSize n);
//! Utf8-aware version of the ANSI strcmp() function.
niExportFunc(tI32) StrCmp(const achar* s1, const achar* s2);
//! Utf8-aware version of the strncmp() function.
niExportFunc(tI32) StrNCmp(const achar* s1, const achar* s2, tSize n);
//! Utf8-aware version of the strncmp() function, however takes number of bytes instead of character count.
niExportFunc(tI32) StrZCmp(const achar* s1, const achar* s2, tSize sz);
//! Utf8-aware version of the stricmp()   function.
niExportFunc(tI32) StrICmp(const achar* s1, const achar* s2);
//! Utf8-aware version of the strnicmp() function.
niExportFunc(tI32) StrNICmp(const achar* s1, const achar* s2, tSize n);
//! Utf8-aware version of the strnicmp() function, however takes number of bytes instead of character count.
niExportFunc(tI32) StrZICmp(const achar* s1, const achar* s2, tSize sz);
//! Utf8-aware version of the strlwr() function.
niExportFunc(achar*) StrLwr(achar* s);
//! Utf8-aware version of the strupr() function.
niExportFunc(achar*) StrUpr(achar* s);
//! Utf8-aware version of the strchr() function.
niExportFunc(const achar*) StrChr(const achar* s, tI32 c);
//! Utf8-aware version of the strrchr() function.
niExportFunc(const achar*) StrRChr(const achar* s, tI32 c);
//! Utf8-aware version of the strstr() function.
niExportFunc(const achar*) StrStr(const achar* s1, const achar* s2);
//! Utf8-aware version of the strrstr() function.
niExportFunc(const achar*) StrRStr(const achar* s1, const achar* s2);
//! Utf8-aware version of the strpbrk() function.
niExportFunc(const achar*) StrPBrk(const achar* s, const achar* set);
//! Utf8-aware version of the strtok() function.
//! \remark The last parameter needs to a be a pointer to a valid achar* variable.
//!     This is non-standard, but it makes StrTok thread safe by not using any global variables.
niExportFunc(achar*) StrTok(achar* s, const achar* set, achar** last);
//! Utf8-aware version of the strtol() function.
niExportFunc(tI64) StrToL(const achar* s, const achar** endp, tI32 base);
//! Utf8-aware version of the atol() function.
niExportFunc(tI64) StrAToL(const achar* s);
//! Utf8-aware version of the strtoul() function.
niExportFunc(tU64) StrToUL(const achar* s, const achar** endp, tI32 base);
//! Utf8-aware version of the atol() function.
niExportFunc(tU64) StrAToUL(const achar* s);
//! Utf8-aware version of the strtod() function.
niExportFunc(tF64) StrToD(const achar* s, const achar** endp);
//! Utf8-aware version of the atof() function.
niExportFunc(tF64) StrAToF(const achar* s);
//! Writes the specified number value as hexadecimal characters in the destination string.
//! \remark The destination string size should be at least 8*2 bytes long.
niExportFunc(void) StrValueToHexa(achar* d, tU64 value);
//! Writes the specified number of bytes as hexadecimal characters in the destination string.
//! \remark The destination string size should be at least byteCount*2.
niExportFunc(void) StrBytesToHexa(achar* d, tU8* bytes, tU32 byteCount);
//! Get a path from a 'command line' string. Gets what is between the first quote '"' pair
//! if present, else what is before the first space character. Returns the size in bytes
//! of the path copied in the destination.
niExportFunc(tI32) StrGetCommandPath(achar* dest, tU32 destMaxSize, const achar* src, tU32* apLen);
//! Return the number of code points in the specified UTF8 string. (equivalent to size for UTF8)
niExportFunc(tSize) StrCCPCount(const cchar* acszA);
//! Return the number of code points in the specified UTF16 string. (size = num cp * sizeof(tU16))
niExportFunc(tSize) StrGCPCount(const gchar* agszA);
//! Return the number of code points in the specified UTF32 string. (size = num cp * sizeof(tU32))
niExportFunc(tSize) StrXCPCount(const xchar* axszA);

niExportFunc(tBool) StrIsInt(const achar* cszStr);
niExportFunc(tI32) StrMapIndex(const tSize anLen, tI32 anIndex);
niExportFunc(tBool) StrStartsWith(const achar* aaszA, const achar* aaszB);
niExportFunc(tBool) StrStartsWithI(const achar* aaszA, const achar* aaszB);
niExportFunc(tBool) StrEndsWith(const achar* aaszA, const achar* aaszB);
niExportFunc(tBool) StrEndsWithI(const achar* aaszA, const achar* aaszB);

//! \remark All return and parameter values are in bytes, not character count.
niExportFunc(achar*) StrZSubstr(achar* apOut, const achar* aStr, tU32 anStrSz, tI32 start, tU32 size = 0xFFFF);
//! \remark All return and parameter values are in bytes, not character count.
niExportFunc(achar*) StrZSlice(achar* apOut, const achar* aStr, tU32 anStrSz, tI32 start, tI32 end = 0xFFFF);
//! \remark All return and parameter values are in bytes, not character count.
niExportFunc(achar*) StrZMid(achar* apOut, const achar* aStr, tU32 anStrSz, tU32 anFirst, tU32 size = 0xFFFF);
//! \remark All return and parameter values are in bytes, not character count.
niExportFunc(achar*) StrZLeft(achar* apOut, const achar* aStr, tU32 anStrSz, tU32 anSize);
//! \remark All return and parameter values are in bytes, not character count.
niExportFunc(achar*) StrZRight(achar* apOut, const achar* aStr, tU32 anStrSz, tU32 anSize);

//! Str find flags
enum eStrFindFlags {
  eStrFindFlags_Forward = 0,
  eStrFindFlags_Reversed = niBit(0),
  eStrFindFlags_ICmp = niBit(1),
  eStrFindFlags_ForwardI = eStrFindFlags_Forward|eStrFindFlags_ICmp,
  eStrFindFlags_ReversedI = eStrFindFlags_Reversed|eStrFindFlags_ICmp,
};

//! \remark All return and parameter values are in bytes, not character count.
niExportFunc(tI32) StrZFindEx(tU32 anFindFlags, const achar* aStr, tU32 anStrSz, const achar* aToFind, tU32 anToFindSz, tI32 anStart = eInvalidHandle);
//! \remark All return and parameter values are in bytes, not character count.
niExportFunc(tI32) StrZFindChar(const achar* aStr, tU32 anStrSz, const tU32 aChar, tI32 anStart = 0);
//! \remark All return and parameter values are in bytes, not character count.
niExportFunc(tI32) StrZRFindChar(const achar* aStr, tU32 anStrSz, const tU32 aChar, tI32 anStart = 0x7fffffff);

//! \remark All return and parameter values are in bytes, not character count.
niExportFunc(tI32) StrZAfterPos(tU32 anFindFlags, const achar* aStr, tU32 anStrSz, const achar* aToFind, tU32 anToFindSz);
//! \remark All return and parameter values are in bytes, not character count.
niExportFunc(achar*) StrZAfterEx(tU32 anFindFlags, achar* apOut, const achar* aStr, tU32 anStrSz, const achar* aToFind, tU32 anToFindSz);
//! \remark All return and parameter values are in bytes, not character count.
niExportFunc(tI32) StrZBeforePos(tU32 anFindFlags, const achar* aStr, tU32 anStrSz, const achar* aToFind, tU32 anToFindSz);
//! \remark All return and parameter values are in bytes, not character count.
niExportFunc(achar*) StrZBeforeEx(tU32 anFindFlags, achar* apOut, const achar* aStr, tU32 anStrSz, const achar* aToFind, tU32 anToFindSz);

niExportFunc(achar*) StrPutPathSep(achar *filename);
niExportFunc(achar*) StrMakeStdPath(achar* path);
niExportFunc(achar*) StrFixPath(achar* dest, const achar* path, int size);
niExportFunc(tBool)  StrIsAbsolutePath(const achar* path);

//!
//! Get next token from string *appString, where tokens are possibly-empty
//! strings separated by characters from aaszDelim.
//!
//! Writes NULs into the string at *appString to end tokens. aaszDelim need
//! not remain constant from call to call. On return, *appString points past
//! the last NUL written (if there might be further tokens), or is NULL (if
//! there are definitely no more tokens).
//!
//! If *appString is NULL, StrSep returns NULL
//!
//! Example:<pre>
//!   achar *token, *string, *tofree;
//!
//!   tofree = string = StrDupModule("value;;test;etc");
//!   while ((token = StrSep(&string, ";")) != NULL)
//!     printf("token=%s\n", token);
//!
//!   free(tofree);
//! </pre>
//!
niExportFunc(achar*) StrSep(achar **stringp, const achar *delim);

//! Same as StrSep, except that it always considers quoted strings as one
//! token and handles "" and \" quote escaping.
niExportFunc(achar*) StrSepQuoted(achar **stringp, const achar *delim, const tU32 aQuote);

//! Returns the byte index of the ':' character of a :// protocol definition
niExportFunc(tI32) StrFindProtocol(const achar* aURL);

//! Set the specified string to zero length.
static inline achar* StrZero(achar* apOut) {
  *apOut = 0;
  return apOut;
}

//! Utf8-aware version of strcpy()
static inline achar* StrCpy(achar* dest, const achar* src) {
  return StrZCpy(dest,niMaxI32,src);
}
//! Utf8-aware version of strcat()
static inline achar* StrCat(achar* dest, const achar* src) {
  return StrZCat(dest,niMaxI32,src);
}
//! Utf8-aware version of strncpy()
static inline achar* StrNCpy(achar* dest, const achar* src, tSize n) {
  return StrZNCpy(dest,niMaxI32,src,n);
}
//! Utf8-aware version of strncat()
static inline achar* StrNCat(achar* dest, const achar* src, tSize n) {
  return StrZNCat(dest,niMaxI32,src,n);
}

//! Check whether the specified string is empty.
static inline tBool StrIsEmpty(const achar* aaszA) {
  return (aaszA == NULL) || (*aaszA == 0);
}
//! Check whether the specified string is not empty.
static inline tBool StrIsNotEmpty(const achar* aaszA) {
  return (aaszA != NULL) && (*aaszA != 0);
}

static inline tI32 StrZFind(const achar* aStr, tU32 anStrSz, const achar* aToFind, tU32 anToFindSz, tI32 anStart = 0) {
  return StrZFindEx(0,aStr,anStrSz,aToFind,anToFindSz,anStart);
}
static inline tI32 StrZFindI(const achar* aStr, tU32 anStrSz, const achar* aToFind, tU32 anToFindSz, tI32 anStart = 0) {
  return StrZFindEx(eStrFindFlags_ICmp,aStr,anStrSz,aToFind,anToFindSz,anStart);
}
static inline tI32 StrZRFind(const achar* aStr, tU32 anStrSz, const achar* aToFind, tU32 anToFindSz, tI32 anStart = 0x7fffffff) {
  return StrZFindEx(eStrFindFlags_Reversed,aStr,anStrSz,aToFind,anToFindSz,anStart);
}
static inline tI32 StrZRFindI(const achar* aStr, tU32 anStrSz, const achar* aToFind, tU32 anToFindSz, tI32 anStart = 0x7fffffff) {
  return StrZFindEx(eStrFindFlags_ReversedI,aStr,anStrSz,aToFind,anToFindSz,anStart);
}

static inline achar* StrZAfter(achar* apOut, const achar* aStr, tU32 anStrSz, const achar* aToFind, tU32 anToFindSz) {
  return StrZAfterEx(0,apOut,aStr,anStrSz,aToFind,anToFindSz);
}
static inline achar* StrZAfterI(achar* apOut, const achar* aStr, tU32 anStrSz, const achar* aToFind, tU32 anToFindSz) {
  return StrZAfterEx(eStrFindFlags_ICmp,apOut,aStr,anStrSz,aToFind,anToFindSz);
}
static inline achar* StrZRAfter(achar* apOut, const achar* aStr, tU32 anStrSz, const achar* aToFind, tU32 anToFindSz) {
  return StrZAfterEx(eStrFindFlags_Reversed,apOut,aStr,anStrSz,aToFind,anToFindSz);
}
static inline achar* StrZRAfterI(achar* apOut, const achar* aStr, tU32 anStrSz, const achar* aToFind, tU32 anToFindSz) {
  return StrZAfterEx(eStrFindFlags_ReversedI,apOut,aStr,anStrSz,aToFind,anToFindSz);
}

static inline achar* StrZBefore(achar* apOut, const achar* aStr, tU32 anStrSz, const achar* aToFind, tU32 anToFindSz) {
  return StrZBeforeEx(0,apOut,aStr,anStrSz,aToFind,anToFindSz);
}
static inline achar* StrZBeforeI(achar* apOut, const achar* aStr, tU32 anStrSz, const achar* aToFind, tU32 anToFindSz) {
  return StrZBeforeEx(eStrFindFlags_ICmp,apOut,aStr,anStrSz,aToFind,anToFindSz);
}
static inline achar* StrZRBefore(achar* apOut, const achar* aStr, tU32 anStrSz, const achar* aToFind, tU32 anToFindSz) {
  return StrZBeforeEx(eStrFindFlags_Reversed,apOut,aStr,anStrSz,aToFind,anToFindSz);
}
static inline achar* StrZRBeforeI(achar* apOut, const achar* aStr, tU32 anStrSz, const achar* aToFind, tU32 anToFindSz) {
  return StrZBeforeEx(eStrFindFlags_ReversedI,apOut,aStr,anStrSz,aToFind,anToFindSz);
}

static inline tBool StrZContains(const achar* aStr, tU32 anStrSz, const achar* aSearch, tU32 anSearchSz, tI32 start) {
  return StrZFind(aStr,anStrSz,aSearch,anSearchSz,start) >= 0;
}
static inline tBool StrZContainsI(const achar* aStr, tU32 anStrSz, const achar* aSearch, tU32 anSearchSz, tI32 start) {
  return StrZFindI(aStr,anStrSz,aSearch,anSearchSz,start) >= 0;
}

static inline tBool StrIsEmptyChar(tU32 c) {
  return StrIsSpace(c) || c == '\n' || c == '\r' || c == '\t';
}

static inline tBool StrIsPathSep(const tU32 c) {
  return (c == '/' || c == '\\');
}
static inline tBool StrStartsWithDriveSep(const char* str) {
  return StrStartsWith(str, ":/") || StrStartsWith(str, ":\\");
}
static inline tBool StrStartsWithHomeExpansion(const char* aPath) {
  return (*aPath == '~') && (*(aPath+1) == '/' || *(aPath+1) == 0);
}

static inline tBool StrEq(const achar* aaszA, const achar* aaszB) {
  return StrCmp(aaszA,aaszB) == 0;
}
static inline tBool StrNEq(const achar* aaszA, const achar* aaszB, tSize anNumChars) {
  return StrNCmp(aaszA,aaszB,anNumChars) == 0;
}
static inline tBool StrIEq(const achar* aaszA, const achar* aaszB) {
  return StrICmp(aaszA,aaszB) == 0;
}
static inline tBool StrNIEq(const achar* aaszA, const achar* aaszB, tSize anNumChars) {
  return StrNICmp(aaszA,aaszB,anNumChars) == 0;
}

static inline tI32 StrMapLengthIndex(const achar* aaszA, tI32 anIndex) {
  return StrMapIndex(StrLen(aaszA),anIndex);
}
static inline tI32 StrMapSizeIndex(const achar* aaszA, tI32 anIndex) {
  return StrMapIndex(StrSize(aaszA),anIndex);
}

static inline tBool StrIsNumberPart(tU32 c, tBool bDotIsNumber niDefaultParam(niTrue)) {
  return StrIsDigit(c) || c == '-' || (bDotIsNumber && (c == '.'));
}

static inline achar* StrDupModule(const achar* aaszStr) {
  const tU32 sizez = StrSizeZ(aaszStr);
  if (sizez == 0)
    return NULL;
  achar* str = (achar*)niMalloc(sizez);
  if (str) {
    ni::MemCopy((tPtr)str,(tPtr)aaszStr,sizez);
    str[sizez-1] = 0;
  }
  return str;
}

//! Return the first non empty character in the string. NULL if there's not
//! text in the string.
niExportFunc(const achar*) StrHasText(const achar* aText);

#if niMinFeatures(15)

//! Unicode code point general character categories
enum eUCPCategory {
  eUCPCategory_Other = 0,
  eUCPCategory_Letter = 1,
  eUCPCategory_Mark = 2,
  eUCPCategory_Number = 3,
  eUCPCategory_Punctuation = 4,
  eUCPCategory_Symbol = 5,
  eUCPCategory_Separator = 6
};

//! Unicode code point particular character types
enum eUCPCharType {
  //! Control
  eUCPCharType_Cc = 0,
  //! Format
  eUCPCharType_Cf = 1,
  //! Unassigned
  eUCPCharType_Cn = 2,
  //! Private use
  eUCPCharType_Co = 3,
  //! Surrogate
  eUCPCharType_Cs = 4,
  //! Lower case letter
  eUCPCharType_Ll = 5,
  //! Modifier letter
  eUCPCharType_Lm = 6,
  //! Other letter
  eUCPCharType_Lo = 7,
  //! Title case letter
  eUCPCharType_Lt = 8,
  //! Upper case letter
  eUCPCharType_Lu = 9,
  //! Spacing mark
  eUCPCharType_Mc = 10,
  //! Enclosing mark
  eUCPCharType_Me = 11,
  //! Non-spacing mark
  eUCPCharType_Mn = 12,
  //! Decimal number
  eUCPCharType_Nd = 13,
  //! Letter number
  eUCPCharType_Nl = 14,
  //! Other number
  eUCPCharType_No = 15,
  //! Connector punctuation
  eUCPCharType_Pc = 16,
  //! Dash punctuation
  eUCPCharType_Pd = 17,
  //! Close punctuation
  eUCPCharType_Pe = 18,
  //! Final punctuation
  eUCPCharType_Pf = 19,
  //! Initial punctuation
  eUCPCharType_Pi = 20,
  //! Other punctuation
  eUCPCharType_Po = 21,
  //! Open punctuation
  eUCPCharType_Ps = 22,
  //! Currency symbol
  eUCPCharType_Sc = 23,
  //! Modifier symbol
  eUCPCharType_Sk = 24,
  //! Mathematical symbol
  eUCPCharType_Sm = 25,
  //! Other symbol
  eUCPCharType_So = 26,
  //! Line separator
  eUCPCharType_Zl = 27,
  //! Paragraph separator
  eUCPCharType_Zp = 28,
  //! Space separator
  eUCPCharType_Zs = 29
};

//! Unicode code point script identifications
enum eUCPScript {
  eUCPScript_Arabic = 0,
  eUCPScript_Armenian = 1,
  eUCPScript_Bengali = 2,
  eUCPScript_Bopomofo = 3,
  eUCPScript_Braille = 4,
  eUCPScript_Buginese = 5,
  eUCPScript_Buhid = 6,
  eUCPScript_Canadian_Aboriginal = 7,
  eUCPScript_Cherokee = 8,
  eUCPScript_Common = 9,
  eUCPScript_Coptic = 10,
  eUCPScript_Cypriot = 11,
  eUCPScript_Cyrillic = 12,
  eUCPScript_Deseret = 13,
  eUCPScript_Devanagari = 14,
  eUCPScript_Ethiopic = 15,
  eUCPScript_Georgian = 16,
  eUCPScript_Glagolitic = 17,
  eUCPScript_Gothic = 18,
  eUCPScript_Greek = 19,
  eUCPScript_Gujarati = 20,
  eUCPScript_Gurmukhi = 21,
  eUCPScript_Han = 22,
  eUCPScript_Hangul = 23,
  eUCPScript_Hanunoo = 24,
  eUCPScript_Hebrew = 25,
  eUCPScript_Hiragana = 26,
  eUCPScript_Inherited = 27,
  eUCPScript_Kannada = 28,
  eUCPScript_Katakana = 29,
  eUCPScript_Kharoshthi = 30,
  eUCPScript_Khmer = 31,
  eUCPScript_Lao = 32,
  eUCPScript_Latin = 33,
  eUCPScript_Limbu = 34,
  eUCPScript_Linear_B = 35,
  eUCPScript_Malayalam = 36,
  eUCPScript_Mongolian = 37,
  eUCPScript_Myanmar = 38,
  eUCPScript_New_Tai_Lue = 39,
  eUCPScript_Ogham = 40,
  eUCPScript_Old_Italic = 41,
  eUCPScript_Old_Persian = 42,
  eUCPScript_Oriya = 43,
  eUCPScript_Osmanya = 44,
  eUCPScript_Runic = 45,
  eUCPScript_Shavian = 46,
  eUCPScript_Sinhala = 47,
  eUCPScript_Syloti_Nagri = 48,
  eUCPScript_Syriac = 49,
  eUCPScript_Tagalog = 50,
  eUCPScript_Tagbanwa = 51,
  eUCPScript_Tai_Le = 52,
  eUCPScript_Tamil = 53,
  eUCPScript_Telugu = 54,
  eUCPScript_Thaana = 55,
  eUCPScript_Thai = 56,
  eUCPScript_Tibetan = 57,
  eUCPScript_Tifinagh = 58,
  eUCPScript_Ugaritic = 59,
  eUCPScript_Yi = 60,
  eUCPScript_Balinese = 61,
  eUCPScript_Cuneiform = 62,
  eUCPScript_Nko = 63,
  eUCPScript_Phags_Pa = 64,
  eUCPScript_Phoenicia = 65
};

//! Get the unicode properties of the specified code point.
//! \param cp is the code point (character)
//! \param apCharType: if not null, will contain the cp's character type (eUCPCharType)
//! \param apScript: if not null, will contain the cp's character script (eUCPScript)
//! \return the cp's category (eUCPCategory)
niExportFunc(tU32) StrGetUCPProps(tU32 cp, tU32* apCharType, tU32* apScript);

//! Check whether the specified character is a punctuation character.
static inline tBool StrIsPunct(tU32 ch) {
  return ni::StrGetUCPProps(ch,NULL,NULL) == ni::eUCPCategory_Punctuation;
}

//! Check whether the specified character can be considered a word for
//! auto line break purposes.
static inline tBool StrIsWordChar(tU32 ch) {
  ni::tU32 script;
  ni::tU32 cat = ni::StrGetUCPProps(ch,NULL,&script);
  return (cat == ni::eUCPCategory_Letter) &&
      (script == ni::eUCPScript_Han ||
       script == ni::eUCPScript_Hangul);
}

#endif // niMinFeatures(15)

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __STRINGLIB_H_8B4B4FF5_B474_47EB_B185_6F69B2E6C016__
