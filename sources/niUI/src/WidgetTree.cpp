// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "WidgetTree.h"
#include "HardCodedSkin.h"
#include <niLang/Utils/WeakPtr.h>

class cWidgetTreeNode;

#define TREENOTIFY_LAYOUT   niBit(0)
#define TREENOTIFY_SCROLLBARS niBit(1)
#define TREENOTIFY_CULLING    niBit(2)
#define TREENOTIFY_ALL      (TREENOTIFY_LAYOUT|TREENOTIFY_SCROLLBARS|TREENOTIFY_CULLING)
#define TREENOTIFY_NODES    TREENOTIFY_ALL
#define TREENOTIFY_EXPANDED   TREENOTIFY_ALL
#define TREENOTIFY_SETWIDGET  TREENOTIFY_CULLING

#define niFlagEq(n,p,x) (niFlagIs(n,x) == niFlagIs(p,x))

//--------------------------------------------------------------------------------------------
//
//  Tree Node
//
//--------------------------------------------------------------------------------------------
const tF32 _kfTabOffset = 16.0f;
const tF32 _kfTextHeightMargin = 4.0f;
const tF32 _kfTextLeftMargin = 2.0f;
const tF32 _kfTextRightMargin = 2.0f;
const sRectf _rectInvalid = sRectf(0,ni::TypeMax<tF32>(),0,0);
const tF32 _kfAttrSpacing = 2.0f;
const sVec2f _kvAttrSize = {16.0f,16.0f};

#define eWidgetTreeNodeFlags_Culled   niBit(16)

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidgetTreeNode declaration.
class cWidgetTreeNode : public ni::cIUnknownImpl<ni::iWidgetTreeNode,ni::eIUnknownImplFlags_Default>
{
  niBeginClass(cWidgetTreeNode);

 public:
  ///////////////////////////////////////////////
  cWidgetTreeNode(tU32 anIndex,
                  cWidgetTree* apParentWdg,
                  cWidgetTreeNode* apParentTreeNode,
                  tWidgetTreeNodeFlags aFlags = eWidgetTreeNodeFlags_Default)
  {
    mnDepth = 0;
    mnTreeIndex = eInvalidHandle;
    mnDrawIndex = 0;
    mnTextColor = 0;
    mnTextBackColor = 0;
    mNodeRect = _rectInvalid;
    mTextRect = _rectInvalid;
    mnAttributes = 0;
    mnVisibleAttributesMask = 0xFFFFFFFF;
    mnRowColor = 0;

    mnIndex = anIndex;
    mFlags = aFlags;

    mpwParentWidget = apParentWdg;
    mpwParentTreeNode = apParentTreeNode;
    Ptr<iWidgetTreeNode> n = (QPtr<cWidgetTreeNode>(mpwParentTreeNode)).ptr();
    while (n.IsOK()) {
      ++mnDepth;
      n = n->GetParentNode();
    }
    _SetCulling(eTrue);
    apParentWdg->_RegisterNode(this);
  }

  ///////////////////////////////////////////////
  ~cWidgetTreeNode() {
    Invalidate();
  }

  ///////////////////////////////////////////////
  ni::tBool __stdcall IsOK() const {
    niClassIsOK(cWidgetTreeNode);
    return mpwParentWidget.IsOK();
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
    QPtr<cWidgetTree> ptrParentWidget(mpwParentWidget);
    if (ptrParentWidget.IsOK()) {
      niGuardObject(this);
      SetWidget(NULL);
      Clear();
      {
        QPtr<cWidgetTreeNode> ptrParentTreeNode(mpwParentTreeNode);
        if (ptrParentTreeNode.IsOK()) {
          astl::find_erase(static_cast<cWidgetTreeNode*>(ptrParentTreeNode.ptr())->mvChildren,this);
          mpwParentTreeNode.SetNull();
        }
      }
      ptrParentWidget->_UnregisterNode(this);
      mpwParentWidget.SetNull();
    }
  }

  ///////////////////////////////////////////////
  iWidget * __stdcall GetParentWidget() const {
    QPtr<cWidgetTree> ptrParentWidget(mpwParentWidget);
    return ptrParentWidget.IsOK() ? ptrParentWidget->mpWidget : NULL;
  }

  ///////////////////////////////////////////////
  ni::iWidgetTreeNode * __stdcall GetParentNode() const {
    QPtr<cWidgetTreeNode> ptrParentTreeNode(mpwParentTreeNode);
    return ptrParentTreeNode.GetRawAndSetNull();
  }

  ///////////////////////////////////////////////
  sRectf __stdcall GetNodeRect() const {
    return mNodeRect;
  }

  ///////////////////////////////////////////////
  sRectf __stdcall GetScrolledNodeRect() const {
    QPtr<cWidgetTree> ptrParentWidget(mpwParentWidget);
    if (!ptrParentWidget.IsOK()) return sRectf::Null();
    return mNodeRect+ptrParentWidget->mvScrollOffset;
  }

  ///////////////////////////////////////////////
  sRectf __stdcall GetAbsoluteNodeRect() const {
    QPtr<cWidgetTree> ptrParentWidget(mpwParentWidget);
    if (!ptrParentWidget.IsOK()) return sRectf::Null();
    return mNodeRect + ptrParentWidget->mvScrollOffset +
        ptrParentWidget->mpWidget->GetClientPosition() +
        ptrParentWidget->mpWidget->GetAbsolutePosition();
  }

  ///////////////////////////////////////////////
  sRectf __stdcall GetTextRect() const {
    return mTextRect;
  }

  ///////////////////////////////////////////////
  sRectf __stdcall GetScrolledTextRect() const {
    QPtr<cWidgetTree> ptrParentWidget(mpwParentWidget);
    if (!ptrParentWidget.IsOK()) return sRectf::Null();
    return mTextRect + ptrParentWidget->mvScrollOffset;
  }

  ///////////////////////////////////////////////
  sRectf __stdcall GetAbsoluteTextRect() const {
    QPtr<cWidgetTree> ptrParentWidget(mpwParentWidget);
    if (!ptrParentWidget.IsOK()) return sRectf::Null();
    return mTextRect +
        ptrParentWidget->mvScrollOffset +
        ptrParentWidget->mpWidget->GetClientPosition() +
        ptrParentWidget->mpWidget->GetAbsolutePosition();
  }

  ///////////////////////////////////////////////
  tU32 __stdcall GetDepth() const {
    return mnDepth;
  }

  ///////////////////////////////////////////////
  tU32 __stdcall GetIndex() const {
    return mnIndex;
  }

  ///////////////////////////////////////////////
  tU32 __stdcall GetNumChildNodes() const {
    return mvChildren.size();
  }

  ///////////////////////////////////////////////
  ni::iWidgetTreeNode * __stdcall GetChildNode(tU32 anIndex) const {
    if (anIndex >= mvChildren.size()) return NULL;
    return mvChildren[anIndex];
  }

  ///////////////////////////////////////////////
  ni::iWidgetTreeNode * __stdcall GetChildNodeFromName(const achar *aaszName) const {
    niLoopit(tTreeNodeVec::const_iterator,it,mvChildren) {
      if (ni::StrEq((*it)->GetName(),aaszName)) return *it;
      ni::iWidgetTreeNode* pNode = (*it)->GetChildNodeFromName(aaszName);
      if (pNode) return pNode;
    }
    return NULL;
  }

  ///////////////////////////////////////////////
  tU32 __stdcall GetChildNodeIndex(const iWidgetTreeNode* apNode) const {
    niLoop(i,mvChildren.size()) {
      if (mvChildren[i].ptr() == apNode)
        return i;
    }
    return eInvalidHandle;
  }

  ///////////////////////////////////////////////
  ni::iWidgetTreeNode * __stdcall AddChildNode(const achar *aaszName) {
    QPtr<cWidgetTree> ptrParentWidget(mpwParentWidget);
    if (!ptrParentWidget.IsOK()) return NULL;
    Ptr<cWidgetTreeNode> ptrNew = niNew cWidgetTreeNode(mvChildren.size(),ptrParentWidget,this);
    ptrNew->SetName(aaszName);
    mvChildren.push_back(ptrNew);
    ptrParentWidget->_NotifyUpdateLayout(TREENOTIFY_NODES);
    return ptrNew.GetRawAndSetNull();
  }

  ///////////////////////////////////////////////
  ni::iWidgetTreeNode * __stdcall AddChildNodeBefore(const achar *aaszName, tU32 anIndex) {
    QPtr<cWidgetTree> ptrParentWidget(mpwParentWidget);
    if (!ptrParentWidget.IsOK()) return NULL;
    Ptr<cWidgetTreeNode> ptrNew = niNew cWidgetTreeNode(mvChildren.size(),ptrParentWidget,this);
    ptrNew->SetName(aaszName);
    if (anIndex >= mvChildren.size()) {
      mvChildren.push_back(ptrNew);
    }
    else {
      mvChildren.insert(mvChildren.begin()+anIndex,ptrNew);
    }
    ptrParentWidget->_NotifyUpdateLayout(TREENOTIFY_NODES);
    return ptrNew.ptr();
  }

  ///////////////////////////////////////////////
  tBool __stdcall RemoveChildNode(ni::iWidgetTreeNode *apNode) {
    QPtr<cWidgetTree> ptrParentWidget(mpwParentWidget);
    if (!ptrParentWidget.IsOK()) return NULL;
    tU32 index = GetChildNodeIndex(apNode);
    if (index != eInvalidHandle) {
      apNode->Invalidate();
      return eTrue;
    }
    else {
      niLoopit(tTreeNodeVec::const_iterator,it,mvChildren) {
        if ((*it)->RemoveChildNode(apNode)) {
          return eTrue;
        }
      }
    }
    return eFalse;
  }

  ///////////////////////////////////////////////
  tBool __stdcall Clear() {
    QPtr<cWidgetTree> ptrParentWidget(mpwParentWidget);
    if (!ptrParentWidget.IsOK()) return eFalse;
    while (!mvChildren.empty()) {
      mvChildren.back()->Invalidate();
    }
    return eTrue;
  }

  ///////////////////////////////////////////////
  iWidgetTreeNode* __stdcall FindNodeFromName(const achar* aaszName) const {
    QPtr<cWidgetTree> ptrParentWidget(mpwParentWidget);
    if (!ptrParentWidget.IsOK()) return NULL;
    iWidgetTreeNode* pRet = NULL;
    if (ni::StrEq(GetName(),aaszName)) {
      pRet = const_cast<cWidgetTreeNode*>(this);
    }
    else {
      for (tU32 i = 0; i < GetNumChildNodes(); ++i) {
        pRet = GetChildNode(i)->FindNodeFromName(aaszName);
        if (pRet)
          break;
      }
    }
    return pRet;
  }

  ///////////////////////////////////////////////
  iWidgetTreeNode* __stdcall FindNodeFromUserdata(const iUnknown* apUserdata) const {
    QPtr<cWidgetTree> ptrParentWidget(mpwParentWidget);
    if (!ptrParentWidget.IsOK()) return NULL;
    iWidgetTreeNode* pRet = NULL;
    if (GetUserdata() == apUserdata) {
      pRet = const_cast<cWidgetTreeNode*>(this);
    }
    else {
      for (tU32 i = 0; i < GetNumChildNodes(); ++i) {
        pRet = GetChildNode(i)->FindNodeFromUserdata(apUserdata);
        if (pRet)
          break;
      }
    }
    return pRet;
  }

  ///////////////////////////////////////////////
  iWidgetTreeNode* __stdcall FindNodeFromWidget(const iWidget* apWidget) const {
    QPtr<cWidgetTree> ptrParentWidget(mpwParentWidget);
    if (!ptrParentWidget.IsOK()) return NULL;
    iWidgetTreeNode* pRet = NULL;
    if (GetWidget() == apWidget) {
      pRet = const_cast<cWidgetTreeNode*>(this);
    }
    else {
      for (tU32 i = 0; i < GetNumChildNodes(); ++i) {
        pRet = GetChildNode(i)->FindNodeFromWidget(apWidget);
        if (pRet)
          break;
      }
    }
    return pRet;
  }

