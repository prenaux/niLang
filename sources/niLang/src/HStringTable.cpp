// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/IHString.h"
#include "API/niLang/STL/utils.h"
#include "API/niLang/Utils/UnknownImpl.h"
#include "API/niLang/Utils/CollectionImpl.h"
#include "API/niLang/Utils/Sync.h"
#include "API/niLang/StringLibIt.h"
#include "API/niLang/STL/string_view.h"
#include "API/niLang/STL/hash_map.h"
#include "API/niLang/STL/hash_set.h"
#include "Lang.h"

using namespace ni;

#define niHStringTable_LogLeakedString
// #define niHStringTable_DebugString

_HDecl(HStringVTable);
niExportFunc(void*) ni_get_vtable_object_iHString()
{
  return _HC(HStringVTable);
}

//----------------------------------------------------------------------------
//
// Section: cHStringCharIt
//
//----------------------------------------------------------------------------
class cHStringCharIt : public ImplRC<iHStringCharIt> {
  niBeginClass(cHStringCharIt);

  Ptr<iHString> _hsp;
  StrCharIt _it;

 public:
  cHStringCharIt(const iHString* ahspString, const StrCharIt& aIt)
      : _hsp(ahspString)
      , _it(aIt)
  {
  }
  cHStringCharIt(const iHString* ahspString, tU32 offset, tU32 size)
      : _hsp(ahspString)
      , _it(ahspString->GetChars() + offset, size)
  {
  }

  virtual iHString* __stdcall GetString() const
  {
    return _hsp;
  }
  virtual iHStringCharIt* __stdcall Clone() const
  {
    return niNew cHStringCharIt(_hsp, _it);
  }

  virtual tBool __stdcall GetIsStart() const
  {
    return _it.is_start();
  }
  virtual tBool __stdcall GetIsEnd() const
  {
    return _it.is_end();
  }
  virtual tU32 __stdcall GetPosition() const
  {
    return _it.pos();
  }
  virtual void __stdcall ToPosition(tU32 anOffsetInBytes)
  {
    _it.to_pos(anOffsetInBytes);
  }
  virtual void __stdcall ToStart()
  {
    _it.to_start();
  }
  virtual tSize __stdcall ToEnd()
  {
    return _it.to_end();
  }
  virtual tSize __stdcall GetNumChars() const
  {
    return _it.length();
  }
  virtual tSize __stdcall GetNumBytes() const
  {
    return _it.sizeInBytes();
  }
  virtual tU32 __stdcall PeekNext() const
  {
    if (_it.is_end())
      return 0;
    return _it.peek_next();
  }
  virtual tU32 __stdcall Next()
  {
    if (_it.is_end())
      return 0;
    return _it.next();
  }
  virtual tU32 __stdcall PeekPrior() const
  {
    if (_it.is_start())
      return 0;
    return _it.peek_prior();
  }
  virtual tU32 __stdcall Prior()
  {
    if (_it.is_start())
      return 0;
    return _it.prior();
  }
  virtual tU32 __stdcall PeekAdvance(tU32 fwd) const
  {
    return _it.peek_next(fwd);
  }
  virtual tU32 __stdcall Advance(tU32 n)
  {
    ni::tU32 c = 0;
    niLoop (i, n) {
      c = this->Next();
    }
    return c;
  }
  virtual tU32 __stdcall PeekRewind(tU32 back) const
  {
    return _it.peek_prev(back);
  }
  virtual tU32 __stdcall Rewind(tU32 n)
  {
    ni::tU32 c = 0;
    niLoop (i, n) {
      c = this->Prior();
    }
    return c;
  }
};

//----------------------------------------------------------------------------
//
// Section: sHString
//
//----------------------------------------------------------------------------
struct sHString;

static void _UnregisterHStringFromTable(sHString* apHString);

struct sHString : public ImplRC<iHString, ni::eImplFlags_Default> {
  const achar* maszChars;
  tU32 mnLength;

