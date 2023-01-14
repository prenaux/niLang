// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Utils/UnknownImpl.h"
#include "API/niLang/IRegex.h"

namespace ni {

//////////////////////////////////////////////////////////////////////
// Usage
//  Filename patterns are composed of regular (printable) characters which
//  may comprise a filename as well as special pattern matching characters:
//
//      .       Matches a period (.).
//          Note that a period in a filename is not treated any
//          differently than any other character.
//
//      ? or ~  Any.
//          Matches any single character except '/' or '\'.
//
//      *       Closure.
//          Matches zero or more occurences of any characters other
//          than '/' or '\'.
//          Leading '*' characters are allowed.
//
//      ^     Substitute.
//          Similar to '*', this matches zero or more occurences of
//          any characters other than '/', '\', or '.'.
//          Leading '^' characters are allowed.
//
//      [ab]    Set.
//          Matches the single character 'a' or 'b'.
//          If the dash '-' character is to be included, it must
//          immediately follow the opening bracket '['.
//          If the closing bracket ']' character is to be included,
//          it must be preceded by a quote '`'.
//
//      [a-z]   Range.
//          Matches a single character in the range 'a' to 'z'.
//          Ranges and sets may be combined within the same set of
//          brackets.
//
//      [!R]    Exclusive range.
//          Matches a single character not in the range 'R'.
//          If range 'R' includes the dash '-' character, the dash
//          must immediately follow the caret '!'.
//
//      !       Not.
//          Makes the following pattern (up to the next '/') match
//          any filename except those what it would normally match.
//
//      /       Path separator.
//          Matches a '/' or '\' pathname (directory) separator.
//          Multiple separators are treated like a single
//          separator.
//          A leading separator indicates an absolute pathname.
//
//      \       Path separator.
//          Same as the '/' character.
//          Note that this character must be escaped if used within
//          string constants ("\\").
//
//      `       Quote.
//          Makes the next character a regular (nonspecial)
//          character.
//          Note that to match the quote character itself, it must
//          be quoted.
//
//  Upper and lower case alphabetic characters are considered identical,
//  i.e., 'a' and 'A' match each other.
//  (What constitutes a lowercase letter depends on the current locale
//  settings.)
//
//  Spaces and control characters are treated as normal characters.
//
// Examples
//  The following patterns in the left column will match the filenames in
//  the middle column and will not match filenames in the right column:
//
//      Pattern   Will Match            Will Not Match
//      -------   ----------            --------------
//      a     a (only)            (anything else)
//      a.      a. (only)           (anything else)
//      a?c     abc, acc, arc, a.c        a, ac, abbc
//      a*c     ac, abc, abbc, acc, a.c     a, ab, acb, bac
//      a*      a, ab, abb, a., a.b       b, ba
//      *     a, ab, abb, a., .foo, a.foo   (nothing)
//      *.      a., ab., abb., a.foo.     a, ab, a.foo, .foo
//      *.*     a., a.b, ah.bc.foo        a
//      ^     a, ab, abb            a., .foo, a.foo
//      ^.      a., ab., abb.         a, .foo, a.foo
//      ^.*       a, a., .foo, a.foo        ab, abb
//      *2.c    2.c, 12.c, foo2.c, foo.12.c   2x.c
//      a[b-z]c   abc, acc, azc (only)      (anything else)
//      [ab0-9]x    ax, bx, 0x, 9x          zx
//      a[-.]b    a-b, a.b (only)         (anything else)
//      a[!a-z]b    a0b, a.b, a@b         aab, azb, aa0b
//      a[!-b]x   a0x, a+x, acx         a-x, abx, axxx
//      a[-!b]x   a-x, a!x, abx (only)      (anything else)
//      a[`]]x    a]x (only)            (anything else)
//      a``x    a`x (only)            (anything else)
//      oh`!    oh! (only)            (anything else)
//      is`?it    is?it (only)          (anything else)
//      !a?c    a, ac, ab, abb, acb, a.foo    abc, a.c, azc
//


#define FPAT_QUOTE      _A('`')   // Quotes a special char
#define FPAT_DEL        _A('/')   // Path delimiter
#define FPAT_DEL2       _A('\\')    // Path delimiter
#define FPAT_NOT        _A('!')   // Exclusion
#define FPAT_ANY        _A('?')   // Any one char
#define FPAT_ANY2       _A('~')   // Any one char
#define FPAT_CLOS       _A('*')   // Zero or more chars
#define FPAT_SUB        _A('^')     // Zero or more nondelimiters
#define FPAT_SET_L      _A('[')   // Set/range open bracket
#define FPAT_SET_R      _A(']')   // Set/range close bracket
#define FPAT_SET_NOT    _A('!')   // Set exclusion
#define FPAT_SET_AHRU   _A('-')   // Set range of chars

#ifndef FPAT_DELIM
#define FPAT_DELIM       1
#endif
#ifndef FPAT_LOWER
#define FPAT_LOWER( c )  ni::StrToLower(c)
#endif
#ifndef FPAT_CHAR
#define FPAT_CHAR ni::achar
#endif

///////////////////////////////////////////////
//  Checks that filename pattern 'pat' is a well-formed pattern.
//
// Returns
//  1 ( eTrue ) if 'pat' is a valid filename pattern, otherwise 0 ( eFalse ).
//
// Caveats
//  If 'pat' is null, 0 ( eFalse ) is returned.
//
//  If 'pat' is empty ( "" ), 1 ( eTrue ) is returned, and it is considered a
//  valid ( but degenerate ) pattern ( the only filename it matches is the
//  empty ( "" ) string ).
niExportFunc(int) afilepattern_isvalid(const FPAT_CHAR* pat)
{
  int len;

  // Check args
  if (pat == NULL)
    return (0);

  // Verify that the pattern is valid
  for (len = 0; pat[len] != _A('\0'); len++)
  {
    switch (pat[len])
    {
      case FPAT_SET_L:
        // Char set
        len++;
        if (pat[len] == FPAT_SET_NOT)
          len++;              // Set negation

        while (pat[len] != FPAT_SET_R)
        {
          if (pat[len] == FPAT_QUOTE)
            len++;          // Quoted char
          if (pat[len] == _A('\0'))
            return (0); // Missing closing bracket
          len++;

          if (pat[len] == FPAT_SET_AHRU)
          {
            // Char range
            len++;
            if (pat[len] == FPAT_QUOTE)
              len++;      // Quoted char
            if (pat[len] == _A('\0'))
              return (0);     // Missing closing bracket
            len++;
          }

          if (pat[len] == _A('\0'))
            return (0); // Missing closing bracket
        }
        break;

      case FPAT_QUOTE:
        // Quoted char
        len++;
        if (pat[len] == _A('\0'))
          return (0);     // Missing quoted char
        break;

      case FPAT_NOT:
        // Negated pattern
        len++;
        if (pat[len] == _A('\0'))
          return (0);     // Missing subpattern
        break;

      default:
        // Valid character
        break;
    }
  }

  return (1);
}

///////////////////////////////////////////////
//  Attempts to match subpattern 'pat' to subfilename 'fname'.
//
// Returns
//  1 ( eTrue ) if the subfilename matches, otherwise 0 ( eFalse ).
//
// Caveats
//  This does not assume that 'pat' is well-formed.
//
//  If 'pat' is empty ( "" ), the only filename it matches is the empty ( "" )
//  string.
//
//  Some non-empty patterns ( e.g., "" ) will match an empty filename ( "" ).
niExportFunc(int) afilepattern_submatch(const FPAT_CHAR* pat, const FPAT_CHAR* fname)
{
  int fch;
  int pch;
  int i;
  int yes, match;
  int lo, hi;

  // Attempt to match subpattern against subfilename
  while (*pat != _A('\0'))
  {
    fch = *fname;
    pch = *pat;
    pat++;

    switch (pch)
    {
      case FPAT_ANY:
      case FPAT_ANY2:
        // Match a single char
#if FPAT_DELIM
        if (fch == FPAT_DEL || fch == FPAT_DEL2 || fch == _A('\0'))
          return (0);
#else
        if (fch == _A('\0'))
          return (0);
#endif
        fname++;
        break;

      case FPAT_CLOS:
        // Match zero or more chars
        i = 0;
#if FPAT_DELIM
        while (fname[i] != _A('\0') && fname[i] != FPAT_DEL && fname[i] != FPAT_DEL2)
          i++;
#else
        while (fname[i] != _A('\0'))
          i++;
#endif
        while (i >= 0)
        {
          if (afilepattern_submatch(pat, fname + i))
            return (1);
          i--;
        }
        return (0);

      case FPAT_SUB:
        // Match zero or more chars
        i = 0;
        while (fname[i] != _A('\0') &&
#if FPAT_DELIM
               fname[i] != FPAT_DEL && fname[i] != FPAT_DEL2 &&
#endif
               fname[i] != _A('.'))
          i++;
        while (i >= 0)
        {
          if (afilepattern_submatch(pat, fname + i))
            return (1);
          i--;
        }
        return (0);

      case FPAT_QUOTE:
        // Match a quoted char
        pch = *pat;
        if (FPAT_LOWER(fch) != FPAT_LOWER(pch) || pch == _A('\0'))
          return (0);
        fname++;
        pat++;
        break;

      case FPAT_SET_L:
        // Match char set/range
        yes = 1;
        if (*pat == FPAT_SET_NOT)
        {
          pat++;
          yes = 0;      // Set negation
        }

        // Look for [ s ], [ - ], [ abc ], [ a-c ]
        match = !yes;
        while (*pat != FPAT_SET_R && *pat != _A('\0'))
        {
          if (*pat == FPAT_QUOTE)
            pat++;          // Quoted char

          if (*pat == _A('\0'))
            break;
          lo = *pat++;
          hi = lo;

          if (*pat == FPAT_SET_AHRU)
          {
            // Range
            pat++;

            if (*pat == FPAT_QUOTE)
              pat++;      // Quoted char

            if (*pat == _A('\0'))
              break;
            hi = *pat++;
          }

          if (*pat == _A('\0'))
            break;

          // Compare character to set range
          if (FPAT_LOWER(fch) >= FPAT_LOWER(lo) &&
              FPAT_LOWER(fch) <= FPAT_LOWER(hi))
            match = yes;
        }

        if (!match)
          return (0);

        if (*pat == _A('\0'))
          return (0);     // Missing closing bracket

        fname++;
        pat++;
        break;

      case FPAT_NOT:
        // Match only if rest of pattern does not match
        if (*pat == _A('\0'))
          return (0);     // Missing subpattern
        i = afilepattern_submatch(pat, fname);
        return !i;

#if FPAT_DELIM
      case FPAT_DEL:
      case FPAT_DEL2:
        // Match path delimiter char
        if (fch != FPAT_DEL && fch != FPAT_DEL2)
          return (0);
        fname++;
        break;
#endif

      default:
        // Match a ( non-null ) char exactly
        if (FPAT_LOWER(fch) != FPAT_LOWER(pch))
          return (0);
        fname++;
        break;
    }
  }

  // Check for complete match
  if (*fname != _A('\0'))
    return (0);

  // Successful match
  return (1);
}


///////////////////////////////////////////////
//  Attempts to match pattern 'pat' to filename 'fname'.
//
// Returns
//  1 ( eTrue ) if the filename matches, otherwise 0 ( eFalse ).
//
// Caveats
//  If 'fname' is null, zero ( eFalse ) is returned.
//
//  If 'pat' is null, zero ( eFalse ) is returned.
//
//  If 'pat' is empty ( "" ), the only filename it matches is the empty
//  string ( "" ).
//
//  If 'fname' is empty, the only pattern that will match it is the empty
//  string ( "" ).
//
//  If 'pat' is not a well-formed pattern, zero ( eFalse ) is returned.
//
//  Upper and lower case letters are treated the same ; alphabetic
//  characters are converted to lower case before matching occurs.
//  Conversion to lower case is dependent upon the current locale setting.
niExportFunc(int) afilepattern_match(const FPAT_CHAR* pat, const FPAT_CHAR* fname)
{
  int rc;

  // Check args
  if (fname == NULL)
    return (0);

  if (pat == NULL)
    return (0);

  // Verify that the pattern is valid, and get its length
  if (!afilepattern_isvalid(pat))
    return (0);

  // Attempt to match pattern against filename
  if (fname[0] == _A('\0'))
    return (pat[0] == _A('\0'));    // Special case
  rc = afilepattern_submatch(pat, fname);

  return (rc);
}

///////////////////////////////////////////////
// afilepattern_matchn()
//  Attempts to match pattern 'pat' to filename 'fname'.
//  This operates like afilepattern_match() except that it does not verify that
//  pattern 'pat' is well-formed, assuming that it has been checked by a
//  prior call to afilepattern_isvalid().
//
// Returns
//  1 ( eTrue ) if the filename matches, otherwise 0 ( eFalse ).
//
// Caveats
//  If 'fname' is null, zero ( eFalse ) is returned.
//
//  If 'pat' is null, zero ( eFalse ) is returned.
//
//  If 'pat' is empty ( "" ), the only filename it matches is the empty ( "" )
//  string.
//
//  If 'pat' is not a well-formed pattern, unpredictable results may occur.
//
//  Upper and lower case letters are treated the same ; alphabetic
//  characters are converted to lower case before matching occurs.
//  Conversion to lower case is dependent upon the current locale setting.
//
// See also
//  afilepattern_match().
niExportFunc(int) afilepattern_matchn(const FPAT_CHAR* pat, const FPAT_CHAR* fname)
{
  int rc;

  // Check args
  if (fname == NULL)
    return (0);

  if (pat == NULL)
    return (0);

  // Assume that pattern is well-formed

  // Attempt to match pattern against filename
  rc = afilepattern_submatch(pat, fname);

  return (rc);
}

class cFilePatternRegex : public cIUnknownImpl<ni::iRegex> {
 public:
  cFilePatternRegex(const achar* aaszPattern, const achar* aaszPatSplitSep) {
    cString str = aaszPattern;
    if (niIsStringOK(aaszPatSplitSep)) {
      astl::vector<cString> split;
      StringSplit(str,aaszPatSplitSep,&split);
      niLoop(i,split.size()) {
        const achar* p = split[i].Chars();
        if (*p == '!') ++p;
        if (afilepattern_isvalid(p))
          _vPatterns.push_back(split[i]);
      }
    }
    else {
      const achar* p = str.Chars();
      if (*p == '!') ++p;
      if (afilepattern_isvalid(p))
        _vPatterns.push_back(str);
    }
  }

