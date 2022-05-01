// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Types.h"

#if niMinFeatures(15)

#include "pcre/pcre.h"
#include "API/niLang/StringDef.h"
#include "API/niLang/IRegex.h"
#include "API/niLang/Utils/UnknownImpl.h"

using namespace ni;

//---------------------------------------------------------------------------
//
// PME object
//
//---------------------------------------------------------------------------

/// PME wraps the PCRE C API into a more perl-like syntax
/**
 * PME wraps the PCRE C API into a more perl-liek syntax.  It supports single matching,
 *   global matching (where the regex resumes where it left off on the previous iteration),
 *   single and global substitutions (including using backrefs in the substituted strings),
 *   splits based on regex comparisons, and a syntactically easy way to get substrings out f
 *   from backrefs and splits.
 */
typedef ni::cString     tPCREString;

/// stores results from matches
typedef astl::pair<ni::tI32,ni::tI32> tPCREMarker;

struct sPCRENamedTable {
  int _rc;
  int _count;
  int _entrySize;
  char* _nameTable;

  sPCRENamedTable(const pcre* code, tBool abCountOnly = eFalse)
      : _rc(0)
      , _count(0)
      , _entrySize(0)
      , _nameTable(NULL)
  {
    if ((_rc = pcre_fullinfo(code, NULL, PCRE_INFO_NAMECOUNT, &_count)) != 0)
      return;
    if (_count > 0 && !abCountOnly) {
      if ((_rc = pcre_fullinfo(code, NULL, PCRE_INFO_NAMEENTRYSIZE, &_entrySize)) != 0)
        return;
      if ((_rc = pcre_fullinfo(code, NULL, PCRE_INFO_NAMETABLE, &_nameTable)) != 0)
        return;
    }
  }

  tBool IsOK() const {
    return _rc == 0;
  }

  tU32 GetCount() const {
    return (tU32)ni::Max(0,_count);
  }
  const char* GetName(tU32 anIndex) const {
    const char* entry = (_nameTable + _entrySize*anIndex);
    const char* p = entry + 2;
    return p;
  }
  int GetNameNumber(tU32 anIndex) const {
    const char* entry = (_nameTable + _entrySize*anIndex);
    return ((entry[0] << 8) + entry[1]);
  }
};

class PME
{
 private:
  /// PME copy constructor
  PME(const PME & r);
  PME& operator = (const PME & r) const;

 public:
  /// default constructor -- virtually worthless
  PME( );
  /// s is the regular expression, opts are PCRE flags bit-wise or'd together
  PME(const tPCREString & s, unsigned opts = 0, bool isGlobal = false);
  /// destructor
  ~PME();

  /// runs a match on s against the regex 'this' was created with -- returns the number of matches found
  int               match(void* apLastStr, const tPCREString & s, unsigned offset = 0);

  /// splits s based on delimiters matching the regex.
  int               split(const tPCREString & s, ///< string to split on
                          int maxfields = 0 ///< maximum number of fields to be split out.  0 means split all fields, but discard any trailing empty bits.  Negative means split all fields and keep trailing empty bits.  Positive means keep up to N fields including any empty fields less than N.  Anything remaining is in the last field.
                          );

  /// substitutes out whatever matches the regex for the second paramter
  tPCREString             sub (
      void* apLastStr,
      const tPCREString & s,
      const tPCREString & r,
      int dodollarsubstitution = 1 );
  /// study the regular expression to make it faster
  void                    study();
  /// returns the substring from the internal mvMarks vector requires having run match or split first
  tPCREString             operator[](int) const;
  /// resets the regex object -- mostly useful for global matching
  void                    reset();

 public:
  /// used internally for operator[]
  /** \deprecated going away */
  static tPCREString substr(const tPCREString & s,
                            const astl::vector<tPCREMarker> & marks,
                            unsigned index);

