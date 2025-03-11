::Import("gui.ni")

function OnSinkAttached(w,a,b) {
  ::dbg("... OnSinkAttached")
  local lb = w.FindWidget("ID_ComboBox");
  local i;
  i = lb.AddItem("alpha");
  i = lb.AddItem("beta");
  i = lb.AddItem("gamma");
  i = lb.AddItem("delta");
  i = lb.AddItem("eta");
  i = lb.AddItem("theta");
}

function ID_ComboBox(iUnknown w, iWidgetCommand cmd) if (cmd.id == ::eWidgetComboBoxCmd.SelectionChanged) {
  local lb = cmd.sender.QueryInterface("iWidgetListBox")
  ::println("... SelectionChanged:" lb.GetSelectedItem(0))
}
