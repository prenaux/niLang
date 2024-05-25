// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"

#if niMinFeatures(20)

#include "WidgetPropertyBox.h"
#include <niLang/STL/set.h>
#include <niLang/Utils/ParseType.h>

//#define PB_DEBUG_DUMP

const sVec4f _kvEBMargin = {1,1,1,1};

// #define USE_DROP_MENU_ENUM
#define USE_DROP_MENU_FLAGS

static const sEnumDef* _GetEnumDefFromMetadata(const achar* aMD) {
  cString enumName = _ASTR(aMD).After("[").Before("]");
  if (enumName[0] == '*')
    enumName = enumName.Chars()+1;
  return ni::GetLang()->GetEnumDef(enumName.Chars());
}

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidgetPropertyBox implementation.

///////////////////////////////////////////////
cWidgetPropertyBox::cWidgetPropertyBox(iWidget* apWidget)
    : mSink(this)
{
  ZeroMembers();
  mpWidget = apWidget;

  mpwSplitter = mpWidget->GetUIContext()->CreateWidget(_A("Splitter"),mpWidget,sRectf(0,0,50,50),eWidgetStyle_DontSerialize,NULL);
  QPtr<iWidgetSplitter> ptrSplitter = mpwSplitter.ptr();
  ptrSplitter->SetSplitterSize(3);
  mpwSplitter->SetMinimumSize(Vec2<tF32>(20,20));
  mpwSplitter->SetDockStyle(eWidgetDockStyle_DockFill);

  mpwTree = mpWidget->GetUIContext()->CreateWidget(_A("Tree"),ptrSplitter->GetSplitterWidget(0),sRectf(0,0),eWidgetStyle_HoldFocus|eWidgetStyle_DontSerialize,_H("ID_PropertyBoxTree"));
  mpwTree->SetDockStyle(eWidgetDockStyle_DockFill);
  mpwTree->SetSkinClass(_H("PropertyBoxTree"));

  mpwEditBoxParent = ptrSplitter->GetSplitterWidget(1);
  mpwEditBoxParent->AddSink(&mSink);

  _UpdateSkin();
}

///////////////////////////////////////////////
cWidgetPropertyBox::~cWidgetPropertyBox()
{
  if (mptrDT.IsOK()) {
    mptrDT->GetSinkList()->RemoveSink(&mSink);
    mptrDT = NULL;
  }
  mpwEditBoxParent->RemoveSink(&mSink);
}

///////////////////////////////////////////////
//! Zeros all the class members.
void cWidgetPropertyBox::ZeroMembers()
{
  mpWidget = NULL;
  mbClearEditWidgetsCache = eFalse;
  mnMaxNumChildren = 30;
  mnMaxNumProperties = 50;
  skin.mcolBarsHoverBackground = ~0;
}

///////////////////////////////////////////////
//! Sanity check.
ni::tBool __stdcall cWidgetPropertyBox::IsOK() const
{
  niClassIsOK(cWidgetPropertyBox);
  return ni::eTrue;
}

///////////////////////////////////////////////
void cWidgetPropertyBox::SetDataTable(iDataTable *apDT)
{
  if (apDT == mptrDT) {
    return;
  }
  QPtr<iWidgetTree> tree = mpwTree.ptr();
  if (!tree.IsOK())
    return;
  tree->Clear();
  ClearEditBoxes();
  if (mptrDT.IsOK()) {
    mptrDT->GetSinkList()->RemoveSink(&mSink);
  }
  mptrDT = niIsOK(apDT)?apDT:NULL;
  if (mptrDT.IsOK()) {
    mptrDT->GetSinkList()->AddSink(&mSink);
    FillNode(mptrDT,tree->GetRootNode());
  }
}

///////////////////////////////////////////////
iDataTable * cWidgetPropertyBox::GetDataTable() const
{
  return mptrDT;
}

///////////////////////////////////////////////
void __stdcall cWidgetPropertyBox::sSink::OnDataTableSink_SetName(iDataTable* apDT)
{
  if (!mpParent) return;
  iWidgetTreeNode* pNode = mpParent->FindNode(NULL,apDT);
  if (pNode) {
    pNode->SetName(apDT->GetName());
  }
}

///////////////////////////////////////////////
void __stdcall cWidgetPropertyBox::sSink::OnDataTableSink_SetMetadata(iDataTable* apDT)
{
  if (!mpParent) return;
  iWidgetTreeNode* pNode = mpParent->FindNode(NULL,apDT);
  if (pNode) {
  }
}

///////////////////////////////////////////////
void __stdcall cWidgetPropertyBox::sSink::OnDataTableSink_AddChild(iDataTable* apDT, iDataTable* apChild)
{
  if (!mpParent) return;
  iWidgetTreeNode* pNode = mpParent->FindNode(NULL,apDT);
  if (pNode) {
    iWidgetTreeNode* pNewNode = pNode->AddChildNode(AZEROSTR);
    mpParent->FillNode(apChild,pNewNode);
  }
}