  sHString()
  {
  }
  ~sHString()
  {
    niAssert(maszChars);
    _UnregisterHStringFromTable(this);
  }

  tBool __stdcall IsOK() const niImpl
  {
    return eTrue;
  }

  void __stdcall Invalidate() niImpl
  {
  }

  const achar* __stdcall GetChars() const niImpl
  {
    return maszChars;
  }
  tU32 __stdcall GetLength() const niImpl
  {
    return mnLength;
  }

  iHString* __stdcall GetLocalized() const niImpl
  {
    return GetLangImpl()->_GetLocalized(NULL, niThis(sHString));
  }
  iHString* __stdcall GetLocalizedEx(iHString* locale) const niImpl
  {
    return GetLangImpl()->_GetLocalized(locale, niThis(sHString));
  }
  tBool __stdcall IsLocalized(iHString* locale) const niImpl
  {
    return GetLangImpl()->_GetLocalized(locale, niThis(sHString)) != this;
  }

  tI32 __stdcall Cmp(const iHString* ahspRight) const niImpl
  {
    if (ahspRight == NULL)
      return -1;
    if (this != ahspRight) {
      return ni::StrCmp(maszChars, ahspRight->GetChars());
    }
    return 0;
  }

  tI32 __stdcall ICmp(const iHString* ahspRight) const niImpl
  {
    if (ahspRight == NULL)
      return -1;
    if (this != ahspRight) {
      return ni::StrICmp(GetChars(), ahspRight->GetChars());
    }
    return 0;
  }

  iHStringCharIt* __stdcall CreateCharIt(tU32 offset) const niImpl
  {
    if (offset > mnLength)
      return NULL;
    return niNew cHStringCharIt(this, offset, mnLength - offset);
  }
  iHStringCharIt* __stdcall CreateRangeIt(tU32 offset, tU32 size) const niImpl
  {
    if (offset + size > mnLength)
      return NULL;
    return niNew cHStringCharIt(this, offset, size);
  }
};

//----------------------------------------------------------------------------
//
// Section: sHStringTable
//
//----------------------------------------------------------------------------
#define USE_HSTRING_SINGLE_ALLOC

struct sHStringTable {
  __sync_mutex_(HStringTable);
  typedef astl::hash_map<astl::string_view, sHString*> tStringMap;
  tStringMap mmapStrings;
  Ptr<iHString> mAtomEmpty;

  sHStringTable()
  {
    mAtomEmpty = CreateHString(astl::string_view("", 0));
  }
  ~sHStringTable()
  {
    //niAssert(mmapStrings.size() == mmapAtoms.size());
  }

  sHStringTable(const sHStringTable&) = delete;
  sHStringTable& operator=(const sHStringTable&) = delete;

  Ptr<sHString> CreateHString(astl::string_view sv)
  {
    __sync_lock_(HStringTable);

    auto it = mmapStrings.find(sv);
    if (it != mmapStrings.end()) {
      return it->second;
    }

#ifdef USE_HSTRING_SINGLE_ALLOC
    // Single allocation: [sHString object][string data + null terminator]
    const size_t stringOffset = (sizeof(sHString) + 7) & ~7; // Align to 8 bytes
    const size_t totalSize = stringOffset + sv.length() + 1;
    void* mem = ni_object_alloc(totalSize, niSourceLoc);

    // Place the string right after the object
    achar* internedStr = (achar*)((tPtr)mem + stringOffset);
    ni::MemCopy((tPtr)internedStr, (tPtr)sv.data(), sv.length());
    internedStr[sv.length()] = 0;

    // Construct the HString object at start of the allocation
    Ptr<sHString> hsp = new (mem) sHString();
    hsp->maszChars = internedStr;
    hsp->mnLength = (tU32)sv.length();
#else
    achar* internedStr = (achar*)niMalloc(sv.length() + 1);
    ni::MemCopy((tPtr)internedStr, (tPtr)sv.data(), sv.length());
    internedStr[sv.length()] = 0;

    Ptr<sHString> hsp = niNew sHString();
    ((sHString*)hsp.ptr())->maszChars = internedStr;
    ((sHString*)hsp.ptr())->mnLength = (tU32)sv.length();
#endif

    astl::upsert(mmapStrings, astl::string_view(internedStr, sv.length()),
                 hsp.ptr());
    return hsp;
  }

