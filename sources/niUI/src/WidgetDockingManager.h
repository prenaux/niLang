#ifndef __WIDGETDOCKINGMANAGER_18926062_H__
#define __WIDGETDOCKINGMANAGER_18926062_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidgetDockingManager declaration.
class cWidgetDockingManager : public ni::cIUnknownImpl<ni::iWidgetSink,ni::eIUnknownImplFlags_Default,ni::iWidgetDockingManager>
{
  niBeginClass(cWidgetDockingManager);

 public:
  //! Constructor.
  cWidgetDockingManager(ni::iWidget *apWidget);
  //! Destructor.
  ~cWidgetDockingManager();

  //! Zeros all the class members.
  void ZeroMembers();
  //! Sanity check.
  ni::tBool __stdcall IsOK() const;

  //// ni::iWidgetDockingManager ///////////////
  tBool __stdcall SetFlags(tWidgetDockingManagerFlags aFlags);
  tWidgetDockingManagerFlags __stdcall GetFlags() const;
  void __stdcall _ApplyFlags();
  tU32 __stdcall GetDockAreaFromPageName(const achar *aaszName) const;
  tU32 __stdcall GetDockAreaHovered(sVec2f avPos) const;
  tU32 __stdcall GetDockAreaFromWidget(iWidget* apWidget) const;
  tU32 __stdcall GetDockAreaFromDockAreaWidget(iWidget* apWidget) const;
  tBool __stdcall DockWidget(tU32 anDock, iWidget *apWidget);
  tBool __stdcall UndockWidget(iWidget *apWidget);
  tU32 __stdcall GetNumDockedWidgets() const;
  iWidget * __stdcall GetDockedWidget(tU32 anIndex) const;
  tU32 __stdcall GetNumDockAreas() const;
  iWidget * __stdcall GetDockArea(tU32 anIndex) const;
  tBool __stdcall SetDockAreaTabContextMenu(iWidget* apMenu);
  iWidget* __stdcall GetDockAreaTabContextMenu() const;
  tU32 __stdcall AddDockArea(tU32 anParent, tU32 aPos, sRectf aRect, tBool abLocal);
  void __stdcall CleanDockAreas();
  void __stdcall ClearDockAreas();
  //// ni::iWidgetDockingManager ///////////////

  //// iWidgetSink //////////////////////////////
  tBool __stdcall OnWidgetSink(iWidget* apWidget, tU32 anMsg, const Var& aA, const Var& aB);
  //// iWidgetSink //////////////////////////////

  void UpdateLocalDockButtons(iWidget* apArea);
  void Resize(tF32 afWidth, tF32 afHeight, tF32 afPrevWidth, tF32 afPrevHeight);
  tU32 GetDockAreaIndex(iWidget* apDockArea) const;
  tU32 GetDockAreaParent(iWidget* apDockArea) const;
  sRectf GetDockPositionRect(tRectEdgesFlags aDockPos, iWidget* apArea, sVec2f avRootPos, tBool abLocal);
  void SetDefaultDockAreaTabMenu();
  tBool PushBackDockArea(iWidget* apWidget);
  tBool RemoveSplitter(iWidget* apWidget);
  tBool DoCleanSplitters(iWidget* apWidget);
  void CleanSplitters();
  tBool RemoveDockArea(tU32 anIndex);
  void SerializeRead(iDataTable* apDT, tWidgetSerializeFlags aFlags);
  void SerializeWrite(iDataTable* apDT, tWidgetSerializeFlags aFlags);
  iWidget* GetDockAreaTab(tU32 anIndex) const;

 public:
  ni::iWidget*        mpWidget;
  tWidgetDockingManagerFlags  mnFlags;

  typedef Ptr<iWidget>      tWidgetPtr;
  typedef astl::vector<tWidgetPtr>  tWidgetVec;
  tWidgetPtr        mpwTab;
  tWidgetVec        mvDocked;
  tWidgetVec        mvDockAreas;
  Ptr<iWidget>      mpwOverlay;
  Ptr<iWidget>      mpwDockAreaTabContextMenu;

  struct sSkin {
    tU32 mnClearColor;
  } mSkin;

  struct sDockButton
  {
    sRectf mRect;
    sRectf mTabRect;
    tBool   mbHover;
    tRectEdgesFlags  mPosition;
    ni::iWidget* mpDockArea;
    tU32    mnDockArea;
    tBool   mbActive;
    tU32    mnColor;
    sDockButton() {
      mRect = sRectf(0,0);
      mbHover = eFalse;
      mPosition = 0;
      mpDockArea = NULL;
      mbActive = eTrue;
      mnColor = 0;
    }
  };
  enum eDockButton
  {
    eDockButton_LocalLeft = 0,
    eDockButton_LocalRight,
    eDockButton_LocalTop,
    eDockButton_LocalBottom,
    eDockButton_LocalTab,
    eDockButton_Left,
    eDockButton_Right,
    eDockButton_Top,
    eDockButton_Bottom,
    eDockButton_Last
  };
  sDockButton mButtons[eDockButton_Last];

  struct sOverlayWidgetSink : public cIUnknownImpl<iWidgetSink>
  {
    sOverlayWidgetSink(cWidgetDockingManager* apDockManager) : mpDockManager(apDockManager) {}
    tBool __stdcall OnWidgetSink(iWidget* apWidget, tU32 anMsg, const Var& aA, const Var& aB);
    cWidgetDockingManager* mpDockManager;
  };
  friend struct sOverlayWidgetSink;

  niEndClass(cWidgetDockingManager);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __WIDGETDOCKINGMANAGER_18926062_H__
