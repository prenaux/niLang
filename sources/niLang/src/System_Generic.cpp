#include "API/niLang/Types.h"
#include "API/niLang/Utils/UnknownImpl.h"
#include "API/niLang/STL/utils.h"
#include "API/niLang/Math/MathRect.h"
#include "API/niLang/IOSWindow.h"
#if defined niPosix && !defined __MINGW32__
#  include <unistd.h>
#else
#  include <process.h> // for getpid()
#endif

#define USE_MOTION_EVENTS

using namespace ni;

#include "GameCtrlAcc.h"

struct sGenericStaticManager {
  Ptr<ni::cGameCtrlAcc> mptrGameCtrlAcc;
  sGenericStaticManager() {
    mptrGameCtrlAcc = niNew cGameCtrlAcc();
  }
};
static sGenericStaticManager _StaticManager;

namespace ni {
cGameCtrlAcc* __stdcall GetGenericGameCtrlAcc() {
  return _StaticManager.mptrGameCtrlAcc;
}
}

//--------------------------------------------------------------------------------------------
//
//  cOSWindowGeneric implementation
//
//--------------------------------------------------------------------------------------------
class cOSWindowGeneric : public ni::ImplRC<ni::iOSWindow,ni::eImplFlags_Default,iOSWindowGeneric>
{
  niBeginClass(cOSWindowGeneric);

 public:
  cOSWindowGeneric() {
    mbRequestedClose = eFalse;
    mptrMT = tMessageHandlerSinkLst::Create();
    mRect = sRecti(0,0,800,600);
    mnWindowStyle = 0;
    mfContentsScale = 1.0f;
    this->SetTitle("niApp");
    this->SetStyle(eOSWindowStyleFlags_FullScreen);
  }
  ~cOSWindowGeneric() {
    Invalidate();
  }

  /////////////////////////////////////////////////////////////////
  virtual iOSWindow* __stdcall GetParent() const {
    return NULL;
  }

  /////////////////////////////////////////////////////////////////
  virtual void __stdcall Invalidate() {
    mptrMT = NULL;
  }

  /////////////////////////////////////////////////////////////////
  virtual tBool __stdcall IsOK() const {
    return mptrMT.IsOK();
  }

  /////////////////////////////////////////////////////////////////
  virtual void __stdcall SetClientAreaWindow(tIntPtr aHandle) {
  }
  virtual tIntPtr __stdcall GetClientAreaWindow() const {
    return 0;
  }

  /////////////////////////////////////////////////////////////////
  virtual tIntPtr __stdcall GetPID() const {
    return getpid();
  }

  /////////////////////////////////////////////////////////////////
  virtual tIntPtr __stdcall GetHandle() const {
    return 0;
  }
  virtual tBool __stdcall GetIsHandleOwned() const {
    return eTrue;
  }

  /////////////////////////////////////////////////////////////////
  virtual void __stdcall ActivateWindow() {
  }
  virtual tBool __stdcall GetIsActive() const {
    return eTrue;
  }

  /////////////////////////////////////////////////////////////////
  virtual tBool __stdcall SwitchIn(tU32 anReason) {
    _PostMessage(eOSWindowMessage_SwitchIn,anReason);
    return eTrue;
  }
  virtual tBool __stdcall SwitchOut(tU32 anReason) {
    _PostMessage(eOSWindowMessage_SwitchIn,anReason);
    return eTrue;
  }

  /////////////////////////////////////////////////////////////////
  virtual void __stdcall SetTitle(const achar* aaszTitle) {
    if (mstrTitle != aaszTitle) {
      mstrTitle = aaszTitle;
    }
  }
  virtual const achar* __stdcall GetTitle() const {
    return mstrTitle.Chars();
  }

  /////////////////////////////////////////////////////////////////
  virtual void __stdcall SetStyle(tOSWindowStyleFlags aStyle) {
    if (mnWindowStyle != aStyle) {
      mnWindowStyle = aStyle;
    }
  }
  virtual tOSWindowStyleFlags __stdcall GetStyle() const {
    return mnWindowStyle;
  }

