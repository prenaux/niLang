// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("gui.ni")
::gui._initialize(); // make sure we have the current ::gGraphics, etc...

mCurrentDop <- invalid
mUI <- null
mImageMap <- null
mwTexturesTab <- null

mTexViewBaseSink <- {
  mTexZoom = 1.0
  mwZoomLabel = null

  function _initialize(w) {
    mwZoomLabel = w.FindWidget("ID_ZoomLabel")
  }

  // zoom in/plus
  function zoomIn() {
    if (mTexZoom >= 8) {
      mTexZoom += 1
    }
    else if (mTexZoom >= 1) {
      mTexZoom += 0.5
    }
    else {
      mTexZoom += 0.1
    }
  }

  // zoom out/minus
  function zoomOut() {
    if (mTexZoom >= 9) {
      mTexZoom -= 1
    }
    else if (mTexZoom >= 1) {
      mTexZoom -= 0.5
    }
    else {
      mTexZoom -= 0.1
    }
  }

  function zoom(aSteps) {
    if (aSteps < 0) {
      // zoom out
      local c = -aSteps
      while (c--) {
        zoomOut()
        if (mTexZoom < 0.1) {
          mTexZoom = 0.1
          break
        }
      }
      ::printdebugln("ZOOM OUT: " + aSteps + ", " + mTexZoom)
    }
    else {
      // zoom in
      local c = aSteps
      while (c--) {
        zoomIn()
        if (mTexZoom > 16.0) {
          mTexZoom = 16.0
          break
        }
      }
      ::printdebugln("ZOOM IN: " + aSteps + ", " + mTexZoom)
    }

    updateZoomLabel()
  }

  ///////////////////////////////////////////////
  function OnSetCursor(aWidget,aPA,aPB) {
    ::gUIContext.cursor = aWidget.FindSkinCursor("","","Zoom")
  }

  ///////////////////////////////////////////////
  function OnLeftClick(w,a,b) {
    this.zoom( 1)
    return true
  }

  ///////////////////////////////////////////////
  function OnRightClick(w,a,b) {
    this.zoom(-1)
    return true
  }

  ///////////////////////////////////////////////
  function OnNCWheel(w,a,b) {
    return OnWheel(w,a,b)
  }

  ///////////////////////////////////////////////
  function OnWheel(w,a,b) {
    this.zoom(a<0?-1:1)
    return true
  }

  ///////////////////////////////////////////////
  function updateZoomLabel() {
    local text = (mTexZoom*100.0).toint()+"%"
    if (mwZoomLabel) {
      mwZoomLabel.text = text
    }
  }
}.SetDelegate(::gui.baseWidgetSink)

//////////////////////////////////////////////////////////////////////////////////////////////
mTexViewSink <- {
  mTexture = null

  function OnPaint(aWidget,aPA,aCanvas) {
    local texture = mTexture
    if (texture) {
      local tw = texture.width*mTexZoom
      local th = texture.height*mTexZoom
      local cs = ::Vec2(tw+4,th*2+6);
      aWidget.client_size = ::Vec2(tw+4,th*2+6);
      aWidget.font.color = ::RGBA(1,1,1,1).toint()
      try {
        aCanvas.color_a = ::gGraphics.color4_from_name["darkorange"].toint()
        aCanvas.Rect(::Vec2(0,0),aWidget.client_size,0)

        aCanvas.color_a = 0xFFFFFFFF
        local r = ::Rect(2,2,tw,th)
        aCanvas.SetDefaultMaterial(texture,::eBlendMode.NoBlending,::eCompiledStates.SS_SmoothClamp)
        aCanvas.Rect(r.top_left,r.bottom_right,0)

        r = ::Rect(2,th+4,tw,th)
        aCanvas.SetDefaultMaterial(texture,::eBlendMode.Translucent,::eCompiledStates.SS_SmoothClamp)
        aCanvas.Rect(r.top_left,r.bottom_right,0)
      }
      catch (e) {}
    }
    else {
      aWidget.client_size = aWidget.size
    }
    return false
  }
}.SetDelegate(mTexViewBaseSink)