  virtual const ni::achar* __stdcall GetImplType() const {
    return _A("FilePattern");
  }

  virtual tBool __stdcall DoesMatch(const achar* aaszString) const {
    niLoop(i,_vPatterns.size()) {
      const achar* p = _vPatterns[i].Chars();
      if (*p == '!') {
        ++p;
        if (!afilepattern_match(p,aaszString)) {
          return eTrue;
        }
      }
      else {
        if (afilepattern_match(p,aaszString)) {
          return eTrue;
        }
      }
    }
    return eFalse;
  }

 private:
  astl::vector<cString> _vPatterns;
};

niExportFunc(ni::iRegex*) CreateFilePatternRegex(const ni::achar* aaszPattern, const ni::achar* aaszPatSplitSep) {
  niCheckSilent(niStringIsOK(aaszPattern),NULL);
  return niNew ni::cFilePatternRegex(aaszPattern,aaszPatSplitSep);
}

niExportFunc(iUnknown*) New_niLang_FilePatternRegex(const Var& avarA, const Var& avarB) {
  cString regex = VarGetString(avarA);
  cString sep = VarGetString(avarB);
  ni::Ptr<iRegex> ptrRegex = CreateFilePatternRegex(regex.Chars(),sep.Chars());
  return ptrRegex.GetRawAndSetNull();
}

}