  /////////////////////////////////////////////////////////////////
  virtual void __stdcall SetShow(tOSWindowShowFlags aStyle) {
  }
  virtual tOSWindowShowFlags __stdcall GetShow() const {
    return 0;
  }

  /////////////////////////////////////////////////////////////////
  virtual void __stdcall SetZOrder(eOSWindowZOrder aZOrder) {
  }
  virtual eOSWindowZOrder __stdcall GetZOrder() const {
    return eOSWindowZOrder_Normal;
  }

  /////////////////////////////////////////////////////////////////
  virtual void __stdcall SetSize(const sVec2i& avSize) {
    mRect.SetSize(avSize);
  }
  virtual sVec2i __stdcall GetSize() const {
    return mRect.GetSize();
  }
  virtual void __stdcall SetPosition(const sVec2i& avPos) {
    mRect.SetTopLeft(avPos);
  }
  virtual sVec2i __stdcall GetPosition() const {
    return *(sVec2i*)&mRect.x;
  }
  virtual void __stdcall SetRect(const sRecti& aRect) {
    mRect = aRect;
  }
  virtual sRecti __stdcall GetRect() const {
    return mRect;
  }

  /////////////////////////////////////////////////////////////////
  virtual void __stdcall SetClientSize(const sVec2i& avSize) {
    SetSize(avSize);
  }
  virtual sVec2i __stdcall GetClientSize() const {
    return GetSize();
  }

  /////////////////////////////////////////////////////////////////
  virtual void __stdcall SetContentsScale(tF32 afContentsScale) {
    mfContentsScale = afContentsScale;
  }

  /////////////////////////////////////////////////////////////////
  virtual tF32 __stdcall GetContentsScale() const niImpl {
    return mfContentsScale;
  }

  /////////////////////////////////////////////////////////////////
  virtual tMessageHandlerSinkLst* __stdcall GetMessageHandlers() const {
    return mptrMT;
  }

  /////////////////////////////////////////////////////////////////
  virtual tBool __stdcall UpdateWindow(tBool abBlockingMessages) {
    return eTrue;
  }

  /////////////////////////////////////////////////////////////////
  virtual void __stdcall CenterWindow() {
  }

  /////////////////////////////////////////////////////////////////
  virtual tBool __stdcall GetRequestedClose() const {
    return mbRequestedClose;
  }
  virtual void __stdcall SetRequestedClose(tBool abRequested) {
    mbRequestedClose = abRequested;
  }

  /////////////////////////////////////////////////////////////////
  virtual void __stdcall SetCursor(eOSCursor aCursor) {
  }
  virtual eOSCursor __stdcall GetCursor() const {
    return eOSCursor_None;
  }

  /////////////////////////////////////////////////////////////////
  virtual tBool __stdcall InitCustomCursor(tIntPtr aID, tU32 anWidth, tU32 anHeight, tU32 anPivotX, tU32 anPivotY, const tU32* apData) {
    return eFalse;
  }
  virtual tIntPtr __stdcall GetCustomCursorID() const {
    return 0;
  }
  virtual void __stdcall SetCursorPosition(const sVec2i& avCursorPos) {
  }
  virtual sVec2i __stdcall GetCursorPosition() const {
    return sVec2i::Zero();
  }
  virtual void __stdcall SetCursorCapture(tBool abCapture) {
  }
  virtual tBool __stdcall GetCursorCapture() const {
    return eFalse;
  }
  virtual void __stdcall SetCursorClip(const sRecti& aRect) {
  }
  virtual sRecti __stdcall GetCursorClip() const {
    return sRecti::Null();
  }
  virtual tBool __stdcall GetIsCursorOverClient() const {
    return eTrue;
  }

  /////////////////////////////////////////////////////////////////
  virtual void __stdcall TryClose() {
  }

  /////////////////////////////////////////////////////////////////
  virtual void __stdcall SetDropTarget(tBool abDropTarget) {
  }
  virtual tBool __stdcall GetDropTarget() const {
    return eFalse;
  }

