// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/IHString.h"
#include "API/niLang/STL/utils.h"
#include "API/niLang/Utils/UnknownImpl.h"
#include "API/niLang/Utils/CollectionImpl.h"
#include "API/niLang/Utils/Sync.h"
#include "API/niLang/StringLibIt.h"
#include "Lang.h"

using namespace ni;

#define niHStringTable_LogLeakedString
// #define niHStringTable_DebugString

_HDecl(HStringVTable);
niExportFunc(void*) ni_get_vtable_object_iHString() {
  return _HC(HStringVTable);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// cHStringCharIt declaration.
class cHStringCharIt : public ImplRC<iHStringCharIt>
{
  niBeginClass(cHStringCharIt);

  tHStringPtr _hsp;
  StrCharIt _it;

 public:
  cHStringCharIt(const iHString* ahspString, const StrCharIt& aIt)
      : _hsp(ahspString)
      , _it(aIt)
  {
  }
  cHStringCharIt(const iHString* ahspString, tU32 offset, tU32 size)
      : _hsp(ahspString)
      , _it(ahspString->GetChars()+offset,size)
  {
  }

  virtual iHString* __stdcall GetString() const {
    return _hsp;
  }
  virtual iHStringCharIt* __stdcall Clone() const {
    return niNew cHStringCharIt(_hsp,_it);
  }

  virtual tBool __stdcall GetIsStart() const {
    return _it.is_start();
  }
  virtual tBool __stdcall GetIsEnd() const {
    return _it.is_end();
  }
  virtual tU32 __stdcall GetPosition() const {
    return _it.pos();
  }
  virtual void __stdcall ToPosition(tU32 anOffsetInBytes) {
    _it.to_pos(anOffsetInBytes);
  }
  virtual void __stdcall ToStart() {
    _it.to_start();
  }
  virtual tSize __stdcall ToEnd() {
    return _it.to_end();
  }
  virtual tSize __stdcall GetNumChars() const {
    return _it.length();
  }
  virtual tSize __stdcall GetNumBytes() const {
    return _it.sizeInBytes();
  }
  virtual tU32 __stdcall PeekNext() const {
    if (_it.is_end()) return 0;
    return _it.peek_next();
  }
  virtual tU32 __stdcall Next() {
    if (_it.is_end()) return 0;
    return _it.next();
  }
  virtual tU32 __stdcall PeekPrior() const {
    if (_it.is_start()) return 0;
    return _it.peek_prior();
  }
  virtual tU32 __stdcall Prior() {
    if (_it.is_start()) return 0;
    return _it.prior();
  }
  virtual tU32 __stdcall PeekAdvance(tU32 fwd) const {
    return _it.peek_next(fwd);
  }
  virtual tU32 __stdcall Advance(tU32 n) {
    ni::tU32 c = 0;
    niLoop(i,n) {
      c = this->Next();
    }
    return c;
  }
  virtual tU32 __stdcall PeekRewind(tU32 back) const {
    return _it.peek_prev(back);
  }
  virtual tU32 __stdcall Rewind(tU32 n) {
    ni::tU32 c = 0;
    niLoop(i,n) {
      c = this->Prior();
    }
    return c;
  }
};

//////////////////////////////////////////////////////////////////////////////////////////////
// cHString implementation.
class cHString :  public ImplRC<iHString,ni::eImplFlags_Default>
{
  niBeginClass(cHString);

 public:
  cHString() {
  }
  ~cHString() {
    niAssert(maszChars);
    GetLangImpl()->_UnregisterHString(this);
  }

  tBool __stdcall IsOK() const niImpl {
    niClassIsOK(cHString);
    return eTrue;
  }

  void __stdcall Invalidate() niImpl {
  }

  const achar* __stdcall GetChars() const niImpl {
    return maszChars;
  }
  tU32 __stdcall GetLength() const niImpl {
    return mnLength;
  }

  iHString* __stdcall GetLocalized() const niImpl {
    return GetLangImpl()->_GetLocalized(NULL,niThis(cHString));
  }
  iHString* __stdcall GetLocalizedEx(iHString* locale) const niImpl {
    return GetLangImpl()->_GetLocalized(locale,niThis(cHString));
  }
  tBool __stdcall IsLocalized(iHString* locale) const niImpl {
    return GetLangImpl()->_GetLocalized(locale,niThis(cHString)) != this;
  }

  tI32 __stdcall Cmp(const iHString* ahspRight) const niImpl {
    if (ahspRight == NULL)
      return -1;
    if (this != ahspRight) {
      return ni::StrCmp(maszChars,ahspRight->GetChars());
    }
    return 0;
  }

  tI32 __stdcall ICmp(const iHString* ahspRight) const niImpl {
    if (ahspRight == NULL)
      return -1;
    if (this != ahspRight) {
      return ni::StrICmp(GetChars(),ahspRight->GetChars());
    }
    return 0;
  }

  iHStringCharIt* __stdcall CreateCharIt(tU32 offset) const niImpl {
    if (offset > mnLength) return NULL;
    return niNew cHStringCharIt(this,offset,mnLength-offset);
  }
  iHStringCharIt* __stdcall CreateRangeIt(tU32 offset, tU32 size) const niImpl {
    if (offset+size > mnLength) return NULL;
    return niNew cHStringCharIt(this,offset,size);
  }

 public:
  const achar* maszChars;
  tU32 mnLength;

  niEndClass(cHString);
};

//////////////////////////////////////////////////////////////////////////////////////////////
// HStringTable implementation.

///////////////////////////////////////////////
// TODO: This isnt called anywhere atm, ideally we should call it at the program's exit.
void __stdcall cLang::_InvalidateHStringTable()
{
  __sync_local_type_(tStringTableHMap,mapHStrings,HStringTable);
  while (GetNumLocales()) {
    ClearLocalization(GetLocale(0));
  }
  if (!mapHStrings->empty())
  {
    niWarning(niFmt(_A("HStringTable invalidated, still has %d strings inside."),
                    mapHStrings->size()));
#ifdef niHStringTable_LogLeakedString
    tU32 count = 0;
    for (tStringTableHMap::const_iterator it = mapHStrings->begin();
         it != mapHStrings->end(); ++it, ++count)
    {
      iHString* pHStr = it->second;
      niWarning(niFmt(_A("string[%d]: %s (%d refs)"),
                      count,it->first.Chars(),pHStr->GetNumRefs()));
    }
    niWarning(niFmt(_A("-------------------------")));
#endif
  }
}

///////////////////////////////////////////////
tHStringPtr cLang::CreateHString(const cString& astrKey) {
  Ptr<iHString> hsp;
  {
    __sync_local_type_(tStringTableHMap,mapHStrings,HStringTable);
    tStringTableHMap::iterator it = mapHStrings->find(astrKey);
    if (it == mapHStrings->end()) {
      hsp = niNew cHString();
      it = astl::upsert(*mapHStrings, astrKey, hsp);
      ((cHString*)hsp.ptr())->maszChars = it->first.Chars();
      ((cHString*)hsp.ptr())->mnLength = it->first.Length();
      niDebugAssertMsg(
        mapHStrings->find(hsp->GetChars()) != mapHStrings->end(),
        "Can't find string just after adding it, string likely contains a zero before its length.");
    }
    else {
      hsp = (cHString*)it->second;
    }
  }
  return hsp;
}

///////////////////////////////////////////////
void cLang::_UnregisterHString(iHString* apHString) {
  __sync_local_type_(tStringTableHMap,mapHStrings,HStringTable);
  tStringTableHMap::iterator it = mapHStrings->find(apHString->GetChars());
  niPanicAssert(it != mapHStrings->end());
  mapHStrings->erase(it);
}

///////////////////////////////////////////////
void __stdcall cLang::SetDefaultLocale(iHString* ahspLocale) {
  __sync_set_(mhspDefaultLocale,ahspLocale,HStringTable);
}
iHString* __stdcall cLang::GetDefaultLocale() const {
  __sync_local_sptr_(iHString,hspDefaultLocale,HStringTable);
  return hspDefaultLocale.ptr();
}

///////////////////////////////////////////////
tU32 __stdcall cLang::GetNumLocales() const {
  __sync_local_type_(tLocalesHMap,mapLocales,HStringTable);
  return (tU32)mapLocales->size();
}

///////////////////////////////////////////////
iHString* __stdcall cLang::GetLocale(tU32 index) const {
  __sync_local_type_(tLocalesHMap,mapLocales,HStringTable);
  tU32 c = 0;
  for (tLocalesHMap::iterator it = mapLocales->begin(); it != mapLocales->end(); ++it) {
    if (c++ == index)
      return it->first;
  }
  return NULL;
}

///////////////////////////////////////////////
tBool __stdcall cLang::ClearLocalization(iHString* locale) {
  __sync_local_type_(tLocalesHMap,mapLocales,HStringTable);
  tLocalesHMap::iterator it = mapLocales->find(locale);
  if (it != mapLocales->end()) {
    tLocalizationMap* lmap = &it->second->map;
    lmap->clear();
    delete lmap;
    mapLocales->erase(it);
    return eTrue;
  }
  else {
    return eFalse;
  }
}

///////////////////////////////////////////////
cLang::sLocalizationTable* cLang::_GetLocalizationTable(iHString* locale, tBool abNew) const {
  if (HStringIsEmpty(locale))
    return NULL;
  __sync_llocal_type(tLocalesHMap,mapLocales);
  tLocalesHMap::iterator it = mapLocales->find(locale);
  if (it != mapLocales->end()) {
    return it->second;
  }
  else {
    // create a new localization map if requested
    if (abNew) {
      sLocalizationTable* lmap = new sLocalizationTable();
      astl::upsert(*mapLocales,locale,lmap);
      return lmap;
    }
  }
  return NULL;
}
tBool __stdcall cLang::SetLocalization(iHString* locale, iHString* native, iHString* localized) {
  __sync_lock_(HStringTable);
  niCheckSilent(HStringIsNotEmpty(locale) &&
                HStringIsNotEmpty(native), eFalse);
  sLocalizationTable* ltbl = _GetLocalizationTable(locale,eTrue);
  if (!ltbl)
    return eFalse;
  tLocalizationMap* lmap = &ltbl->map;
  astl::upsert(*lmap,native,localized);
  return eTrue;
}
tBool __stdcall cLang::SetLocalizationMap(iHString* locale, const tStringCMap* apLocalizationMap) {
  __sync_lock_(HStringTable);
  niCheckSilent(HStringIsNotEmpty(locale) && niIsOK(apLocalizationMap), eFalse);
  sLocalizationTable* ltbl = _GetLocalizationTable(locale,eTrue);
  if (!ltbl)
    return eFalse;
  tLocalizationMap* lmap = &ltbl->map;
  for (tStringCMap::const_iterator it = apLocalizationMap->begin();
       it != apLocalizationMap->end();
       ++it)
  {
    tHStringPtr native = CreateHString(it->first);
    tHStringPtr localized = CreateHString(it->second);
    astl::upsert(*lmap,native,localized);
  }
  return eTrue;
}
tU32 __stdcall cLang::GetLocalizationMap(iHString* locale, tStringCMap* apLocalizedMap) const {
  __sync_lock_(HStringTable);
  niCheckSilent(HStringIsNotEmpty(locale), eFalse);
  const sLocalizationTable* ltbl = _GetLocalizationTable(locale,eFalse);
  if (!ltbl)
    return 0;
  const tLocalizationMap* lmap = &ltbl->map;
  if (niIsOK(apLocalizedMap)) {
    for (tLocalizationMap::const_iterator it = lmap->begin(); it != lmap->end(); ++it) {
      apLocalizedMap->Put(niHStr(it->first),niHStr(it->second));
    }
  }
  return (tU32)lmap->size();
}

///////////////////////////////////////////////
// return the native string if not localized or invalid locale
iHString* __stdcall cLang::_GetLocalized(iHString* locale, iHString* native) {
  if (!locale)
    locale = const_cast<tHStringPtr&>(mhspDefaultLocale).ptr();
  if (HStringIsEmpty(locale))
    return native;
  __sync_lock_(HStringTable);
  sLocalizationTable* ltbl = _GetLocalizationTable(locale,mbMarkMissingLocalization);
  if (!ltbl)
    return native;
  tLocalizationMap* lmap = &ltbl->map;
  tLocalizationMap::const_iterator lmapIt = lmap->find(native);
  if (lmapIt == lmap->end()) {
    if (mbMarkMissingLocalization) {
      ltbl->missing->push_back(native->GetChars());
      // add an entry so that next try wont be marked as missing
      astl::upsert(*lmap,native,native);
    }
    return native;
  }
  else {
    return lmapIt->second.ptr();
  }
}

///////////////////////////////////////////////
void __stdcall cLang::SetMarkMissingLocalization(tBool abMarkMissing) {
  mbMarkMissingLocalization = abMarkMissing;
}
tBool __stdcall cLang::GetMarkMissingLocalization() const {
  return mbMarkMissingLocalization;
}

///////////////////////////////////////////////
tStringCVec* __stdcall cLang::GetMissingLocalization(iHString* locale) const {
  __sync_lock_(HStringTable);
  niCheckSilent(HStringIsNotEmpty(locale), NULL);
  sLocalizationTable* ltbl = _GetLocalizationTable(locale,eFalse);
  if (!ltbl)
    return NULL;
  return ltbl->missing;
}
