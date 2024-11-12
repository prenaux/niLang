#include "stdafx.h"
#include <niLang/Math/MathRect.h>
#include <niUI/Experimental/IGpu.h>

using namespace ni;

namespace {

struct FGpu {
};

struct GpuCanvasBase : public ni::cWidgetSinkImpl<> {
  struct MainGpuCanvas : public ni::cWidgetSinkImpl<> {
    WeakPtr<GpuCanvasBase> _parent;
    MainGpuCanvas(GpuCanvasBase* aParent) : _parent(aParent) {}
    tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
      QPtr<GpuCanvasBase> p = _parent;
      p->_DrawMain(apCanvas);
      return eFalse;
    }
  };

  TEST_CONSTRUCTOR(GpuCanvasBase) {
    niDebugFmt(("GpuCanvasBase::GpuCanvasBase"));
    mbAnimated = eFalse;
  }
  ~GpuCanvasBase() {
    niDebugFmt(("GpuCanvasBase::~GpuCanvasBase"));
  }

  NN<iGraphicsDriverGpu> _driverGpu = niDeferredInit(NN<iGraphicsDriverGpu>);

  tBool __stdcall OnSinkAttached() niImpl {
    QPtr<iGraphicsDriverGpu> driverGpu = mpWidget->GetUIContext()->GetGraphics()->GetDriver();
    CHECK_RET(niIsOK(driverGpu),eFalse);
    _driverGpu = AsNN(driverGpu.raw_ptr());

    Ptr<iWidget> main = mpWidget->GetUIContext()->CreateWidgetRaw("MyCanvas",mpWidget);
    main->AddSink(niNew MainGpuCanvas(this));
    main->SetDockStyle(eWidgetDockStyle_DockFill);
    main->SetIgnoreInput(eTrue);

    return eTrue;
  }

  tBool mbAnimated;
  tF64 mfAnimationTime = 0.0;
  void _ToggleAnimation() {
    mbAnimated = !mbAnimated;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niOverride {
    if (mbAnimated) {
      mfAnimationTime += ni::GetLang()->GetFrameTime();
    }
    return eFalse;
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

struct GpuClear : public GpuCanvasBase {
  TEST_CONSTRUCTOR_BASE(GpuClear,GpuCanvasBase) {
  }

  void _DrawMain(iCanvas* apCanvas) niImpl {
    apCanvas->BlitFill(apCanvas->GetViewport().ToFloat(), 0xFF996633);
  }
};
TEST_FIXTURE_WIDGET(FGpu,GpuClear);

}
