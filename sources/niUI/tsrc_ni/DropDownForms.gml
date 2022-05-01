<Canvas
  title="DropDownForms"
  id="ID_DropDownFormsForm"
  client_size="Vec2(400,300)"
  padding="Vec4(50,50,50,50)"
  code="DropDownForms_form.ni">

  <Dummy width="200" height="25" dock_style="Grid">
    <Button
      id="ID_ExpressionEditorBt"
      width="25"
      dock_style="DockRight" />
    <EditBox
      id="ID_ExpressionEditorText"
      width="130"
      height="25"
      dock_style="DockFill"
      style="HoldFocus|eWidgetEditBoxStyle.SelectAllOnSetFocus"
      text="1 + 2" />
  </Dummy>

</Canvas>