  /////////////////////////////////////////////////////////////////
  virtual void __stdcall SetBackgroundUpdate(tBool abBackgroundUpdate) {
  }
  virtual tBool __stdcall GetBackgroundUpdate() const {
    return eFalse;
  }

  /////////////////////////////////////////////////////////////////
  virtual tBool __stdcall RedrawWindow() {
    _PostMessage(eOSWindowMessage_Paint);
    return eTrue;
  }

  /////////////////////////////////////////////////////////////////
  virtual tU32 __stdcall GetMonitor() const {
    return 0;
  }

  /////////////////////////////////////////////////////////////////
  virtual tBool __stdcall SetFullScreen(tU32 anMonitor) {
    if (anMonitor != eInvalidHandle) {
      SetStyle(eOSWindowStyleFlags_FullScreen);
    }
    else {
      SetStyle(eOSWindowStyleFlags_Regular);
    }
    return eTrue;
  }
  virtual tU32 __stdcall GetFullScreen() const {
    return (niFlagIs(mnWindowStyle,eOSWindowStyleFlags_NoTitle) ||
            niFlagIs(mnWindowStyle,eOSWindowStyleFlags_Overlay)) ?
        0 : eInvalidHandle;
  }

  /////////////////////////////////////////////////////////////////
  virtual tBool __stdcall GetIsMinimized() const {
    return eFalse;
  }
  virtual tBool __stdcall GetIsMaximized() const {
    return eFalse;
  }

  /////////////////////////////////////////////////////////////////
  virtual void __stdcall SetFocus() {
  }
  virtual tBool __stdcall GetHasFocus() const {
    return eTrue;
  }

  /////////////////////////////////////////////////////////////////
  virtual void __stdcall SetRefreshTimer(tF32 afTime) {
  }
  virtual tF32 __stdcall GetRefreshTimer() const {
    return 0.0f;
  }

  /////////////////////////////////////////////////////////////////
  virtual void __stdcall Clear() {
  }
  virtual void __stdcall SetClearColor(tU32 anColor) {
  }
  virtual tU32 __stdcall GetClearColor() const {
    return 0;
  }
  virtual void __stdcall SetText(const achar* aaszText) {
  }
  virtual const achar* __stdcall GetText() const {
    return AZEROSTR;
  }
  virtual void __stdcall SetTextColor(tU32 anColor) {
  }
  virtual tU32 __stdcall GetTextColor() const {
    return 0;
  }
  virtual void __stdcall SetFontName(const achar* aaszFont) {
  }
  virtual const achar* __stdcall GetFontName() const {
    return AZEROSTR;
  }
  virtual void __stdcall SetFontSize(const tU32 anSize) {
  }
  virtual tU32 __stdcall GetFontSize() const {
    return 12;
  }
  virtual void __stdcall SetFontWeight(const tU32 anWeight) {
  }
  virtual tU32 __stdcall GetFontWeight() const {
    return 0;
  }
  virtual void __stdcall SetLogo(const achar* aaszName) {
  }
  virtual const achar* __stdcall GetLogo() const {
    return AZEROSTR;
  }

