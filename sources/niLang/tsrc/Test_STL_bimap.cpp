#include "stdafx.h"
#include "../src/API/niLang/IHString.h"
#include "../src/API/niLang/STL/bimap.h"

typedef astl::bimap<ni::cString,ni::tU32> tStrIntBMap;
typedef astl::bimap<ni::tUUID,ni::tHStringPtr> tUUIDNameBMap;
typedef astl::bimap<ni::tHStringPtr,ni::tUUID> tNameUUIDBMap;

struct ASTL_bimap {};

TEST_FIXTURE(ASTL_bimap,strmap) {
  TEST_TRACK_MEMORY_BEGIN();

  tStrIntBMap bmap;

  bmap.insert(_A("a"),123);
  bmap.insert(_A("a"),1);
  bmap.insert(_A("b"),2);
  bmap.insert(_A("c"),3);
  bmap.insert(_A("d"),4);
  bmap.insert(_A("e"),5);
  bmap.insert(_A("f"),6);
  bmap.insert(_A("F"),6);

  tStrIntBMap::const_iterator_to itTo = bmap.findTo(4);
  CHECK_EQUAL(_ASTR(tStrIntBMap::firstTo(itTo)),_ASTR("d"));

  tStrIntBMap::const_iterator_from itFrom = bmap.findFrom("d");
  CHECK_EQUAL(tStrIntBMap::secondFrom(itFrom),4);

  CHECK(!bmap.hasTo(123));
  CHECK(bmap.hasTo(1));

  CHECK_EQUAL(1,bmap.getFrom(_A("a")));
  CHECK_EQUAL(2,bmap.getFrom(_A("b")));
  CHECK_EQUAL(3,bmap.getFrom(_A("c")));
  CHECK_EQUAL(4,bmap.getFrom(_A("d")));
  CHECK_EQUAL(5,bmap.getFrom(_A("e")));

  CHECK_EQUAL(_ASTR("a"),bmap.getTo(1));
  CHECK_EQUAL(_ASTR("b"),bmap.getTo(2));
  CHECK_EQUAL(_ASTR("c"),bmap.getTo(3));
  CHECK_EQUAL(_ASTR("d"),bmap.getTo(4));
  CHECK_EQUAL(_ASTR("e"),bmap.getTo(5));

  CHECK(!bmap.hasFrom("f"));
  CHECK(bmap.hasFrom("F"));
  CHECK_EQUAL(6,bmap.getFrom(_A("F")));
  CHECK_EQUAL(_ASTR("F"),bmap.getTo(6));

  CHECK_EQUAL(6,bmap.size());
  bmap.eraseFrom(bmap.findFrom(_A("a")));
  CHECK_EQUAL(5,bmap.size());
  CHECK(!bmap.hasTo(1));
  CHECK(!bmap.hasFrom(_A("a")));

  TEST_TRACK_MEMORY_END();
}

