#include <niLang/Types.h>

#ifdef niWindows

#include <niLang/Platforms/Win32/Win32_Redef.h>
#include <xinput.h>

#include <niLang/ILang.h>
#include <niLang/STL/utils.h>
#include <niLang/Utils/UnknownImpl.h>
#include "API/niLang/IGameCtrl.h"

using namespace ni;

static tU32 _xinputInitCount = 0;
static astl::vector<Ptr<iGameCtrl> >  _xinputCtrls;
static tIntPtr _xinputDLL = NULL;

// #define TRACE_XINPUT
#define XINPUT_NUM_CONTROLLERS 4

#define DEADZONE_LEFT_THUMB (XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
#define DEADZONE_RIGHT_THUMB (XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
#define DEADZONE_TRIGGER (XINPUT_GAMEPAD_TRIGGER_THRESHOLD)

typedef void (WINAPI * tpfnXInputEnable)(
    _In_  BOOL enable);
static tpfnXInputEnable _pfnXInputEnable = NULL;

typedef DWORD (WINAPI * tpfnXInputGetCapabilities)(
    _In_   DWORD dwUserIndex,
    _In_   DWORD dwFlags,
    _Out_  XINPUT_CAPABILITIES *pCapabilities);
static tpfnXInputGetCapabilities _pfnXInputGetCapabilities = NULL;

typedef DWORD (WINAPI * tpfnXInputGetState)(
    _In_   DWORD dwUserIndex,
    _Out_  XINPUT_STATE *pState);
static tpfnXInputGetState _pfnXInputGetState = NULL;

typedef DWORD (WINAPI * tpfnXInputSetState)(
    _In_     DWORD dwUserIndex,
    _Inout_  XINPUT_VIBRATION *pVibration);
static tpfnXInputSetState _pfnXInputSetState = NULL;

//
// Devices that support batteries
//
#define BATTERY_DEVTYPE_GAMEPAD         0x00
#define BATTERY_DEVTYPE_HEADSET         0x01

//
// Flags for battery status level
//
#define BATTERY_TYPE_DISCONNECTED       0x00    // This device is not connected
#define BATTERY_TYPE_WIRED              0x01    // Wired device, no battery
#define BATTERY_TYPE_ALKALINE           0x02    // Alkaline battery source
#define BATTERY_TYPE_NIMH               0x03    // Nickel Metal Hydride battery source
#define BATTERY_TYPE_UNKNOWN            0xFF    // Cannot determine the battery type

// These are only valid for wireless, connected devices, with known battery types
// The amount of use time remaining depends on the type of device.
#define BATTERY_LEVEL_EMPTY             0x00
#define BATTERY_LEVEL_LOW               0x01
#define BATTERY_LEVEL_MEDIUM            0x02
#define BATTERY_LEVEL_FULL              0x03

typedef struct _MY_XINPUT_BATTERY_INFORMATION {
  BYTE BatteryType;
  BYTE BatteryLevel;
} MY_XINPUT_BATTERY_INFORMATION, *PMY_XINPUT_BATTERY_INFORMATION;

typedef DWORD (WINAPI * tpfnXInputGetBatteryInformation)(
    _In_   DWORD dwUserIndex,
    _In_   BYTE devType,
    _Out_  MY_XINPUT_BATTERY_INFORMATION *pBatteryInformation
                                                         );
static tpfnXInputGetBatteryInformation _pfnXInputGetBatteryInformation = NULL;

//--------------------------------------------------------------------------------------------
//
//  cXInputGameCtrl implementation
//
//--------------------------------------------------------------------------------------------
class cXInputGameCtrl : public cIUnknownImpl<iGameCtrl>
{
  const tU32 mnIndex;
  struct sStates {
    tBool mbIsConnected;
    tU16 mButtons;
    sVec2f mvLeft, mvRight;
    tF32 mfLeftTrigger, mfRightTrigger;
    sStates() {
      niZeroMember(*this);
    }
    sStates(const sStates& aRight) {
      ni::MemCopy((tPtr)this,(tPtr)&aRight,sizeof(*this));
    }
  };
  sStates mStates;

 public:
  cXInputGameCtrl(tU32 anIndex) : mnIndex(anIndex) {
    niZeroMember(mStates);
  }
  ~cXInputGameCtrl() {
  }

  //// iGameCtrl /////////////////////////////
  tBool __stdcall IsOK() const niImpl {
    return eTrue;
  }

