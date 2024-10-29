#ifndef __IUICONTEXT_45078280_H__
#define __IUICONTEXT_45078280_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/IProf.h>
#include <niLang/IRegex.h>

namespace ni {

/** \addtogroup niUI
 * @{
 */

//! UI input modifiers.
//! \remark The key values are intentionally the same as ni::eKeyMod
enum eUIInputModifier
{
  //! Modifier A.
  eUIInputModifier_A = niBit(16),
  //! Modifier A.
  eUIInputModifier_Control = eUIInputModifier_A,
  //! Modifier A.
  eUIInputModifier_Command = eUIInputModifier_A,
  //! Modifier A.
  eUIInputModifier_AddSelection = eUIInputModifier_A,
  //! Modifier B.
  eUIInputModifier_B = niBit(17),
  //! Modifier B.
  eUIInputModifier_Shift = eUIInputModifier_B,
  //! Modifier B.
  eUIInputModifier_AddSelectionRange = eUIInputModifier_B,
  //! Modifier C.
  eUIInputModifier_C = niBit(18),
  //! Modifier C.
  eUIInputModifier_Alt = eUIInputModifier_C,
  //! Modifier C.
  eUIInputModifier_Shortcut = eUIInputModifier_C,
  //! KeyDown is repeating.
  eUIInputModifier_KeyDownRepeat = niBit(31),
  //! \internal
  eUIInputModifier_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! UI input submit.
enum eUIInputSubmitFlags
{
  //! Disabled
  eUIInputSubmitFlags_Disabled = 0,
  //! Input action Submit A.
  //! \remark Standard is Enter and NumPadEnter.
  eUIInputSubmitFlags_SubmitA = niBit(0),
  //! Input action Submit B.
  //! \remark Standard is Space bar.
  eUIInputSubmitFlags_SubmitB = niBit(1),
  //! Input action Submit C.
  //! \remark Standard is Ctrl+Enter.
  eUIInputSubmitFlags_SubmitC = niBit(2),
  //! Left click.
  eUIInputSubmitFlags_LeftClick = niBit(3),
  //! Left double click.
  eUIInputSubmitFlags_LeftDoubleClick = niBit(4),
  //! Right click.
  eUIInputSubmitFlags_RightClick = niBit(5),
  //! Right double click.
  eUIInputSubmitFlags_RightDoubleClick = niBit(6),
  //! Button standard.
  eUIInputSubmitFlags_StandardButton = eUIInputSubmitFlags_SubmitA|eUIInputSubmitFlags_SubmitB|eUIInputSubmitFlags_LeftClick,
  //! Icon standard.
  eUIInputSubmitFlags_StandardIcon = eUIInputSubmitFlags_SubmitA|eUIInputSubmitFlags_SubmitB|eUIInputSubmitFlags_LeftDoubleClick,
  //! Single Line Text box standard.
  eUIInputSubmitFlags_StandardSingleLineText = eUIInputSubmitFlags_SubmitA|eUIInputSubmitFlags_SubmitC,
  //! Multi Line Text box standard.
  eUIInputSubmitFlags_StandardMultiLineText = eUIInputSubmitFlags_SubmitC,
  //! UI Context default input submit flags default.
  eUIInputSubmitFlags_Default = eUIInputSubmitFlags_Disabled,
  //! \internal
  eUIInputSubmitFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! UI Input submit flags type. \see ni::eUIInputSubmitFlags
typedef tU32 tUIInputSubmitFlags;

//! UI Context Skin interface.
struct iUISkin : public iUnknown
{
  niDeclareInterfaceUUID(iUISkin,0x22caeae6,0xdcac,0xf94a,0xa0,0x63,0x87,0xfb,0x2a,0x73,0xb7,0xd3)

