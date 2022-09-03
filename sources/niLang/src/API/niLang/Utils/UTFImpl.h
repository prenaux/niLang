#ifndef __UTF8_H_901EA323_E1DF_4E6D_B93C_93AF408EC232__
#define __UTF8_H_901EA323_E1DF_4E6D_B93C_93AF408EC232__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../STL/iterator.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

// Define this if you want stricter UTF parsing.
#ifndef niUTFAssertUnreachable
#define niUTFAssertUnreachable(msg) //niAssertUnreachable(msg)
#endif

enum eUTFError {
  eUTFError_OK,
  eUTFError_NotEnoughtRoom,
  eUTFError_InvalidLead,
  eUTFError_InvalidSequence,
  eUTFError_OverlongSequence,
  eUTFError_InvalidCodePoint
};

// Unicode constants
// Leading (high) surrogates: 0xd800 - 0xdbff
// Trailing (low) surrogates: 0xdc00 - 0xdfff
const ni::tU16 knUTFLeadSurrogateMin  = 0xd800u;
const ni::tU16 knUTFLeadSurrogateMax  = 0xdbffu;
const ni::tU16 knUTFTrailSurrogateMin = 0xdc00u;
const ni::tU16 knUTFTrailSurrogateMax = 0xdfffu;
const ni::tU16 knUTFLeadOffset        = knUTFLeadSurrogateMin - (0x10000 >> 10);
const ni::tU32 knUTFSurrogateOffset   = 0x10000u - (knUTFLeadSurrogateMin << 10) - knUTFTrailSurrogateMin;
const ni::tU32 knUTFReplacementMarker = 0xfffd;
const ni::tU32 knUTFMaxCharSize       = 4;
// Maximum valid value for a Unicode code point
const ni::tU32 knUTFCodePointMax      = 0x0010ffffu;

template<typename _T8>
inline ni::tU8 utf_mask8(_T8 oc) {
  return static_cast<ni::tU8>(0xff & oc);
}
template<typename _T16>
inline ni::tU16 utf_mask16(_T16 oc) {
  return static_cast<ni::tU16>(0xffff & oc);
}

template<typename _T8>
inline bool utf_is_trail(_T8 oc) {
  return ((utf_mask8(oc) >> 6) == 0x2);
}
inline bool utf_is_unexpected_continuation(const cchar* it) {
  return ((*it & 0xC0) == 0x80);
}
template <typename _T16>
inline bool utf_is_lead_surrogate(_T16 cp) {
  return (cp >= knUTFLeadSurrogateMin && cp <= knUTFLeadSurrogateMax);
}
template <typename _T16>
inline bool utf_is_trail_surrogate(_T16 cp) {
  return (cp >= knUTFTrailSurrogateMin && cp <= knUTFTrailSurrogateMax);
}

template <typename _T16>
inline bool utf_is_surrogate(_T16 cp) {
  return (cp >= knUTFLeadSurrogateMin && cp <= knUTFTrailSurrogateMax);
}
template <typename _T32>
inline bool utf_is_code_point_valid(_T32 cp) {
  return (cp <= knUTFCodePointMax && !utf_is_surrogate(cp) && cp != 0xfffe && cp != 0xffff);
}

////////////////////////////////////////////////////////////////////////////////
// UTF8
struct utf8 {
  typedef cchar tChar;

