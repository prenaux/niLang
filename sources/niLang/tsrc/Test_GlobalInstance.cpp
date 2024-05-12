#include "stdafx.h"
#include "../src/API/niLang/STL/utils.h"
#include "../src/API/niLang/ILang.h"

using namespace ni;

struct FGlobalInstance
{
};

struct Dummy : public ImplRC<iUnknown> {
  cString _name;
  Dummy(const achar* aName) : _name(aName) {}
};

TEST_FIXTURE(FGlobalInstance,Range) {
  Ptr<tGlobalInstanceCMap> map = tGlobalInstanceCMap::Create();
  astl::upsert(*map, _ASTR("Dummy1"), niNew Dummy("1"));
  astl::upsert(*map, _ASTR("Alpha"), niNew Dummy("A"));
  astl::upsert(*map, _ASTR("Dummy.alpha"), niNew Dummy("alpha"));
  astl::upsert(*map, _ASTR("Dummy.bmp"), niNew Dummy("bmp"));
  astl::upsert(*map, _ASTR("Dummy.dds"), niNew Dummy("dds"));
  astl::upsert(*map, _ASTR("Gamma"), niNew Dummy("G"));
  astl::upsert(*map, _ASTR("Dummy2"), niNew Dummy("1"));
  astl::upsert(*map, _ASTR("Dummy.tga"), niNew Dummy("tga"));
  astl::upsert(*map, _ASTR("Dummy.jpeg"), niNew Dummy("jpeg"));
  astl::upsert(*map, _ASTR("Delta"), niNew Dummy("D"));
  astl::upsert(*map, _ASTR("Dummy.jpg"), niNew Dummy("jpg"));
  astl::upsert(*map, _ASTR("Dummy3"), niNew Dummy("1"));
  astl::upsert(*map, _ASTR("Dummy.hdr"), niNew Dummy("hdr"));

  int found = 0;
  for (tGlobalInstanceCMap::const_iterator it = map->lower_bound("Dummy.");
       it != map->end() && it->first.StartsWith("Dummy."); ++it)
  {
    niDebugFmt(("... %s = %s", it->first, ((Dummy*)it->second.ptr())->_name));
    ++found;
  }
  CHECK_EQUAL(7,found);
}

TEST_FIXTURE(FGlobalInstance,PutGet) {
  Ptr<iUnknown> i1 = niNew Dummy("1");
  Ptr<iUnknown> i2 = niNew Dummy("2");
  Ptr<tGlobalInstanceCMap> map = tGlobalInstanceCMap::Create();
  map->Put("i", i1);
  CHECK_EQUAL((tIntPtr)i1.ptr(), (tIntPtr)map->Get("i").GetIUnknownPointer());
  map->Put("i", i2);
  CHECK_EQUAL((tIntPtr)i2.ptr(), (tIntPtr)map->Get("i").GetIUnknownPointer());
}