  tBool __stdcall Update() niImpl {
    const sStates wasStates = mStates;

    XINPUT_STATE state;
    tBool bSomethingChanged = eFalse;

    if (_pfnXInputGetState(mnIndex,&state) != ERROR_SUCCESS) {
      bSomethingChanged = (mStates.mbIsConnected == eTrue);
      mStates.mbIsConnected = eFalse;
    }
    else {

      {
        bSomethingChanged = (mStates.mbIsConnected == eFalse);
        mStates.mbIsConnected = eTrue;
      }

      {
        float LX = state.Gamepad.sThumbLX;
        float LY = state.Gamepad.sThumbLY;

        // determine how far the controller is pushed
        float magnitude = ni::Sqrt(LX*LX + LY*LY);
        // check if the controller is outside a circular dead zone
        if (magnitude > DEADZONE_LEFT_THUMB)
        {
          bSomethingChanged = eTrue;

          // clip the magnitude at its expected maximum value
          if (magnitude > 32767) magnitude = 32767;

          // adjust magnitude relative to the end of the dead zone
          magnitude -= DEADZONE_LEFT_THUMB;

          // optionally normalize the magnitude with respect to its expected range
          // giving a magnitude value of 0.0 to 1.0
          float normalizedLX = LX / magnitude;
          float normalizedLY = LY / magnitude;
          float normalizedMagnitude = magnitude / (32767 - DEADZONE_LEFT_THUMB);

          mStates.mvLeft.x = normalizedLX * normalizedMagnitude;
          mStates.mvLeft.y = normalizedLY * normalizedMagnitude;
        }
        else {
          bSomethingChanged = bSomethingChanged || (mStates.mvLeft != sVec2f::Zero());
          mStates.mvLeft = sVec2f::Zero();
        }
      }

      {
        float RX = state.Gamepad.sThumbRX;
        float RY = state.Gamepad.sThumbRY;

        // determine how far the controller is pushed
        float magnitude = ni::Sqrt(RX*RX + RY*RY);

        // check if the controller is outside a circular dead zone
        if (magnitude > DEADZONE_RIGHT_THUMB)
        {
          bSomethingChanged = eTrue;

          // clip the magnitude at its expected maximum value
          if (magnitude > 32767) magnitude = 32767;

          // adjust magnitude relative to the end of the dead zone
          magnitude -= DEADZONE_RIGHT_THUMB;

          // optionally normalize the magnitude with respect to its expected range
          // giving a magnitude value of 0.0 to 1.0
          float normalizedRX = RX / magnitude;
          float normalizedRY = RY / magnitude;
          float normalizedMagnitude = magnitude / (32767 - DEADZONE_RIGHT_THUMB);

          mStates.mvRight.x = normalizedRX * normalizedMagnitude;
          mStates.mvRight.y = normalizedRY * normalizedMagnitude;
        }
        else {
          bSomethingChanged = bSomethingChanged || (mStates.mvRight != sVec2f::Zero());
          mStates.mvRight = sVec2f::Zero();
        }
      }

      {
        float magnitude = state.Gamepad.bLeftTrigger;
        if (magnitude > DEADZONE_TRIGGER) {
          bSomethingChanged = eTrue;
          magnitude -= (float)DEADZONE_TRIGGER;
          magnitude /= (float)(255 - DEADZONE_TRIGGER);
        }
        else {
          bSomethingChanged = bSomethingChanged || (!niFloatIsZero(mStates.mfLeftTrigger));
          magnitude = 0.0;
        }
        mStates.mfLeftTrigger = magnitude;
      }

      {
        float magnitude = state.Gamepad.bRightTrigger;
        if (magnitude > DEADZONE_TRIGGER) {
          bSomethingChanged = eTrue;
          magnitude -= (float)DEADZONE_TRIGGER;
          magnitude /= (float)(255 - DEADZONE_TRIGGER);
        }
        else {
          bSomethingChanged = bSomethingChanged || (!niFloatIsZero(mStates.mfRightTrigger));
          magnitude = 0.0;
        }
        mStates.mfRightTrigger = magnitude;
      }

      {
        bSomethingChanged = bSomethingChanged || (mStates.mButtons != state.Gamepad.wButtons);
        mStates.mButtons = state.Gamepad.wButtons;
      }
    }

#ifdef TRACE_XINPUT
    if (bSomethingChanged) {
      niDebugFmt(("... UPDATED XINPUT %d: Left:%s, Right:%s, LT: %g, RT: %g, Buttons: %x",
                  mnIndex,
                  mStates.mvLeft,
                  mStates.mvRight,
                  mStates.mfLeftTrigger,
                  mStates.mfRightTrigger,
                  mStates.mButtons));
    }
#endif

    return bSomethingChanged;
  }