  //########################################################################################
  //! \name Skins
  //########################################################################################
  //! @{
  //! Get the error overlay.
  //! {Property}
  virtual tBool __stdcall SetErrorOverlay(iOverlay* apOverlay) = 0;
  //! Get the error overlay.
  //! {Property}
  virtual iOverlay* __stdcall GetErrorOverlay() const = 0;
  //! Set the context's image map.
  //! {Property}
  virtual void __stdcall SetImageMap(iImageMap* apImageMap) = 0;
  //! Get the context's image map.
  //! {Property}
  virtual iImageMap* __stdcall GetImageMap() const = 0;
  //! Remove all skins loaded.
  //! \remark The default skin will remain.
  //! \remark A SkinChanged message will be broadcasted to all widgets when the skins are cleared.
  virtual void __stdcall ClearSkins() = 0;

  //! Add a new skin.
  //! \remark If a skin with the specified name has already been loaded the method will fail.
  virtual tBool __stdcall AddSkin(iDataTable* apDT) = 0;
  //! Add a new skin from the specified resource.
  virtual tBool __stdcall AddSkinFromRes(iHString* ahspRes) = 0;
  //! Unload a skin.
  //! \remark The default skin cannot be removed
  virtual tBool __stdcall RemoveSkin(iHString* ahspSkin) = 0;

  //! Set the default skin.
  //! {Property}
  virtual tBool __stdcall SetDefaultSkin(iHString* ahspName) = 0;
  //! Get the default skin.
  //! {Property}
  virtual iHString* __stdcall GetDefaultSkin() const = 0;

  //! Get the number of skins.
  //! {Property}
  virtual tU32 __stdcall GetNumSkins() const = 0;
  //! Get the name of the skin at the specified index.
  //! {Property}
  virtual iHString* __stdcall GetSkinName(tU32 anIndex) const = 0;
  //! Get the index of the skin with the specified name.
  //! \remark Return eInvalidHandle if the skin does not exist.
  //! {Property}
  virtual tU32 __stdcall GetSkinIndex(iHString* ahspName) const = 0;
  //! Get the data table associated with the specified skin.
  //! {Property}
  virtual iDataTable* __stdcall GetSkinDataTable(iHString* ahspSkin) const = 0;
  //! Get the data table associated with the error skin.
  //! {Property}
  virtual iDataTable* __stdcall GetErrorSkinDataTable() const = 0;
  //! @}

};

niExportFunc(iUnknown*) New_niUI_UISkin(const Var& aGraphicsContext, const Var&);

//! UI Context interface.
struct iUIContext : public iUnknown
{
  niDeclareInterfaceUUID(iUIContext,0x2c6b89cb,0x0e9e,0x41fc,0x92,0xbb,0xae,0x6a,0xa3,0xee,0x44,0x2c)

  //########################################################################################
  //! \name Graphics context
  //########################################################################################
  //! @{

  //! Get the parent graphics object.
  //! {Property}
  virtual iGraphics* __stdcall GetGraphics() const = 0;
  //! Get the parent graphics context attached to this UI context.
  //! {Property}
  virtual iGraphicsContext* __stdcall GetGraphicsContext() const = 0;
  //! @}

  //########################################################################################
  //! \name Input
  //########################################################################################
  //! @{

  //! Send a Window message to the UI context.
  //! \remark This method should be called in the application's window message sink to send input to the UI context.
  virtual void __stdcall SendWindowMessage(eOSWindowMessage aMsg, const Var& avarA, const Var& avarB) = 0;
  //! Set the input modifiers. \see ni::eUIInputModifier
  //! {Property}
  virtual void __stdcall SetInputModifiers(tU32 anInputModifier) = 0;
  //! Get the input modifiers. \see ni::eUIInputModifier
  //! {Property}
  virtual tU32 __stdcall GetInputModifiers() const = 0;
  //! Set the default input submit flags.
  //! {Property}
  virtual void __stdcall SetDefaultInputSubmitFlags(tUIInputSubmitFlags aSubmitFlags) = 0;
  //! Get the default input submit flags.
  //! {Property}
  virtual tUIInputSubmitFlags __stdcall GetDefaultInputSubmitFlags() const = 0;

