// Copyright 2007-2016 TalanSoft, Co. All Rights Reserved.
::Import("console.ni")
::Import("fs.ni")
::Import("seq.ni")
::Import("algo.ni")

mInLogSink <- false
mListBox <- null
mCmdLine <- null
mColors <- {
  [::eLogFlags.Raw] = ::eColor.lightgrey,
  [::eLogFlags.Debug] = ::eColor.palegreen,
  [::eLogFlags.Error] = ::eColor.yellow,
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
  mCmdLine.style |= ::eWidgetStyle.NotifyParent

  ::gLang.system_message_handlers.AddSink(w)

  ::log("Console form opened.")

  local formId = w.id
  local logId = mListBox.id
  ::console.registerNS("Console")
  ::console.registerCommand(
    "Console","Clear","Clear the console",
    function(args) : (formId, logId) {
      local wForm = ::gUIContext.root_widget.FindWidget(formId)
      local wLog = wForm.FindWidget(logId)
      wLog.ClearItems()
    })

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

function OnSetFocus(w,a,b) {
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
function OnNotify(w,a,msg) {
  if (a == mCmdLine) {
    if (msg.id == ::eUIMessage.KeyDown) {
      switch (msg.a) {
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
}

///////////////////////////////////////////////
mMaxHistoryItems <- 50
mHistoryCurrent <- 0
mHistory <- []

function cmdRun() {
  local cmd = mCmdLine.text
  if (!cmd.empty()) {
    addMessage(::eLogFlags.Raw,"]" + cmd)
    ::gConsole.RunCommand(cmd)
    mCmdLine.text = ""
    addHistoryItem(cmd)
  }
}

function cmdPrev() {
}

function cmdNext() {
  setHistoryItem(1)
}

function setHistoryItem(aDelta) {
  if (mHistory.empty())
    return
  mHistoryCurrent += aDelta
  if (mHistoryCurrent < 0)
    mHistoryCurrent = mHistory.len()-1
  else if (mHistoryCurrent >= mHistory.len())
    mHistoryCurrent = 0
  mCmdLine.text = mHistory[mHistoryCurrent]
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
