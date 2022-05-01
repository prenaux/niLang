#pragma once
#ifndef __PROGRESSMONITOR_H_DA57A257_FA2C_DA4C_9727_E320480D802B__
#define __PROGRESSMONITOR_H_DA57A257_FA2C_DA4C_9727_E320480D802B__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"
#include "../StringDef.h"
#include "./Sync.h"
#include "./Stopwatch.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

/**

   Usage:
   <pre>
     numOperations = ...;
     sProgressMonitor progress("OperationName", numOperations);
     for (int i = 0; i < numOperations; ++i) {
       doOperation();
       progress.Count();
     }
     progress.Done();
   </pre>

 */
struct sProgressMonitor {
  sProgressMonitor(
    const achar* aName,
    tInt aTotal,
    tInt aStepMinSeconds = 3,
    tInt aStepSeconds = 10)
  {
    _name = aName;
    _total = ni::Max(aTotal,0);
    _stepMinSeconds = _nextMinSeconds = ni::Max(aStepMinSeconds,1);
    _stepSeconds =  _nextSeconds = ni::Max(aStepSeconds,1);

    if (_total == 0) {
      _stepPercent = ni::TypeMax<tInt>();
      _nextCount = ni::TypeMax<tInt>();
    }
    else {
      _stepPercent = ni::Max(aTotal / 10, 1); // 10% or 1 count.
      _nextCount = _stepPercent;
    }

    if (_total == 0) {
      Log(niFmt("%s: Starting", aName));
    }
    else {
      Log(niFmt("%s: Starting with %d items.", aName, aTotal));
    }
  }

  virtual void Log(const achar* msg) {
    niLog(Info, msg);
  }

  tInt GetTotal() const {
    __sync_lock();
    return _total;
  }

  tInt GetCount() const {
    __sync_lock();
    return _count;
  }

  tInt Count() {
    __sync_lock();
    const tInt ret = ++_count;
    const tInt currSeconds = (tInt)_stopwatch.GetSeconds();
    if (_count >= _nextCount && currSeconds >= _nextMinSeconds) {
      _Update("Processed");
      return ret;
    }
    if (currSeconds >= _nextSeconds) {
      _Update("Processed");
      return ret;
    }
    return ret;
  }

  void Done() {
    _Update("Done");
  }

 private:
  __sync_mutex();
  cString _name;
  tInt _total = 0;
  tInt _stepPercent = 0;
  tInt _stepSeconds = 0;
  tInt _stepMinSeconds = 0;
  tInt _count = 0;
  sStopwatch _stopwatch;
  tInt _nextCount = 0;
  tInt _nextSeconds = 0;
  tInt _nextMinSeconds = 0;

  void _Update(const achar* aStatus) {
    __sync_lock();
    const tInt currSeconds = (tInt)_stopwatch.GetSeconds();
    _nextSeconds = currSeconds + _stepSeconds;
    _nextMinSeconds = currSeconds + _stepMinSeconds;
    // A bit fiddly, but permits both the comparison with 0 and the double divide.
    const tInt elapsed = _stopwatch.GetMs();
    cString rate;
    if (elapsed == 0)
      rate = "infinity";
    else
      rate.Set((tInt) (_count * 1000L / (double) elapsed));
    if (_total > 0) {
      _nextCount = _count + _stepPercent;
      Log(niFmt(
        "%s: %s %d / %d items (%2.2f%%) in %.1fs (%g/sec)",
        _name, aStatus,
        _count, _total, (100.0 * _count / _total),
        _stopwatch.GetSeconds(), rate));
    } else {
      Log(niFmt(
        "%s: %s %d items in %.1fs (%g/sec)",
        _name, aStatus, _count,
        _stopwatch.GetSeconds(), rate));
    }
  }
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __PROGRESSMONITOR_H_DA57A257_FA2C_DA4C_9727_E320480D802B__