  //! Set the hovering delay in seconds.
  //! {Property}
  //! \remark Default is 0 which means that it is disabled.
  virtual void __stdcall SetHoverDelay(tF32 afDelay) = 0;
  //! Get the hovering delay in seconds.
  //! {Property}
  virtual tF32 __stdcall GetHoverDelay() const = 0;
  //! Set the input modifier required to enable the hover. \see ni::eUIInputModifier
  //! {Property}
  virtual void __stdcall SetHoverInputModifiers(tU32 anInputModifiers) = 0;
  //! Get the input modifier required to enable the hover. \see ni::eUIInputModifier
  //! {Property}
  virtual tU32 __stdcall GetHoverInputModifiers() const = 0;
  //! @}

  //########################################################################################
  //! \name Skins
  //########################################################################################
  //! @{
  //! Create new UISkin from a skin file
  virtual iUISkin* __stdcall CreateUISkin(iHString* apSkinPath, tF32 afContentsScale) = 0;

  //! Get the UISkin object
  //! {Property}
  virtual iUISkin* __stdcall GetUISkin() const = 0;
  //! Set the UISkin object
  virtual void __stdcall SetUISkin(iUISkin* apUISkin) = 0;
  //! Set the UISkin from given path
  virtual void __stdcall SetUISkinFromPath(iHString* ahspSkinPath) = 0;

  //! Set the context's image map.
  //! {Property}
  virtual void __stdcall SetImageMap(iImageMap* apImageMap) = 0;
  //! Get the context's image map.
  //! {Property}
  virtual iImageMap* __stdcall GetImageMap() const = 0;

  //! Get the error overlay.
  //! {Property}
  virtual tBool __stdcall SetErrorOverlay(iOverlay* apOverlay) = 0;
  //! Get the error overlay.
  //! {Property}
  virtual iOverlay* __stdcall GetErrorOverlay() const = 0;

  //! Remove all skins loaded.
  //! \remark The default skin will remain.
  //! \remark A SkinChanged message will be broadcasted to all widgets when the skins are cleared.
  virtual void __stdcall ClearSkins() = 0;

  //! Add a new skin.
  //! \remark If a skin with the specified name has already been loaded the method will fail.
  virtual tBool __stdcall AddSkin(iDataTable* apDT) = 0;
  //! Add a new skin from the specified resource.
  virtual tBool __stdcall AddSkinFromRes(iHString* ahspRes) = 0;
  //! Unload a skin.
  //! \remark The default skin cannot be removed
  virtual tBool __stdcall RemoveSkin(iHString* ahspSkin) = 0;

  //! Set the default skin.
  //! {Property}
  virtual tBool __stdcall SetDefaultSkin(iHString* ahspName) = 0;
  //! Get the default skin.
  //! {Property}
  virtual iHString* __stdcall GetDefaultSkin() const = 0;

  //! Get the number of skins.
  //! {Property}
  virtual tU32 __stdcall GetNumSkins() const = 0;
  //! Get the name of the skin at the specified index.
  //! {Property}
  virtual iHString* __stdcall GetSkinName(tU32 anIndex) const = 0;
  //! Get the index of the skin with the specified name.
  //! \remark Return eInvalidHandle if the skin does not exist.
  //! {Property}
  virtual tU32 __stdcall GetSkinIndex(iHString* ahspName) const = 0;
  //! Get the data table associated with the specified skin.
  //! {Property}
  virtual iDataTable* __stdcall GetSkinDataTable(iHString* ahspSkin) const = 0;

  //! Apply the specified skin to a widget and all its children.
  virtual tBool __stdcall ApplySkin(iWidget* apWidget, iHString* ahspName) = 0;
  //! Find the font of the specified item in the widget's skin.
  virtual iFont*   __stdcall FindSkinFont(iHString* ahspSkin, iHString* ahspClass, iHString* ahspState, iHString* ahspName) const = 0;
  //! Find the cursor of the specified item in the widget's skin.
  virtual iOverlay* __stdcall FindSkinCursor(iHString* ahspSkin, iHString* ahspClass, iHString* ahspState, iHString* ahspName) const = 0;
  //! Find the element of the specified item in the widget's skin.
  virtual iOverlay* __stdcall FindSkinElement(iHString* ahspSkin, iHString* ahspClass, iHString* ahspState, iHString* ahspName) const = 0;
  //! Find the color of the specified item in the widget's skin.
  virtual sColor4f __stdcall FindSkinColor(const sColor4f& aDefault, iHString* ahspSkin, iHString* ahspClass, iHString* ahspState, iHString* ahspName) const = 0;
  //! @}

