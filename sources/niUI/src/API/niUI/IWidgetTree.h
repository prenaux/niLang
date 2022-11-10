#ifndef __IWIDGETTREE_83152536_H__
#define __IWIDGETTREE_83152536_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {

struct iWidgetTree;
struct iWidgetTreeNode;

/** \addtogroup niUI
 * @{
 */

//! Widget tree node drop mode.
enum eWidgetTreeNodeDropMode {
  //! Drop on the node.
  eWidgetTreeNodeDropMode_On = 0,
  //! Drop below the node.
  eWidgetTreeNodeDropMode_Below = 1,
  //! Drop above the node.
  eWidgetTreeNodeDropMode_Above = 2,
  //! Invalid drop area.
  eWidgetTreeNodeDropMode_Invalid = 3,
  //! \internal
  eWidgetTreeNodeDropMode_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Widget tree style.
enum eWidgetTreeStyle
{
  //! Select the item on left click down. Note that this disables the
  //! secondary selection.
  eWidgetTreeStyle_ClickDownSelect = niBit(eWidgetStyle_MaxBit+0),
  //! Don't show the horizontal scrollbar.
  eWidgetTreeStyle_NoHScroll = niBit(eWidgetStyle_MaxBit+1),
  //! Don't show the vertical scrollbar.
  eWidgetTreeStyle_NoVScroll = niBit(eWidgetStyle_MaxBit+2),
  //! Automaticall scroll to the selected nodes.
  eWidgetTreeStyle_Autoscroll = niBit(eWidgetStyle_MaxBit+3),
  //! Don't draw the root node.
  eWidgetTreeStyle_DontDrawRoot = niBit(eWidgetStyle_MaxBit+4),
  //! Always show the horizontal scrollbar.
  eWidgetTreeStyle_AlwaysHScroll = niBit(eWidgetStyle_MaxBit+5),
  //! Always show the vertical scrollbar.
  eWidgetTreeStyle_AlwaysVScroll = niBit(eWidgetStyle_MaxBit+6),
  //! Single selection only.
  eWidgetTreeStyle_SingleSelection = niBit(eWidgetStyle_MaxBit+7),
  //! \internal
  eWidgetTreeStyle_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Widget tree notify message
enum eWidgetTreeCmd
{
  //! A tree node has been selected.
  eWidgetTreeCmd_Selected = 0,
  //! \param A iWidgetTreeNode
  //! A tree node has been unselected.
  //! \param A iWidgetTreeNode
  eWidgetTreeCmd_Unselected = 1,
  //! A tree node has been expanded.
  //! \param A iWidgetTreeNode
  //! \remark This is sent before the node is actually drawn so it's the good time
  //!     to fill the node with the appropriate children if it's not already done.
  eWidgetTreeCmd_Expanded = 2,
  //! A tree node has been collapsed.
  //! \param A iWidgetTreeNode
  eWidgetTreeCmd_Collapsed = 3,
  //! A tree node has been selected as a secondary selection.
  //! \param A iWidgetTreeNode
  eWidgetTreeCmd_SecondarySelected = 4,
  //! A tree node has been unselected as a secondary selection.
  //! \param A iWidgetTreeNode
  eWidgetTreeCmd_SecondaryUnselected = 5,
  //! A tree node's attribute have changed.
  //! \param A iWidgetTreeNode
  //! \param B the previous attributes
  eWidgetTreeCmd_SetAttributes = 6,
  //! A tree node's name has changed.
  //! \param A iWidgetTreeNode
  //! \param B the previous attributes
  eWidgetTreeCmd_SetName = 7,
  //! \internal
  eWidgetTreeCmd_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Widget tree node flags.
enum eWidgetTreeNodeFlags
{
  //! The node is selected.
  eWidgetTreeNodeFlags_Selected = niBit(0),
  //! The node is expanded.
  eWidgetTreeNodeFlags_Expanded = niBit(1),
  //! Don't draw the name of the node.
  eWidgetTreeNodeFlags_DontDrawName = niBit(2),
  //! The node is expandable even if it doesn't have any children.
  eWidgetTreeNodeFlags_Expandable = niBit(3),
  //! The widget in the tree node will be placed vertically.
  eWidgetTreeNodeFlags_WidgetPlaceV = niBit(4),
  //! The widget in the tree node will be placed horizontally.
  eWidgetTreeNodeFlags_WidgetPlaceH = niBit(5),
  //! The widget in the tree node will be placed vertically and horizontally.
  eWidgetTreeNodeFlags_WidgetPlace = eWidgetTreeNodeFlags_WidgetPlaceV|eWidgetTreeNodeFlags_WidgetPlaceH,
  //! The widget in the tree node will be sized to fit in the tree.
  eWidgetTreeNodeFlags_WidgetSize = niBit(6),
  //! The widget in the tree node will be placed at the left of the text.
  eWidgetTreeNodeFlags_WidgetLeft = niBit(7),
  //! Use the tree node's text color.
  eWidgetTreeNodeFlags_UseTextColor = niBit(8),
  //! The widget tree node is visible.
  eWidgetTreeNodeFlags_Visible = niBit(9),
  //! Default tree node flags.
  eWidgetTreeNodeFlags_Default = eWidgetTreeNodeFlags_WidgetPlace|eWidgetTreeNodeFlags_Visible,
  //! \internal
  eWidgetTreeNodeFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Widget tree node flags type. \see eWidgetTreeNodeFlags
typedef tU32 tWidgetTreeNodeFlags;

//////////////////////////////////////////////////////////////////////////////////////////////
//! Widget tree node interface.
struct iWidgetTreeNode : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetTreeNode,0x96fa976d,0xc757,0x42d9,0xb8,0xef,0xde,0x82,0x35,0x9d,0x1a,0x6c)