///////////////////////////////////////////////
void __stdcall cWidgetPropertyBox::sSink::OnDataTableSink_RemoveChild(iDataTable* apDT, iDataTable* apChild)
{
  if (!mpParent) return;
  iWidgetTreeNode* pNode = mpParent->FindNode(NULL,apDT);
  if (pNode) {
    iWidgetTreeNode* pChildNode = mpParent->FindNode(pNode,apChild);
    if (pChildNode) {
      // remove the widgest of the child tables
      niLoop(i,apChild->GetNumChildren()) {
        OnDataTableSink_RemoveChild(apChild,apChild->GetChildFromIndex(i));
      }
      // remove the widgets of the properties
      niLoop(i,apChild->GetNumProperties()) {
        OnDataTableSink_RemoveProperty(apChild,i);
      }
      pNode->RemoveChildNode(pChildNode);
    }
  }
}

///////////////////////////////////////////////
void __stdcall cWidgetPropertyBox::sSink::OnDataTableSink_SetProperty(iDataTable* apDT, tU32 anProperty)
{
  if (!mpParent || mnIgnoreSetProperty == anProperty) return;
  ePropertyEditBoxType ebtypeProperty = mpParent->GetPropertyEditBoxType(apDT,anProperty);
  if (ebtypeProperty == ePropertyEditBoxType_Hidden)
    return;
  iWidgetTreeNode* pNode = mpParent->FindNode(NULL,apDT);
  if (!pNode) return;
  iWidgetTreeNode* pPropertyNode = pNode->GetChildNodeFromName(apDT->GetPropertyName(anProperty));
  if (!pPropertyNode) {
    pPropertyNode = pNode->AddChildNode(apDT->GetPropertyName(anProperty));
  }
  mpParent->SetEditBox(pPropertyNode,apDT,anProperty,ebtypeProperty);
}

///////////////////////////////////////////////
void __stdcall cWidgetPropertyBox::sSink::OnDataTableSink_SetMetadata(iDataTable* apDT, tU32 anProperty)
{
  if (!mpParent) return;
  iWidgetTreeNode* pNode = mpParent->FindNode(NULL,apDT);
  if (pNode) {
    iWidgetTreeNode* pPropertyNode = pNode->GetChildNodeFromName(apDT->GetPropertyName(anProperty));
    ePropertyEditBoxType ebtypeProperty = mpParent->GetPropertyEditBoxType(apDT,anProperty);
    mpParent->SetEditBox(pPropertyNode,apDT,anProperty,ebtypeProperty);
  }
}

///////////////////////////////////////////////
void __stdcall cWidgetPropertyBox::sSink::OnDataTableSink_RemoveProperty(iDataTable* apDT, tU32 anProperty)
{
  if (!mpParent) return;
  iWidgetTreeNode* pNode = mpParent->FindNode(NULL,apDT);
  if (pNode) {
    iWidgetTreeNode* pPropertyNode = pNode->GetChildNodeFromName(apDT->GetPropertyName(anProperty));
    if (pPropertyNode) {
      mpParent->RemoveWidgetEditBox(pPropertyNode->GetWidget());
      pNode->RemoveChildNode(pPropertyNode);
    }
  }
}

