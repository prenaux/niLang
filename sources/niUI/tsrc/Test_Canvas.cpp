#include "stdafx.h"
#include <niLang/Math/MathRect.h>

using namespace ni;

struct FCanvas {
};

struct CanvasBase : public ni::cWidgetSinkImpl<> {
  struct MainCanvas : public ni::cWidgetSinkImpl<> {
    CanvasBase* _parent;
    MainCanvas(CanvasBase* aParent) : _parent(aParent) {}
    tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
      _parent->_DrawMain(apCanvas);
      return eFalse;
    }
  };

  TEST_CONSTRUCTOR(CanvasBase) {
    niDebugFmt(("CanvasBase::CanvasBase"));
    mbAnimated = eFalse;
    mbRedrawEveryFrame = eTrue;
  }
  ~CanvasBase() {
    niDebugFmt(("CanvasBase::~CanvasBase"));
  }

  tBool mbRedrawEveryFrame;

  WeakPtr<iCanvas> _topLeftCanvas;
  WeakPtr<iCanvas> _botLeftCanvas;

  tBool __stdcall OnSinkAttached() niImpl {
    Ptr<iWidget> main = mpWidget->GetUIContext()->CreateWidgetRaw("MyCanvas",mpWidget);
    main->AddSink(niNew MainCanvas(this));
    main->SetDockStyle(eWidgetDockStyle_DockFill);
    main->SetIgnoreInput(eTrue);
    return eTrue;
  }

  tBool mbAnimated;
  tF32 mfAnimationTime;
  void _ToggleAnimation() {
    mbAnimated = !mbAnimated;
  }

  tBool __stdcall OnLeftClickDown(const sVec2f& avMousePos, const sVec2f& avNCMousePos) niImpl {
    _ToggleAnimation();
    return eFalse;
  }
  tBool __stdcall OnKeyDown(eKey aKey, tU32 aKeyMod) niImpl {
    if (aKey == eKey_Space) {
      _ToggleAnimation();
    }
    return eFalse;
  }

  virtual void _DrawMain(iCanvas* apCanvas) = 0;
};

struct Clear : public CanvasBase {
  TEST_CONSTRUCTOR_BASE(Clear,CanvasBase) {
  }

  void _DrawMain(iCanvas* apCanvas) niImpl {
    apCanvas->BlitFill(apCanvas->GetViewport().ToFloat(), 0xFF996633);
  }
};
TEST_FIXTURE_WIDGET(FCanvas,Clear);