  virtual tIntPtr __stdcall GetParentHandle() const {
    return 0;
  }
  virtual tU32 __stdcall IsParentWindow(tIntPtr aHandle) const {
    return 0;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall GenericInputString(const achar* aaszString) {
    StrCharIt it(aaszString);
    while (!it.is_end()) {
      tU32 c = it.next();
      _PostMessage(eOSWindowMessage_KeyChar,c);
    }
  }

  ///////////////////////////////////////////////
  virtual void __stdcall GenericSendMessage(tU32 anMsg, const Var& avarA, const Var& avarB) {
    _PostMessage((eOSWindowMessage)anMsg,avarA,avarB);
  }

  ///////////////////////////////////////////////
  __forceinline tBool _PostMessage(eOSWindowMessage aMsg, const Var& avarA = niVarNull, const Var& avarB = niVarNull) {
    return ni::SendMessages(mptrMT,aMsg,avarA,avarB);
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall AttachGraphicsAPI(iOSGraphicsAPI* apAPI) {
    mptrAttachedGraphicsAPI = apAPI;
    return mptrAttachedGraphicsAPI.IsOK();
  }
  virtual iOSGraphicsAPI* __stdcall GetGraphicsAPI() const {
    return mptrAttachedGraphicsAPI;
  }

 private:
  Ptr<iOSGraphicsAPI>         mptrAttachedGraphicsAPI;
  Ptr<tMessageHandlerSinkLst> mptrMT;
  sRecti                      mRect;
  cString                     mstrTitle;
  tOSWindowStyleFlags         mnWindowStyle;
  tF32                        mfContentsScale;
  tBool                       mbRequestedClose;

  niEndClass(cOSWindowGeneric);
};

namespace ni {
niExportFunc(ni::iOSWindow*) CreateGenericWindow() {
  return niNew cOSWindowGeneric();
}
niExportFunc(iUnknown*) New_niLang_OSWindowGeneric(const Var&, const Var&) {
  return CreateGenericWindow();
}
}

//--------------------------------------------------------------------------------------------
//
//  Generic system platform stuff
//
//--------------------------------------------------------------------------------------------
#if defined niAndroid || defined niIOS || defined niJSCC || defined niLinux || defined niQNX

#include "Lang.h"

///////////////////////////////////////////////
void cLang::_PlatformStartup() {
}

///////////////////////////////////////////////
tU32 __stdcall cLang::GetNumMonitors() const {
  return 0;
}
tU32 __stdcall cLang::GetMonitorIndex(tIntPtr aHandle) const {
  return eInvalidHandle;
}
tIntPtr __stdcall cLang::GetMonitorHandle(tU32 anIndex) const {
  return eInvalidHandle;
}
const achar* __stdcall cLang::GetMonitorName(tU32 anIndex) const {
  return AZEROSTR;
}
sRecti __stdcall cLang::GetMonitorRect(tU32 anIndex) const {
  return sRecti::Null();
}
tOSMonitorFlags __stdcall cLang::GetMonitorFlags(tU32 anIndex) const {
  return 0;
}

#if !defined niQNX && !defined niJSCC
///////////////////////////////////////////////
iOSWindow* __stdcall cLang::CreateWindow(iOSWindow* apParent, const achar* aaszTitle, const sRecti& aRect, tOSWindowCreateFlags aCreate, tOSWindowStyleFlags aStyle) {
  return 0;
}
#endif

iOSWindow* __stdcall cLang::CreateWindowEx(tIntPtr aOSWindowHandle, tOSWindowCreateFlags aCreate) {
  return NULL;
}
eOSMessageBoxReturn __stdcall cLang::MessageBox(iOSWindow* apParent, const achar* aaszTitle, const achar* aaszText, tOSMessageBoxFlags aFlags)
{
  cString strTitle = niIsStringOK(aaszTitle)?aaszTitle:"Message";
  cString strText = niIsStringOK(aaszText)?aaszText:_A("");
  niDebugFmt((_A("--- %s ---\n%s\n"),
              strTitle.Chars(),
              strText.Chars()));
  return eOSMessageBoxReturn_Yes;
}
cString __stdcall cLang::OpenFileDialog(iOSWindow* aParent, const achar* aTitle, const achar* aFilter, const achar* aInitDir) {
  niError("Not implemented");
  return "<NOT IMPLEMENTED>";
}
cString __stdcall cLang::SaveFileDialog(iOSWindow* aParent, const achar* aTitle, const achar* aFilter, const achar* aInitDir) {
  niError("Not implemented");
  return "<NOT IMPLEMENTED>";
}
cString __stdcall cLang::PickDirectoryDialog(iOSWindow* aParent, const achar* aTitle, const achar* aInitDir) {
  niError("Not implemented");
  return "<NOT IMPLEMENTED>";
}

///////////////////////////////////////////////
tU32 __stdcall cLang::GetNumGameCtrls() const {
  return 0;
}
iGameCtrl* __stdcall cLang::GetGameCtrl(tU32 aulIdx) const {
  return NULL;
}
#endif