  static inline tSize sequence_length(const cchar* lead_it) {
    ni::tU8 lead = utf_mask8(*lead_it);
    if (lead < 0x80)
      return 1;
    else if ((lead >> 5) == 0x6)
      return 2;
    else if ((lead >> 4) == 0xe)
      return 3;
    else if ((lead >> 3) == 0x1e)
      return 4;
    else
      return 0;
  }
  static inline tSize char_width(ni::tU32 cp) {
    niAssert(utf_is_code_point_valid(cp));
    if (cp < 0x80)                        // one byte
      return 1;
    else if (cp < 0x800) {                // two bytes
      return 2;
    }
    else if (cp < 0x10000) {              // three bytes
      return 3;
    }
    else {      // four bytes
      return 4;
    }
  }
  static inline bool is_overlong_sequence(ni::tU32 cp, ni::tSize length) {
    if (cp < 0x80) {
      if (length != 1)
        return true;
    }
    else if (cp < 0x800) {
      if (length != 2)
        return true;
    }
    else if (cp < 0x10000) {
      if (length != 3)
        return true;
    }

    return false;
  }
  /// get_sequence_x functions decode utf-8 sequences of the length x
  static inline eUTFError get_sequence_1(const cchar*& it, const cchar* end, ni::tU32* code_point)
  {
    if (it != end) {
      if (code_point)
        *code_point = utf_mask8(*it);
      return eUTFError_OK;
    }
    return eUTFError_NotEnoughtRoom;
  }
  static inline eUTFError get_sequence_2(const cchar*& it, const cchar* end, ni::tU32* code_point)
  {
    eUTFError ret_code = eUTFError_NotEnoughtRoom;
    if (it != end) {
      ni::tU32 cp = utf_mask8(*it);
      if (++it != end) {
        if (utf_is_trail(*it)) {
          cp = ((cp << 6) & 0x7ff) + ((*it) & 0x3f);

          if (code_point)
            *code_point = cp;
          ret_code = eUTFError_OK;
        }
        else
          ret_code = eUTFError_InvalidSequence;
      }
      else
        ret_code = eUTFError_NotEnoughtRoom;
    }
    return ret_code;
  }
  static inline eUTFError get_sequence_3(const cchar*& it, const cchar* end, ni::tU32* code_point)
  {
    eUTFError ret_code = eUTFError_NotEnoughtRoom;

    if (it != end) {
      ni::tU32 cp = utf_mask8(*it);
      if (++it != end) {
        if (utf_is_trail(*it)) {
          cp = ((cp << 12) & 0xffff) + ((utf_mask8(*it) << 6) & 0xfff);
          if (++it != end) {
            if (utf_is_trail(*it)) {
              cp += (*it) & 0x3f;

              if (code_point)
                *code_point = cp;
              ret_code = eUTFError_OK;
            }
            else
              ret_code = eUTFError_InvalidSequence;
          }
          else
            ret_code = eUTFError_NotEnoughtRoom;
        }
        else
          ret_code = eUTFError_InvalidSequence;
      }
      else
        ret_code = eUTFError_NotEnoughtRoom;
    }

    return ret_code;
  }
  static inline eUTFError get_sequence_4(const cchar*& it, const cchar* end, ni::tU32* code_point)
  {
    eUTFError ret_code = eUTFError_NotEnoughtRoom;

    if (it != end) {
      ni::tU32 cp = utf_mask8(*it);
      if (++it != end) {
        if (utf_is_trail(*it)) {
          cp = ((cp << 18) & 0x1fffff) + ((utf_mask8(*it) << 12) & 0x3ffff);
          if (++it != end) {
            if (utf_is_trail(*it)) {
              cp += (utf_mask8(*it) << 6) & 0xfff;
              if (++it != end) {
                if (utf_is_trail(*it)) {
                  cp += (*it) & 0x3f;

                  if (code_point)
                    *code_point = cp;
                  ret_code = eUTFError_OK;
                }
                else
                  ret_code = eUTFError_InvalidSequence;
              }
              else
                ret_code = eUTFError_NotEnoughtRoom;
            }
            else
              ret_code = eUTFError_InvalidSequence;
          }
          else
            ret_code = eUTFError_NotEnoughtRoom;
        }
        else
          ret_code = eUTFError_InvalidSequence;
      }
      else
        ret_code = eUTFError_NotEnoughtRoom;
    }

    return ret_code;
  }
  static inline eUTFError validate_next(const cchar*& it, const cchar* end, ni::tU32* code_point)
  {
    // Save the original value of it so we can go back in case of failure
    // Of course, it does not make much sense with i.e. stream iterators
    const cchar* original_it = it;

    ni::tU32 cp = 0;
    // Determine the sequence length based on the lead octet
    tSize length = sequence_length(it);
    if (length == 0) {
      if (utf_is_unexpected_continuation(it))
        ++it;
      length = sequence_length(it);
      if (length == 0)
        return eUTFError_InvalidLead;
    }

    // Now that we have a valid sequence length, get trail octets and calculate the code point
    eUTFError err = eUTFError_OK;
    switch (length) {
      case 1: err = get_sequence_1(it, end, &cp); break;
      case 2: err = get_sequence_2(it, end, &cp); break;
      case 3: err = get_sequence_3(it, end, &cp); break;
      case 4: err = get_sequence_4(it, end, &cp); break;
    }
    if (err == eUTFError_OK) {
      if (utf_is_code_point_valid(cp)) {
        if (!is_overlong_sequence(cp, length)){
          // Passed! Return here.
          if (code_point)
            *code_point = cp;
          ++it;
          return eUTFError_OK;
        }
        else
          err = eUTFError_OverlongSequence;
      }
      else
        err = eUTFError_InvalidCodePoint;
    }

    // Failure branch - restore the original value of the iterator
    it = original_it;
    return err;
  }
  static inline eUTFError validate_next(const cchar*& it, const cchar* end) {
    return validate_next(it, end, 0);
  }
  static inline const cchar* find_invalid(const cchar* start, const cchar* end) {
    const cchar* result = start;
    while (result != end) {
      eUTFError err_code = validate_next(result, end);
      if (err_code != eUTFError_OK)
        return result;
    }
    return result;
  }
  static inline bool is_valid(const cchar* start, const cchar* end) {
    return (find_invalid(start, end) == end);
  }
  static inline cchar* append(ni::tU32 cp, cchar* result)
  {
    niAssert(utf_is_code_point_valid(cp));
    if (cp < 0x80)                        // one octet
      *(result++) = static_cast<cchar>(cp);
    else if (cp < 0x800) {                // two octets
      *(result++) = static_cast<cchar>((cp >> 6)          | 0xc0);
      *(result++) = static_cast<cchar>((cp & 0x3f)        | 0x80);
    }
    else if (cp < 0x10000) {              // three octets
      *(result++) = static_cast<cchar>((cp >> 12)         | 0xe0);
      *(result++) = static_cast<cchar>(((cp >> 6) & 0x3f) | 0x80);
      *(result++) = static_cast<cchar>((cp & 0x3f)        | 0x80);
    }
    else {                                // four octets
      *(result++) = static_cast<cchar>((cp >> 18)         | 0xf0);
      *(result++) = static_cast<cchar>(((cp >> 12) & 0x3f)| 0x80);
      *(result++) = static_cast<cchar>(((cp >> 6) & 0x3f) | 0x80);
      *(result++) = static_cast<cchar>((cp & 0x3f)        | 0x80);
    }
    return result;
  }
  static inline ni::tU32 raw_next(const cchar*& it) {
    ni::tU32 cp = utf_mask8(*it);
    tSize length = utf8::sequence_length(it);
    if (length == 0) {
      if (utf_is_unexpected_continuation(it))
        ++it;
      length = sequence_length(it);
    }
    switch (length) {
      case 1:
        break;
      case 2:
        ++it;
        cp = ((cp << 6) & 0x7ff) + ((*it) & 0x3f);
        break;
      case 3:
        ++it;
        cp = ((cp << 12) & 0xffff) + ((utf_mask8(*it) << 6) & 0xfff);
        ++it;
        cp += (*it) & 0x3f;
        break;
      case 4:
        ++it;
        cp = ((cp << 18) & 0x1fffff) + ((utf_mask8(*it) << 12) & 0x3ffff);
        ++it;
        cp += (utf_mask8(*it) << 6) & 0xfff;
        ++it;
        cp += (*it) & 0x3f;
        break;
      default:
        niUTFAssertUnreachable("Invalid utf sequence_length.");
        break;
    }
    ++it;
    return cp;
  }
  static inline ni::tU32 raw_peek(const cchar* it) {
    // One indirection so that the parameter isnt a reference and thus isnt modified.
    return raw_next(it);
  }
  static inline ni::tU32 next(const cchar*& it, const cchar* end) {
    ni::tU32 cp;
    eUTFError err_code = utf8::validate_next(it, end, &cp);
    switch (err_code) {
      case eUTFError_OK :
        return cp;
      case eUTFError_NotEnoughtRoom :
        niUTFAssertUnreachable("not_enough_room");
        break;
      case eUTFError_InvalidLead :
        niUTFAssertUnreachable("invalid_lead(*it)");
        break;
      case eUTFError_InvalidSequence :
        niUTFAssertUnreachable("invalid_sequence(*it)");
        break;
      case eUTFError_OverlongSequence :
        niUTFAssertUnreachable("invalid_overlong_sequence(*it)");
        break;
      case eUTFError_InvalidCodePoint :
        niUTFAssertUnreachable("invalid_code_point(cp)");
        break;
    }
    return 0;
  }
  static inline ni::tU32 next(const cchar*& it) {
    return next(it,it+4);
  }
  static inline ni::tU32 peek_next(const cchar* it) {
    return next(it);
  }
  static inline ni::tU32 peek_next(const cchar* it, const cchar* end) {
    return next(it, end);
  }
  static inline ni::tU32 prior(const cchar*& it) {
    while (utf_is_trail(*(--it))) ;
    const cchar* temp = it;
    return next(temp);
  }
  static inline ni::tU32 prior(const cchar*& it, const cchar* start) {
    const cchar* end = it;
    while (utf_is_trail(*(--it))) {
      if (it < start) {
        niAssert(0 && "invalid_utf8(*it)"); // error - no lead byte in the sequence
        return '?';
      }
    }
    const cchar* temp = it;
    return next(temp, end);
  }
  static inline ni::tU32 peek_prior(const cchar* it) {
    return prior(it);
  }
  static inline ni::tU32 peek_prior(const cchar* it, const cchar* start) {
    return prior(it,start);
  }

