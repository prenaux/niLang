#include "stdafx.h"
#include "TestCanvasWidgetSink.h"

using namespace ni;

struct FCanvas {
};

struct Clear : public TestCanvasWidgetSink {
  TEST_CONSTRUCTOR_BASE(Clear,TestCanvasWidgetSink) {
  }

  void PaintTest(iCanvas* apCanvas) niImpl {
    apCanvas->BlitFill(apCanvas->GetViewport().ToFloat(), 0xFF996633);
  }
};
TEST_FIXTURE_WIDGET(FCanvas,Clear);
