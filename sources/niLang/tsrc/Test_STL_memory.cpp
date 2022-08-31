#include "stdafx.h"
#include "../src/API/niLang/STL/memory.h"

namespace {

using namespace ni;

struct ASTL_memory {
};

struct sMyData {
  tInt _foo = 111;
};

TEST_FIXTURE(ASTL_memory, shared_ptr) {
  {
    astl::shared_ptr<sMyData> a = astl::make_shared<sMyData>();
    CHECK_EQUAL(111, a->_foo);
  }

  {
    astl::shared_ptr<sMyData> a = astl::make_shared<sMyData>(sMyData { 123 });
    CHECK_EQUAL(123, a->_foo);
    CHECK_EQUAL(1, a.use_count());

    {
      astl::shared_ptr<sMyData> b = a;
      CHECK_EQUAL(2, a.use_count());
    }
    CHECK_EQUAL(1, a.use_count());
  }
}

TEST_FIXTURE(ASTL_memory, weak_ptr) {
  astl::weak_ptr<sMyData> w;
  CHECK_EQUAL(true, w.expired());

  {
    astl::shared_ptr<sMyData> a = astl::make_shared<sMyData>();
    CHECK_EQUAL(111, a->_foo);
    CHECK_EQUAL(1, a.use_count());

    w = a;
    CHECK_EQUAL(1, a.use_count());
    CHECK_EQUAL(false, w.expired());
    CHECK_EQUAL(a.get(), w.lock().get());

    astl::weak_ptr<sMyData> wa = a;
    astl::shared_ptr<sMyData> la = wa.lock();
    CHECK_EQUAL(2, a.use_count());
    CHECK_EQUAL(false, wa.expired());
    CHECK_EQUAL(a.get(), la.get());
  }

  {
    CHECK_EQUAL(true, w.expired());
    astl::shared_ptr<sMyData> z = w.lock();
    CHECK_EQUAL((sMyData*)nullptr, z.get());
  }
}

// a function consuming a unique_ptr can take it by value or by rvalue reference
astl::unique_ptr<sMyData> _test_pass_through(astl::unique_ptr<sMyData> p) {
    p->_foo = 888;
    return p;
}

TEST_FIXTURE(ASTL_memory, unique_ptr) {
  astl::unique_ptr<sMyData> a = astl::make_unique<sMyData>();
  CHECK_EQUAL(111, a->_foo);
  CHECK_EQUAL(true, (!!a));

  astl::unique_ptr<sMyData> movedA = _test_pass_through(astl::move(a));
  CHECK_EQUAL(888, movedA->_foo);
  CHECK_EQUAL(true, (!!movedA));

  CHECK_EQUAL(false, (!!a));
}

} // end of anonymous namespace
