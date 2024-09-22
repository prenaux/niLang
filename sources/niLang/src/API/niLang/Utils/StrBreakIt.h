#ifndef __STRBREAKIT_H_5888C8D6_D201_4D76_94FE_C4F82F27E0FC__
#define __STRBREAKIT_H_5888C8D6_D201_4D76_94FE_C4F82F27E0FC__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../StringLibIt.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_String
 * @{
 */

template <typename BREAKER>
class StrBreakIt : public eastl::iterator<eastl::forward_iterator_tag,ni::StrCharIt> {
  typedef eastl::iterator<eastl::forward_iterator_tag,ni::StrCharIt> iterator;
  mutable BREAKER _breaker;
  const StrCharIt _whole;
  StrCharIt _cur;

  StrCharIt toBeforeBreak(const StrCharIt& aCurrent) const {
    StrCharIt it(aCurrent);
    while (!it.is_end()) {
      const achar* cur = it.current();
      tU32 breakLen = _breaker.ShouldBreak(const_cast<const StrCharIt&>(it));
      if (breakLen > 0) {
        return StrCharIt(aCurrent.start(),cur);
      }
      it.next();
    }
    _breaker.lastSeparator = StrCharIt(_whole.end(),_whole.end());
    return StrCharIt(aCurrent.current(),_whole.end());
  }

  void _Init() {
    // make sur _whole.end() is not null
    _whole.sizeInBytes();
    niAssert(_whole.end() != NULL);
    to_start();
  }

  // No default constructor
  StrBreakIt();

 public:
  explicit StrBreakIt(const StrCharIt& aCharIt, const BREAKER& aBreaker = BREAKER())
      : _whole(aCharIt), _cur(aCharIt.start(),aCharIt.start()), _breaker(aBreaker)
  {
    _Init();
  }

  const BREAKER& breaker() const {
    return _breaker;
  }
  const StrCharIt& whole() const {
    return _whole;
  }
  const StrCharIt& current() const {
    return _cur;
  }
  ni::tBool is_start() const {
    return _cur.start() == _whole.start();
  }
  ni::tBool is_end() const {
    return _whole.end() == _cur.start();
  }
  void to_start() {
    _cur = toBeforeBreak(_whole);
  }
  ni::tSize to_end() {
    ni::tSize n = 0;
    while (!is_end()) {
      next();
      ++n;
    }
    return n;
  }

  StrCharIt peek_next() const {
    tU32 breakLen = _breaker.ShouldBreak(StrCharIt(_cur.end(),_whole.end()));
    const achar* newStart = _cur.end() + breakLen;
    return toBeforeBreak(StrCharIt(newStart,_whole.end()));
  }
  const StrCharIt& next() {
    _cur = peek_next();
    return _cur;
  }

  const StrCharIt& advance(tU32 n) {
    niLoop(i,n) {
      next();
      if (is_end())
        break;
    }
    return current();
  }

  bool operator == (const StrBreakIt& rhs) const {
    return (_cur == rhs._cur);
  }
  bool operator != (const StrBreakIt& rhs) const {
    return (_cur != rhs._cur);
  }

  iterator& operator ++ () {
    this->next();
    return *this;
  }
  iterator operator ++ (int) {
    iterator temp = *this;
    this->next();
    return temp;
  }

  StrCharIt peek_next(tU32 fwd) const {
    if (fwd == 0)
      return current();
    StrBreakIt temp = *this;
    while (--fwd && !temp.is_end()) {
      ++temp;
    }
    return temp.peek_next();
  }
};

struct StrBreakLine {
  StrCharIt lastSeparator;
  StrBreakLine() : lastSeparator(AZEROSTR) {
  }
  tU32 ShouldBreak(const StrCharIt& it) {
    const tU32 c = it.peek_next();
    // \n (Unix/Linux)
    if (c == '\n') {
      lastSeparator = StrCharIt(it.current(),it.current()+1);
      return 1;
    }
    if (c == '\r') {
      // \r\n (Windows)
      if (it.peek_next(2) == '\n') {
        lastSeparator = StrCharIt(it.current(),it.current()+2);
        return 2;
      }
      // \r (MacOS 9 and below)
      lastSeparator = StrCharIt(it.current(),it.current()+1);
      return 1;
    }

    return 0;
  }
};
typedef StrBreakIt<StrBreakLine> StrBreakLineIt;

struct StrBreakWord {
  StrCharIt lastSeparator;
  StrBreakWord() : lastSeparator(AZEROSTR) {
  }

  tBool IsLetter(const StrCharIt& it, tU32& c) const {
    c = it.peek_next();
    if (c == '\'' || c == '-') {
      // Dash '-' and Apostroph ''' are part of a word if preceded by another letter.
      return StrIsLetterDigit(it.peek_prior());
    }
    return StrIsLetterDigit(c);
  }
  tU32 ShouldBreak(const StrCharIt& it) {
    StrCharIt cursor(it);
    tU32 c = 0;
    while (!cursor.is_end() && !IsLetter(cursor,c)) {
      cursor.next();
    }
    if (cursor.current() != it.current()) {
      lastSeparator = StrCharIt(it.current(),cursor.current());
    }
    return niUnsafeNarrowCast(tU32,cursor.current()-it.current());
  }
};
typedef StrBreakIt<StrBreakWord> StrBreakWordIt;

struct StrBreakSpace {
  StrCharIt lastSeparator;
  StrBreakSpace() : lastSeparator(AZEROSTR) {
  }

  tBool IsSpace(const StrCharIt& it, tU32& c) const {
    c = it.peek_next();
    return c == '\r' || c == '\n' || StrIsSpace(c);
  }
  tU32 ShouldBreak(const StrCharIt& it) {
    // any other space character...
    StrCharIt cursor(it);
    tU32 c = 0;
    while (!cursor.is_end() && IsSpace(cursor,c)) {
      cursor.next();
    }
    if (cursor.current() != it.current()) {
      lastSeparator = StrCharIt(it.current(),cursor.current());
    }
    return niUnsafeNarrowCast(tU32,cursor.current()-it.current());
  }
};
typedef StrBreakIt<StrBreakSpace> StrBreakSpaceIt;

struct StrBreakChr {
  astl::vector<tU32> splitters;
  StrCharIt lastSeparator;

  StrBreakChr() : lastSeparator(AZEROSTR) {}
  StrBreakChr(const StrBreakChr& aBreaker) : lastSeparator(AZEROSTR) {
    splitters = aBreaker.splitters;
  }
  StrBreakChr(const astl::vector<tU32>& aSplitters) : lastSeparator(AZEROSTR) {
    splitters = aSplitters;
  }
  StrBreakChr(const achar* aaszSplitters) : lastSeparator(AZEROSTR) {
    StrCharIt it(aaszSplitters);
    while (!it.is_end()) {
      splitters.push_back(it.next());
    }
  }

  tBool IsSplit(const StrCharIt& it, tU32& c) const {
    c = it.peek_next();
    niLoop(i,splitters.size()) {
      if (c == splitters[i])
        return eTrue;
    }
    return eFalse;
  }
  tU32 ShouldBreak(const StrCharIt& it) {
    StrCharIt cursor(it);
    tU32 c = 0;
    while (!cursor.is_end() && IsSplit(cursor,c)) {
      cursor.next();
    }
    if (cursor.current() != it.current()) {
      lastSeparator = StrCharIt(it.current(),cursor.current());
    }
    return niUnsafeNarrowCast(tU32,cursor.current()-it.current());
  }
};
typedef StrBreakIt<StrBreakChr> StrBreakChrIt;

struct StrBreakIdentifier {
  StrCharIt lastSeparator;
  StrBreakIdentifier() : lastSeparator(AZEROSTR) {
  }

  tBool IsIdLetter(const StrCharIt& it, tU32& c) const {
    c = it.peek_next();
    return c == '_' || StrIsLetterDigit(c);
  }
  tU32 ShouldBreak(const StrCharIt& it) {
    StrCharIt cursor(it);
    tU32 c = 0;
    while (!cursor.is_end() && !IsIdLetter(cursor,c)) {
      cursor.next();
    }
    if (cursor.current() != it.current()) {
      lastSeparator = StrCharIt(it.current(),cursor.current());
    }
    return niUnsafeNarrowCast(tU32,cursor.current()-it.current());
  }
};
typedef StrBreakIt<StrBreakIdentifier> StrBreakIdentifierIt;

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __STRBREAKIT_H_5888C8D6_D201_4D76_94FE_C4F82F27E0FC__