  //########################################################################################
  //! \name Nodes
  //########################################################################################
  //! @{

  //! Get the parent widget tree.
  //! {Property}
  virtual iWidget* __stdcall GetParentWidget() const = 0;
  //! Get the parent node.
  //! \remark NULL if root node.
  //! {Property}
  virtual iWidgetTreeNode* __stdcall GetParentNode() const = 0;
  //! Get the node's rectangle.
  //! \remark Zero if the node is not visible.
  //! {Property}
  virtual sRectf __stdcall GetNodeRect() const = 0;
  //! Get the node's scrolled rectangle.
  //! \remark Zero if the node is not visible.
  //! {Property}
  virtual sRectf __stdcall GetScrolledNodeRect() const = 0;
  //! Get the node's absolute rectangle.
  //! \remark Zero if the node is not visible.
  //! {Property}
  virtual sRectf __stdcall GetAbsoluteNodeRect() const = 0;
  //! Get the node's text rectangle.
  //! \remark Zero if the node is not visible.
  //! {Property}
  virtual sRectf __stdcall GetTextRect() const = 0;
  //! Get the node's scrolled text rectangle.
  //! \remark Zero if the node is not visible.
  //! {Property}
  virtual sRectf __stdcall GetScrolledTextRect() const = 0;
  //! Get the node's absolute rectangle.
  //! \remark Zero if the node is not visible.
  //! {Property}
  virtual sRectf __stdcall GetAbsoluteTextRect() const = 0;
  //! Get the node index.
  //! {Property}
  virtual tU32 __stdcall GetIndex() const = 0;
  //! Get the tree index.
  //! {Property}
  virtual tU32 __stdcall GetTreeIndex() const = 0;
  //! Get the number of child nodes.
  //! {Property}
  virtual tU32 __stdcall GetNumChildNodes() const = 0;
  //! Get the child node at the specified index.
  //! {Property}
  virtual iWidgetTreeNode* __stdcall GetChildNode(tU32 anIndex) const = 0;
  //! Get the index of the specified child node.
  //! {Property}
  //! \remark Return eInvalidHandle if the specified node isnt a child of this node.
  virtual tU32 __stdcall GetChildNodeIndex(const iWidgetTreeNode* apNode) const = 0;
  //! Get the first child node with the specified name.
  //! {Property}
  //! \remark This function will search in the children nodes as well.
  virtual iWidgetTreeNode* __stdcall GetChildNodeFromName(const achar* aaszName) const = 0;
  //! Add a new child node.
  virtual iWidgetTreeNode* __stdcall AddChildNode(const achar* aaszName) = 0;
  //! Add a new child node before the specified index.
  //! \remark If the index is out of range it behaves like AddChildNode (aka add a node at the end)
  virtual iWidgetTreeNode* __stdcall AddChildNodeBefore(const achar* aaszName, tU32 anIndex) = 0;
  //! Remove a child node.
  //! \remark This function will search in the children nodes as well.
  virtual tBool __stdcall RemoveChildNode(iWidgetTreeNode* apNode) = 0;
  //! Remove all children.
  virtual tBool __stdcall Clear() = 0;
  //! Find the first node with the specified name.
  //! \remark This include this node.
  virtual iWidgetTreeNode* __stdcall FindNodeFromName(const achar* aaszName) const = 0;
  //! Find the first node with the specified userdata.
  //! \remark This include this node.
  virtual iWidgetTreeNode* __stdcall FindNodeFromUserdata(const iUnknown* apUserdata) const = 0;
  //! Find the first node with the specified widget.
  //! \remark This include this node.
  virtual iWidgetTreeNode* __stdcall FindNodeFromWidget(const iWidget* apWidget) const = 0;
  //! Find the node that is below the specified position, position in absolute coordinates.
  //! \remark This include this node.
  virtual iWidgetTreeNode* __stdcall FindNodeFromPosition(const sVec2f& avPos) const = 0;
  //! Get the previous sibiling of this tree node.
  //! {Property}
  //! \remark Return NULL if it's the first child of its parent.
  virtual iWidgetTreeNode* __stdcall GetPrevSibling() const = 0;
  //! Get the next sibiling of this tree node.
  //! {Property}
  //! \remark Return NULL if it's the last child of its parent.
  virtual iWidgetTreeNode* __stdcall GetNextSibling() const = 0;
  //! Get the node above of this node, can be in another parent node.
  //! {Property}
  //! \remark Return null if its the node the most above.
  virtual iWidgetTreeNode* __stdcall GetAbove() const = 0;
  //! Get the node below of this node, can be in another parent node.
  //! {Property}
  //! \remark Return null if its the node the most below.
  virtual iWidgetTreeNode* __stdcall GetBelow() const = 0;
  //! @}


