#ifndef __TIMERMANAGER_H_8CD12188_7621_431E_BC04_57E70EF8C0FF__
#define __TIMERMANAGER_H_8CD12188_7621_431E_BC04_57E70EF8C0FF__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Types.h>
#include <niLang/STL/map.h>
#include <niLang/STL/utils.h>

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

class TimerManager {
 private:
  struct sHandlerAndId {
    Ptr<iMessageHandler> mHandler;
    tU32 mnId;
    sHandlerAndId(const iMessageHandler* aHandler, tU32 anId)
        : mHandler(aHandler)
        , mnId(anId)
    {}
    bool operator == (const sHandlerAndId& a) const {
      return a.mnId == mnId && a.mHandler == mHandler;
    }
    bool operator < (const sHandlerAndId& a) const {
      if (mHandler.ptr() > a.mHandler.ptr()) {
        return false;
      }
      else if (mHandler.ptr() == a.mHandler.ptr()) {
        return mnId < a.mnId;
      }
      return true; // is less
    }
  };
  struct sTimer {
    tF32 mfStart;
    tF32 mfDuration;
    sTimer(tF32 afDuration) : mfStart(0), mfDuration(afDuration) {
    }
    tBool IsExpired() const {
      return (mfStart >= mfDuration);
    }
  };

  typedef astl::map<sHandlerAndId,sTimer> tTimerMap;
  tTimerMap mmapTimers;

 public:
  void ClearTimers() {
    mmapTimers.clear();
  }

  tU32 GetNumTimers() const {
    return mmapTimers.size();
  }

  tBool SetTimer(iMessageHandler* apHandler, tU32 anTimerId, tF32 afDuration) {
    const sHandlerAndId targetAndId(apHandler,anTimerId);
    tTimerMap::iterator it = mmapTimers.find(targetAndId);
    if (it == mmapTimers.end()) {
      if (afDuration <= niEpsilon5) {
        // Timer doesnt exist, and we dont need to add it...
        return eFalse;
      }
      it = astl::upsert(mmapTimers,targetAndId,sTimer(afDuration));
    }
    else {
      // If afDuration < 0 the next call to Update() will take care of removing the timer
      it->second = sTimer(afDuration);
    }
    return eTrue;
  }

  tF32 GetTimer(const iMessageHandler* apHandler, tU32 anTimerId) {
    tTimerMap::iterator it = mmapTimers.find(sHandlerAndId(apHandler,anTimerId));
    return it == mmapTimers.end() ? -1 : it->second.mfDuration;
  }

  void InvalidateHandlerTimers(const iMessageHandler* apHandler) {
    for (tTimerMap::iterator it = mmapTimers.begin(); it != mmapTimers.end(); ++it) {
      if (it->first.mHandler == apHandler) {
        it->second.mfDuration = -1;
      }
    }
  }

  void UpdateTimers(tF32 afFrameTime) {
    niAssert(afFrameTime >= 0.0f && afFrameTime <= 10.0f);
    for (tTimerMap::iterator it = mmapTimers.begin(); it != mmapTimers.end(); ) {
      sTimer& timer = it->second;
      if (timer.mfDuration <= niEpsilon5) {
        it = astl::map_erase(mmapTimers,it);
      }
      else {
        timer.mfStart += afFrameTime;
        const tF32 fDuration = timer.mfDuration;
        tU32 triggerCount = 0;
        while (timer.IsExpired()) {
          ++triggerCount;
          niAssert(triggerCount < 100);
          if (triggerCount >= 100) {
            niWarning(niFmt("Timers[%s]: Timer '%p:%d' triggered more than 100 times in one update - cancelled.",GetTimersName(),it->first.mHandler.ptr(),it->first.mnId));
            break;
          }
          TimerTriggered(it->first.mHandler,it->first.mnId,fDuration);
          if (mmapTimers.empty())
            break; // in case timers have been cleared by TimerTriggered
          if (timer.mfDuration <= niEpsilon5) {
            break;
          }
          timer.mfStart -= timer.mfDuration;
        }
        if (mmapTimers.empty()) {
          break; // in case timers have been cleared by TimerTriggered
        }
        ++it;
      }
    }
    // Remove all invalid timers
    {
      for (tTimerMap::iterator it = mmapTimers.begin(); it != mmapTimers.end(); ) {
        sTimer& timer = it->second;
        if (timer.mfDuration <= niEpsilon5) {
          it = astl::map_erase(mmapTimers,it);
        }
        else {
          ++it;
        }
      }
    }
  }

  virtual const achar* __stdcall GetTimersName() const = 0;
  virtual void __stdcall TimerTriggered(iMessageHandler* apHandler, tU32 anId, tF32 afDuration) = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __TIMERMANAGER_H_8CD12188_7621_431E_BC04_57E70EF8C0FF__