  //########################################################################################
  //! \name Widgets
  //########################################################################################
  //! @{

  //! Check whether the specified widget sink class can be created.
  virtual tBool __stdcall HasWidgetSinkClass(const achar *aszClassName) const = 0;
  //! Create a widget sink of the specified class.
  virtual iWidgetSink* __stdcall CreateWidgetSink(const achar *aszClassName, iWidget* apWidget) = 0;
#if niMinFeatures(15)
  //! Create a widget sink from the the specified script script.
  virtual iWidgetSink* __stdcall CreateWidgetSinkFromScript(iHString* ahspRes) = 0;
#endif

  //! Create a widget.
  virtual iWidget* __stdcall CreateWidget(const achar *aszClassName, iWidget *apwParent, const sRectf &arectPos = sRectf::Null(), tU32 anStyle = 0, iHString* ahspID = NULL) = 0;

  //! Create a widget without attaching any sink to it.
  //! \remark Use ni::iWidget::AddSink, ni::iWidget::AddPostSink,
  //!         ni::iWidget::AddClassSink or ni::iWidget::AddClassPostSink to
  //!         add custom or registered sink.
  virtual iWidget* __stdcall CreateWidgetRaw(const achar *aszClassName, iWidget *apwParent, const sRectf &arectPos = sRectf::Null(), tU32 anStyle = 0, iHString* ahspID = NULL) = 0;

  //! Create a widget from the specified datatable.
  //! \remark apwParent is the parent widget,
  //!     if specified the datatable is loaded in a Canvas widget with a fill dock style,
  //!     if not the datatable is loaded in a Form widget with a snap center dock style.
  //! \remark ahspID if not null is the id of the widget (overwrite the 'id' property in the datatable).
  //! \remark ahspTitle if not null is the title of the form widget (overwrite the 'title' property in the datatable).
  //! \return The newly created widget.
  virtual iWidget* __stdcall CreateWidgetFromDataTable(iDataTable* apDT, iWidget* apwParent, iHString* ahspID, iHString* ahspTitle) = 0;

  //! Create a widget from a datatable in the specified resource.
  virtual iWidget* __stdcall CreateWidgetFromResource(iHString* ahspRes, iWidget* apwParent, iHString* ahspID, iHString* ahspTitle) = 0;

  //! Get the root widget.
  //! {Property}
  virtual iWidget* __stdcall GetRootWidget() const = 0;
  //! Set the active widget.
  //! {Property}
  virtual tBool __stdcall SetActiveWidget(iWidget* apWidget) = 0;
  //! Get the active widget.
  //! {Property}
  virtual iWidget* __stdcall GetActiveWidget() const = 0;
  //! Get the number of widgets created.
  //! {Property}
  //! \remark This function is meant for debugging purpose.
  virtual tU32 __stdcall GetNumWidgets() const = 0;
  //! Get the widget at the specified index.
  //! {Property}
  //! \remark This function is meant for debugging purpose.
  virtual iWidget* __stdcall GetWidget(tU32 anIndex) const = 0;
  //! Serialize the specified widget.
  //! \param apWidget is the widget to serialize.
  //! \param apDT is the source/destination data table.
  //! \param anFlags is the serialization flags.
  //! \param apFilter regular expression that determines which class will be serialized. Is NULL all classes will be serialized.
  //! \return eFalse if failed, else eTrue.
  virtual tBool __stdcall SerializeWidget(iWidget* apWidget, iDataTable* apDT, tWidgetSerializeFlags anFlags, iRegex* apFilter) = 0;
  //! Create a widget command instance.
  virtual iWidgetCommand* __stdcall CreateWidgetCommand() = 0;
  //! Send a command to the specified widget.
  virtual tBool __stdcall SendCommand(iWidget* apDest, iWidgetCommand* apCmd) = 0;
  //! Draw the GUI.
  virtual void __stdcall Draw() = 0;
  //! Draw the current mouse cursor or set it as hardware cursor on the
  //! specified window if it is specified and supported by the graphics
  //! context.
  virtual void __stdcall DrawCursor(iOSWindow* apWindow) = 0;
  //! Updates the GUI.
  //! \return eTrue if succeeded, eFalse if failed.
  //! \remark Update will fail if called from inside a widget's message handler.
  virtual tBool __stdcall Update(tF32 fTime) = 0;
  //! Resize the UI context.
  //! \return eTrue if succeeded, eFalse if failed.
  virtual tBool __stdcall Resize(const sRectf& aRootRect, const tF32 afContentsScale) = 0;
  //! Get the contents' scale factor.
  //! {Property}
  virtual tF32 __stdcall GetContentsScale() const = 0;
  //! Set whether debug draw is enabled.
  //! {Property}
  virtual void __stdcall SetDebugDraw(tBool abDebug) = 0;
  //! Get whether debug draw is enabled.
  //! {Property}
  virtual tBool __stdcall GetDebugDraw() const = 0;
  //! @}

