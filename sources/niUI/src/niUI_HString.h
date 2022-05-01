#if !defined __niUI_HSTRING_H__ || defined __MODULE_HSTRING_TABLE_IMPL__
#undef  __niUI_HSTRING_H__
#define __niUI_HSTRING_H__

#undef _HDecl_
#undef _HDecl
#undef _HC

#if defined __MODULE_HSTRING_TABLE_IMPL__
#pragma message("-- HString Table implementation --")

#define _HDecl_(X,STRING) ni::tHStringPtr _hstr_##X = _H(STRING)
#define _HDecl(X) _HDecl_(X,#X)

#else

#define _HDecl_(X,STRING)                       \
  extern ni::tHStringPtr _hstr_##X
#define _HDecl(X)                               \
  extern ni::tHStringPtr _hstr_##X

#endif

#define _HC(X) _hstr_##X

_HDecl(ID_Desktop);

_HDecl(__ID_DrawOpCaptureHUD__);
// _HDecl(SubmitA);
// _HDecl(SubmitB);
// _HDecl(SubmitC);
_HDecl(native);

// UIContext
_HDecl(UIWidget);
// _HDecl(LeftDoubleClick);
// _HDecl(RightDoubleClick);
// _HDecl(LeftClick);
// _HDecl(RightClick);
// _HDecl(MouseMove);
// _HDecl(Cancel);
// _HDecl(ModifierA);
// _HDecl(ModifierB);
// _HDecl(ModifierC);
// _HDecl(Wheel);
// _HDecl(ContextMenu);
// _HDecl(ContextHelp);
// _HDecl(Copy);
// _HDecl(Cut);
// _HDecl(Paste);
// _HDecl(Undo);
// _HDecl(Redo);
// _HDecl(MoveFocus);

// Widget
_HDecl(Form);
_HDecl(Dummy);
_HDecl(Canvas);
_HDecl(__ID_HoverWidget__);
// Canvas
_HDecl(ID_CanvasScrollV);
_HDecl(ID_CanvasScrollH);
// Form
_HDecl(form_button_Close);
_HDecl(form_button_Minimize);
_HDecl(form_button_Maximize);
_HDecl(form_button_Restore);
_HDecl(form_button_Fold);
// Toolbar
_HDecl(ID_Toolbar_Bars);
_HDecl(ID_Toolbar_TopBar);
_HDecl(ID_Toolbar_BotBar);
_HDecl(ID_Toolbar_Planet);
_HDecl(ID_Toolbar_Menu);
// ButtonWidget
_HDecl(Button_Normal);
_HDecl(Button_Down);
_HDecl(Button_Hover);
_HDecl(Button_FocusMarker);

#endif // __niUI_HSTRING_H__
