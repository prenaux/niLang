#include "stdafx.h"
#include "../src/API/niLang/Utils/TimerSleep.h"
#include "../src/API/niLang/Utils/DataTableUtils.h"

struct FAccurateSleep {};

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

TEST_FIXTURE(FAccurateSleep, MeasureAccuracy_SleepSecsCoarse) {
  sSleepData data = _MeasureAccuracy(ni::SleepSecsCoarse, 1.0/60.0, 60);
  ni::Ptr<ni::iDataTable> dt = ni::CreateDataTable("sSleepData");
  dt->SetString("functionName","SleepSecsCoarse");
  niDebugFmt(("... SleepOs: %s",
              ni::DataTableToXML(
                ni::ToDataTable(dt,data),
                ni::eFalse)));
#ifdef niWindows
  // expect 15ms error max
  CHECK_LE(data.meanerr, 15e-3);
#else
  // expect 5ms error max
  CHECK_LE(data.meanerr, 5e-3);
#endif
}

TEST_FIXTURE(FAccurateSleep, MeasureAccuracy_SleepSecsSpin) {
  sSleepData data = _MeasureAccuracy(ni::SleepSecsSpin, 1.0/60.0, 60);
  ni::Ptr<ni::iDataTable> dt = ni::CreateDataTable("sSleepData");
  dt->SetString("functionName","SleepSecsSpin");
  niDebugFmt(("... SleepSpin: %s",
              ni::DataTableToXML(
                ni::ToDataTable(dt,data),
                ni::eFalse)));
  // expect 15us error max
  CHECK_LE(data.meanerr, 2e-5);
}

TEST_FIXTURE(FAccurateSleep, MeasureAccuracy_SleepSecs) {
  sSleepData data = _MeasureAccuracy(ni::SleepSecs, 1.0/60.0, 60);
  ni::Ptr<ni::iDataTable> dt = ni::CreateDataTable("sSleepData");
  dt->SetString("functionName","SleepSecs (Precise)");
  niDebugFmt(("... SleepPrecise: %s",
              ni::DataTableToXML(
                ni::ToDataTable(dt,data),
                ni::eFalse)));
  // expect 20us error max
  CHECK_LE(data.meanerr, 2e-5);
}
