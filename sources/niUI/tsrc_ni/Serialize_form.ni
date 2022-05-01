
function OnSinkAttached(w,a,b) {
  ::dbg("... OnSinkAttached")
}

function OnSerializeWidget(w,a,b) {
  ::dbg("... OnSerializeWidget: " b ":" a)
  if (b & ::eSerializeFlags.Read) {
    ::println("... READ WIDGET")
    local bt = w.FindWidget("ID_Button")
    bt.text = a.string.serialized_button_text
  }
  else if (b & ::eSerializeFlags.Write) {
    ::println("... WRITE WIDGET")
  }
}


function OnSerializeFinalize(w,a,b) {
  ::dbg("... OnSerializeFinalize: " b ":" a)
  if (b & ::eWidgetSerializeFlags.Read) {
    ::println("... READ FINALIZE")
    local bt = w.FindWidget("ID_Button")
    bt.text += " - Finalized"
  }
  else if (b & ::eWidgetSerializeFlags.Write) {
    ::println("... WRITE FINALIZE")
  }
}
