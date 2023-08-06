#include "stdafx.h"
#include <niLang/STL/string_view.h>

struct ASTL_string_view {
};

TEST_FIXTURE(ASTL_string_view, length) {
  astl::string_view strView("Hello");
  CHECK_EQUAL(5, strView.length());
}

TEST_FIXTURE(ASTL_string_view, empty) {
  astl::string_view strView;
  CHECK(strView.empty());
}

TEST_FIXTURE(ASTL_string_view, compare) {
  astl::string_view strView1("Test");
  astl::string_view strView2("Test");
  CHECK_EQUAL(0, strView1.compare(strView2));
}

TEST_FIXTURE(ASTL_string_view, at) {
  astl::string_view strView("Hello");
  CHECK_EQUAL('H', strView.at(0));
}