  tU32 __stdcall GetIndex() const niImpl {
    return mnIndex;
  }

  tBool __stdcall GetIsConnected() const niImpl {
    return mStates.mbIsConnected;
  }
  const achar*  __stdcall GetName() const niImpl {
    return "XInput";
  }

  tU32  __stdcall GetNumAxis() const niImpl {
    return 4;
  }

  tU32  __stdcall GetNumButtons() const niImpl {
    return 16;
  }

  tF32  __stdcall GetButton(tU32 anButton) niImpl {
    switch (anButton) {
      case eGameCtrlButton_A:
        return (mStates.mButtons & XINPUT_GAMEPAD_A) ? 1.0f : 0.0f;
      case eGameCtrlButton_B:
        return (mStates.mButtons & XINPUT_GAMEPAD_B) ? 1.0f : 0.0f;
      case eGameCtrlButton_X:
        return (mStates.mButtons & XINPUT_GAMEPAD_X) ? 1.0f : 0.0f;
      case eGameCtrlButton_Y:
        return (mStates.mButtons & XINPUT_GAMEPAD_Y) ? 1.0f : 0.0f;
      case eGameCtrlButton_LeftBlock:
        return (mStates.mButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) ? 1.0f : 0.0f;
      case eGameCtrlButton_RightBlock:
        return (mStates.mButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) ? 1.0f : 0.0f;
      case eGameCtrlButton_Back:
        return (mStates.mButtons & XINPUT_GAMEPAD_BACK) ? 1.0f : 0.0f;
      case eGameCtrlButton_Start:
        return (mStates.mButtons & XINPUT_GAMEPAD_START) ? 1.0f : 0.0f;
      case eGameCtrlButton_LeftThumb:
        return (mStates.mButtons & XINPUT_GAMEPAD_LEFT_THUMB) ? 1.0f : 0.0f;
      case eGameCtrlButton_RightThumb:
        return (mStates.mButtons & XINPUT_GAMEPAD_RIGHT_THUMB) ? 1.0f : 0.0f;
      case eGameCtrlButton_LeftTrigger:
        return mStates.mfLeftTrigger;
      case eGameCtrlButton_RightTrigger:
        return mStates.mfRightTrigger;
      case eGameCtrlButton_DPadUp:
        return (mStates.mButtons & XINPUT_GAMEPAD_DPAD_UP) ? 1.0f : 0.0f;
      case eGameCtrlButton_DPadDown:
        return (mStates.mButtons & XINPUT_GAMEPAD_DPAD_DOWN) ? 1.0f : 0.0f;
      case eGameCtrlButton_DPadLeft:
        return (mStates.mButtons & XINPUT_GAMEPAD_DPAD_LEFT) ? 1.0f : 0.0f;
      case eGameCtrlButton_DPadRight:
        return (mStates.mButtons & XINPUT_GAMEPAD_DPAD_RIGHT) ? 1.0f : 0.0f;
    }
    return 0.0f;
  }
  tF32  __stdcall GetAxis(eGameCtrlAxis anAxis) niImpl {
    switch (anAxis) {
      case eGameCtrlAxis_LX:
        return mStates.mvLeft.x;
      case eGameCtrlAxis_LY:
        return mStates.mvLeft.y;
      case eGameCtrlAxis_RX:
        return mStates.mvRight.x;
      case eGameCtrlAxis_RY:
        return mStates.mvRight.y;
    }
    return 0.0f;
  }

  tBool __stdcall GetCanVibrate() const niImpl {
    return _pfnXInputSetState != NULL;
  }
  void __stdcall Vibrate(const sVec2f& aSpeed) niImpl {
    if (_pfnXInputSetState) {
      XINPUT_VIBRATION vibration = {
        (WORD)(aSpeed.x * 65535.0f),
        (WORD)(aSpeed.y * 65535.0f)
      };
      _pfnXInputSetState(mnIndex,&vibration);
    }
  }

