#define INITGUID // so that we dont need to link with dxguid.lib, must be first...
#include "API/niLang/Types.h"

#ifdef niWindows

#include "API/niLang/ILang.h"

#include "API/niLang/Platforms/Win32/Win32_Redef.h"
#define DIRECTINPUT_VERSION 0x800
#include <dinput.h>

#include "API/niLang/STL/utils.h"
#include "API/niLang/Utils/UnknownImpl.h"
#include "API/niLang/IGameCtrl.h"
#include "API/niLang_ModuleDef.h"

#pragma comment(lib,"dinput8.lib")

using namespace ni;

static const achar* GetDIError(HRESULT hr) {
  switch (hr) {
    case DIERR_OLDDIRECTINPUTVERSION:   return _A("DIERR_OLDDIRECTINPUTVERSION");
    case DIERR_BETADIRECTINPUTVERSION:    return _A("DIERR_BETADIRECTINPUTVERSION");
    case DIERR_BADDRIVERVER:    return _A("DIERR_BADDRIVERVER");
    case DIERR_DEVICENOTREG:    return _A("DIERR_DEVICENOTREG");
    case DIERR_NOTFOUND:      return _A("DIERR_NOTFOUND");
    case DIERR_INVALIDPARAM:    return _A("DIERR_INVALIDPARAM");
    case DIERR_NOINTERFACE:     return _A("DIERR_NOINTERFACE");
    case DIERR_GENERIC:       return _A("DIERR_GENERIC");
    case DIERR_OUTOFMEMORY:     return _A("DIERR_OUTOFMEMORY");
    case DIERR_UNSUPPORTED:     return _A("DIERR_UNSUPPORTED");
    case DIERR_NOTINITIALIZED:    return _A("DIERR_NOTINITIALIZED");
    case DIERR_ALREADYINITIALIZED:  return _A("DIERR_ALREADYINITIALIZED");
    case DIERR_NOAGGREGATION:   return _A("DIERR_NOAGGREGATION");
    case DIERR_OTHERAPPHASPRIO:   return _A("DIERR_OTHERAPPHASPRIO");
    case DIERR_INPUTLOST:     return _A("DIERR_INPUTLOST");
    case DIERR_ACQUIRED:      return _A("DIERR_ACQUIRED");
    case DIERR_NOTACQUIRED:     return _A("DIERR_NOTACQUIRED");
    case DIERR_INSUFFICIENTPRIVS: return _A("DIERR_INSUFFICIENTPRIVS");
    case DIERR_DEVICEFULL:      return _A("DIERR_DEVICEFULL");
    case DIERR_MOREDATA:      return _A("DIERR_MOREDATA");
    case DIERR_NOTDOWNLOADED:   return _A("DIERR_NOTDOWNLOADED");
    case DIERR_HASEFFECTS:      return _A("DIERR_HASEFFECTS");
    case DIERR_NOTEXCLUSIVEACQUIRED:return _A("DIERR_NOTEXCLUSIVEACQUIRED");
    case DIERR_INCOMPLETEEFFECT:  return _A("DIERR_INCOMPLETEEFFECT");
    case DIERR_NOTBUFFERED:     return _A("DIERR_NOTBUFFERED");
    case DIERR_EFFECTPLAYING:   return _A("DIERR_EFFECTPLAYING");
    case DIERR_UNPLUGGED:     return _A("DIERR_UNPLUGGED");
    case DIERR_REPORTFULL:      return _A("DIERR_REPORTFULL");
    case DIERR_MAPFILEFAIL:     return _A("DIERR_MAPFILEFAIL");
  }
  return _A("DIERR_UNKNOWN");
}
#define DIERROR() niError(niFmt(_A("Direct Input Error : %s."),GetDIError(hr)));

static tU32 _dinputInitCount = 0;
static IDirectInput8* _pDI = NULL;
static astl::vector<Ptr<iGameCtrl> > _dinputCtrls;

static BOOL __stdcall EnumGameCtrlDeviceCallback(const DIDEVICEINSTANCE* pDevice, void* ptrThis);

