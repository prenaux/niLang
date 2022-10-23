#include "stdafx.h"
#include <niLang/STL/expected.h>
#include <niLang/STL/memory.h>

using namespace ni;

struct ASTL_expected {};

struct sError {
  cString _msg;
  astl::shared_ptr<sError> _prev;
};

typedef astl::expected<cString,sError> tExpectedString;

static astl::expected<cString,sError> doSucceed() {
  return "OK";
}

static astl::expected<cString,sError> doFail() {
  return astl::make_unexpected(sError { "Failed" });
}

TEST_FIXTURE(ASTL_expected,base) {
  sError err = {"an error 1"};
  sError err2 = {"an error 2", astl::make_shared<sError>(err)};

  {
    tExpectedString r;
    CHECK_EQUAL(false,!!r);
    CHECK_EQUAL(_ASTR("DefaultValue"), r.value_or("DefaultValue"));
  }

  {
    auto r = doSucceed();
    CHECK_EQUAL(true,!!r);
    CHECK_EQUAL(_ASTR("OK"), r.value());
    CHECK_EQUAL(_ASTR("OK"), r.value_or("DefaultValue"));

    // and_then/or_else return astl::expected
    bool andThen = false;
    bool orElse = false;
    r = r.and_then([&andThen](auto&& v) -> decltype(r) {
      andThen = true;
      return v;
    }).or_else([&orElse](auto&& err) {
      orElse = true;
    });
    CHECK_EQUAL(true,andThen);
    CHECK_EQUAL(false,orElse);

    // map/map_error return astl::expected<decltype<map return value>,
    // decltype<map_error return value>>
    auto r2 = r.map([](auto&& v) {
      return v + v;
    }).map_error([](auto&& e) {
      return sError { e._msg + e._msg };
    });
    CHECK_EQUAL(_ASTR("OKOK"), r2.value());

    // a way to chain calls and return some default value if there was an
    // error
    const cString vvv = r.map([](auto&& v) {
      return v + ";OK1";
    }).map([](auto&& v) {
      return v + ";OK2";
    }).value_or("ERRDEFAULT");
    CHECK_EQUAL(_ASTR("OK;OK1;OK2"), vvv);
  }

  {
    auto r = doFail();
    CHECK_EQUAL(true,!r);
    CHECK_EQUAL(_ASTR("Failed"), r.error()._msg);
    CHECK_EQUAL(_ASTR("DefaultValue"), r.value_or("DefaultValue"));

    // and_then/or_else return a astl::expected
    bool andThen = false;
    bool orElse = false;
    r = r.and_then([&andThen](auto&& v) -> decltype(r) {
      andThen = true;
      return v;
    }).or_else([&orElse](auto&& err) {
      orElse = true;
    });
    CHECK_EQUAL(false,andThen);
    CHECK_EQUAL(true,orElse);

    // map/map_error return astl::expected<decltype<map return value>,
    // decltype<map_error return value>>
    auto r2 = r.map([](auto&& v) {
      return v + v;
    }).map_error([](auto&& e) {
      return sError { e._msg + e._msg };
    });
    CHECK_EQUAL(_ASTR("FailedFailed"), r2.error()._msg);

    // a way to chain calls and return some default value if there was an
    // error
    const cString vvv = r.map([](auto&& v) {
      return v + ";OK1";
    }).map([](auto&& v) {
      return v + ";OK2";
    }).value_or("ERRDEFAULT");
    CHECK_EQUAL(_ASTR("ERRDEFAULT"), vvv);
  }
}
