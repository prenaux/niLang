#include "stdafx.h"
#include "../src/API/niLang/Utils/CollectionImpl.h"

struct FRand {
};

using namespace ni;

TEST_FIXTURE(FRand,ObjectBase) {
  int4 randObj = ni_prng_init(ni_prng_get_seed_from_maybe_secure_source());
  niDebugFmt(("... randObj: %s", randObj));
  niLoop(i,10) {
    niDebugFmt(("... rand[%d]: %s", i, ni_prng_next_i32(&randObj)));
  }
}

TEST_FIXTURE(FRand,RandInt64) {
  RandSeed(0);
  tI64 nMin = TypeMax<tI64>(), nMax = TypeMin<tI64>();
  niLoop(i,10) {
    niDebugFmt(("... RandInt64[%d]: %s", i, RandInt64()));
  }
  niLoop(i,10000) {
    tI64 r = RandInt64();
    nMin = Min(r,nMin);
    nMax = Max(r,nMax);
  }
  niDebugFmt(("... RandInt64, Min: %s, Max: %s", nMin, nMax));
}

TEST_FIXTURE(FRand,RandInt) {
  RandSeed(0);
  tI32 nMin = TypeMax<tI32>(), nMax = TypeMin<tI32>();
  niLoop(i,10) {
    niDebugFmt(("... RandInt[%d]: %s", i, RandInt()));
  }
  niLoop(i,10000) {
    tI32 r = RandInt();
    nMin = Min(r,nMin);
    nMax = Max(r,nMax);
  }
  niDebugFmt(("... RandInt, Min: %s, Max: %s", nMin, nMax));
}

TEST_FIXTURE(FRand,RandIntRange) {
  RandSeed(0);
  tI32 nMin = TypeMax<tI32>(), nMax = TypeMin<tI32>();
  niLoop(i,10) {
    niDebugFmt(("... RandInt[%d]: %s", i, RandIntRange(-2,7)));
  }
  niLoop(i,10000) {
    tI32 r = RandIntRange(-2,7);
    nMin = Min(r,nMin);
    nMax = Max(r,nMax);
  }
  niDebugFmt(("... RandIntRange, Min: %s, Max: %s", nMin, nMax));
  CHECK(nMin >= -2);
  CHECK(nMax <= 7);
}

TEST_FIXTURE(FRand,RandFloat) {
  RandSeed(0);
  tF64 nMin = TypeMax<tF64>(), nMax = TypeMin<tF64>();
  niLoop(i,10) {
    niDebugFmt(("... RandFloat[%d]: %s", i, RandFloat()));
  }
  niLoop(i,10000) {
    tF64 r = RandFloat();
    nMin = Min(r,nMin);
    nMax = Max(r,nMax);
  }
  niDebugFmt(("... RandFloat, Min: %s, Max: %s", nMin, nMax));
  CHECK(nMin >= 0.0);
  CHECK(nMax <= 1.0);
}

TEST_FIXTURE(FRand,RandSignedFloat) {
  RandSeed(0);
  tF64 nMin = TypeMax<tF64>(), nMax = TypeMin<tF64>();
  niLoop(i,10) {
    niDebugFmt(("... RandSignedFloat[%d]: %s", i, RandSignedFloat()));
  }
  niLoop(i,10000) {
    tF64 r = RandSignedFloat();
    nMin = Min(r,nMin);
    nMax = Max(r,nMax);
  }
  niDebugFmt(("... RandSignedFloat, Min: %s, Max: %s", nMin, nMax));
  CHECK(nMin >= -1.0);
  CHECK(nMax <= 1.0);
}

TEST_FIXTURE(FRand,RandFloatRange) {
  RandSeed(0);
  tF64 nMin = TypeMax<tF64>(), nMax = TypeMin<tF64>();
  niLoop(i,10) {
    niDebugFmt(("... RandFloat[%d]: %s", i, RandFloatRange(-2,7)));
  }
  niLoop(i,10000) {
    tF64 r = RandFloatRange(-2,7);
    nMin = Min(r,nMin);
    nMax = Max(r,nMax);
  }
  niDebugFmt(("... RandFloatRange, Min: %s, Max: %s", nMin, nMax));
  CHECK(nMin >= -2);
  CHECK(nMax <= 7);
}
