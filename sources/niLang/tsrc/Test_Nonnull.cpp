#include "stdafx.h"
#include <niLang/Utils/Nonnull.h>
#include <niLang/STL/set.h>
#include <niLang/STL/hash_set.h>

using namespace ni;

struct FNonnull {};

struct sTestItem : public cIUnknownImpl<iUnknown> {
  sTestItem(const achar* aName)
      : _name(aName)
  {}
  ni::cString _name;
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
  CHECK_EQUAL(_ASTR("fooItem"), TestNonnull(itemA.ptr()));
  CHECK_EQUAL(_ASTR("fooItem"), TestConstNonnull(itemA));
  CHECK_EQUAL(_ASTR("fooItem"), TestConstNonnull(itemA.c_ptr()));
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
  MaybePtr<sTestItem> itemA = ni::NewNonnull<sTestItem>("fooItem");
  CHECK(itemA.has_value());
  itemA.map([&](auto&& v) {
    CHECK_EQUAL(1, v->GetNumRefs());
    CHECK_EQUAL(_ASTR("fooItem"), v->_name);
  });
  CHECK_EQUAL(1, itemA.value()->GetNumRefs());
}

TEST_FIXTURE(FNonnull,maybe_ptr) {
  {
    MaybePtr<sTestItem> itemA = ni::NewMaybePtr<sTestItem>("fooItem");
    CHECK(itemA.has_value());
    CHECK_NOT_EQUAL(nullptr, (sTestItem*&)itemA.value());
    itemA.map([&](auto&& v) {
      CHECK_EQUAL(1, v->GetNumRefs());
      CHECK_EQUAL(_ASTR("fooItem"), v->_name);
    });
  }

  {
    MaybePtr<sTestItem> itemA = ni::MakeMaybePtr((sTestItem*)NULL);
    CHECK(!itemA.has_value());
  }

  {
    MaybePtr<sTestItem> itemA;
    CHECK(!itemA.has_value());
  }
}

TEST_FIXTURE(FNonnull,value_or_fn) {
  MaybePtr<sTestItem> itemA = ni::MakeMaybePtr((sTestItem*)NULL);
  CHECK(!itemA.has_value());
  Nonnull<sTestItem> r = itemA.value_or_fn([]() {
    return ni::NewNonnull<sTestItem>("defaultItem");
  });
  CHECK_EQUAL(_ASTR("defaultItem"), r->_name);
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
