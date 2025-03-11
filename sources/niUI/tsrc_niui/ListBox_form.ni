::Import("gui.ni")

function OnSinkAttached(w,a,b) {
  ::dbg("... OnSinkAttached")

  local i;

  local longText = "lah lah lah this is a beautiful text that's very long and should make us scroll for our lives, ta ta ta ta dah dah 1 -- lah lah lah this is a beautiful text that's very long and should make us scroll for our lives, ta ta ta ta dah dah 2 -- lah lah lah this is a beautiful text that's very long and should make us scroll for our lives, ta ta ta ta dah dah 3 -- lah lah lah this is a beautiful text that's very long and should make us scroll for our lives, ta ta ta ta dah dah 4"

  local lb1 = w.FindWidget("ID_ListBox1");
  lb1.AddItem(longText);
  for (local j = 0; j < 50; ++j) {
    i = lb1.AddItem("n" + j);
    lb1.SetItemText(1,i,"t"+j);
  }

  local lb = w.FindWidget("ID_ListBox");
  lb.style |= ::eWidgetListBoxStyle.HasHeader|::eWidgetListBoxStyle.HeaderSort
  lb.SetColumn(0,"First",200);
  lb.AddColumn("Second",200);
  lb.AddColumn("Third",1000);
  i = lb.AddItem("alpha");
  lb.SetItemText(1,i,"a");
  lb.SetItemText(2,i,longText);
  i = lb.AddItem("beta");
  lb.SetItemText(1,i,"b");
  i = lb.AddItem("gamma");
  lb.SetItemText(1,i,"c");
  i = lb.AddItem("delta");
  lb.SetItemText(1,i,"d");
  i = lb.AddItem("eta");
  lb.SetItemText(1,i,"e");
  i = lb.AddItem("theta");
  lb.SetItemText(1,i,"t");

  for (local j = 0; j < 50; ++j) {
    i = lb.AddItem("n" + j);
    lb.SetItemText(1,i,"t"+j);
  }
}