  //########################################################################################
  //! \name Properties
  //########################################################################################
  //! @{

  //! Set the node's flags.
  //! {Property}
  virtual tBool __stdcall SetFlags(tWidgetTreeNodeFlags aFlags) = 0;
  //! Get the node's flags.
  //! {Property}
  virtual tWidgetTreeNodeFlags __stdcall GetFlags() const = 0;
  //! Set the node's name.
  //! {Property}
  virtual tBool __stdcall SetName(const achar* aVal) = 0;
  //! Get the node's name.
  //! {Property}
  virtual const achar* __stdcall GetName() const = 0;
  //! Set the node's display name.
  //! {Property}
  virtual tBool __stdcall SetDisplayName(const achar* aVal) = 0;
  //! Get the node's display name.
  //! {Property}
  virtual const achar* __stdcall GetDisplayName() const = 0;
  //! Set the node's icon
  //! {Property}
  virtual tBool __stdcall SetIcon(iOverlay* aVal) = 0;
  //! Get the node's icon
  //! {Property}
  virtual iOverlay* __stdcall GetIcon() const = 0;
  //! Set the node's selected status.
  //! {Property}
  //! \remark When set/unset selected the node's tree selection list
  //!     will be automatically updated.
  virtual tBool __stdcall SetSelected(tBool abSelected) = 0;
  //! Get the node's selected status.
  //! {Property}
  virtual tBool __stdcall GetSelected() const = 0;
  //! Set the node's expanded status.
  //! {Property}
  virtual tBool __stdcall SetExpanded(tBool abExpanded) = 0;
  //! Get the node's expanded status.
  //! {Property}
  virtual tBool __stdcall GetExpanded() const = 0;
  //! Set the node's user data.
  //! {Property}
  virtual tBool __stdcall SetUserdata(iUnknown* apUserData) = 0;
  //! Get the node's user data.
  //! {Property}
  virtual iUnknown* __stdcall GetUserdata() const = 0;
  //! Set the node's widget.
  //! {Property}
  virtual tBool __stdcall SetWidget(iWidget* apWidget) = 0;
  //! Get the node's widget.
  //! {Property}
  virtual iWidget* __stdcall GetWidget() const = 0;
  //! Set the text foreground color.
  //! {Property}
  virtual void __stdcall SetTextColor(tU32 anColor) = 0;
  //! Get the text foreground color.
  //! {Property}
  virtual tU32 __stdcall GetTextColor() const  = 0;
  //! Set the text background color.
  //! {Property}
  virtual void __stdcall SetTextBackColor(tU32 anColor) = 0;
  //! Get the text background color.
  //! {Property}
  virtual tU32 __stdcall GetTextBackColor() const = 0;
  //! Set an explicit row background color.
  //! {Property}
  //! \remark Enabled if the color is different of 0x00000000
  virtual void __stdcall SetRowColor(tU32 anRow) = 0;
  //! Get an explicit row background color.
  //! {Property}
  virtual tU32 __stdcall GetRowColor() const = 0;
  //! @}

  //########################################################################################
  //! \name Attributes
  //########################################################################################
  //! @{

