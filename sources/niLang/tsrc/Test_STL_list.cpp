#include "stdafx.h"
#define TEST_DOPRINT
#include "../src/API/niLang/STL/list.h"
#include "../src/API/niLang/STL/vector.h"
#include "../src/API/niLang/STL/map.h"

namespace {
// template class astl::list<ni::tInt>;

typedef astl::list<ni::tInt> tIntLst;
typedef astl::list<ni::tU8>  tU8Lst;
typedef astl::list<ni::tU16> tU16Lst;
typedef astl::list<ni::tU32> tU32Lst;

static ni::tU32 _numCons = 0;
static ni::tU32 _numDes = 0;
struct sTestItem {
  ni::tU32 v;
  sTestItem(ni::tU32 x = 0) {
    v = x;
    ++_numCons;
  }
  sTestItem(const sTestItem& x) { // copy constructor
    v = x.v;
    ++_numCons;
  }
  ~sTestItem() {
    ++_numDes;
  }
};
typedef astl::list<sTestItem> tTestLst;

struct ASTL_list {
};

TEST_FIXTURE(ASTL_list,alloc) {
  _numCons = _numDes = 0;
  tTestLst lst;
  lst.push_back(1);
  lst.push_back(2);
  lst.push_back(3);
  CHECK_EQUAL(1,lst.front().v);
  CHECK_EQUAL(3,lst.back().v);
  CHECK_EQUAL(3,lst.size());
  lst.clear();
  CHECK_EQUAL(_numCons,_numDes);
}

TEST_FIXTURE(ASTL_list,IntLst_pushback) {
  tIntLst vec;
  TEST_PRINT(niFmt(_A("LIST SIZE: %d\n"),sizeof(vec)));
  vec.push_back(1);
  vec.push_back(2);
  vec.push_back(3);
  CHECK_EQUAL(1,vec.front());
  CHECK_EQUAL(3,vec.back());
  CHECK_EQUAL(3,vec.size());
}

TEST_FIXTURE(ASTL_list,U8Lst_pushback) {
  tU8Lst vec;
  vec.push_back(1);
  vec.push_back(2);
  vec.push_back(3);
  CHECK_EQUAL(1,vec.front());
  CHECK_EQUAL(3,vec.back());
  CHECK_EQUAL(3,vec.size());
}

TEST_FIXTURE(ASTL_list,U16Lst_pushback) {
  tU16Lst vec;
  vec.push_back(1);
  vec.push_back(2);
  vec.push_back(3);
  CHECK_EQUAL(1,vec.front());
  CHECK_EQUAL(3,vec.back());
  CHECK_EQUAL(3,vec.size());
}

TEST_FIXTURE(ASTL_list,U32Lst_pushback) {
  tU32Lst vec;
  vec.push_back(1);
  vec.push_back(2);
  vec.push_back(3);
  CHECK_EQUAL(1,vec.front());
  CHECK_EQUAL(3,vec.back());
  CHECK_EQUAL(3,vec.size());
}

} // end of anonymous namespace