//////////////////////////////////////////////////////////////////////////////////////////////
mRTViewSink <- {
  function OnPaint(aWidget,aPA,aCanvas) {
    local dopc = ::gGraphics.draw_op_capture
    if (dopc) {
      local i = dopc.capture_stop_at
      if (i != invalid) {
        local cdo = dopc.captured_draw_op[i]
        local ct = dopc.captured_draw_op_time[i]
        local cc = dopc.captured_draw_op_context[i]
        if (cc) {
          local texture = cc.render_target[0]
          local tw = texture.width*mTexZoom
          local th = texture.height*mTexZoom
          aWidget.client_size = ::Vec2(tw+4,th*2+6);
          aWidget.font.color = ::RGBA(1,1,1,1).toint()
          try {
            aCanvas.color_a = ::gGraphics.color4_from_name["darkorange"].toint()
            aCanvas.Rect(::Vec2(0,0),aWidget.client_size,0)

            aCanvas.color_a = 0xFFFFFFFF
            local r = ::Rect(2,2,tw,th)
            aCanvas.SetDefaultMaterial(texture,::eBlendMode.NoBlending,::eCompiledStates.SS_SmoothClamp)
            aCanvas.Rect(r.top_left,r.bottom_right,0)

            r = ::Rect(2,th+4,tw,th)
            aCanvas.SetDefaultMaterial(texture,::eBlendMode.Translucent,::eCompiledStates.SS_SmoothClamp)
            aCanvas.Rect(r.top_left,r.bottom_right,0)
          }
          catch (e) {
          }
          return false
        }
      }
    }

    aWidget.client_size = aWidget.size
    return false
  }
}.SetDelegate(mTexViewBaseSink)

function getTexName(t) {
  local ctxt = "[" + t.device_resource_name + "]"
  {
    local glTexture = t.QueryInterface("iGLTexture")
    if (glTexture) {
      ctxt += ::format(" (GL: %d, FBO: %d)",
                       glTexture.glhandle,glTexture.glfbohandle)
    }
  }
  ctxt += ::format(", %dx%dx%d, %d, %s",
                   t.width,t.height,t.depth,
                   t.num_mip_maps,
                   t.?pixel_format.?format)
  return ctxt
}

//////////////////////////////////////////////////////////////////////////////////////////////
function OnSinkAttached(w,a,b) {
  local stopAt = w.FindWidget("ID_StopAt")
  w.style |= ::eWidgetStyle.Free
  w.form_frame_flags &= ~::eRectFrameFlags.Center;
  local dopc = ::gGraphics.draw_op_capture
  if (dopc) {
    local stopAtEnabled = w.FindWidget("ID_StopAtEnabled")
    if (stopAtEnabled.check) {
      dopc.capture_stop_at = 0
      if (stopAt) {
        stopAt.scroll_range = ::Vec2(0,dopc.num_captured)
        stopAt.page_size = 1
        stopAt.scroll_position = dopc.capture_stop_at
      }
    }
  }
  w.Place(
    ::Rect(0.649,0.295,0.5,0.7),
    0,
    ::Vec4(0,0,0,0))

  w.FindWidget("ID_Tabs").active_page_index = 0

  local pb = w.FindWidget("ID_PBContext")
  if (pb)
    pb.data_table = ::gLang.CreateDataTable("States")
  local pb = w.FindWidget("ID_PBDrawOp")
  if (pb)
    pb.data_table = ::gLang.CreateDataTable("States")

  mwTexturesTab = w.FindWidget("Texs")
  if (mwTexturesTab) {
    // ::gui.setIconSmall(mwTexturesTab,"ID_RefreshList","reload") // commented, as we dont bundle the icon anymore
    local wTexView = mwTexturesTab.FindWidget("ID_TextureView")
    mTexViewSink._initialize(mwTexturesTab)
    mTexViewSink.updateZoomLabel()
    wTexView.AddSink(mTexViewSink)
    fillTextureList(false)
    updateCurrentTexture()
  }

  local wRTView = w.FindWidget("ID_RT")
  if (wRTView) {
    mRTViewSink._initialize(wRTView)
    mRTViewSink.updateZoomLabel()
    wRTView.AddSink(mRTViewSink);
  }

  if (stopAt)
    stopAt.SetFocus()
}

function OnSetFocus(w,a,b) {
  local stopAt = w.FindWidget("ID_StopAt")
  if (w.visible && stopAt)
    stopAt.SetFocus()
}

function OnVisible(w,a,b) {
  if (a) {
    OnSetFocus(w,null,null)
  }
}

function OnDestroy(w,a,b) {
  ::gUIContext.draw_op_capture = false
}

function OnNCPaint(w,a,b) {
  local fX = w.client_position.x - w.padding.x
  local fY = w.client_position.y - w.padding.y
  local fW = w.client_size.x + w.padding.x + w.padding.z
  local fH = w.client_size.y + w.padding.y + w.padding.w
  b.BlitFillAlpha(
    ::Rect(fX,fY,fW,fH),
    ::RGBA(0.0,0.05,0.12,0.85).toint())
  return false
}

