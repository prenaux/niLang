::NewImport("gui.ni");

function OnSinkAttached(w,a,b) {
}

function ID_ExpressionEditorBt(w,cmd) if (cmd.id == ::eWidgetButtonCmd.Clicked) {
  ::dbg("... ID_ExpressionEditorBt")
  ::ExpressionEditor_evalExpression <- function(exprText) {
    ::log("MY EVALUATOR: " + exprText);
    return ::gLang.Eval(exprText);
  }
  local wForm = ::gui.createDropDown(cmd.sender,"script://forms/ExpressionEditor.gml")
  wForm.SendMessage(
    10000,w,w.FindWidget("ID_ExpressionEditorText").text)
}

function OnDefault(w,msgId,a,b) switch (msgId) {
  case 10001: {
    ::println("... Evaled valid expression: " + a)
    w.FindWidget("ID_ExpressionEditorText").text = a
    break;
  }
}
