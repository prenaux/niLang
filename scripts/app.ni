// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

::Import("lang.ni")
::Import("gui.ni")
::Import("console.ni")
::Import("loading_queue.ni")
::Import("sexp.ni")

if (!::gUIContext) {
  throw "No UIContext initialized before starting a hosted app."
}

::app <- {
  mName = "niApp"
  mStarted = false
  mbExiting = false
  mConfigFile = "app_config.ni"
  mIsNullRenderer = true // by default no renderer...
  mMainWindow = null
  mDebugDumpedFPS = 0

  // Loading queue
  mLQ = ::loading_queue.Clone()
  mStartupQueue = []

  ///////////////////////////////////////////////
  // Called when a blocking process wants progress to be reported
  function updateProgress() {
  }

  ///////////////////////////////////////////////
  // Called after the basic startup to init the application
  function appStartup() {
  }

  ///////////////////////////////////////////////
  // Called before the final shutdown to cleanup the application
  function appShutdown() {
  }

  ///////////////////////////////////////////////
  // Called when the Application is update, after the standard updates
  function appUpdate() {
  }

  ///////////////////////////////////////////////
  function startFormApp(aFormPath,aDir,aConfig) {
    local load = function() : (aFormPath,aDir,aConfig) {
      // Load the form
      local formPath = aFormPath
      local formDT = ::lang.loadDataTable("xml",formPath)
      local formTitle = formDT.string.title
      if (formTitle.empty()) {
        formTitle = "FormApp"
      }
      ::gLang.property["ni.app.name"] = formTitle
      ::app.setTitle(formTitle + ::lang.getTitleBuildDesc())

      if ("gFormWidget" in ::getroottable()) {
        ::gFormWidget.?Invalidate()
      }

      // Create the form
      ::gFormWidget <- ::gUIContext.CreateWidgetFromDataTable(formDT,::gUIContext.root_widget,null,null)

      // Fill the form if its a canvas and that no dock_style has been defined
      if (formDT.name == "Canvas") {
        if (!formDT.HasProperty("dock_style")) {
          ::gFormWidget.dock_style = ::eWidgetDockStyle.DockFill
        }
      }

      if (!::gFormWidget.HasChild(::gUIContext.GetFocusedWidget(),true)) {
        ::gFormWidget.SetFocus();
      }

      ::log("Loaded form app:" aFormPath)
    }

    local loadConfig = function() : (aFormPath) {
      // Load configs from the form's xml
      local formDT = ::lang.loadDataTable("xml",aFormPath)
      if (formDT.vec2.window_size.x > 0) {
        ::gConfig.window.size <- formDT.vec2.window_size;
      }
      if (formDT.vec2.window_position.x > 0) {
        ::gConfig.window.position <- formDT.vec2.window_position;
      }
    }

    local reloadApp = (("gFormWidget" in ::getroottable()) && ::gFormWidget != null)
    if (!reloadApp) {
      //
      // This is called once at startup
      //
      if (typeof(aConfig) == "string") {
        ::gConfig <- {}
        ::Import(aConfig,::gConfig)
      }
      else if (typeof(aConfig) == "table") {
        ::gConfig <- aConfig
      }
      else if (aConfig == null) {
        ::gConfig <- {}
        ::Import("app_config.ni",::gConfig)
      }
      else {
        throw "Invalid aConfig parameter."
      }

      if (::?gResources && aDir && aDir.len()) {
        ::gResources.AddSource(aDir)
      }

      loadConfig();

      // Queue the loading method...
      ::app.queueStartup(load)
      // Startup the application
      ::app.startup(::gConfig)
    }
    else {
      loadConfig();
      // setWindowPositionAndSizeFromConfig(); // I dont think we want to reset the window post when reloading an app we're working on
      ::app.queueStartup(load)
    }
  }

  function getConfigPath() {
    return ::gLang.property["ni.dirs.home"]+"niApp/config/"+
      ::gLang.property["ni.app.name"]+".config.xml"
  }

  ///////////////////////////////////////////////
  function setTitle(aTitle) {
    mMainWindow.title = aTitle
  }

  ///////////////////////////////////////////////
  function useLoadSaveImageMap() {
    return (("load_save_image_map" in ::gConfig.misc) && ::gConfig.misc.load_save_image_map)
  }
  function logMessage(astrMsg) {}

  ///////////////////////////////////////////////
  function setWindowPositionAndSizeFromConfig() {
    if (::gConfig.window.size) {
      mMainWindow.size = ::gConfig.window.size
    }
    if (::gConfig.window.position) {
      mMainWindow.position = ::gConfig.window.position
    }
    else {
      // Center the window if no position has been explicitly specified
      mMainWindow.CenterWindow()
    }
    if (::gConfig.window.fullscreen) {
      setFullScreen(::gConfig.window.fullscreen);
    }
    if (::gConfig.window.show_flags) {
      mMainWindow.show = ::gConfig.window.show_flags
    }
  }

  ///////////////////////////////////////////////
  function startup(aConfig) {
    if (mStarted)
      return true;

    mStarted = true;

    ::gLang.property["ni.app.name"] = mName

    ::printdebugln("--- APP STARTUP BEGIN: "+::lang.getHostOS()+
                   ", isEmbedded: "+::lang.isEmbedded()+" ---")

    if (aConfig) mConfigFile = aConfig

    // Load the configuration
    if (typeof(mConfigFile) == "table") {
      ::gConfig <- mConfigFile;
    }
    else {
      ::gConfig <- {}
      ::Import(mConfigFile,::gConfig)
    }

    ::printdebugln("--- Get Existing Objects ---")

    /// Setup the window
    mMainWindow = ::gLang.global_instance["iOSWindow"]
    if (!mMainWindow)
      throw "Can't get existing iOSWindow."

    mMainWindow.message_handlers.AddSink(this)
    ::gWindow <- mMainWindow
    if (::gConfig.misc.drop_target) {
      if (mMainWindow)
        mMainWindow.drop_target = true
    }
    setWindowPositionAndSizeFromConfig();

    // Check the type of renderer...
    mIsNullRenderer = (::gGraphicsContext.driver.name == "NULL")

    // Initialize the root widget
    ::gUIContext.root_widget.AddSink(this)

    // Initialize the loading queue
    if ("loading_screen" in ::gConfig.misc) {
      mLQ.initialize(::gGraphicsContext,::gConfig.misc.loading_screen)
    }
    else {
      mLQ.initialize(::gGraphicsContext,"niUI://loading.tga")
    }
    mLQ.queue("Loading")
    mLQ.mFont = ::gGraphics.font_from_name["Default"].CreateFontInstance(null)
    mLQ.mFont.SetSizeAndResolution(
      ::Vec2(14,14) * mMainWindow.contents_scale, // the loading queue draws in device space
      14,
      mMainWindow.contents_scale)

    //// Load/save image map bitmap... ////
    mLQ.queue(function() {
      if (::app.useLoadSaveImageMap()) {
        if (!::gui.loadImageMap(null,null)) {
          ::logError("The default UI image map is corrupted.")
        }
      }
      try {
        ::gUIContext.default_skin = ::gConfig.misc.ui_skin
      } catch (e) {}
    })

    //// App initialization ////
    ::app.appStartup()

    ///// Sink callbacks ////
    mLQ.queue(mStartupQueue)
    mStartupQueue = null

    /// Setup the other stuff...
    queueLoading(null)

    ::printdebugln("--- APP STARTUP QUEUED ---");
    return true
  }

  ///////////////////////////////////////////////
  function shutdown() {
    ::printdebugln("--- APP SHUTDOWN BEGIN ---")
    if (useLoadSaveImageMap()) {
      ::gui.saveImageMap(null,null)
    }
    ::app.appShutdown()
    if (::lang.isValid(::gUIContext)) {
      ::gUIContext.Invalidate()
    }
    if (::gGraphicsContext) {
      ::gGraphicsContext.Invalidate()
      ::gGraphicsContext = null;
    }
    if (::?gResources) {
      ::gResources = null
    }
    ::gGraphics = null
    ::gUIContext = null
    mStarted = false
    local collected = ::lang.gc()
    ::printdebugln("--- APP SHUTDOWN END (GC:"+collected+") ---")
  }

  ///////////////////////////////////////////////
  function GetThreadID() {
    return ::GetMainThreadID();
  }

  ///////////////////////////////////////////////
  function HandleMessage(aMsg,aA,aB) {
    if (aMsg == ::eOSWindowMessage.Drop) {
      local w = ::gUIContext.root_widget.FindWidgetByPos(aB)
      if (w && (w.style&::eWidgetStyle.DragDestination)) {
        local text = "";
        local dt = ::gLang.CreateDataTable("DropFiles")
        dt.string.type = "drop"
        foreach (v in aA) {
          local fdt = ::gLang.CreateDataTable("File")
          fdt.string.path = v
          if (!text.empty()) text += ";"
          text += v
          dt.AddChild(fdt)
        }
        dt.string.text = text
        ::dbg("DROPPED '"+text+"' on "+(w?w.id:"FO"))
        if (w) {
          w.SendMessage(::eUIMessage.Paste,dt,null)
        }
        ::gWindow.ActivateWindow();
      }
    }
    return false;
  }

  ///////////////////////////////////////////////
  function isFullScreen() {
    if (::gConfig.misc.can_fullscreen) {
      return ::gWindow.full_screen != invalid
    }
    else {
      return false
    }
  }
  function setFullScreen(abFullScreen) {
    if (!::gConfig.misc.can_fullscreen)
      return false
    ::gWindow.full_screen = abFullScreen ? ::gWindow.monitor : invalid
  }
  function toggleFullScreen() {
    setFullScreen(!isFullScreen(),true)
  }

  ///////////////////////////////////////////////
  function queueLoading(aFunc,aThis) {
    mLQ.queue(aFunc,aThis)
  }
  function queueStartup(aFunc,aThis) {
    if (mStartupQueue) {
      if (aThis) {
        mStartupQueue.append(::closure.new(aThis,aFunc))
      }
      else {
        mStartupQueue.append(aFunc)
      }
    }
    else {
      queueLoading(aFunc,aThis)
    }
  }

  ///////////////////////////////////////////////
  // Root widget sink...
  function OnWidgetSink(aWidget,aMsg,aPA,aPB) {
    switch (aMsg) {
      case ::eUIMessage.ContextUpdate: {
        local frameTime = aPA.tofloat()
        ::app.appUpdate()
        break;
      }
      case ::eUIMessage.ContextAfterDraw: {
        mLQ.draw(aWidget.uicontext.contents_scale)
        return false;
      }
    }
    return false;
  }

  ///////////////////////////////////////////////
  function main() {
    // Load from startApp argument
    local startApp = ::sexp.utils.getAfter(::GetArgs(),"-startApp","")
    if (!("reload" in ::getroottable()) && startApp.?len()) {
      local appType = startApp.getext().ToLower();
      switch (appType) {
        case "niui":
        case "gml": {
          ::reload <- function() : (startApp) {
            ::app.startFormApp(startApp)
          }
          break;
        }
        default: {
          throw "Unknown application type '" + appType + "'."
        }
      }
      if (!("reload" in ::getroottable())) {
        ::reload <- function() : (startApp) {
          ::gui.dialogErrorMessage("Error","Unknown application type : "+startApp)
        }
      }
      else {
        ::reload()
      }

      ::app.queueStartup(#{
        ::console.registerCommand("Global","Reload","", ::reload, ::eKey.F9);
        ::console.registerCommand("Global","ReloadSkin","", function() {
          ::gui.reloadSkin();
        }, ::eKey.F2);
        ::console.registerCommand("Global","DebugUI","", function() {
          ::gUIContext.debug_draw = !::gUIContext.debug_draw
        },::eKey.F1);
      });
    }

    return ::app.startup()
  }
}

// Main entry point
::main <- ::app.main
