::Import("lang.ni")
::Import("fs.ni")
::Import("seq.ni")
::Import("niUI")

mInLogSink <- false
mIsEvaluating <- false
mListBox <- null
mExprEditor <- null
mExprResult <- null
mColors <- {
  [::eLogFlags.Raw] = ::eColor.lightgrey,
  [::eLogFlags.Debug] = ::eColor.palegreen,
  [::eLogFlags.Error] = ::eColor.yellow,
  [::eLogFlags.Warning] = ::eColor.darkorange
}
mwDest <- null
mLastExprEvaled <- ""

if (!("ExpressionEditor_evalExpression" in ::getroottable())) {
  ::ExpressionEditor_evalExpression <- null
}

function OnSinkAttached(w,a,b) {
  // ::dbg("... OnSinkAttached")
  mExprEditor = w.FindWidget("ID_ExpressionEditor");
  mListBox = w.FindWidget("ID_ExpressionEditorLog");
  mExprResult = w.FindWidget("ID_ExpressionResult");
  ::gLang.system_message_handlers.AddSink(w)
  mExprEditor.SetFocus()
}

function OnDestroy(w,a,b) {
  ::gLang.system_message_handlers.RemoveSink(w)
}

function _evalExpression(exprText, _abForceEval) {
  if (!_abForceEval && (exprText == mLastExprEvaled)) {
    _addMessage(null, "Expression already evaluated: " + exprText);
    return
  }
  mLastExprEvaled = exprText;

  _addMessage(null, "Evaluating: " + exprText);
  local evaled;
  mIsEvaluating = true;
  try {
    if (::ExpressionEditor_evalExpression) {
      evaled = ::ExpressionEditor_evalExpression(exprText);
    }
    else {
      evaled = ::gLang.Eval(exprText);
    }
  }
  catch (e) {
  }
  mIsEvaluating = false;

  if (!evaled) {
    _addMessage(::eLogFlags.Error, "Expression evaluation failed.")
    mExprResult.text = "#ERROR#"
  }
  else {
    local name = evaled.GetName()
    local type = ::EnumToString(evaled.type, ::eExpressionVariableType)
    local flags = ::FlagsToString(evaled.flags, ::eExpressionVariableFlags)

    local result = ""
    if (name.?len()) {
      result += name;
    }
    if (evaled.string.?len()) {
      if (name.?len()) {
        result += " "
      }
      result += "= "
      result += evaled.string
    }

    _addMessage(::eLogFlags.Raw,
                (name.?len() > 0 ? ("name: " + name + ", ") : "") +
                (type.?len() > 0 ? ("type: " + type + ", ") : "") +
                (flags.?len() > 0 ? ("flags: " + flags + ", ") : "") +
                (result.?len() > 0 ? ("result: " + result + "\n") : ""))
    mExprResult.text = result

    if (result != "EVALERR") {
      // Expression was evaluated successfully, so we'll update it
      mLastExprEvaled = mLastExprEvaled.replace("\n"," ")
      mwDest.?SendMessage(10001, mLastExprEvaled, null);
    }
  }
}

function ID_ExpressionEditor(w,aCmd) {
  if (aCmd.id == ::eWidgetEditBoxCmd.Validated) {
    local exprText = mExprEditor.text
    if (mLastExprEvaled != exprText) {
      _evalExpression(exprText)
    }
  }
}

function ID_ExpressionEvalBt(w,aCmd) {
  if (aCmd.id == ::eWidgetButtonCmd.Clicked) {
    local exprText = mExprEditor.text
    _evalExpression(exprText, true)
  }
}

function handleSetExpression(awDest,aNewExpr) {
  mwDest = awDest
  mExprEditor.text = aNewExpr
  _evalExpression(aNewExpr)
}

function handleSystemLog(logType, msg) {
  if (mInLogSink || !mIsEvaluating)
    return
  mInLogSink = true
  try {
    _addFilteredMessage(logType, msg)
  }
  catch (e) {
  }
  mInLogSink = false
}

function OnDefault(aWidget,aMsg,aA,aB) {
  switch (aMsg) {
    case 10000: {
      handleSetExpression(aA,aB)
      break;
    }
    case ::eSystemMessage.Log: {
      handleSystemLog(aA,aB)
      break;
    }
  }
}

function _addLine(type,line) {
  local itemIndex = mListBox.AddItem(line)
  local color = mColors[?type]
  if (color) {
    mListBox.SetItemTextColor(invalid, itemIndex, color)
  }
}

///////////////////////////////////////////////
function _addMessage(type,msg) {
  if (msg.contains("\n")) {
    msg = msg.trimrightex("\n")
    if (msg.empty())
      return;
    local fp = ::fs.createStringFile(msg)
    foreach (line in ::seq.fileLines(fp)) {
      _addLine(type,line)
    }
  }
  else {
    if (msg.empty())
      return;
    _addLine(type,msg)
  }
  mListBox.selected = mListBox.num_items-1
  mListBox.AutoScroll()
}

///////////////////////////////////////////////
function _addFilteredMessage(type,msg) {
  local color = mColors[?type]
  local logMsg = false;
  if (msg.icontains("expression.cpp")) {
    logMsg = true;
    if (msg.contains("] (")) {
      msg = msg.rbefore("[")
    }
  }
  if (logMsg) {
    _addMessage(type,msg)
  }
}
