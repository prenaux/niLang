#ifndef __WIDGETMENU_30223356_H__
#define __WIDGETMENU_30223356_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

typedef astl::vector<Ptr<iWidgetMenuItem> > tWidgetMenuItemVec;
typedef tWidgetMenuItemVec::iterator            tWidgetMenuItemVecIt;
typedef tWidgetMenuItemVec::const_iterator        tWidgetMenuItemVecCIt;

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidgetMenu declaration.
class cWidgetMenu : public ni::ImplRC<ni::iWidgetSink,ni::eImplFlags_Default,ni::iWidgetMenu>
{
  niBeginClass(cWidgetMenu);

 public:
  //! Constructor.
  cWidgetMenu(iWidget* apWidget);
  //! Destructor.
  ~cWidgetMenu();

  //! Zeros all the class members.
  void ZeroMembers();
  //! Sanity check.
  ni::tBool __stdcall IsOK() const;

  //// iWidgetSink //////////////////////////////
  tBool __stdcall OnWidgetSink(iWidget *apWidget, tU32 anMsg, const Var& avarA, const Var& avarB);
  //// iWidgetSink //////////////////////////////

  //// ni::iWidgetMenu /////////////////////////
  tBool __stdcall ClearItems();
  iWidgetMenuItem* __stdcall AddItem(const achar* aaszName, iHString* ahspID, tWidgetMenuItemFlags aFlags);
  tBool __stdcall RemoveItem(iWidgetMenuItem* apItem);
  tU32 __stdcall GetNumItems() const;
  iWidgetMenuItem* __stdcall GetItem(tU32 anIndex) const;
  iWidgetMenuItem* __stdcall GetItemFromName(const achar* aaszName) const;
  iWidgetMenuItem* __stdcall GetItemFromID(iHString* ahspID) const;
  tBool __stdcall Open();
  tBool __stdcall Close();
  tBool __stdcall SortItems();
  tU32 __stdcall GetMenuWidth() const;
  tU32 __stdcall GetMenuHeight() const;
  tU32 __stdcall GetItemWidth() const;
  tU32 __stdcall GetItemHeight() const;
  void __stdcall UpdateSizes();
  //// ni::iWidgetMenu /////////////////////////

  tU32 DrawItemRect(iCanvas* apIM, iFont* apFont, tU32 anIndex, tU32 aX, tU32 aY);
  tU32 DrawItemOvr(iCanvas* apIM, iFont* apFont, tU32 anIndex, tU32 aX, tU32 aY);
  tU32 DrawItemText(iCanvas* apIM, iFont* apFont, tU32 anIndex, tU32 aX, tU32 aY);
  tU32 FindItemFromPos(const sVec2f& avPos);
  void SelectItem(tU32 anNewSel);
  struct sMouseMessageResult
  {
    tBool       bIntersect;
    eWidgetMenuCmd  Msg;
    Ptr<iWidgetMenuItem>  ptrItem;
    sMouseMessageResult() {
      bIntersect = eFalse;
      Msg = eWidgetMenuCmd_ForceDWORD;
    }
  };
  tBool _TestMouseIntersect(const sVec2f& avMousePos);
  sMouseMessageResult ProcessMouseMessage(tU32 anMsg, const sVec2f& avMousePos);
  sMouseMessageResult ProcessKeyInput(tU32 aKey, tU32 anKeyMod, tBool abIsDown);
  void ProcessMouseClickResult(const sMouseMessageResult& aRes, iWidget* apRoot);
  void ProcessClick(sMouseMessageResult& aRes);
  void ShowSubMenu();
  void CloseSubMenu();
  void SelectFirstItem();
  void SelectLastItem();
  void SelectNextItem();
  void SelectPrevItem();
  void SelectNextSeparator();
  void SelectPrevSeparator();

 private:
  iWidget*    mpWidget;
  tWidgetMenuItemVec  mvItems;
  tU32      mnSelectedItem;
  tU32      mnRightMargin;
  tU32      mnLeftMargin;
  tU32      mnItemHeight;
  tU32      mnItemWidth;
  tU32      mnWidth;
  tU32      mnHeight;
  iWidget*    mpActiveSubMenu;
  tBool     mbKeyInput;
  tBool     mbClickDown;

  tBool       mbShouldUpdateSizes;
  tBool   mbIsSubMenu;
  tBool   mbIsSubSubMenu;

  tHStringPtr     mhspPrevOpenLocale;

  void InitSkin();
  struct sSkin {
    Ptr<iOverlay> radio;
    Ptr<iOverlay> check;
    Ptr<iOverlay> arrowRight;

    // EDMOND _newUI_
    tU32 _kcolBorder;
    tU32 _kcolBorderSelected;
    tU32 _kcolSeparator;
    tU32 _kcolBackNormal;
    tU32 _kcolBackSelected;
    tU32 _kcolLeftMargin;
    tU32 _kcolBackTitle;
    tU32 _kcolBorderTitle;
    tU32 _kcolTextDisabled;
  } skin;

  niEndClass(cWidgetMenu);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __WIDGETMENU_30223356_H__
