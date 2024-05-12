#define niNoUnsafePtr

#if _DEBUG
void Test_Nonnull_Trace(const char* msg);
#define TRACE_NI_NONNULL(X) Test_Nonnull_Trace("ni::Nonnull " X);
#define TRACE_ASTL_NON_NULL(X) Test_Nonnull_Trace("astl::non_null " X);
#endif

#include "stdafx.h"
#include <niLang/StringBase.h>
#include <niLang/Utils/Nonnull.h>
#include <niLang/STL/set.h>
#include <niLang/STL/hash_set.h>
#include <niLang/STL/memory.h>
#include <niLang/Utils/CrashReport.h>

// test compile time errors
#define TEST_CTERR 0

using namespace ni;

/*

   Pierre: Why the anonymous namespace?

   I just spent 2hrs debugging this:
   ```
   Test_Nonnull.cpp
     struct sTestItem : public ImplRC<iUnknown> {

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
void _DumpStack(const achar* aName) {
  if (aName) {
    niLog(Debug,aName);
  }
  cString r;
  ni_stack_get_current(r,NULL,0);
  niLog(Warning,r);
}
#define DUMP_STACK(NAME) //_DumpStack(NAME)

void Test_Nonnull_Trace(const char* msg) {
  niLog(Debug,msg);
  DUMP_STACK(NULL);
}

namespace {

struct FNonnull {};

struct sAddRefReleaseCounter {
  ni::tU32 _numAddRef = 0;
  ni::tU32 _numRelease = 0;
};

struct sTestItem : public ImplRC<iUnknown> {
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
    DUMP_STACK("AddRef");
    NI_UNKIMPL_ADDREF();
  }
  virtual tI32 __stdcall Release() {
    if (_addRefReleaseCounter) {
      ++_addRefReleaseCounter->_numRelease;
    }
    DUMP_STACK("Release");
    NI_UNKIMPL_RELEASE();
  }

  ni::cString _name;
  sAddRefReleaseCounter* _addRefReleaseCounter;
};

QPtr<sTestItem> CreateTestItem(const achar* aName, sAddRefReleaseCounter* aCounter) {
  niCheck(niStringIsOK(aName),NULL);
  return ni::MakePtr<sTestItem>(aName,aCounter);
}

ni::cString TestNonnull(astl::non_null<sTestItem*> v) {
  return v->_name;
}
ni::cString TestNonnullConst(astl::non_null<sTestItem const*> v) {
  return v->_name;
}

TEST_FIXTURE(FNonnull,non_null) {
  int value = 123;
  astl::non_null<int*> v { &value };
  CHECK_EQUAL(value, *v);
  *v = 456;
  CHECK_EQUAL(456, *v);
}

TEST_FIXTURE(FNonnull,const_cast_non_null) {
  int value = 123;
  astl::non_null<const int*> v { &value };
  CHECK_EQUAL(value, *v);
  *astl::const_cast_non_null<int*>(v) = 456;
  CHECK_EQUAL(456, *v);
}

TEST_FIXTURE(FNonnull,non_null_hash_set) {
  int a = 123;
  int b = 456;

  astl::hash_set<astl::non_null<const int*> > hashedSet;
  hashedSet.insert(astl::as_non_null(&a));
  hashedSet.insert(astl::as_non_null(&b));
  CHECK(astl::contains(hashedSet, astl::as_non_null(&a)));
  CHECK(astl::contains(hashedSet, astl::as_non_null(&b)));
}

TEST_FIXTURE(FNonnull,shared_non_null) {
  astl::shared_non_null<int> u = astl::make_shared_non_null<int>(123);
  CHECK_EQUAL(123, *u);
  astl::shared_non_null<int> v = astl::make_shared_non_null<int>(123);
  CHECK_EQUAL(123, *v);

  *v = 456;
  CHECK_EQUAL(456, *v);

  // new shared pointer
#if TEST_CTERR
  // strict non_null, so doesnt compile
  v = astl::make_shared<int>(789);
  CHECK_EQUAL(789, *v);
#endif

  // new nonnull pointer
  v = astl::make_shared_non_null<int>(888);
  CHECK_EQUAL(888, *v);

  // copy pointer
  v = u;
  CHECK(!!u.raw_ptr());
  CHECK_EQUAL(123, *v);

  // move pointer
  v = astl::move(u);
  CHECK(!u.raw_ptr());
  CHECK_EQUAL(123, *v);
}

TEST_FIXTURE(FNonnull,unique_non_null) {
  astl::unique_non_null<int> u = astl::make_unique_non_null<int>(123);
  CHECK_EQUAL(123, *u);
  astl::unique_non_null<int> v = astl::make_unique_non_null<int>(123);
  CHECK_EQUAL(123, *v);

  *v = 456;
  CHECK_EQUAL(456, *v);

  // new unique pointer
#if TEST_CTERR
  // strict non_null, so doesnt compile
  v = astl::make_unique<int>(789);
  CHECK_EQUAL(789, *v);
#endif

  v = astl::make_unique_non_null<int>(888);
  CHECK_EQUAL(888, *v);

  // copy pointer
#if TEST_CTERR
  // doesnt compile since we cant copy assign a unique_ptr
  v = u;
  CHECK(!!u.raw_ptr());
  CHECK_EQUAL(123, *v);
#endif

  // move pointer
  v = astl::move(u);
  CHECK(!u.raw_ptr());
  CHECK_EQUAL(123, *v);
}

TEST_FIXTURE(FNonnull,base) {
  Nonnull<sTestItem> itemA = ni::MakeNonnull<sTestItem>("fooItem");
  CHECK_NOT_EQUAL(nullptr, (sTestItem*&)itemA);
  CHECK_EQUAL(1, itemA->GetNumRefs());
  CHECK_EQUAL(_ASTR("fooItem"), itemA->_name);
  CHECK_EQUAL(_ASTR("fooItem"), TestNonnull(itemA));
  CHECK_EQUAL(_ASTR("fooItem"), TestNonnull(itemA.non_null()));
  CHECK_EQUAL(_ASTR("fooItem"), TestNonnullConst(itemA));
  CHECK_EQUAL(_ASTR("fooItem"), TestNonnullConst(itemA.non_null_const()));
#if TEST_CTERR
  Nonnull<sTestItem> itemB = ni::MakePtr<sTestItem>("fooItem");
  itemA = ni::MakePtr<sTestItem>("fooItem");
#endif
}

TEST_FIXTURE(FNonnull,const_cast_Nonnull) {
  Nonnull<const sTestItem> itemA = ni::MakeNonnull<sTestItem>("fooItem");
  CHECK_NOT_EQUAL(nullptr, (sTestItem*&)itemA);
  CHECK_EQUAL(_ASTR("fooItem"), itemA->_name);
  astl::const_cast_non_null<sTestItem*>(itemA)->_name = "constChanged";
  CHECK_EQUAL(_ASTR("constChanged"), itemA->_name);
}

TEST_FIXTURE(FNonnull,RefCount) {
  sAddRefReleaseCounter counterA;
  sAddRefReleaseCounter counterB;
  sAddRefReleaseCounter counterC;

  {
    Ptr<sTestItem> itemA = ni::MakePtr<sTestItem>("itemA", &counterA);
    CHECK_EQUAL(1, itemA.raw_ptr()->GetNumRefs());
    Ptr<sTestItem> itemB = ni::MakePtr<sTestItem>("itemB", &counterB);
    CHECK_EQUAL(1, itemB.raw_ptr()->GetNumRefs());
    Nonnull<sTestItem> nnC = ni::MakeNonnull<sTestItem>("itemC", &counterC);
    CHECK_EQUAL(1, nnC->GetNumRefs());
    {
      Nonnull<sTestItem> nnA = ni::Nonnull<sTestItem>{itemA.raw_ptr()};
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
  Nonnull<sTestItem> itemA = ni::MakeNonnull<sTestItem>("fooItem");
  CHECK_NOT_EQUAL(nullptr, (sTestItem*&)itemA);

  Nonnull<sTestItem> itemA2 = astl::move(itemA);
  CHECK_NOT_EQUAL(nullptr, (sTestItem*&)itemA2);
  CHECK_EQUAL(nullptr, (sTestItem*&)itemA);

  CHECK_EQUAL(1, itemA2->GetNumRefs());

  CHECK_EQUAL(_ASTR("fooItem"), itemA2->_name);
}

TEST_FIXTURE(FNonnull,maybe) {
  {
    Ptr<sTestItem> itemA = ni::MakeNonnull<sTestItem>("fooItem");
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
  Nonnull<sTestItem> itemA = ni::MakeNonnull<sTestItem>("itemA");
  Nonnull<sTestItem> itemB = ni::MakeNonnull<sTestItem>("itemB");

  astl::hash_set<ni::Nonnull<sTestItem> > hashedSet;
  hashedSet.insert(itemA);
  hashedSet.insert(itemB);
  CHECK(astl::contains(hashedSet, itemA));
  CHECK(astl::contains(hashedSet, itemB));
}

ni::cString Test_niCheckNonnull_AddRefReleaseCounter(
  sAddRefReleaseCounter& counter,
  const tI32 expectedAddRef,
  const tI32 expectedRelease)
{
  cString err;
  if (counter._numRelease != expectedRelease) {
    err.CatFormat("[sTestItem_InvalidNumRelease expected %d = got %d]",
                  expectedRelease, counter._numRelease);
  }
  if (counter._numAddRef != expectedAddRef) {
    err.CatFormat("[sTestItem_InvalidNumAddRef expected %d = got %d]",
                  expectedAddRef, counter._numAddRef);
  }
  return err.IsEmpty() ? _ASTR("OK") : err;
}

template <typename T>
ni::cString Test_niCheckNonnull(Nonnull<T>& r, const achar* aName) {
  if (r.raw_ptr() == nullptr)
    return _ASTR("UnexpectedNull");
  if (!r->_name.Eq(aName))
    return _ASTR(niFmt("sTestItem_WrongName:%s = %s",aName,r->_name));
  if (r.raw_ptr()) {
    if (r->GetNumRefs() != 1)
      return _ASTR(niFmt("sTestItem_InvalidRefCount:%d", r->GetNumRefs()));
    if (!r->_addRefReleaseCounter) {
      return _ASTR("sTestItem_NoAddRefReleaseCounter");
    }
  }
  return _ASTR("HasValue");
}

ni::cString Test_niCheckNonnull_RawNullPtr(sAddRefReleaseCounter* aCounter) {
  niUnused(aCounter);
  Nonnull<sTestItem> r = niCheckNonnull(r,(sTestItem*)nullptr,_ASTR("IsNull"));
  return Test_niCheckNonnull(r,NULL);
}

ni::cString Test_niCheckNonnull_Create(const achar* aName,
                                       sAddRefReleaseCounter* aCounter) {
  Nonnull<sTestItem> r = niCheckNonnull(r,CreateTestItem(aName,aCounter),_ASTR("IsNull"));
  return Test_niCheckNonnull(r,aName);
}

ni::cString Test_niCheckNonnull_FromPtr(const achar* aName,
                                        sAddRefReleaseCounter* aCounter) {
  Nonnull<sTestItem> r = CreateTestItem("dummy",NULL).non_null();
  {
    QPtr<sTestItem> p = CreateTestItem(aName,aCounter);
    r = niCheckNonnull(r,p,_ASTR("IsNull"));
  }
  return Test_niCheckNonnull(r,aName);
}

ni::cString Test_niCheckNonnull_CreateWeakPtr(
  const achar* aName,sAddRefReleaseCounter* aCounter) {
  Nonnull<sTestItem> r = niCheckNonnull(r,WeakPtr<sTestItem>(CreateTestItem(aName,aCounter)),_ASTR("IsNull"));
  return Test_niCheckNonnull(r,aName);
}

ni::cString Test_niCheckNonnull_FromWeakPtr(
  const achar* aName, sAddRefReleaseCounter* aCounter) {
  Nonnull<sTestItem> r = CreateTestItem("dummy",NULL).non_null();
  {
    QPtr<sTestItem> p = CreateTestItem(aName,aCounter);
    if (p.has_value()) {
      if (p.raw_ptr()->GetNumRefs() != 1)
        return _ASTR("InvalidNumRef_p_1");
    }
    WeakPtr<sTestItem> w = p;
    if (p.has_value()) {
      if (p.raw_ptr()->GetNumRefs() != 1)
        return _ASTR("InvalidNumRef_w_1");
    }
    r = niCheckNonnull(r,w,_ASTR("IsNull"));
  }
  return Test_niCheckNonnull(r,aName);
}

//
// Run with:
// $ ham pass1 && ham Run_Test_niLang BUILD=da FIXTURE="FNonnull,niCheckNonnull*"
//
TEST_FIXTURE(FNonnull,niCheckNonnull_MacroVariants) {
  // this test only test that the macros compile correctly
  Nonnull<sTestItem> a = niCheckNonnull(a,ni::MakePtr<sTestItem>("a"),;);
  niUnused(a);
  Nonnull<sTestItem> b = niCheckNonnullMsg(b,ni::MakePtr<sTestItem>("b"),niFmt("Can't create b."),;);
  niUnused(b);
  Nonnull<sTestItem> c = niCheckNonnullSilent(c,ni::MakePtr<sTestItem>("c"),;);
  niUnused(c);
  Nonnull<sTestItem> d = AsNonnull(ni::MakePtr<sTestItem>("d"));
}

TEST_FIXTURE(FNonnull,niCheckNonnull_HasValue) {
  // Should be HasValue
  {
    sAddRefReleaseCounter createCounter;
    CHECK_EQUAL(_ASTR("HasValue"), Test_niCheckNonnull_Create("Foobar", &createCounter));
    CHECK_EQUAL(_ASTR("OK"), Test_niCheckNonnull_AddRefReleaseCounter(createCounter,1,1));
  }
  {
    sAddRefReleaseCounter fromPtrCounter;
    CHECK_EQUAL(_ASTR("HasValue"), Test_niCheckNonnull_FromPtr("Foobar",&fromPtrCounter));
    CHECK_EQUAL(_ASTR("OK"), Test_niCheckNonnull_AddRefReleaseCounter(fromPtrCounter,2,2));
  }
}

TEST_FIXTURE(FNonnull,niCheckNonnull_IsNull) {
  // Should be IsNull
  {
    sAddRefReleaseCounter nullCreateCounter;
    CHECK_EQUAL(_ASTR("IsNull"), Test_niCheckNonnull_Create(AZEROSTR,&nullCreateCounter));
    CHECK_EQUAL(_ASTR("OK"), Test_niCheckNonnull_AddRefReleaseCounter(nullCreateCounter,0,0));
  }
  {
    sAddRefReleaseCounter nullFromPtrCounter;
    CHECK_EQUAL(_ASTR("IsNull"), Test_niCheckNonnull_FromPtr(AZEROSTR,&nullFromPtrCounter));
    CHECK_EQUAL(_ASTR("OK"), Test_niCheckNonnull_AddRefReleaseCounter(nullFromPtrCounter,0,0));
  }
  {
    sAddRefReleaseCounter rawNullPtr;
    CHECK_EQUAL(_ASTR("IsNull"), Test_niCheckNonnull_RawNullPtr(&rawNullPtr));
    CHECK_EQUAL(_ASTR("OK"), Test_niCheckNonnull_AddRefReleaseCounter(rawNullPtr,0,0));
  }
}

TEST_FIXTURE(FNonnull,niCheckNonnull_WeakPtr_HasValue) {
  {
    sAddRefReleaseCounter createCounter;
    CHECK_EQUAL(_ASTR("HasValue"), Test_niCheckNonnull_CreateWeakPtr(
      "Foobar", &createCounter));
    CHECK_EQUAL(_ASTR("OK"), Test_niCheckNonnull_AddRefReleaseCounter(createCounter,2,2));
  }
  {
    sAddRefReleaseCounter fromPtrCounter;
    CHECK_EQUAL(_ASTR("HasValue"), Test_niCheckNonnull_FromWeakPtr("Foobar",&fromPtrCounter));
    CHECK_EQUAL(_ASTR("OK"), Test_niCheckNonnull_AddRefReleaseCounter(fromPtrCounter,2,2));
  }
}

TEST_FIXTURE(FNonnull,niCheckNonnull_WeakPtr_IsNull) {
  {
    sAddRefReleaseCounter createCounter;
    CHECK_EQUAL(_ASTR("IsNull"), Test_niCheckNonnull_CreateWeakPtr(
      NULL, &createCounter));
    CHECK_EQUAL(_ASTR("OK"), Test_niCheckNonnull_AddRefReleaseCounter(createCounter,0,0));
  }
  {
    sAddRefReleaseCounter fromPtrCounter;
    CHECK_EQUAL(_ASTR("IsNull"), Test_niCheckNonnull_FromWeakPtr(NULL,&fromPtrCounter));
    CHECK_EQUAL(_ASTR("OK"), Test_niCheckNonnull_AddRefReleaseCounter(fromPtrCounter,0,0));
  }
}

astl::shared_ptr<cString> CreateSharedString(const achar* aName) {
  if (!niStringIsOK(aName))
    return nullptr;
  return astl::make_shared<cString>(_ASTR(aName));
}

ni::cString Test_niCheckNonnull_shared_ptr(const achar* aName) {
  astl::shared_non_null<cString> r = niCheckNonnull(r,CreateSharedString(aName),_ASTR("is_null"));
  return _ASTR(niFmt("is_string:%s",*r));
}

ni::cString Test_niCheckNonnull_weak_ptr(const achar* aName) {
  astl::shared_ptr<cString> mystr = CreateSharedString(aName);
  astl::weak_ptr<cString> weakstr = mystr;
  astl::shared_non_null<cString> r = niCheckNonnull(r,weakstr.lock(),_ASTR("is_null"));
  return _ASTR(niFmt("is_string:%s",*r));
}

TEST_FIXTURE(FNonnull,niCheckNonnull_shared_ptr) {
  CHECK_EQUAL(_ASTR("is_string:Foo"), Test_niCheckNonnull_shared_ptr("Foo"));
  CHECK_EQUAL(_ASTR("is_null"), Test_niCheckNonnull_shared_ptr(NULL));
}

TEST_FIXTURE(FNonnull,niCheckNonnull_weak_ptr) {
  CHECK_EQUAL(_ASTR("is_string:Foo"), Test_niCheckNonnull_weak_ptr("Foo"));
  CHECK_EQUAL(_ASTR("is_null"), Test_niCheckNonnull_weak_ptr(NULL));
}

#if 0
TEST_FIXTURE(FNonnull,niCheckNonnull_IncorrectUse) {
  Nonnull<sTestItem> r = niCheckNonnull(r,CreateTestItem("r",NULL),;);
  {
    // Should have a compiler error, using r by mistake
    Nonnull<sTestItem> rInvalid = niCheckNonnull(r,CreateTestItem("rInvalid",NULL),;);
  }
  // Should have a compiler error, using r by mistake
  Nonnull<sTestItem> rInvalid2 = niCheckNonnull(r,CreateTestItem("rInvalid",NULL),;);
}
#endif

#if 0
TEST_FIXTURE(FNonnull,CompilerFailure) {
  // Should have a compiler error, Ptr<> is not Nonnull<>
  Ptr<sTestItem> r = niCheckNonnull(r,(sTestItem*)nullptr,_ASTR("IsNull"));
  // Should have a compiler error, WeakPtr<> is not Nonnull<>
  WeakPtr<sTestItem> r = niCheckNonnull(r,(sTestItem*)nullptr,_ASTR("IsNull"));
}
#endif

} // end of anonymous namespace