function OnPaint(w,a,b) {
  local dopc = ::gGraphics.draw_op_capture
  local txt = ""

  local stopAtEnabled = w.FindWidget("ID_StopAtEnabled")
  if (stopAtEnabled.check) {
    local stopAt = w.FindWidget("ID_StopAt")
    if (dopc.num_captured != stopAt.scroll_range.y.toint()) {
      stopAt.scroll_range = ::Vec2(0,dopc.num_captured)
    }
  }
  local i = dopc.capture_stop_at
  if (i != invalid) {
    if (mCurrentDop != i) {
      local cdo = dopc.captured_draw_op[i]
      local ct = dopc.captured_draw_op_time[i]
      local cc = dopc.captured_draw_op_context[i]

      local pb = w.FindWidget("ID_PBContext")
      if (pb)
      {
        local dt = ::gLang.CreateDataTableWriteStack(pb.data_table)
        if (!cc) {
          dt.top.Clear()
        }
        else {
          dt.vec4.scissor = cc.scissor_rect
          dt.vec4.viewport = cc.viewport
          dt.matrix.camera_view_matrix = cc.fixed_states.camera_view_matrix
          dt.matrix.camera_projection_matrix = cc.fixed_states.camera_projection_matrix
          dt.matrix.view_matrix = cc.fixed_states.view_matrix
          dt.matrix.projection_matrix = cc.fixed_states.projection_matrix

          function writeStates(cc,states,intf,name) : (dt) {
            if (dt.Push(name)) {
              if (cc && cc[states]) {
                local s = cc[states]
                if (typeof(s) == "int") {
                  s = ::gGraphics[?"compiled_"+states][?s]
                }
                if (s) {
                  s = ::LINT_AS_TYPE("iSamplerStates", s);
                  s.SerializeDataTable(
                    dt.top,
                    ::eSerializeFlags.Write|::eSerializeFlags.TypeInfoMetadata)
                }
                else {
                  dt.top.Clear()
                  dt.string.NA = "CantFindCompiled"
                }
              }
              else {
                dt.top.Clear()
                dt.string.NA = "NA"
              }
              dt.top.name = name
              dt.Pop()
            }
          }

          writeStates(cc,
                      "fixed_states",
                      "iFixedStates",
                      "FSContext")

          if (cdo && cdo.material) {
            local m = cdo.material
            writeStates(m,
                        "rasterizer_states",
                        "iRasterizerStates",
                        "RSMaterial")
            writeStates(m,
                        "depth_stencil_states",
                        "iDepthStencilStates",
                        "DSMaterial")
            if (dt.Push("MatMaterial")) {
              local m = m
              if (m) {
                dt.top.Clear()
                dt.SetFlags("Flags",::eMaterialFlags,m.flags)
                dt.SetEnum("BlendMode",::eBlendMode,m.blend_mode)
                foreach (i in ::eMaterialChannel.Last) {
                  local t = { sampler_states = m.channel_sampler_states[i] }
                  writeStates(t,
                              "sampler_states",
                              "iSamplerStates",
                              "SamplerStates"+i)
                }
              }
              else {
                dt.top.Clear()
                dt.string.NA = "NA"
              }
              dt.Pop();
            }
          }
        }
      }

      local pb = w.FindWidget("ID_PBDrawOp")
      if (pb)
      {
        local dt = ::gLang.CreateDataTableWriteStack(pb.data_table)
        if (!cdo) {
          dt.top.Clear()
        }
        else {
          local mat = cdo.material
          if (dt.Push("Material")) {
            if (mat) {
              mat.Serialize(
                ::eSerializeMode.Write,
                dt.top,
                "")
            }
            dt.Pop()
          }
        }
      }
      txt += "* Index: " + (i+1) + "/" + (dopc.num_captured) + ", Time: " + ::format("%.3fms\n",ct*1000.0)

      function getShaderName(cc,unit) {
        local sh = cc.shader[unit]
        if (sh) return sh.device_resource_name
        else return "NA"
      }
      function getMatText(m) {
        if (!m) {
          return "  - Material: None\n"
        }
        local txt = "";
        txt += "  - Material: " + m.name + " (" + ::format("%X",m.ToIntPtr()) + ")\n"
        txt += "  -- Flags: " + ::FlagsToString(m.flags,::eMaterialFlags) + "\n"
        txt += "  -- BlendModes: " + ::EnumToString(m.blend_mode,::eBlendMode) + "\n"
        txt += "  -- RS: " + ::EnumToString(m.rasterizer_states,::eCompiledStates) + "\n"
        txt += "  -- DS: " + ::EnumToString(m.depth_stencil_states,::eCompiledStates) + "\n"
        txt += "  -- Shader[Vertex]: " + getShaderName(m,::eShaderUnit.Vertex) + "\n"
        txt += "  -- Shader[Pixel]: " + getShaderName(m,::eShaderUnit.Pixel) + "\n"
        for (local i = 0; i < 16; ++i) {
          local ctxt = "";
          ctxt += "  -- ["
          ctxt += ::EnumToString(i,::eMaterialChannel)
          ctxt += "]: "
          if (m.channel_color[i] == ::RGBA(1,1,1,1)) {
            ctxt += "(One) "
          }
          else if (m.channel_color[i] == ::RGBA(0,0,0,0)) {
            ctxt += "(Zero) "
          }
          else {
            ctxt += "("
            ctxt += m.channel_color[i]
            ctxt += ") "
          }
          local t = m.channel_texture[i]
          if (t) {
            ctxt += getTexName(t)
          }
          else {
            ctxt += "No Texture"
          }
          ctxt += " - SS: " + ::EnumToString(m.channel_sampler_states[i],::eCompiledStates)
          txt += ctxt + "\n"
        }
        return txt;
      }
      local function appendRTName(txt,cc,i) {
        local rt = cc.render_target[i]
        if (rt) {
          local texName = getTexName(rt);
          return txt + "  - RT["+i+"]: " + texName + "\n"
        }
        return txt
      }
      function getDSName(cc) {
        local txt = "";
        local ds = cc.depth_stencil
        if (ds) {
          txt += getTexName(ds)
        }
        else {
          txt = "NA"
        }
        return txt
      }

      if (cc) {
        txt += "* Context:\n"
        txt += "  - scissor: " + cc.scissor_rect + "\n"
        txt += "  - viewport: " + cc.viewport + "\n"
        txt = appendRTName(txt,cc,0);
        txt = appendRTName(txt,cc,1);
        txt = appendRTName(txt,cc,2);
        txt = appendRTName(txt,cc,3);
        txt += "  - DS: " + getDSName(cc) + "\n"
        txt += getMatText(cc.material)
      }
      if (cdo) {
        txt += "* DrawOperation:\n"
        txt += "  - Matrix: " + cdo.matrix + "\n"
        txt += "  - PrimType: " + ::EnumToString(cdo.primitive_type,::eGraphicsPrimitiveType) + "\n"
        if (cdo.local_bounding_volume) {
          local bv = cdo.local_bounding_volume;
          txt += "  - LocalBV: " + bv.min + " : " + bv.max + "\n"
        }
        else {
          txt += "  - LocalBV: None\n"
        }
        if (cdo.bounding_volume) {
          local bv = cdo.bounding_volume;
          txt += "  - WorldBV: " + bv.min + " : " + bv.max + "\n"
        }
        else {
          txt += "  - WorldBV: None\n"
        }
        if (cdo.vertex_array) {
          txt += "  - VA:\t" +
            ::format("%X",cdo.vertex_array.ToIntPtr()) + " - " +
            ::EnumToString(cdo.vertex_array.usage,::eArrayUsage) + "\n"
        }
        if (cdo.index_array) {
          txt += "  - IA:\t" +
            ::format("%X",cdo.index_array.ToIntPtr()) + " - " +
            ::EnumToString(cdo.index_array.usage,::eArrayUsage) + "\n"
        }
        if (cdo.vertex_array) {
          txt += "  - Vertices: " + cdo.vertex_array.num_vertices + "\n"
          txt += "  - FVF: " + ::FlagsToString(cdo.vertex_array.fvf,::eFVF)
        }
        else {
          txt += "  - Vertices: None\n"
        }
        txt += "  - FirstIndex: " + cdo.first_index + "\n"
        txt += "  - NumIndices: " + cdo.num_indices + " (" + (cdo.num_indices/3) + "tris)\n"
        txt += "  - BaseVtxIdx: " + cdo.base_vertex_index + "\n"
        txt += getMatText(cdo.material)
      }
      else {
        local clear = dopc.captured_clear[i]
        txt += "* ClearBuffers: \n"
        txt += "  - Flags: " + ::FlagsToString(clear.x,::eClearBuffersFlags) + "\n"
        txt += "  - Color: " + ::RGBA(clear.y) + "\n"
        txt += "  - Depth: " + ::ultof(clear.z) + "\n"
        txt += "  - Stencil: " + clear.w + "\n"
      }
      mCurrentDop = i
      w.FindWidget("ID_Count").text = txt
    }
  }
  else {
    txt += "- Number of Clear and DrawOps :" + dopc.num_captured + "\n"
    w.FindWidget("ID_Count").text = txt
  }

  return true;
}
function ID_StopAt(w,cmd) {
  if (cmd.id != ::eWidgetScrollBarCmd.Scrolled) return
  local stopAtEnabled = w.FindWidget("ID_StopAtEnabled")
  if (stopAtEnabled.check) {
    local dopc = ::gGraphics.draw_op_capture
    if (dopc) {
      dopc.capture_stop_at = cmd.sender.scroll_position
    }
  }
  else {
    mCurrentDop = invalid
  }
}
function ID_StopAtEnabled(w,cmd) {
  local dopc = ::gGraphics.draw_op_capture
  if (cmd.id == ::eWidgetButtonCmd.Checked) {
    local stopAt = w.FindWidget("ID_StopAt")
    stopAt.scroll_range = ::Vec2(0,dopc.num_captured)
    stopAt.page_size = 1
    if (stopAt.scroll_position >= dopc.num_captured) {
      stopAt.scroll_position = 0
      dopc.capture_stop_at = 0
    }
    else {
      dopc.capture_stop_at = stopAt.scroll_position
    }
    stopAt.SetFocus()
  }
  else if (cmd.id == ::eWidgetButtonCmd.UnChecked) {
    dopc.capture_stop_at = invalid
  }
}

