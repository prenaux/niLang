#ifndef __BINTREE_310502_H__
#define __BINTREE_310502_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Types.h>

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

//! Binary Tree node type.
enum eBinTreeNode {
  eBinTreeNode_Left,
  eBinTreeNode_Right
};

//! Binary Tree node template.
template<class T> class BinTreeNode {
 public:
  T* GetData()
  {
    return &mData;
  }

  BinTreeNode(T aData, BinTreeNode<T> *aParent, eBinTreeNode aParentDir)
  {
    for(int i=0;i<2;i++) mChild[i]=NULL;
    mData = aData;
    mParent = aParent;
    mParentDir = aParentDir;
  }

  BinTreeNode<T>* AddChild(eBinTreeNode i, T aData)
  {
    if(mChild[i]==NULL){
      mChild[i] = new BinTreeNode<T>(aData, this,i);
      return mChild[i];
    }
    return NULL;
  }

  BinTreeNode<T>* GetChild(eBinTreeNode i)
  {
    return mChild[i];
  }

  BinTreeNode<T>* GetParent()
  {
    return mParent;
  }

 private:
  BinTreeNode<T>* mChild[2];
  BinTreeNode<T>* mParent;
  T mData;
  eBinTreeNode mParentDir;
};

//! Binary tree template.
template<class T> class BinTree {
 public:
  BinTree(){
    mlNumOfNodes =0;
    mFirstNode = NULL;
  }

  ~BinTree()
  {
    Clear();
  }

  /**
   * Clears the entire tree
   * \return number of nodes deleted
   */
  int Clear()
  {
    mlNum=0;
    DeleteNode(mFirstNode);
    mFirstNode = NULL;
    return mlNum;
  }

  /**
   * Insert a node to the tree.
   * \todo only works to set the root node.
   * \param aData the data to insert
   * \return
   */
  BinTreeNode<T>* Insert(T aData)
  {
    if(mFirstNode==NULL)
    {
      mFirstNode = new BinTreeNode<T>(aData, NULL,eBinTreeNode_Left);
      mlNumOfNodes++;

      return mFirstNode;
    }

    //Insertion other then at the root is not supported!
    BinTreeNode<T>* Node = mFirstNode;
    eBinTreeNode c;
    while(true)
    {
      //if(Node->GetData()<aData)
      c = eBinTreeNode_Left;
      //else
      //  c = eBinTreeNode_Right;

      if(Node->GetChild(c)==NULL)
      {
        Node = Node->AddChild(c, aData);
        break;
      }
      else
      {
        Node = Node->GetChild(c);
      }
    }
    mlNumOfNodes++;

    return Node;
  }

  /**
   * Insert a node into a certain node in the tree
   * \param aData the data to insert
   * \param aNode the node to insert the data in
   * \param aChild what child to insert at
   * \return
   */
  BinTreeNode<T>* InsertAt(T aData,BinTreeNode<T>* aNode, eBinTreeNode aChild=eBinTreeNode_Left)
  {
    if(aNode == NULL)return NULL;

    if(aNode->GetChild(aChild)!=NULL) {
      aChild = aChild==eBinTreeNode_Left ? eBinTreeNode_Right : eBinTreeNode_Left;
      if(aNode->GetChild(aChild)!=NULL)return NULL;
    }

    return aNode->AddChild(aChild, aData);
  }

  /**
   * Get the size of the tree
   * \return
   */
  int Size()
  {
    return mlNumOfNodes;
  }

  astl::list<BinTreeNode<T>*>& GetLeafList()
  {
    mlstNodes.clear();
    PopulateLeafList(mFirstNode);
    return  mlstNodes;
  }

  /**
   * Get a list of all the nodes in the tree
   * \return
   */
  astl::list<BinTreeNode<T>*>& GetNodeList()
  {
    mlstNodes.clear();
    PopulateNodeList(mFirstNode);
    return  mlstNodes;
  }

 private:
  int mlNumOfNodes;
  BinTreeNode<T>* mFirstNode;
  int mlNum;

  astl::list<BinTreeNode<T>*> mlstNodes;

  void DeleteNode(BinTreeNode<T>* aNode)
  {
    if(aNode==NULL) return;

    DeleteNode(aNode->GetChild(eBinTreeNode_Left));
    DeleteNode(aNode->GetChild(eBinTreeNode_Right));

    delete aNode;
    mlNum++;
  }

  void PopulateNodeList(BinTreeNode<T>* aNode)
  {
    if(aNode==NULL) return;

    PopulateNodeList(aNode->GetChild(eBinTreeNode_Left));
    mlstNodes.push_back(aNode);
    PopulateNodeList(aNode->GetChild(eBinTreeNode_Right));
  }

  void PopulateLeafList(BinTreeNode<T>* aNode)
  {
    if(aNode==NULL) return;

    if(aNode->GetChild(eBinTreeNode_Left)==NULL &&
       aNode->GetChild(eBinTreeNode_Right)==NULL)
    {
      mlstNodes.push_back(aNode);
    }

    PopulateLeafList(aNode->GetChild(eBinTreeNode_Left));
    PopulateLeafList(aNode->GetChild(eBinTreeNode_Right));
  }


};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __BINTREE_310502_H__