  void UnregisterHString(sHString* apHString)
  {
    niAssert(apHString != nullptr);
    __sync_lock_(HStringTable);
    auto it = mmapStrings.find(
      astl::string_view(apHString->maszChars, apHString->mnLength));
    niAssert(it != mmapStrings.end());
#ifdef USE_HSTRING_SINGLE_ALLOC
    // we dont need to call the destructor or free here as UnregisterHString
    // is called from the sHString destructor and is going to release the
    // whole block with ni_object_free()
#else
    niFree((void*)apHString->maszChars);
#endif
    mmapStrings.erase(it);
  }
};

///////////////////////////////////////////////
static inline sHStringTable& _GetHStringTable()
{
  static sHStringTable _hstringTableInst;
  return _hstringTableInst;
}

///////////////////////////////////////////////
static void _UnregisterHStringFromTable(sHString* apHString)
{
  _GetHStringTable().UnregisterHString(apHString);
}

namespace ni {

///////////////////////////////////////////////
niExportFuncCPP(Ptr<iHString>) CreateHStringFromView(
  astl::string_view aStringView)
{
  if (aStringView.empty()) {
    return _GetHStringTable().mAtomEmpty;
  }
  return _GetHStringTable().CreateHString(aStringView);
}

///////////////////////////////////////////////
niExportFunc(iHString*) CreateHStringForC(const char* aStr, std::size_t aLen)
{
  if (!niStringIsOK(aStr)) {
    return _GetHStringTable().mAtomEmpty;
  }
  if (aLen == 0) {
    aLen = ni::StrSize(aStr);
  }
  Ptr<iHString> hsp =
    _GetHStringTable().CreateHString(astl::string_view(aStr, aLen));
  return hsp.GetRawAndSetNull();
}

} // namespace ni

//----------------------------------------------------------------------------
//
// Section: cLang Localization
//
//----------------------------------------------------------------------------

///////////////////////////////////////////////
void __stdcall cLang::SetDefaultLocale(iHString* ahspLocale)
{
  __sync_set_(mhspDefaultLocale, ahspLocale, Locales);
}
iHString* __stdcall cLang::GetDefaultLocale() const
{
  __sync_local_sptr_(iHString, hspDefaultLocale, Locales);
  return hspDefaultLocale.ptr();
}

///////////////////////////////////////////////
tU32 __stdcall cLang::GetNumLocales() const
{
  __sync_local_type_(tLocalesHMap, mapLocales, Locales);
  return (tU32)mapLocales->size();
}

///////////////////////////////////////////////
iHString* __stdcall cLang::GetLocale(tU32 index) const
{
  __sync_local_type_(tLocalesHMap, mapLocales, Locales);
  tU32 c = 0;
  for (tLocalesHMap::iterator it = mapLocales->begin(); it != mapLocales->end();
       ++it)
  {
    if (c++ == index)
      return it->first;
  }
  return NULL;
}

