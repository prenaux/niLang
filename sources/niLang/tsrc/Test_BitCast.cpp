#include "stdafx.h"
#include "../src/API/niLang/Utils/CollectionImpl.h"

struct FBitCast {
};

using namespace ni;

TEST_FIXTURE(FBitCast,bit_cast) {
  const tF32 _123 = 123.0f;
  tU32 fooi = bit_cast<tU32>(_123);
  tF32 foof = bit_cast<tF32>(fooi);
  CHECK_EQUAL(_123, foof);
}
TEST_FIXTURE(FBitCast,bit_castz) {
  tU64 fooi = bit_castz<tU64>((tI32)123);
  CHECK_EQUAL(123, fooi);
}
