#ifndef __WIDGETPROPERTYBOX_2020207_H__
#define __WIDGETPROPERTYBOX_2020207_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#if niMinFeatures(20)

enum ePropertyEditBoxType
{
  ePropertyEditBoxType_Unknown,
  ePropertyEditBoxType_Hidden,
  ePropertyEditBoxType_Text,
  ePropertyEditBoxType_ROText,
  ePropertyEditBoxType_TextML,
  ePropertyEditBoxType_ROTextML,
  ePropertyEditBoxType_Enum,
  ePropertyEditBoxType_Flags,
  ePropertyEditBoxType_Bool,
  ePropertyEditBoxType_Range,
  ePropertyEditBoxType_Color,
  ePropertyEditBoxType_Font,
};

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidgetPropertyBox declaration.
class cWidgetPropertyBox : public ni::cIUnknownImpl<ni::iWidgetSink,ni::eIUnknownImplFlags_Default,ni::iWidgetPropertyBox>
{
  niBeginClass(cWidgetPropertyBox);

 public:
  //! Constructor.
  cWidgetPropertyBox(iWidget* apWidget);
  //! Destructor.
  ~cWidgetPropertyBox();

  //! Zeros all the class members.
  void ZeroMembers();
  //! Sanity check.
  ni::tBool __stdcall IsOK() const;

  //// ni::iWidgetPropertyBox //////////////////
  void __stdcall SetDataTable(iDataTable *apDT);
  iDataTable * __stdcall GetDataTable() const;
  void __stdcall ClearEditWidgetsCache();
  void __stdcall SetMaxNumChildren(tU32 anMax) { mnMaxNumChildren = anMax; }
  tU32 __stdcall GetMaxNumChildren() const { return mnMaxNumChildren; }
  void __stdcall SetMaxNumProperties(tU32 anMax) { mnMaxNumProperties = anMax; }
  tU32 __stdcall GetMaxNumProperties() const { return mnMaxNumProperties; }
  //// ni::iWidgetPropertyBox //////////////////

  //// ni::iWidgetSink /////////////////////////
  tBool __stdcall OnWidgetSink(iWidget* apWidget, tU32 anMsg, const Var& aA, const Var& aB);
  //// ni::iWidgetSink /////////////////////////

  void FillNode(iDataTable* apDT, iWidgetTreeNode* apNode);
  void ClearNode(iWidgetTreeNode* apNode);
  iWidgetTreeNode* FindNode(iWidgetTreeNode* apNode, iDataTable* apDT);
  iWidget* CreateWidgetEditBox(ePropertyEditBoxType aType, iHString* ahspMD);
  void RemoveWidgetEditBox(iWidget* apWidget);
  void ClearEditBoxes();
  void DoSetProperty(iWidget* apWidget);
  ePropertyEditBoxType  GetPropertyEditBoxType(iDataTable* apDT,  tU32 anIndex);
  ePropertyEditBoxType  GetWidgetEditBoxType(iWidget* apWidget);
  iWidget* SetEditBox(iWidgetTreeNode* apTreeNode, iDataTable* apDT,  tU32 anIndex, ePropertyEditBoxType type);
  void SetEditBoxInDataTable(iWidgetTreeNode* apNode, iDataTable* apDT);

 private:
  iWidget*        mpWidget;
  Ptr<iDataTable> mptrDT;
  Ptr<iWidget>    mpwTree;
  Ptr<iWidget>    mpwSplitter;
  Ptr<iWidget>    mpwEditBoxParent;


  tBool mbClearEditWidgetsCache;
  tU32  mnMaxNumChildren;
  tU32  mnMaxNumProperties;

  struct sEditBox {
    ePropertyEditBoxType  type;
    Ptr<iWidget>    w;
    tBool         used;
    sEditBox() {
      type = ePropertyEditBoxType_Unknown;
      used = eFalse;
    }
  };
  typedef astl::list<sEditBox>            tEBLst;
  typedef astl::hstring_hash_map<tEBLst>  tEBMap;
  tEBMap mmapEB;

  struct sSink : public cIUnknownImpl<iDataTableSink,eIUnknownImplFlags_NoRefCount,iWidgetSink>
  {
    friend class cWidgetPropertyBox;
    sSink(cWidgetPropertyBox* apParent) : mpParent(apParent), mnIgnoreSetProperty(eInvalidHandle) {}
    void __stdcall OnDataTableSink_SetName(iDataTable* apDT);
    void __stdcall OnDataTableSink_SetMetadata(iDataTable* apDT);
    void __stdcall OnDataTableSink_AddChild(iDataTable* apDT, iDataTable* apChild);
    void __stdcall OnDataTableSink_RemoveChild(iDataTable* apDT, iDataTable* apChild);
    void __stdcall OnDataTableSink_SetProperty(iDataTable* apDT, tU32 anProperty);
    void __stdcall OnDataTableSink_SetMetadata(iDataTable* apDT, tU32 anProperty);
    void __stdcall OnDataTableSink_RemoveProperty(iDataTable* apDT, tU32 anProperty);
    ni::Var __stdcall OnDataTableSink_GetProperty(ni::iDataTable *,ni::tU32);
    tBool __stdcall OnWidgetSink(iWidget* apWidget, tU32 anMsg, const Var& aA, const Var& aB);
    cWidgetPropertyBox* mpParent;
    tU32        mnIgnoreSetProperty;
  } mSink;

  struct {
    tU32 mcolBarsHoverBackground;
  } skin;
  void _UpdateSkin();

  niEndClass(cWidgetPropertyBox);
};

#endif

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __WIDGETPROPERTYBOX_2020207_H__
