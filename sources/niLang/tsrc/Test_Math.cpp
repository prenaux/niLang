#include "stdafx.h"

#include "../src/API/niLang/Math/MathUtils.h"
#include "../src/API/niLang/Math/MathMatrix.h"

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
