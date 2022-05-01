<Canvas
  title="ExpressionEditor"
  id="ID_ExpressionEditorForm"
  rect="Rect(0,0,400,300)"
  client_size="Vec2(400,300)"
  code="forms/ExpressionEditor.ni">

  <EditBox
    id="ID_ExpressionEditor"
    height="24 * 3"
    dock_style="DockTop"
    text=""
    style="HoldFocus|eWidgetEditBoxStyle.MultiLine|eWidgetEditBoxStyle.ValidateOnLostFocus"
    />

  <Dummy height="24" margin_top="3" dock_style="DockTop">
    <Button
      id="ID_ExpressionEvalBt"
      width="50"
      dock_style="DockRight"
      text="Eval" />
    <EditBox
      id="ID_ExpressionResult"
      dock_style="DockFill"
      text="Result"
      style="HoldFocus|eWidgetEditBoxStyle.ReadOnly|eWidgetEditBoxStyle.SelectAllOnSetFocus"
      />
  </Dummy>

  <ListBox
    id="ID_ExpressionEditorLog"
    margin_top="3"
    dock_style="DockFill"
    text=""
    style="HoldFocus"
    />

</Canvas>