  ///////////////////////////////////////////////
  iWidgetTreeNode* __stdcall FindNodeFromPosition(const sVec2f& avPos) const {
    QPtr<cWidgetTree> ptrParentWidget(mpwParentWidget);
    if (!ptrParentWidget.IsOK()) return NULL;
    iWidgetTreeNode* pRet = NULL;
    if (mNodeRect.Intersect(avPos)) {
      pRet = niThis(cWidgetTreeNode);
    }
    else {
      for (tU32 i = 0; i < GetNumChildNodes(); ++i) {
        pRet = GetChildNode(i)->FindNodeFromPosition(avPos);
        if (pRet)
          break;
      }
    }
    return pRet;
  }

  ///////////////////////////////////////////////
  iWidgetTreeNode* __stdcall GetPrevSibling() const {
    QPtr<cWidgetTreeNode> ptrParentTreeNode(mpwParentTreeNode);
    if (!ptrParentTreeNode.IsOK()) return NULL;
    tU32 nIndex = ptrParentTreeNode->GetChildNodeIndex(this);
    niAssert(nIndex < ptrParentTreeNode->GetNumChildNodes());
    while (nIndex != 0) {
      --nIndex;
      cWidgetTreeNode* pRet = (cWidgetTreeNode*)ptrParentTreeNode->GetChildNode(nIndex);
      if (pRet->GetIsVisible()) {
        return pRet;
      }
    }
    return NULL;
  }

  ///////////////////////////////////////////////
  iWidgetTreeNode* __stdcall GetNextSibling() const {
    QPtr<cWidgetTreeNode> ptrParentTreeNode(mpwParentTreeNode);
    if (!ptrParentTreeNode.IsOK()) return NULL;
    tU32 nIndex = ptrParentTreeNode->GetChildNodeIndex(this);
    niAssert(nIndex < ptrParentTreeNode->GetNumChildNodes());
    while (nIndex != ptrParentTreeNode->GetNumChildNodes() -1) {
      ++nIndex;
      cWidgetTreeNode* pRet = (cWidgetTreeNode*)ptrParentTreeNode->GetChildNode(nIndex);
      if (pRet->GetIsVisible()) {
        return pRet;
      }
    }
    return NULL;
  }

  ///////////////////////////////////////////////
  iWidgetTreeNode* __stdcall GetAbove() const {
    QPtr<cWidgetTreeNode> ptrParentTreeNode(mpwParentTreeNode);

    // no parent node, so its the root nothing could be drawn above it
    if (!ptrParentTreeNode.IsOK())
      return NULL;

    // get the previous sibling
    iWidgetTreeNode* pRet = GetPrevSibling();
    if (pRet) {
      // if the previous sibling is expanded and has children, return its last child
      while (pRet->GetNumChildNodes() && pRet->GetExpanded()) {
        pRet = pRet->GetChildNode(pRet->GetNumChildNodes()-1);
      }
    }
    else {
      // if no previous sibling, the parent is above
      pRet = ptrParentTreeNode.GetRawAndSetNull();
    }

    // check if the node above is visible
    if (pRet && !((cWidgetTreeNode*)pRet)->GetIsVisible()) {
      return pRet->GetAbove();
    }

    return pRet;
  }

  ///////////////////////////////////////////////
  iWidgetTreeNode* __stdcall GetBelow() const {
    QPtr<cWidgetTree> ptrParentWidget(mpwParentWidget);
    if (!ptrParentWidget.IsOK())
      return NULL;

    // if has children and is expanded, return the first child
    if (!mvChildren.empty() && GetExpanded()) {
      iWidgetTreeNode* pRet = mvChildren[0];
      if (((cWidgetTreeNode*)pRet)->GetIsVisible()) {
        return pRet;
      }

      pRet = pRet->GetNextSibling();
      if (pRet)
        return pRet;
    }

    // not expanded or no children, get the first sibling
    iWidgetTreeNode* pRet = GetNextSibling();
    if (pRet)
      return pRet;

    // if no next sibling, the first parent that has a sibiling
    pRet = (QPtr<cWidgetTreeNode>(mpwParentTreeNode)).ptr();
    while (pRet) {
      iWidgetTreeNode* pSib = pRet->GetNextSibling();
      if (pSib)
        return pSib;
      pRet = pRet->GetParentNode();
    }

    return NULL;
  }

  ///////////////////////////////////////////////
  tBool __stdcall SetFlags(tWidgetTreeNodeFlags aFlags) {
    tU32 nPrevFlags = mFlags;
    mFlags = aFlags;
    _NotifyFlagsChange(mFlags,nPrevFlags);
    return eTrue;
  }

  ///////////////////////////////////////////////
  tWidgetTreeNodeFlags __stdcall GetFlags() const {
    return mFlags;
  }

  ///////////////////////////////////////////////
  tBool __stdcall SetName(const achar *aVal) {
    Var prevName = mstrName;
    mstrName = aVal;

    QPtr<cWidgetTree> ptrParentWidget(mpwParentWidget);
    if (ptrParentWidget.IsOK()) {
      ptrParentWidget->_NotifyUpdateLayout(TREENOTIFY_NODES);
    }
    _Notify(eWidgetTreeCmd_SetName,prevName);
    return eTrue;
  }

  ///////////////////////////////////////////////
  const achar * __stdcall GetName() const {
    return mstrName.Chars();
  }

  ///////////////////////////////////////////////
  tBool __stdcall SetIcon(iOverlay *aVal) {
    mptrIcon = aVal;
    return mptrIcon.IsOK();
  }

  ///////////////////////////////////////////////
  iOverlay * __stdcall GetIcon() const {
    return mptrIcon;
  }