  //! Set the node's attributes.
  //! {Property}
  virtual void __stdcall SetAttributes(tU32 anAttributes) = 0;
  //! Get the node's attributes.
  //! {Property}
  virtual tU32 __stdcall GetAttributes() const = 0;
  //! Set the visible attributes mask.
  //! {Property}
  //! \remark By default is 0xFFFFFFFF, all visible
  virtual void __stdcall SetVisibleAttributesMask(tU32 anAttributes) = 0;
  //! Set the visible attributes mask.
  //! {Property}
  virtual tU32 __stdcall GetVisibleAttributesMask() const = 0;
  //! @}
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! Widget tree interface.
struct iWidgetTree : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetTree,0xb9cd35a1,0x6ad7,0x442d,0x87,0x7b,0xfd,0xc0,0xaa,0x9f,0x6c,0xf9)
  //! Get the root tree node.
  //! {Property}
  virtual iWidgetTreeNode* __stdcall GetRootNode() const = 0;
  //! Get the number of nodes.
  //! {Property}
  virtual tU32 __stdcall GetNumNodes() const = 0;
  //! Get the node at the specified index.
  //! {Property}
  virtual iWidgetTreeNode* __stdcall GetNode(tU32 anIndex) const = 0;
  //! Get the number of nodes selected.
  //! {Property}
  virtual tU32 __stdcall GetNumSelectedNodes() const = 0;
  //! Get the selected node at the specified index.
  //! {Property}
  virtual iWidgetTreeNode* __stdcall GetSelectedNode(tU32 anIndex) const = 0;
  //! Clear the tree.
  //! \remark This clear the root node, notify node and selection.
  virtual tBool __stdcall Clear() = 0;
  //! Clear the selection of all nodes.
  virtual tBool __stdcall ClearSelection() = 0;
  //! Get a node from an absolute position.
  virtual iWidgetTreeNode* __stdcall GetNodeFromPosition(const sVec2f& avAbsPos) const = 0;
  //! Get the drop mode of the specified node from the specified absolute position.
  virtual eWidgetTreeNodeDropMode __stdcall GetNodeDropMode(iWidgetTreeNode* apNode, const sVec2f& avAbsPos) const = 0;
  //! Set the secondary selection item.
  //! {Property}
  virtual void __stdcall SetSecondarySelection(iWidgetTreeNode* apNode) = 0;
  //! Get the secondary selection item.
  //! {Property}
  virtual iWidgetTreeNode* __stdcall GetSecondarySelection() const = 0;
  //! Get the pivot selection.
  //! {Property}
  virtual iWidgetTreeNode* __stdcall GetPivotSelection() const = 0;
  //! Scroll to the specified item node.
  //! {Propety}
  virtual void __stdcall ScrollToNode(iWidgetTreeNode* apNode) = 0;

  //! Save tree states.
  //! \remark Tree states are the expanded status, selected status and scrolling.
  virtual tBool __stdcall SaveTreeStates(iDataTable* apStates) = 0;
  //! Load tree states.
  virtual tBool __stdcall LoadTreeStates(iDataTable* apStates, ni::tBool abLoadMatchingNames) = 0;
  //! Push the tree's states.
  virtual tBool __stdcall PushStates() = 0;
  //! Pop the tree's states.
  virtual tBool __stdcall PopStates(ni::tBool abLoadMatchingNames) = 0;

  //! Set the number of visible attributes.
  //! {Property}
  virtual void __stdcall SetNumVisibleAttributes(tU32 anCount) = 0;
  //! Get the number of visible attributes.
  //! {Property}
  virtual tU32 __stdcall GetNumVisibleAttributes() const = 0;
  //! Set the specified attribute's icon.
  //! {Property}
  virtual void __stdcall SetAttributeIcon(tU32 anIndex, iOverlay* apIcon) = 0;
  //! Get the specified attribute's icon.
  //! {Property}
  virtual iOverlay* __stdcall GetAttributeIcon(tU32 anIndex) const = 0;
  //! Set the specified attribute's name.
  //! {Property}
  virtual void __stdcall SetAttributeName(tU32 anIndex, iHString* ahspName) = 0;
  //! Get the specified attribute's name.
  //! {Property}
  virtual iHString* __stdcall GetAttributeName(tU32 anIndex) const = 0;

  //! Set the hover node.
  //! {Property}
  //! \remark The id should be the node's tree index.
  virtual tBool __stdcall SetHoverNode(tU32 anNodeTreeIndex) = 0;
  //! Get the hover node.
  //! {Property}
  //! \remark The id is the node's tree index.
  virtual tU32 __stdcall GetHoverNode() const = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IWIDGETTREE_83152536_H__