  pcre* mpRE; ///< pcre structure from pcre_compile
  unsigned mnOptions; ///< bit-flag options for pcre_compile
  pcre_extra* mpExtra;  ///< results from pcre_study
  int mnMatches; ///< number of matches returned from last pcre_exec call
  astl::vector<tPCREMarker> mvMarks; ///< last set of indexes of matches
  tPCREString mstrLast; ///< copy of the last string matched
  void * mpAddrOfLastString; ///< used for checking for change of string in global match

  bool mbIsGlobal; ///< non-pcre flag for 'g' behaviour
  int  mnLastGlobalPosition; ///< end of last match when mbIsGlobal != 0

  /// compiles the regex -- automatically called on construction
  void compile(const tPCREString & s);

  /// used to make a copy of a regex object
  static pcre * clone_re(pcre * apRE);

  /// deals with $1-type constructs in the replacement string in a substitution
  tPCREString UpdateReplacementString ( const tPCREString & r );

  /// flag as to whether this regex is valid (compiled without error)
  tInt                    mnError;
  cString                 mstrErrorMessage;
  tInt                    mnErrorOffset;
};

PME::PME() {
  reset();
  mnOptions = 9;
  mpRE = NULL;
  mpExtra = NULL;
  mnLastGlobalPosition = 0;
  mbIsGlobal = 0;
  mnMatches = 0;
}

PME::PME(const tPCREString & s, unsigned opts, bool isGlobal) {
  mpRE = NULL;
  mbIsGlobal = isGlobal;
  mnOptions = opts;
  reset();
  compile(s);
  mpExtra = NULL;
  mnLastGlobalPosition = 0;
  mnMatches = 0;
}

PME::~PME() {
  if (mpRE) {
    pcre_free(mpRE);
    mpRE = NULL;
  }
}

void PME::reset()
{
  mpAddrOfLastString = NULL;
  mnLastGlobalPosition = 0;
  mvMarks.clear ( );
  mstrLast.clear ( );
  mnError = 0;
}

void  PME::compile(const tPCREString & s)
{
  niCAssert(sizeof(int) == sizeof(mnError));
  niCAssert(sizeof(int) == sizeof(mnErrorOffset));
  if (mpRE) {
    pcre_free(mpRE);
    mpRE = NULL;
  }
  const char * errorptr;
  mstrErrorMessage.clear();
  mnError = mnErrorOffset = 0;
  mpRE = pcre_compile2(s.c_str(), mnOptions, (int*)&mnError, &errorptr, (int*)&mnErrorOffset, 0);
  if (mnError != 0) {
    mstrErrorMessage = errorptr;
  }
}

pcre* PME::clone_re(pcre * apRE)
{
  if ( !apRE )
    return 0;
  size_t size = 0;
  pcre_fullinfo(apRE, 0, PCRE_INFO_SIZE, &size);
  pcre * newre = (pcre *) new char[size];
  memcpy(newre, apRE, size);
  return newre;
}

int PME::match(void* apLastStr, const tPCREString & s, unsigned offset)
{
  size_t msize = 0;
  pcre_fullinfo(mpRE, 0, PCRE_INFO_CAPTURECOUNT, &msize);
  msize = 3*(msize+1);
  int *m = new int[msize];

  astl::vector<tPCREMarker> marks;

  // if we got a new string, reset the global position counter
  if (mpAddrOfLastString != apLastStr) {
    //    fprintf ( stderr, "PME RESETTING: new string\n" );
    mnLastGlobalPosition = 0;
  }

  if ( mbIsGlobal ) {
    offset += mnLastGlobalPosition;
  }

  //fprintf ( stderr, "string: '%s' length: %d offset: %d\n", s.c_str ( ), s.length ( ), offset );
  mnMatches = pcre_exec(mpRE, mpExtra, s.c_str(), s.length(), offset, 0, m, msize);
  //fprintf ( stderr, "pcre_exec result = %d\n", mnMatches );

  for ( int i = 0, *p = m ; i < mnMatches ; i++, p+=2 ) {
    marks.push_back(tPCREMarker(p[0], p[1]));
  }

  delete[] m;

  // store the last set of results locally, as well as returning them
  mvMarks = marks;
  mstrLast = s;
  mpAddrOfLastString = apLastStr;
  if (mbIsGlobal) {
    if (mnMatches == PCRE_ERROR_NOMATCH) {
      //      fprintf ( stderr, "PME RESETTING: reset for no match\n" );
      mnLastGlobalPosition = 0; // reset the position for next match (perl does this)
    } else if ( mnMatches > 0 ) {
      //      fprintf ( stderr, "PME RESETTING: setting to %d\n", marks[0].second );
      mnLastGlobalPosition = marks[0].second; // increment by the end of the match
    } else {
      //      fprintf ( stderr, "PME RESETTING: reset for no unknown\n" );
      mnLastGlobalPosition = 0;
    }
  }

  int returnvalue = 0;
  if (mnMatches > 0) {
    returnvalue = mnMatches;
  } else {
    returnvalue = 0;
  }

  return returnvalue;
}

