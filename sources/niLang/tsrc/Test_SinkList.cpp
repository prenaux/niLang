#include "stdafx.h"
#include "../src/API/niLang/Utils/SinkList.h"

using namespace ni;

struct iMySink : public iUnknown
{
	niDeclareInterfaceUUID(iMySink,0x1b8aef94,0xcb93,0x45d9,0x8d,0x69,0xfe,0xba,0xeb,0xcb,0x35,0x75);
	virtual tBool __stdcall OnMySink_Count(tU32 c) = 0;
};

typedef SinkList<iMySink> tMySinkList;

struct MySinkImpl : public cIUnknownImpl<iMySink>
{
  const achar* _name;
	tU32 _counter;
	MySinkImpl(const achar* aName = "NA") {
    _name = aName;
		_counter = 0;
	}
	tBool __stdcall OnMySink_Count(tU32 c) {
		_counter += c;
		return eTrue;
	}
};

struct FSinkList {
};

TEST_FIXTURE(FSinkList,Add) {
	Ptr<MySinkImpl> a = niNew MySinkImpl();
	Ptr<MySinkImpl> b = niNew MySinkImpl();
	Ptr<MySinkImpl> c = niNew MySinkImpl();
	Ptr<tMySinkList> sinkLst = tMySinkList::Create();
  Ptr<iCollection> emptyCollection = sinkLst->_ImmutableCollection().ptr();

	sinkLst->AddSink(a);
	CHECK_EQUAL(1UL,sinkLst->size());
  Ptr<iCollection> aCollection = sinkLst->_ImmutableCollection().ptr();
  CHECK_EQUAL(aCollection.ptr(),sinkLst->_ImmutableCollection().ptr());

	sinkLst->AddSink(b);
	CHECK_EQUAL(2UL,sinkLst->size());
  Ptr<iCollection> bCollection = sinkLst->_ImmutableCollection().ptr();
  CHECK_EQUAL(bCollection.ptr(),sinkLst->_ImmutableCollection().ptr());

	sinkLst->AddSink(c);
	CHECK_EQUAL(3UL,sinkLst->size());
  Ptr<iCollection> cCollection = sinkLst->_ImmutableCollection().ptr();
  CHECK_EQUAL(cCollection.ptr(),sinkLst->_ImmutableCollection().ptr());

  CHECK(emptyCollection != aCollection);
  CHECK(aCollection != bCollection);
  CHECK(bCollection != cCollection);
	CHECK_EQUAL(1UL,aCollection->GetSize());
	CHECK_EQUAL(2UL,bCollection->GetSize());
	CHECK_EQUAL(3UL,cCollection->GetSize());
}


TEST_FIXTURE(FSinkList,Clear) {
	Ptr<MySinkImpl> a = niNew MySinkImpl("a");
	Ptr<MySinkImpl> b = niNew MySinkImpl("b");
	Ptr<MySinkImpl> c = niNew MySinkImpl("c");
	Ptr<tMySinkList> sinkLst = tMySinkList::Create();
	sinkLst->AddSink(a);
	sinkLst->AddSink(b);
	sinkLst->AddSink(c);
	CHECK_EQUAL(3UL,sinkLst->size());
	sinkLst->Clear();
	CHECK_EQUAL(0UL,sinkLst->size());
	CHECK(sinkLst->IsEmpty());
}

