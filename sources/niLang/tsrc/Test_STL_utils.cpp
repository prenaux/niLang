#include "stdafx.h"
#include <niLang/STL/utils.h>
#include <niLang/STL/type_traits.h>
#include <niLang/STL/array.h>
#include <niLang/STL/sort.h>

using namespace ni;

struct ASTL_utils {};

static auto GetVector() {
  return astl::vector<tInt>({ 1, 2, 3 });
}

static const auto GetConstVector() {
  return astl::vector<tInt>({ 1, 2, 3 });
}

static auto GetMap() {
  return astl::map<tInt,cString>({ {1,"a"}, {2,"b"}, {3,"c"} });
}

TEST_FIXTURE(ASTL_utils, dangling_find_vector) {
  {
    astl::vector<tInt> v = GetVector();
    auto it = astl::find(v, 1);
    static_assert(astl::is_same<decltype(it), astl::vector<tInt>::iterator>::value, "Unexpected type");
    CHECK_EQUAL(1, *it);
  }

  {
    auto it = astl::find(GetVector(), 1);
    static_assert(astl::is_same<decltype(it), astl::dangling_iterator>::value, "Unexpected type");
    // CHECK_EQUAL(1, *it); // Doesnt compile because dangling_iterator is empty
  }

  {
    const astl::vector<tInt> v = GetConstVector();
    auto it = astl::find(v, 1);
    static_assert(astl::is_same<decltype(it), astl::vector<tInt>::const_iterator>::value, "Unexpected type");
    CHECK_EQUAL(1, *it);
  }

  {
    auto it = astl::find(GetConstVector(), 1);
    static_assert(astl::is_same<decltype(it), astl::dangling_iterator>::value, "Unexpected type");
    // CHECK_EQUAL(1, *it); // Doesnt compile because dangling_iterator is empty
  }
}

TEST_FIXTURE(ASTL_utils, dangling_upsert_map) {
  {
    astl::map<tInt,cString> m = GetMap();
    auto it = astl::upsert(m, 123, "123");
    static_assert(astl::is_same<decltype(it), astl::map<tInt,cString>::iterator>::value, "Unexpected type");
    CHECK_EQUAL(123, it->first);
    CHECK_EQUAL(_ASTR("123"), it->second);
  }

  /* Doest no compile, as it should since upsert() requires a lvalue
  {
    auto it = astl::upsert(GetMap(), 123, "123");
    static_assert(astl::is_same<decltype(it), astl::map<tInt,cString>::iterator>::value, "Unexpected type");
    CHECK_EQUAL(123, it->first);
    CHECK_EQUAL(_ASTR("123"), it->second);
  }
  */
}

TEST_FIXTURE(ASTL_utils, emplace_back) {
  static int _numConstructed = 0;
  static int _numValInit = 0;
  static int _numDefaulted = 0;
  static int _numCopied = 0;
  static int _numMoved = 0;

  struct MyFagiano {

    cString _name;
    int _age;

    MyFagiano() {
      ++_numConstructed;
      ++_numDefaulted;
    }

    MyFagiano(const cString& name, int age)
        : _name(name)
        , _age(age)
    {
      ++_numConstructed;
      ++_numValInit;
    }

    MyFagiano(const MyFagiano& aOther)
        : _name(aOther._name)
        , _age(aOther._age)
    {
      ++_numConstructed;
      ++_numCopied;
    }

    MyFagiano(MyFagiano&& aOther)
        : _name(aOther._name)
        , _age(aOther._age)
    {
      ++_numConstructed;
      ++_numMoved;
    }

    static void ResetCounts() {
      _numConstructed = 0;
      _numValInit = 0;
      _numDefaulted = 0;
      _numCopied = 0;
      _numMoved = 0;
    }

    static sVec4i Counts() {
      return Vec4i(_numValInit, _numDefaulted, _numCopied, _numMoved);
    }
  };

  {
    astl::vector<MyFagiano> fagiani;
    MyFagiano::ResetCounts();
    MyFagiano& alpo = astl::emplace_back(fagiani, "Roger", 54);
    CHECK_EQUAL(54, alpo._age);
    CHECK_EQUAL(_ASTR("Roger"), alpo._name);
    CHECK_EQUAL(1, _numConstructed);
    CHECK_EQUAL(Vec4i(1,0,0,0), MyFagiano::Counts());
  }

  {
    astl::vector<MyFagiano> fagiani;
    MyFagiano::ResetCounts();
    MyFagiano& alpo = astl::emplace_back(fagiani);
    alpo._name = "Roger";
    alpo._age = 54;
    CHECK_EQUAL(54, alpo._age);
    CHECK_EQUAL(_ASTR("Roger"), alpo._name);
    CHECK_EQUAL(1, _numConstructed);
    CHECK_EQUAL(Vec4i(0,1,0,0), MyFagiano::Counts());
  }

  {
    astl::vector<MyFagiano> fagiani;
    MyFagiano::ResetCounts();
    MyFagiano& alpo = astl::push_back(fagiani);
    alpo._name = "Roger";
    alpo._age = 54;
    CHECK_EQUAL(54, alpo._age);
    CHECK_EQUAL(_ASTR("Roger"), alpo._name);
    CHECK_EQUAL(1, _numConstructed);
    CHECK_EQUAL(Vec4i(0,1,0,0), MyFagiano::Counts());
  }

  {
    astl::vector<MyFagiano> fagiani;
    MyFagiano::ResetCounts();
    MyFagiano& alpo = astl::push_back(fagiani, MyFagiano("Roger", 54));
    CHECK_EQUAL(54, alpo._age);
    CHECK_EQUAL(_ASTR("Roger"), alpo._name);
    CHECK_EQUAL(2, _numConstructed);
    CHECK_EQUAL(Vec4i(1,0,1,0), MyFagiano::Counts());
  }

  {
    astl::vector<MyFagiano> fagiani;
    MyFagiano::ResetCounts();
    MyFagiano& alpo = astl::emplace_back(fagiani, MyFagiano("Roger", 54));
    CHECK_EQUAL(54, alpo._age);
    CHECK_EQUAL(_ASTR("Roger"), alpo._name);
    CHECK_EQUAL(2, _numConstructed);
#ifdef EASTL_VERSION
    // EASTL does a (better) move
    CHECK_EQUAL(Vec4i(1,0,0,1), MyFagiano::Counts());
#else
    // Standard STL does a copy instead of a move
    CHECK_EQUAL(Vec4i(1,0,1,0), MyFagiano::Counts());
#endif
  }

  {
    astl::vector<MyFagiano> fagiani;
    MyFagiano::ResetCounts();
    const MyFagiano roger = MyFagiano("Roger", 54);
    MyFagiano& alpo = astl::push_back(fagiani, roger);
    alpo._name = "alpo";
    CHECK_EQUAL(54, alpo._age);
    CHECK_EQUAL(_ASTR("alpo"), alpo._name);
    CHECK_EQUAL(2, _numConstructed);
    CHECK_EQUAL(Vec4i(1,0,1,0), MyFagiano::Counts());
  }

  {
    astl::vector<MyFagiano> fagiani;
    MyFagiano::ResetCounts();
    const MyFagiano roger = MyFagiano("Roger", 54);
    MyFagiano& alpo = astl::emplace_back(fagiani, roger);
    alpo._name = "alpo";
    CHECK_EQUAL(54, alpo._age);
    CHECK_EQUAL(_ASTR("alpo"), alpo._name);
    CHECK_EQUAL(2, _numConstructed);
    CHECK_EQUAL(Vec4i(1,0,1,0), MyFagiano::Counts());
  }
}

