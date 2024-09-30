// Copyright 2007-2016 TalanSoft, Co. All Rights Reserved.
::Import("console.ni")
::Import("fs.ni")
::Import("seq.ni")
::Import("algo.ni")
::Import("gui.ni")

mInLogSink <- false
mListBox <- null
mCmdLine <- null
mColors <- {
  [::eLogFlags.Raw] = ::eColor.lightgrey,
  [::eLogFlags.Debug] = ::eColor.palegreen,
  [::eLogFlags.Error] = ::eColor.red,
  [::eLogFlags.Warning] = ::eColor.darkorange
}

function OnSinkAttached(w,a,b) {
  mListBox = w.FindWidget("ID_Log");
  mListBox.max_num_items = 0xFFFF

  local lastLogs = ::GatherLastLogs(200);
  addMessage(::eLogFlags.Info, "... added last logs: " + lastLogs.GetSize())
  foreach (msg in lastLogs) {
    local logType = ::eLogFlags.Raw
    if (msg.startswith("I/")) logType = ::eLogFlags.Info
    else if (msg.startswith("D/")) logType = ::eLogFlags.Debug
    else if (msg.startswith("E/")) logType = ::eLogFlags.Error
    else if (msg.startswith("W/")) logType = ::eLogFlags.Warning
    addMessage(logType,msg)
  }

  mCmdLine = w.FindWidget("ID_CmdLine")
  mCmdLine.AddSink({
    _thisOnCmdLineKeyDown = ::closure.new(this, OnCmdLineKeyDown)

    function OnMoveFocus(w,a,b) {
      // ::println("... OnMoveFocus");
      // capture the focus so that we can use tab to complete commands
      return true;
    }
    function OnKeyDown(w,a,b) : (OnCmdLineKeyDown) {
      // ::println("... OnKeyDown:" a);
      return _thisOnCmdLineKeyDown(w,a,b);
    }
  }.SetDelegate(::gui.baseWidgetSink));

  ::gLang.system_message_handlers.AddSink(w)

  ::log("Console form opened.")

  local formId = w.id
  local logId = mListBox.id
  ::console.registerNS("Terminal", {
    Clear = {
      _desc = "Clear the console",
      _func = function(args) : (formId, logId) {
        local wForm = ::gUIContext.root_widget.FindWidget(formId)
        local wLog = wForm.FindWidget(logId).QueryInterface("iWidgetListBox")
        wLog.ClearItems()
      }
    }
  }, true)

  w.Place(w.rect, ::eWidgetDockStyle.SnapTopRight, ::Vec4(5,5,5,5));
}

function OnDestroy(w,a,b) {
  ::gLang.system_message_handlers.RemoveSink(w)
  ::gUIContext.SetShowTerminal(false)
}

function OnSkinChanged(w,a,b) {
  mCmdLine.font = mListBox.font
}

function OnDefault(aWidget,aMsg,aA,aB) {
  if (aMsg != ::eSystemMessage.Log)
    return
  if (mInLogSink)
    return
  mInLogSink = true
  try {
    local logType = aA
    local msg = aB
    addMessage(logType, msg)
  }
  catch (e) {
  }
  mInLogSink = false
}

function OnSetFocus(w,_a,_b) {
  local cmdLine = w.FindWidget("ID_CmdLine")
  if (w.visible && cmdLine)
    cmdLine.SetFocus()
}

function OnVisible(w,a,b) {
  if (a) {
    OnSetFocus(w)
  }
}

///////////////////////////////////////////////
function OnCmdLineKeyDown(w,a,b) {
  if (a == ::eKey.Tab) {
    // ::println("... Tab: cmdComplete");
    cmdComplete();
  }
  else {
    cmdClearComplete();
    switch (a) {
      case ::eKey.Up: {
        setHistoryItem(-1)
        break;
      }
      case ::eKey.Down: {
        setHistoryItem(1)
        break;
      }
      case ::eKey.NumPadEnter:
      case ::eKey.Enter: {
        cmdRun();
        // fallthrough
      }
      default: {
        mHistoryCurrent = mHistory.len()
        break;
      }
    }
  }
}

///////////////////////////////////////////////
mMaxHistoryItems <- 50
mHistoryCurrent <- 0
mHistory <- []
mToComplete <- null

function cmdRun() {
  local cmd = mCmdLine.text
  if (!cmd.empty()) {
    addMessage(::eLogFlags.Raw,"]" + cmd)
    ::gConsole.RunCommand(cmd)
    mCmdLine.text = ""
    addHistoryItem(cmd)
  }
}

function cmdClearComplete() {
  mToComplete = null
}

function cmdComplete() {
  if (!mToComplete) {
    mToComplete = mCmdLine.text
  }
  // ::println("... cmdComplete:" mToComplete)
  local completedCommand = ::gConsole.CompleteCommandLine(
    mToComplete, true);
  if (!completedCommand.empty()) {
    mCmdLine.text = completedCommand
    mCmdLine.MoveCursorEnd(true)
  }
}

function setHistoryItem(aDelta) {
  if (mHistory.empty())
    return
  mHistoryCurrent += aDelta
  if (mHistoryCurrent < 0)
    mHistoryCurrent = mHistory.len()-1
  else if (mHistoryCurrent >= mHistory.len())
    mHistoryCurrent = 0
  mCmdLine.text = mHistory[?mHistoryCurrent]
  mCmdLine.MoveCursorEnd(true)
}

function addHistoryItem(cmd) {
  if (!::algo.contains(mHistory,cmd)) {
    while (mHistory.len() >= mMaxHistoryItems) {
      mHistory.remove(0)
    }
    mHistory.Add(cmd)
    mHistoryCurrent = mHistory.len()
  }
}

///////////////////////////////////////////////
function addMessage(type,msg) {
  local color = mColors[?type]
  local fp = ::fs.createStringFile(msg)
  if (msg.endswith("\n"))
    fp.Resize(fp.size-1)
  foreach (line in ::seq.fileLines(fp)) {
    local itemIndex = mListBox.AddItem(line)
    if (color) {
      mListBox.SetItemTextColor(invalid, itemIndex, mColors[type])
    }
  }
  mListBox.selected = mListBox.num_items-1
  mListBox.AutoScroll()
}