tPCREString PME::substr(const tPCREString & s, const astl::vector<tPCREMarker> & marks, unsigned index)
{
  if (index >= marks.size() ) {
    return "";
  }

  int begin = marks[index].first;
  if ( begin == -1 )
    return "";
  int end = marks[index].second;
  return s.substr(begin, end-begin);
}

/** Splits into at most maxfields.  If maxfields is unspecified or 0,
 ** trailing empty matches are discarded.  If maxfields is positive,
 ** no more than maxfields fields will be returned and trailing empty
 ** matches are preserved.  If maxfields is empty, all fields (including
 ** trailing empty ones) are returned.  This *should* be the same as the
 ** perl behaviour
 */
int
PME::split(const tPCREString & s, int maxfields)
{
  /// stores the marks for the split
  astl::vector<tPCREMarker> oMarks;

  // this is a list of current trailing empty matches if maxfields is
  //   unspecified or 0.  If there is stuff in it and a non-empty match
  //   is found, then everything in here is pushed into oMarks and then
  //   the new match is pushed on.  If the end of the string is reached
  //   and there are empty matches in here, they are discarded.
  astl::vector<tPCREMarker> oCurrentTrailingEmpties;

  int nOffset = 0;
  int nMatchesFound = 0;

  // while we are still finding matches and maxfields is 0 or negative
  //   (meaning we get all matches), or we haven't gotten to the number
  //   of specified matches
  int nMatchStatus = match((void*)&s,s,nOffset);
  while ((nMatchStatus) && ((maxfields < 1) || nMatchesFound < maxfields))
  {
    nMatchesFound++;
    //    printf ( "nMatchesFound = %d\n", nMatchesFound );
    // check to see if the match is empty
    if ( nOffset != mvMarks [ 0 ].first ) {
      //fprintf ( stderr, "Match is not empty\n" );
      // if this one isn't empty, then make sure to push anything from
      //   oCurrentTrailingEmpties into oMarks
      oMarks.insert ( oMarks.end ( ),
                      oCurrentTrailingEmpties.begin ( ),
                      oCurrentTrailingEmpties.end ( ) );

      // grab from nOffset to mvMarks[0].first and again from mvMarks[0].second to
      //   the end of the string
      oMarks.push_back ( tPCREMarker ( nOffset, mvMarks [ 0 ].first ) );

    }
    // else the match was empty and we have to do some checking
    //   to see what to do
    else {
      //fprintf ( stderr, "match was empty\n" );
      // if maxfields == 0, discard empty trailing matches
      if ( maxfields == 0 ) {
        //fprintf ( stderr, "putting empty into trailing empties list");
        oCurrentTrailingEmpties.push_back ( tPCREMarker ( nOffset, mvMarks [ 0 ].first ) );

      }
      // else we keep all the matches, empty or no
      else {
        //fprintf ( stderr, "Keeping empty match\n" );
        // grab from nOffset to mvMarks[0].first and again from mvMarks[0].second to
        //   the end of the string
        oMarks.push_back ( tPCREMarker ( nOffset, mvMarks [ 0 ].first ) );

      }

    }

    // set nOffset to the beginning of the second part of the split
    nOffset = mvMarks [ 0 ].second;
  } // end while ( match ( ... ) )

  //fprintf ( stderr, "***match status = %d offset = %d\n", nMatchStatus, nOffset);

  // if there were no matches found, push the whole thing on
  if ( nMatchesFound == 0 ) {
    //    printf ( "Putting the whole thing in..\n" );
    oMarks.push_back ( tPCREMarker ( 0, s.length ( ) ) );
  }
  // if we ran out of matches, then append the rest of the string
  //   onto the end of the last split field
  else if ( maxfields > 0 &&
            nMatchesFound >= maxfields ) {
    //    printf ( "Something else..\n" );
    oMarks [ oMarks.size ( ) - 1 ].second = s.length ( );
  }
  // else we have to add another entry for the end of the string
  else {
    //    printf ( "Something REALLY else..\n" );
    oMarks.push_back ( tPCREMarker ( mvMarks [ 0 ].second, s.length ( ) ) );
  }


  mvMarks = oMarks;

  //fprintf ( stderr, "match returning %d\n", mvMarks.size ( ) );
  mnMatches = mvMarks.size ( );
  return mvMarks.size ( );
}

