#ifndef __WIDGETTREE_680079_H__
#define __WIDGETTREE_680079_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

class cWidgetTreeNode;
#include "IntervalOverlap.h"

typedef astl::vector<Ptr<cWidgetTreeNode> > tTreeNodeVec;
typedef astl::vector<cWidgetTreeNode*>  tTreeNodePVec;
typedef astl::list<cWidgetTreeNode*>  tTreeNodePLst;

static const tU32 knNumTreeNodeAttributes = 32;

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidgetTree declaration.
class cWidgetTree : public ni::cIUnknownImpl<ni::iWidgetSink,ni::eIUnknownImplFlags_Default,ni::iWidgetTree>
{
  niBeginClass(cWidgetTree);

 public:
  //! Constructor.
  cWidgetTree(iWidget* apWidget);
  //! Destructor.
  ~cWidgetTree();

  //! Zeros all the class members.
  void ZeroMembers();
  //! Sanity check.
  ni::tBool __stdcall IsOK() const;

  //// ni::iWidgetSink /////////////////////////
  tBool __stdcall OnWidgetSink(iWidget *apWidget, tU32 anMsg, const Var& avarA, const Var& avarB);
  //// ni::iWidgetSink /////////////////////////

  //// ni::iWidgetTree /////////////////////////
  ni::iWidgetTreeNode * __stdcall GetRootNode() const;
  tU32 __stdcall GetNumNodes() const;
  iWidgetTreeNode* __stdcall GetNode(tU32 anIndex) const;
  tU32 __stdcall GetNumSelectedNodes() const;
  ni::iWidgetTreeNode * __stdcall GetSelectedNode(tU32 anIndex) const;
  tBool __stdcall Clear();
  tBool __stdcall ClearSelection();
  iWidgetTreeNode* __stdcall GetNodeFromPosition(const sVec2f& avAbsPos) const;
  eWidgetTreeNodeDropMode __stdcall GetNodeDropMode(iWidgetTreeNode* apNode, const sVec2f& avAbsPos) const;
  void __stdcall SetSecondarySelection(iWidgetTreeNode* apNode);
  iWidgetTreeNode* __stdcall GetSecondarySelection() const;
  iWidgetTreeNode* __stdcall GetPivotSelection() const;
  void __stdcall ScrollToNode(iWidgetTreeNode* apNode);
  tBool __stdcall SaveTreeStates(iDataTable* apStates);
  tBool __stdcall LoadTreeStates(iDataTable* apStates, ni::tBool abLoadMatchingNames);
  tBool __stdcall PushStates();
  tBool __stdcall PopStates(ni::tBool abLoadMatchingNames);
  void __stdcall SetNumVisibleAttributes(tU32 anCount);
  tU32 __stdcall GetNumVisibleAttributes() const;
  void __stdcall SetAttributeIcon(tU32 anIndex, iOverlay* apIcon);
  iOverlay* __stdcall GetAttributeIcon(tU32 anIndex) const;
  void __stdcall SetAttributeName(tU32 anIndex, iHString* ahspName);
  iHString* __stdcall GetAttributeName(tU32 anIndex) const;
  tBool __stdcall SetHoverNode(tU32 anNodeTreeIndex);
  tU32 __stdcall GetHoverNode() const;
  //// ni::iWidgetTree /////////////////////////

  void _AddSelectedNode(cWidgetTreeNode* apNode);
  void _RemoveSelectedNode(cWidgetTreeNode* apNode);
  void _Paint(const sVec2f& avMousePos, iCanvas* c);
  void _ProcessClick(tU32 aMsg, const sVec2f& avMousePos, tBool abExecute, tU32 aModifier);
  void _UpdateScrollOffset();
  ni::iWidgetTreeNode* __stdcall _FindNodeByPos(const sVec2f& avMousePos) const;
  ni::iWidgetTreeNode* __stdcall _GetHomeNode() const;
  ni::iWidgetTreeNode* __stdcall _GetEndNode() const;
  tBool _DoSelect(ni::iWidgetTreeNode* apNode, tU32 aModifier, tBool abForceSetSelect);
  tBool _DoSelectFinal(ni::iWidgetTreeNode* apNode, tU32 aModifier, tBool abForceSetSelect);
  void _ClearSelectRange();
  void _SetSelectRange(ni::iWidgetTreeNode* apFirst, ni::iWidgetTreeNode* apLast);
  void _RegisterNode(cWidgetTreeNode* apNode);
  void _UnregisterNode(cWidgetTreeNode* apNode);
  void _NotifyUpdateLayout(tU32 anDirty);
  void _UpdateLayout();
  void _DoUpdateLayout(tU32 anFlags);
  void _UpdateSkin();
  sVec4f _GetItemFrameRect() const;
  void _CreateWidgetScrollBars();

 public:
  iWidget*                mpWidget;
  tTreeNodePVec             mvNodes;
  tTreeNodePLst             mlstVisibleNodes;
  tTreeNodePVec             mvSelected;
  cWidgetTreeNode*            mpSelectedLast;
  cWidgetTreeNode*            mpSecondarySel;
  cWidgetTreeNode*            mpSelectedRangeLast;
  ni::Ptr<ni::iWidgetTreeNode>  mptrRootNode;
  ni::Ptr<ni::iWidget>      mptrVScroll;
  ni::Ptr<ni::iWidget>      mptrHScroll;
  sVec2f               mvScrollOffset;
  ni::tBool               mbDragging;
  eWidgetTreeNodeDropMode         mDropMode;
  ni::Ptr<ni::iWidgetTreeNode>  mptrHighlightedNode;
  // ni::tBool                mbProcessLeftClickUp;
  tU32                  mnNotify;
  tIntPtr                                 mnHoverTreeNodeIndex;
  struct sSkin {
    Ptr<iOverlay>  nodeFrame;
    Ptr<iOverlay>  collapsed;
    Ptr<iOverlay>  expanded;
    Ptr<iOverlay>  node;
    tU32                 colFirst;
    tU32                 colSec;
    tU32                 colRowHover;
    tU32                 colTextFront;
    tU32                 colTextBack;
    tU32                 colSignBack;
    tU32                 colDropBorder;
    tU32                 colSelBack;
    tU32                 colSelBorder;
    tU32                 colLastSelBack;
  } skin;
  astl::stack<Ptr<iDataTable>,astl::list<Ptr<iDataTable> > >    mstkStates;

  struct sAttributeDef {
    Ptr<iOverlay>     icon;
    tHStringPtr             name;
  };
  sAttributeDef  mAttributes[knNumTreeNodeAttributes];
  tU32           mnNumVisibleAttributes;

  niEndClass(cWidgetTree);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __WIDGETTREE_680079_H__
