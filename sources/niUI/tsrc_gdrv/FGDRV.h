#pragma once
#ifndef __FGDRV_H_3426D9CE_81BA_3E4A_9716_FC36725205F6__
#define __FGDRV_H_3426D9CE_81BA_3E4A_9716_FC36725205F6__

#include "stdafx.h"
#include <niLang/Math/MathRect.h>
#include <niLang/Utils/DataTableUtils.h>
#include <niLang_ModuleDef.h>
#include <niUI/IGpu.h>

namespace ni {

struct FGDRV_WindowHandler;

struct sFGDRV_Base : public UnitTest::iTestClass {
  // test stuff
  tBool _isInteractive = eFalse;
  tU32 _testStepCount = 0;
  tBool _animated = eTrue;
  tF64 _animationTime = 0.0;
  tF32 _pingpongTime = 0.0;

  // graphics driver config
  tHStringPtr _gdrvName;
  tU32 _swapInterval = 1;
  tTextureFlags _backBufferFlags = eTextureFlags_Default|eTextureFlags_Default|eTextureFlags_RTAA4Samples;
  tClearBuffersFlags _clearBuffers = eClearBuffersFlags_ColorDepthStencil;
  tU32 _clearColor = 0;
  tF64 _clearTimer = 0.0f;

  // variables
  NN<iMessageQueue> _mq = niDeferredInit(NN<iMessageQueue>);
  NN<iOSWindow> _window = niDeferredInit(NN<iOSWindow>);
  NN<iGraphics> _graphics = niDeferredInit(NN<iGraphics>);
  NN<iGraphicsContext> _graphicsContext = niDeferredInit(NN<iGraphicsContext>);
  NN<iMessageHandler> _windowHandler = niDeferredInit(NN<iMessageHandler>);

  // UnitTest::iTestClass
  niFn(tBool) Start(UnitTest::TestResults& testResults_) niImpl;
  niFn(tBool) Step(UnitTest::TestResults& testResults_) niImpl;
  niFn(void) End(UnitTest::TestResults& testResults_) niImpl;

  // Test stuff
  virtual tBool OnInit(UnitTest::TestResults& testResults_);
  virtual tBool BeforePaint(UnitTest::TestResults& testResults_);
  virtual tBool AfterPaint(UnitTest::TestResults& testResults_);
  virtual tBool OnPaint(UnitTest::TestResults& testResults_);
};

}
#endif // __FGDRV_H_3426D9CE_81BA_3E4A_9716_FC36725205F6__
