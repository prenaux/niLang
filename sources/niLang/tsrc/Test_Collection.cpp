#define niNoUnsafePtr
#include "stdafx.h"
#include "../src/API/niLang/Utils/CollectionImpl.h"

#if !defined niPtr_NoUnsafeAPI
#error "Expected niPtr_NoUnsafeAPI."
#endif

template struct ni::cMutableCollectionImpl<
  ni::CollectionTraitsMap<ni::CollectionTraitsI32,ni::CollectionTraitsI32> >;
template struct ni::tInterfaceCVec<ni::iUnknown>;

struct FCollection {
};

TEST_FIXTURE(FCollection,MutableVector) {

  ni::Nonnull<ni::iMutableCollection> vec{CreateCollectionVector(ni::eType_U32)};
  CHECK(vec->IsEmpty());
  CHECK(vec->GetSize() == 0);

  vec->Add(1);
  vec->Add(2);
  vec->Add(3);
  vec->Add(4);
  vec->Add(5);
  vec->Add(6);
  // test casting to vector implementation type
  ni::tU32CVec* nativeVec = (ni::tU32CVec*)vec.raw_ptr();
  nativeVec->push_back(7);
  nativeVec->push_back(8);

  CHECK_EQUAL(8,vec->GetSize());

  {
    ni::Ptr<ni::iIterator> itNotFound = vec->Find(1234);
    CHECK(!itNotFound.has_value());
    CHECK(itNotFound.is_null());
  }

  ni::Ptr<ni::iIterator> itFive = vec->Find(5);
  CHECK(itFive.has_value());
  CHECK_EQUAL(5,itFive.non_null()->Value().GetIntValue());

  const ni::tU32 seqResult[7] = {1,2,3,5,6,8,78};
  ni::tU32 seq[7] = {0};
  ni::tU32* seqPtr = seq;

  CHECK(vec->GetDataPtr() != NULL);
  CHECK(vec->GetDataSize() == 8*4);
  CHECK(memcmp(vec->GetDataPtr(),seqResult,3*4) == 0);

  ni::tBool first = ni::eTrue;
  niLoopIterator(it,vec) {
    const ni::tU32 v = it->Value().GetIntValue();
    if (first) {
      CHECK(vec->Remove(4)); // remove 4
      CHECK(!it->IsOK());
      CHECK(vec->Remove(7)); // remove 7
      CHECK(vec->Add(78));
      CHECK(vec->Remove(1));
      CHECK_EQUAL(6,vec->GetSize());
      // itFive has been invalidated because we modified the collection
      // while iterating
      CHECK(itFive.non_null()->Value().IsNull());
      first = ni::eFalse;
    }
    niDebugFmt(("R-V: %d",v));
    *seqPtr++ = v;
  }

  niLoopIterator(it,vec) {
    const ni::tU32 v = it->Value().GetIntValue();
    niDebugFmt(("V: %d",v));
    *seqPtr++ = v;
  }

  CHECK_EQUAL(6,vec->GetSize());
  niLoop(i,niCountOf(seq)) {
    CHECK_EQUAL(seqResult[i],seq[i]);
  }

  CHECK_EQUAL(2,vec->GetFirst().GetIntValue());
  CHECK_EQUAL(78,vec->GetLast().GetIntValue());

  vec->RemoveFirst();
  CHECK_EQUAL(5,vec->GetSize());
  CHECK_EQUAL(3,vec->GetFirst().GetIntValue());

  vec->RemoveLast();
  CHECK_EQUAL(4,vec->GetSize());
  CHECK_EQUAL(8,vec->GetLast().GetIntValue());

  vec->SetFirst(123);
  vec->SetLast(321);
  CHECK_EQUAL(4,vec->GetSize());
  CHECK_EQUAL(123,vec->GetFirst().GetIntValue());
  CHECK_EQUAL(321,vec->GetLast().GetIntValue());

  CHECK(vec->Clear());
  CHECK(vec->IsEmpty());
  CHECK_EQUAL(0,vec->GetSize());
  CHECK(!itFive.non_null()->HasNext());
  CHECK(!itFive.IsOK());
}

TEST_FIXTURE(FCollection,MutableMap) {
  ni::Nonnull<ni::iMutableCollection> map{
    ni::cMutableCollectionImpl<
    ni::CollectionTraitsMap<
    ni::CollectionTraitsString,
    ni::CollectionTraitsU32> >::Create()
  };

  CHECK(map->IsEmpty());
  CHECK(map->GetSize() == 0);

  CHECK(map->Add("1"));
  CHECK(map->Add("78"));
  CHECK(map->Add("32"));
  CHECK_EQUAL(ni::kTrue, map->Contains("1"));
  CHECK_EQUAL(ni::kFalse,map->Contains("22"));
  CHECK_EQUAL(ni::kTrue, map->Contains("78"));
  CHECK(map->Clear());
  CHECK(map->IsEmpty());
  CHECK(map->Put("1",1));
  CHECK(map->Put("2",2));
  CHECK(map->Put("3",3));
  CHECK(map->Put("4",4));
  CHECK(map->Put("5",5));
  CHECK(map->Put("6",6));
  CHECK(map->Put("7",7));
  CHECK(map->Put("8",8));

  CHECK_EQUAL(8,map->GetSize());

  CHECK(map->GetDataPtr() == NULL);
  CHECK(map->GetDataSize() == 0);

  {
    ni::Ptr<ni::iIterator> itNotFound = map->Find("1234");
    CHECK(!itNotFound.has_value());
    CHECK(itNotFound.is_null());
  }

  ni::Ptr<ni::iIterator> itFive = map->Find("5");
  CHECK(itFive.has_value());
  CHECK_EQUAL(5,itFive.non_null()->Value().GetIntValue());

  const ni::tU32 seqResult[7] = {1,2,3,5,6,8,78};
  ni::tU32 seq[7] = {0};
  ni::tU32* seqPtr = seq;

  ni::tBool first = ni::eTrue;
  niLoopIterator(it,map) {
    const ni::tU32 v = it->Value().GetIntValue();
    if (first) {
      CHECK(map->Remove("4")); // remove 4
      CHECK(map->Remove("7")); // remove 7
      CHECK(map->Put("99",78)); // use 99 as key, cause I want it to be the last one
      CHECK(map->Remove("1"));
      CHECK_EQUAL(6,map->GetSize());
      CHECK_EQUAL(5,itFive.non_null()->Value().GetIntValue());
      first = ni::eFalse;
    }
    niDebugFmt(("V: %d",v));
    *seqPtr++ = v;
  }

  CHECK_EQUAL(6,map->GetSize());
  niLoop(i,niCountOf(seq)) {
    CHECK_EQUAL(seqResult[i],seq[i]);
  }

  map->RemoveFirst();
  CHECK_EQUAL(5,map->GetSize());
  CHECK_EQUAL(3,map->GetFirst().GetIntValue());

  map->RemoveLast();
  CHECK_EQUAL(4,map->GetSize());
  CHECK_EQUAL(8,map->GetLast().GetIntValue());

  map->SetFirst(123);
  map->SetLast(321);
  CHECK_EQUAL(4,map->GetSize());
  CHECK_EQUAL(123,map->GetFirst().GetIntValue());
  CHECK_EQUAL(321,map->GetLast().GetIntValue());

  CHECK(map->Clear());
  CHECK(map->IsEmpty());
  CHECK(map->GetSize() == 0);
  CHECK(!itFive.non_null()->HasNext());
  CHECK(!itFive.IsOK());
}