void PME::study()
{
  const char * errorptr = NULL;
  mpExtra = pcre_study(mpRE, 0, &errorptr);
  // check for error condition
  if (errorptr != NULL) {
    if (mpExtra != NULL) {
      pcre_free(mpExtra);
    }
    mpExtra = NULL;
  }
}

tPCREString PME::operator[](int index) const {
  return substr ( mstrLast, mvMarks, index );
}

tPCREString PME::UpdateReplacementString ( const tPCREString & r ) {
  tPCREString finalreplacement = r;

  // search for each backref and store it out
  PME dollars ( "\\$([0-9]+)", 0, true );

  // search each backref
  while ( int numdollars = dollars.match((void*)&r,r)) {

    // create a regex to replace the backref
    tPCREString regextext;
    regextext << "[$]" << dollars[1];
    PME dollarsub(regextext);

    // do the replacement, telling it not to look for backref
    finalreplacement = dollarsub.sub (
        (void*)&finalreplacement,
        finalreplacement,
        (*this)[atoi(dollars[1].c_str())],
        0 );
  }

  return finalreplacement;
}

tPCREString PME::sub(void* apLastStr, const tPCREString& s, const tPCREString& r, int dodollarsubstitution)
{
  tPCREString newstream;
  if (mbIsGlobal) {
    int endoflastmatch = 0;
    while (match(apLastStr,s)) {
      // copy from the end of the last match to the beginning of the current match, then
      // copy in the replacement
      newstream << s.substr ( endoflastmatch, mvMarks[0].first - endoflastmatch );
      tPCREString finalreplacement = r;
      if (dodollarsubstitution) {
        finalreplacement = UpdateReplacementString ( r );
      }
      newstream << finalreplacement;
      endoflastmatch = mvMarks[0].second;
    }
    // copy the last bit
    newstream << s.substr ( endoflastmatch );
  }
  else {
    int mnMatches = match(apLastStr,s);
    if (mnMatches > 0) {
      tPCREString finalreplacement = r;
      if (dodollarsubstitution) {
        finalreplacement = UpdateReplacementString ( r );
      }

      newstream << s.substr ( 0, mvMarks[0].first );
      newstream << finalreplacement;
      newstream << s.substr ( mvMarks[0].second );
    } else {
      newstream << s;
    }
  }

  return newstream;
}

//---------------------------------------------------------------------------
//
// PCRE utils
//
//---------------------------------------------------------------------------
static unsigned int _GetPcreOptions(const tPCREString& opts) {
  unsigned int return_opts = 0;
  if ( strchr ( opts.c_str ( ), 'i' ) ) {
    return_opts |= PCRE_CASELESS;
  }
  if ( strchr ( opts.c_str ( ), 'm' ) ) {
    return_opts |= PCRE_MULTILINE;
  }
  if ( strchr ( opts.c_str ( ), 's' ) ) {
    return_opts |= PCRE_DOTALL;
  }
  if ( strchr ( opts.c_str ( ), 'x' ) ) {
    return_opts |= PCRE_EXTENDED;
  }
  // not perl compatible
  if ( strchr ( opts.c_str ( ), 'U' ) ) {
    return_opts |= PCRE_UNGREEDY;
  }

  // if 'g' is set, it stores the previous
  //   result end position
  if ( strchr ( opts.c_str ( ), 'g' ) ) {
    return_opts |= ePCREOptionsFlags_Global;
  }

  return return_opts;
}