TEST_FIXTURE(FSinkList,Remove) {
	Ptr<MySinkImpl> a = niNew MySinkImpl("a");
  CHECK_EQUAL(1,a->mprotected_nNumRefs);
	Ptr<MySinkImpl> b = niNew MySinkImpl("b");
  CHECK_EQUAL(1,b->mprotected_nNumRefs);
	Ptr<MySinkImpl> c = niNew MySinkImpl("c");
  CHECK_EQUAL(1,c->mprotected_nNumRefs);

	Ptr<tMySinkList> sinkLst = tMySinkList::Create();

  auto getListMask2 = [&](tMySinkList* apLst) -> sVec4i {
    sVec4i has = {0,0,0,0};
    niLoopit(tMySinkList::tImmutableCollection::const_iterator,it,*apLst->_ImmutableCollection()) {
      ++has.w;
      if (*it == a) {
        niAssert(has.x == 0);
        has.x = 1;
      }
      else if (*it == b) {
        niAssert(has.y == 0);
        has.y = 1;
      }
      else if (*it == c) {
        niAssert(has.z == 0);
        has.z = 1;
      }
    }
    return has;
  };

  auto getListMask = [&](tMySinkList* apLst) -> sVec4i {
    sVec4i has = {0,0,0,0};
    niLoopImmutableIterator(tMySinkList,it,apLst) {
      ++has.w;
      if (it->_Value() == a) {
        niAssert(has.x == 0);
        has.x = 1;
      }
      else if (it->_Value() == b) {
        niAssert(has.y == 0);
        has.y = 1;
      }
      else if (it->_Value() == c) {
        niAssert(has.z == 0);
        has.z = 1;
      }
    }
    return has;
  };

	sinkLst->AddSink(a);
  CHECK_EQUAL(2,a->mprotected_nNumRefs);
  CHECK_EQUAL(Vec4i(1,0,0,1), getListMask2(sinkLst));
  CHECK_EQUAL(Vec4i(1,0,0,1), getListMask(sinkLst));

	sinkLst->AddSink(b);
  CHECK_EQUAL(2,b->mprotected_nNumRefs);
  CHECK_EQUAL(Vec4i(1,1,0,2), getListMask2(sinkLst));
  CHECK_EQUAL(Vec4i(1,1,0,2), getListMask(sinkLst));

	sinkLst->AddSink(c);
  CHECK_EQUAL(2,c->mprotected_nNumRefs);
  CHECK_EQUAL(Vec4i(1,1,1,3), getListMask2(sinkLst));
  CHECK_EQUAL(Vec4i(1,1,1,3), getListMask(sinkLst));

	CHECK_EQUAL(3UL,sinkLst->size());

	sinkLst->RemoveSink(b);
  CHECK_EQUAL(1,b->mprotected_nNumRefs);
	CHECK_EQUAL(2UL,sinkLst->size());
  CHECK_EQUAL(Vec4i(1,0,1,2), getListMask(sinkLst));

	sinkLst->RemoveSink(c);
  CHECK_EQUAL(1,c->mprotected_nNumRefs);
	CHECK_EQUAL(1UL,sinkLst->size());
  CHECK_EQUAL(Vec4i(1,0,0,1), getListMask(sinkLst));

	sinkLst->RemoveSink(a);
  CHECK_EQUAL(1,a->mprotected_nNumRefs);
  CHECK_EQUAL(Vec4i(0,0,0,0), getListMask(sinkLst));

	CHECK_EQUAL(0UL,sinkLst->size());

	CHECK(sinkLst->IsEmpty());
}


TEST_FIXTURE(FSinkList,Remove2) {
	Ptr<MySinkImpl> a = niNew MySinkImpl();
  CHECK_EQUAL(1,a->mprotected_nNumRefs);
	Ptr<tMySinkList> sinkLst = tMySinkList::Create();
	sinkLst->AddSink(a);
  CHECK_EQUAL(2,a->mprotected_nNumRefs);
  {
    Ptr<iMySink> ptrSink = a.ptr();
    {
      Ptr<tMySinkList::sIterator> it = (tMySinkList::sIterator*)sinkLst->Find(a.ptr());
      niAssert(it.IsOK());
      it = NULL;
      sinkLst->RemoveSink(a.ptr());
    }
  }
  CHECK_EQUAL(1,a->mprotected_nNumRefs);
	CHECK_EQUAL(0UL,sinkLst->size());
	CHECK(sinkLst->IsEmpty());
}

TEST_FIXTURE(FSinkList,Call) {
	Ptr<MySinkImpl> a = niNew MySinkImpl();
	Ptr<MySinkImpl> b = niNew MySinkImpl();
	Ptr<MySinkImpl> c = niNew MySinkImpl();
	Ptr<tMySinkList> sinkLst = tMySinkList::Create();
	sinkLst->AddSink(a);
	sinkLst->AddSink(b);
	sinkLst->AddSink(c);
	CHECK_EQUAL(3UL,sinkLst->size());
	niCallSinkVoid_(MySink,sinkLst,Count,(1));
	niCallSinkVoid_(MySink,sinkLst,Count,(1));
	niCallSinkVoid_(MySink,sinkLst,Count,(1));
	CHECK_EQUAL(3UL,a->_counter);
	CHECK_EQUAL(3UL,b->_counter);
	CHECK_EQUAL(3UL,c->_counter);
}
