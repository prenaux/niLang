#pragma once
#ifndef __STRINGBASE_27803022_H__
#define __STRINGBASE_27803022_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "IToString.h"
#include "StringLib.h"
#include "StringLibIt.h"
#include "Utils/StringIntToStr.h"
#include "Utils/SmartPtr.h"
#include "Utils/Buffer.h"
#include "STL/memory.h"
#include "Math/MathFloat.h"

#define niCore_StringBase_CatFormat

namespace ni {

struct iExpressionContext;

/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_String
 * @{
 */

class cString
{
#define STR_INIT : mnCapacity(0), mnLen(0), mpStorage(StringEmpty<achar>())
  achar*  mpStorage;
  tU32    mnLen;
  tU32    mnCapacity;

 public:
  typedef achar        tChar;
  typedef achar        value_type;
  typedef const achar* const_iterator;
  typedef achar*     iterator;
  enum {
    npos = -1
  };

  cString() STR_INIT {
  }
  ~cString() {
    clear();
  }
  cString(const cString& str) STR_INIT  { *this = str; }
  cString(const char* aszStr, tSize aLen) STR_INIT {
    if (aLen < 0) {
      Set(aszStr);
    }
    else {
      Set(aszStr,aszStr+aLen);
    }
  }
  cString(const cchar* aszStr) STR_INIT { Set(aszStr); }
  cString(const gchar* aszStr) STR_INIT { Set(aszStr); }
  cString(const xchar* aszStr) STR_INIT { Set(aszStr); }
  cString(const cchar* pBegin, const cchar* pEnd) STR_INIT { Set(pBegin,pEnd); }
  cString(const gchar* pBegin, const gchar* pEnd) STR_INIT { Set(pBegin,pEnd); }
  cString(const xchar* pBegin, const xchar* pEnd) STR_INIT { Set(pBegin,pEnd); }
  cString(const StrCharIt& aIt) STR_INIT { Set(aIt); }
  explicit cString(const tUUID& obj) STR_INIT { Set(obj); }
  explicit cString(const iToString* obj) STR_INIT { Set(obj); }
  explicit cString(const tF64* pSrc, tSize nNum, const achar* uszSep) STR_INIT { Set(pSrc,nNum,uszSep); }
  explicit cString(const tF32* pSrc, tSize nNum, const achar* uszSep) STR_INIT { Set(pSrc,nNum,uszSep); }
  explicit cString(const tI32* pSrc, tSize nNum, const achar* uszSep) STR_INIT { Set(pSrc,nNum,uszSep); }
  template<typename TF> explicit
  cString(const sVec2<TF>& obj, const achar* aszPrefix = NULL) STR_INIT { Set(obj,aszPrefix); }
  template<typename TF> explicit
  cString(const sVec3<TF>& obj, const achar* aszPrefix = NULL) STR_INIT { Set(obj,aszPrefix); }
  template<typename TF> explicit
  cString(const sVec4<TF>& obj, const achar* aszPrefix = NULL) STR_INIT { Set(obj,aszPrefix); }
  template<typename TF> explicit
  cString(const sMatrix<TF>& obj, const achar* aszPrefix = NULL) STR_INIT { Set(obj,aszPrefix); }

  achar* data() {
    return mpStorage;
  }
  const achar* c_str() const {
    return mpStorage;
  }

  inline tU32 capacity() const { return this->mnCapacity; }
  inline tU32 length()   const { return this->mnLen; }
  inline tU32 size()     const { return length(); }
  inline tBool empty()   const { return this->mnLen == 0; }

  const achar at(const tU32 index) const {
    niAssert(index <= length());
    const achar* pReadBuffer = c_str();
    return pReadBuffer[index];
  }
  const achar operator [] (const tU32 index) const {
    niAssert(index <= length());
    const achar* pReadBuffer = c_str();
    return pReadBuffer[index];
  }

  tI32 MapIndex(tI32 anIndex) const {
    return (anIndex < 0) ? ni::Max(0,length()+anIndex) : anIndex;
  }

  //! Return a character iterator to iterate over the whole string.
  StrCharIt charIt() const {
    const achar* pReadBuffer = c_str();
    return StrCharIt(pReadBuffer,pReadBuffer+mnLen);
  }
  //! Return a character iterator to iterate over the string starting at the specified character index.
  StrCharIt charIt(const tU32 index) const {
    const achar* pReadBuffer = c_str();
    tU32 offset = StrOffset(pReadBuffer,index);
    niAssert(offset <= mnLen);
    return StrCharIt(pReadBuffer+offset,pReadBuffer+mnLen);
  }
  //! Return a character iterator to iterate over the specifed character range in the string.
  StrCharIt charIt(const tU32 index, const tU32 anCharCount) const {
    const achar* pReadBuffer = c_str();
    tU32 offset = StrOffset(pReadBuffer,index);
    niAssert(offset <= mnLen);
    tU32 rangeEnd = StrOffset(pReadBuffer+offset,anCharCount);
    niAssert(rangeEnd <= mnLen);
    return StrCharIt(pReadBuffer+offset,pReadBuffer+rangeEnd);
  }
  //! Return a character iterator to iterate over the string starting at the specified offset.
  StrCharIt charZIt(const tU32 offset) const {
    const achar* pReadBuffer = c_str();
    return StrCharIt(pReadBuffer+offset,pReadBuffer+mnLen);
  }
  //! Return a character iterator to iterate over the specifed byte range in the string.
  StrCharIt charZIt(const tU32 offset, const tU32 anSize) const {
    niAssert(offset <= mnLen);
    tU32 rangeEnd = offset+anSize;
    niAssert(rangeEnd <= mnLen);
    const achar* pReadBuffer = c_str();
    return StrCharIt(pReadBuffer+offset,pReadBuffer+rangeEnd);
  }

  cString& operator = (const cString & rhs) {
    if (this == &rhs) return *this;
    resize(rhs.length());
    if (!rhs.length()) return *this;
    achar* pWriteBuffer = data();
    ni::MemCopy((tPtr)pWriteBuffer, (tPtr)rhs.c_str(), sizeof(achar)*length());
    pWriteBuffer[length()] = 0;
    return *this;
  }
  cString&  operator += (const cString & rhs) {
    return append(rhs);
  }
  cString operator + (const cString & rhs) const {
    cString result(*this);
    result.append(rhs);
    return result;
  }
  cString&  operator += (const achar* rhs) {
    return append(rhs);
  }
  cString operator + (const achar* rhs) const {
    cString result(*this);
    result.append(rhs);
    return result;
  }

  tU32 appendChar(tU32 c) {
    const tU32 prevLen = mnLen;
    resize(length()+4); // make sure we have at least four more bytes available

    achar* pWriteBuffer = data();
    const tU32 cpWidth = StrSetChar(&pWriteBuffer[prevLen],c);
    // Adjust length to its final size
    mnLen = prevLen+cpWidth;
    pWriteBuffer[mnLen] = 0;

    return cpWidth;
  }

  const tU32 front() const {
    return StrGetNext(c_str());
  }
  const tU32 back() const {
    const achar* pReadBuffer = c_str();
    niAssert(pReadBuffer != NULL);
    if (mnLen == 0) {
      return 0;
    }
    else {
      return StrGetPrior(pReadBuffer+mnLen);
    }
  }

  cString& push_back(tU32 c) {
    appendChar(c);
    return *this;
  }
  cString& appendEx(const cchar* rhs, tU32 anSizeInBytes) {
    if (!anSizeInBytes)
      anSizeInBytes = StrSize(rhs);
    if (anSizeInBytes) {
      // we use reserve and prev buffer that we deallocate to
      // handle self-append
      tU32 prevLen = mnLen;
      tU32 prevCapacity = mnCapacity;
      achar* prev = NULL;
      resize(length()+anSizeInBytes,&prev);
      {
        achar* pWriteBuffer = data();
        ni::MemCopy((tPtr)&pWriteBuffer[prevLen],(tPtr)rhs,anSizeInBytes);
        niAssert(pWriteBuffer[mnLen] == 0);
      }
      if (prev) {
        _deallocate(prev,prevCapacity);
      }
    }
    return *this;
  }
  cString& append(const cchar* rhs) {
    return appendEx(rhs,0);
  }
  cString& append(const gchar* rhs) {
    return append(niToAChars(rhs));
  }
  cString& append(const xchar* rhs) {
    return append(niToAChars(rhs));
  }
  cString& append(const cString& rhs) {
    return appendEx(rhs.c_str(),rhs.length()*sizeof(achar));
  }

  cString& insertEx(tI32 anPos, const achar* rhs, tU32 anSizeInBytes) {
    if (!anSizeInBytes)
      anSizeInBytes = StrSize(rhs);
    if (anSizeInBytes) {
      anPos = MapIndex(anPos);
      if ((tU32)anPos >= length())
        return appendEx(rhs,anSizeInBytes); // faster and the test is necessary anyway
      // make space for the string to insert
      tU32 rightChars = length()-anPos;
      tU32 prevCapacity = mnCapacity;
      achar* prev = NULL;
      resize(length()+anSizeInBytes,&prev);

      {
        achar* pWriteBuffer = data();
        ni::MemMove((tPtr)&pWriteBuffer[anPos+anSizeInBytes],(tPtr)&pWriteBuffer[anPos],sizeof(achar)*rightChars);
        niAssert(pWriteBuffer[mnLen] == 0);
        ni::MemCopy((tPtr)&pWriteBuffer[anPos],(tPtr)rhs,sizeof(achar)*anSizeInBytes);
        niAssert(pWriteBuffer[mnLen] == 0);
      }

      if (prev) {
        _deallocate(prev,prevCapacity);
      }
    }
    return *this;
  }
  cString& insertEx(const_iterator it, const achar* rhs, tU32 anSizeInBytes) {
    return insertEx((tI32)(it-begin()),rhs,anSizeInBytes);
  }
  cString& insert(tI32 anPos, tU32 anChar) {
    achar tmp[9];
    achar* p = tmp;
    const tU32 sz = StrSetChar(p,anChar);
    p += sz; *p = 0;
    return insertEx(anPos,tmp,sz);
  }
  cString& insert(tI32 anPos, const achar* rhs) {
    return insertEx(anPos,rhs,0);
  }
  cString& insert(tI32 anPos, const cString& rhs) {
    return insertEx(anPos,rhs.c_str(),rhs.length());
  }
  cString& insert(const_iterator it, achar rhs) {
    return insert((tI32)(it-begin()),rhs);
  }
  cString& insert(const_iterator it, const achar* rhs) {
    return insert((tI32)(it-begin()),rhs);
  }
  cString& insert(const_iterator it, const cString& rhs) {
    return insert((tI32)(it-begin()),rhs);
  }

  cString substr(tI32 start, tU32 count = 0x7fffffff) const {
    start = MapIndex(start);
    if ((tU32)(start) >= length())
      return cString();
    if ((count == 0x7fffffff) || ((tU32)(start+count) >= length()))
      count = length() - start;
    cString result;
    result.Set(Chars() + start,count);
    return result;
  }

  // get a string between [start,end[ (end index not included)
  cString slice(tI32 start, tI32 end = 0x7fffffff) const {
    start = MapIndex(start);
    end = MapIndex(end);
    if (end < start) return cString();
    return substr(start,end-start);
  }

  void erase(tI32 start = 0, tI32 count = 0x7fffffff) {
    start = MapIndex(start);
    if ((tU32)(start) >= length())
      return;
    if ((count == 0x7fffffff) || ((tU32)(start+count) > length()))
      count = length() - start;
    tU32 newSize = length()-count;
    if (newSize) {
      achar* pWriteBuffer = data();
      ni::MemMove((tPtr)&pWriteBuffer[start], (tPtr)&pWriteBuffer[start+count],
                  sizeof(achar)*(length() - count - start));
      resize(length() - count);
    }
    else {
      clear();
    }
  }
  void eraseCharAt(tI32 anCharIndex) {
    const achar* pReadBuffer = c_str();
    tU32 offset = StrOffset(pReadBuffer,anCharIndex);
    niAssert(offset < this->size());
    this->erase(offset,StrSequenceLength(pReadBuffer+offset));
  }

  tI32 find(const achar* str, tU32 anSize = 0, tI32 start = 0) const {
    return StrZFind(Chars(),size(),str,anSize,start);
  }
  tI32 find(const cString& str, tI32 start = 0) const {
    return find(str.Chars(),str.size(), start);
  }
  tI32 ifind(const achar* str, tU32 anSize = 0, tI32 start = 0) const {
    return StrZFindI(Chars(),size(),str,anSize,start);
  }
  tI32 ifind(const cString& str, tI32 start = 0) const {
    return ifind(str.Chars(), str.size(), start);
  }
  tI32 rfind(const achar* str, tU32 anSize = 0, tI32 start = 0x7fffffff) const {
    return StrZRFind(Chars(),size(),str,anSize,start);
  }
  tI32 rfind(const cString& str, tI32 start = 0x7fffffff) const {
    return rfind(str.Chars(),str.size(),start);
  }
  tI32 irfind(const achar* str, tU32 anSize = 0, tI32 start = 0x7fffffff) const {
    return StrZRFindI(Chars(),size(),str,anSize,start);
  }
  tI32 irfind(const cString& str, tI32 start = 0x7fffffff) const {
    return irfind(str.Chars(),str.size(),start);
  }
  tBool contains(const cString & str, tI32 start = 0) const {
    return find(str,start) >= 0;
  }
  tBool contains(const achar* str, tU32 anSize = 0, tI32 start = 0) const {
    return find(str,anSize,start) >= 0;
  }
  tBool icontains(const cString & str, tI32 start = 0) const {
    return ifind(str,start) >= 0;
  }
  tBool icontains(const achar* str, tU32 anSize = 0, tI32 start = 0) const {
    return ifind(str,anSize,start) >= 0;
  }

  tI32 find(tU32 c, tI32 start = 0) const {
    return StrZFindChar(Chars(),size(),c,start);
  }
  tI32 rfind(tU32 c, tI32 start = 0x7fffffff) const {
    return StrZRFindChar(Chars(),size(),c,start);
  }
  tBool contains(tU32 c, tI32 start = 0) const {
    return find(c,start) >= 0;
  }

  // do a strcmp
  inline tI32 cmp(const achar* aszRight) const {
    return StrCmp(Chars(),aszRight);
  }
  inline tI32 cmp(const cString &str) const {
    return cmp(str.Chars());
  }
  // do a strncmp
  inline tI32 ncmp(const achar* aszRight, tU32 anNumChars) const {
    return StrNCmp(Chars(),aszRight,anNumChars);
  }
  inline tI32 ncmp(const cString& str, tU32 anNumChars) const {
    return ncmp(str.Chars(),anNumChars);
  }
  // do a stricmp
  inline tI32 icmp(const achar* aszRight) const {
    return StrICmp(Chars(),aszRight);
  }
  inline tI32 icmp(const cString &str) const {
    return icmp(str.Chars());
  }
  // do a strnicmp
  inline tI32 nicmp(const achar* aszRight, tU32 anNumChars) const {
    return StrNICmp(Chars(),aszRight,anNumChars?anNumChars:NumChars());
  }
  inline tI32 nicmp(const cString &str, tU32 anNumChars) const {
    return nicmp(str.Chars(),anNumChars);
  }

  //! Allocate memory from the string's allocator
  static achar* _allocate(tU32 size) {
    return (achar*)niMalloc(size);
  }
  //! Deallocate memory allocated with the string's allocator
  static void _deallocate(achar* ptr, tU32 size) {
    niFree(ptr);
  }

  //! Reserves memory for the specified number of characters.
  //! \remark If appPrevBuf is not NULL memory is never deallocated and the pointer is
  //!         set to the previous memory pointer if it should be deallocated.
  //! \remark This include the end zero.
  void SetCapacity(tU32 anNewCapacity, achar** appPrevBuf = NULL, tBool abDontPreserveContent = eFalse) {
    niAssert(anNewCapacity < 1024*1024*16); // 16MB is a lot for a string, it'll be out of the
    // release build, and allows to catch corrupted
    // strings faster

    achar* pPrevBuf = data();
    tU32 nPrevLen = mnLen;
    tU32 nPrevCapacity = mnCapacity;
    mnCapacity = anNewCapacity;
    if (mnCapacity == 0) {
      // Set the empty/zero string
      mnLen = 0;
      mpStorage = StringEmpty<achar>();
    }
    else {
      // Allocate a new memory block
      mpStorage = _allocate(mnCapacity);
      niAssert(mpStorage != NULL);
      // Readjust len if necessary
      mnLen = ni::Min(mnLen,mnCapacity-1); // capacity-1 need one spot for the end zero character
      // Set the end zero
      mpStorage[mnLen] = 0;
      // Was some data in the string ?
      if (nPrevLen && !abDontPreserveContent) {
        // Copy previous data if necessary
        ni::MemCopy((tPtr)mpStorage,(tPtr)pPrevBuf,ni::Min(nPrevLen,mnLen)*sizeof(achar));
      }
    }
    // Need to deallocate ?
    if (nPrevCapacity) {
      if (appPrevBuf) {
        *appPrevBuf = pPrevBuf;
      }
      else {
        _deallocate(pPrevBuf,nPrevCapacity);
      }
    }
  }

  //! Reserves memory for the specified number of characters (not including the end zero).
  //! \remark This will allocate exactly the requested amount of memory but not lower
  //!         than the currently allocated size/capacity.
  void reserve(tU32 anNumChars, achar** appPrevBuf = NULL) {
    if (mnCapacity && (anNumChars <= (mnCapacity-1))) {
      return;
    }
    SetCapacity(anNumChars+1,appPrevBuf);
  }

  //! Clear the string
  void clear() {
    SetCapacity(0,NULL);
  }

  //! Set the capactity to the minimum required to contain the stored string
  void compact() {
    SetCapacity(mnLen+1,NULL);
  }

  //! Resize the string, sets the end character to zero.
  //! \remark This will allocate at least the requested amount of memory with alignment.
  //! \remark resize never deallocates memory, use clear() to release the memory
  void resize(const tU32 len, achar** appPrevBuf = NULL)
  {
    niAssert(len < 1024*1024*16); // 16MB is a lot for a string, it'll be out of the
    // release build, and allows to catch corrupted
    // strings faster
    if (len > mnLen) {
      // Grow the string
      if (len >= mnCapacity) {
        if (mnLen == 0) {
          // 7 chars (8 with end zero) min are allocated for each string
          reserve(ni::Max(len,7),appPrevBuf);
        }
        else {
          // up the capacity by at least a 1.5 (*3/2) factor
          reserve(ni::Max(len,(mnCapacity*3)>>1),appPrevBuf);
        }
      }
      mnLen = len;
      {
        achar* pWriteBuffer = data();
        pWriteBuffer[mnLen] = 0;
      }
    }
    else if (len < mnLen) {
      // Reduce the string's size
      mnLen = len;
      {
        achar* pWriteBuffer = data();
        pWriteBuffer[mnLen] = 0;
      }
    }
    else {
      // Already the requested size
    }
  }

  iterator begin() {
    achar* str = data();
    return &str[0];
  }
  iterator end() {
    achar* str = data();
    // last character, can't be accessed, should always be zero
    niAssert(str[mnLen] == 0);
    return &str[mnLen];
  }

  const_iterator begin() const {
    const achar* str = c_str();
    return &str[0];
  }
  const_iterator end() const {
    const achar* str = c_str();
    // last character, can't be accessed, should always be zero
    niAssert(str[mnLen] == 0);
    return &str[mnLen];
  }

  // clear the string content and preallocate nSize bytes
  void Clear(tSize anReserve = 0) { niUnused(anReserve); return clear(); }

  // return the string's length
  tU32 Len() const      { return length(); }
  tU32 Length() const   { return length(); }
  tU32 Size() const     { return length(); }
  tU32 NumChars() const { return StrLen(Chars()); }

  // return eTrue if the string is empty
  tBool IsEmpty() const;

  // return eTrue if the string is not empty
  tBool IsNotEmpty() const;

  // return the string content
  const achar* Chars() const { return c_str(); }

  // Remove a cchar at the given position
  const achar* Remove(tU32 lPos = eInvalidHandle);

  // return the string without begining and end quote
  cString GetWithoutBEQuote() const;

  // strip quotes from a string, the same way as bash does it for arguments
  cString StripQuotes() const;

  // equal
  tBool Eq(const achar* str) const { return !!StrEq(c_str(),str); }
  tBool Eq(const cString& str) const { return !!StrEq(c_str(),str.c_str()); }
  tBool IEq(const achar* str) const { return !!StrIEq(c_str(),str); }
  tBool IEq(const cString& str) const { return !!StrIEq(c_str(),str.c_str()); }

  // == operators
  bool operator == (const achar* str)   const { return !!Eq(str); }
  bool operator == (const cString& str) const { return !!Eq(str); }
  // != operators
  bool operator != (const achar* str) const   { return !Eq(str); }
  bool operator != (const cString& str) const { return !Eq(str); }
  // >= operators
  bool operator >= (const achar* str) const   { return cmp(str) >= 0; }
  bool operator >= (const cString& str) const { return cmp(str) >= 0; }
  // <= operators
  bool operator <= (const achar* str) const   { return cmp(str) <= 0; }
  bool operator <= (const cString& str) const { return cmp(str) <= 0; }
  // > operators
  bool operator > (const achar* str) const    { return cmp(str) > 0; }
  bool operator > (const cString& str) const  { return cmp(str) > 0; }
  // < operators
  bool operator < (const achar* str) const    { return cmp(str) < 0; }
  bool operator < (const cString& str) const  { return cmp(str) < 0; }

  // Convert to lower case
  const achar* ToLower() { return StrLwr(data()); }
  // Convert to upper case
  const achar* ToUpper() { return StrUpr(data()); }

  const achar* TrimRightEx(const achar* pszTargets);
  const achar* TrimRight();

  const achar* TrimLeftEx(const achar* pszTargets);
  const achar* TrimLeft();

  const achar* TrimEx(const achar* aszTargets);
  const achar* Trim();

  const achar* Normalize();

  tBool StartsWith(const achar* aszStr) const  { return StrStartsWith(Chars(),aszStr);  }
  tBool StartsWithI(const achar* aszStr) const { return StrStartsWithI(Chars(),aszStr); }
  tBool EndsWith(const achar* aszStr) const    { return StrEndsWith(Chars(),aszStr);  }
  tBool EndsWithI(const achar* aszStr) const   { return StrEndsWithI(Chars(),aszStr); }

  cString Left(tU32 anCount) const {
    return Mid(0,anCount);
  }
  cString Right(tU32 anCount) const {
    const tU32 thisLen = mnLen;
    anCount = ni::Min(anCount,thisLen);
    return Mid(thisLen-anCount,anCount);
  }
  cString Mid(tU32 anFirst, tU32 anCount = eInvalidHandle) const {
    if (anFirst >= this->length())
      return AZEROSTR;
    cString str;
    const achar* pReadBuffer = c_str();
    const achar* b = pReadBuffer+StrOffset(pReadBuffer,anFirst);
    const achar* e = pReadBuffer+mnLen;
    while (anCount-- && b < e) {
      tU32 c = StrGetNextX(&b);
      str.appendChar(c);
    }
    return str;
  }

  cString ZLeft(tU32 anBytes) const {
    return ZMid(0,anBytes);
  }
  cString ZRight(tU32 anBytes) const {
    anBytes = ni::Min(anBytes,size());
    return ZMid(size()-anBytes,anBytes);
  }
  cString ZMid(tU32 anFirstByte, tU32 anBytes = eInvalidHandle) const {
    if (anFirstByte >= this->size())
      return AZEROSTR;
    cString str;
    const achar* pReadBuffer = c_str();
    const achar* b = pReadBuffer+anFirstByte;
    const achar* e = pReadBuffer+mnLen;
    while (anBytes && b < e) {
      tU32 c = StrGetNextX(&b);
      anBytes -= str.appendChar(c);
    }
    return str;
  }

  cString AfterEx(tU32 anFindFlags, const achar* aToFind, tU32 anToFindSz = 0) const {
    tI32 afterPos = StrZAfterPos(anFindFlags,Chars(),size(),aToFind,anToFindSz);
    if (afterPos == -1) return AZEROSTR;
    return ZRight(size()-afterPos);
  }
  cString AfterEx(tU32 anFindFlags, const cString& str) const {
    return AfterEx(anFindFlags,str.Chars(),str.size());
  }

  cString BeforeEx(tU32 anFindFlags, const achar* aToFind, tU32 anToFindSz = 0) const {
    tI32 beforePos = StrZBeforePos(anFindFlags,Chars(),size(),aToFind,anToFindSz);
    if (beforePos == -1) return AZEROSTR;
    return ZLeft(beforePos);
  }
  cString BeforeEx(tU32 anFindFlags, const cString& str) const {
    return BeforeEx(anFindFlags,str.Chars(),str.size());
  }

  cString After(const achar* aToFind, tU32 anToFindSz = 0) const {
    return AfterEx(0,aToFind,anToFindSz);
  }
  cString After(const cString& str) const {
    return After(str.Chars(),str.size());
  }
  cString RAfter(const achar* aToFind, tU32 anToFindSz = 0) const {
    return AfterEx(eStrFindFlags_Reversed,aToFind,anToFindSz);
  }
  cString RAfter(const cString& str) const {
    return RAfter(str.Chars(),str.size());
  }
  cString AfterI(const achar* aToFind, tU32 anToFindSz = 0) const {
    return AfterEx(eStrFindFlags_ICmp,aToFind,anToFindSz);
  }
  cString AfterI(const cString& str) const {
    return AfterI(str.Chars(),str.size());
  }
  cString RAfterI(const achar* aToFind, tU32 anToFindSz = 0) const {
    return AfterEx(eStrFindFlags_ReversedI,aToFind,anToFindSz);
  }
  cString RAfterI(const cString& str) const {
    return RAfterI(str.Chars(),str.size());
  }

  cString Before(const achar* aToFind, tU32 anToFindSz = 0) const {
    return BeforeEx(0,aToFind,anToFindSz);
  }
  cString Before(const cString& str) const {
    return Before(str.Chars(),str.size());
  }
  cString RBefore(const achar* aToFind, tU32 anToFindSz = 0) const {
    return BeforeEx(eStrFindFlags_Reversed,aToFind,anToFindSz);
  }
  cString RBefore(const cString& str) const {
    return RBefore(str.Chars(),str.size());
  }
  cString BeforeI(const achar* aToFind, tU32 anToFindSz = 0) const {
    return BeforeEx(eStrFindFlags_ICmp,aToFind,anToFindSz);
  }
  cString BeforeI(const cString& str) const {
    return BeforeI(str.Chars(),str.size());
  }
  cString RBeforeI(const achar* aToFind, tU32 anToFindSz = 0) const {
    return BeforeEx(eStrFindFlags_ReversedI,aToFind,anToFindSz);
  }
  cString RBeforeI(const cString& str) const {
    return RBeforeI(str.Chars(),str.size());
  }

  // conversion to usual types
  tF32  Float()           const;  // return the float in the string
  tF64  Double()              const;  // return the float in the string
  tI32  Long()            const;  // return the long in the string
  tU32  ULong()           const;  // return the unsigned long in the string
  tI64  LongLong()          const;  // return the long long in the string
  tU64  ULongLong()         const;  // return the unsigned long long in the string
  tBool Bool(tBool bDefault = eFalse) const;  // return a tBool (1/0 or eTrue/eFalse) in the string
  tI32  NumberArray(tF64* pDest, tSize nNum, tBool bDotIsNumber, const achar* aaszSkipToFirst) const;
  tI32  NumberArray(tF32* pDest, tSize nNum, tBool bDotIsNumber, const achar* aaszSkipToFirst) const;
  tI32  NumberArray(tI32* pDest, tSize nNum, tBool bDotIsNumber, const achar* aaszSkipToFirst) const;
  tU32  FourCC()              const;
  tU32  Color()           const;
  tUUID UUID()                        const;
  tVersion Version()            const;  // return a version number from a x.x.x version string

  static tUUID _ToUUID(const achar* szUUID, const tSize baseLen);

  template<typename TF> sVec2<TF>  Vec2() const {
    const achar skipTo[2] = { '(', 0 };
    sVec2<TF> r;
    NumberArray(r.ptr(),2,eTrue,skipTo);
    return r;
  }
  template<typename TF> sVec3<TF>  Vec3() const {
    const achar skipTo[2] = { '(', 0 };
    sVec3<TF> r;
    NumberArray(r.ptr(),3,eTrue,skipTo);
    return r;
  }
  template<typename TF> sVec4<TF>  Vec4() const {
    const achar skipTo[2] = { '(', 0 };
    sVec4<TF> r;
    NumberArray(r.ptr(),4,eTrue,skipTo);
    return r;
  }
  template<typename TF> sMatrix<TF>   Matrix()  const {
    const achar skipTo[2] = { '(', 0 };
    sMatrix<TF> r;
    NumberArray(r.ptr(),16,eTrue,skipTo);
    return r;
  }

  // direct setting of the string
  const achar* SetFourCC(tU32 lFourCC);
  const achar* SetColor(tU32 ulColor);
  const achar* Set(tI32 lValue);
  const achar* Set(tU32 lValue);
  const achar* Set(tI64 lValue);
  const achar* Set(tU64 lValue);
  const achar* Set(tF32 dValue);
  const achar* Set(tF64 dValue);
  const achar* Set(tBool bValue);
  const achar* Set(const tF64* pSrc, tSize nNum, const achar* uszSeparator);
  const achar* Set(const tF32* pSrc, tSize nNum, const achar* uszSeparator);
  const achar* Set(const tI32* pSrc, tSize nNum, const achar* uszSeparator);
  const achar* Set(const cchar* pBegin, const cchar* pEnd);
  const achar* Set(const gchar* pBegin, const gchar* pEnd);
  const achar* Set(const xchar* pBegin, const xchar* pEnd);
  const achar* Set(const cchar* pBegin, tSize anCPCount) { return Set(pBegin,pBegin+anCPCount); }
  const achar* Set(const gchar* pBegin, tSize anCPCount) { return Set(pBegin,pBegin+anCPCount); }
  const achar* Set(const xchar* pBegin, tSize anCPCount) { return Set(pBegin,pBegin+anCPCount); }
  const achar* Set(const cchar* pBegin) { return Set(pBegin,(tU32)StrCCPCount(pBegin)); }
  const achar* Set(const gchar* pBegin) { return Set(pBegin,(tU32)StrGCPCount(pBegin)); }
  const achar* Set(const xchar* pBegin) { return Set(pBegin,(tU32)StrXCPCount(pBegin)); }
  const achar* Set(const StrCharIt& aIt) { return Set(aIt.start(),aIt.start()+aIt.sizeInBytes()); }

  const achar* Set(const iToString* aVal);
  const achar* Set(const tUUID& aUUID);

#if defined niTypeIntIsOtherType
  const achar* Set(signed int v) { return Set((tI64)v); }
  const achar* Set(unsigned int v) { return Set((tU64)v); }
#endif
#if defined niTypeIntPtrIsOtherType
  const achar* Set(tIntPtr v) { return Set((tI64)v); }
  const achar* Set(tUIntPtr v) { return Set((tU64)v); }
#endif

#if niMinFeatures(20)
  const achar* SetDouble(
      tF64 dValue,
      eDoubleToStringMode aMode = eDoubleToStringMode_ShortestDouble,
      tI32 aDigitsOrPrecision = 15,
      const char* aInfinitySymbol = "Infinity",
      const char* aNaNSymbol = "NaN",
      char aExponentCharacter = 'e');
#endif

  template<typename TF>
  const achar* Set(const sVec2<TF>& obj, const achar* aszPrefix = NULL) {
    if (aszPrefix)  *this += aszPrefix;
    else      *this += cString("Vec2");
    this->appendChar('(');
    niLoop(i,2) {
      *this << obj[i];
      if (i+1 < 2) this->appendChar(',');
    }
    this->appendChar(')');
    return Chars();
  }
  template<typename TF>
  const achar* Set(const sVec3<TF>& obj, const achar* aszPrefix = NULL) {
    if (aszPrefix)  *this += aszPrefix;
    else      *this += cString("Vec3");
    this->appendChar('(');
    niLoop(i,3) {
      *this << obj[i];
      if (i+1 < 3) this->appendChar(',');
    }
    this->appendChar(')');
    return Chars();
  }
  template<typename TF>
  const achar* Set(const sVec4<TF>& obj, const achar* aszPrefix = NULL) {
    if (aszPrefix)  *this += aszPrefix;
    else      *this += cString("Vec4");
    this->appendChar('(');
    niLoop(i,4) {
      *this << obj[i];
      if (i+1 < 4) this->appendChar(',');
    }
    this->appendChar(')');
    return Chars();
  }
  template<typename TF>
  const achar* Set(const sMatrix<TF>& obj, const achar* aszPrefix = NULL) {
    if (aszPrefix)  *this += aszPrefix;
    else      *this += cString("Matrix");
    this->appendChar('(');
    niLoop(i,16) {
      *this << obj[i];
      if (i+1 < 16) this->appendChar(',');
    }
    this->appendChar(')');
    return Chars();
  }

  // << opertaor
  cString& operator << (tI8 lVal);
  cString& operator << (tU8 lVal);
  cString& operator << (tI16 lVal);
  cString& operator << (tU16 lVal);
  cString& operator << (tI32 lVal);
  cString& operator << (tU32 lVal);
  cString& operator << (tI64 lVal);
  cString& operator << (tU64 lVal);
  cString& operator << (tF32 fVal);
  cString& operator << (tF64 fVal);
  cString& operator << (const cchar* uszChar);
  cString& operator << (const gchar* szChar);
  cString& operator << (const xchar* szChar);
  cString& operator << (const iToString* obj);
  cString& operator << (const cString& str) { *this += str; return *this; }
  cString& operator << (const tUUID& uuidVal);

#if defined niTypeIntIsOtherType
  cString& operator << (signed int v) {
    this->append(cString().Set((tI64)v));
    return *this;
  }
  cString& operator << (unsigned int v) {
    this->append(cString().Set((tU64)v));
    return *this;
  }
#endif
#if defined niTypeIntPtrIsOtherType
  cString& operator << (tIntPtr v) {
    this->append(cString().Set((tI64)v));
    return *this;
  }
  cString& operator << (tUIntPtr v) {
    this->append(cString().Set((tU64)v));
    return *this;
  }
#endif

  template<typename TF>
  cString& operator << (const sVec2<TF>& obj) { *this += cString(obj); return *this; }
  template<typename TF>
  cString& operator << (const sVec3<TF>& obj) { *this += cString(obj); return *this; }
  template<typename TF>
  cString& operator << (const sVec4<TF>& obj) { *this += cString(obj); return *this; }
  template<typename TF>
  cString& operator << (const sMatrix<TF>& obj) { *this += cString(obj); return *this; }

  static const achar* ConstEmpty() {
    return StringEmpty<achar>();
  }
  static const achar* ConstUUIDFmtUpper() {
    static const achar fmt[] = {
      '%','0','8','X','-',
      '%','0','4','X','-',
      '%','0','4','X','-',
      '%','0','2','X','%','0','2','X','-',
      '%','0','2','X',
      '%','0','2','X',
      '%','0','2','X',
      '%','0','2','X',
      '%','0','2','X',
      '%','0','2','X',0
    };
    return fmt;
  }
  static const achar* ConstUUIDFmtLower() {
    static const achar fmt[] = {
      '%','0','8','x','-',
      '%','0','4','x','-',
      '%','0','4','x','-',
      '%','0','2','x','%','0','2','x','-',
      '%','0','2','x',
      '%','0','2','x',
      '%','0','2','x',
      '%','0','2','x',
      '%','0','2','x',
      '%','0','2','x',0
    };
    return fmt;
  }
  static const achar* ConstUUIDFmt() {
    return ConstUUIDFmtUpper();
  }
  static const achar* ConstTrue() {
    static const achar fmt[] = { 't','r','u','e',0 };
    return fmt;
  }
  static const achar* ConstFalse() {
    static const achar fmt[] = { 'f','a','l','s','e',0 };
    return fmt;
  }
  static const achar* ConstYes() {
    static const achar fmt[] = { 'y','e','s',0 };
    return fmt;
  }
  static const achar* ConstNo() {
    static const achar fmt[] = { 'n','o',0 };
    return fmt;
  }
  static const achar* ConstOn() {
    static const achar fmt[] = { 'o','n',0 };
    return fmt;
  }
  static const achar* ConstOff() {
    static const achar fmt[] = { 'o','f','f',0 };
    return fmt;
  }
  static const achar* ConstTrim() {
    static const achar fmt[] = { '\r','\n','\t',' ',0 };
    return fmt;
  }
  static const achar* ConstFmtFloat() {
    static const achar fmt[] = { '%','g',0 };
    return fmt;
  }
  static const achar* ConstSharpNull() {
    static const achar fmt[] = { '#','N','u','l','l','#',0 };
    return fmt;
  }

  const achar* CatFormat(const tChar* aaszFormat);
  const achar* Format(const tChar* aaszFormat) {
    *this = aaszFormat;
    return this->Chars();
  }
  const achar* CatFormat(const tChar* aaszFormat, const Var& a1);
  const achar* Format(const tChar* aaszFormat, const Var& a1) {
    this->clear();
    return this->CatFormat(aaszFormat,a1);
  }
  const achar* CatFormat(const tChar* aaszFormat, const Var& a1, const Var& a2,
                         const Var& a3 = niVarNull, const Var& a4 = niVarNull);
  const achar* Format(const tChar* aaszFormat, const Var& a1, const Var& a2,
                      const Var& a3 = niVarNull, const Var& a4 = niVarNull)
  {
    this->clear();
    return this->CatFormat(aaszFormat,a1,a2,a3,a4);
  }
  const achar* CatFormat(const tChar* aaszFormat,
                         const Var& a1, const Var& a2, const Var& a3,
                         const Var& a4, const Var& a5, const Var& a6 = niVarNull,
                         const Var& a7 = niVarNull, const Var& a8 = niVarNull);
  const achar* Format(const tChar* aaszFormat,
                      const Var& a1, const Var& a2, const Var& a3,
                      const Var& a4, const Var& a5, const Var& a6 = niVarNull,
                      const Var& a7 = niVarNull, const Var& a8 = niVarNull)
  {
    this->clear();
    return this->CatFormat(aaszFormat,a1,a2,a3,a4,a5,a6,a7,a8);
  }
  const achar* CatFormat(
      const tChar* aaszFormat,
      const Var& a1, const Var& a2, const Var& a3,
      const Var& a4, const Var& a5, const Var& a6,
      const Var& a7, const Var& a8, const Var& a9,
      const Var& a10 = niVarNull, const Var& a11 = niVarNull, const Var& a12 = niVarNull,
      const Var& a13 = niVarNull, const Var& a14 = niVarNull, const Var& a15 = niVarNull,
      const Var& a16 = niVarNull, const Var& a17 = niVarNull, const Var& a18 = niVarNull,
      const Var& a19 = niVarNull, const Var& a20 = niVarNull, const Var& a21 = niVarNull,
      const Var& a22 = niVarNull, const Var& a23 = niVarNull, const Var& a24 = niVarNull);
  const achar* Format(
      const tChar* aaszFormat,
      const Var& a1, const Var& a2, const Var& a3,
      const Var& a4, const Var& a5, const Var& a6,
      const Var& a7, const Var& a8, const Var& a9,
      const Var& a10 = niVarNull, const Var& a11 = niVarNull, const Var& a12 = niVarNull,
      const Var& a13 = niVarNull, const Var& a14 = niVarNull, const Var& a15 = niVarNull,
      const Var& a16 = niVarNull, const Var& a17 = niVarNull, const Var& a18 = niVarNull,
      const Var& a19 = niVarNull, const Var& a20 = niVarNull, const Var& a21 = niVarNull,
      const Var& a22 = niVarNull, const Var& a23 = niVarNull, const Var& a24 = niVarNull)
  {
    this->clear();
    return this->CatFormat(aaszFormat,
                           a1,a2,a3,a4,a5,a6,a7,a8,
                           a9,a10,a11,a12,a13,a14,a15,a16,
                           a17,a18,a19,a20,a21,a22,a23,a24);
  }

  const achar* CatFormat(iExpressionContext* apExprCtx, const tChar* aaszFormat);
  const achar* Format(iExpressionContext* apExprCtx, const tChar* aaszFormat) {
    this->clear();
    return this->CatFormat(apExprCtx,aaszFormat);
  }
  const achar* CatFormat(iExpressionContext* apExprCtx, const tChar* aaszFormat, const Var& a1);
  const achar* Format(iExpressionContext* apExprCtx, const tChar* aaszFormat, const Var& a1) {
    this->clear();
    return this->CatFormat(apExprCtx,aaszFormat,a1);
  }
  const achar* CatFormat(iExpressionContext* apExprCtx, const tChar* aaszFormat, const Var& a1, const Var& a2,
                         const Var& a3 = niVarNull, const Var& a4 = niVarNull);
  const achar* Format(iExpressionContext* apExprCtx, const tChar* aaszFormat, const Var& a1, const Var& a2,
                      const Var& a3 = niVarNull, const Var& a4 = niVarNull)
  {
    this->clear();
    return this->CatFormat(apExprCtx,aaszFormat,a1,a2,a3,a4);
  }
  const achar* CatFormat(iExpressionContext* apExprCtx, const tChar* aaszFormat,
                         const Var& a1, const Var& a2, const Var& a3,
                         const Var& a4, const Var& a5, const Var& a6 = niVarNull,
                         const Var& a7 = niVarNull, const Var& a8 = niVarNull);
  const achar* Format(iExpressionContext* apExprCtx, const tChar* aaszFormat,
                      const Var& a1, const Var& a2, const Var& a3,
                      const Var& a4, const Var& a5, const Var& a6 = niVarNull,
                      const Var& a7 = niVarNull, const Var& a8 = niVarNull)
  {
    this->clear();
    return this->CatFormat(apExprCtx,aaszFormat,a1,a2,a3,a4,a5,a6,a7,a8);
  }
  const achar* CatFormat(iExpressionContext* apExprCtx, const tChar* aaszFormat,
                         const Var& a1, const Var& a2, const Var& a3,
                         const Var& a4, const Var& a5, const Var& a6,
                         const Var& a7, const Var& a8, const Var& a9,
                         const Var& a10 = niVarNull, const Var& a11 = niVarNull, const Var& a12 = niVarNull,
                         const Var& a13 = niVarNull, const Var& a14 = niVarNull, const Var& a15 = niVarNull,
                         const Var& a16 = niVarNull, const Var& a17 = niVarNull, const Var& a18 = niVarNull,
                         const Var& a19 = niVarNull, const Var& a20 = niVarNull, const Var& a21 = niVarNull,
                         const Var& a22 = niVarNull, const Var& a23 = niVarNull, const Var& a24 = niVarNull);
  const achar* Format(iExpressionContext* apExprCtx, const tChar* aaszFormat,
                      const Var& a1, const Var& a2, const Var& a3,
                      const Var& a4, const Var& a5, const Var& a6,
                      const Var& a7, const Var& a8, const Var& a9,
                      const Var& a10 = niVarNull, const Var& a11 = niVarNull, const Var& a12 = niVarNull,
                      const Var& a13 = niVarNull, const Var& a14 = niVarNull, const Var& a15 = niVarNull,
                      const Var& a16 = niVarNull, const Var& a17 = niVarNull, const Var& a18 = niVarNull,
                      const Var& a19 = niVarNull, const Var& a20 = niVarNull, const Var& a21 = niVarNull,
                      const Var& a22 = niVarNull, const Var& a23 = niVarNull, const Var& a24 = niVarNull)
  {
    this->clear();
    return this->CatFormat(apExprCtx,aaszFormat,
                           a1,a2,a3,a4,a5,a6,a7,a8,
                           a9,a10,a11,a12,a13,a14,a15,a16,
                           a17,a18,a19,a20,a21,a22,a23,a24);
  }

  tU32 Hash() const {
    const ni::achar* s = c_str();
    ni::tU32 l = mnLen;
    ni::tU32 h = l; // seed
    ni::tU32 step = (l>>5)|1; // if the string is too long, don't hash all its chars
    for (; l >= step; l -= step)
      h = h ^ ((h<<5)+(h>>2)+*(s++));
    return h;
  }
};

///////////////////////////////////////////////
inline tBool ToString(const iToString* apToString, cString& dest) {
  if (!apToString) return eFalse;
  dest = apToString->ToString().Chars();
  return eTrue;
}

///////////////////////////////////////////////
inline tBool cString::IsEmpty() const
{
  return this->empty();
}

///////////////////////////////////////////////
inline tBool cString::IsNotEmpty() const
{
  return !IsEmpty();
}

///////////////////////////////////////////////
inline const achar* cString::Remove(tU32 lPos)
{
  const tU32 backCharWidth = StrCharWidth(back());
  const tU32 lm1 = size()-backCharWidth;
  if (lPos == eInvalidHandle)
    lPos = lm1;
  if (lPos == 0) {
    *this = this->substr(StrSequenceLength(Chars() + lPos));
  }
  else if (lPos == lm1) {
    *this = this->substr(0,lm1);
  }
  else if (lPos < lm1 && lPos > 0) {
    *this = this->substr(0,lPos)+
        this->substr(lPos+StrSequenceLength(Chars() + lPos));
  }
  return Chars();
}

///////////////////////////////////////////////
inline cString cString::GetWithoutBEQuote() const
{
  const achar begChar = Chars()[0];
  const achar endChar = Chars()[Len()-1];
  const tBool bBegQuote = (begChar == '"' || begChar == '\'') ? eTrue : eFalse;
  const tBool bEndQuote = (bBegQuote && begChar == endChar) ? eTrue : eFalse;
  if (!bBegQuote && !bEndQuote) return *this;
  const tU32 frontCW = StrCharWidth(front());
  const tU32 backCW = StrCharWidth(back());
  return substr(bBegQuote?frontCW:0,
                size()-((bEndQuote?backCW:0)+(bBegQuote?frontCW:0)));
}

///////////////////////////////////////////////
inline cString cString::StripQuotes() const
{
  const char* s = Chars();
  const tU32 len = Len();
  cString r;
  r.resize(len);
  tU32 j = 0;
  char* d = r.data();
  for (tU32 i = 0; i < len; ++i) {
    if (s[i] == '"') {
      if (i == 0 || s[i-1] != '\\') {
        // skip the " if its not \"
        continue;
      }
    }
    d[j++] = s[i];
  }
  r.resize(j);
  return r;
}

///////////////////////////////////////////////
inline const achar* cString::TrimRightEx(const achar* pszTargets)
{
  // if we're not trimming anything, we're not doing any work
  if ((pszTargets == NULL) || (*pszTargets == 0))
    return Chars();

  // find beginning of trailing matches by starting at beginning
  const achar* psz = Chars();
  const achar* pszLast = NULL;
  while (*psz != 0) {
    const tU32 c = StrGetNext(psz);
    if (StrChr(pszTargets,c) != NULL) {
      if (pszLast == NULL)
        pszLast = psz;
    }
    else {
      pszLast = NULL;
    }
    StrGetNextX(&psz);
  }

  if (pszLast != NULL) {
    // truncate at left-most matching character
    tSize iLast = pszLast-Chars();
    *this = this->substr(0,(tU32)iLast); // TODO: Fix unsafe type truncation (tU32)iLast
  }

  return Chars();
}

///////////////////////////////////////////////
inline const achar* cString::TrimRight()
{
  return TrimRightEx(ConstTrim());
}

///////////////////////////////////////////////
inline const achar* cString::TrimLeftEx(const achar* pszTargets)
{
  if ((pszTargets == NULL) || (*pszTargets == 0))
    return Chars();

  const achar* psz = Chars();
  while ((*psz != 0) && (StrChr(pszTargets,StrGetNext(psz)) != NULL)) {
    StrGetNextX(&psz);
  }

  if (psz != Chars()) {
    // fix up data and length
    tSize iFirst = psz-Chars();
    *this = this->substr((tI32)iFirst); // TODO: Fix unsafe type truncation (tI32)iFirst
  }

  return Chars();
}

///////////////////////////////////////////////
inline const achar* cString::TrimLeft()
{
  return TrimLeftEx("\r\n\t ");
}

///////////////////////////////////////////////
inline const achar* cString::TrimEx(const achar* aszTargets)
{
  TrimRightEx(aszTargets);
  TrimLeftEx(aszTargets);
  return Chars();
}

///////////////////////////////////////////////
inline const achar* cString::Trim()
{
  return TrimEx("\r\n\t ");
}

///////////////////////////////////////////////
inline const achar* cString::Normalize()
{
  cString strNew;

  // remove spaces
  Trim();

  tU32 prior = 0;
  const achar* p = Chars();
  while (*p) {
    const tU32 next = StrGetNextX(&p);
    // p-1 is safe as the begining spaces have been removed
    if (StrIsSpace(next) && StrIsSpace(prior)) {
      // do nothing, skip the spaces...
    }
    else {
      strNew.appendChar(next);
    }
    prior = next;
  }

  *this = strNew;
  return Chars();
}

///////////////////////////////////////////////
inline tF32 cString::Float() const {
  return (tF32)StrAToF(Chars());
}
inline tF64 cString::Double() const {
  return (tF64)StrAToF(Chars());
}
inline tI32 cString::Long() const {
  return (tI32)StrToL(Chars(),NULL,10);
}
inline tU32 cString::ULong() const {
  return (tU32)StrToUL(Chars(),NULL,10);
}
inline tI64 cString::LongLong() const {
  return StrToL(Chars(),NULL,10);
}
inline tU64 cString::ULongLong() const {
  return StrToUL(Chars(),NULL,10);
}

///////////////////////////////////////////////
inline tBool cString::Bool(tBool abDefault) const
{
  if (IsEmpty())
    return abDefault;
  if (IEq("true") || IEq("t") || IEq("yes") || IEq("on"))
    return eTrue;
  if (IEq("false") || IEq("f") || IEq("no") || IEq("off"))
    return eFalse;
  const tU32 longValue = Long();
  if (longValue == 1)
    return eTrue;
  if (longValue == 0)
    return eFalse;
  return abDefault;
}

///////////////////////////////////////////////
inline tI32 cString::NumberArray(tF64* pDest, tSize nNum, tBool bDotIsNumber, const achar* aaszSkipToFirst) const
{
  tSize dim = 0;
  cString current;
  const achar* p = Chars();

  if (aaszSkipToFirst) {
    tI32 pos = this->find(aaszSkipToFirst);
    if (pos == this->npos)
      return -1;
    p += pos;
  }

  while (*p && !StrIsNumberPart(*p,bDotIsNumber)) {
    ++p;
  }
  if (!*p)
    return -1;

  do {
    current.appendChar(*(p++));
    if (!(*p) || !StrIsNumberPart(*p,bDotIsNumber)) {
      pDest[dim++] = current.Double();
      current.appendChar('\n');
      if (dim == nNum)
        break;
      while (*p && !StrIsNumberPart(*p,bDotIsNumber))
        ++p;
      current.Clear(16);
    }
  } while(*p);

  return (dim == nNum) ? tI32(p-Chars()) : -1;
}

///////////////////////////////////////////////
inline tI32 cString::NumberArray(tF32* pDest, tSize nNum, tBool bDotIsNumber, const achar* aaszSkipToFirst) const
{
  tSize dim = 0;
  cString current;
  const achar* p = Chars();

  if (aaszSkipToFirst) {
    tI32 pos = this->find(aaszSkipToFirst);
    if (pos == this->npos)
      return -1;
    p += pos;
  }

  while (*p && !StrIsNumberPart(*p,bDotIsNumber)) { ++p; }
  if (!*p) return -1;

  do
  {
    current.appendChar(*(p++));
    if (!(*p) || !StrIsNumberPart(*p,bDotIsNumber)) {
      pDest[dim++] = current.Float();
      current.appendChar('\n');
      if (dim == nNum) break;
      while (*p && !StrIsNumberPart(*p,bDotIsNumber)) ++p;
      current.Clear();
    }
  } while(*p);

  return (dim == nNum) ? tI32(p-Chars()) : -1;
}


///////////////////////////////////////////////
inline tI32 cString::NumberArray(tI32* pDest, tSize nNum, tBool bDotIsNumber, const achar* aaszSkipToFirst) const
{
  tSize dim = 0;
  cString current;
  const achar* p = Chars();

  if (aaszSkipToFirst) {
    tI32 pos = this->find(aaszSkipToFirst);
    if (pos == this->npos)
      return -1;
    p += pos;
  }

  while (*p && !StrIsNumberPart(*p,bDotIsNumber)) { ++p; }
  if (!*p) return -1;

  do
  {
    current.appendChar(*(p++));
    if (!(*p) || !StrIsNumberPart(*p,bDotIsNumber))
    {
      pDest[dim++] = current.Long();
      current.appendChar('\n');
      if (dim == nNum) break;
      while (*p && !StrIsNumberPart(*p,bDotIsNumber)) ++p;
      current.Clear(16);
    }
  } while(*p);

  return (dim == nNum) ? tI32(p-Chars()) : -1;
}

///////////////////////////////////////////////
inline tU32 cString::FourCC() const
{
  if (Len() < 4)
    return 0;
  const achar* pChars = this->c_str();
  cchar a = cchar(pChars[0]&0xFF);
  cchar b = cchar(pChars[1]&0xFF);
  cchar c = cchar(pChars[2]&0xFF);
  cchar d = cchar(pChars[3]&0xFF);
  return niFourCC(a,b,c,d);
}

///////////////////////////////////////////////
inline tVersion cString::Version() const
{
  tI32 lNums[3] = {0,0,0};
  NumberArray(lNums,3,eFalse,NULL);
  return niMakeVersion(lNums[0],lNums[1],lNums[2]);
}

///////////////////////////////////////////////
inline tU32 cString::Color() const
{
  tI32 lNums[4] = {0,0,0,0};
  NumberArray(lNums,4,eFalse,NULL);
  return ULColorBuild(lNums[0],lNums[1],lNums[2],lNums[3]);
}

///////////////////////////////////////////////
inline const achar* cString::Set(tI32 lValue) {
  achar buf[ni::kMaxCharsIntToStr];
  return Set(StringIntToStr(buf,niCountOf(buf),lValue),buf+ni::kMaxCharsIntToStr-1);
}
inline const achar* cString::Set(tU32 lValue) {
  achar buf[ni::kMaxCharsIntToStr];
  return Set(StringUIntToStr(buf,niCountOf(buf),lValue),buf+ni::kMaxCharsIntToStr-1);
}
inline const achar* cString::Set(tI64 lValue) {
  achar buf[ni::kMaxCharsIntToStr];
  return Set(StringIntToStr(buf,niCountOf(buf),lValue),buf+ni::kMaxCharsIntToStr-1);
}
inline const achar* cString::Set(tU64 lValue) {
  achar buf[ni::kMaxCharsIntToStr];
  return Set(StringUIntToStr(buf,niCountOf(buf),lValue),buf+ni::kMaxCharsIntToStr-1);
}

///////////////////////////////////////////////
inline const achar* cString::Set(tBool bValue) {
  *this = bValue?ConstTrue():ConstFalse();
  return this->Chars();
}

///////////////////////////////////////////////
inline const achar* cString::Set(const tF64* pSrc, tSize nNum, const achar* uszSeparator) {
  cString buffer;
  Clear(nNum+nNum*4);
  for (tSize i = 0; i < nNum; ++i) {
    *this += buffer.Set(pSrc[i]);
    if (uszSeparator && i < nNum-1)
      *this += uszSeparator;
  }
  return Chars();
}
inline const achar* cString::Set(const tF32* pSrc, tSize nNum, const achar* uszSeparator) {
  cString buffer;
  Clear(nNum+nNum*4);
  for (tSize i = 0; i < nNum; ++i) {
    *this += buffer.Set(pSrc[i]);
    if (uszSeparator && i < nNum-1)
      *this += uszSeparator;
  }
  return Chars();
}
inline const achar* cString::Set(const tI32* pSrc, tSize nNum, const achar* uszSeparator) {
  cString buffer;
  Clear(nNum+nNum*4);
  for (tSize i = 0; i < nNum; ++i) {
    *this += buffer.Set(pSrc[i]);
    if (uszSeparator && i < nNum-1)
      *this += uszSeparator;
  }
  return Chars();
}

///////////////////////////////////////////////
inline const achar* cString::SetFourCC(tU32 lFourCC)
{
  Clear(4);
  cchar fourcc[5];
  fourcc[0] = (cchar)niFourCCA(lFourCC);
  fourcc[1] = (cchar)niFourCCB(lFourCC);
  fourcc[2] = (cchar)niFourCCC(lFourCC);
  fourcc[3] = (cchar)niFourCCD(lFourCC);
  fourcc[4] = 0;
  *this = fourcc;
  return Chars();
}

///////////////////////////////////////////////
inline const achar* cString::SetColor(tU32 ulColor)
{
  tI32 ulVals[4];
  ulVals[0] = ULColorGetR(ulColor);
  ulVals[1] = ULColorGetG(ulColor);
  ulVals[2] = ULColorGetB(ulColor);
  ulVals[3] = ULColorGetA(ulColor);
  return Set(ulVals, 4, cString(" ").Chars());
}

///////////////////////////////////////////////
inline const achar* cString::Set(const iToString* aVal)
{
  if (aVal)
    ni::ToString(aVal,*this);
  return Chars();
}

///////////////////////////////////////////////
inline const achar* cString::Set(const cchar* pBegin, const cchar* pEnd) {
  const tU32 nSize = (tU32)((pEnd-pBegin)*sizeof(achar));
  this->resize(nSize);
  ni::MemCopy((tPtr)data(),(tPtr)pBegin,nSize);
  return Chars();
}
inline const achar* cString::Set(const gchar* pBegin, const gchar* pEnd) {
  const tU32 nSize = (tU32)((pEnd-pBegin)*sizeof(gchar));
  *this = BufferUTF8(pBegin,nSize).Chars();
  return Chars();
}
inline const achar* cString::Set(const xchar* pBegin, const xchar* pEnd) {
  const tU32 nSize = (tU32)((pEnd-pBegin)*sizeof(xchar));
  *this = BufferUTF8(pBegin,nSize).Chars();
  return Chars();
}

///////////////////////////////////////////////
inline cString& cString::operator << (tI8 lVal) {
  this->append(cString().Set((tI32)lVal));
  return *this;
}
inline cString& cString::operator << (tU8 ulVal) {
  this->append(cString().Set((tU32)ulVal));
  return *this;
}
inline cString& cString::operator << (tI16 lVal) {
  this->append(cString().Set((tI32)lVal));
  return *this;
}
inline cString& cString::operator << (tU16 ulVal) {
  this->append(cString().Set((tU32)ulVal));
  return *this;
}
inline cString& cString::operator << (tI32 lVal) {
  this->append(cString().Set(lVal));
  return *this;
}
inline cString& cString::operator << (tU32 ulVal) {
  this->append(cString().Set(ulVal));
  return *this;
}
inline cString& cString::operator << (tI64 lVal) {
  this->append(cString().Set(lVal));
  return *this;
}
inline cString& cString::operator << (tU64 ulVal) {
  this->append(cString().Set(ulVal));
  return *this;
}
inline cString& cString::operator << (tF32 fVal) {
  this->append(cString().Set(fVal));
  return *this;
}
inline cString& cString::operator << (tF64 dVal) {
  this->append(cString().Set(dVal));
  return *this;
}
inline cString& cString::operator << (const tUUID& uuidVal) {
  this->append(cString().Set(uuidVal));
  return *this;
}
inline cString& cString::operator << (const cchar* szChar) {
  this->append(szChar);
  return *this;
}
inline cString& cString::operator << (const gchar* gszChar) {
  this->append(gszChar);
  return *this;
}
inline cString& cString::operator << (const xchar* xszChar) {
  this->append(xszChar);
  return *this;
}

///////////////////////////////////////////////
inline cString& cString::operator << (const iToString* obj)
{
  if (obj) {
    cString str;
    ni::ToString(obj,str);
    this->append(str);
  }
  return *this;
}

///////////////////////////////////////////////
inline tUUID cString::_ToUUID(const achar* szUUID, const tSize baseLen)
{
  tInt  i;
  const achar* cp;
  achar buf[3];

  tSize len = baseLen;

  // skip any non-number character
  while (len && *szUUID && !ni::StrIsXDigit((cchar)*szUUID)) {
    ++szUUID;
    --len;
  }

  // length check
  if (len < 36)
    return kuuidZero;

  // syntax check
  for (i=0, cp = szUUID; i <= 36; ++i,++cp) {
    if ((i == 8) || (i == 13) || (i == 18) || (i == 23)) {
      if (!ni::StrIsXDigit((cchar)*cp))
        continue;
      else
        return kuuidZero;
    }
    if (i == 36) {
      if (*cp == 0 || !ni::StrIsXDigit((cchar)*cp))
        continue;
      else
        return kuuidZero;
    }
    else {
      if (!ni::StrIsXDigit((cchar)*cp))
        return kuuidZero;
    }
  }

  // parsing
  tUUIDTime uuid;

  uuid.nTimeLow = (ni::tU32)StrToL(szUUID, NULL, 16);
  uuid.nTimeMid = (ni::tU16)StrToL(szUUID+9, NULL, 16);
  uuid.nTimeHiAndVersion = (ni::tU16)StrToL(szUUID+14, NULL, 16);

  buf[2] = 0;
  cp = szUUID+19;

  buf[0] = *cp++;
  buf[1] = *cp++;
  uuid.nClockSeqReserved = (ni::tU8)StrToL(buf, NULL, 16);

  buf[0] = *cp++;
  buf[1] = *cp++;
  uuid.nClockSeqLow = (ni::tU8)StrToL(buf, NULL, 16);

  ++cp; // skip '-'

  for (i = 0; i < 6; ++i) {
    buf[0] = *cp++;
    buf[1] = *cp++;
    uuid.nNodes[i] = (ni::tU8)StrToL(buf, NULL, 16);
  }

  return (tUUID&)uuid;
}
inline tUUID cString::UUID() const {
  return _ToUUID(Chars(),Length());
}

#if niMinFeatures(20)
///////////////////////////////////////////////
inline const achar* cString::SetDouble(
    tF64 dValue, eDoubleToStringMode aMode,
    tI32 aDigitsOrPrecision,
    const char* aInfinitySymbol,
    const char* aNaNSymbol,
    char aExponentCharacter)
{
  char buff[kDoubleToStringBufferSize];
  *this = DoubleToString(buff,niCountOf(buff),
                         dValue,
                         aMode,
                         aDigitsOrPrecision,
                         aInfinitySymbol,
                         aNaNSymbol,
                         aExponentCharacter);
  return Chars();
}
#endif

#undef STR_INIT

/**@}*/
/**@}*/
} // End of ni
/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __STRINGBASE_27803022_H__
