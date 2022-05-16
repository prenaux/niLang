// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("niUI")
::Import("lang.ni")
::Import("math.ni")

::gUIContext <- ::gLang.global_instance["iUIContext"]
::gGraphics <- ::gUIContext.?graphics
::gGraphicsContext <- ::gUIContext.?graphics_context

::gui <- {
  //--------------------------------------------------------------------------------------------
  //
  //  Toolbar
  //
  //--------------------------------------------------------------------------------------------
  function hideToolbarPopup() {
    if (::gUIContext.toolbar.auto_hide)
      ::gUIContext.toolbar.show_page = false
  }

  //--------------------------------------------------------------------------------------------
  //
  //  Overlays
  //
  //--------------------------------------------------------------------------------------------
  function loadOverlayEx(aName,aBlend,aFiltered,aForceReload) {
    if (aForceReload) {
      local tex = ::gGraphics.GetTextureFromName(aName)
      if (tex) {
        local texResMan = ::gGraphics.GetTextureDeviceResourceManager();
        texResMan.Unregister(tex)
      }
    }
    local o = ::gGraphics.CreateOverlayResource(aName)
    o.blend_mode = aBlend
    o.filtering = aFiltered
    return o
  }
  function loadOverlayTranslucent(aName,aForceReload) {
    return ::gui.loadOverlayEx(aName,::eBlendMode.Translucent,false,aForceReload)
  }
  function loadOverlayTranslucentF(aName,aForceReload) {
    return ::gui.loadOverlayEx(aName,::eBlendMode.Translucent,true,aForceReload)
  }
  function loadOverlaySolid(aName,aForceReload) {
    return ::gui.loadOverlayEx(aName,::eBlendMode.NoBlending,false,aForceReload)
  }
  function loadOverlaySolidF(aName,aForceReload) {
    return ::gui.loadOverlayEx(aName,::eBlendMode.NoBlending,true,aForceReload)
  }
  function loadOverlayAdditive(aName,aForceReload) {
    return ::gui.loadOverlayEx(aName,::eBlendMode.Additive,false,aForceReload)
  }
  function loadOverlayAdditiveF(aName,aForceReload) {
    return ::gui.loadOverlayEx(aName,aForceReload,::eBlendMode.Additive,true,aForceReload)
  }

  function loadIcon(aW,id,name,max,min) {
    local w = id ? aW.FindWidget(id) : aW
    if (w) {
      local skinDT = aW.uicontext.skin_data_table[w.skin]
      if (skinDT) {
        local iconSet = skinDT.string.icon_set
        return aW.uicontext.image_map.AddImageFromIconSet(null,iconSet,name,max,min)
      }
    }
  }
  function loadIconSmall(aW,id,name) {
    return loadIcon(aW,id,name,32,16)
  }
  function loadIconLarge(aW,id,name) {
    return loadIcon(aW,id,name,64,16)
  }

  function setIcon(aW,id,name,max,min,filter) {
    local w = id ? aW.FindWidget(id) : aW
    if (w) {
      local skinDT = aW.uicontext.skin_data_table[w.skin]
      if (skinDT) {
        local iconSet = skinDT.string.icon_set
        w.icon = aW.uicontext.image_map.AddImageFromIconSet(null,iconSet,name,max,min)
        if (w.icon) {
          w.icon.filtering = filter
          w.style |= ::eWidgetButtonStyle.IconCenter|::eWidgetButtonStyle.NoText|::eWidgetButtonStyle.IconFit
        }
      }
    }
    return w
  }
  function setIconSmall(aW,id,name) {
    return setIcon(aW,id,name,32,16,false)
  }
  function setIconLarge(aW,id,name) {
    return setIcon(aW,id,name,128,16,false)
  }
  function setIconSmallF(aW,id,name) {
    return setIcon(aW,id,name,32,16,true)
  }
  function setIconLargeF(aW,id,name) {
    return setIcon(aW,id,name,128,16,true)
  }

  function blitFit(aDest,aSrc) {
    local aspectRatio = aSrc.width.tofloat() / aSrc.height.tofloat()
    local toFinalSizeScale = aspectRatio < 1.0 ?
      aSrc.height.tofloat() / aDest.height.tofloat() :
      aSrc.width.tofloat() / aDest.width.tofloat()
    local fbFinalW = aSrc.width.tofloat() / toFinalSizeScale
    local fbFinalH = aSrc.height.tofloat() / toFinalSizeScale
    if (aspectRatio < 1.0) {
      // vertical/portrait
      aDest.BlitStretch(aSrc, 0, 0, (aDest.width - fbFinalW) / 2, 0,
                        aSrc.width, aSrc.height,
                        fbFinalW, fbFinalH)
    }
    else {
      // horizontal/landscape
      aDest.BlitStretch(aSrc, 0, 0, 0, (aDest.height - fbFinalH) / 2,
                        aSrc.width, aSrc.height,
                        fbFinalW, fbFinalH)
    }
    return aDest
  }

  function bitmapToThumbnail(aBitmap,aThumbW,aThumbH) {
    local finalBitmap = ::gGraphics.CreateBitmap2D(aThumbW,aThumbH,"R8G8B8A8")
    finalBitmap.Clearf(::Vec4(0,0,0,0))
    blitFit(finalBitmap,aBitmap)
    return finalBitmap
  }

  function load2DBitmap(path,g) {
    g = g || (::gGraphics)

    local bmpBase
    if (typeof path == "string") {
      bmpBase = g.LoadBitmapFromRes(path,"");
    }
    else {
      // assume iFile
      bmpBase = g.LoadBitmap(path,"");
    }
    if (!bmpBase)
      return null

    local hasAlpha =
      (bmpBase.pixel_format.num_abits) &&
      (!bmpBase.pixel_format.format.contains("X"));

    switch (bmpBase.type) {
      case ::eBitmapType["2D"]: {
        return bmpBase
      }
      case ::eBitmapType.Cube: {
        local bmpCube = bmpBase.QueryInterface("iBitmapCube");
        local pxfCaps = bmpBase.pixel_format.caps;
        local pxf = bmpBase.pixel_format;
        // If the pixel format is block compressed and doesnt support
        // clear/blit fallback to a RGBA bitmap and rely on the pixel format
        // conversion to do the job.
        if ((pxfCaps&::ePixelFormatCaps.BlockCompressed) &&
            (!(pxfCaps&::ePixelFormatCaps.Clear) ||
             !(pxfCaps&::ePixelFormatCaps.Blit)))
        {
          pxf = g.CreatePixelFormat("R8G8B8A8");
          // ::println("... CONVERTING PXF", bmpCube.pixel_format.format, "to", pxf.format)
          bmpCube = bmpCube.CreateConvertedFormat(pxf);
        }
        local bmp2d = g.CreateBitmap2DEx(bmpCube.width*4,bmpCube.width*3,bmpCube.pixel_format);
        bmp2d.Clearf(::Vec4(0,0,0,0));
        // unfold the cube...
        local width = bmpCube.width;
        bmp2d.Blit(
          bmpCube.face[0],
          0,0,
          2*width,1*width,
          width,width,
          0);
        bmp2d.Blit(
          bmpCube.face[1],
          0,0,
          0*width,1*width,
          width,width,
          0);
        bmp2d.Blit(
          bmpCube.face[2],
          0,0,
          1*width,0*width,
          width,width,
          0);
        bmp2d.Blit(
          bmpCube.face[3],
          0,0,
          1*width,2*width,
          width,width,
          0);
        bmp2d.Blit(
          bmpCube.face[4],
          0,0,
          1*width,1*width,
          width,width,
          0);
        bmp2d.Blit(
          bmpCube.face[5],
          0,0,
          3*width,1*width,
          width,width,
          0);
        return bmp2d
      }
    }
    return null;
  }

  //--------------------------------------------------------------------------------------------
  //
  //  Widgets
  //
  //--------------------------------------------------------------------------------------------
  function createWidget(aCtx,aClass,aParent,aRect,aID,aStyle)
  {
    local w = aCtx.CreateWidget(aClass,aParent?aParent:null,aRect,aStyle,aID)
    return w
  }

  function createWidgetFromXML(aParent,aXML) {
    return ::gUIContext.CreateWidgetFromDataTable(
      ::lang.dataTableFromString(aXML), aParent, "", "")
  }

  function createDummy(aCtx,aParent,aRect)
  {
    return ::gui.createWidget(aCtx,"Dummy",aParent,aRect,"",0)
  }

  function createCanvas(aCtx,aParent,aRect,aID,aHoldFocus,aSink)
  {
    local w = ::gui.createWidget(aCtx,"Canvas",aParent,aRect,aID,aHoldFocus?::eWidgetStyle.HoldFocus:0)
    if (aSink) w.AddSink(aSink)
    return w
  }

  function createScrollCanvas(aCtx,aParent,aRect,aID,aHoldFocus,aSink)
  {
    local w = ::gui.createWidget(aCtx,"Canvas",aParent,aRect,aID,::eWidgetCanvasStyle.ScrollH|::eWidgetCanvasStyle.ScrollV|(aHoldFocus?::eWidgetStyle.HoldFocus:0))
    if (aSink) w.AddSink(aSink)
    return w
  }

  function createTab(aCtx,aParent,aRect)
  {
    return ::gui.createWidget(aCtx,"Tab",aParent,aRect,"",0)
  }

  function createForm(aCtx,aParent,aRect,aText,aID,aFlags)
  {
    local w = ::gui.createWidget(aCtx,"Form",aParent,aRect,aID,aFlags|::eWidgetStyle.HoldFocus)
    if (aText)  w.title = aText
    return w
  }

  function createSplitterH(aCtx,aParent,aID)
  {
    local w = ::gui.createWidget(aCtx,"Splitter",aParent,::Rect(0,0,1,1),aID,::eWidgetStyle.HoldFocus|::eWidgetSplitterStyle.Horizontal)
    return w
  }

  function createSplitterV(aCtx,aParent,aID)
  {
    local w = ::gui.createWidget(aCtx,"Splitter",aParent,::Rect(0,0,1,1),aID,::eWidgetStyle.HoldFocus)
    return w
  }

  function createEmptySplitterH(aCtx,aParent,aID)
  {
    local w = ::gui.createWidget(aCtx,"Splitter",aParent,::Rect(0,0,1,1),aID,::eWidgetStyle.HoldFocus|::eWidgetSplitterStyle.Empty|::eWidgetSplitterStyle.Horizontal)
    return w
  }

  function createEmptySplitterV(aCtx,aParent,aID)
  {
    local w = ::gui.createWidget(aCtx,"Splitter",aParent,::Rect(0,0,1,1),aID,::eWidgetStyle.HoldFocus|::eWidgetSplitterStyle.Empty)
    return w
  }

  function createLabel(aCtx,aParent,aRect,aText,abTrans,aID)
  {
    local w = ::gui.createWidget(aCtx,"Label",aParent,aRect,aID,0)
    if (aText) w.text = aText
    return w
  }

  function createLabelFit(aCtx,aParent,aRect,aText,abTrans,aID)
  {
    local w = ::gui.createWidget(aCtx,"Label",aParent,aRect,aID,0)
    if (aText) w.text = aText
    return w
  }

  function createButton(aCtx,aParent,aRect,aText,aID)
  {
    local w = ::gui.createWidget(aCtx,"Button",aParent,aRect,aID,0)
    if (aText) w.text = aText
    return w
  }

  function createButtomImage(aCtx,aParent,aRect,aText,aImg,aID) {
    local w = ::gui.createWidget(aCtx,"Button",aParent,aRect,aID,::eWidgetButtonStyle.BitmapButton|::eWidgetButtonStyle.NoFrame|::eWidgetButtonStyle.NoFrame|::eWidgetButtonStyle.IconStretch)
    if (aText) w.text = aText
    if (aImg) w.icon = ::gui.loadOverlayTranslucentF(aImg)
    return w
  }

  function createRadioButton(aCtx,aParent,aRect,aText,aID,aGroupID)
  {
    local w = ::gui.createWidget(aCtx,"Button",aParent,aRect,aID,::eWidgetButtonStyle.RadioButton)
    if (aText) w.text = aText
    if (aGroupID) w.group_id = aGroupID
    return w
  }

  function createCheckBox(aCtx,aParent,aRect,aText,aID)
  {
    local w = ::gui.createWidget(aCtx,"Button",aParent,aRect,aID,::eWidgetButtonStyle.CheckBox|::eWidgetButtonStyle.OnOff)
    if (aText) w.text = aText
    return w
  }

  function createEditBox(aCtx,aParent,aRect,aText,aID)
  {
    local w = ::gui.createWidget(aCtx,"EditBox",aParent,aRect,aID,::eWidgetStyle.HoldFocus)
    if (aText) w.text = aText
    return w
  }

  function createTextBox(aCtx,aParent,aRect,aText,aID)
  {
    local w = ::gui.createWidget(aCtx,"EditBox",aParent,aRect,aID,::eWidgetStyle.HoldFocus|::eWidgetEditBoxStyle.MultiLine)
    if (aText) w.text = aText
    return w
  }

  function createComboBox(aCtx,aParent,aRect,aID,aSel,aValues)
  {
    local w = ::gui.createWidget(aCtx,"ComboBox",aParent,aRect,aID,::eWidgetStyle.HoldFocus|::eWidgetComboBoxStyle.DropDown)
    if (aValues) {
      foreach (v in aValues)
      w.AddItem(v.tostring())
    }
    w.selected = aSel
    return w
  }

  function createEditComboBox(aCtx,aParent,aRect,aID,aSel,aValues)
  {
    local w = ::gui.createWidget(aCtx,"ComboBox",aParent,aRect,aID,::eWidgetStyle.HoldFocus)
    if (aValues) {
      foreach (v in aValues)
      w.AddItem(v.tostring())
    }
    w.selected = aSel
    return w
  }

  function createListBox(aCtx,aParent,aRect,aID,abHeader)
  {
    local w = ::gui.createWidget(aCtx,"ListBox",aParent,aRect,aID,::eWidgetStyle.HoldFocus|(abHeader?::eWidgetListBoxStyle.HasHeader:0))
    return w
  }

  function createProgressBar(aCtx,aParent,aRect,aRange)
  {
    local w = ::gui.createWidget(aCtx,"ProgressBar",aParent,aRect,"",0)
    w.SetRange(aRange.x,aRange.y)
    return w
  }

  function createMenu(aCtx,aParent,aID,aStyle)
  {
    local w = ::gui.createWidget(aCtx,"Menu",aParent,::Rect(),aID,aStyle|::eWidgetStyle.Free)
    w.visible = false
    w.enabled = false
    return w
  }

  function createSubMenu(aCtx,aParent,aID)
  {
    local w = aCtx.CreateWidget("Menu",aParent?aParent:null,::Rect(),::eWidgetStyle.Free,aID)
    w.visible = false
    w.enabled = false
    return w
  }

  function createGroup(aCtx,aParent,aRect,aText,aID)
  {
    local w = ::gui.createWidget(aCtx,"Group",aParent,aRect,aID,0)
    w.text = aText
    return w
  }

  function createFoldableGroup(aCtx,aParent,aRect,aText,aID)
  {
    local w = ::gui.createWidget(aCtx,"Group",aParent,aRect,aID,::eWidgetGroupStyle.Foldable)
    w.text = aText
    return w
  }

  function createTree(aCtx,aParent,aRect,aID,abDrawRoot)
  {
    return ::gui.createWidget(
      aCtx,"Tree",aParent,aRect,aID,
      (abDrawRoot?0:(::eWidgetTreeStyle.DontDrawRoot))|
        ::eWidgetStyle.HoldFocus
    )
  }

  function createPropertyBox(aCtx,aParent,aRect,aID,abDefaultDataTable)
  {
    return ::gui.createWidget(aCtx,"PropertyBox",aParent,aRect,aID,(abDefaultDataTable?0:(::eWidgetTreeStyle.NoDefaultDataTable))|::eWidgetStyle.HoldFocus)
  }

  function createSliderH(aCtx,aParent,aRect,aID)
  {
    return ::gui.createWidget(aCtx,"ScrollBar",aParent,aRect,aID,::eWidgetScrollBarStyle.Slider|::eWidgetScrollBarStyle.Horz)
  }

  function createSliderV(aCtx,aParent,aRect,aID)
  {
    return ::gui.createWidget(aCtx,"ScrollBar",aParent,aRect,aID,::eWidgetScrollBarStyle.Slider)
  }

  function createScrollBarH(aCtx,aParent,aRect,aID)
  {
    return ::gui.createWidget(aCtx,"ScrollBar",aParent,aRect,aID,::eWidgetScrollBarStyle.Horz)
  }

  function createScrollBarV(aCtx,aParent,aRect,aID)
  {
    return ::gui.createWidget(aCtx,"ScrollBar",aParent,aRect,aID,0)
  }

  function createDropDown(aButton,aResPath) {
    local c = ::gUIContext.CreateWidgetFromResource(aResPath, ::gUIContext.root_widget, null,null)
    if (!c) {
      throw "Can't create toolbar drop down from '"+aResPath+"'."
    }
    return createDropDownEx(aButton, c);
  }

  function createDropDownEx(aButton,aWidget) {
    local c = ::gUIContext.CreateWidget(
      "Group",aWidget.parent,
      ::Rect(0,0,100,100),
      ::eWidgetStyle.Temp|
        ::eWidgetStyle.Free|
        ::eWidgetButtonStyle.IconButton|
        ::eWidgetButtonStyle.NoText,
      "__ID_DropDown__")
    c.skin_class = "DropDownGroup"
    c.auto_layout = ::eWidgetAutoLayoutFlags.Size
    aWidget.parent = c
    c.ComputeAutoLayout(c.auto_layout)
    // aWidget.visible = false

    if (aButton) {
      local position = aButton.absolute_position + ::Vec2(0, aButton.size.y);
      if (position.x+c.size.x > ::gUIContext.root_widget.size.x) {
        position.x = aButton.absolute_rect.z - c.size.x
      }
      if (position.y+c.size.y > ::gUIContext.root_widget.size.y) {
        position.y = aButton.absolute_rect.y - c.size.y
      }
      c.position = position
    }
    c.exclusive = true
    c.zorder = ::eWidgetZOrder.Overlay

    local postSink = ::delegate(::gui.baseWidgetSink, {
      function OnNCRightClickDown(w,a,b) {
        ::dbg("... OnNCRightClickDown Destroy")
        w.Destroy()
      }
      function OnNCLeftClickDown(w,a,b) {
        ::dbg("... OnNCLeftClickDown Destroy")
        w.Destroy()
      }
      function OnDestroy(w,a,b) : (aButton) {
        if (aButton && aButton.QueryInterface("iWidgetButton")) {
          aButton.check = false
        }
      }
    });

    c.AddPostSink(postSink)
    aWidget.SetFocus()
    return aWidget
  }

  function initDesktopWidget() {
    if (::gUIContext.root_widget.FindWidget("ID_Desktop") != null)
      throw "Desktop widget already created."
    local dm = ::gUIContext.CreateWidget("DockingManager",null,::Rect(),::eWidgetStyle.HoldFocus,"ID_Desktop")
    dm.dock_style = ::eWidgetDockStyle.DockFill
    dm.zorder = ::eWidgetZOrder.Bottom
    return dm
  }

  function getDesktopWidget() {
    local rw = ::gUIContext.root_widget
    local dm = rw.FindWidget("ID_Desktop")
    return dm || rw;
  }

  function saveLayoutDT(aDT,abChild)
  {
    if (!::gUIContext.SerializeWidget(::gui.getDesktopWidget(),
                                      aDT,
                                      ::eWidgetSerializeFlags.Write|
                                      (abChild?::eWidgetSerializeFlags.Children:0),
                                      null))
      throw "Can't serialize write the layout."
  }

  function loadLayoutDT(aDT,abChild)
  {
    if (!::gUIContext.SerializeWidget(::gui.getDesktopWidget(),
                                      aDT,
                                      ::eWidgetSerializeFlags.Read|
                                      (abChild?::eWidgetSerializeFlags.Children:0),
                                      null))
      throw "Can't serialize read the layout ."
  }

  function saveLayout(aPath,abChild)
  {
    local dt = ::gLang.CreateDataTable("")
    saveLayoutDT(dt,abChild)
    ::lang.writeDataTable("",dt,aPath)
  }

  function loadLayout(aPath,abChild)
  {
    local dt = ::lang.loadDataTable("",aPath)
    loadLayoutDT(dt,abChild)
  }

  //!
  //! \param aTitle is the title of the message box
  //! \param aMsg is the message to display
  //! \param aButtons is the list of buttons of the message box (ID_ will be prependanded and used as ID), if null a OK button will be placed.
  //!
  function createMessageBox2(aUICtx,aParent,aTitle,aMsg,aButtons,aIcon)
  {
    if (!aButtons) aButtons = ["OK"]

    local form = ::gui.createForm(aUICtx,aParent,::Rect(0,0,100,100),aTitle,"ID_MessageBox."+aTitle,
                                  ::eWidgetFormStyle.Move|::eWidgetStyle.FocusActivate|
                                  ::eWidgetStyle.DontSerialize|::eWidgetFormStyle.CloseButton|
                                  ::eWidgetStyle.HoldFocus)

    local sink = ::delegate(::gui.baseWidgetSink, {
      function OnCommand(w,aCmd) {
        if (aCmd.id == ::eWidgetButtonCmd.Clicked) {
          w.Destroy()
        }
      }
      function OnSubmit(w,a,b) {
        w.Destroy()
      }
      function OnCancel(w,a,b) {
        w.Destroy()
      }
    })
    form.padding = ::Vec4(5,5,5,5)
    if (aButtons.len() == 1)
      form.input_submit_flags = ::eUIInputSubmitFlags.SubmitA

    local bottom = ::gUIContext.CreateWidget("Dummy",form,::Rect(0,0,38,22),0,null)
    bottom.dock_style = ::eWidgetDockStyle.DockBottom
    local offset = ::gUIContext.CreateWidget("Dummy",form,::Rect(0,0,32,10),0,null)
    offset.dock_style = ::eWidgetDockStyle.DockBottom
    local top = ::gUIContext.CreateWidget("Dummy",form,::Rect(0,0,10,10),0,null)
    top.dock_style = ::eWidgetDockStyle.DockFill

    // create the icon
    local iconLeft = 0
    local iconHeight = 0
    if (aIcon) {
      iconLeft = 40
      iconHeight = 40
      local left = ::gUIContext.CreateWidget("Dummy",top,::Rect(0,0,iconLeft,iconHeight),0,"ID_IconDummy")
      left.dock_style = ::eWidgetDockStyle.DockLeft
      local icon = ::gui.createButton(aUICtx,left,::Rect(0,0,32,32),"","ID_Icon")
      icon.style |= ::eWidgetButtonStyle.IconFit|::eWidgetButtonStyle.NoFrame
      icon.dock_style = ::eWidgetDockStyle.SnapCenter
      ::gui.setIconLarge(form,"ID_Icon",aIcon)
    }

    // create the label
    local label = ::gui.createLabel(aUICtx,top,::Rect(0,0,10,10),aMsg,true,"ID_Label")
    label.font_format_flags = ::eFontFormatFlags.CenterH|::eFontFormatFlags.CenterV
    label.dock_style = ::eWidgetDockStyle.DockFill

    // comput the size of the text
    local rectMsg = label.font.ComputeTextSize(::Rect(0,0),aMsg,0)
    local rectTitle = label.font.ComputeTextSize(::Rect(0,0),aTitle,0)+10
    rectTitle.width += 25
    local rect = (rectTitle.width > rectMsg.width) ? rectTitle : rectMsg
    rect.width += iconLeft

    // get the form's size
    local iconSize = 32
    local buttonsWidth = 86*aButtons.len()
    local formWidth;
    if (buttonsWidth > rect.width) {
      formWidth = buttonsWidth
    }
    else {
      formWidth = rect.width
    }
    local formHeight = bottom.size.y+offset.size.y+::gMath.Max(rectMsg.height,iconHeight)

    // create the buttons
    local buttonsDock = ::gUIContext.CreateWidget("Dummy",bottom,
                                                  //::Rect(0,0,(iconSize)*aButtons.len(),(iconSize)),0,null)
                                                  ::Rect(0,0,buttonsWidth,bottom.size.y),0,null)
    buttonsDock.dock_style = ::eWidgetDockStyle.SnapCenter
    foreach (bt in aButtons) {
      local w = ::gui.createButton(aUICtx,buttonsDock,::Rect(0,0,80,16),bt,"ID_"+bt)
      w.dock_style = ::eWidgetDockStyle.DockLeft
      w.style = ::eWidgetButtonStyle.IconFit
    }

    // setup the form's size and setup the sink
    form.exclusive = true
    form.fit_size = ::Vec2(formWidth,formHeight)
    form.ApplyDockStyle(::eWidgetDockStyle.SnapCenter)
    form.AddSink(sink)
    form.zorder = ::eWidgetZOrder.TopMost
    form.input_submit_flags = ::eUIInputSubmitFlags.SubmitA
    try {
      ::gui.setIconSmall(form,"ID_OK","validation")
      ::gui.setIconSmall(form,"ID_Yes","validation")
      ::gui.setIconSmall(form,"ID_No","cancel")
      ::gui.setIconSmall(form,"ID_Cancel","cancel")
    } catch(e) {}
    return form
  }

  function createMessageBox(aUICtx,aParent,aTitle,aMsg,aButtons)
  {
    return createMessageBox2(aUICtx,aParent,aTitle,aMsg,aButtons,null)
  }

  function findWidget(aID1,aID2,aID3,aID4)
  {
    local w = ::gUIContext.root_widget.FindWidget(aID1)
    if (aID2 && w) {
      w = w.FindWidget(aID2);
      if (aID3 && w) {
        w = w.FindWidget(aID3);
        if (aID4 && w) {
          w = w.FindWidget(aID4);
        }
      }
    }
    return w
  }

  function dialogErrorMessage(aTitle,aErr) {
    return ::gui.createMessageBox2(::gUIContext,null,aTitle,aErr,null,"messagebox_critical")
  }
  function dialogWarningMessage(aTitle,aErr) {
    return ::gui.createMessageBox2(::gUIContext,null,aTitle,aErr,null,"messagebox_warning")
  }
  function dialogInfoMessage(aTitle,aErr) {
    return ::gui.createMessageBox2(::gUIContext,null,aTitle,aErr,null,"messagebox_info")
  }
  function dialogQuestionMessage(aTitle,aErr) {
    return ::gui.createMessageBox2(::gUIContext,null,aTitle,aErr,null,"messagebox_question")
  }
  function dialogMessage(aTitle,aMsg) {
    return ::gui.createMessageBox(::gUIContext,null,aTitle,aMsg,null)
  }
  function dialogNotImplemented(aTitle) {
    return ::gui.createMessageBox(::gUIContext,null,aTitle,"Not implemented.",null)
  }
  function dialogMessageEx2(aParent,aTitle,aMsg,aButtons,aSink) {
    local form = ::gui.createMessageBox(::gUIContext,aParent,aTitle,aMsg,aButtons)
    local sink = ::delegate(::gui.baseWidgetSink, {
      function OnCommand(w,aCmd) {
        if (aCmd.id == ::eWidgetButtonCmd.Clicked) {
          w.Destroy()
        }
      }
    })
    form.AddSink(sink)
    if (aSink) form.AddSink(aSink)
    return form
  }
  function dialogMessageEx(aTitle,aMsg,aButtons,aSink) {
    dialogMessageEx2(null,aTitle,aMsg,aButtons,aSink)
  }
  function dialogMessageYesNo(aTitle,aMsg,aSink) {
    ::gui.dialogMessageEx2(null,aTitle,aMsg,["Yes","No"],aSink)
  }
  function dialogMessageOkCancel(aTitle,aMsg,aSink) {
    dialogMessageEx2(null,aTitle,aMsg,["Ok","Cancel"],aSink)
  }

  ///////////////////////////////////////////////
  function dialogEnterNameEx(aParent,aTitle,aDefaultName,aSink) {
    local w, dummy
    local form = ::gui.createForm(::gUIContext,aParent,::Rect(),aTitle,"",
                                  (::eWidgetFormStyle.Default|::eWidgetStyle.FocusActivate|::eWidgetStyle.DontSerialize|::eWidgetFormStyle.CloseButton)&
                                  (~(::eWidgetFormStyle.ResizeBottomEdge|::eWidgetFormStyle.ResizeTopEdge|::eWidgetFormStyle.ResizeAnyCorner)))
    form.padding = ::Vec4(1,2,1,3)

    form.fit_size = ::Vec2(250,44)
    form.minimum_size = ::Vec2(170,44)
    form.ApplyDockStyle(::eWidgetDockStyle.SnapCenter)
    form.exclusive = true
    w = ::gui.createEditBox(::gUIContext,form,::Rect(0,0,250,22),aDefaultName,"ID_Text");
    w.dock_style = ::eWidgetDockStyle.DockTop
    w.selection = w.all_text_range

    {
      dummy = ::gui.createDummy(::gUIContext,form,::Rect(0,0,150,24))
      dummy.padding = ::Vec4(0,2,0,0)
      dummy.dock_style = ::eWidgetDockStyle.DockTop
      dummy = ::gui.createDummy(::gUIContext,dummy,::Rect(0,0,150,22))
      dummy.dock_style = ::eWidgetDockStyle.SnapCenter
      w = ::gui.createButton(::gUIContext,dummy,::Rect(0,0,75,22),"OK","ID_OK");
      w.dock_style = ::eWidgetDockStyle.DockLeft
      w = ::gui.createButton(::gUIContext,dummy,::Rect(0,0,75,22),"Cancel","ID_Cancel");
      w.dock_style = ::eWidgetDockStyle.DockLeft
    }

    try {
      ::gui.setIconSmall(form,"ID_OK","validation")
      ::gui.setIconSmall(form,"ID_Yes","validation")
      ::gui.setIconSmall(form,"ID_No","cancel")
      ::gui.setIconSmall(form,"ID_Cancel","cancel")
    } catch(e) {}

    local sink = ::delegate(::gui.baseWidgetSink, {
      function OnCancel(w,a,b) {
        w.Destroy()
      }
      function OnSubmit(w,a,b) {
        w.Destroy()
      }
      function OnCommand(w,aCmd) {
        if (aCmd.sender.class_name == "EditBox" && aCmd.id == ::eWidgetEditBoxCmd.Validated) {
          w.SendMessage(::eUIMessage.Submit,w,null)
        }
        else if (aCmd.sender.class_name == "Button" && aCmd.id == ::eWidgetButtonCmd.Clicked) {
          if (aCmd.sender.id == "ID_OK") {
            w.SendMessage(::eUIMessage.Submit,w,null)
          }
          else if (aCmd.sender.id == "form_button_Close") {
            w.SendMessage(::eUIMessage.Cancel,w,null)
            return true
          }
          else if (aCmd.sender.id == "ID_Cancel") {
            w.SendMessage(::eUIMessage.Cancel,w,null)
          }
        }
      }
    })
    form.input_submit_flags = ::eUIInputSubmitFlags.SubmitA
    form.AddSink(sink)
    if (aSink) form.AddSink(aSink)
    form.child_from_id["ID_Text"].SetFocus()
    return form
  }
  function dialogEnterName(aTitle,aDefaultName,aSink) {
    return ::gui.dialogEnterNameEx(null,aTitle,aDefaultName,aSink)
  }

  ///////////////////////////////////////////////
  function dialogUserPwdEx2(aParent,aTitle,aDefaultUser,aSink) {
    local w, dummy
    local form = ::gui.createForm(::gUIContext,aParent,::Rect(0,0,250,44),aTitle,"",
                                  (::eWidgetFormStyle.Default|::eWidgetStyle.FocusActivate|::eWidgetStyle.DontSerialize|::eWidgetFormStyle.CloseButton)&
                                  (~(::eWidgetFormStyle.ResizeBottomEdge|::eWidgetFormStyle.ResizeTopEdge|::eWidgetFormStyle.ResizeAnyCorner)))
    form.padding = ::Vec4(2,2,1,2)

    {
      dummy = ::gui.createDummy(::gUIContext,form,::Rect(0,0,250,22))
      dummy.dock_style = ::eWidgetDockStyle.DockTop
      w = ::gui.createLabel(::gUIContext,dummy,::Rect(0,0,65,22),"User:");
      w.dock_style = ::eWidgetDockStyle.DockLeft
      w = ::gui.createEditBox(::gUIContext,dummy,::Rect(0,0,200,22),aDefaultUser,"ID_User");
      w.dock_style = ::eWidgetDockStyle.DockFill
      w.selection = w.all_text_range
    }

    {
      dummy = ::gui.createDummy(::gUIContext,form,::Rect(0,0,250,22))
      dummy.dock_style = ::eWidgetDockStyle.DockTop
      w = ::gui.createLabel(::gUIContext,dummy,::Rect(0,0,65,22),"Password:");
      w.dock_style = ::eWidgetDockStyle.DockLeft
      w = ::gui.createEditBox(::gUIContext,dummy,::Rect(0,0,200,22),"","ID_Pwd");
      w.replace_char = '*'
      w.dock_style = ::eWidgetDockStyle.DockFill
    }

    {
      dummy = ::gui.createDummy(::gUIContext,form,::Rect(0,0,150,24))
      dummy.padding = ::Vec4(0,2,0,0)
      dummy.dock_style = ::eWidgetDockStyle.DockTop
      dummy = ::gui.createDummy(::gUIContext,dummy,::Rect(0,0,150,22))
      dummy.dock_style = ::eWidgetDockStyle.SnapCenter
      w = ::gui.createButton(::gUIContext,dummy,::Rect(0,0,75,22),"OK","ID_OK");
      w.dock_style = ::eWidgetDockStyle.DockLeft
      w = ::gui.createButton(::gUIContext,dummy,::Rect(0,0,75,22),"Cancel","ID_Cancel");
      w.dock_style = ::eWidgetDockStyle.DockLeft
    }

    try {
      ::gui.setIconSmall(form,"ID_OK","validation")
      ::gui.setIconSmall(form,"ID_Cancel","cancel")
    } catch(e) {}

    local sink = ::delegate(::gui.baseWidgetSink, {
      function OnCancel(w,a,b) {
        w.Destroy()
      }
      function OnSubmit(w,a,b) {
        w.Destroy()
      }
      function OnCommand(w,aCmd) {
        if (aCmd.sender.class_name == "EditBox" && aCmd.id == ::eWidgetEditBoxCmd.Validated) {
          w.SendMessage(::eUIMessage.Submit,w,null)
        }
        else if (aCmd.sender.class_name == "Button" && aCmd.id == ::eWidgetButtonCmd.Clicked) {
          if (aCmd.sender.id == "ID_OK") {
            w.SendMessage(::eUIMessage.Submit,w,null)
          }
          else if (aCmd.sender.id == "form_button_Close") {
            w.SendMessage(::eUIMessage.Cancel,w,null)
            return true
          }
          else if (aCmd.sender.id == "ID_Cancel") {
            w.SendMessage(::eUIMessage.Cancel,w,null)
          }
        }
      }
    })
    form.auto_layout |= ::eWidgetAutoLayoutFlags.Size
    form.input_submit_flags = ::eUIInputSubmitFlags.SubmitA
    form.AddSink(sink)
    if (aSink) {
      form.AddSink(aSink)
    }
    form.minimum_size = ::Vec2(170,form.size.y)
    form.FindWidget("ID_User").SetFocus()
    form.ApplyDockStyle(::eWidgetDockStyle.SnapCenter)
    form.exclusive = true
    return form
  }
  function dialogUserPwd(aTitle,aDefaultUser,aSink) {
    return ::gui.dialogUserPwdEx2(null,aTitle,aDefaultUser,aSink)
  }

  ///////////////////////////////////////////////
  function loadFormEx(aRes,aParent,aID,aTitle,aStyle,aRect,abCenter,abExclusive) {
    local form = ::gUIContext.CreateWidgetFromResource(aRes,aParent,aID?aID:"",aTitle?aTitle:"")
    if (!form)    throw "Can't create form from resource '"+aRes+"'."
    if (!aStyle)  form.style = aParent ? (0) : (::eWidgetFormStyle.Default|::eWidgetStyle.FocusActivate|::eWidgetStyle.HoldFocus|::eWidgetFormStyle.CloseButton)
    else      form.style = aStyle
    if (aRect)    form.rect = aRect
    if (abCenter) form.ApplyDockStyle(::eWidgetDockStyle.SnapCenter)
    form.exclusive = abExclusive

    return form
  }
  function loadForm(aRes,aParent,aID) {
    return loadFormEx(aRes,aParent,aID,null,null,null,false,false)
  }

  ///////////////////////////////////////////////
  function popupForm(aRes,aParent,aID) {
    return loadFormEx(aRes,aParent,aID,null,null,null,true,true)
  }

  ///////////////////////////////////////////////
  function loadFormToggled(aRes,aParent,aID) {
    local w = ::gUIContext.root_widget.FindWidget(aID)
    if (w) {
      local dm = ::gui.getDesktopWidget().QueryInterface("iWidgetDockingManager")
      if (dm) {
        dm.UndockWidget(w)
      }
      w.Destroy()
      w = null
    }
    else {
      w = loadForm(aRes,aParent,aID)
      if (w) {
        ::gUIContext.active_widget = w
      }
    }
    return w
  }

  ///////////////////////////////////////////////
  function printWidgets(aRoot) {
    function print(e,idx,baseStr) {
      local str = baseStr;
      for (local i = 0; i < idx; ++i) {
        str += " "
      }
      str += "(" + e.class_name
      if (!e.id.empty()) {
        str += " id \"" + e.id + "\""
      }
      if (!e.text.empty()) {
        str += " text \"" + e.text + "\""
      }
      str += " absolute_rect ::" + e.absolute_rect.ToRectString()
      if (e.num_children) {
        for (local i = 0; i < e.num_children; ++i) {
          str += "\n"
          str += print(e.child_from_index[i],idx+1,"")
        }
      }
      str += ")"
      return str
    }
    if (typeof aRoot == "string") {
      local id = aRoot
      aRoot = ::gUIContext.root_widget.FindWidget(id)
      if (!aRoot) {
        ::log("Can't find widget: " + id)
        return
      }
    }
    else if (!aRoot) {
      aRoot = ::gUIContext.root_widget
    }
    ::log(print(aRoot,0,""))
  }

  ///////////////////////////////////////////////
  function printSystemFonts() {
    local c = ::gGraphics
    for (local i = 0; i < c.num_system_fonts; ++i) {
      ::dbg("["+i+"] : "+c.system_font_name[i]+" ("+c.system_font_name[i]+")")
    }
  }

  ///////////////////////////////////////////////
  function reloadSkin(aContext) {
    aContext = aContext || ::gUIContext
    ::printdebugln("REMOVE SKIN :" + aContext.default_skin)
    local skin = aContext.default_skin
    aContext.RemoveSkin(aContext.default_skin)
    aContext.default_skin = skin
    aContext.root_widget.?BroadcastMessage(::eUIMessage.SkinChanged,null,null)
    ::printdebugln("SKIN LOADED : " + aContext.default_skin)
  }

  ///////////////////////////////////////////////
  // Load the global image map
  function loadImageMap(aIM,aPath) {
    local imageMap = aIM || (::gUIContext.image_map)
    local path = aPath
    if (!path) {
      path = "data/base/cache/AppIcons.imagemap"
    }
    local fp = ::lang.urlOpen(path)
    if (fp) {
      if (!imageMap.Serialize(fp,::eImageMapSerializeFlags.Read)) {
        return false
      }
      ::log("Loaded ImageMap from '"+path+"'")
    }
    else {
      ::log("Couldn't find ImageMap '"+path+"'")
    }
    return true
  }

  ///////////////////////////////////////////////
  // Save the global image map
  function saveImageMap(aIM,aPath) {
    local imageMap = aIM ||  ::gUIContext.image_map
    local path = aPath
    if (!path) {
      path = "data/base/cache/AppIcons.imagemap"
    }
    if (!imageMap.should_serialize) {
      ::log("ImageMap '"+path+"' already up-to-date.")
      return false
    }
    ::gRootFS.FileMakeDir(path.getdir())
    local fp = ::fs.fileOpenWrite(path)
    if (!fp) throw "Can't open file : " + path
    if (!imageMap.Serialize(fp,::eImageMapSerializeFlags.Write)) {
      ::logError("Couldn't save ImageMap to '"+path+"'")
    }
    else {
      ::log("Saved ImageMap to '"+path+"'")
    }
    return true
  }

  function toggleDrawOpCapture(aCtx) {
    aCtx = aCtx || ::gUIContext
    if (aCtx.draw_op_capture) {
      aCtx.draw_op_capture = false
    }
    else {
      aCtx.draw_op_capture = true
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////
} // end of gui

if (::lang.getHostOS() == "osx") {
  ::gui.keyModToString <- function(aKey,aKeyMod) {
    switch (aKey) {
      // macOS "system" shortcuts always show as "Ctrl" instead of "Cmd" to
      // avoid confusion.
      case ::eKey.Q:
      case ::eKey.W:
      case ::eKey.Space:
          return ::FlagsToString(aKeyMod&~0xFF, ::eKeyMod).replace("Control","Ctrl")
      default:
          return ::FlagsToString(aKeyMod&~0xFF, ::eKeyMod).replace("Control","Cmd")
    }
  }
}
else {
  ::gui.keyModToString <- function(aKey,aKeyMod) {
    return ::FlagsToString(aKeyMod&~0xFF, ::eKeyMod).replace("Control","Ctrl")
  }
}
//////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------
//
//  Loading Overlay
//
//--------------------------------------------------------------------------------------------
::tLoadingOverlay <- {
  _speed = 10.0
  _overlay = null

  function new(path) {
    local t = this.Clone()
    path = path || "niUI://loading.tga"
    t._speed <- 10.0
    t._overlay <- ::gGraphics.CreateOverlayResource(path)
    if (t._overlay) {
      t._overlay.filtering = true
      t._overlay.blend_mode = ::eBlendMode.PreMulAlpha
    }
    return t
  }
  function draw(canvas,r) {
    if (_overlay) {
      local sz = _overlay.size
      local pos = r.top_left + (r.size / 2)
      local mtx = ::Matrix();
      mtx *= ::gMath.MatrixTranslation(-::Vec3(sz.x/2,sz.y/2,0));
      mtx *= ::gMath.MatrixRotationZ(-::gMath.WrapRad(::gLang.TimerInSeconds()*_speed));
      mtx *= ::gMath.MatrixTranslation(::Vec3(pos.x,pos.y,0));
      local wasMatrix = canvas.matrix
      for (local i = 0; i < 1; ++i) {
        canvas.matrix = mtx
        canvas.BlitOverlay(
          ::Rect(0,0,sz.x,sz.y)
          _overlay)
      }
      canvas.matrix = wasMatrix
    }
  }
}

//--------------------------------------------------------------------------------------------
//
//  Base Sink
//
//--------------------------------------------------------------------------------------------
::gui.baseWidgetSink <- {
  __debug_name = "baseWidgetSink"
  __jumptable = {}

  function __Dummy(aE,aA,aB) {
    return false
  }
  function __DummyCommand(aWidget,aPA) {
    return false
  }
  function __DummyDefault(aWidget,aMsg,aPA,aPB) {
    return false
  }
  function __Command(aWidget,aPA,aPB) {
    local ret = OnCommand(aWidget,aPA);
    if (ret == false) {
      local sender = aPA.sender
      if (sender.?id) {
        local id = sender.id
        if (id in this) {
          try {
            ret = this[id].call(this,aWidget,aPA)
          }
          catch(e) {
            // if an exception has been thrown we replace the handler
            // by a dummy
            this[id] <- ::gui.baseWidgetSink.__DummyCommand
            throw "Error in UI Command Handler '"+id+"' :\n" + e
          }
        }
      }
    }
    return ret
  }

  function OnWidgetSink(aWidget,aMsg,aPA,aPB) {
    local ret
    try
    {
      if (aMsg in __jumptable) {
        local v = __jumptable.rawget(aMsg)
        if (v in this) {
          ret = this[v].call(this,aWidget,aPA,aPB)
        }
        else {
          ret = false
        }
      }
      else {
        ret = OnDefault(aWidget,aMsg,aPA,aPB);
      }
    }
    catch (e) {
      if (aMsg in __jumptable) {
        local v = __jumptable.rawget(aMsg)
        this[v] <- ::gui.baseWidgetSink.__Dummy
      }
      else {
        this["OnDefault"] <- ::gui.baseWidgetSink.__DummyDefault
      }
      local msgName = ::EnumToString(aMsg, ::eUIMessage)
      throw "Error in UI Message Handler "+aMsg+" ("+msgName+") :\n" + e
    }
    return ret==null?false:ret
  }
}

// initialize the base widget sink
function initBaseWidgetSinkTable(sink) {
  //  ::printdebugln("SINK TABLE REGISTER")
  local et = ::eUIMessage.gettable()
  foreach (key,val in et) {
    local v = "On"+key
    sink.__jumptable[val] <- v
    //sink[v] <- sink._Dummy
  }
  sink.__jumptable[::eUIMessage.Command] = "__Command"
  sink["OnCommand"] <- sink.__DummyCommand
  sink["OnDefault"] <- sink.__DummyDefault
  //  foreach (key,val in sink.__jumptable) {
  //    ::printdebugln(::format("REG: %d - %s (%s)",key,val,et.getkey(key)))
  //  }
}
initBaseWidgetSinkTable(::gui.baseWidgetSink);

//////////////////////////////////////////////////////////////////////////////////////////////
// Interop
::interop.UIWidget <- #(path,iid) => ::interop.doEval(path,iid,::gui.baseWidgetSink)
