#include "stdafx.h"

struct TestCanvasWidgetSink : public ni::cWidgetSinkImpl<> {
  TEST_CONSTRUCTOR(TestCanvasWidgetSink)
  {
    mbAnimated = eFalse;
  }
  ~TestCanvasWidgetSink()
  {
  }

  tBool __stdcall OnSinkAttached() niOverride
  {
    return eTrue;
  }

  tBool mbAnimated = eFalse;
  tF32 mfAnimationTime = 0.0f;
  void _ToggleAnimation()
  {
    mbAnimated = !mbAnimated;
  }

  tBool __stdcall OnLeftClickDown(const sVec2f& avMousePos,
                                  const sVec2f& avNCMousePos) niOverride
  {
    _ToggleAnimation();
    return eFalse;
  }
  tBool __stdcall OnKeyDown(eKey aKey, tU32 aKeyMod) niOverride
  {
    if (aKey == eKey_Space) {
      _ToggleAnimation();
    }
    return eFalse;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos,
                          iCanvas* apCanvas) niOverride
  {
    this->PaintTest(apCanvas);
    return eFalse;
  }

  virtual void PaintTest(iCanvas* apCanvas) = 0;
};