//---------------------------------------------------------------------------
//
// PCRE implementation
//
//---------------------------------------------------------------------------
class cPCRE : public ni::cIUnknownImpl<ni::iPCRE,ni::eIUnknownImplFlags_DontInherit1,ni::iRegex>
{
  niBeginClass(cPCRE);

 public:
  ///////////////////////////////////////////////
  cPCRE() {
    ZeroMembers();
  }

  ///////////////////////////////////////////////
  ~cPCRE() {
    Invalidate();
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
    Reset();
  }

  ///////////////////////////////////////////////
  void __stdcall ZeroMembers() {
    _pme.reset();
    _isCompiled = eFalse;
  }

  ///////////////////////////////////////////////
  ni::tBool __stdcall IsOK() const {
    niClassIsOK(cPCRE);
    return ni::eTrue;
  }

  ///////////////////////////////////////////////
  const achar* __stdcall GetImplType() const {
    return _A("PCRE");
  }
  ni::tBool __stdcall DoesMatch(const achar* aaszString) const {
    return niThis(cPCRE)->MatchRaw(aaszString) > 0;
  }

  ///////////////////////////////////////////////
  virtual ePCREError __stdcall Compile(const achar* aaszRegEx, const achar* aaszOpt) {
    tU32 opt = _GetPcreOptions(aaszOpt);
    return Compile2(aaszRegEx,opt);
  }
  virtual ePCREError __stdcall Compile2(const achar* aaszRegEx, tPCREOptionsFlags aOpt) {
    if (!niStringIsOK(aaszRegEx))
      return ePCREError_BadRegexString;

    _isCompiled = eFalse;

    // get options
    unsigned opts = aOpt;
    opts &= ~ePCREOptionsFlags_Global;
    opts &= ~ePCREOptionsFlags_Optimize;

    _pme.reset();
    _pme.mnOptions = opts;
    _pme.mbIsGlobal = niFlagIs(aOpt,ePCREOptionsFlags_Global);
    _pme.compile(aaszRegEx);
    if (_pme.mnError != 0) {
      cString e;
      e << "Compilation error ("
        << (tI32)_pme.mnError << ":" << (tI32)_pme.mnErrorOffset << ") : "
        << _pme.mstrErrorMessage.Chars();
      niError(e.Chars());
      return (ePCREError)_pme.mnError;
    }
    if (niFlagIs(aOpt,ePCREOptionsFlags_Optimize)) {
      _pme.study();
    }

    _isCompiled = eTrue;
    return ePCREError_OK;
  }

