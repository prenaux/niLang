#include "stdafx.h"

#include "../src/API/niLang/Math/MathUtils.h"
#include "../src/API/niLang/Math/MathMatrix.h"
#include "../src/API/niLang/Math/MathProb.h"

using namespace ni;

struct FMath {
};

TEST_FIXTURE(FMath,ComputeNumPow2Levels) {
  CHECK_EQUAL(0,ni::ComputeNumPow2Levels(0));
  CHECK_EQUAL(1,ni::ComputeNumPow2Levels(1));
  CHECK_EQUAL(2,ni::ComputeNumPow2Levels(2));
  CHECK_EQUAL(3,ni::ComputeNumPow2Levels(4));
  CHECK_EQUAL(4,ni::ComputeNumPow2Levels(8));
  CHECK_EQUAL(5,ni::ComputeNumPow2Levels(16));
  CHECK_EQUAL(6,ni::ComputeNumPow2Levels(32));
  CHECK_EQUAL(7,ni::ComputeNumPow2Levels(64));
  CHECK_EQUAL(8,ni::ComputeNumPow2Levels(128));
  CHECK_EQUAL(9,ni::ComputeNumPow2Levels(256));

  CHECK_EQUAL(8,ni::ComputeNumPow2Levels(200));
  CHECK_EQUAL(9,ni::ComputeNumPow2Levels(500));
  CHECK_EQUAL(10,ni::ComputeNumPow2Levels(550));
}

TEST_FIXTURE(FMath,Matrix2Euler) {
  sMatrixf R = sMatrixf::Identity();

  MatrixRotationYawPitchRoll<tF32>(R, niRad(45), niRad(30), niRad(10));
  niDebugFmt(("... R: %s", R));

  // tF32 yaw, pitch, roll;
  // MatrixGetYawPitchRollDeg(R, yaw, pitch, roll);
  // niDebugFmt(("... R: yaw: %g, pitch: %g, roll: %g", yaw, pitch, roll));

  sVec3f e = MatrixDecomposeYawPitchRoll(R);
  e.x = niDeg(e.x);
  e.y = niDeg(e.y);
  e.z = niDeg(e.z);
  niDebugFmt(("... R: x: %g, y: %g, z: %g", e.x, e.y, e.z));

  CHECK_CLOSE(Vec3f(30, 45, 10), e, sVec3f::Epsilon());
}

enum eProbability {
  eProbability_VeryLow,
  eProbability_Low,
  eProbability_Medium,
  eProbability_High,
  eProbability_VeryHigh,
  eProbability_Last,
};

TEST_FIXTURE(FMath,ProbRaw) {
  //
  // NAME: WEIGHT -> NORMALIZED
  // Verylow: 0.1 / 2.15 = 0.0465%
  // Low: 15 / 2.15 = 6.976%
  // Medium: 50 / 2.15 = 23.25%
  // High: 65 / 2.15 = 30.23%
  // Veryhigh: 85 / 2.15 = 39.53%
  // -- Total: 215
  //

  // Define the probabilities of each categories
  ni::Ptr<ni::tF64CVec> probs = ProbArray(0.1, 15.0, 50.0, 65.0, 85.0);

  // Create the alias arrays used for the prng generation afterward.
  ni::Ptr<ni::tF64CVec> amq = ni::tF64CVec::Create();
  amq->resize(probs->size());
  ni::Ptr<ni::tU32CVec> ama = ni::tU32CVec::Create();
  ama->resize(probs->size()*2);
  ProbSampleBuildAliasMethodArrays(probs->data(), probs->size(),
                                    amq->data(), ama->data());
  CHECK_EQUAL(probs->size(), amq->size());
  CHECK_EQUAL(probs->size()*2, ama->size());

  // Sample 7000 values
  const tU32 knPickCount = 7000;
  ni::int4 prng = ni_prng_init(123);
  ni::Ptr<ni::tU32CVec> r = ni::tU32CVec::Create();
  r->resize(knPickCount);
  ProbSampleAliasMethod(
    r->data(), knPickCount,
    amq->data(), ama->data(), probs->size(),
    [&]() { return ni::RandFloat(&prng); });
  CHECK_EQUAL(knPickCount, r->size());

  // Check the results
  tU32 pickedCount[eProbability_Last] = {};
  tF64 pickedPct[eProbability_Last] = {};
  const tU32* rdata = r->data();
  niLoop(i,r->size()) {
    ++pickedCount[rdata[i]];
  }

  niLoop(i,eProbability_Last) {
    pickedPct[i] = ((tF64)pickedCount[i] / (tF64)knPickCount)*100.0;
    niDebugFmt(("... eProbability[%d]: %d, %g%%",
                i, pickedCount[i], pickedPct[i]));
  }
  CHECK_CLOSE(0.0465, pickedPct[eProbability_VeryLow], 0.1);
  CHECK_CLOSE(6.976, pickedPct[eProbability_Low], 1.0);
  CHECK_CLOSE(23.25, pickedPct[eProbability_Medium], 1.0);
  CHECK_CLOSE(30.23, pickedPct[eProbability_High], 1.0);
  CHECK_CLOSE(39.53, pickedPct[eProbability_VeryHigh], 1.0);
}

TEST_FIXTURE(FMath,ProbHL) {
  ni::Nonnull<ni::tF64CVec> probs = ProbArray(0.1, 15.0, 50.0, 65.0, 85.0);

  ni::int4 prng = ni_prng_init(123);
  const tU32 knPickCount = 7000;
  Nonnull<tU32CVec> r{tU32CVec::Create()};
  ProbSampleAliasMethod(r, knPickCount, probs, &prng);
  CHECK_EQUAL(knPickCount, r->size());

  // Check the results
  tU32 pickedCount[eProbability_Last] = {};
  tF64 pickedPct[eProbability_Last] = {};
  const tU32* rdata = r->data();
  niLoop(i,r->size()) {
    ++pickedCount[rdata[i]];
  }

  niLoop(i,eProbability_Last) {
    pickedPct[i] = ((tF64)pickedCount[i] / (tF64)knPickCount)*100.0;
    niDebugFmt(("... eProbability[%d]: %d, %g%%",
                i, pickedCount[i], pickedPct[i]));
  }
  CHECK_CLOSE(0.0465, pickedPct[eProbability_VeryLow], 0.1);
  CHECK_CLOSE(6.976, pickedPct[eProbability_Low], 1.0);
  CHECK_CLOSE(23.25, pickedPct[eProbability_Medium], 1.0);
  CHECK_CLOSE(30.23, pickedPct[eProbability_High], 1.0);
  CHECK_CLOSE(39.53, pickedPct[eProbability_VeryHigh], 1.0);
}
