::Import("gui.ni");

mCanvasSink <- ::delegate(::gui.baseWidgetSink, {
  _wireframe = false
  _spring = ::gGraphics.CreateDampedSpring2(10.0,1.0)

  function OnKeyDown(w,aKey,aKeyMod) {
    ::dbg("... OnKeyDown:" aKey);
    if (aKey == ::eKey.W) {
      _wireframe = !_wireframe;
    }
  }

  function OnSetCursor(w,a,b) {
    w.uicontext.cursor = null;
  }

  function OnPaint(w,a,b) {
    local cursorPos = ::Vec2(a)
    local canvas = b
    canvas.BlitFill(w.widget_rect, w.has_focus ? 0xAADDDD : ~0)

    local material = canvas.vgmaterial
    material.SetRasterizerStates(_wireframe ?
                                 ::eCompiledStates.RS_WireframeScissor :
                                 ::eCompiledStates.RS_NoCullingFilledScissor);

    local style = canvas.vgstyle;
    style.SetFill(true);
    style.SetStrokeColor(::Vec3(0,0,0));
    style.SetStrokeWidth(3.0);
    style.SetFillColor(::Vec3(1,1,1));
    style.SetLineJoin(::eVGLineJoin.Round);
    style.SetInnerJoin(::eVGInnerJoin.Jag);

    local path = ::gGraphics.CreateVGPath()

    // heart shape
    if (false) {
      path.Clear();
      path.AddSVGPath(
        " M 10,30" +
          " A 20,20 0,0,1 50,30" +
          " A 20,20 0,0,1 90,30" +
          " Q 90,60 50,90" +
          " Q 10,60 10,30 z");
      path.Scale(::Vec2(5,5));
      canvas.DrawPath(path);
    }

    // draw the bubble
    {
      local font = w.font.CreateFontInstance(null);
      font.SetSizeAndResolution(::Vec2(20,20),20,w.GetUIContext().GetContentsScale());
      font.color = 0xFF000000;

      local text = "Hello Paul!\nWelcome to niLang, a new journey begin!\nPress [Enter] to continue.";
      local textRect = font.ComputeTextSize(::Rect(235.0,185.0,0,0), text, 0)

      local rx = 30.0, ry = 30.0;
      path.Clear();
      path.SpeechBubbleRect(
        textRect.x-rx,textRect.y-ry,
        textRect.width+(rx*2.0),textRect.height+(ry*2.0),
        rx,ry,cursorPos.x,cursorPos.y,30.0,40.0)
      canvas.DrawPath(path);

      local nextX = textRect.z - 10.0, nextY = textRect.w + 5.0;
      style.SetFill(false);
      path.Clear();
      path.MoveTo(nextX,nextY);
      path.LineTo(nextX+5.0,nextY+5.0);
      path.LineTo(nextX+10.0,nextY);
      canvas.DrawPath(path);
      style.SetFill(true);

      canvas.BlitText(font, textRect, 0, text)
    }

    // draw the cursor
    path.Clear()
    path.Circle(cursorPos.x,cursorPos.y,5.0);
    canvas.DrawPath(path);
  }
})

function OnSinkAttached(w,a,b) {
  ::dbg("... OnSinkAttached")
  local wCanvas = w.FindWidget("ID_Canvas");
  wCanvas.AddSink(mCanvasSink)
  wCanvas.style |= ::eWidgetStyle.HoldFocus
  wCanvas.SetFocus()
}

function OnSerializeFinalize(w,a,b) {
  ::dbg("... OnSerializeFinalize")
  local wCanvas = w.FindWidget("ID_Canvas");
  wCanvas.SetFocus()
}

function OnSize(w,a,b) {
  ::dbg("... OnSize")
  local wCanvas = w.FindWidget("ID_Canvas");
  ::dbg("... wCanvas: " wCanvas.absolute_rect)
}

function OnPaint(w,a,b) {
}