  ///////////////////////////////////////////////
  virtual ePCREError __stdcall GetLastCompileError() const {
    return (ePCREError)_pme.mnError;
  }
  virtual const achar* __stdcall GetLastCompileErrorDesc() const {
    return _pme.mstrErrorMessage.Chars();
  }
  virtual tU32 __stdcall GetLastCompileErrorOffset() const {
    return (tU32)_pme.mnErrorOffset;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall GetIsCompiled() const {
    return _isCompiled;
  }

  ///////////////////////////////////////////////
  virtual tPCREOptionsFlags __stdcall GetOptions() const {
    if (!_isCompiled) return 0;
    return _pme.mnOptions;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall Reset() {
    _pme.reset();
  }
  virtual tU32 __stdcall GetNumMarkers() const {
    if (!_isCompiled) return 0;
    return _pme.mvMarks.size();
  }
  virtual sVec2i __stdcall GetMarker(tU32 anIndex) const {
    if (!_isCompiled || anIndex > _pme.mvMarks.size()) return sVec2i::Zero();
    tPCREMarker m = _pme.mvMarks[anIndex];
    return Vec2(m.first,m.second);
  }
  virtual cString __stdcall GetString(tU32 anIndex) const {
    if (!_isCompiled || anIndex > _pme.mvMarks.size()) return AZEROSTR;
    return _pme[anIndex].c_str();
  }

  ///////////////////////////////////////////////
  virtual tU32 __stdcall GetNumNamed() const {
    if (!_isCompiled) return 0;
    return sPCRENamedTable(_pme.mpRE).GetCount();
  }
  virtual const cchar* __stdcall GetNamedName(tU32 anIndex) const {
    if (!_isCompiled) return AZEROSTR;
    sPCRENamedTable table(_pme.mpRE);
    if (anIndex >= table.GetCount())
      return AZEROSTR;
    return table.GetName(anIndex);
  }
  virtual sVec2i __stdcall GetNamedMarker(const cchar* aaszName) const {
    tU32 i = GetNamedIndex(aaszName);
    return GetMarker(i);
  }
  virtual cString __stdcall GetNamedString(const cchar* aaszName) const {
    tU32 i = GetNamedIndex(aaszName);
    return GetString(i);
  }
  virtual tU32 __stdcall GetNamedIndex(const cchar* aaszName) const {
    if (!_isCompiled) return eInvalidHandle;
    return pcre_get_stringnumber(_pme.mpRE,aaszName);
  }

  ///////////////////////////////////////////////
  virtual tI32 __stdcall MatchRaw(const achar* aaszString) {
    niCheck(_isCompiled,0);
    if (!niStringIsOK(aaszString)) return 0;
    return _pme.match((void*)aaszString,aaszString);
  }
  virtual tI32 __stdcall Match(iHString* ahspString, tU32 anOffset) {
    niCheck(_isCompiled,0);
    if (!ahspString || HStringIsEmpty(ahspString))
      return 0;
    if (anOffset && anOffset >= ahspString->GetLength())
      return 0;
    return MatchRaw(ahspString->GetChars()+anOffset);
  }

  ///////////////////////////////////////////////
  virtual tI32 __stdcall Split(const achar* aaszString, tI32 anMaxFields) {
    niCheck(_isCompiled,0);
    if (!niStringIsOK(aaszString)) return 0;
    return _pme.split(aaszString,anMaxFields);
  }

  ///////////////////////////////////////////////
  virtual cString __stdcall Sub(const achar* aaszString, const achar* aaszReplacement, tBool abDoDollarSub) {
    niCheck(_isCompiled,AZEROSTR);
    if (!niStringIsOK(aaszString)) return AZEROSTR;
    if (!niStringIsOK(aaszReplacement)) return AZEROSTR;
    tPCREString str = _pme.sub((void*)aaszString,aaszString,aaszReplacement,abDoDollarSub);
    return str.Chars();
  }

 private:
  PME _pme;
  tBool _isCompiled;

  niEndClass(cPCRE);
};

namespace ni {

niExportFunc(ni::iPCRE*) CreatePCRE(const achar* aRegex, const achar* aOptions) {
  ni::Ptr<iPCRE> pcre = niNew cPCRE();
  if (niStringIsOK(aRegex)) {
    if (pcre->Compile(aRegex,aOptions) != ePCREError_OK) {
      cString e;
      e << "Can't compile pcre regex '" << aRegex << "'.";
      niError(e.Chars());
      return NULL;
    }
  }
  return pcre.GetRawAndSetNull();
}

niExportFunc(iUnknown*) New_niLang_PCRE(const Var& avarA, const Var& avarB) {
  ni::Ptr<iPCRE> pcre = CreatePCRE();
  if (VarIsString(avarA)) {
    cString regex = VarGetString(avarA);
    cString options = VarGetString(avarB);
    if (pcre->Compile(regex.Chars(),options.Chars()) != ePCREError_OK) {
      cString e;
      e << "Can't compile pcre regex '" << regex << "'.";
      niError(e.Chars());
      return NULL;
    }
  }
  return pcre.GetRawAndSetNull();
}

}
#endif // #if niMinFeatures(15)
