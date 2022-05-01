#include "stdafx.h"
#include "../src/API/niLang/Utils/Random.h"
#include "../src/API/niLang/Utils/IndexedMap.h"

using namespace ni;

namespace ni {
template class IndexedMap<cString,tU32>;
template class IndexedMap<tHStringPtr,tU32>;
}

#ifdef _DEBUG
const ni::tU32 kNumTests = 1000;
#else
const ni::tU32 kNumTests = 10000;
#endif

struct FIndexedMap
{
  astl::vector<cString>     _strings;
  astl::vector<tHStringPtr> _hstrings;

  FIndexedMap()
  {
    _strings.reserve(kNumTests);
    _hstrings.reserve(kNumTests);
    niLoop(i,kNumTests) {
      _strings.push_back(niFmt("%d",i));
      _hstrings.push_back(_H(_strings.back()));
    }
  }

  template<typename MAP>
  bool test_str_insert(MAP& map) {
    niLoop(i,kNumTests) {
      map[_strings[i].Chars()] = i;
    }
    return true;
  }
  template<typename MAP>
  bool test_str_lookup(MAP& map) {
    ni::RandSeed(123);
    niLoop(i,kNumTests) {
      ni::tU32 k = ni::RandIntRange(0,(ni::tI32)kNumTests-1);
      if (map[_strings[k].Chars()] != k)
        return false;
    }
    return true;
  }

};

TEST_FIXTURE(FIndexedMap,MapIndexStringSeq) {
  TEST_TRACK_MEMORY_BEGIN();
  IndexedMap<cString,tU32> imap;
  TEST_TIMING_BEGIN(Insert) {
    niLoop(i,kNumTests) {
      tU32 r = imap.Set(_strings[i],i);
      CHECK_EQUAL(i,r);
      if (r != i)
        break;
    }
    CHECK_EQUAL(kNumTests,imap.GetSize());
    TEST_TIMING_END() }
  TEST_TIMING_BEGIN(Lookup) {
    bool err = false;
    ni::RandSeed(123);
    niLoop(i,kNumTests) {
      ni::tU32 k = ni::RandIntRange(0,(ni::tI32)kNumTests-1);
      if (imap[_strings[k]] != k) {
        err = true;
        break;
      }
    }
    CHECK(!err);
    TEST_TIMING_END() }
  CHECK_EQUAL(10,imap[_A("10")]);
  CHECK_EQUAL(20,imap[_A("20")]);
  CHECK_EQUAL(30,imap[_A("30")]);
  TEST_TIMING_BEGIN(IndexLookup) {
    bool err = false;
    ni::RandSeed(123);
    niLoop(i,kNumTests) {
      ni::tU32 k = ni::RandIntRange(0,(ni::tI32)kNumTests-1);
      if (imap[k] != k) {
        err = true;
        break;
      }
    }
    CHECK(!err);
    TEST_TIMING_END() }
  TEST_TIMING_BEGIN(Remove) {
    niLoopr(ri,kNumTests) {
      imap.EraseIndex(ri);
    }
    CHECK_EQUAL(0,imap.GetSize());
    TEST_TIMING_END() }
  TEST_TIMING_BEGIN(InsertAfterRemove) {
    niLoop(i,kNumTests) {
      imap.Set(_strings[i],i);
    }
    CHECK_EQUAL(kNumTests,imap.GetSize());
    TEST_TIMING_END() }
  CHECK_EQUAL(10,imap[_A("10")]);
  CHECK_EQUAL(20,imap[_A("20")]);
  CHECK_EQUAL(30,imap[_A("30")]);
  TEST_TRACK_MEMORY_END();
}

TEST_FIXTURE(FIndexedMap,MapIndexHStringSeq) {
  TEST_TRACK_MEMORY_BEGIN();
  IndexedMap<tHStringPtr,tU32> imap;
  TEST_TIMING_BEGIN(Insert) {
    niLoop(i,kNumTests) {
      tU32 r = imap.Set(_hstrings[i],i);
      CHECK_EQUAL(i,r);
      if (r != i)
        break;
    }
    CHECK_EQUAL(kNumTests,imap.GetSize());
    TEST_TIMING_END() }
  TEST_TIMING_BEGIN(Lookup) {
    bool err = false;
    ni::RandSeed(123);
    niLoop(i,kNumTests) {
      ni::tU32 k = ni::RandIntRange(0,(ni::tI32)kNumTests-1);
      if (imap[_hstrings[k]] != k) {
        err = true;
        break;
      }
    }
    CHECK(!err);
    TEST_TIMING_END() }
  CHECK_EQUAL(10,imap[_H("10")]);
  CHECK_EQUAL(20,imap[_H("20")]);
  CHECK_EQUAL(30,imap[_H("30")]);
  TEST_TIMING_BEGIN(IndexLookup) {
    bool err = false;
    ni::RandSeed(123);
    niLoop(i,kNumTests) {
      ni::tU32 k = ni::RandIntRange(0,(ni::tI32)kNumTests-1);
      if (imap[k] != k) {
        err = true;
        break;
      }
    }
    CHECK(!err);
    TEST_TIMING_END() }
  TEST_TIMING_BEGIN(Remove) {
    niLoopr(ri,kNumTests) {
      imap.EraseIndex(ri);
    }
    CHECK_EQUAL(0,imap.GetSize());
    TEST_TIMING_END() }
  TEST_TIMING_BEGIN(InsertAfterRemove) {
    niLoop(i,kNumTests) {
      imap.Set(_hstrings[i],i);
    }
    CHECK_EQUAL(kNumTests,imap.GetSize());
    TEST_TIMING_END() }
  CHECK_EQUAL(10,imap[_H("10")]);
  CHECK_EQUAL(20,imap[_H("20")]);
  CHECK_EQUAL(30,imap[_H("30")]);
  TEST_TRACK_MEMORY_END();
}
