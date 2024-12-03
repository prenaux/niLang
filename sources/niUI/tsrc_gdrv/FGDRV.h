#pragma once
#ifndef __FGDRV_H_3426D9CE_81BA_3E4A_9716_FC36725205F6__
#define __FGDRV_H_3426D9CE_81BA_3E4A_9716_FC36725205F6__

#include "stdafx.h"
#include <niLang/Math/MathRect.h>
#include <niLang/Utils/DataTableUtils.h>
#include <niLang_ModuleDef.h>
#include <niUI/IGpu.h>

namespace ni {

struct FGDRV {
  // test stuff
  tBool _isInteractive = eFalse;

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

  tBool _InitTest(const achar* aTestName);
  void _Invalidate();
  virtual ~FGDRV();
};

struct FGDRV_WindowHandler : public ImplRC<iMessageHandler> {
  const tU64 _threadId;
  FGDRV* _context;

  FGDRV_WindowHandler(FGDRV* aContext);

  tU64 __stdcall GetThreadID() const niImpl;

  virtual void OnPaint();
  virtual void _Paint() final;
  virtual void __stdcall HandleMessage(const tU32 anMsg, const Var& a, const Var& b) override;
};

}
#endif // __FGDRV_H_3426D9CE_81BA_3E4A_9716_FC36725205F6__