  //########################################################################################
  //! \name Cursor
  //########################################################################################
  //! @{

  //! Set the current mouse cursor.
  //! {Property}
  virtual tBool __stdcall SetCursor(iOverlay* pCursor) = 0;
  //! Get the current mouse cursor.
  //! {Property}
  virtual iOverlay* __stdcall GetCursor() const = 0;
  //! Get the cursor position.
  //! {Property}
  virtual sVec2f __stdcall GetCursorPosition() const = 0;
  //! @}

#if niMinFeatures(20)
  //########################################################################################
  //! \name Toolbar
  //########################################################################################
  //! @{

  //! Initialize the a toolbar in the root widget.
  virtual void __stdcall InitializeDefaultToolbar() = 0;
  //! Set the UI default toolbar.
  //! {Property}
  virtual ni::tBool __stdcall SetToolbar(iWidget* apToolbar) = 0;
  //! Get the UI default toolbar.
  //! {Property}
  virtual ni::iWidget* __stdcall GetToolbar() const = 0;
  //! @}
#endif

  //########################################################################################
  //! \name DrawOpCapture
  //########################################################################################
  //! @{

  //! Set whether the draw operation capture hud is enabled.
  //! {Property}
  virtual void __stdcall SetDrawOpCapture(tBool abEnabled) = 0;
  //! Get whether the draw operation capture hud is enabled.
  //! {Property}
  virtual ni::tBool __stdcall GetDrawOpCapture() const = 0;
  //! @}

  //########################################################################################
  //! \name Manual drawing
  //########################################################################################
  //! @{

  //! Draw the specified widget in the specified canvas.
  virtual tBool __stdcall DrawWidget(iWidget* apWidget, iCanvas* apCanvas) = 0;
  //! Draw the specified widget in the specified canvas. The widget is
  //! transformed using the specified matrix. Clipping and scissoring is
  //! disabled.
  virtual tBool __stdcall DrawTransformedWidget(iWidget* apWidget, iCanvas* apCanvas, const sMatrixf& aBaseMatrix, tBool abUseScissor) = 0;
  //! @}


  //########################################################################################
  //! \name UI behaviors
  //########################################################################################
  //! @{

  //! Set the drag start distance.
  //! \remark This define the minimum number of pixels the cursor has to move before a drag start message is sent.
  //! {Property}
  virtual void __stdcall SetDragStartDistance(tU32 anPixelDistance) = 0;
  //! Get the drag start distance.
  //! {Property}
  virtual tU32 __stdcall GetDragStartDistance() const = 0;
  //! @}

  //########################################################################################
  //! \name Finger
  //########################################################################################
  //! @{

