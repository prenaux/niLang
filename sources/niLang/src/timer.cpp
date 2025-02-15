// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "Lang.h"

//
// While this would be nice <chrono> is a cursed dependency its the first
// thing that breaks when there's any issue with the C++ compiler. And yes
// this is a lot more common that we'd want, you use Clang, but it wants to
// use gcc's stdlib? better hope the versions (and stars) are aligned - for
// example it'll pick what it thinks is the latest version on your machine but
// you might not know that another program/library installed a more recent GCC
// version than the GCC version you actually use and expect...
//
// #if !defined __JSCC__
// #define niUseStdHighResolutionClock
// #endif
//
#if defined niUseStdHighResolutionClock

#include <chrono>

namespace ni {
niExportFunc(tF64) TimerInSeconds() {
  static const auto _start = std::chrono::high_resolution_clock::now();
	return (std::chrono::high_resolution_clock::now() - _start).count() / 1e9;
}
}

//--------------------------------------------------------------------------------------------
//
//  JSCC
//
//--------------------------------------------------------------------------------------------
#elif defined __JSCC__
#include <emscripten/emscripten.h>

namespace ni {
niExportFunc(tF64) TimerInSeconds() {
	return emscripten_get_now() / 1000.0;
}
}

//--------------------------------------------------------------------------------------------
//
//  OSX
//
//--------------------------------------------------------------------------------------------
#elif defined(__APPLE_CC__) && defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && !defined __IOS__ && !defined __IPHONE_OS_VERSION_MIN_REQUIRED

niExportFunc(uint64_t) __niMach_TimerNano();

namespace ni {

niExportFunc(tF64) TimerInSeconds() {
  return (tF64)__niMach_TimerNano() / 1e9;
}

}

//--------------------------------------------------------------------------------------------
//
//  Undefined Posix compatible platform
//
//--------------------------------------------------------------------------------------------
#elif defined niPosix

#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#if 0
#include "API/niLang/STL/run_once.h"
#endif

namespace ni {

niExportFunc(tF64) TimerInSeconds() {
  static const bool clock_gettime_available = []() {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
      return true;
    }
    else {
      return false;
    }
  }();

#if 0
  niRunOnce {
    if (clock_gettime_available) {
      niLog(Info, "TimerInSeconds: using clock_gettime with CLOCK_MONOTONIC.");
    }
    else {
      niLog(Info, "TimerInSeconds: using gettimeofday (CLOCK_MONOTONIC not available).");
    }
  };
#endif

  if (clock_gettime_available) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (tF64)ts.tv_sec + (tF64)ts.tv_nsec / 1e9;
  }

  struct timeval currentTime;
  gettimeofday(&currentTime, 0);
  tU64 const dsecs = currentTime.tv_sec;
  tU64 const dus = currentTime.tv_usec;
  return ((tF64)(dsecs*1000000 + dus)) / 1e6;
}

}

//--------------------------------------------------------------------------------------------
//
//  Win32
//
//--------------------------------------------------------------------------------------------
#elif defined niWin32
#include "API/niLang/Platforms/Win32/Win32_Redef.h"
#include <mmsystem.h>
#ifdef niPragmaCommentLib
#pragma comment(lib,"winmm.lib")
#endif

namespace ni {

static DWORD mStartTick;
static LONGLONG mLastTime;
static LARGE_INTEGER mStart;
static LARGE_INTEGER mFreq;
static DWORD mTimerMask = 0;

static void _Start() {
  if (mTimerMask == 0) { // find the lowest core this process uses
    DWORD_PTR procMask;
    DWORD_PTR sysMask;
    ::GetProcessAffinityMask(::GetCurrentProcess(),&procMask,&sysMask);
    mTimerMask = 1;
    while ((mTimerMask & procMask) == 0) {
      mTimerMask <<= 1;
    }
  }
  HANDLE thread = ::GetCurrentThread();
  DWORD oldMask = ::SetThreadAffinityMask(thread,mTimerMask);
  ::QueryPerformanceFrequency(&mFreq);
  ::QueryPerformanceCounter(&mStart);
  mStartTick = ::GetTickCount();
  ::SetThreadAffinityMask(thread,oldMask);

  mLastTime = 0;
}

static LONGLONG _GetMicroseconds() {
  static bool _isStarted = false;
  if (!_isStarted) {
    _isStarted = true;
    _Start();
  }

  LARGE_INTEGER curTime;
  HANDLE thread = GetCurrentThread();
  DWORD oldMask = SetThreadAffinityMask(thread,mTimerMask);
  QueryPerformanceCounter(&curTime);
  SetThreadAffinityMask(thread,oldMask);

  LONGLONG newTime = curTime.QuadPart - mStart.QuadPart;

  // check against GetTickCount
  tU32 newTicks = (tU32)(1000 * newTime / mFreq.QuadPart);
  tU32 check = GetTickCount() - mStartTick;
  tI32 msOff = (tI32)(newTicks - check);
  if (msOff < -100 || msOff > 100) {
    LONGLONG adjust = ni::Min(msOff * mFreq.QuadPart / 1000, newTime - mLastTime);
    mStart.QuadPart += adjust;
    newTime -= adjust;
  }

  mLastTime = newTime;

  return 1000000 * newTime / mFreq.QuadPart;
}

niExportFunc(tF64) TimerInSeconds() {
  return (double)_GetMicroseconds() / 1e6;
}

}

//--------------------------------------------------------------------------------------------
//
//  Unknown
//
//--------------------------------------------------------------------------------------------
#else

#error "Unknown platform for Timer implementation."

#endif

//--------------------------------------------------------------------------------------------
//
//  FrameTime
//
//--------------------------------------------------------------------------------------------
using namespace ni;

const tF64 _kfMinFrameTime = 1.0/1000.0;
const tF64 _kfMaxFrameTime = 1.0/1.0;

void __stdcall cLang::ResetFrameTime() {
  mfTotalFrameTime = 0;
  mfAvgCounter = 0;
  mfFrameTime = 1.0/60.0;
  mnFrameNumber = 0;
  mnAverageFPSCounter = 0;
  mnAverageFPS = 0;
  mfLastElapsedTime = 0;
}
tBool __stdcall cLang::UpdateFrameTime(const tF64 afElapsedTime) {
  if (mnFrameNumber == 0) {
    // first frame, no valid frame time yet
    mfFrameTime = 1.0/60.0;
  }
  else {
    mfFrameTime = ni::Clamp(afElapsedTime-mfLastElapsedTime, _kfMinFrameTime, _kfMaxFrameTime);
  }

  mfLastElapsedTime = afElapsedTime;

  mfTotalFrameTime += mfFrameTime;
  ++mnFrameNumber;

  // average FPS
  ++mnAverageFPSCounter;
  mfAvgCounter += mfFrameTime;
  if (mfAvgCounter >= 1.0f) {
    mnAverageFPS = mnAverageFPSCounter;
    mfAvgCounter = 0.0f;
    mnAverageFPSCounter = 0;
  }

  return eTrue;
}
tF64 __stdcall cLang::GetTotalFrameTime() const {
  return mfTotalFrameTime;
}
tF64 __stdcall cLang::GetFrameTime() const {
  return mfFrameTime;
}
tU32 __stdcall cLang::GetFrameNumber() const {
  return mnFrameNumber;
}
tF32 __stdcall cLang::GetFrameRate() const {
  return ni::FInvert(mfFrameTime);
}
tU32 __stdcall cLang::GetAverageFrameRate() const {
  return mnAverageFPS;
}

///////////////////////////////////////////////
tF64 cLang::TimerInSeconds() const {
  return ni::TimerInSeconds();
}
