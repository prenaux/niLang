#include "stdafx.h"
#include <niLang/STL/span.h>
#include <niLang/STL/utils.h>

using namespace ni;

struct ASTL_span {};

template <typename T, tSize N> EA_NODISCARD
constexpr auto slide(astl::span<T,N> s, tSize offset, tSize width) {
  return s.subspan(offset, offset + width <= s.size() ? width : 0U);
}

template <typename T, tSize N> EA_NODISCARD
ni::cString ToString(const astl::span<T,N>& seq)
{
  ni::cString r;
  for (const auto& elem : seq)
    r << elem << " ";
  if (!r.empty()) {
    // remove at the back
    r.Remove();
  }
  return r;
}

TEST_FIXTURE(ASTL_span,c_array) {
  constexpr int a[] { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
  constexpr auto b = astl::array<int,3> { 8, 7, 6 };
  static_assert(b[1] == 7);

  astl::vector<ni::cString> r;
  for (tSize offset{}; ; ++offset) {
    static constexpr tSize width{6};
    auto s = slide(astl::make_span(a), offset, width);
    if (s.empty())
      break;
    r.push_back(ToString(s));
    niPrintln(niFmt("... r[%d]: %s", r.size()-1, r.back()));
  }

  CHECK_EQUAL(_ASTR("0 1 2 3 4 5"), r[0]);
  CHECK_EQUAL(_ASTR("1 2 3 4 5 6"), r[1]);
  CHECK_EQUAL(_ASTR("2 3 4 5 6 7"), r[2]);
  CHECK_EQUAL(_ASTR("3 4 5 6 7 8"), r[3]);

  CHECK(!astl::contains(astl::make_span(a), astl::make_span(b)));

  {
    auto sub1 = astl::make_span(a + 1, 4);
    niPrintln(niFmt("... sub1: %s", ToString(sub1)));
    CHECK_EQUAL(_ASTR("1 2 3 4"), ToString(sub1));
    CHECK_EQUAL(4, sub1.size());
    CHECK(astl::contains(astl::make_span(a), sub1));
    CHECK(!astl::starts_with(astl::make_span(a), sub1));
  }

  {
    auto subbeg = astl::make_span(a, 3);
    niPrintln(niFmt("... subbeg: %s", ToString(subbeg)));
    CHECK_EQUAL(_ASTR("0 1 2"), ToString(subbeg));
    CHECK_EQUAL(3, subbeg.size());
    CHECK(astl::contains(astl::make_span(a), subbeg));
    CHECK(astl::starts_with(astl::make_span(a), subbeg));
    CHECK(!astl::ends_with(astl::make_span(a), subbeg));
  }

  {
    auto subend = astl::make_span(a + niCountOf(a) - 2, 2);
    niPrintln(niFmt("... subend: %s", ToString(subend)));
    CHECK_EQUAL(_ASTR("7 8"), ToString(subend));
    CHECK_EQUAL(2, subend.size());
    CHECK(astl::contains(astl::make_span(a), subend));
    CHECK(!astl::starts_with(astl::make_span(a), subend));
    CHECK(astl::ends_with(astl::make_span(a), subend));
  }
}

TEST_FIXTURE(ASTL_span,astl_array) {
  astl::array<int,9> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
  astl::array<int,3> b = { 8, 7, 6 };

  astl::vector<ni::cString> r;
  for (tSize offset{}; ; ++offset) {
    static constexpr tSize width{6};
    auto s = slide(astl::make_span(a), offset, width);
    if (s.empty())
      break;
    r.push_back(ToString(s));
    niPrintln(niFmt("... r[%d]: %s", r.size()-1, r.back()));
  }

  CHECK_EQUAL(_ASTR("0 1 2 3 4 5"), r[0]);
  CHECK_EQUAL(_ASTR("1 2 3 4 5 6"), r[1]);
  CHECK_EQUAL(_ASTR("2 3 4 5 6 7"), r[2]);
  CHECK_EQUAL(_ASTR("3 4 5 6 7 8"), r[3]);

  CHECK(!astl::contains(astl::make_span(a), astl::make_span(b)));

  {
    auto sub1 = astl::make_span(a.begin() + 1, 4);
    niPrintln(niFmt("... sub1: %s", ToString(sub1)));
    CHECK_EQUAL(_ASTR("1 2 3 4"), ToString(sub1));
    CHECK_EQUAL(4, sub1.size());
    CHECK(astl::contains(astl::make_span(a), sub1));
    CHECK(!astl::starts_with(astl::make_span(a), sub1));
  }

  {
    auto subbeg = astl::make_span(a.begin(), 3);
    niPrintln(niFmt("... subbeg: %s", ToString(subbeg)));
    CHECK_EQUAL(_ASTR("0 1 2"), ToString(subbeg));
    CHECK_EQUAL(3, subbeg.size());
    CHECK(astl::contains(astl::make_span(a), subbeg));
    CHECK(astl::starts_with(astl::make_span(a), subbeg));
    CHECK(!astl::ends_with(astl::make_span(a), subbeg));
  }

  {
    auto subend = astl::make_span(a.begin() + a.size() - 2, 2);
    niPrintln(niFmt("... subend: %s", ToString(subend)));
    CHECK_EQUAL(_ASTR("7 8"), ToString(subend));
    CHECK_EQUAL(2, subend.size());
    CHECK(astl::contains(astl::make_span(a), subend));
    CHECK(!astl::starts_with(astl::make_span(a), subend));
    CHECK(astl::ends_with(astl::make_span(a), subend));
  }
}

TEST_FIXTURE(ASTL_span,vector) {
  astl::vector<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
  astl::vector<int> b = { 8, 7, 6 };

  astl::vector<ni::cString> r;
  for (tSize offset{}; ; ++offset) {
    static constexpr tSize width{6};
    auto s = slide(astl::make_span(a), offset, width);
    if (s.empty())
      break;
    r.push_back(ToString(s));
    niPrintln(niFmt("... r[%d]: %s", r.size()-1, r.back()));
  }

  CHECK_EQUAL(_ASTR("0 1 2 3 4 5"), r[0]);
  CHECK_EQUAL(_ASTR("1 2 3 4 5 6"), r[1]);
  CHECK_EQUAL(_ASTR("2 3 4 5 6 7"), r[2]);
  CHECK_EQUAL(_ASTR("3 4 5 6 7 8"), r[3]);

  CHECK(!astl::contains(astl::make_span(a), astl::make_span(b)));

  {
    auto sub1 = astl::make_span(a.begin() + 1, 4);
    niPrintln(niFmt("... sub1: %s", ToString(sub1)));
    CHECK_EQUAL(_ASTR("1 2 3 4"), ToString(sub1));
    CHECK_EQUAL(4, sub1.size());
    CHECK(astl::contains(astl::make_span(a), sub1));
    CHECK(!astl::starts_with(astl::make_span(a), sub1));
  }

  {
    auto subbeg = astl::make_span(a.begin(), 3);
    niPrintln(niFmt("... subbeg: %s", ToString(subbeg)));
    CHECK_EQUAL(_ASTR("0 1 2"), ToString(subbeg));
    CHECK_EQUAL(3, subbeg.size());
    CHECK(astl::contains(astl::make_span(a), subbeg));
    CHECK(astl::starts_with(astl::make_span(a), subbeg));
    CHECK(!astl::ends_with(astl::make_span(a), subbeg));
  }

  {
    auto subend = astl::make_span(a.begin() + a.size() - 2, 2);
    niPrintln(niFmt("... subend: %s", ToString(subend)));
    CHECK_EQUAL(_ASTR("7 8"), ToString(subend));
    CHECK_EQUAL(2, subend.size());
    CHECK(astl::contains(astl::make_span(a), subend));
    CHECK(!astl::starts_with(astl::make_span(a), subend));
    CHECK(astl::ends_with(astl::make_span(a), subend));
  }
}

TEST_FIXTURE(ASTL_span,subspan) {
  constexpr int data[] { 0, 1, 2, 3, 4, 5, 6 };
  auto span = astl::span<const int> { data, 4 };

  ni::cString r;
  for (auto offset{0U}; offset != span.size(); ++offset) {
    r << span.subspan(offset).front() << " ";
  }
  r.Remove();

  CHECK_EQUAL(_ASTR("0 1 2 3"), r);
}