  //! Set the drag finger start distance.
  //! \remark This define the minimum number of pixels a finger has to move before a drag start message is sent.
  //! {Property}
  virtual void __stdcall SetDragFingerStartDistance(tU32 anFinger, tU32 anPixelDistance) = 0;
  //! Get the drag finger start distance.
  //! {Property}
  virtual tU32 __stdcall GetDragFingerStartDistance(tU32 anFinger) const = 0;
  //! Get a finger's latest screen position and pressure.
  //! {Property}
  virtual sVec3f __stdcall GetFingerPosition(tU32 anFinger) const = 0;
  //! Get a finger's latest down state.
  //! {Property}
  virtual tBool __stdcall GetFingerDown(tU32 anFinger) const = 0;
  //! @}

  //########################################################################################
  //! \name Shorcuts
  //########################################################################################
  //! @{

  //! Remove all shortcuts.
  virtual void __stdcall ClearShortcuts() = 0;
  //! Add a shortcut.
  virtual void __stdcall AddShortcut(tU32 anKey, iHString* ahspCmd) = 0;
  //! Remove a shortcut.
  virtual void __stdcall RemoveShortcut(tU32 anKey) = 0;
  //! Get the command of the specified shortcut.
  //! {Property}
  virtual iHString* __stdcall GetShortcutCommand(tU32 anKey) const = 0;
  //! @}

  //########################################################################################
  //! \name Input
  //########################################################################################
  //! @{

  //! Get a keyboard key's down state.
  //! {Property}
  virtual tBool __stdcall GetKeyDown(tU8 aKey) const = 0;
  //! \param avPosition is in UI units.
  virtual void __stdcall InputFingerMove(tU32 anFinger, const sVec3f& avPosition) = 0;
  //! \param avRelMove is in UI units.
  virtual void __stdcall InputFingerRelativeMove(tU32 anFinger, const sVec3f& avRelMove) = 0;
  //! \param avPosition is in UI units.
  virtual void __stdcall InputFingerPress(tU32 anFinger, const sVec3f& avPosition, tBool abDown) = 0;
  virtual void __stdcall InputKeyPress(eKey aKey, tBool abDown) = 0;
  virtual void __stdcall InputKeyChar(tU32 aCharCodePoint, eKey aKeyLeadingToKeyChar) = 0;
  virtual void __stdcall InputMouseWheel(const tF32 afDelta) = 0;
  virtual void __stdcall InputDoubleClick(ePointerButton aPointer) = 0;
  //! \param avRelMove is in UI units.
  virtual void __stdcall InputRelativeMouseMove(const sVec2f& avRelMove) = 0;
  virtual void __stdcall InputGameCtrl(iGameCtrl* apGameController) = 0;
  virtual void __stdcall InputPinch(const tF32 afScale, const eGestureState aState) = 0;
  //! Get the focused widget, that is the widget that is receiving the input messages.
  //! {Property}
  virtual iWidget* __stdcall GetFocusedWidget() const = 0;
  //! @}

  //########################################################################################
  //! \name Profiler
  //########################################################################################
  //! @{

  //! Create an iProfDraw instance which uses the specifed canvas and font.
  virtual iProfDraw* __stdcall CreateProfDraw(iCanvas* apCanvas, iFont* apFont) const = 0;
  //! @}

  //########################################################################################
  //! \name ShowTerminal
  //########################################################################################
  //! @{

  //! Set whether the builtin terminal is enabled.
  //! {Property}
  virtual void __stdcall SetShowTerminal(tBool abEnabled) = 0;
  //! Get whether the builtin terminal is enabled.
  //! {Property}
  virtual ni::tBool __stdcall GetShowTerminal() const = 0;
  //! @}
};

niExportFunc(iUnknown*) New_niUI_UIContext(const Var& aGraphicsContext, const Var&);

niExportFunc(ni::iUIContext*) CreateUIContext(iGraphicsContext* apGraphicsContext, iUISkin* apSkin, tF32 afContentsScale);
niExportFunc(ni::iUISkin*) CreateUISkin(iGraphicsContext* apGraphicsContext, iHString* ahspDefaultSkinPath, tF32 afContentsScale);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
};
#endif // __IUICONTEXT_45078280_H__