  static inline void advance(const cchar*& it, tUInt n) {
    for (tUInt i = 0; i < n; ++i) {
      next(it);
    }
  }
  static inline void advance (const cchar*& it, tUInt n, const cchar* end) {
    for (tUInt i = 0; i < n; ++i) {
      next(it, end);
    }
  }
  static inline ni::tSize distance(const cchar* first, const cchar* last) {
    tSize dist = 0;
    for ( ; first < last; ++dist)
      next(first, last);
    return dist;
  }
  static inline cchar* replace_invalid(const cchar* start, const cchar* end, cchar* out, ni::tU32 replacement) {
    while (start != end) {
      const cchar* sequence_start = start;
      eUTFError err_code = utf8::validate_next(start, end);
      switch (err_code) {
        case eUTFError_OK :
          for (const cchar* it = sequence_start; it != start; ++it)
            *out++ = *it;
          break;
        case eUTFError_NotEnoughtRoom:
          niAssert(0 && "not_enough_room");
          break;
        case eUTFError_InvalidLead:
          append (replacement, out);
          ++start;
          break;
        case eUTFError_InvalidSequence:
        case eUTFError_OverlongSequence:
        case eUTFError_InvalidCodePoint:
          append (replacement, out);
          ++start;
          // just one replacement mark for the sequence
          while (utf_is_trail(*start) && start != end)
            ++start;
          break;
      }
    }
    return out;
  }
  static inline cchar* replace_invalid(const cchar* start, const cchar* end, cchar* out) {
    static const ni::tU32 replacement_marker = utf_mask16(0xfffd);
    return replace_invalid(start, end, out, replacement_marker);
  }
};