  ///////////////////////////////////////////////
  tBool __stdcall SetSelected(tBool abSelected) {
    QPtr<cWidgetTree> ptrParentWidget(mpwParentWidget);
    if (!ptrParentWidget.IsOK())
      return eFalse;
    if (niFlagIs(mFlags,eWidgetTreeNodeFlags_Selected) == abSelected)
      return eTrue;
    tU32 nPrevFlags = mFlags;
    niFlagOnIf(mFlags,eWidgetTreeNodeFlags_Selected,abSelected);
    if (abSelected) {
      ptrParentWidget->_AddSelectedNode(this);
      // expand all parents, a selected node has to be visible
#pragma niTodo("Could add a flag to disable this.")
      iWidgetTreeNode* pParent = (QPtr<cWidgetTreeNode>(mpwParentTreeNode)).ptr();
      while (pParent) {
        pParent->SetExpanded(eTrue);
        pParent = pParent->GetParentNode();
      }
    }
    else {
      ptrParentWidget->_RemoveSelectedNode(this);
    }
    _NotifyFlagsChange(mFlags,nPrevFlags);
    if (abSelected) {
      ptrParentWidget->ScrollToNode(this);
    }
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall GetSelected() const {
    return niFlagIs(mFlags,eWidgetTreeNodeFlags_Selected);
  }

  ///////////////////////////////////////////////
  tBool __stdcall SetExpanded(tBool abExpanded) {
    tU32 nPrevFlags = mFlags;
    niFlagOnIf(mFlags,eWidgetTreeNodeFlags_Expanded,abExpanded);
    for (tU32 i = 0; i < GetNumChildNodes(); ++i) {
      iWidgetTreeNode* pNode = GetChildNode(i);
      niStaticCast(cWidgetTreeNode*,pNode)->_SetParentExpanded(abExpanded);
    }

    _NotifyFlagsChange(mFlags,nPrevFlags);
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall GetExpanded() const {
    return niFlagIs(mFlags,eWidgetTreeNodeFlags_Expanded);
  }

  ///////////////////////////////////////////////
  tBool __stdcall SetUserdata(iUnknown *apUserData) {
    mptrUserdata = apUserData;
    return mptrUserdata.IsOK();
  }

  ///////////////////////////////////////////////
  iUnknown * __stdcall GetUserdata() const {
    return mptrUserdata;
  }

  ///////////////////////////////////////////////
  tBool __stdcall SetWidget(iWidget* apWidget) {
    if (apWidget == mptrWidget)
      return eTrue;

    if (mptrWidget.IsOK()) {
      if (niFlagIs(mptrWidget->GetStyle(),eWidgetStyle_ItemOwned)) {
        mptrWidget->Destroy();
        mptrWidget = NULL;
      }
    }
    mptrWidget = apWidget;
    if (mptrWidget.IsOK()) {
      mptrWidget->SetVisible(eFalse);
      mptrWidget->SetEnabled(eFalse);
      mfWidgetAH = ni::FDiv(mptrWidget->GetSize().y,mptrWidget->GetSize().x);
    }

    QPtr<cWidgetTree> ptrParentWidget(mpwParentWidget);
    if (ptrParentWidget.IsOK()) {
      ptrParentWidget->_NotifyUpdateLayout(TREENOTIFY_SETWIDGET);
    }
    return eTrue;
  }

  ///////////////////////////////////////////////
  iWidget* __stdcall GetWidget() const {
    return mptrWidget;
  }

  ///////////////////////////////////////////////
  void __stdcall SetTextColor(tU32 anColor) {
    mnTextColor = anColor;
  }
  tU32 __stdcall GetTextColor() const  {
    return mnTextColor;
  }

  ///////////////////////////////////////////////
  void __stdcall SetTextBackColor(tU32 anColor) {
    mnTextBackColor = anColor;
  }
  tU32 __stdcall GetTextBackColor() const {
    return mnTextBackColor;
  }

  ///////////////////////////////////////////////
  tBool __stdcall GetIsVisible() const {
    if (!niFlagIs(mFlags,eWidgetTreeNodeFlags_Visible))
      return eFalse;

    iWidgetTreeNode* p = (QPtr<cWidgetTreeNode>(mpwParentTreeNode)).ptr();
    while (p) {
      if (!p->GetExpanded())
        return eFalse;
      p = p->GetParentNode();
    }

    return eTrue;
  }

  ///////////////////////////////////////////////
  void __stdcall SetAttributes(tU32 anAttributes) {
    if (mnAttributes != anAttributes) {
      const tU32 prevAttr = mnAttributes;
      mnAttributes = anAttributes;
      _Notify(eWidgetTreeCmd_SetAttributes,prevAttr);
    }
  }
  tU32 __stdcall GetAttributes() const {
    return mnAttributes;
  }

  ///////////////////////////////////////////////
  void __stdcall SetVisibleAttributesMask(tU32 anVisibleAttributesMask) {
    mnVisibleAttributesMask = anVisibleAttributesMask;
  }
  tU32 __stdcall GetVisibleAttributesMask() const {
    return mnVisibleAttributesMask;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetRowColor(tU32 anRow) {
    mnRowColor = anRow;
  }
  virtual tU32 __stdcall GetRowColor() const {
    return mnRowColor;
  }

  ///////////////////////////////////////////////
  iFont* __stdcall _GetFont() const {
    QPtr<cWidgetTree> ptrParentWidget(mpwParentWidget);
    if (ptrParentWidget.IsOK()) {
      return ptrParentWidget->mpWidget->GetFont();
    }
    return NULL;
  }

  ///////////////////////////////////////////////
  void _NotifyFlagsChange(tU32 anFlags, tU32 anPrevFlags) {
    if (anFlags == anPrevFlags)
      return;

    if (niFlagIs(anPrevFlags,eWidgetTreeNodeFlags_Selected) != niFlagIs(anFlags,eWidgetTreeNodeFlags_Selected)) {
      // Unselected
      if (niFlagIs(anPrevFlags,eWidgetTreeNodeFlags_Selected) &&
          niFlagIsNot(anFlags,eWidgetTreeNodeFlags_Selected))
      {
        _Notify(eWidgetTreeCmd_Unselected);
      }
      // Selected
      else if (niFlagIs(anFlags,eWidgetTreeNodeFlags_Selected) &&
               niFlagIsNot(anPrevFlags,eWidgetTreeNodeFlags_Selected))
      {
        _Notify(eWidgetTreeCmd_Selected);
      }
    }

    if (niFlagIs(anPrevFlags,eWidgetTreeNodeFlags_Expanded) != niFlagIs(anFlags,eWidgetTreeNodeFlags_Expanded)) {
      // Collapsed
      if (niFlagIs(anPrevFlags,eWidgetTreeNodeFlags_Expanded) &&
          niFlagIsNot(anFlags,eWidgetTreeNodeFlags_Expanded))
      {
        _Notify(eWidgetTreeCmd_Collapsed);
        QPtr<cWidgetTree> ptrParentWidget(mpwParentWidget);
        if (ptrParentWidget.IsOK()) {
          ptrParentWidget->_NotifyUpdateLayout(TREENOTIFY_EXPANDED);
        }
      }
      // Expanded
      else if (niFlagIs(anFlags,eWidgetTreeNodeFlags_Expanded) &&
               niFlagIsNot(anPrevFlags,eWidgetTreeNodeFlags_Expanded))
      {
        _Notify(eWidgetTreeCmd_Expanded);
        QPtr<cWidgetTree> ptrParentWidget(mpwParentWidget);
        if (ptrParentWidget.IsOK()) {
          ptrParentWidget->_NotifyUpdateLayout(TREENOTIFY_EXPANDED);
        }
      }
    }
  }

  ///////////////////////////////////////////////
  void _Notify(eWidgetTreeCmd aMsg, const Var& avarA = niVarNull) {
    QPtr<cWidgetTree> ptrParentWidget(mpwParentWidget);
    if (ptrParentWidget.IsOK() && ptrParentWidget->mpWidget) {
      ptrParentWidget->mpWidget->SendCommand(
          ptrParentWidget->mpWidget->GetParent(),aMsg,this,avarA);
    }
  }

  ///////////////////////////////////////////////
  tBool _SetParentExpanded(tBool abExpanded) {
#pragma niTodo("Add a flag to determin whether we want non-visible nodes to be automatically deselected")
    if (niFlagIs(mFlags,eWidgetTreeNodeFlags_Selected) && !abExpanded) {
      SetSelected(eFalse);
    }
    for (tU32 i = 0; i < GetNumChildNodes(); ++i) {
      iWidgetTreeNode* pNode = GetChildNode(i);
      // if we are displaying we need to make sure that the parent is really expanded,
      // if we are hiding all the widgets need to be hidden no matter what
      niStaticCast(cWidgetTreeNode*,pNode)->_SetParentExpanded(!abExpanded ? abExpanded : GetExpanded());
    }
    return eTrue;
  }


  ///////////////////////////////////////////////
  void _UpdateWidgetPosition() {
    tBool bVisible = GetIsVisible() && !_GetCulling();
    if (mptrWidget.IsOK()) {
      mptrWidget->SetVisible(bVisible);
      mptrWidget->SetEnabled(bVisible);

      QPtr<cWidgetTree> ptrParentWidget(mpwParentWidget);
      if (bVisible && ptrParentWidget.IsOK()) {
        sVec2f treePos = ptrParentWidget->mpWidget->GetAbsolutePosition()+
            ptrParentWidget->mpWidget->GetClientPosition();
        sVec2f newPos = mptrWidget->GetAbsolutePosition();
        sRectf nodeRect = GetScrolledNodeRect();
        const sVec4f& margin = mptrWidget->GetMargin();
        if (niFlagIs(mFlags,eWidgetTreeNodeFlags_WidgetPlaceV)) {
          newPos.y = treePos.y+nodeRect.Top()+margin.y;
        }
        if (niFlagIs(mFlags,eWidgetTreeNodeFlags_WidgetPlaceH)) {
          if (niFlagIs(mFlags,eWidgetTreeNodeFlags_WidgetLeft)) {
            newPos.x = treePos.x+nodeRect.Left()+margin.x;
            mTextRect.Move(Vec2<tF32>(mptrWidget->GetSize().x+margin.x+margin.z,0));
          }
          else {
            newPos.x = ptrParentWidget->mpWidget->GetAbsoluteRect().GetRight() - mptrWidget->GetSize().x - margin.z;
          }
        }
        mptrWidget->SetAbsolutePosition(newPos);
      }
    }
  }


  ///////////////////////////////////////////////
  void _ComputeNodeRect() {
    if (!GetIsVisible()) {
      mNodeRect = _rectInvalid;
      mTextRect = _rectInvalid;
    }
    else {
      QPtr<cWidgetTree> ptrParentWidget(mpwParentWidget);
      if (!ptrParentWidget.IsOK())
        return;

      const tF32 fh = _GetFont()->GetHeight();
      const sVec4f frame = ptrParentWidget->_GetItemFrameRect();
      const tBool bDontDrawRoot = niFlagIs(ptrParentWidget->mpWidget->GetStyle(),eWidgetTreeStyle_DontDrawRoot);

      mNodeRect.Left() = (mnDepth*_kfTabOffset);
      cWidgetTreeNode* n = (cWidgetTreeNode*)GetAbove();
      if (!n) {
        if (bDontDrawRoot) {
          mNodeRect = sRectf::Null();
          //                     mTextRect = _rectInvalid;
          return;
        }
        else {
          mNodeRect.Top() = 0.0f;
        }
      }
      else {
        mNodeRect.Top() = n->GetNodeRect().Bottom();
      }
      tF32 w = ptrParentWidget->mpWidget->GetSize().x-mNodeRect.Left()-frame.Right();
      if (ptrParentWidget->mptrVScroll.IsOK() && ptrParentWidget->mptrVScroll->GetVisible()) {
        w -= ptrParentWidget->mptrVScroll->GetSize().x;
      }
      mNodeRect.SetWidth(w);
      mNodeRect.SetHeight(fh+_kfTextHeightMargin);

      // compute the inital text size
      if (niFlagIs(mFlags,eWidgetTreeNodeFlags_DontDrawName)) {
        mTextRect = _rectInvalid;
      }
      else {
        mTextRect = _GetFont()->ComputeTextSize(sRectf(mNodeRect.GetTopLeft()),mstrName.Chars(),0);
        mNodeRect.SetWidth(mTextRect.GetWidth());
        mNodeRect.SetHeight(ni::Max(mTextRect.GetHeight(),mNodeRect.GetHeight(),fh));
        mTextRect = sRectf(mNodeRect.Left()+_kfTextLeftMargin,mNodeRect.Top()+frame.Top(),mNodeRect.GetWidth()-(_kfTextLeftMargin+_kfTextRightMargin),mNodeRect.GetHeight());
      }

      if (mptrWidget.IsOK()) {
        const sVec4f& margin = mptrWidget->GetMargin();
        if (niFlagIs(mFlags,eWidgetTreeNodeFlags_WidgetSize)) {
          const tF32 newW = mNodeRect.GetWidth();
          const tF32 newH = newW * mfWidgetAH;
          mptrWidget->SetSize(Vec2(newW-margin.x-margin.z,newH-margin.y-margin.w));
        }
        if (niFlagIs(mFlags,eWidgetTreeNodeFlags_WidgetPlaceV)) {
          const tF32 wh = mptrWidget->GetSize().y+margin.y+margin.w;
          mNodeRect.SetHeight(ni::Max(mNodeRect.GetHeight(),wh));
        }
      }

      mNodeRect.Bottom() += frame.Top()+frame.Bottom();
    }
  }


  ///////////////////////////////////////////////
  void _SetCulling(tBool abCulled) {
    niFlagOnIf(mFlags,eWidgetTreeNodeFlags_Culled,abCulled);
  }

  ///////////////////////////////////////////////
  tBool _GetCulling() const {
    return niFlagIs(mFlags,eWidgetTreeNodeFlags_Culled);
  }


  ///////////////////////////////////////////////
  void _SetTreeIndex(tU32 anTreeIndex) {
    mnTreeIndex = anTreeIndex;
  }

  ///////////////////////////////////////////////
  tU32 __stdcall GetTreeIndex() const {
    return mnTreeIndex;
  }

  ///////////////////////////////////////////////
  void _UpdateDrawIndex() {
    mnDrawIndex = 0;
    cWidgetTreeNode* n = (cWidgetTreeNode*)GetAbove();
    if (n) {
      while (n) {
        ++mnDrawIndex;
        n = (cWidgetTreeNode*)n->GetAbove();
      }
    }
  }

  ///////////////////////////////////////////////
  tU32 _GetDrawIndex() const {
    return mnDrawIndex;
  }

  ///////////////////////////////////////////////
  sRectf __stdcall _GetAttributeRect(const sVec2f& avSize, tU32 anRightIndex, tF32 afRightMargin) const {
    sRectf r = GetScrolledNodeRect();
    r.SetLeft(afRightMargin - ((tF32)(anRightIndex+1) * (avSize.x+_kfAttrSpacing)));
    r.SetTop(r.GetTop() + (r.GetHeight()/2 - avSize.y/2));
    r.SetRight(r.GetLeft() + avSize.x);
    r.SetBottom(r.GetTop() + avSize.y);
    return r;
  }
  sRectf __stdcall _GetAttributeRectBg(const sVec2f& avSize, tU32 anRightIndex, tF32 afRightMargin) const {
    sRectf r = GetScrolledNodeRect();
    r.SetLeft(afRightMargin - ((tF32)(anRightIndex+1) * (avSize.x+_kfAttrSpacing)));
    r.SetRight(r.GetLeft() + avSize.x +_kfAttrSpacing);
    return r;
  }

 public:
  WeakPtr<cWidgetTree>    mpwParentWidget;
  WeakPtr<cWidgetTreeNode>  mpwParentTreeNode;
  tU32        mnDepth;
  tU32        mnIndex;
  tU32        mnTreeIndex;
  tU32        mnDrawIndex;
  tTreeNodeVec    mvChildren;
  cString       mstrName;
  tWidgetTreeNodeFlags  mFlags;
  Ptr<iOverlay> mptrIcon;
  Ptr<iUnknown> mptrUserdata;
  Ptr<iWidget>  mptrWidget;
  tF32                mfWidgetAH;
  tU32        mnTextColor;
  tU32        mnTextBackColor;
  sRectf     mNodeRect;
  sRectf     mTextRect;
  tU32                mnAttributes;
  tU32                mnVisibleAttributesMask;
  tU32                mnRowColor;

  niEndClass(cWidgetTreeNode);
};

//--------------------------------------------------------------------------------------------
//
//  Tree
//
//--------------------------------------------------------------------------------------------
// const tU32 _knTimerProcessLeftClickUp = eWidgetSystemTimer_Last+1;
// const tF32 _kfTimerTimeProcessLeftClickUp = 0.5f;
const tF32 _kfLeftMargin = 2;
const tU32 _knDropBorderDistance = 4;

#define SCROLL_SCALEX tF32(mpWidget->GetFont()->GetMaxCharWidth())
#define SCROLL_SCALEY tF32(mpWidget->GetFont()->GetMaxCharHeight())

#pragma niTodo("Don't recompute the scroll bar layouts when changing the scroll position.")
#pragma niTodo("Recompute the layout only when necessary, for that need to finish the notification.")
#pragma niTodo("Draw icons, keyboard input, multiple selection, optimization of the rendering, send notification messages.")
#pragma niTodo("Add a flag to allow multiple selection or not")


///////////////////////////////////////////////
static inline tU32 _GetTextFrontColor(cWidgetTree* apTree, cWidgetTreeNode* apNode)
{
  tU32 frontColor = (apNode->GetTextColor() > 0) ?
      apNode->GetTextColor() : apTree->skin.colTextFront;
  return frontColor;
}

///////////////////////////////////////////////
static inline void _UpdateNodeLayout(cWidgetTreeNode* apNode, tF32& afMaxX, tF32& afHeight)
{
  apNode->_ComputeNodeRect();
  //niPrintln(niFmt(_A("Layout Node '%s' (%s) is visible\n"),apNode->GetName(),cString(apNode->GetNodeRect(),_A("Rect")).Chars()));
  sRectf nodeRect = apNode->GetNodeRect();
  if (nodeRect.Right() > afMaxX)
    afMaxX = nodeRect.Right();
  afHeight += nodeRect.GetHeight();
  for (tU32 i = 0; i < apNode->GetNumChildNodes(); ++i) {
    _UpdateNodeLayout((cWidgetTreeNode*)apNode->GetChildNode(i),afMaxX,afHeight);
  }
}

///////////////////////////////////////////////
static inline void _PushChars(cWidgetTree* apTree, iCanvas* c, iFont* apFont, cWidgetTreeNode* apNode, tF32 afX)
{
  tU32 nodeFlags = apNode->GetFlags();
  sRectf r = apNode->GetScrolledTextRect();

  iOverlay* icon = apNode->GetIcon();
  if (icon) {
    const sVec2f iconSize = icon->GetSize();
    c->BlitOverlay(
        sRectf(r.Left(),r.Top()+(r.GetHeight()-iconSize.y)/2,iconSize.x,iconSize.x),
        icon);
    r.Left() += iconSize.x;
  }
  if (niFlagIsNot(nodeFlags,eWidgetTreeNodeFlags_DontDrawName)) {
    tU32 colFront = _GetTextFrontColor(apTree, apNode);
    apNode->_GetFont()->SetColor(colFront);
    r.Top() += 1;
    c->BlitText(
        apNode->_GetFont(),
        r,
        eFontFormatFlags_CenterV,
        apNode->GetName());
  }
}

///////////////////////////////////////////////
static inline void _PushRect(iCanvas* c, tU32 anCol, const sRectf& aRect) {
  c->BlitRect(aRect,anCol);
}

///////////////////////////////////////////////
static inline void _PushLines(iFont* apFont, iCanvas* c, cWidgetTreeNode* apNode, tF32 afX, tF32& afY, tU32 anCol, const sVec2f& avOffset)
{
  sRectf nodeRect = apNode->GetScrolledNodeRect();
  tF32 fH = nodeRect.GetHeight();
  afY += fH;
  if (apNode->GetExpanded() && apNode->GetNumChildNodes())
  {
    for (tU32 i = 0; i < apNode->GetNumChildNodes(); ++i)
    {
      tF32 fPrevY = afY;
      _PushLines(apFont,c,(cWidgetTreeNode*)apNode->GetChildNode(i),afX+16,afY,anCol,avOffset);
      if (i+1 == apNode->GetNumChildNodes())
        c->BlitLine(avOffset+Vec2<tF32>(afX+6,fPrevY),
                    avOffset+Vec2<tF32>(afX+6,fPrevY+(fH/2)+1),anCol);
      else
        c->BlitLine(avOffset+Vec2<tF32>(afX+6,fPrevY),
                    avOffset+Vec2<tF32>(afX+6,afY),anCol);
      c->BlitLine(avOffset+Vec2<tF32>(afX+6,fPrevY+(fH/2)+1),
                  avOffset+Vec2<tF32>(afX+14,fPrevY+(fH/2)+1),anCol);
    }
  }
}

///////////////////////////////////////////////
// X should be the margin where the first attribute will be be drawn, afY should be offset
static inline tU32 _CheckAttrCollision(cWidgetTreeNode* apNode,
                                       const tU32 anNumAttrs,
                                       const sVec2f& avAttrSize,
                                       const tF32 afRight,
                                       const sVec2f& avMousePos)
{
  sRectf nodeRect = apNode->GetScrolledNodeRect();
  // tF32 fH = nodeRect.GetHeight();
  // const tU32 attrs = apNode->mnAttributes;
  const tU32 attrsMask = apNode->mnVisibleAttributesMask;
  niLoop(i,anNumAttrs) {
    if (!(attrsMask&niBit(i)))
      continue; // attribute not visible, skip
    const sRectf attrRect = apNode->_GetAttributeRect(avAttrSize,anNumAttrs-i-1,afRight);
    if (attrRect.Intersect(avMousePos))
      return i;
  }
  return eInvalidHandle;
}

static inline void _PushAttrRect(
    iCanvas* c, cWidgetTreeNode* apNode,
    const tU32 anAttrIndex, const tU32 anNumAttrs,
    const sVec2f& avAttrSize, const tF32 afRight,
    const sVec2f& avMousePos)
{
  const tU32 attrs = apNode->mnAttributes;
  const tU32 attrsMask = apNode->mnVisibleAttributesMask;
  const tU32 attrFlag = niBit(anAttrIndex);
  if (!(attrsMask&attrFlag))
    return; // attribute not visible, skip
  sRectf attrRect = apNode->_GetAttributeRect(avAttrSize,anNumAttrs-anAttrIndex-1,afRight);
  const tU32 color = attrRect.Intersect(avMousePos) ? 0xFFFFFF00 : ~0;
  c->BlitRect(attrRect,color);
  if (attrs&attrFlag) {
    attrRect.Move(Vec2<tF32>(3,3));
    attrRect.Inflate(Vec2<tF32>(-6,-6));
    c->BlitFill(attrRect,color);
  }
}

static inline void _PushAttrRectBg(
    iCanvas* c, cWidgetTree* t, cWidgetTreeNode* apNode,
    const tU32 anAttrIndex, const tU32 anNumAttrs,
    const sVec2f& avAttrSize, const tF32 afRight,
    const sVec2f& avMousePos)
{
  // const tU32 attrs = apNode->mnAttributes;
  const tU32 attrsMask = apNode->mnVisibleAttributesMask;
  const tU32 attrFlag = niBit(anAttrIndex);
  if (!(attrsMask&attrFlag))
    return; // attribute not visible, skip
  sRectf attrRect = apNode->_GetAttributeRectBg(avAttrSize,anNumAttrs-anAttrIndex-1,afRight);
  tU32 color = ~0;
  if (apNode->GetTreeIndex() == (tU32)t->mnHoverTreeNodeIndex) {
    color = t->skin.colRowHover;
  }
  else if (apNode->mnRowColor) {
    color = apNode->mnRowColor;
  }
  else {
    color = (apNode->_GetDrawIndex()&1)?t->skin.colSec:t->skin.colFirst;
  }
  c->BlitFill(attrRect,color);
}

static inline void _PushAttrIcon(
    iCanvas* c, cWidgetTreeNode* apNode,
    const tU32 anAttrIndex, const tU32 anNumAttrs,
    iOverlay* apIcon,
    const sVec2f& avAttrSize, const tF32 afRight,
    const sVec2f& avMousePos)
{
  const tU32 attrs = apNode->mnAttributes;
  const tU32 attrsMask = apNode->mnVisibleAttributesMask;
  const tU32 attrFlag = niBit(anAttrIndex);
  if (!(attrsMask&attrFlag))
    return; // attribute not visible, skip
  sRectf attrRect = apNode->_GetAttributeRect(avAttrSize,anNumAttrs-anAttrIndex-1,afRight);
  const tBool mouseOver = attrRect.Intersect(avMousePos);
  if (mouseOver) {
    attrRect.Move(Vec2<tF32>(1,1));
    attrRect.Inflate(Vec2<tF32>(-2,-2));
  }
  const sColor4f color = (attrs&attrFlag) ?
      Vec4<tF32>(1.0f,1.0f,1.0f,1.0f) :
      Vec4<tF32>(0.5f,0.5f,0.5f,1.0f);
  apIcon->SetColor(color);
  apIcon->SetFiltering(eTrue);
  c->BlitOverlay(attrRect,apIcon);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidgetTree implementation.

///////////////////////////////////////////////
cWidgetTree::cWidgetTree(iWidget* apWidget)
{
  niGuardConstructor(cWidgetTree);

  ZeroMembers();
  mpWidget = apWidget;
  mptrRootNode = niNew cWidgetTreeNode(0,this,NULL,eWidgetTreeNodeFlags_Default|eWidgetTreeNodeFlags_Expanded);
  mptrRootNode->SetName(_A("Root"));
  _CreateWidgetScrollBars();
}

///////////////////////////////////////////////
cWidgetTree::~cWidgetTree()
{
  niGuardConstructor(cWidgetTree);
  mptrHighlightedNode = NULL;
  mptrRootNode = NULL;
  mptrVScroll = NULL;
  mptrHScroll = NULL;
}

///////////////////////////////////////////////
void cWidgetTree::_CreateWidgetScrollBars() {
  if (niFlagIsNot(mpWidget->GetStyle(),eWidgetTreeStyle_NoHScroll)) {
    if (!mptrHScroll.IsOK()) {
      mptrHScroll = mpWidget->GetUIContext()->CreateWidget(_A("ScrollBar"),mpWidget,sRectf(),
                                                         eWidgetStyle_NCRelative|eWidgetScrollBarStyle_Horz);
    }
    if (niFlagIs(mpWidget->GetStyle(),eWidgetTreeStyle_AlwaysHScroll)) {
      mptrHScroll->SetEnabled(eFalse);
      mptrHScroll->SetVisible(eTrue);
    }
  }
  else if (mptrHScroll.IsOK()) {
    mptrHScroll->Destroy();
    mptrHScroll = NULL;
  }
  if (niFlagIsNot(mpWidget->GetStyle(),eWidgetTreeStyle_NoVScroll)) {
    if (!mptrVScroll.IsOK()) {
      mptrVScroll = mpWidget->GetUIContext()->CreateWidget(_A("ScrollBar"),mpWidget,sRectf(),
                                                         eWidgetStyle_NCRelative);
    }
    if (niFlagIs(mpWidget->GetStyle(),eWidgetTreeStyle_AlwaysVScroll)) {
      mptrVScroll->SetEnabled(eFalse);
      mptrVScroll->SetVisible(eTrue);
    }
  }
  else if (mptrVScroll.IsOK()) {
    mptrVScroll->Destroy();
    mptrVScroll = NULL;
  }
}

///////////////////////////////////////////////
//! Zeros all the class members.
void cWidgetTree::ZeroMembers()
{
  mpWidget = NULL;
  mbDragging = eFalse;
  mDropMode = eWidgetTreeNodeDropMode_On;
  // mbProcessLeftClickUp = eFalse;
  mpSelectedLast = NULL;
  mpSecondarySel = NULL;
  mnNotify = 0;
  mvScrollOffset = sVec2f::Zero();
  mpSelectedRangeLast = NULL;
  mnNumVisibleAttributes = 0;
  mnHoverTreeNodeIndex = eInvalidHandle;
}

///////////////////////////////////////////////
//! Sanity check.
ni::tBool __stdcall cWidgetTree::IsOK() const
{
  niClassIsOK(cWidgetTree);
  return mpWidget != NULL;
}

///////////////////////////////////////////////
ni::iWidgetTreeNode * cWidgetTree::GetRootNode() const
{
  return mptrRootNode;
}

///////////////////////////////////////////////
tU32 cWidgetTree::GetNumSelectedNodes() const
{
  return mvSelected.size();
}

///////////////////////////////////////////////
ni::iWidgetTreeNode * cWidgetTree::GetSelectedNode(tU32 anIndex) const
{
  if (anIndex >= mvSelected.size()) return NULL;
  return mvSelected[anIndex];
}

///////////////////////////////////////////////
tBool __stdcall cWidgetTree::OnWidgetSink(iWidget *apWidget, tU32 anMsg, const Var& avarA, const Var& avarB)
{
  if (!mpWidget)
    return eFalse;
  niGuardObject((iWidgetSink*)this);
  switch (anMsg)
  {
    case eUIMessage_Destroy:
      // Make sure all nodes are destoried before we release the mpWidget
      ClearSelection();
      if (mptrRootNode.IsOK()) {
        mptrRootNode->Invalidate();
        mptrRootNode = NULL;
      }

      mpWidget = NULL;
      break;
    case eUIMessage_NCSize:
      _NotifyUpdateLayout(TREENOTIFY_ALL);
      _UpdateLayout();
      break;
    case eUIMessage_FontChanged:
    case eUIMessage_SkinChanged:
      _UpdateSkin();
      return eFalse;
    case eUIMessage_StyleChanged: {
      const tU32 prevStyle = avarA.mU32;
      const tU32 newStyle = mpWidget->GetStyle();
      if (!niFlagEq(prevStyle,newStyle,eWidgetTreeStyle_NoHScroll) ||
          !niFlagEq(prevStyle,newStyle,eWidgetTreeStyle_NoVScroll) ||
          !niFlagEq(prevStyle,newStyle,eWidgetTreeStyle_AlwaysHScroll) ||
          !niFlagEq(prevStyle,newStyle,eWidgetTreeStyle_AlwaysVScroll))
      {
        _CreateWidgetScrollBars();
        _NotifyUpdateLayout(TREENOTIFY_ALL);
      }
      if (!niFlagEq(prevStyle,newStyle,eWidgetTreeStyle_DontDrawRoot)) {
        _UpdateScrollOffset();
      }
      break;
    }
    case eUIMessage_Paint: {
      iCanvas* c = VarQueryInterface<iCanvas>(avarB);
      if (c) {
        _Paint(avarA.GetVec2f(),c);
      }
      break;
    }
    case eUIMessage_KeyDown:
      {
        const tU32 key = avarA.mU32;
        switch (key) {
          case eKey_Left:
            if (!mvSelected.empty()) {
              iWidgetTreeNode* pNode = mvSelected.back();
              pNode->SetExpanded(eFalse);
            }
            break;
          case eKey_Right:
            if (!mvSelected.empty()) {
              iWidgetTreeNode* pNode = mvSelected.back();
              pNode->SetExpanded(eTrue);
            }
            break;
          case eKey_Home:
            {
              _DoSelect(_GetHomeNode(),apWidget->GetUIContext()->GetInputModifiers(),eTrue);
              break;
            }
          case eKey_End:
            {
              _DoSelect(_GetEndNode(),apWidget->GetUIContext()->GetInputModifiers(),eTrue);
              break;
            }
          case eKey_Up:
            if (!mvSelected.empty()) {
              iWidgetTreeNode* pNode = mvSelected.back();
              if (pNode && pNode->GetAbove() &&
                  (niFlagIsNot(mpWidget->GetStyle(),eWidgetTreeStyle_DontDrawRoot) ||
                   pNode->GetAbove() != mptrRootNode))
              {
                _DoSelect(pNode->GetAbove(),apWidget->GetUIContext()->GetInputModifiers(),eTrue);
              }
            }
            else {
              _DoSelect(_GetEndNode(),apWidget->GetUIContext()->GetInputModifiers(),eTrue);
            }
            break;
          case eKey_Down:
            if (!mvSelected.empty()) {
              iWidgetTreeNode* pNode = mvSelected.back();
              _DoSelect(pNode->GetBelow(),apWidget->GetUIContext()->GetInputModifiers(),eTrue);
            }
            else {
              _DoSelect(_GetHomeNode(),apWidget->GetUIContext()->GetInputModifiers(),eTrue);
            }
            break;
          case eKey_Enter:
          case eKey_NumPadEnter:
          case eKey_Space:
            if (!mvSelected.empty()) {
              iWidgetTreeNode* pNode = mvSelected.back();
              pNode->SetExpanded(pNode->GetExpanded()?eFalse:eTrue);
            }
            break;
        }
        break;
      }
    case eUIMessage_RightClickDown:
      SetSecondarySelection(NULL);
      _ProcessClick(anMsg, *((sVec2f*)avarA.mV2F),
                    eFalse, apWidget->GetUIContext()->GetInputModifiers());
      break;
    case eUIMessage_LeftClickDown:
      apWidget->SetCapture(eTrue);
      SetSecondarySelection(NULL);
      _ProcessClick(anMsg, *((sVec2f*)avarA.mV2F),
                    eFalse, apWidget->GetUIContext()->GetInputModifiers());
      break;
    case eUIMessage_LeftClickUp:
      if (mpSecondarySel && (mpSecondarySel->GetTreeIndex() == (tU32)mnHoverTreeNodeIndex)) {
        _DoSelect(mpSecondarySel,apWidget->GetUIContext()->GetInputModifiers(),eTrue);
      }
    case eUIMessage_NCLeftClickUp:
      SetSecondarySelection(NULL);
      apWidget->SetCapture(eFalse);
      break;
    case eUIMessage_LeftDoubleClick:
      _ProcessClick(anMsg, *((sVec2f*)avarA.mV2F),eTrue,apWidget->GetUIContext()->GetInputModifiers());
      break;
    case eUIMessage_MouseLeave: {
      mnHoverTreeNodeIndex = eInvalidHandle;
      break;
    }
    case eUIMessage_MouseMove: {
      iWidgetTreeNode* n = _FindNodeByPos(*((sVec2f*)avarA.mV2F));
      if (n) {
        mnHoverTreeNodeIndex = n->GetTreeIndex();
      }
      else {
        mnHoverTreeNodeIndex = eInvalidHandle;
      }
      break;
    }
    case eUIMessage_Timer: {
      break;
    }
    case eUIMessage_Command:
      {
        Ptr<iWidgetCommand> cmd = ni::VarQueryInterface<iWidgetCommand>(avarA);
        if (cmd->GetSender() == mptrHScroll || cmd->GetSender() == mptrVScroll) {
          _UpdateScrollOffset();
          _NotifyUpdateLayout(TREENOTIFY_CULLING);
        }
        else {
          if (mpWidget->GetParent()) {
            mpWidget->GetParent()->SendMessage(anMsg,avarA,avarB);
          }
        }
        break;
      }
    case eUIMessage_DragMouseEnter:
      mbDragging = eTrue;
      break;
    case eUIMessage_DragMouseLeave:
      mbDragging = eFalse;
      mptrHighlightedNode = NULL;
      break;
    case eUIMessage_DragMouseMove:
      if (mbDragging) {
        sVec2f mousePos = avarA.GetVec2f();
        mptrHighlightedNode = _FindNodeByPos(mousePos);
        if (mptrHighlightedNode.IsOK()) {
          mDropMode = GetNodeDropMode(
              mptrHighlightedNode,mousePos+mpWidget->GetAbsolutePosition()+mpWidget->GetClientPosition());
        }
      }
      break;
    case eUIMessage_Copy:
      {
        Ptr<iDataTable> dt = ni::VarQueryInterface<ni::iDataTable>(avarA);
        if (dt.IsOK()) {
          cString selName;
          iWidgetTreeNode* node = mpSecondarySel ? mpSecondarySel : mvSelected.size() > 0 ? mvSelected[0] : NULL;
          if (node) {
            selName = node->GetName();
          }
          dt->SetString(_A("text"),selName.Chars());
        }
        break;
      }
    case eUIMessage_Cut:
      break;
    case eUIMessage_Paste:
      break;
    default:
      return eFalse;
  }
  return eTrue;
}

///////////////////////////////////////////////
void cWidgetTree::_AddSelectedNode(cWidgetTreeNode* apNode)
{
  astl::push_back_once(mvSelected,apNode);
  if (mpWidget)
    mpWidget->Redraw();
}

///////////////////////////////////////////////
void cWidgetTree::_RemoveSelectedNode(cWidgetTreeNode* apNode)
{
  astl::find_erase(mvSelected,apNode);
  if (mpWidget)
    mpWidget->Redraw();
}

///////////////////////////////////////////////
void cWidgetTree::_Paint(const sVec2f& avMousePos, iCanvas* c)
{
  _UpdateLayout();

  iFont* pFont = mpWidget->GetFont();
  // sVec2f vOffset = mvScrollOffset;
  sVec2f clSize = mpWidget->GetClientSize();

  // Draw backgrounds
  niLoopit(tTreeNodePLst::const_iterator,it,mlstVisibleNodes) {
    cWidgetTreeNode* n = (*it);
    sRectf rect = n->GetScrolledNodeRect();
    rect.Left() = 0;
    rect.Right() = clSize.x;
    if (n->GetTreeIndex() == (tU32)mnHoverTreeNodeIndex) {
      c->BlitFill(rect,skin.colRowHover);
    }
    else if (n->mnRowColor) {
      c->BlitFill(rect,n->mnRowColor);
    }
    else {
      c->BlitFill(rect,
                  (n->GetSelected() || (n->_GetDrawIndex()&1)) ?
                  skin.colSec :
                  skin.colFirst);
    }
  }

  // Draw selections
  niLoopit(tTreeNodePLst::const_iterator,it,mlstVisibleNodes) {
    cWidgetTreeNode* n = (*it);
    if (n->GetSelected() || (n == mpSecondarySel)) {
      sRectf selRect = n->GetScrolledNodeRect();
      selRect.Left() = 0;
      selRect.Right() = clSize.x;
      selRect.Bottom() += 1;
      if (n->GetSelected() || !(n == mpSecondarySel)) {
        c->BlitFillAlpha(selRect,
                         (this->mpSelectedLast == n) ?
                         skin.colLastSelBack :
                         skin.colSelBack);
      }
      c->BlitRect(selRect,skin.colSelBorder);
    }
  }

  {
    niLoopit(tTreeNodePLst::const_iterator,it,mlstVisibleNodes) {
      sRectf rect = (*it)->GetScrolledNodeRect();
      sRectf frameRect = rect;
      frameRect.Left() = 0;
      frameRect.Right() = clSize.x;
      c->BlitOverlayFrame(
          frameRect,
          skin.nodeFrame,
          eRectFrameFlags_Edges);
      if ((*it)->GetNumChildNodes() || niFlagIs((*it)->GetFlags(),eWidgetTreeNodeFlags_Expandable)) {
        if ((*it)->GetExpanded()) {
          sVec2f size = skin.expanded->GetSize();
          c->BlitOverlay(
              sRectf(rect.Left() - size.x, rect.Top() + (rect.GetHeight()/2-size.y/2),
                          size.x,size.y),
              skin.expanded);
        }
        else {
          sVec2f size = skin.collapsed->GetSize();
          c->BlitOverlay(
              sRectf(rect.Left() - size.x, rect.Top() + (rect.GetHeight()/2-size.y/2),
                          size.x,size.y),
              skin.collapsed);
        }
      }
    }
  }

  // Draw the text
  {
    niLoopit(tTreeNodePLst::const_iterator,it,mlstVisibleNodes) {
      _PushChars(this,c,pFont,*it,_kfLeftMargin);
    }
  }

  // Draw the attributes
  niLoop(i,mnNumVisibleAttributes) {
    // draw icons
    if (mAttributes[i].icon.IsOK()) {
      niLoopit(tTreeNodePLst::const_iterator,it,mlstVisibleNodes) {
        _PushAttrIcon(c,*it,i,mnNumVisibleAttributes,
                      mAttributes[i].icon,
                      _kvAttrSize,clSize.x,avMousePos);
      }
    }
    else
    {
      niLoopit(tTreeNodePLst::const_iterator,it,mlstVisibleNodes) {
        _PushAttrRect(c,*it,i,mnNumVisibleAttributes,
                      _kvAttrSize,clSize.x,avMousePos);
      }
    }
  }

  // Push the drag'n drop overlays
  if (mptrHighlightedNode.IsOK()) {
    sRectf parentRect = (mptrHighlightedNode->GetParentNode() ?
                              mptrHighlightedNode->GetParentNode()->GetScrolledNodeRect() :
                              sRectf::Null());
    parentRect.Right() = mpWidget->GetClientSize().x;
    sRectf rect = mptrHighlightedNode->GetScrolledNodeRect();
    // rect.Left() = 0;
    rect.Right() = mpWidget->GetClientSize().x;
    if (mDropMode == eWidgetTreeNodeDropMode_Above) {
      c->BlitRect(parentRect,skin.colDropBorder);
      rect.Left() = parentRect.Left();
      c->BlitFill(sRectf(rect.GetTopLeft()+Vec2f(0,-1),rect.GetTopRight()+Vec2f(0,1)),skin.colDropBorder);
    }
    else if (mDropMode == eWidgetTreeNodeDropMode_Below) {
      c->BlitRect(parentRect,skin.colDropBorder);
      rect.Left() = parentRect.Left();
      c->BlitFill(sRectf(rect.GetBottomLeft()+Vec2f(0,1),rect.GetBottomRight()+Vec2f(0,-1)),skin.colDropBorder);
    }
    else /*if (mDropMode == eWidgetTreeNodeDropMode_On)*/ {
      c->BlitRect(rect,skin.colDropBorder);
      c->BlitRect((sVec4f&)rect+Vec4f(-1,-1,1,1),skin.colDropBorder);
    }
  }
}

///////////////////////////////////////////////
tBool cWidgetTree::ClearSelection()
{
  tTreeNodePVec toUnselect = mvSelected;
  niLoop(i,toUnselect.size()) {
    toUnselect[i]->SetSelected(eFalse);
  }
  mvSelected.clear();
  return eTrue;
}

///////////////////////////////////////////////
iWidgetTreeNode* __stdcall cWidgetTree::GetNodeFromPosition(const sVec2f& avAbsPos) const
{
  niCheckSilent(niIsOK(mpWidget),NULL);
  sVec2f clientPos = avAbsPos-mpWidget->GetAbsolutePosition()-mpWidget->GetClientPosition();
  return _FindNodeByPos(clientPos);
}

///////////////////////////////////////////////
eWidgetTreeNodeDropMode __stdcall cWidgetTree::GetNodeDropMode(iWidgetTreeNode* apNode, const sVec2f& avAbsPos) const
{
  niCheckIsOK(mpWidget,eWidgetTreeNodeDropMode_Invalid);
  niCheckIsOK(apNode,eWidgetTreeNodeDropMode_Invalid);
  sVec2f clientPos = avAbsPos-mpWidget->GetAbsolutePosition()-mpWidget->GetClientPosition();
  eWidgetTreeNodeDropMode dropMode = eWidgetTreeNodeDropMode_On;
  sRectf rect = apNode->GetScrolledNodeRect();
  if (apNode->GetParentNode()) {
    if (ni::Abs(rect.Bottom()-clientPos.y) <= _knDropBorderDistance) {
      dropMode = eWidgetTreeNodeDropMode_Below;
    }
    else if (ni::Abs(rect.Top()-clientPos.y) <= _knDropBorderDistance) {
      dropMode = eWidgetTreeNodeDropMode_Above;
    }
  }
  return dropMode;
}

///////////////////////////////////////////////
void cWidgetTree::_ProcessClick(tU32 anMsg, const sVec2f& avMousePos, tBool abExecute, tU32 aModifier)
{
  cWidgetTreeNode* pNode = (cWidgetTreeNode*)_FindNodeByPos(avMousePos);
  if (pNode) {
    if (mnNumVisibleAttributes) {
      const tU32 attrCollided = _CheckAttrCollision(pNode,mnNumVisibleAttributes,
                                                    _kvAttrSize,mpWidget->GetClientSize().x,
                                                    avMousePos);
      if (attrCollided != eInvalidHandle) {
        if (anMsg == eUIMessage_LeftClickDown) {
          const tU32 attrFlag = niBit(attrCollided);
          tU32 attr = pNode->GetAttributes();
          if (attr&attrFlag) {
            niFlagOff(attr,attrFlag);
          }
          else {
            niFlagOn(attr,attrFlag);
          }
          pNode->SetAttributes(attr);
        }
        else if (anMsg == eUIMessage_RightClickDown) {
          _DoSelect(pNode,aModifier,eTrue);
        }
        return;
      }
    }

    sRectf nodeRect = pNode->GetScrolledNodeRect();
    nodeRect.Move(Vec2<tF32>(-16,0));
    nodeRect.SetWidth(16);
    if (// has children or can be expanded
            (pNode->GetNumChildNodes() ||
             niFlagIs(pNode->GetFlags(),eWidgetTreeNodeFlags_Expandable)) &&
            // clicked on the expansion arrow or double clicked without modifier
            (nodeRect.Intersect(avMousePos) ||
             ((anMsg == eUIMessage_LeftDoubleClick) && !aModifier)) &&
            // left click or double click
            ((anMsg == eUIMessage_LeftDoubleClick) ||
             (anMsg == eUIMessage_LeftClickDown))
        )
    {
      pNode->SetExpanded(pNode->GetExpanded()?eFalse:eTrue);
    }
    else if (anMsg == eUIMessage_RightClickDown) {
      if (!pNode->GetSelected()) {
        _DoSelect(pNode,aModifier,eTrue);
      }
    }
    else if (niFlagIs(mpWidget->GetStyle(),eWidgetTreeStyle_ClickDownSelect)) {
      _DoSelect(pNode,aModifier,eTrue);
    }
    else {
      if (anMsg == eUIMessage_LeftClickDown) {
        SetSecondarySelection(pNode);
      }
      if ((anMsg == eUIMessage_LeftClickUp) ||
          niFlagIs(aModifier,eUIInputModifier_AddSelectionRange))
      {
        tBool bProcessed = _DoSelect(pNode,aModifier,anMsg==eUIMessage_LeftClickUp);
        if (bProcessed && abExecute) {
          pNode->SetExpanded(pNode->GetExpanded()?eFalse:eTrue);
        }
      }
    }
  }
  else {
    ClearSelection();
  }
}

///////////////////////////////////////////////
ni::iWidgetTreeNode * cWidgetTree::_FindNodeByPos(const sVec2f& avMousePos) const {
  niThis(cWidgetTree)->_UpdateLayout();
  niLoopit(tTreeNodePLst::const_iterator,it,mlstVisibleNodes) {
    sRectf nodeRect = (*it)->GetNodeRect();
    if (avMousePos.y >= nodeRect.Top()+mvScrollOffset.y && avMousePos.y < nodeRect.Bottom()+mvScrollOffset.y) {
      return *it;
    }
  }
  return NULL;
}

///////////////////////////////////////////////
ni::iWidgetTreeNode* __stdcall cWidgetTree::_GetHomeNode() const
{
  if (niFlagIs(mpWidget->GetStyle(),eWidgetTreeStyle_DontDrawRoot)) {
    return mptrRootNode.IsOK() ? mptrRootNode->GetBelow() : NULL;
  }
  else {
    return mptrRootNode;
  }
}

///////////////////////////////////////////////
ni::iWidgetTreeNode* __stdcall cWidgetTree::_GetEndNode() const
{
  ni::iWidgetTreeNode* pLast = _GetHomeNode();
  while (pLast) {
    ni::iWidgetTreeNode* pBelow = pLast->GetBelow();
    if (!pBelow)
      return pLast;
    pLast = pBelow;
  }
  return NULL;
}

///////////////////////////////////////////////
void cWidgetTree::_UpdateScrollOffset()
{
  mvScrollOffset = Vec2<tF32>(0,0);
  if (mptrHScroll.IsOK() && mptrHScroll->GetVisible()) {
    mvScrollOffset.x = -QPtr<iWidgetScrollBar>(mptrHScroll)->GetScrollPosition()*SCROLL_SCALEX;
  }
  if (mptrVScroll.IsOK() && mptrVScroll->GetVisible()) {
    mvScrollOffset.y = -QPtr<iWidgetScrollBar>(mptrVScroll)->GetScrollPosition()*SCROLL_SCALEY;
  }
}

///////////////////////////////////////////////
tBool __stdcall cWidgetTree::Clear()
{
  ClearSelection();
  if (mptrRootNode.IsOK()) {
    mptrRootNode->Invalidate();
    mptrRootNode = NULL;
  }
  if (!mpWidget)
    return eFalse;
  mptrRootNode = niNew cWidgetTreeNode(0,this,NULL);
  mptrRootNode->SetName(_A("Root"));
  mptrRootNode->SetExpanded(eTrue);
  return eTrue;
}

tBool cWidgetTree::_DoSelect(ni::iWidgetTreeNode* apNode, tU32 aModifier, tBool abForceSetSelect)
{
  if (!apNode)
    return eFalse;

  const tBool bSingleSelection = niFlagIs(mpWidget->GetStyle(),eWidgetTreeStyle_SingleSelection);

  // Range selection
  if ((!bSingleSelection) && niFlagIs(aModifier,eUIInputModifier_AddSelectionRange) && mpSelectedLast != NULL) {
    tBool bNewSelect = eTrue;
    cWidgetTreeNode* pPivot = mpSelectedLast;
    cWidgetTreeNode* pSel = (cWidgetTreeNode*)apNode;

    sRectf pivotRect = pPivot->GetNodeRect();
    sRectf selRect = pSel->GetNodeRect();
    tBool bSelUp = (pivotRect.y > selRect.y);

    if (mpSelectedRangeLast) {
      sRectf prevRect = mpSelectedRangeLast->GetNodeRect();
      // if the selection direction is the same we can optimize,
      // otherwise we'll just reselect everything since only the
      // pivot item will be common
      tBool bPrevUp = (pivotRect.y > prevRect.y);
      if (bSelUp == bPrevUp) {
        // if the previous and current selections are the same
        // we dont have anything to do selection direction is
        // the same
        if (mpSelectedRangeLast != pSel) {
          if (bSelUp) {
            // selecting upward
            if (prevRect.y < selRect.y) {
              // if the previous selection is above the
              // new selection remove select items
              // downward until we meet the new
              // selection
              ni::iWidgetTreeNode* pCur = mpSelectedRangeLast;
              while (pCur) {
                if (pCur == pSel)
                  break;
                pCur->SetSelected(eFalse);
                pCur = pCur->GetBelow();
              }
            }
            else {
              // if the previous selection is below the
              // new selection, add the items upward
              // until we meet the new selection
              ni::iWidgetTreeNode* pCur = mpSelectedRangeLast->GetAbove();
              while (pCur) {
                pCur->SetSelected(eTrue);
                if (pCur == pSel)
                  break;
                pCur = pCur->GetAbove();
              }
            }
          }
          else {
            // selecting downward
            if (prevRect.y < selRect.y) {
              // if the previous selection is above the
              // new selection add all items until we
              // meet the new sel
              ni::iWidgetTreeNode* pCur = mpSelectedRangeLast->GetBelow();
              while (pCur) {
                pCur->SetSelected(eTrue);
                if (pCur == pSel)
                  break;
                pCur = pCur->GetBelow();
              }
            }
            else {
              // if the previous selection is above the
              // new selection remove all items until we
              // meet the new sel
              ni::iWidgetTreeNode* pCur = mpSelectedRangeLast;
              while (pCur) {
                if (pCur == pSel)
                  break;
                pCur->SetSelected(eFalse);
                pCur = pCur->GetAbove();
              }
            }
          }
        }
        bNewSelect = eFalse;
        mpSelectedRangeLast = pSel;
      }
    }

    if (bNewSelect) {

      mpSelectedRangeLast = pSel;
      if (bSelUp) {
        ni::Swap(pPivot,pSel);
      }

      // select the range
      ClearSelection();
      ni::iWidgetTreeNode* pCur = pPivot;
      while (pCur) {
        pCur->SetSelected(eTrue);
        if (pCur == pSel)
          break;
        pCur = pCur->GetBelow();
      }
    }

    // make sure the new selection is at the back of the selection
    if (!mvSelected.empty() && mvSelected.back() != mpSelectedRangeLast) {
      tTreeNodePVec::iterator it = astl::find(mvSelected,mpSelectedRangeLast);
      if (it != mvSelected.end()) {
        ni::Swap(mvSelected[astl::iterator_index(mvSelected,it)],mvSelected.back());
      }
    }
    ScrollToNode(mpSelectedRangeLast);
  }
  else {
    mpSelectedRangeLast = NULL;
    if ((!bSingleSelection) && aModifier == eUIInputModifier_AddSelection) {
      apNode->SetSelected(apNode->GetSelected()?eFalse:eTrue);
    }
    else {
      if (mvSelected.size() == 1 && mvSelected[0] == apNode) {
        // do nothing, already selected...
      }
      else {
        if (mvSelected.size() == 1) {
          // one selection, changing just proceed
          mvSelected[0]->SetSelected(eFalse);
          apNode->SetSelected(eTrue);
        }
        else {
          tBool bSelect = eTrue;
          tTreeNodePLst toUnselect;
          niLoop(i,mvSelected.size()) {
            if (mvSelected[i] == apNode) {
              bSelect = eFalse;
              if (!abForceSetSelect) break;
            }
            else {
              toUnselect.push_back(mvSelected[i]);
            }
          }
          if (bSelect || abForceSetSelect) {
            niLoopit(tTreeNodePLst::iterator,it,toUnselect) {
              (*it)->SetSelected(eFalse);
            }
            if (bSelect)
              apNode->SetSelected(eTrue);
          }
        }
      }
    }

    mpSelectedLast = niUnsafeCast(cWidgetTreeNode*,apNode);
    // make sure the new selection is at the back of the selection
    if (!mvSelected.empty() && mvSelected.back() != mpSelectedLast) {
      tTreeNodePVec::iterator it = astl::find(mvSelected,mpSelectedLast);
      if (it != mvSelected.end()) {
        ni::Swap(mvSelected[astl::iterator_index(mvSelected,it)],mvSelected.back());
      }
    }
    ScrollToNode(mpSelectedLast);
  }
  return eTrue;
}

///////////////////////////////////////////////
void __stdcall cWidgetTree::SetSecondarySelection(iWidgetTreeNode* apNode)
{
  if (!mpWidget)
    return;
  if (apNode != mpSecondarySel) {
    mpWidget->SendCommand(mpWidget->GetParent(),eWidgetTreeCmd_SecondaryUnselected);
  }
  mpSecondarySel = niGetIfOK(niUnsafeCast(cWidgetTreeNode*,apNode));
  if (mpSecondarySel) {
    mpWidget->SendCommand(mpWidget->GetParent(),eWidgetTreeCmd_SecondarySelected);
  }
}

///////////////////////////////////////////////
iWidgetTreeNode* __stdcall cWidgetTree::GetSecondarySelection() const
{
  return mpSecondarySel;
}

///////////////////////////////////////////////
iWidgetTreeNode* __stdcall cWidgetTree::GetPivotSelection() const {
  return mpSelectedLast;
}

///////////////////////////////////////////////
void cWidgetTree::_RegisterNode(cWidgetTreeNode* apNode)
{
  mvNodes.push_back(apNode);
  apNode->_SetTreeIndex(mvNodes.size()-1);
  _NotifyUpdateLayout(TREENOTIFY_NODES);
  if (mpWidget)
    mpWidget->Redraw();
}

///////////////////////////////////////////////
void cWidgetTree::_UnregisterNode(cWidgetTreeNode* apNode)
{
  _RemoveSelectedNode(apNode);
  if (apNode == mpSelectedLast) {
    mpSelectedLast = NULL;
  }
  if (apNode == mpSelectedRangeLast) {
    mpSelectedRangeLast = NULL;
  }
  if (apNode == mpSecondarySel) {
    SetSecondarySelection(NULL);
  }
  tTreeNodePVec::iterator it = astl::find(mvNodes,apNode);
  if (it != mvNodes.end()) {
    tU32 i = astl::iterator_index(mvNodes,it);
    mvNodes.erase(mvNodes.begin()+i);
    astl::find_erase(mlstVisibleNodes,apNode);
    for (tTreeNodePVec::iterator it = mvNodes.begin()+i; it != mvNodes.end(); ++it) {
      (*it)->_SetTreeIndex(astl::iterator_index(mvNodes,it));
    }
    _NotifyUpdateLayout(TREENOTIFY_NODES);
  }
  if (mpWidget)
    mpWidget->Redraw();
}

///////////////////////////////////////////////
void cWidgetTree::_NotifyUpdateLayout(tU32 anDirty)
{
  mnNotify |= anDirty;
}

///////////////////////////////////////////////
void cWidgetTree::_UpdateLayout()
{
  if (mnNotify) {
    _DoUpdateLayout(mnNotify);
    niFlagOff(mnNotify,TREENOTIFY_ALL);
  }
}

///////////////////////////////////////////////
void cWidgetTree::_DoUpdateLayout(tU32 anFlags)
{
  if (mpWidget)
    mpWidget->Redraw();

  tU32 nHScroll = 0;
  tU32 nVScroll = 0;
  niLoop(i,10) {
    _UpdateScrollOffset();

    tF32 fMaxX = 0.0f;
    tF32 fMaxY = 0.0f;
    if (niFlagIs(anFlags,TREENOTIFY_LAYOUT) || niFlagIs(anFlags,TREENOTIFY_SCROLLBARS)) {
      _UpdateNodeLayout((cWidgetTreeNode*)mptrRootNode.ptr(),fMaxX,fMaxY);
    }

    // update scrollbars
    if (niFlagIs(anFlags,TREENOTIFY_SCROLLBARS) && (mptrVScroll.IsOK() || mptrHScroll.IsOK()))
    {
      tBool bWasHScroll = (mptrHScroll.IsOK() && mptrHScroll->GetVisible());
      tBool bWasVScroll = (mptrVScroll.IsOK() && mptrVScroll->GetVisible());
      tBool bHScroll = !!nHScroll, bVScroll = !!nVScroll;
      tF32 fSizeX = mpWidget->GetSize().x,
          fSizeY = mpWidget->GetSize().y;

      if (mptrHScroll.IsOK() &&
          (fMaxX > fSizeX
           || bHScroll
           || niFlagIs(mpWidget->GetStyle(),eWidgetTreeStyle_AlwaysHScroll)))
      { // we have to show the horizontal scroll bar
        bHScroll = eTrue;
        fSizeY -= kfScrollBarSize;
        if (mptrVScroll.IsOK() &&
            (fMaxY > fSizeY-kfScrollBarSize
             || bVScroll
             || niFlagIs(mpWidget->GetStyle(),eWidgetTreeStyle_AlwaysVScroll)))
        { // with the hz do we need the vertical one ?
          bVScroll = eTrue;
          fSizeX -= kfScrollBarSize;
        }
      }
      else if (mptrVScroll.IsOK() &&
               (fMaxY > fSizeY
                || bVScroll
                || niFlagIs(mpWidget->GetStyle(),eWidgetTreeStyle_AlwaysVScroll)))
      { // we have to show the vertical scroll bar
        bVScroll = eTrue;
        fSizeX -= kfScrollBarSize;
        if (mptrHScroll.IsOK() &&
            (fMaxX > fSizeX-kfScrollBarSize
             || bHScroll
             || niFlagIs(mpWidget->GetStyle(),eWidgetTreeStyle_AlwaysHScroll)))
        { // with the vert do we need the horizontal one ?
          bHScroll = eTrue;
          fSizeY -= kfScrollBarSize;
        }
      }

      sRectf rectV, rectH;
      if (bHScroll && bVScroll) {
        rectH = sRectf(0,mpWidget->GetSize().y-kfScrollBarSize,mpWidget->GetSize().x-kfScrollBarSize,kfScrollBarSize);
        rectV = sRectf(mpWidget->GetSize().x-kfScrollBarSize,0,kfScrollBarSize,mpWidget->GetSize().y-kfScrollBarSize);
      }
      else if (bHScroll) {
        rectH = sRectf(0,mpWidget->GetSize().y-kfScrollBarSize,mpWidget->GetSize().x,kfScrollBarSize);
      }
      else if (bVScroll) {
        rectV = sRectf(mpWidget->GetSize().x-kfScrollBarSize,0,kfScrollBarSize,mpWidget->GetSize().y);
      }

      if (bHScroll) {
        ++nHScroll;
        mptrHScroll->SetRect(rectH);
        mptrHScroll->SetVisible(eTrue);
        mptrHScroll->SetEnabled(eTrue);
        if (fMaxX < fSizeX) {
          QPtr<iWidgetScrollBar>(mptrHScroll)->SetScrollRange(sVec2f::Zero());
        }
        else {
          QPtr<iWidgetScrollBar>(mptrHScroll)->SetScrollRange(
              Vec2<tF32>(0,((fMaxX-fSizeX)+SCROLL_SCALEX)/SCROLL_SCALEX));
        }
      }
      else if (mptrHScroll.IsOK()) {
        mptrHScroll->SetVisible(eFalse);
        mptrHScroll->SetEnabled(eFalse);
      }

      if (bVScroll) {
        ++nVScroll;
        mptrVScroll->SetRect(rectV);
        mptrVScroll->SetVisible(eTrue);
        mptrVScroll->SetEnabled(eTrue);
        if (fMaxY < fSizeY) {
          QPtr<iWidgetScrollBar>(mptrVScroll)->SetScrollRange(sVec2f::Zero());
        }
        else {
          QPtr<iWidgetScrollBar>(mptrVScroll)->SetScrollRange(
              Vec2<tF32>(0,(fMaxY-fSizeY)/SCROLL_SCALEY));
        }
      }
      else if (mptrVScroll.IsOK()) {
        mptrVScroll->SetVisible(eFalse);
        mptrVScroll->SetEnabled(eFalse);
      }

      mpWidget->SetClientSize(Vec2<tF32>(fSizeX,fSizeY));
      _UpdateScrollOffset();

      if (bWasHScroll != bHScroll || bWasVScroll != bVScroll) {
        // we need to re-update the layout
      }
      else {
        break; // done
      }
    }
    else {
      break; // done
    }
  }

  // culling
  if (niFlagIs(anFlags,TREENOTIFY_CULLING) || niFlagIs(anFlags,TREENOTIFY_SCROLLBARS) || niFlagIs(anFlags,TREENOTIFY_LAYOUT)) {
    //niPrintln(_A("-- Node Culling --\n"));

    sVec2f vScrollOffset = mvScrollOffset;
    // get the visibility 'window'
    tF32 fViewMin = -vScrollOffset.y;
    tF32 fViewMax = fViewMin+mpWidget->GetClientSize().y;

    astl::set<cWidgetTreeNode*> setToUpdateStatus;

    // reset the visibility list
    niLoopit(tTreeNodePLst::const_iterator,it,mlstVisibleNodes) {
      (*it)->_SetCulling(eTrue);
      setToUpdateStatus.insert(*it);
    }
    mlstVisibleNodes.clear();

    // detect the overlapping pairs
    struct _Local {
      static void DetectOverlap(cWidgetTreeNode* n, tF32 fViewMin, tF32 fViewMax, tTreeNodePLst& visList, astl::set<cWidgetTreeNode*>& toUpdate) {
        const sRectf& r = n->GetNodeRect();
        if (!(r.Bottom() < fViewMin || r.Top() > fViewMax)) {
          visList.push_back(n);
          toUpdate.insert(n);
        }
        if (n->GetNumChildNodes() && (n->GetFlags()&eWidgetTreeNodeFlags_Expanded)) {
          niLoop(i,n->GetNumChildNodes()) {
            DetectOverlap(niStaticCast(cWidgetTreeNode*,n->GetChildNode(i)),fViewMin,fViewMax,visList,toUpdate);
          }
        }
      }
    };
    if (niFlagIs(mpWidget->GetStyle(),eWidgetTreeStyle_DontDrawRoot)) {
      iWidgetTreeNode* n = mptrRootNode;
      if (n->GetNumChildNodes() && (n->GetFlags()&eWidgetTreeNodeFlags_Expanded)) {
        niLoop(i,n->GetNumChildNodes()) {
          _Local::DetectOverlap(niStaticCast(cWidgetTreeNode*,n->GetChildNode(i)),
                                fViewMin,fViewMax,mlstVisibleNodes,setToUpdateStatus);
        }
      }
    }
    else {
      _Local::DetectOverlap(niStaticCast(cWidgetTreeNode*,mptrRootNode.ptr()),fViewMin,fViewMax,mlstVisibleNodes,setToUpdateStatus);
    }

    // set the culling state
    niLoopit(tTreeNodePLst::const_iterator,it,mlstVisibleNodes) {
      (*it)->_SetCulling(eFalse);
      (*it)->_UpdateDrawIndex();
      //niPrintln(niFmt(_A("Node '%s' (%s) is visible\n"),(*it)->GetName(),cString((*it)->GetNodeRect(),_A("Rect")).Chars()));
    }

    // set the set
    niLoopit(astl::set<cWidgetTreeNode*>::iterator,it,setToUpdateStatus) {
      (*it)->_UpdateWidgetPosition();
    }
  }
}

///////////////////////////////////////////////
tU32 __stdcall cWidgetTree::GetNumNodes() const
{
  return mvNodes.size();
}

///////////////////////////////////////////////
iWidgetTreeNode* __stdcall cWidgetTree::GetNode(tU32 anIndex) const
{
  niCheckSilent(anIndex < mvNodes.size(),NULL);
  return mvNodes[anIndex];
}

///////////////////////////////////////////////
void __stdcall cWidgetTree::ScrollToNode(iWidgetTreeNode* apNode) {
  if (!mpWidget) return;
  if (!niIsOK(apNode)) return;
  if (!mptrVScroll.IsOK()) return;
  cWidgetTreeNode* n = niUnsafeCast(cWidgetTreeNode*,apNode);

  sRectf rect = n->GetNodeRect();
  if (rect == _rectInvalid)
    return; // collapsed...

  sVec2f vScrollOffset = mvScrollOffset;
  tF32 fViewMin = -vScrollOffset.y;
  tF32 fViewSize = mpWidget->GetClientSize().y;
  tF32 fViewMax = fViewMin+fViewSize;
  if (rect.Top() < fViewMin) {
    QPtr<iWidgetScrollBar>(mptrVScroll)->SetScrollPosition(rect.Top()/SCROLL_SCALEY);
  }
  else if (rect.Bottom() > fViewMax) {
    QPtr<iWidgetScrollBar>(mptrVScroll)->SetScrollPosition((rect.Bottom()-fViewSize)/SCROLL_SCALEY);
  }
  else {
    // nothing, entierly visible
  }
}

///////////////////////////////////////////////
void cWidgetTree::_UpdateSkin()
{
  skin.nodeFrame = mpWidget->FindSkinElement(NULL,NULL,_H("NodeFrame"));
  skin.collapsed = mpWidget->FindSkinElement(NULL,NULL,_H("Collapsed"));
  skin.expanded = mpWidget->FindSkinElement(NULL,NULL,_H("Expanded"));

  const sColor4f color = sColor4f::White();
  skin.colFirst = ULColorBuild(mpWidget->FindSkinColor(color,NULL,NULL,_H("FirstRow")));
  skin.colSec = ULColorBuild(mpWidget->FindSkinColor(color,NULL,NULL,_H("SecondRow")));
  skin.colRowHover = ULColorBuild(mpWidget->FindSkinColor(color,NULL,NULL,_H("RowHover")));
  skin.colTextFront = ULColorBuild(mpWidget->FindSkinColor(color,NULL,NULL,_H("TextFront")));
  skin.colTextBack = ULColorBuild(mpWidget->FindSkinColor(color,NULL,NULL,_H("TextBack")));
  skin.colSignBack = ULColorBuild(mpWidget->FindSkinColor(color,NULL,NULL,_H("SignBack")));
  skin.colDropBorder = ULColorBuild(mpWidget->FindSkinColor(color,NULL,NULL,_H("DropBorder")));
  skin.colSelBack = ULColorBuild(mpWidget->FindSkinColor(color,NULL,NULL,_H("SelBack")));
  skin.colSelBorder = ULColorBuild(mpWidget->FindSkinColor(color,NULL,NULL,_H("SelBorder")));
  skin.colLastSelBack = ULColorBuild(mpWidget->FindSkinColor(color,NULL,NULL,_H("LastSelBack")));

  _NotifyUpdateLayout(TREENOTIFY_ALL);
}

///////////////////////////////////////////////
sVec4f cWidgetTree::_GetItemFrameRect() const
{
  if (skin.nodeFrame.IsOK())
    return skin.nodeFrame->GetFrame();
  return sVec4f::Zero();
}

///////////////////////////////////////////////
inline void _SaveTreeNode(iWidgetTreeNode* apNode, iDataTable* apStates) {
  if (ni::StrLen(apNode->GetName()) <= 0)
    return;
  Ptr<iDataTable> dt = ni::CreateDataTable(apNode->GetName());
  if ((apNode->GetFlags() & eWidgetTreeNodeFlags_Expandable) || apNode->GetNumChildNodes()) {
    dt->SetBool(_A("expanded"),apNode->GetExpanded());
  }
  if (apNode->GetSelected()) {
    dt->SetBool(_A("selected"),apNode->GetSelected());
  }
  niLoop(i,apNode->GetNumChildNodes()) {
    _SaveTreeNode(apNode->GetChildNode(i),dt);
  }
  apStates->AddChild(dt);
}

tBool __stdcall cWidgetTree::SaveTreeStates(iDataTable* apStates)
{
  niCheckIsOK(mpWidget,eFalse);
  niCheckIsOK(apStates,eFalse);
  // save scrolling
  {
    QPtr<iWidgetScrollBar> hscroll = mptrVScroll.ptr();
    if (hscroll.IsOK()) {
      apStates->SetFloat(_A("hscroll_position"),hscroll->GetScrollPosition());
    }
  }
  {
    QPtr<iWidgetScrollBar> vscroll = mptrVScroll.ptr();
    if (vscroll.IsOK()) {
      apStates->SetFloat(_A("vscroll_position"),vscroll->GetScrollPosition());
    }
  }
  // save nodes
  _SaveTreeNode(mptrRootNode,apStates);
  return eTrue;
}

///////////////////////////////////////////////
inline void _LoadTreeNode(iWidgetTreeNode* apNode, iDataTable* apStates, tBool abLoadMatchingName) {
  iDataTable* dt = NULL;
  if (ni::StrLen(apNode->GetName()) <= 0)
    return;
  if (abLoadMatchingName) {
    dt = apStates->GetChild(apNode->GetName());
  }
  else {
    dt = apStates->GetChildFromIndex(apNode->GetIndex());
  }
  if (dt) {
    if ((apNode->GetFlags() & eWidgetTreeNodeFlags_Expandable) || apNode->GetNumChildNodes()) {
      tU32 nIndex = dt->GetPropertyIndex(_A("expanded"));
      if (nIndex != eInvalidHandle)
        apNode->SetExpanded(dt->GetBoolFromIndex(nIndex));
    }
    {
      tU32 nIndex = dt->GetPropertyIndex(_A("selected"));
      if (nIndex != eInvalidHandle)
        apNode->SetSelected(dt->GetBoolFromIndex(nIndex));
    }
    niLoop(i,apNode->GetNumChildNodes()) {
      _LoadTreeNode(apNode->GetChildNode(i),dt,abLoadMatchingName);
    }
  }
}

tBool __stdcall cWidgetTree::LoadTreeStates(iDataTable* apStates, ni::tBool abLoadMatchingNames)
{
  niCheckIsOK(mpWidget,eFalse);
  niCheckIsOK(apStates,eFalse);
  // load nodes
  _LoadTreeNode(mptrRootNode,apStates,abLoadMatchingNames);
  // load scrolling
  {
    QPtr<iWidgetScrollBar> hscroll = mptrVScroll.ptr();
    if (hscroll.IsOK()) {
      tU32 nIndex = apStates->GetPropertyIndex(_A("hscroll_position"));
      if (nIndex != eInvalidHandle) {
        hscroll->SetScrollPosition(apStates->GetFloatFromIndex(nIndex));
      }
    }
  }
  {
    QPtr<iWidgetScrollBar> vscroll = mptrVScroll.ptr();
    if (vscroll.IsOK()) {
      tU32 nIndex = apStates->GetPropertyIndex(_A("vscroll_position"));
      if (nIndex != eInvalidHandle) {
        vscroll->SetScrollPosition(apStates->GetFloatFromIndex(nIndex));
      }
    }
  }
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetTree::PushStates()
{
  niCheckIsOK(mpWidget,eFalse);
  Ptr<iDataTable> dt = ni::CreateDataTable(_A("TreeStates"));
  niCheckSilent(dt.IsOK(),eFalse);
  if (!SaveTreeStates(dt)) return eFalse;
  mstkStates.push(dt);
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetTree::PopStates(ni::tBool abLoadMatchingNames)
{
  niCheckIsOK(mpWidget,eFalse);
  if (mstkStates.empty()) return eFalse;
  Ptr<iDataTable> dt = mstkStates.top();
  mstkStates.pop();
  return LoadTreeStates(dt,abLoadMatchingNames);
}

///////////////////////////////////////////////
void __stdcall cWidgetTree::SetNumVisibleAttributes(tU32 anCount) {
  mnNumVisibleAttributes = anCount;
}
tU32 __stdcall cWidgetTree::GetNumVisibleAttributes() const {
  return mnNumVisibleAttributes;
}

///////////////////////////////////////////////
void __stdcall cWidgetTree::SetAttributeIcon(tU32 anIndex, iOverlay* apIcon) {
  niCheckSilent(anIndex < knNumTreeNodeAttributes,;);
  mAttributes[anIndex].icon = niGetIfOK(apIcon);
}
iOverlay* __stdcall cWidgetTree::GetAttributeIcon(tU32 anIndex) const {
  niCheckSilent(anIndex < knNumTreeNodeAttributes,NULL);
  return mAttributes[anIndex].icon;
}

///////////////////////////////////////////////
void __stdcall cWidgetTree::SetAttributeName(tU32 anIndex, iHString* ahspName) {
  niCheckSilent(anIndex < knNumTreeNodeAttributes,;);
  mAttributes[anIndex].name = ahspName;
}
iHString* __stdcall cWidgetTree::GetAttributeName(tU32 anIndex) const {
  niCheckSilent(anIndex < knNumTreeNodeAttributes,NULL);
  return mAttributes[anIndex].name;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetTree::SetHoverNode(tU32 anNodeTreeIndex) {
  mnHoverTreeNodeIndex = anNodeTreeIndex;
  return eTrue;
}
tU32 __stdcall cWidgetTree::GetHoverNode() const {
  return mnHoverTreeNodeIndex;
}
