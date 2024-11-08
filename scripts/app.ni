// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

::Import("lang.ni")
::Import("gui.ni")
::Import("console.ni")
::Import("loading_queue.ni")
::Import("sexp.ni")
::Import("app_config.ni")

if (!::gUIContext) {
  throw "No UIContext initialized before starting a hosted app."
}

::gAppConfig <- ::tAppConfig.Clone()

::app <- {
  mName = "niApp"
  mStarted = false
  mbExiting = false
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
  function registerCppScriptingHost() {
    local shName = "niScriptCpp.ScriptingHost";
    if (::gLang.GetGlobalInstance(shName))
      return; // already registered
    ::log("Registering Cpp ScriptingHost.");
    ::Import("niScriptCpp");
    local sh = ::CreateGlobalInstance(shName);
    ::gLang.AddScriptingHost("cpp",sh);
    ::gLang.AddScriptingHost("cpp2",sh);
    ::gLang.AddScriptingHost("cni",sh);
    // enable Cpp compilation
    ::gLang.SetProperty("niScriptCpp.Compile", "1");
  }

  ///////////////////////////////////////////////
  function startFormApp(aFormPath,_aDir) {
    local loadForm = function() : (aFormPath,registerCppScriptingHost) {
      local loadingStart = ::clock()
      ::log("Loading form app '" + aFormPath + "'.")

      // Load the form
      local formPath = aFormPath
      local formDT = ::lang.loadDataTable("xml",formPath)

      local formCode = formDT.string.code
      if (formCode.?len() &&
          (formCode.contains(".cpp") ||
           formCode.contains(".cni"))) {
        registerCppScriptingHost();
      }

      local formTitle = formDT.string.title
      if (formTitle.empty()) {
        formTitle = "FormApp"
      }
      ::gLang.property["ni.app.name"] = formTitle
      ::app.setTitle(formTitle + ::lang.getTitleBuildDesc())

      if (::?gFormWidget) {
        ::?gFormWidget.?Invalidate()
      }

      // Create the form
      ::gFormWidget <- ::gUIContext.CreateWidgetFromDataTable(formDT,::gUIContext.root_widget,null,null)
      if (!::gFormWidget) {
        throw "Can't create main form widget."
      }

      // Fill the form if its a canvas and that no dock_style has been defined
      if (formDT.name == "Canvas") {
        if (!formDT.HasProperty("dock_style")) {
          ::gFormWidget.dock_style = ::eWidgetDockStyle.DockFill
        }
      }

      if (!::gFormWidget.HasChild(::gUIContext.GetFocusedWidget(),true)) {
        ::gFormWidget.SetFocus();
      }

      ::log(::format("Done, loaded form app '%s' in %.6fs",
                     aFormPath, ::clock()-loadingStart))
    }

    local loadConfig = function() : (aFormPath) {
      // Load configs from the form's xml
      local formDT = ::lang.loadDataTable("xml",aFormPath)
      if (formDT.vec2.window_size.x > 0) {
        ::gAppConfig.window.size <- formDT.vec2.window_size;
      }
      if (formDT.vec2.window_position.x > 0) {
        ::gAppConfig.window.position <- formDT.vec2.window_position;
      }
    }

    local reloadApp = mStarted;
    if (!reloadApp) {
      if (::?gResources && _aDir.?len()) {
        ::?gResources.AddSource(_aDir)
      }

      loadConfig();

      // Queue the loading method...
      ::app.queueStartup([
        "Loading Form.",
        loadForm
      ])
      // Startup the application
      ::app.startup()
    }
    else {
      loadConfig();
      ::app.queueStartup([
        "Reloading Form.",
        loadForm,
        null // done
      ])
    }
  }

  ///////////////////////////////////////////////
  mConfigDir = null

  function setConfigDir(aDir) {
    mConfigDir = "".setdir(aDir);
    ::gRootFS.FileMakeDir(mConfigDir)
    if (!::fs.dirExists(mConfigDir)) {
      throw "Config directory doesn't exist and can't be created: '" + mConfigDir + "'"
    }
  }

  function setConfigDirOnce(aDir) {
    if (mConfigDir == null) {
      setConfigDir(aDir)
    }
  }

  function getConfigDir() {
    if (mConfigDir == null) {
      local appName = ::lang.getProperty("ni.app.name")
      if (!appName.len())
        throw "getConfigDir: Can't get ni.app.name for the default config directory."
      setConfigDir(
        "".setdir(::lang.getProperty("ni.dirs.config")).adddirback(appName))
    }
    return mConfigDir
  }

  function getConfigPath(aName) {
    if (!aName.?len())
      throw "getConfigPath: aName not specified."
    return getConfigDir().setfile(aName).setext("appconfig.xml")
  }

  function loadConfig(aName) {
    local path = getConfigPath(aName)
    if (path && ::fs.fileExists(path)) {
      local dt = ::lang.loadDataTable("xml",path)
      ::log("Loaded config '" + aName + "' from '" + path + "'.")
      return dt
    }
    return null
  }

  function writeConfig(aDT,aName) {
    local path = getConfigPath(aName);
    ::gRootFS.FileMakeDir(path.getdir())
    ::lang.writeDataTable("xml", aDT, path)
    ::log("Wrote config '" + aName + "' to '" + path + "'.")
  }

  ///////////////////////////////////////////////
  function setTitle(aTitle) {
    mMainWindow.title = aTitle
  }

  ///////////////////////////////////////////////
  function useLoadSaveImageMap() {
    return ::gAppConfig.?misc.?load_save_image_map
  }
  function logMessage(astrMsg) {}

  ///////////////////////////////////////////////
  function setWindowPositionAndSizeFromConfig() {
    if (::gAppConfig.window.size) {
      mMainWindow.size = ::gAppConfig.window.size
    }
    if (::gAppConfig.window.position) {
      mMainWindow.position = ::gAppConfig.window.position
    }
    else {
      // Center the window if no position has been explicitly specified
      mMainWindow.CenterWindow()
    }
    if (::gAppConfig.window.fullscreen) {
      setFullScreen(::gAppConfig.window.fullscreen);
    }
    if (::gAppConfig.window.show_flags) {
      mMainWindow.show = ::gAppConfig.window.show_flags
    }
  }

  ///////////////////////////////////////////////
  function startup() {
    if (mStarted)
      return true;

    mStarted = true;

    ::gLang.property["ni.app.name"] = mName

    ::printdebugln("--- APP STARTUP BEGIN: "+::lang.getHostOS()+
                   ", isEmbedded: "+::lang.isEmbedded()+" ---")

    ::printdebugln("--- Get Existing Objects ---")

    /// Setup the window
    mMainWindow = ::gLang.global_instance["iOSWindow"].?QueryInterface("iOSWindow")
    if (!mMainWindow)
      throw "Can't get existing iOSWindow."

    mMainWindow.message_handlers.AddSink(this)
    ::gWindow <- mMainWindow
    if (::gAppConfig.misc.drop_target) {
      if (mMainWindow)
        mMainWindow.drop_target = true
    }
    setWindowPositionAndSizeFromConfig();

    // Initialize the root widget
    ::gUIContext.root_widget.AddSink(this)

    // Initialize the loading queue
    if ("loading_screen" in ::gAppConfig.misc) {
      mLQ.initialize(::gGraphicsContext)
    }
    else {
      mLQ.initialize(::gGraphicsContext)
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
        ::gUIContext.default_skin = ::gAppConfig.misc.ui_skin
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
      ::gGraphicsContext = ::LINT_AS_TYPE("iGraphicsContext",null);
    }
    if (::?gResources) {
      ::?gResources = null
    }
    ::gGraphics = ::LINT_AS_TYPE("iGraphics",null);
    ::gUIContext = ::LINT_AS_TYPE("iUIContext",null);
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
  function isFullScreen() bool {
    if (::gAppConfig.misc.can_fullscreen) {
      return ::gWindow.full_screen != invalid
    }
    else {
      return false
    }
  }
  function setFullScreen(abFullScreen) bool {
    if (!::gAppConfig.misc.can_fullscreen)
      return false
    ::gWindow.full_screen = abFullScreen ? ::gWindow.monitor : invalid
    return true
  }
  function toggleFullScreen() void {
    setFullScreen(!isFullScreen())
  }

  ///////////////////////////////////////////////
  function queueLoading(aFunc,_aThis) void {
    mLQ.queue(aFunc,_aThis)
  }
  function queueStartup(aFunc,_aThis) void {
    if (mStartupQueue) {
      if (_aThis) {
        mStartupQueue.append(::closure.new(_aThis,aFunc))
      }
      else {
        mStartupQueue.append(aFunc)
      }
    }
    else {
      queueLoading(aFunc,_aThis)
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
    local startApp = ::sexp.utils.getAfter(::?GetArgs(),"-startApp","")
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
        ::console.registerCommand("Global","ReloadSkin","", #{
          ::app.queueLoading([
            "Reloading Skin."
            #{ ::gui.reloadSkin() }
            null
          ]);
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