TEST_FIXTURE(ASTL_utils, sort) {
  astl::array<int, 10> s = {5, 7, 4, 2, 8, 6, 1, 9, 0, 3};
  CHECK_NOT_EQUAL(0, s[0]);

  astl::sort(s.begin(),s.end());
  niLoop(i,10) {
    CHECK_EQUAL(i, s[i]);
  }
}

TEST_FIXTURE(ASTL_utils, stable_sort) {
  astl::array<int, 10> s = {5, 7, 4, 2, 8, 6, 1, 9, 0, 3};
  CHECK_NOT_EQUAL(0, s[0]);

  astl::stable_sort(s.begin(),s.end());
  niLoop(i,10) {
    CHECK_EQUAL(i, s[i]);
  }
}

TEST_FIXTURE(ASTL_utils, find) {
  astl::array<int, 10> s = {5, 7, 4, 2, 8, 6, 1, 9, 0, 3};
  CHECK_NOT_EQUAL(0, s[0]);

  astl::array<int, 10>::iterator it = astl::find(s.begin(),s.end(),9);
  CHECK_NOT_EQUAL(it, s.end());
}

TEST_FIXTURE(ASTL_utils, reverse) {
  astl::array<int, 10> s = {5, 7, 4, 2, 8, 6, 1, 9, 0, 3};
  CHECK_NOT_EQUAL(0, s[0]);
  astl::stable_sort(s.begin(),s.end());

  astl::reverse(s.begin(),s.end());
  niLoop(i,10) {
    CHECK_EQUAL(i, s[9-i]);
  }
}

TEST_FIXTURE(ASTL_utils, swap) {
  int a = 10;
  int b = 20;
  CHECK_EQUAL(10, a);
  CHECK_EQUAL(20, b);
  ni::Swap(a,b);
  CHECK_EQUAL(10, b);
  CHECK_EQUAL(20, a);
  astl::swap(a,b);
  CHECK_EQUAL(10, a);
  CHECK_EQUAL(20, b);
}

TEST_FIXTURE(ASTL_utils, unique) {
  astl::vector<int> v{1,2,1,1,3,3,3,4,5,4};

  auto last = astl::unique(v.begin(), v.end());
  // v now holds {1 2 1 3 4 5 4 x x x}, where 'x' is indeterminate
  v.erase(last, v.end());

  // sort followed by unique, to remove all duplicates
  astl::sort(v.begin(), v.end()); // {1 1 2 3 4 4 5}

  last = astl::unique(v.begin(), v.end());
  // v now holds {1 2 3 4 5 x x}, where 'x' is indeterminate
  v.erase(last, v.end());

  niLoop(i,5) {
    CHECK_EQUAL(i+1, v[i]);
  }
}