function ID_StopAtBreakBegin(w,cmd) {
  if (cmd.id != ::eWidgetButtonCmd.Clicked) return
  local dopc = ::gGraphics.draw_op_capture
  dopc.capture_flags |= ::eGraphicsCaptureFlags.BreakOnStopAtBegin
}

function ID_StopAtBreakEnd(w,cmd) {
  if (cmd.id != ::eWidgetButtonCmd.Clicked) return
  local dopc = ::gGraphics.draw_op_capture
  dopc.capture_flags |= ::eGraphicsCaptureFlags.BreakOnStopAtEnd
}

///////////////////////////////////////////////
function setLabel(aText) {
  mwTexturesTab.FindWidget("ID_Label").text = aText
}

///////////////////////////////////////////////
function fillTextureList(aForce) {
  local cb = mwTexturesTab.FindWidget("ID_TexList")
  local list = cb.dropped_widget
  if (aForce || list.num_items != ::gGraphics.num_textures) {
    list.ClearItems()
    local texs1 = []
    local texs2 = []
    local ucount = 0
    for (local i = 0; i < ::gGraphics.num_textures; ++i) {
      local tex = ::gGraphics.texture_from_index[i]
      local name = tex.device_resource_name || ""
      if (!name || name.empty()) {
        name = "UNNAMED#"+(ucount++)+"-"+tex.width+"x"+tex.height+"x"+tex.depth+"-"+tex.pixel_format.format
        texs2.append(name)
      }
      else {
        texs1.append(name);
      }
    }
    texs1.sort()
    foreach (t in texs1) {
      list.AddItem(t)
    }
    foreach (t in texs2) {
      list.AddItem(t)
    }
    cb.num_lines = list.num_items
  }
  updateCurrentTexture()
}