///////////////////////////////////////////////
ni::Var __stdcall cWidgetPropertyBox::sSink::OnDataTableSink_GetProperty(ni::iDataTable *,ni::tU32)
{
  return niVarNull;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetPropertyBox::sSink::OnWidgetSink(iWidget* apWidget, tU32 anMsg, const Var& aA, const Var& aB)
{
  niGuardObject((iWidgetSink*)this);
  switch (anMsg) {
    case eUIMessage_Command:
      {
        Ptr<iWidgetCommand> ptrCmd = ni::VarQueryInterface<iWidgetCommand>(aA);
        niAssert(ptrCmd.IsOK());
        if (ptrCmd->GetSender() != mpParent->mpwTree &&
            (((ptrCmd->GetSender()->GetClassName() == _H("ScrollBar")) &&
              ptrCmd->GetID() == eWidgetScrollBarCmd_Scrolled) ||
             ((ptrCmd->GetSender()->GetClassName() == _H("EditBox")) &&
              ptrCmd->GetID() == eWidgetEditBoxCmd_Validated) ||
             ((ptrCmd->GetSender()->GetClassName() == _H("ComboBox")) &&
              ptrCmd->GetID() == eWidgetListBoxCmd_SelectionChanged) ||
             ((ptrCmd->GetSender()->GetClassName() == _H("Button")) &&
              (ptrCmd->GetID() == eWidgetButtonCmd_Checked ||
               ptrCmd->GetID() == eWidgetButtonCmd_UnChecked))))
        {
          mpParent->DoSetProperty(ptrCmd->GetSender());
        }
        break;
      }
    case eUIMessage_Notify:
      {
        QPtr<iMessageDesc> ptrMsg = aB;
        switch (ptrMsg->GetID()) {
          case eUIMessage_SetFocus: {
            if (mpParent->mpwTree.ptr() != aA.mpIUnknown) {
              sVec2f vPos = mpParent->mpWidget->GetUIContext()->GetCursorPosition()-mpParent->mpwTree->GetAbsolutePosition();
              mpParent->mpwTree->SendMessage(eUIMessage_LeftClickDown,vPos-mpParent->mpwTree->GetClientPosition(),vPos);
            }
            break;
          }
          case eUIMessage_LostFocus: {
            if (mpParent->mpwTree.ptr() != aA.mpIUnknown) {
              mpParent->DoSetProperty(VarQueryInterface<iWidget>(aA));
            }
            break;
          }
        }
        break;
      }
    case eUIMessage_NCPaint: {
      Ptr<iWidget> pwTree = mpParent ? mpParent->mpwTree.ptr() : NULL;
      QPtr<iWidgetTree> tree = pwTree.ptr();
      Ptr<iCanvas> canvas = VarQueryInterface<iCanvas>(aB);
      if (tree.IsOK() && canvas.IsOK()) {
        Ptr<iWidgetTreeNode> hoverNode;
        const sRectf clippedRect = apWidget->GetClippedClientRect();
        const sVec2f vMousePos = aA.GetVec2f();
        if (clippedRect.Intersect(vMousePos)) {
          hoverNode = tree->GetNodeFromPosition(vMousePos+apWidget->GetAbsolutePosition());
        }
        if (!hoverNode.IsOK()) {
          hoverNode = tree->GetNode(tree->GetHoverNode());
        }
        if (hoverNode.IsOK()) {
          tree->SetHoverNode(hoverNode->GetTreeIndex());
          sRectf nodeRect = hoverNode->GetScrolledNodeRect(); // Tree Client relative
          nodeRect.Move(pwTree->GetAbsolutePosition()); // Absolute
          nodeRect.Move(-apWidget->GetAbsolutePosition()); // Bg Client relative
          nodeRect.SetLeft(0);
          nodeRect.SetRight(apWidget->GetSize().x);
          canvas->BlitFillAlpha(nodeRect,mpParent->skin.mcolBarsHoverBackground);
        }
      }
      break;
    }
    default:
      return eFalse;
  }
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetPropertyBox::OnWidgetSink(iWidget* apWidget, tU32 anMsg, const Var& aA, const Var& aB)
{
  if (anMsg == eUIMessage_SkinChanged) {
    _UpdateSkin();
  }
  return eFalse;
}

///////////////////////////////////////////////
void cWidgetPropertyBox::FillNode(iDataTable* apDT, iWidgetTreeNode* apNode)
{
  apNode->Clear();
  apNode->SetName(apDT->GetName());
  apNode->SetWidget(NULL);
  apNode->SetFlags(apNode->GetFlags()|eWidgetTreeNodeFlags_Expandable);
  apNode->SetUserdata(apDT);
  for (tU32 id = 0; id < apDT->GetNumChildren(); ++id) {
    iDataTable* c = apDT->GetChildFromIndex(id);
    iWidgetTreeNode* pNewNode = apNode->AddChildNode(AZEROSTR);
    if (id == mnMaxNumChildren) {
      pNewNode->SetName(niFmt(_A("%s %d...%d\n"),
                              c->GetName(),
                              id,
                              apDT->GetNumChildren()));
      pNewNode->SetFlags(pNewNode->GetFlags()|eWidgetTreeNodeFlags_Expandable);
      break;
    }
    else {
      FillNode(c,pNewNode);
    }
  }
  for (tU32 ip = 0; ip < apDT->GetNumProperties(); ++ip) {
    ePropertyEditBoxType ebtypeProperty = GetPropertyEditBoxType(apDT,ip);
    if (ebtypeProperty == ePropertyEditBoxType_Hidden)
      continue;
    iWidgetTreeNode* pNewNode = apNode->AddChildNode(AZEROSTR);
    if (ip == mnMaxNumProperties) {
      pNewNode->SetName(niFmt(_A("Property %d...%d\n"),ip,apDT->GetNumProperties()));
      break;
    }
    else {
      pNewNode->SetName(apDT->GetPropertyName(ip));
      SetEditBox(pNewNode,apDT,ip,ebtypeProperty);
    }
  }
}

///////////////////////////////////////////////
void cWidgetPropertyBox::ClearNode(iWidgetTreeNode* apNode)
{
  apNode->SetUserdata(NULL);
  for (tU32 i = 0; i < apNode->GetNumChildNodes(); ++i) {
    ClearNode(apNode->GetChildNode(i));
  }
}

///////////////////////////////////////////////
iWidgetTreeNode* cWidgetPropertyBox::FindNode(iWidgetTreeNode* apNode, iDataTable* apDT)
{
  iWidgetTreeNode* pNode = apNode?apNode:QPtr<iWidgetTree>(mpwTree)->GetRootNode();
  if (pNode->GetUserdata() == apDT)
    return pNode;
  for (tU32 i = 0; i < pNode->GetNumChildNodes(); ++i) {
    iWidgetTreeNode* pNodeFound = FindNode(pNode->GetChildNode(i),apDT);
    if (pNodeFound)
      return pNodeFound;
  }
  return NULL;
}

///////////////////////////////////////////////
iWidget* cWidgetPropertyBox::CreateWidgetEditBox(ePropertyEditBoxType aType, iHString* ahspMD)
{
  if (aType == ePropertyEditBoxType_Hidden)
    return NULL;

  tEBLst* lst = NULL;
  tEBMap::iterator mapIt = mmapEB.find(ahspMD);
  if (mapIt == mmapEB.end()) {
    mapIt = astl::upsert(mmapEB,ahspMD,tEBLst());
    lst = &mapIt->second;
  }
  else {
    lst = &mapIt->second;
    niLoopit(tEBLst::iterator,ita,*lst) {
      sEditBox& eb = *ita;
      if (!eb.used && eb.type == aType) {
        eb.used = eTrue;
        eb.w->SetVisible(eTrue);
        eb.w->SetEnabled(eTrue);
        return eb.w;
      }
    }
  }

  iWidget* pEB = NULL;
  iUIContext* pCtx = mpWidget->GetUIContext();
  switch (aType) {
    case ePropertyEditBoxType_ROText:
    case ePropertyEditBoxType_Text:
      {
        pEB = pCtx->CreateWidget(_A("EditBox"),mpwEditBoxParent,sRectf(0,0,10,22),
                                 eWidgetStyle_DontSerialize|
                                 eWidgetStyle_NotifyParent|
                                 eWidgetEditBoxStyle_PasteValidate|
                                 ((aType==ePropertyEditBoxType_ROText)?
                                  (tU32)eWidgetEditBoxStyle_ReadOnly:
                                  (tU32)eWidgetStyle_DragDestination|eWidgetStyle_HoldFocus),
                                 NULL);
        pEB->SetSkinClass(_H("PropertyBoxEditBox"));
        if (aType==ePropertyEditBoxType_ROText) {
          pEB->SetIgnoreInput(eTrue);
        }
        break;
      }
    case ePropertyEditBoxType_ROTextML:
    case ePropertyEditBoxType_TextML:
      {
        pEB = pCtx->CreateWidget(_A("EditBox"),mpwEditBoxParent,sRectf(0,0,10,102),
                                 (eWidgetStyle_DontSerialize|
                                  eWidgetStyle_NotifyParent|
                                  eWidgetEditBoxStyle_MultiLine|
                                  eWidgetEditBoxStyle_PasteValidate)|
                                 ((aType==ePropertyEditBoxType_ROTextML)?
                                  (tU32)eWidgetEditBoxStyle_ReadOnly:(tU32)eWidgetStyle_HoldFocus),
                                 NULL);
        Ptr<iDataTableReadStack> ptrDT = ni::CreateDataTableReadStack(ParseTypeToDT(niHStr(ahspMD)));
        tU32 nNumLines = ptrDT->GetIntDefault(_A("numlines"),5);
        pEB->SetSize(Vec2<tF32>(10,pEB->GetFont()->GetHeight()*nNumLines+10));
        pEB->SetSkinClass(_H("PropertyBoxEditBox"));
        if (aType==ePropertyEditBoxType_ROTextML) {
          pEB->SetIgnoreInput(eTrue);
        }
        break;
      }
    case ePropertyEditBoxType_Color:
      {
        pEB = pCtx->CreateWidget(
            _A("ComboBox"),mpwEditBoxParent,sRectf(0,0,10,22),
            eWidgetStyle_DontSerialize|eWidgetStyle_HoldFocus|
            eWidgetComboBoxStyle_NoDefaultListBox,NULL);
        Ptr<iWidget> w = pCtx->CreateWidget(_A("ColorPicker"),pEB,sRectf(0,0,100,100),eWidgetStyle_DontSerialize|eWidgetStyle_HoldFocus,_H("ComboBox_ColorPicker"));
        w->SetRelativeSize(Vec2<tF32>(1.0f,0.0f));
        pEB->SetSkinClass(_H("PropertyBoxComboBox"));
        break;
      }
    case ePropertyEditBoxType_Font:
      {
        pEB = pCtx->CreateWidget(
            _A("ComboBox"),mpwEditBoxParent,sRectf(0,0,10,22),
            eWidgetStyle_DontSerialize|eWidgetStyle_HoldFocus|
            eWidgetComboBoxStyle_NoDefaultListBox,NULL);
        Ptr<iWidget> w = mpWidget->GetUIContext()->CreateWidget(
            _A("ListBox"),
            pEB,
            sRectf(0,0,100,80),
            eWidgetStyle_Free|
            eWidgetStyle_NotifyParent|
            eWidgetStyle_HoldFocus,
            _H("ComboBox_FontPicker"));
        w->SetRelativeSize(Vec2<tF32>(1.0f,0.0f));

        QPtr<iWidgetListBox> ptrLB = w.ptr();
        iGraphics* pGraphics = mpWidget->GetGraphics();

        astl::hstring_map_cmp<tInt> mapFonts;
        niLoop(i,pGraphics->GetNumFonts()) {
          iFont* f = pGraphics->GetFontFromIndex(i);
          if (!f) continue;
          //             ptrLB->AddItem(niHStr(f->GetName()));
          astl::upsert(mapFonts,f->GetName(),1);
        }
        niLoop(i,pGraphics->GetNumSystemFonts()) {
          //             ptrLB->AddItem(niHStr(pGraphics->GetSystemFontName(i)));
          astl::upsert(mapFonts,pGraphics->GetSystemFontName(i),1);
        }
        niLoopit(astl::hstring_map_cmp<tInt>::const_iterator,it,mapFonts) {
          ptrLB->AddItem(niHStr(it->first));
        }

        //         ptrLB->SetSortKey(0);
        //         ptrLB->SetSortAscendant(eTrue);
        QPtr<iWidgetComboBox> ptrCB = pEB;
        ptrCB->SetNumLines(ni::Max(ptrLB->GetNumItems(),10));
        pEB->SetSkinClass(_H("PropertyBoxComboBox"));
        break;
      }
    case ePropertyEditBoxType_Enum:
      {
        pEB = pCtx->CreateWidget(_A("ComboBox"),mpwEditBoxParent,sRectf(0,0,10,22),eWidgetStyle_DontSerialize|eWidgetStyle_HoldFocus|eWidgetComboBoxStyle_DropDown,NULL);
#ifdef USE_DROP_MENU_ENUM
        {
          Ptr<iWidget> wMenu = pCtx->CreateWidget(_A("Menu"),pEB,sRectf::Null(),0,NULL);
          wMenu->SetLocale(_H("native"));
          QPtr<iWidgetMenu> menu = wMenu;
          Ptr<iExpressionContext> ptrCtx = ni::CreateExpressionContext();
          ptrCtx->SetEnums(niHStr(ahspMD));
          const sEnumDef* pDefaultEnum = ptrCtx->GetDefaultEnumDef();
          for (tU32 i = 0; i < ptrCtx->GetNumEnums(); ++i) {
            const sEnumDef* pEnumDef = ptrCtx->GetEnumDef(ptrCtx->GetEnumName(i));
            for (tU32 j = 0; j < pEnumDef->GetNumElements(); ++j) {
              cString strItemName;
              if (pDefaultEnum != pEnumDef) {
                strItemName += pEnumDef->GetName();
                strItemName += _A(".");
              }
              strItemName += pEnumDef->GetElementName(j);
              menu->AddItem(strItemName.Chars(),NULL,eWidgetMenuItemFlags_Group);
            }
          }
          // menu->SortItems();
          QPtr<iWidgetComboBox> ptrCB = pEB;
          ptrCB->SetDroppedWidget(wMenu);
        }
#else
        {
          QPtr<iWidgetListBox> ptrLB = pEB;
          const sEnumDef* pEnumDef = _GetEnumDefFromMetadata(niHStr(ahspMD));
          if (pEnumDef) {
            for (tU32 j = 0; j < pEnumDef->mnNumValues; ++j) {
              cString strItemName;
              // strItemName += pEnumDef->maszName;
              // strItemName += _A(".");
              strItemName += pEnumDef->mpValues[j].maszName;
              /*tU32 nItem =*/ ptrLB->AddItem(strItemName.Chars());
            }
          }
          QPtr<iWidgetComboBox> ptrCB = pEB;
          ptrCB->SetNumLines(ptrLB->GetNumItems());
        }
#endif
        pEB->SetSkinClass(_H("PropertyBoxComboBox"));
        break;
      }
    case ePropertyEditBoxType_Flags:
      {
        pEB = pCtx->CreateWidget(_A("ComboBox"),mpwEditBoxParent,sRectf(0,0,10,22),eWidgetStyle_DontSerialize|eWidgetStyle_HoldFocus|eWidgetComboBoxStyle_Multiselect|eWidgetComboBoxStyle_ClickAddSelection,NULL);
#ifdef USE_DROP_MENU_FLAGS
        {
          Ptr<iWidget> wMenu = pCtx->CreateWidget(_A("Menu"),pEB,sRectf::Null(),0,NULL);
          wMenu->SetLocale(_H("native"));
          QPtr<iWidgetMenu> menu = wMenu.ptr();

          const sEnumDef* pEnumDef = _GetEnumDefFromMetadata(niHStr(ahspMD));
          if (pEnumDef) {
            for (tU32 j = 0; j < pEnumDef->mnNumValues; ++j) {
              cString strItemName;
              // strItemName += pEnumDef->maszName;
              // strItemName += _A(".");
              strItemName += pEnumDef->mpValues[j].maszName;
              menu->AddItem(strItemName.Chars(),NULL,eWidgetMenuItemFlags_Check);
            }
          }

          // menu->SortItems();
          QPtr<iWidgetComboBox> ptrCB = pEB;
          ptrCB->SetDroppedWidget(wMenu);
        }
#else
        {
          QPtr<iWidgetListBox> ptrLB = pEB;
          Ptr<iExpressionContext> ptrCtx = ni::CreateExpressionContext();
          ptrCtx->SetEnums(niHStr(ahspMD));
          const sEnumDef* pDefaultEnum = ptrCtx->GetDefaultEnumDef();
          for (tU32 i = 0; i < ptrCtx->GetNumEnums(); ++i) {
            const sEnumDef* pEnumDef = ptrCtx->GetEnumDef(ptrCtx->GetEnumName(i));
            for (tU32 j = 0; j < pEnumDef->GetNumElements(); ++j) {
              cString strItemName;
              if (pDefaultEnum != pEnumDef) {
                strItemName += pEnumDef->GetName();
                strItemName += _A(".");
              }
              strItemName += pEnumDef->GetElementName(j);
              tU32 nItem = ptrLB->AddItem(strItemName.Chars());
            }
          }
          QPtr<iWidgetComboBox> ptrCB = pEB;
          ptrCB->SetNumLines(ptrLB->GetNumItems());
        }
#endif
        pEB->SetSkinClass(_H("PropertyBoxComboBox"));
        break;
      }
    case ePropertyEditBoxType_Range:
      {
        pEB = pCtx->CreateWidget(_A("ScrollBar"),mpwEditBoxParent,sRectf(0,0,10,22),eWidgetStyle_DontSerialize|eWidgetStyle_HoldFocus|eWidgetScrollBarStyle_Slider|eWidgetScrollBarStyle_Horz,NULL);
        {
          QPtr<iWidgetScrollBar> ptrSB = pEB;
          Ptr<iDataTableReadStack> ptrDT = ni::CreateDataTableReadStack(ParseTypeToDT(niHStr(ahspMD)));
          ptrSB->SetScrollRange(Vec2<tF32>(
              ptrDT->GetFloatDefault(_A("min"),0.0f),
              ptrDT->GetFloatDefault(_A("max"),100.0f)));
          ptrSB->SetPageSize(
              ptrDT->GetFloatDefault(_A("step"),1.0f));
        }
        // pEB->SetSkinClass(_H("PropertyBoxScrollBar"));
        break;
      }
    case ePropertyEditBoxType_Bool:
      {
        pEB = pCtx->CreateWidget(_A("Button"),mpwEditBoxParent,sRectf(0,0,10,22),eWidgetStyle_DontSerialize|eWidgetStyle_HoldFocus|eWidgetButtonStyle_OnOff|eWidgetButtonStyle_CheckBox,NULL);
        pEB->SetSkinClass(_H("PropertyBoxButton"));
        break;
      }
    case ePropertyEditBoxType_Hidden:
    case ePropertyEditBoxType_Unknown:
      break;
  }
  if (niIsOK(pEB)) {
    pEB->SetDockStyle(eWidgetDockStyle_DockFillWidth);
    pEB->SetMargin(_kvEBMargin);
    sEditBox eb;
    eb.type = aType;
    eb.used = eTrue;
    eb.w = pEB;
    lst->push_back(eb);
  }
  return pEB;
}

///////////////////////////////////////////////
void cWidgetPropertyBox::RemoveWidgetEditBox(iWidget* apWidget)
{
  niAssert(niIsOK(apWidget));
  niLoopit(tEBMap::iterator,it,mmapEB) {
    niLoopit(tEBLst::iterator,ita,it->second) {
      sEditBox& eb = *ita;
      if (eb.w == apWidget) {
        eb.used = eFalse;
        eb.w->SetVisible(eFalse);
        eb.w->SetEnabled(eFalse);
        return;
      }
    }
  }
  //astl::find_erase(mlstEB,apWidget);
  //apWidget->Destroy();
}

///////////////////////////////////////////////
void cWidgetPropertyBox::ClearEditBoxes()
{
  if (mbClearEditWidgetsCache) {
    niLoopit(tEBMap::iterator,it,mmapEB) {
      niLoopit(tEBLst::iterator,ita,it->second) {
        sEditBox& eb = *ita;
        if (eb.w.IsOK()) {
          eb.w->Destroy();
        }
      }
    }
    mmapEB.clear();
    mbClearEditWidgetsCache = eFalse;
  }
  else {
    niLoopit(tEBMap::iterator,it,mmapEB) {
      niLoopit(tEBLst::iterator,ita,it->second) {
        sEditBox& eb = *ita;
        if (eb.w.IsOK()) {
          eb.used = eFalse;
          eb.w->SetVisible(eFalse);
          eb.w->SetEnabled(eFalse);
        }
      }
    }
  }
}

///////////////////////////////////////////////
void __stdcall cWidgetPropertyBox::ClearEditWidgetsCache()
{
  mbClearEditWidgetsCache = eTrue;
}

///////////////////////////////////////////////
void cWidgetPropertyBox::DoSetProperty(iWidget* apWidget)
{
  if (!niIsOK(apWidget)) return;
  Ptr<iWidgetTreeNode> ptrModifiedNode = QPtr<iWidgetTree>(mpwTree)->GetRootNode()->FindNodeFromWidget(apWidget);
  if (ptrModifiedNode.IsOK()) {
    iWidgetTreeNode* pNode = ptrModifiedNode->GetParentNode();
    niAssert(pNode);
    QPtr<iDataTable> ptrDT = pNode->GetUserdata();
    niAssert(ptrDT.IsOK());
    SetEditBoxInDataTable(ptrModifiedNode,ptrDT);
  }
}

///////////////////////////////////////////////
ePropertyEditBoxType cWidgetPropertyBox::GetPropertyEditBoxType(iDataTable* apDT,  tU32 anIndex)
{
  iHString* hspMD = apDT->GetMetadataFromIndex(anIndex);
  if (!ni::HStringIsEmpty(hspMD)) {
    cString strName;
    //cString(hspMD->GetString()).SplitNameFlags(&strName,NULL);
    ParseTypeDescEx(niHStr(hspMD),&strName,NULL,NULL);
    if (strName.IEq(_A("hidden"))) {
      return ePropertyEditBoxType_Hidden;
    }
    else if (strName.IEq(_A("enum"))) {
      return ePropertyEditBoxType_Enum;
    }
    else if (strName.IEq(_A("color"))) {
      return ePropertyEditBoxType_Color;
    }
    else if (strName.IEq(_A("font"))) {
      return ePropertyEditBoxType_Font;
    }
    else if (strName.IEq(_A("range"))) {
      return ePropertyEditBoxType_Range;
    }
    else if (strName.IEq(_A("bool")) || strName.IEq(_A("i8")) || strName.IEq(_A("u8"))) {
      return ePropertyEditBoxType_Bool;
    }
    else if (strName == _A("flags")) {
      return ePropertyEditBoxType_Flags;
    }
    else if (strName == _A("rotext") || strName == _A("ro")) {
      return ePropertyEditBoxType_ROText;
    }
    else if (strName == _A("ml") || strName == _A("textml")) {
      return ePropertyEditBoxType_TextML;
    }
    else if (strName == _A("roml") || strName == _A("rotextml")) {
      return ePropertyEditBoxType_ROTextML;
    }
  }
  return ePropertyEditBoxType_Unknown;
}

///////////////////////////////////////////////
ePropertyEditBoxType cWidgetPropertyBox::GetWidgetEditBoxType(iWidget* apWidget)
{
  if (!niIsOK(apWidget))
    return ePropertyEditBoxType_Unknown;

  if (apWidget->GetClassName() == _H("Button")) {
    if (apWidget->GetParent() == mpwEditBoxParent) {
      return ePropertyEditBoxType_Bool;
    }
    else {
      return ePropertyEditBoxType_Flags;
    }
  }
  else if (apWidget->GetClassName() == _H("ComboBox")) {
    if (apWidget->GetChildFromID(_H("ComboBox_ColorPicker")))
      return ePropertyEditBoxType_Color;
    else if (apWidget->GetChildFromID(_H("ComboBox_FontPicker")))
      return ePropertyEditBoxType_Font;

    return
        niFlagIs(apWidget->GetStyle(),eWidgetComboBoxStyle_Multiselect) ?
        ePropertyEditBoxType_Flags :
        ePropertyEditBoxType_Enum ;
  }
  else if (apWidget->GetClassName() == _H("ScrollBar")) {
    return ePropertyEditBoxType_Range;
  }
  else if (apWidget->GetClassName() == _H("EditBox")) {
    if (apWidget->GetStyle()&eWidgetEditBoxStyle_ReadOnly) {
      if (apWidget->GetStyle()&eWidgetEditBoxStyle_MultiLine)
        return ePropertyEditBoxType_ROTextML;
      else
        return ePropertyEditBoxType_ROText;
    }
    else {
      if (apWidget->GetStyle()&eWidgetEditBoxStyle_MultiLine)
        return ePropertyEditBoxType_TextML;
      else
        return ePropertyEditBoxType_Text;
    }
  }

  return ePropertyEditBoxType_Unknown;
}

///////////////////////////////////////////////
inline void _UpdateEBColor(iWidget* apEB)
{
  if (apEB) {
    Ptr<iWidget> w = apEB->GetChildFromID(_H("ComboBox_Button"));
    QPtr<iWidgetButton> b = w.ptr();
    if (b.IsOK()) {
      Ptr<iOverlay> o;
      sVec4f col = apEB->GetGraphics()->GetColor4FromName(apEB->GetText());
      o = w->GetGraphics()->CreateOverlayColor(col);
      o->SetBlendMode(eBlendMode_NoBlending);
      o->SetColor(col);
      o->SetSize(w->GetSize()-Vec2<tF32>(2,2));
      b->SetIcon(o);
      w->SetStyle(w->GetStyle()|eWidgetButtonStyle_BitmapButton);
    }
  }
}

///////////////////////////////////////////////
iWidget* cWidgetPropertyBox::SetEditBox(iWidgetTreeNode* apTreeNode, iDataTable* apDT, tU32 anProperty, ePropertyEditBoxType ebtypeProperty)
{
  if (!mSink.mpParent || !apTreeNode) {
    return NULL;
  }
  // if hidden dont set any edit box for it
  if (ebtypeProperty == ePropertyEditBoxType_Hidden)
    return NULL;

  iWidget* pEB = apTreeNode->GetWidget();
  ePropertyEditBoxType ebtypeWidget = GetWidgetEditBoxType(pEB);
  // if unknown, use the already existing widget type by default
  if (ebtypeProperty == ePropertyEditBoxType_Unknown)
    ebtypeProperty = ebtypeWidget;
  // if still unknown, then use a text field
  if (ebtypeProperty == ePropertyEditBoxType_Unknown)
    ebtypeProperty = ePropertyEditBoxType_Text;

  if (ebtypeWidget != ebtypeProperty) {
    if (pEB) {
      RemoveWidgetEditBox(pEB);
    }
    pEB = CreateWidgetEditBox(ebtypeProperty,apDT->GetMetadataFromIndex(anProperty));
    niAssert(niIsOK(pEB));
    QPtr<iWidgetEditBox> eb = pEB;
    if (eb.IsOK()) {
      eb->MoveCursorHome(ni::eFalse);
      eb->AutoScroll();
    }
    apTreeNode->SetFlags(apTreeNode->GetFlags()&(~eWidgetTreeNodeFlags_WidgetPlaceH));
    apTreeNode->SetWidget(pEB);
    //mlstEB.push_back(pEB);
  }

  switch (ebtypeProperty) {
    case ePropertyEditBoxType_ROText:
    case ePropertyEditBoxType_Text:
    case ePropertyEditBoxType_ROTextML:
    case ePropertyEditBoxType_TextML:
      {
        pEB->SetText(_H(apDT->GetStringFromIndex(anProperty)));
        break;
      }
    case ePropertyEditBoxType_Color:
      {
        cString strP = apDT->GetStringFromIndex(anProperty);
        pEB->SetText(_H(strP));
        _UpdateEBColor(pEB);
        break;
      }
    case ePropertyEditBoxType_Font:
    case ePropertyEditBoxType_Flags:
    case ePropertyEditBoxType_Enum:
      {
        cString strP = apDT->GetStringFromIndex(anProperty);
        //QPtr<iWidgetListBox>(pEB)->SetSelectedItem(0,aszP);
        pEB->SetText(_H(strP));
        break;
      }
    case ePropertyEditBoxType_Range:
      {
        tF32 f = apDT->GetFloatFromIndex(anProperty);
        QPtr<iWidgetScrollBar>(pEB)->SetScrollPosition(f);
        break;
      }
    case ePropertyEditBoxType_Bool:
      {
        tI32 b = apDT->GetIntFromIndex(anProperty);
        QPtr<iWidgetButton>(pEB)->SetCheck(b?eTrue:eFalse);
        break;
      }
    case ePropertyEditBoxType_Hidden:
    case ePropertyEditBoxType_Unknown:
      break;
  }

  return pEB;
}

///////////////////////////////////////////////
void cWidgetPropertyBox::SetEditBoxInDataTable(iWidgetTreeNode* apNode, iDataTable* apDT)
{
  cString strCurVal = apDT->GetString(apNode->GetName());
  cString strNewVal;

  ePropertyEditBoxType type = GetWidgetEditBoxType(apNode->GetWidget());
  if (type == ePropertyEditBoxType_Hidden)
    return;
  switch (type) {
    case ePropertyEditBoxType_ROText:
    case ePropertyEditBoxType_Text:
    case ePropertyEditBoxType_ROTextML:
    case ePropertyEditBoxType_TextML:
      {
        strNewVal = niHStr(apNode->GetWidget()->GetText());
        break;
      }
    case ePropertyEditBoxType_Color:
      {
        sVec4f col = mpWidget->GetGraphics()->GetColor4FromName(apNode->GetWidget()->GetText());
        strNewVal = cString(col);
        _UpdateEBColor(apNode->GetWidget());
        break;
      };
    case ePropertyEditBoxType_Font:
    case ePropertyEditBoxType_Flags:
    case ePropertyEditBoxType_Enum:
      {
        //strNewVal = QPtr<iWidgetListBox>(apNode->GetWidget())->GetSelectedItem(0);
        strNewVal = niHStr(apNode->GetWidget()->GetText());
        break;
      }
    case ePropertyEditBoxType_Range:
      {
        strNewVal.Set(QPtr<iWidgetScrollBar>(apNode->GetWidget())->GetScrollPosition());
        break;
      }
    case ePropertyEditBoxType_Bool:
      {
        strNewVal.Set(QPtr<iWidgetButton>(apNode->GetWidget())->GetCheck());
        break;
      }
    case ePropertyEditBoxType_Hidden:
    case ePropertyEditBoxType_Unknown:
      break;
  }

  if (strNewVal != strCurVal) {
    tU32 nIndex = apDT->GetPropertyIndex(apNode->GetName());
    if (nIndex != eInvalidHandle) {
      mSink.mnIgnoreSetProperty = nIndex;
      apDT->SetStringFromIndex(nIndex,strNewVal.Chars());
      mSink.mnIgnoreSetProperty = eInvalidHandle;
    }
  }
}

///////////////////////////////////////////////
void cWidgetPropertyBox::_UpdateSkin() {
  if (mpWidget && mpwTree.IsOK()) {
    skin.mcolBarsHoverBackground = ULColorBuild(
        mpwTree->FindSkinColor(sVec4f::White(),NULL,NULL,_H("RowHover")));
  }
}

#endif