TEST_FIXTURE(ASTL_bimap,uuid) {
  ni::tHStringPtr hspIHString = _H("iHString");
  ni::tHStringPtr hspILang = _H("iLang");
  ni::tHStringPtr hspIUnknown = _H("iUnknown");
  ni::tHStringPtr hspIDispatch = _H("iDispatch");
  const ni::tU32 numTests = 1000;
  TEST_TIMING_BEGIN(bmapHStrLookup) {
    tUUIDNameBMap bmap;
    bmap.insert(niGetInterfaceUUID(ni::iHString), hspIHString);
    bmap.insert(niGetInterfaceUUID(ni::iLang), hspILang);
    bmap.insert(niGetInterfaceUUID(ni::iUnknown), hspIUnknown);
    bmap.insert(niGetInterfaceUUID(ni::iDispatch), hspIDispatch);
    ni::tU32 c = 0;
    niLoop(i,numTests) {
      if (bmap.getTo(hspIHString) == niGetInterfaceUUID(ni::iHString))
        ++c;
      if (bmap.getTo(hspILang) == niGetInterfaceUUID(ni::iLang))
        ++c;
      if (bmap.getTo(hspIUnknown) == niGetInterfaceUUID(ni::iUnknown))
        ++c;
      if (bmap.getTo(hspIDispatch) == niGetInterfaceUUID(ni::iDispatch))
        ++c;
    }
    CHECK_EQUAL(4*numTests,c);
  } TEST_TIMING_END();
  TEST_TIMING_BEGIN(bmap2HStrLookup) {
    tNameUUIDBMap bmap;
    bmap.insert(hspIHString,niGetInterfaceUUID(ni::iHString));
    bmap.insert(hspILang,niGetInterfaceUUID(ni::iLang));
    bmap.insert(hspIUnknown,niGetInterfaceUUID(ni::iUnknown));
    bmap.insert(hspIDispatch,niGetInterfaceUUID(ni::iDispatch));
    ni::tU32 c = 0;
    niLoop(i,numTests) {
      if (bmap.getFrom(hspIHString) == niGetInterfaceUUID(ni::iHString))
        ++c;
      if (bmap.getFrom(hspILang) == niGetInterfaceUUID(ni::iLang))
        ++c;
      if (bmap.getFrom(hspIUnknown) == niGetInterfaceUUID(ni::iUnknown))
        ++c;
      if (bmap.getFrom(hspIDispatch) == niGetInterfaceUUID(ni::iDispatch))
        ++c;
    }
    CHECK_EQUAL(4*numTests,c);
  } TEST_TIMING_END();
  TEST_TIMING_BEGIN(mapHStrLookup) {
    astl::map<ni::tHStringPtr,ni::tUUID> map;
    map[hspIHString] = niGetInterfaceUUID(ni::iHString);
    map[hspILang] = niGetInterfaceUUID(ni::iLang);
    map[hspIUnknown] = niGetInterfaceUUID(ni::iUnknown);
    map[hspIDispatch] = niGetInterfaceUUID(ni::iDispatch);
    ni::tU32 c = 0;
    niLoop(i,numTests) {
      if (map[hspIHString] == niGetInterfaceUUID(ni::iHString))
        ++c;
      if (map[hspILang] == niGetInterfaceUUID(ni::iLang))
        ++c;
      if (map[hspIUnknown] == niGetInterfaceUUID(ni::iUnknown))
        ++c;
      if (map[hspIDispatch] == niGetInterfaceUUID(ni::iDispatch))
        ++c;
    }
    CHECK_EQUAL(4*numTests,c);
  } TEST_TIMING_END();
  TEST_TIMING_BEGIN(bmapUUIDLookup) {
    tUUIDNameBMap bmap;
    bmap.insert(niGetInterfaceUUID(ni::iHString), hspIHString);
    bmap.insert(niGetInterfaceUUID(ni::iLang), hspILang);
    bmap.insert(niGetInterfaceUUID(ni::iUnknown), hspIUnknown);
    bmap.insert(niGetInterfaceUUID(ni::iDispatch), hspIDispatch);
    ni::tU32 c = 0;
    niLoop(i,numTests) {
      if (bmap.getFrom(niGetInterfaceUUID(ni::iHString)) == hspIHString)
        ++c;
      if (bmap.getFrom(niGetInterfaceUUID(ni::iLang)) == hspILang)
        ++c;
      if (bmap.getFrom(niGetInterfaceUUID(ni::iUnknown)) == hspIUnknown)
        ++c;
      if (bmap.getFrom(niGetInterfaceUUID(ni::iDispatch)) == hspIDispatch)
        ++c;
    }
    CHECK_EQUAL(4*numTests,c);
  } TEST_TIMING_END();
  TEST_TIMING_BEGIN(bmap2UUIDLookup) {
    tNameUUIDBMap bmap;
    bmap.insert(hspIHString,niGetInterfaceUUID(ni::iHString));
    bmap.insert(hspILang,niGetInterfaceUUID(ni::iLang));
    bmap.insert(hspIUnknown,niGetInterfaceUUID(ni::iUnknown));
    bmap.insert(hspIDispatch,niGetInterfaceUUID(ni::iDispatch));
    ni::tU32 c = 0;
    niLoop(i,numTests) {
      if (bmap.getTo(niGetInterfaceUUID(ni::iHString)) == hspIHString)
        ++c;
      if (bmap.getTo(niGetInterfaceUUID(ni::iLang)) == hspILang)
        ++c;
      if (bmap.getTo(niGetInterfaceUUID(ni::iUnknown)) == hspIUnknown)
        ++c;
      if (bmap.getTo(niGetInterfaceUUID(ni::iDispatch)) == hspIDispatch)
        ++c;
    }
    CHECK_EQUAL(4*numTests,c);
  } TEST_TIMING_END();
  TEST_TIMING_BEGIN(mapUUIDLookup) {
    astl::map<ni::tUUID,ni::tHStringPtr> map;
    map[niGetInterfaceUUID(ni::iHString)] = hspIHString;
    map[niGetInterfaceUUID(ni::iLang)] = hspILang;
    map[niGetInterfaceUUID(ni::iUnknown)] = hspIUnknown;
    map[niGetInterfaceUUID(ni::iDispatch)] = hspIDispatch;
    ni::tU32 c = 0;
    niLoop(i,numTests) {
      if (map[niGetInterfaceUUID(ni::iHString)] == hspIHString)
        ++c;
      if (map[niGetInterfaceUUID(ni::iLang)] == hspILang)
        ++c;
      if (map[niGetInterfaceUUID(ni::iUnknown)] == hspIUnknown)
        ++c;
      if (map[niGetInterfaceUUID(ni::iDispatch)] == hspIDispatch)
        ++c;
    }
    CHECK_EQUAL(4*numTests,c);
  } TEST_TIMING_END();
}