tBool __stdcall DInputGameCtrls_Startup() {
  if (_dinputInitCount != 0)
    return eTrue;

  // Initialize the DirectInput object
  HRESULT hr = DirectInput8Create(
      GetModuleHandle(NULL),
      DIRECTINPUT_VERSION,IID_IDirectInput8,(void**)&_pDI,NULL);
  if (FAILED(hr)) {
    niSafeRelease(_pDI);
    niWarning(_A("Can't initialize the direct input object."));
    return eFalse;
  }

  // Enum the game controllers
  if (_pDI) {
    HRESULT hr = _pDI->EnumDevices(DI8DEVCLASS_GAMECTRL,
                                   EnumGameCtrlDeviceCallback,
                                   NULL,
                                   DIEDFL_ALLDEVICES);
    if (FAILED(hr)) {
      niSafeRelease(_pDI);
      niError(_A("Can't enumerate game controllers."));
      return eFalse;
    }
  }
  niLog(Info,niFmt(_A("Input, found '%d' game controllers."),_dinputCtrls.size()));

  ++_dinputInitCount;
  return eTrue;
}
void __stdcall DInputGameCtrls_Shutdown() {
  if (!_dinputInitCount || --_dinputInitCount != 0)
    return;
  astl::clear_invalidate(_dinputCtrls);
  niSafeRelease(_pDI);
}
tU32 __stdcall DInputGameCtrls_GetNumGameCtrls() {
  return _dinputCtrls.size();
}
iGameCtrl* __stdcall DInputGameCtrls_GetGameCtrl(tU32 anIndex) {
  return anIndex < _dinputCtrls.size() ? _dinputCtrls[anIndex].ptr() : NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////
#define GAMEPAD_DEADZONENORM  ((SHORT)(0.0 * 10000.0f))

class cDInputGameCtrl : public ImplRC<iGameCtrl>
{
  typedef astl::vector<DIDEVICEOBJECTINSTANCE> tDevObjVec;
  typedef tDevObjVec::iterator        tDevObjVecIt;

 public:
  cDInputGameCtrl(IDirectInput8* apDI,  tU32 aulIndex, const DIDEVICEINSTANCE* pDevInst) {

    ZeroMembers();

    mDevInst = *pDevInst;
    mnIndex = aulIndex;

    HRESULT hr = apDI->CreateDevice(mDevInst.guidInstance,&mpDevice,NULL);
    if (FAILED(hr)) {
      DIERROR();
      niError(_A("Can't initialize the device."));
      niSafeRelease(mpDevice);
      return;
    }

    mCaps.dwSize = sizeof(mCaps);
    hr = mpDevice->GetCapabilities(&mCaps);
    if (FAILED(hr))
    {
      DIERROR();
      niError(_A("Can't get device capabilities."));
      niSafeRelease(mpDevice);
      return;
    }

    hr = mpDevice->EnumObjects(EnumObjectsCallback, this, DIDFT_ALL);
    if (FAILED(hr))
    {
      DIERROR();
      niError(_A("Can't enumerate device objects."));
      niSafeRelease(mpDevice);
      return;
    }

    hr = mpDevice->SetDataFormat(&c_dfDIJoystick2);
    if (FAILED(hr))
    {
      DIERROR();
      niError(_A("Can't set the data format."));
      niSafeRelease(mpDevice);
      return;
    }

    //  hr = mpDevice->SetCooperativeLevel(aHWND, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
    //  if (FAILED(hr))
    //  {
    //    DIERROR();
    //    niError(_A("Can't set the cooperative level."));
    //    niSafeRelease(mpDevice);
    //    return;
    //  }

    // Retrieve the controller's properties
    {
      niLoop(i,mvObjs.size()) {
        const DIDEVICEOBJECTINSTANCE& doi = mvObjs[i];
        if (doi.dwType & DIDFT_BUTTON) {
          niLog(Info,niFmt(_A("Game controller %d (%s), Button[%d] '%s' (%s)."),
                      GetIndex(), GetName(),
                      DIDFT_GETINSTANCE(doi.dwType),
                      doi.tszName,
                      niEnumToChars(eGameCtrlButton,DIDFT_GETINSTANCE(doi.dwType))));
        }
        else if (doi.dwType & DIDFT_AXIS) {
          eGameCtrlAxis axis = eGameCtrlAxis_Last;
          if (doi.guidType == GUID_XAxis) { axis = eGameCtrlAxis_LX; }
          else if (doi.guidType == GUID_YAxis) {  axis = eGameCtrlAxis_LY; }
          else if (doi.guidType == GUID_ZAxis) {  axis = eGameCtrlAxis_LZ; }
          else if (doi.guidType == GUID_RxAxis) { axis = eGameCtrlAxis_RX; }
          else if (doi.guidType == GUID_RyAxis) { axis = eGameCtrlAxis_RY; }
          else if (doi.guidType == GUID_RzAxis) { axis = eGameCtrlAxis_RZ; }

          if (axis == eGameCtrlAxis_Last) {
            continue;
          }

          DIPROPRANGE diRange;
          diRange.diph.dwSize       = sizeof(diRange);
          diRange.diph.dwHeaderSize = sizeof(DIPROPHEADER);
          diRange.diph.dwHow        = DIPH_BYID;
          diRange.diph.dwObj        = doi.dwType; // Specify the enumerated axis
          diRange.lMin              = -1000;
          diRange.lMax              = +1000;
          hr = mpDevice->GetProperty(DIPROP_RANGE, &diRange.diph);
          if (FAILED(hr)) {
            DIERROR();
            niError(niFmt(_A("Can't get the axis[%s]'s range."),doi.tszName));
            niSafeRelease(mpDevice);
            return;
          }

          DIPROPDWORD diDeadZone;
          diDeadZone.diph.dwSize       = sizeof(diDeadZone);
          diDeadZone.diph.dwHeaderSize = sizeof(DIPROPHEADER);
          diDeadZone.diph.dwHow        = DIPH_BYID;
          diDeadZone.diph.dwObj        = doi.dwType; // Specify the enumerated axis

          hr = mpDevice->GetProperty(DIPROP_DEADZONE, &diDeadZone.diph);
          if (FAILED(hr)) {
            DIERROR();
            niError(niFmt(_A("Can't get the axis[%s]'s deadzone."),doi.tszName));
            niSafeRelease(mpDevice);
            return;
          }

          mAxisProps[axis].mMin = diRange.lMin;
          mAxisProps[axis].mMax = diRange.lMax;
          mAxisProps[axis].mRange = diRange.lMax-diRange.lMin;
          mAxisProps[axis].mfInvRange = ni::FInvert(tF32(mAxisProps[axis].mRange));
          mAxisProps[axis].mDeadZone = diDeadZone.dwData ? diDeadZone.dwData : GAMEPAD_DEADZONENORM;
          mAxisProps[axis].mDeadZone = tU32(tF32(mAxisProps[axis].mDeadZone)/10000.0f*tF32(mAxisProps[axis].mRange));
          niLog(Info,niFmt(_A("Game controller %d (%s), Axis[%d] '%s' (%s) : [%d]-[%d], %d"),
                      GetIndex(), GetName(),
                      DIDFT_GETINSTANCE(doi.dwType),
                      doi.tszName,
                      niEnumToChars(eGameCtrlAxis,axis),
                      mAxisProps[axis].mMin,
                      mAxisProps[axis].mMax,
                      mAxisProps[axis].mDeadZone));
        }
      }
    }

    niLog(Info,niFmt(_A("Game controller %d (%s) input device created."), GetIndex(), GetName()));
    mpDevice->Acquire();
  }
  ~cDInputGameCtrl() {
    Invalidate();
  }

  //// iGameCtrl /////////////////////////////
  void ZeroMembers() {
    mbIsConnected = eTrue;
    mpDevice = NULL;
    mtimeRate = eInvalidHandle;
    mnIndex = eInvalidHandle;
    memset(&mDevInst, 0, sizeof(mDevInst));
    memset(&mCaps, 0, sizeof(mCaps));
    mvObjs.clear();
    ni::MemZero((tPtr)&mState,sizeof(mState));
    ni::MemZero((tPtr)&mAxisProps[0],sizeof(mAxisProps));
  }
  tBool __stdcall IsOK() const {
    return mpDevice != NULL;
  }
  void __stdcall Invalidate() {
    if (mpDevice) {
      mpDevice->Unacquire();
      niSafeRelease(mpDevice);
    }
  }

  tU32 __stdcall GetIndex() const {
    return mnIndex;
  }

  tBool __stdcall GetIsConnected() const {
    return mbIsConnected;
  }
  const achar*  __stdcall GetName() const {
    return mDevInst.tszProductName;
  }

  tU32  __stdcall GetNumAxis() const {
    return mCaps.dwAxes;
  }
  tU32  __stdcall GetNumButtons() const {
    return mCaps.dwButtons;
  }
  tU32  __stdcall GetNumPovs() const {
    return mCaps.dwPOVs;
  }

  tBool __stdcall Update() {
    if (!IsOK())
      return eFalse;

    tBool bRet = eTrue;
    HRESULT hr = mpDevice->Poll();
    if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
      mpDevice->Acquire();
    }
    hr = mpDevice->GetDeviceState(sizeof(mState), &mState);
    if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
      bRet = eFalse;

    return bRet;
  }

  tF32  __stdcall GetButton(tU32 ulButton) {
    if (ulButton >= GetNumButtons() || ulButton >= niCountOf(mState.rgbButtons))
      return 0.0f;
    return (mState.rgbButtons[ulButton] & 0x80) ? 1.0f : 0.0f;
  }
  tF32  __stdcall GetAxis(eGameCtrlAxis axis) {
    if (axis >= niCountOf(mAxisProps))
      return 0.0f;

    tI32 lRet = 0;
    switch (axis) {
      case eGameCtrlAxis_LX : lRet = mState.lX; break;
      case eGameCtrlAxis_LY : lRet = mState.lY; break;
      case eGameCtrlAxis_LZ : lRet = mState.lZ/*?mState.lZ:32767*/; break;
      case eGameCtrlAxis_RX : lRet = mState.lRx/*?mState.lRx:32767*/; break;
      case eGameCtrlAxis_RY : lRet = mState.lRy/*?mState.lRy:32767*/; break;
      case eGameCtrlAxis_RZ : lRet = mState.lRz/*?mState.lRz:32767*/; break;
    }

    tI32 median = mAxisProps[axis].mRange/2;

    // center the value around zero
    lRet -= mAxisProps[axis].mMin;
    lRet -= median;

    // check for dead zone
    if (lRet > -mAxisProps[axis].mDeadZone && lRet < mAxisProps[axis].mDeadZone)
      return 0.0f;

    // return normalize value
    return (float(lRet) * mAxisProps[axis].mfInvRange);
  }

  tBool __stdcall GetCanVibrate() const niImpl { return eFalse; }
  void __stdcall Vibrate(const sVec2f& aSpeed) niImpl {}

  tBool __stdcall GetHasBattery() const { return eFalse; }
  tF32 __stdcall GetBatteryLevel() const { return 1.0f; }
  //// iGameCtrl /////////////////////////////

 public:
  tBool         mbIsConnected;
  DIDEVICEINSTANCE    mDevInst;
  DIDEVCAPS       mCaps;
  tDevObjVec        mvObjs;
  tU32          mnIndex;
  IDirectInputDevice8*  mpDevice;
  DIJOYSTATE2       mState;
  tU32          mtimeRate;

  struct {
    tI32  mMin;
    tI32  mMax;
    tU32    mRange;
    tF32  mfInvRange;
    tI32  mDeadZone;
  } mAxisProps[eGameCtrlAxis_Last];

  static BOOL __stdcall EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pObj, void* ptrThis) {
    cDInputGameCtrl* pGCtrl = reinterpret_cast<cDInputGameCtrl*>(ptrThis);
    pGCtrl->mvObjs.push_back(*pObj);
    return TRUE;
  }
};

static BOOL __stdcall EnumGameCtrlDeviceCallback(const DIDEVICEINSTANCE* pDevice, void* ptrThis)
{
  Ptr<cDInputGameCtrl> ptrGameCtrl = niNew cDInputGameCtrl(_pDI, _dinputCtrls.size(), pDevice);
  if (ptrGameCtrl.IsOK()) {
    _dinputCtrls.push_back(ptrGameCtrl.ptr());
  }
  return TRUE;
}
#endif