///////////////////////////////////////////////
function ID_TexList(w,cmd) {
  if (cmd.id != ::eWidgetListBoxCmd.SelectionChanged)
    return false
  fillTextureList(false)
  mTexViewSink.updateZoomLabel()
}

///////////////////////////////////////////////
function ID_RefreshList(w,cmd) {
  if (cmd.id != ::eWidgetButtonCmd.Clicked)
    return false
  fillTextureList(true)
}

///////////////////////////////////////////////
function updateCurrentTexture() {
  local texture = null
  local list = mwTexturesTab.FindWidget("ID_TexList")
  local texName = list.child_from_id["ComboBox_EditBox"].text
  if (texName.startswith("UNNAMED#"))
    texture = ::gGraphics.texture_from_index[list.selected]
  else
    texture = ::gGraphics.texture_from_name[texName]
  if (texture) {
    ::LINT_CHECK_TYPE("interface_def<iTexture>",texture)
    local type = ::EnumToString(texture.type,::eBitmapType)
    local flags = ::FlagsToString(texture.flags,::eTextureFlags)
    local fmt = texture.?pixel_format.?format || "Unknown"
    setLabel(getTexName(texture))
  }
  else {
    setLabel("Can't get texture ["+texName+"]")
  }
  mTexViewSink.mTexture = texture;
}
