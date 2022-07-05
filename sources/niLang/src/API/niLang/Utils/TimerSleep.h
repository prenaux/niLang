#ifndef __TIMERSLEEP_H_8DBC16C5_BC42_4A98_A91C_894513AD8920__
#define __TIMERSLEEP_H_8DBC16C5_BC42_4A98_A91C_894513AD8920__
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

#ifdef niWinDesktop
//--------------------------------------------------------------------------------------------
//
//  Windows Desktop version
//
//--------------------------------------------------------------------------------------------

// Don't include windows.h just for this
extern "C" __declspec(dllimport) void __stdcall Sleep(ni::tU32 dwMilliseconds);

namespace ni {

inline void SleepMs(tU32 anMs) {
  ::Sleep(anMs);
}

}

#elif defined niPosix
//--------------------------------------------------------------------------------------------
//
//  Posix version
//
//--------------------------------------------------------------------------------------------
#include <unistd.h>

namespace ni {

inline void SleepMs(tU32 anMs) {
  usleep(anMs*1000);
}

}

#else
//--------------------------------------------------------------------------------------------
//
//  Unknown platform
//
//--------------------------------------------------------------------------------------------
#error "SleepMs not implemented for this platform."

#endif

namespace ni {

//! Generally coarse and imprecise (up to 15ms variation) but uses the least
//! cpu. Use ni::SleepSecsPrecise if you need a precise sleep.
void SleepSecsCoarse(ni::tF64 aSeconds) {
  ni::SleepMs(aSeconds * 1000.0);
}

//! The most precise sleep method, but uses 100% cpu.
__forceinline void SleepSecsSpin(const ni::tF64 aSeconds) {
  // spin lock
  ni::tF64 spinStart = ni::TimerInSeconds();
  while ((ni::TimerInSeconds() - spinStart) < aSeconds) {
  }
}

//! A precise sleep method with low cpu usage. The best default choice.
__forceinline void SleepSecs(ni::tF64 aSeconds) {
  // Inspired by https://blat-blatnik.github.io/computerBear/making-accurate-sleep-function/

  static niThreadLocal11 ni::tF64 estimate = 5e-3;
  static niThreadLocal11 ni::tF64 mean = 5e-3;
  static niThreadLocal11 ni::tF64 m2 = 0;
  static niThreadLocal11 ni::tU64 count = 1;

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

  SleepSecsSpin(aSeconds);
}

} // end of namespace ni

/**@}*/
/**@}*/
#endif // __TIMERSLEEP_H_8DBC16C5_BC42_4A98_A91C_894513AD8920__
