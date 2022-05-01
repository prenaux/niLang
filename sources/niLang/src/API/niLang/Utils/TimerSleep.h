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

/**@}*/
/**@}*/
#endif // __TIMERSLEEP_H_8DBC16C5_BC42_4A98_A91C_894513AD8920__
