// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

::Import("lang.ni")
::Import("closure.ni")
::Import("concurrent.ni")

::loading_queue <- {
  mCanvas = null
  mQueue = []
  mProgressMessage = null
  mOverlayColor = 0x66000005
  mOverlayColor2 = 0xC0101011
  mFont = null
  mMiniProgress = false

  function initialize(aGraphicsContext) {
    mCanvas = aGraphicsContext.graphics.CreateCanvas(aGraphicsContext,null)
  }

  function isDone() {
    return (mProgressMessage == null);
  }

  function draw(aContentsScale) {
    if (isDone())
      return

    aContentsScale = (aContentsScale != null) ? aContentsScale : 1.0

    local gc = mCanvas.graphics_context
    local vp = ::Rect(0,0,gc.width,gc.height)
    local progSize = ::Vec2(230,50) * aContentsScale
    local size = ::Vec2(256,256) * aContentsScale
    local pos = ::Vec2((vp.width/2-size.x/2).toint(),(vp.height/2-size.y/2).toint()-50)
    gc.fixed_states.SetOrthoMatrices(true,vp,-1,1)
    gc.viewport = vp
    gc.scissor_rect = vp

    if (mMiniProgress) {
      local subSize = 7 * aContentsScale
      local margin = ::Vec2(subSize)
      local spinnerSize = getSpinnerSize(subSize)
      local spinnerPos = ::Vec2(vp.width-spinnerSize.x-3,vp.height-spinnerSize.y-3)-margin
      mCanvas.SetDefaultMaterial(null,::eBlendMode.Translucent,0)
      mCanvas.RectA(spinnerPos-margin,spinnerPos+spinnerSize+margin,0,0x33000000)
      drawSpinner(mCanvas,spinnerPos,subSize)

      if (mFont && mProgressMessage) {
        local spinnerHeight = spinnerSize.y + margin.y*2.0 + 8
        mCanvas.BlitText(
          mFont,
          ::Rect(0, vp.height - spinnerHeight, vp.width - spinnerSize.x - margin.x*2.0 - 10, spinnerHeight),
          ::eFontFormatFlags.Right|::eFontFormatFlags.CenterV|::eFontFormatFlags.Border,
          mProgressMessage)
      }
    }
    else {
      local subSize = 7 * aContentsScale
      local margin = ::Vec2(2,3) * aContentsScale
      local spinnerSize = getSpinnerSize(subSize)
      local spinnerPos = ::Vec2(vp.width/2.0-spinnerSize.x/2.0,vp.height/2.0-spinnerSize.y/2.0)

      mCanvas.SetDefaultMaterial(null,::eBlendMode.Translucent,0)
      mCanvas.RectA(::Vec2(0,0),vp.size,0,mOverlayColor)
      mCanvas.RectA(::Vec2(0,spinnerPos.y-(spinnerSize.y*2.5)),
                ::Vec2(vp.width,spinnerPos.y+(spinnerSize.y*3.5)),0,
                mOverlayColor2)

      if (mFont && mProgressMessage) {
        local r = ::Rect()
        r.top_left = ::Vec2(0, spinnerPos.y + spinnerSize.y * 0.5);
        r.size = ::Vec2(vp.width, spinnerSize.y * 1.5);
        mCanvas.BlitText(mFont,r,
                     ::eFontFormatFlags.CenterH|::eFontFormatFlags.CenterV|::eFontFormatFlags.Border,
                     mProgressMessage)
      }
      drawSpinner(mCanvas,spinnerPos - ::Vec2(0,spinnerSize.y / 2.0),subSize)
    }

    mCanvas.Flush()
    return mQueue.empty() ? false : true
  }

  function getSpinnerSize(subSize) {
    return ::Vec2(3*(subSize+1),3*(subSize+1))
  }
  function drawSpinner(aCanvas,spinnerPos,subSize) {
    aCanvas.SetDefaultMaterial(null,::eBlendMode.NoBlending,0)
    local xpos = spinnerPos.x
    for (local x = 0; x < 3; ++x) {
      local ypos = spinnerPos.y
      for (local y = 0; y < 3; ++y) {
        local p = ::Vec2(xpos,ypos)
        local f = ::gMath.Max(0.3,::gMath.RandFloat())
        aCanvas.RectA(p,p+::Vec2(subSize,subSize),0,
                      ::RGBA(f,f,f,1).toint())
        ypos += subSize+1
      }
      xpos += subSize+1
    }
  }

  function queue(aFunc,aThis) {
    if (typeof aFunc == "array") {
      foreach (f in aFunc) {
        queue(f)
      }
    }
    else {
      local _this = this;
      switch (typeof(aFunc)) {
        case "null": {
          ::concurrent.mainRun(function() : (_this,aFunc) {
            _this.mProgressMessage = null
            _this.unregisterLogSink()
          })
          break;
        }
        case "string": {
          ::concurrent.mainRun(function() : (_this,aFunc) {
            switch (aFunc) {
              case "$MiniProgress$": {
                _this.mMiniProgress = true
                break;
              }
              case "$FullProgress$": {
                _this.mMiniProgress = false
                break;
              }
              default: {
                _this.mProgressMessage = aFunc
                _this.registerLogSink()
                break;
              }
            }
            return true
          })
          ::concurrent.mainRun(function() { ::SleepMs(100); });
          break;
        }
        default: {
          ::concurrent.mainRun(aFunc,aThis)
          break;
        }
      }
    }
  }

  mLastUpdate = ::gLang.TimerInSeconds()
  mInLogSink = false
  mLogSinkRegistered = false
  mLogUpdateTime = 0.5

  ///////////////////////////////////////////////
  function registerLogSink() {
    if (mLogSinkRegistered)
      return
    mLogSinkRegistered = true
    ::gLang.system_message_handlers.AddSink(this)
  }
  function unregisterLogSink() {
    if (!mLogSinkRegistered)
      return
    mLogSinkRegistered = false
    ::gLang.system_message_handlers.RemoveSink(this)
  }
  function GetThreadID() {
    return ::GetMainThreadID();
  }
  function HandleMessage(aMsg,aA,aB) {
    if (aMsg != ::eSystemMessage.Log)
      return
    if (mInLogSink || !::gWindow)
      return
    mInLogSink = true
    local curTime = ::gLang.TimerInSeconds()
    if ((curTime - this.mLastUpdate) > mLogUpdateTime) {
      ::gWindow.RedrawWindow()
      this.mLastUpdate = curTime
    }
    mInLogSink = false
  }
}