////////////////////////////////////////////////////////////////////////////////
// UTF16
struct utf16 {
  typedef gchar tChar;

  static inline ni::tU32 sequence_length(const gchar* it) {
    ni::tU32 cp = utf_mask16(*it++);
    if (utf_is_lead_surrogate(cp)) {
      return 4;
    }
    return 2;
  }
  static inline int char_width(ni::tU32 cp) {
    niAssert(utf_is_code_point_valid(cp));
    if (cp > 0xffff) {
      return 4;
    }
    return 2;
  }
  static inline ni::tU32 next(const gchar*& it) {
    ni::tU32 cp = utf_mask16(*it++);
    if (utf_is_lead_surrogate(cp)) {
      ni::tU32 trail_surrogate = utf_mask16(*it++);
      cp = (cp << 10) + trail_surrogate + knUTFSurrogateOffset;
    }
    return cp;
  }
  static inline ni::tU32 peek_next(const gchar* it) {
    return next(it);
  }
  static inline ni::tU32 prior(const gchar*& it) {
    if (utf_is_trail_surrogate(*it--)) {
      --it;
    }
    const gchar* temp = it;
    return next(temp);
  }
  static inline ni::tU32 peek_prior(const gchar* it) {
    return prior(it);
  }
  static inline void advance(const gchar*& it, tSize n) {
    niLoop(i,n) {
      next(it);
    }
  }
  static inline ni::tSize distance(const gchar* first, const gchar* last) {
    tSize dist = 0;
    for ( ; first < last; ++dist)
      next(first);
    return dist;
  }
  static inline gchar* append(tU32 cp, gchar* result) {
    niAssert(utf_is_code_point_valid(cp));
    if (cp > 0xffff) { // make a surrogate pair
      *result++ = static_cast<gchar>((cp >> 10)   + knUTFLeadOffset);
      *result++ = static_cast<gchar>((cp & 0x3ff) + knUTFTrailSurrogateMin);
    }
    else {
      *result++ = static_cast<gchar>(cp);
    }
    return result;
  }
};

