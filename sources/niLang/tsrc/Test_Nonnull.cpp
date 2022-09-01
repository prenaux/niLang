#define niNoUnsafePtr
void Test_Nonnull_Trace(const char* msg);
#define TRACE_NI_NONNULL(X) Test_Nonnull_Trace("ni::Nonnull " X);

#include "stdafx.h"
#include <niLang/StringBase.h>
#include <niLang/Utils/Nonnull.h>
#include <niLang/STL/set.h>
#include <niLang/STL/hash_set.h>

using namespace ni;

/*

   Pierre: Why the anonymous namespace?

   I just spent 2hrs debugging this:
   ```
   Test_Nonnull.cpp
     struct sTestItem : public cIUnknownImpl<iUnknown> {

   Test_STL_list.cpp
     struct sTestItem {
   ```

   It leads to random crashes only in the Linux debug builds (da).

   For some incomprensible reason the Linux linker picks one implementation
   randomly if you have more than one with the same name in different object
   files. As opposed to anonymize the symbols automatically or failing the
   compilation. Also this only happens in the debug build, with any
   optimisation flag this works as it does on every other platform.

   Adding an anonymous namespace in all the tests that declare a local struct
   fixes the issue. That's probably the best solution.

*/
void Test_Nonnull_Trace(const char* msg) {
  niLog(Debug,msg);
}

namespace {

struct FNonnull {};

struct sAddRefReleaseCounter {
  ni::tU32 _numAddRef = 0;
  ni::tU32 _numRelease = 0;
};

struct sTestItem : public cIUnknownImpl<iUnknown> {
  sTestItem(const achar* aName, sAddRefReleaseCounter* apAddRefReleaseCounter = NULL)
      : _name(aName)
      , _addRefReleaseCounter(apAddRefReleaseCounter)
  {
    // niDebugFmt(("... sTestItem"));
  }
  ~sTestItem() {
    // niDebugFmt(("... ~sTestItem"));
  }

  virtual tI32 __stdcall AddRef() {
    if (_addRefReleaseCounter) {
      ++_addRefReleaseCounter->_numAddRef;
    }
    NI_UNKIMPL_ADDREF();
  }
  virtual tI32 __stdcall Release() {
    if (_addRefReleaseCounter) {
      ++_addRefReleaseCounter->_numRelease;
    }
    NI_UNKIMPL_RELEASE();
  }