  tBool __stdcall GetHasBattery() const {
    if (_pfnXInputGetBatteryInformation) {
      MY_XINPUT_BATTERY_INFORMATION info = {0};
      _pfnXInputGetBatteryInformation(mnIndex, BATTERY_DEVTYPE_GAMEPAD, &info);
      return info.BatteryType > BATTERY_TYPE_WIRED;
    }
    return eFalse;
  }
  tF32 __stdcall GetBatteryLevel() const {
    if (_pfnXInputGetBatteryInformation) {
      MY_XINPUT_BATTERY_INFORMATION info = {0};
      _pfnXInputGetBatteryInformation(mnIndex, BATTERY_DEVTYPE_GAMEPAD, &info);
      switch (info.BatteryLevel) {
        case BATTERY_LEVEL_EMPTY:
          return 0.0f;
        case BATTERY_LEVEL_LOW:
          return 0.33f;
        case BATTERY_LEVEL_MEDIUM:
          return 0.66f;
        case BATTERY_LEVEL_FULL:
          return 1.0f;
      }
    }
    return 1.0f;
  }
  //// iGameCtrl /////////////////////////////
};

//--------------------------------------------------------------------------------------------
//
//  cOSWindowGeneric implementation
//
//--------------------------------------------------------------------------------------------
tBool __stdcall XInputGameCtrls_Startup() {
  if (_xinputInitCount != 0)
    return eTrue;

  _xinputDLL = ni_dll_load("xinput1_3.dll");
  if (!_xinputDLL) {
    _xinputDLL = ni_dll_load("xinput1_2.dll");
    if (!_xinputDLL) {
      _xinputDLL = ni_dll_load("xinput1_1.dll");
      if (!_xinputDLL) {
        niWarning("No XInput DLL.");
        return eFalse;
      }
    }
  }

  _pfnXInputEnable = (tpfnXInputEnable)ni_dll_get_proc(_xinputDLL, "XInputEnable");
  if (!_pfnXInputEnable) {
    niWarning("Can't get XInputEnable in XInput DLL.");
    ni_dll_free(_xinputDLL);
    _xinputDLL = NULL;
    return eFalse;
  }

  _pfnXInputGetCapabilities = (tpfnXInputGetCapabilities)ni_dll_get_proc(_xinputDLL, "XInputGetCapabilities");
  if (!_pfnXInputGetCapabilities) {
    niWarning("Can't get XInputGetCapabilities in XInput DLL.");
    ni_dll_free(_xinputDLL);
    _xinputDLL = NULL;
    return eFalse;
  }

  _pfnXInputGetState = (tpfnXInputGetState)ni_dll_get_proc(_xinputDLL, "XInputGetState");
  if (!_pfnXInputGetState) {
    niWarning("Can't get XInputGetState in XInput DLL.");
    ni_dll_free(_xinputDLL);
    _xinputDLL = NULL;
    return eFalse;
  }

  _pfnXInputSetState = (tpfnXInputSetState)ni_dll_get_proc(_xinputDLL, "XInputSetState");
  if (!_pfnXInputSetState) {
    niWarning("Can't get XInputSetState in XInput DLL, vibrate won't be available.");
  }

  _pfnXInputGetBatteryInformation = (tpfnXInputGetBatteryInformation)ni_dll_get_proc(_xinputDLL, "XInputGetBatteryInformation");
  if (!_pfnXInputGetBatteryInformation) {
    niWarning("Can't get XInputGetBatteryInformation in XInput DLL, battery info won't be available.");
  }

  niLoop(i,XINPUT_NUM_CONTROLLERS) {
    _xinputCtrls.push_back(niNew cXInputGameCtrl(i));
  }

  tU32 numConnected = 0;
  niLoop(i,_xinputCtrls.size()) {
    if (_xinputCtrls[i]->GetIsConnected())
      ++numConnected;
  }

  niLog(Info,niFmt("Input, found '%d' game controllers, %d connected.",
              _xinputCtrls.size(),
              numConnected));

  ++_xinputInitCount;
  return eTrue;
}
void __stdcall XInputGameCtrls_Shutdown() {
  if (!_xinputInitCount || --_xinputInitCount != 0)
    return;
  astl::clear_invalidate(_xinputCtrls);
}
tU32 __stdcall XInputGameCtrls_GetNumGameCtrls() {
  return _xinputCtrls.size();
}
iGameCtrl* __stdcall XInputGameCtrls_GetGameCtrl(tU32 anIndex) {
  return anIndex < _xinputCtrls.size() ? _xinputCtrls[anIndex].ptr() : NULL;
}

#endif
