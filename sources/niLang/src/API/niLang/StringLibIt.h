#pragma once
#ifndef __STRINGLIBIT_H_4DAC3019_5160_4377_A2FF_7CB5CAB10D2A__
#define __STRINGLIBIT_H_4DAC3019_5160_4377_A2FF_7CB5CAB10D2A__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "StringLib.h"
#include "STL/iterator.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_String
 * @{
 */

class StrCharIt : public eastl::iterator<eastl::bidirectional_iterator_tag,ni::tU32> {
  typedef eastl::iterator<eastl::bidirectional_iterator_tag,ni::tU32> iterator;
  const achar* _start;
  const achar* _end;
  const achar* _cur;
  tSize        _len;

 public:
  StrCharIt(const achar* aBegin, const achar* aEnd = NULL, tSize aLen = 0)
      : _start(aBegin)
      , _end(aEnd)
      , _cur(aBegin)
      , _len(aLen)
  {
    niAssert(_start != NULL);
  }
  StrCharIt(const achar* aBegin, tU32 anSizeInBytes, tSize aLen = 0)
      : _start(aBegin)
      , _end(anSizeInBytes?aBegin+anSizeInBytes:NULL)
      , _cur(aBegin)
      , _len(aLen)
  {
    niAssert(_start != NULL);
  }

  const achar* start() const {
    return _start;
  }
  const achar* end() const {
    return _end;
  }
  const achar* current() const {
    return _cur;
  }
  void to_pos(tU32 anPos) {
    _cur = _start + anPos;
  }
  ni::tU32 pos() const {
    return (tU32)(_cur-_start);
  }
  ni::tBool is_start() const {
    return _cur == _start;
  }
  ni::tBool is_end() const {
    return
        (_end && _cur >= _end) ||
        (peek_next() == 0);
  }
  void to_start() {
    _cur = _start;
  }
  ni::tSize to_end() {
    ni::tSize n = 0;
    while (!is_end()) {
      next();
      ++n;
    }
    return n;
  }

  ni::tSize compute_lengthAndSizeInBytes() {
    const achar* prev = _cur;
    _cur = _start;
    _len = to_end();
    if (!_end)
      _end = _start+pos();
    _cur = prev;
    return _len;
  }
  ni::tSize length(tBool abForceCompute = eFalse) const {
    if (!_len || abForceCompute) {
      const_cast<StrCharIt*>(this)->compute_lengthAndSizeInBytes();
    }
    return _len;
  }
  ni::tSize sizeInBytes(tBool abForceCompute = eFalse) const {
    if (!_end || abForceCompute) {
      const_cast<StrCharIt*>(this)->compute_lengthAndSizeInBytes();
    }
    return (ni::tSize)(_end-_start);
  }

  ni::tU32 peek_next() const {
    return StrGetNext(_cur);
  }
  ni::tU32 next() {
    return StrGetNextX(&_cur);
  }
  ni::tU32 peek_prior() const {
    return StrGetPrior(_cur);
  }
  ni::tU32 prior() {
    return StrGetPriorX(&_cur);
  }

  ni::tU32 advance(tU32 n) {
    ni::tU32 c = 0;
    niLoop(i,n) {
      c = next();
    }
    return c;
  }
  ni::tU32 rewind(tU32 n) {
    ni::tU32 c = 0;
    niLoop(i,n) {
      c = prior();
    }
    return c;
  }

  bool operator == (const StrCharIt& rhs) const {
    return (_cur == rhs._cur);
  }
  bool operator != (const StrCharIt& rhs) const {
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
  iterator& operator -- () {
    this->prior();
    return *this;
  }
  iterator operator -- (int) {
    StrCharIt temp = *this;
    this->prior();
    return temp;
  }

  ni::tU32 peek_next(tU32 fwd) const {
    if (fwd == 0) return 0;
    StrCharIt temp = *this;
    while (--fwd && !temp.is_end()) {
      ++temp;
    }
    return temp.is_end() ? 0 : temp.peek_next();
  }
  ni::tU32 peek_prev(tU32 back) const {
    if (back == 0) return 0;
    StrCharIt temp = *this;
    while (--back && !temp.is_start()) {
      --temp;
    }
    return temp.is_start() ? 0 : temp.peek_next();
  }
};

niExportFunc(ni::tI32) StrCharItFindFirstOf(ni::StrCharIt& aIt, const ni::tU32* aToFind, const ni::tU32 aNumToFind, ni::tU32* aCharFound);

niExportFunc(ni::tI32) StrCharItRFindFirstOf(ni::StrCharIt& aIt, const ni::tU32* aToFind, const ni::tU32 aNumToFind, ni::tU32* aCharFound);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __STRINGLIBIT_H_4DAC3019_5160_4377_A2FF_7CB5CAB10D2A__