  ni::cString _name;
  sAddRefReleaseCounter* _addRefReleaseCounter;
};

ni::cString TestNonnull(astl::non_null<sTestItem*> v) {
  return v->_name;
}
ni::cString TestConstNonnull(astl::non_null<const sTestItem*> v) {
  return v->_name;
}

TEST_FIXTURE(FNonnull,non_null) {
  int value = 123;
  astl::non_null<int*> v { &value };
  CHECK_EQUAL(value, *v);
}

TEST_FIXTURE(FNonnull,non_null_hash_set) {
  int a = 123;
  int b = 456;

  astl::hash_set<astl::non_null<const int*> > hashedSet;
  hashedSet.insert(astl::make_non_null(&a));
  hashedSet.insert(astl::make_non_null(&b));
  CHECK(astl::contains(hashedSet, astl::make_non_null(&a)));
  CHECK(astl::contains(hashedSet, astl::make_non_null(&b)));
}

TEST_FIXTURE(FNonnull,base) {
  Nonnull<sTestItem> itemA = ni::NewNonnull<sTestItem>("fooItem");
  CHECK_NOT_EQUAL(nullptr, (sTestItem*&)itemA);
  CHECK_EQUAL(1, itemA->GetNumRefs());
  CHECK_EQUAL(_ASTR("fooItem"), itemA->_name);
  CHECK_EQUAL(_ASTR("fooItem"), TestNonnull(itemA));
  CHECK_EQUAL(_ASTR("fooItem"), TestNonnull(itemA.non_null()));
  CHECK_EQUAL(_ASTR("fooItem"), TestConstNonnull(itemA));
  CHECK_EQUAL(_ASTR("fooItem"), TestConstNonnull(itemA.c_non_null()));
}

TEST_FIXTURE(FNonnull,RefCount) {
  sAddRefReleaseCounter counterA;
  sAddRefReleaseCounter counterB;
  sAddRefReleaseCounter counterC;

  {
    Ptr<sTestItem> itemA = ni::NewPtr<sTestItem>("itemA", &counterA);
    CHECK_EQUAL(1, itemA.raw_ptr()->GetNumRefs());
    Ptr<sTestItem> itemB = ni::NewPtr<sTestItem>("itemB", &counterB);
    CHECK_EQUAL(1, itemB.raw_ptr()->GetNumRefs());
    Nonnull<sTestItem> nnC = ni::NewNonnull<sTestItem>("itemC", &counterC);
    CHECK_EQUAL(1, nnC->GetNumRefs());
    {
      Nonnull<sTestItem> nnA = ni::MakeNonnull(itemA.raw_ptr());
      CHECK_NOT_EQUAL(nullptr, (sTestItem*&)nnA);
      CHECK_EQUAL(2, itemA.raw_ptr()->GetNumRefs());
      CHECK_EQUAL(1, itemB.raw_ptr()->GetNumRefs());
      CHECK_EQUAL(1, nnC->GetNumRefs());

      nnA = itemB.non_null();
      CHECK_NOT_EQUAL(nullptr, (sTestItem*&)nnA);
      CHECK_EQUAL(1, itemA.raw_ptr()->GetNumRefs());
      CHECK_EQUAL(2, itemB.raw_ptr()->GetNumRefs());
      CHECK_EQUAL(1, nnC->GetNumRefs());

      nnA = nnC;
      CHECK_EQUAL(1, itemA.raw_ptr()->GetNumRefs());
      CHECK_EQUAL(1, itemB.raw_ptr()->GetNumRefs());
      CHECK_EQUAL(2, nnC->GetNumRefs());
    }
    CHECK_EQUAL(1, itemA.raw_ptr()->GetNumRefs());
    CHECK_EQUAL(1, itemB.raw_ptr()->GetNumRefs());
    CHECK_EQUAL(1, nnC->GetNumRefs());
  }

  niDebugFmt(("... itemA numAddRef: %d, numRelease: %d",
              counterA._numAddRef, counterA._numRelease));
  niDebugFmt(("... itemB numAddRef: %d, numRelease: %d",
              counterB._numAddRef, counterB._numRelease));
  niDebugFmt(("... itemC numAddRef: %d, numRelease: %d",
              counterC._numAddRef, counterC._numRelease));
  CHECK_EQUAL(2, counterA._numAddRef);
  CHECK_EQUAL(2, counterA._numRelease);
  CHECK_EQUAL(2, counterB._numAddRef);
  CHECK_EQUAL(2, counterB._numRelease);
  CHECK_EQUAL(2, counterC._numAddRef);
  CHECK_EQUAL(2, counterC._numRelease);
}

TEST_FIXTURE(FNonnull,move) {
  Nonnull<sTestItem> itemA = ni::NewNonnull<sTestItem>("fooItem");
  CHECK_NOT_EQUAL(nullptr, (sTestItem*&)itemA);

  Nonnull<sTestItem> itemA2 = astl::move(itemA);
  CHECK_NOT_EQUAL(nullptr, (sTestItem*&)itemA2);
  CHECK_EQUAL(nullptr, (sTestItem*&)itemA);

  CHECK_EQUAL(1, itemA2->GetNumRefs());

  CHECK_EQUAL(_ASTR("fooItem"), itemA2->_name);
}

TEST_FIXTURE(FNonnull,maybe) {
  {
    Ptr<sTestItem> itemA = ni::NewNonnull<sTestItem>("fooItem");
    CHECK(itemA.has_value());
    if (itemA.has_value()) {
      ni::Nonnull<sTestItem> v = itemA.non_null();
      CHECK_EQUAL(2, v->GetNumRefs());
      CHECK_EQUAL(_ASTR("fooItem"), v->_name);
    }
    if (itemA.has_value()) {
      astl::non_null<sTestItem*> v = itemA.non_null().non_null();
      CHECK_EQUAL(1, v->GetNumRefs());
      CHECK_EQUAL(_ASTR("fooItem"), v->_name);
    }
    CHECK_EQUAL(1, itemA.non_null()->GetNumRefs());
  }

  {
    Ptr<sTestItem> itemA = (sTestItem*)NULL;
    CHECK(!itemA.has_value());
  }

  {
    Ptr<sTestItem> itemA;
    CHECK(!itemA.has_value());
  }
}

TEST_FIXTURE(FNonnull,hash_set) {
  Nonnull<sTestItem> itemA = ni::NewNonnull<sTestItem>("itemA");
  Nonnull<sTestItem> itemB = ni::NewNonnull<sTestItem>("itemB");

  astl::hash_set<ni::Nonnull<sTestItem> > hashedSet;
  hashedSet.insert(itemA);
  hashedSet.insert(itemB);
  CHECK(astl::contains(hashedSet, itemA));
  CHECK(astl::contains(hashedSet, itemB));
}

} // end of anonymous namespace
