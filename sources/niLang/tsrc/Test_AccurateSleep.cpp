#include "stdafx.h"
#include "../src/API/niLang/Utils/TimerSleep.h"
#include "../src/API/niLang/Utils/DataTableUtils.h"

struct FAccurateSleep {};

//
// Inspired by https://blat-blatnik.github.io/computerBear/making-accurate-sleep-function/
//

void SleepOs(ni::tF64 aSeconds) {
  ni::SleepMs(aSeconds * 1000.0);
}

__forceinline void SleepSpin(const ni::tF64 aSeconds) {
  // spin lock
  ni::tF64 spinStart = ni::TimerInSeconds();
  while ((ni::TimerInSeconds() - spinStart) < aSeconds) {
  }
}

void SleepPrecise(ni::tF64 aSeconds) {
  static ni::tF64 estimate = 5e-3;
  static ni::tF64 mean = 5e-3;
  static ni::tF64 m2 = 0;
  static ni::tU64 count = 1;

  while (aSeconds > estimate) {
    const ni::tF64 start = ni::TimerInSeconds();
    ni::SleepMs(1);
    const ni::tF64 end = ni::TimerInSeconds();

    const ni::tF64 observed = (end - start);
    aSeconds -= observed;

    ++count;
    const ni::tF64 delta = observed - mean;
    mean += delta / count;
    m2   += delta * (observed - mean);
    const ni::tF64 stddev = sqrt(m2 / (count - 1));
    estimate = mean + stddev;
  }

  SleepSpin(aSeconds);
}

struct sSleepData {
  ni::tU32 sampleSize = 0; // how many times we tried to sleep
  ni::tF64 target  = 0; // how much we aimed to sleep for
  ni::tF64 meanerr = 0; // how much were we off on average
  ni::tF64 stddev  = 0; // how big was the variance between calls
  ni::tF64 minerr  = +INFINITY; // minimum error we had
  ni::tF64 maxerr  = -INFINITY; // maximum error we had
};

namespace ni {

ni::iDataTable* ToDataTable(iDataTable* apDT, const sSleepData& aData) {
  niCheckIsOK(apDT,NULL);
  apDT->SetInt("sampleSize", aData.sampleSize);
  apDT->SetFloat("target", aData.target);
  apDT->SetFloat("meanerr", aData.meanerr);
  apDT->SetFloat("stddev", aData.stddev);
  apDT->SetFloat("minerr", aData.minerr);
  apDT->SetFloat("maxerr", aData.maxerr);
  return apDT;
}

}

sSleepData _MeasureAccuracy(void (*aSleep)(ni::tF64 aSecs),
                            const ni::tF64 aSecs,
                            const ni::tU32 aRuns)
{
  sSleepData data;
  aSleep(aSecs); // do one cold run

  ni::tF64 mean = 0, m2 = 0;
  for (ni::tU32 i = 1; i <= aRuns; ++i) {
    const ni::tF64 start = ni::TimerInSeconds();
    aSleep(aSecs);
    const ni::tF64 end = ni::TimerInSeconds();
    const ni::tF64 elapsed = (end - start);

    ni::tF64 delta = elapsed - mean;
    mean += delta / i;
    m2   += delta * (elapsed - mean);

    const ni::tF64 error = elapsed - aSecs;
    data.maxerr = fmax(data.maxerr, error);
    data.minerr = fmin(data.minerr, error);

    // niDebugFmt(("... %.2lf\n", elapsed * 1e3));
  }

  data.sampleSize = aRuns;
  data.target = aSecs;
  data.meanerr = mean - aSecs;
  data.stddev = sqrt(m2 / (aRuns + 1));
  return data;
}

TEST_FIXTURE(FAccurateSleep, MeasureAccuracy) {
  {
    sSleepData data = _MeasureAccuracy(SleepOs, 1.0/60.0, 60);
    ni::Ptr<ni::iDataTable> dt = ni::CreateDataTable("sSleepData");
    dt->SetString("functionName","SleepOs");
    niDebugFmt(("... SleepOs: %s",
                ni::DataTableToXML(
                  ni::ToDataTable(dt,data),
                  ni::eFalse)));
    // expect ~5ms error max
    CHECK_CLOSE(0.0, data.meanerr, 5.0 / 1000.0f);
  }

  {
    sSleepData data = _MeasureAccuracy(SleepSpin, 1.0/60.0, 60);
    ni::Ptr<ni::iDataTable> dt = ni::CreateDataTable("sSleepData");
    dt->SetString("functionName","SleepSpin");
    niDebugFmt(("... SleepSpin: %s",
                ni::DataTableToXML(
                  ni::ToDataTable(dt,data),
                  ni::eFalse)));
    // expect ~1microsecond error max
    CHECK_CLOSE(0.0, data.meanerr, 1.0 / 1000000.0f);
  }

  {
    sSleepData data = _MeasureAccuracy(SleepPrecise, 1.0/60.0, 60);
    ni::Ptr<ni::iDataTable> dt = ni::CreateDataTable("sSleepData");
    dt->SetString("functionName","SleepPrecise");
    niDebugFmt(("... SleepPrecise: %s",
                ni::DataTableToXML(
                  ni::ToDataTable(dt,data),
                  ni::eFalse)));
    // expect ~10microsecond error max
    CHECK_CLOSE(0.0, data.meanerr, 10.0 / 1000000.0f);
  }
}