///////////////////////////////////////////////
tBool __stdcall cLang::ClearLocalization(iHString* locale)
{
  __sync_local_type_(tLocalesHMap, mapLocales, Locales);
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
cLang::sLocalizationTable* cLang::_GetLocalizationTable(iHString* locale,
                                                        tBool abNew) const
{
  if (HStringIsEmpty(locale))
    return NULL;
  __sync_llocal_type(tLocalesHMap, mapLocales);
  tLocalesHMap::iterator it = mapLocales->find(locale);
  if (it != mapLocales->end()) {
    return it->second;
  }
  else {
    // create a new localization map if requested
    if (abNew) {
      sLocalizationTable* lmap = new sLocalizationTable();
      astl::upsert(*mapLocales, locale, lmap);
      return lmap;
    }
  }
  return NULL;
}
tBool __stdcall cLang::SetLocalization(iHString* locale, iHString* native,
                                       iHString* localized)
{
  __sync_lock_(Locales);
  niCheckSilent(HStringIsNotEmpty(locale) && HStringIsNotEmpty(native), eFalse);
  sLocalizationTable* ltbl = _GetLocalizationTable(locale, eTrue);
  if (!ltbl)
    return eFalse;
  tLocalizationMap* lmap = &ltbl->map;
  astl::upsert(*lmap, native, localized);
  return eTrue;
}
tBool __stdcall cLang::SetLocalizationMap(iHString* locale,
                                          const tStringCMap* apLocalizationMap)
{
  __sync_lock_(Locales);
  niCheckSilent(HStringIsNotEmpty(locale) && niIsOK(apLocalizationMap), eFalse);
  sLocalizationTable* ltbl = _GetLocalizationTable(locale, eTrue);
  if (!ltbl)
    return eFalse;
  tLocalizationMap* lmap = &ltbl->map;
  for (tStringCMap::const_iterator it = apLocalizationMap->begin();
       it != apLocalizationMap->end(); ++it)
  {
    Ptr<iHString> native = CreateHStringFromView(it->first);
    Ptr<iHString> localized = CreateHStringFromView(it->second);
    astl::upsert(*lmap, native, localized);
  }
  return eTrue;
}
tU32 __stdcall cLang::GetLocalizationMap(iHString* locale,
                                         tStringCMap* apLocalizedMap) const
{
  __sync_lock_(Locales);
  niCheckSilent(HStringIsNotEmpty(locale), eFalse);
  const sLocalizationTable* ltbl = _GetLocalizationTable(locale, eFalse);
  if (!ltbl)
    return 0;
  const tLocalizationMap* lmap = &ltbl->map;
  if (niIsOK(apLocalizedMap)) {
    for (tLocalizationMap::const_iterator it = lmap->begin(); it != lmap->end();
         ++it)
    {
      apLocalizedMap->Put(niHStr(it->first), niHStr(it->second));
    }
  }
  return (tU32)lmap->size();
}

///////////////////////////////////////////////
// return the native string if not localized or invalid locale
iHString* __stdcall cLang::_GetLocalized(iHString* locale, iHString* native)
{
  if (!locale)
    locale = const_cast<Ptr<iHString>&>(mhspDefaultLocale).ptr();
  if (HStringIsEmpty(locale))
    return native;
  __sync_lock_(Locales);
  sLocalizationTable* ltbl =
    _GetLocalizationTable(locale, mbMarkMissingLocalization);
  if (!ltbl)
    return native;
  tLocalizationMap* lmap = &ltbl->map;
  tLocalizationMap::const_iterator lmapIt = lmap->find(native);
  if (lmapIt == lmap->end()) {
    if (mbMarkMissingLocalization) {
      ltbl->missing->push_back(native->GetChars());
      // add an entry so that next try wont be marked as missing
      astl::upsert(*lmap, native, native);
    }
    return native;
  }
  else {
    return lmapIt->second.ptr();
  }
}

///////////////////////////////////////////////
void __stdcall cLang::SetMarkMissingLocalization(tBool abMarkMissing)
{
  mbMarkMissingLocalization = abMarkMissing;
}
tBool __stdcall cLang::GetMarkMissingLocalization() const
{
  return mbMarkMissingLocalization;
}

///////////////////////////////////////////////
tStringCVec* __stdcall cLang::GetMissingLocalization(iHString* locale) const
{
  __sync_lock_(Locales);
  niCheckSilent(HStringIsNotEmpty(locale), NULL);
  sLocalizationTable* ltbl = _GetLocalizationTable(locale, eFalse);
  if (!ltbl)
    return NULL;
  return ltbl->missing;
}
