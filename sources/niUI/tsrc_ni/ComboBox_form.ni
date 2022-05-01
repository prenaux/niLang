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