////////////////////////////////////////////////////////////////////////////////
// UTF32
struct utf32 {
  typedef xchar tChar;
  static inline ni::tU32 sequence_length(const xchar* it) {
    return 4;
  }
  static inline int char_width(ni::tU32) {
    return 4;
  }
  static inline ni::tU32 next(const xchar*& it) {
    return *it++;
  }
  static inline ni::tU32 peek_next(const xchar* it) {
    return next(it);
  }
  static inline ni::tU32 prior(const xchar*& it) {
    --it;
    const xchar* temp = it;
    return next(temp);
  }
  static inline ni::tU32 peek_prior(const xchar* it) {
    return prior(it);
  }
  static inline void advance(const xchar*& it, tSize n) {
    it += n;
  }
  static inline ni::tSize distance(const xchar* first, const xchar* last) {
    return last - first;
  }
  static inline xchar* append(tU32 cp, xchar* result) {
    *result++ = cp;
    return result;
  }
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/

// Based of code from http://utfcpp.sourceforge.net/
// Copyright 2006 Nemanja Trifunovic
/*
  Permission is hereby granted, free of charge, to any person or organization
  obtaining a copy of the software and accompanying documentation covered by
  this license (the "Software") to use, reproduce, display, distribute,
  execute, and transmit the Software, and to prepare derivative works of the
  Software, and to permit third-parties to whom the Software is furnished to
  do so, all subject to the following:

  The copyright notices in the Software and this entire statement, including
  the above license grant, this restriction and the following disclaimer,
  must be included in all copies of the Software, in whole or in part, and
  all derivative works of the Software, unless such copies or derivative
  works are solely in the form of machine-executable object code generated by
  a source language processor.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
  SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
  FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/
}
#endif // __UTF8_H_901EA323_E1DF_4E6D_B93C_93AF408EC232__
