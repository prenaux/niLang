#include "stdafx.h"
#include <niLang/STL/function_ref.h>

using namespace ni;

struct ASTL_function_ref {};

static int foo(astl::function_ref<int(int)> func) {
    return func(21); // 42
}

TEST_FIXTURE(ASTL_function_ref,base) {
  CHECK_EQUAL(42, foo([](int i) { return i*2; }));
}
