// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Image.h"
#include "ImagePacker.h"

#define BMPMAPTRACE(MSG)  //niDebugFmt(MSG)

cImagePacker::cImagePacker(
    iGraphics* apGraphics,
    tU32 anWidth,
    tU32 anHeight,
    tBool abUseMipMaps,
    const char* aaszBaseName,
    const char* aaszPxf)
{
  mlMinHole = 5;
  mbIsFull = false;

  mnNumMipMaps = abUseMipMaps ? ni::ComputeNumPow2Levels(anWidth>>1,anHeight>>1) : 0;

  Ptr<iTexture>  ptrTex = apGraphics->CreateTexture(
      _H(niFmt(_A("%s_%p"),aaszBaseName,(void*)this)),
      eBitmapType_2D,
      aaszPxf,mnNumMipMaps,anWidth,anHeight,0,
      eTextureFlags_Overlay);
  niCheck(ptrTex.IsOK(),;);

  Ptr<iBitmap2D> ptrBmp = apGraphics->CreateBitmap2D(anWidth,anHeight,ptrTex->GetPixelFormat()->GetFormat());
  niCheck(ptrBmp.IsOK(),;);
  if (mnNumMipMaps) {
    ptrBmp->CreateMipMaps(mnNumMipMaps,eFalse);
  }

  mptrImage = apGraphics->CreateImageFromBitmapAndTexture(ptrBmp,ptrTex);
  niCheck(mptrImage.IsOK(),;);

  Image_SetDebugName(mptrImage,niFmt(_A("IMGPK_%p"),(void*)this));
  Clear();
}

cImagePacker::~cImagePacker()
{
  mptrImage = NULL;
}

tBool __stdcall cImagePacker::IsOK() const {
  return mptrImage.IsOK();
}

void __stdcall cImagePacker::Invalidate() {
  if (mptrImage.IsOK()) {
    mptrImage->Invalidate();
    mptrImage = NULL;
  }
}

tU32 cImagePacker::GetWidth() const {
  return mptrImage->GetWidth();
}
tU32 cImagePacker::GetHeight() const {
  return mptrImage->GetHeight();
}

tU32 __stdcall cImagePacker::GetNumMipMaps() const {
  return mnNumMipMaps;
}

void cImagePacker::Clear()
{
  mRects.Clear();
  // Root node in rect tree
  mRects.Insert(cBitmapRect(0,0,GetWidth(),GetHeight(),-1));

  iBitmap2D* bmp = _GetTargetBitmap(sRecti::Null());
  if (bmp) {
    niLoop(i,1+bmp->GetNumMipMaps()) {
      bmp->GetLevel(i)->Clear();
    }
  }
}

void cImagePacker::DrawOne(iBitmap2D* apBmp, const sVec2i& aNewPos)
{
  Ptr<iBitmap2D> bmp = _GetTargetBitmap(sRecti(aNewPos.x,aNewPos.y,apBmp->GetWidth(),
                                                    apBmp->GetHeight()));
  bmp->Blit(apBmp,0,0,aNewPos.x,aNewPos.y,apBmp->GetWidth(),apBmp->GetHeight());
  const tU32 nNumMips = bmp->GetNumMipMaps();
  if (nNumMips) {
    sVec2i pos = aNewPos;
    sVec2i size = Vec2i(apBmp->GetWidth(),apBmp->GetHeight());
    Ptr<iBitmap2D> srcPrev = apBmp;
    niLoop(i,nNumMips) {
      pos /= 2;
      size /= 2;
      Ptr<iBitmap2D> src = apBmp->GetMipMap(i);
      if (!src.IsOK()) {
        src = srcPrev->CreateResized(size.x,size.y);
      }
      niAssert(src.IsOK());
      Ptr<iBitmap2D> d = bmp->GetMipMap(i);
      if (niIsOK(d)) {
        d->Blit(src,0,0,
                pos.x,pos.y,
                src->GetWidth(),src->GetHeight());
      }
      srcPrev = src;
    }
  }
}

void cImagePacker::DrawBitmap(iBitmap2D* apBmp, const sVec2i& aDestPos, tBool abFiltered) {
  if (abFiltered) {
    for (int i=-1; i<=1; ++i) {
      for(int j=-1; j<=1; ++j) {
        if ((i==0 || j==0) && (i!=j)) {
          DrawOne(apBmp,Vec2<tI32>(aDestPos.x+1+i,aDestPos.y+1+j));
        }
      }
    }
  }

  DrawOne(apBmp,Vec2<tI32>(aDestPos.x,aDestPos.y));
}

sRecti cImagePacker::_DoInsert(const sRecti& aRect, iBitmap2D *apSrc, tBool abFiltered, const int aPadding)
{
  tBool bFoundEmptyNode = false;

  //Debug
  int node=0;
  niUnused(node);

  //Get the leaves of the tree and search it for a good pos.
  tRectTreeNodeList& lstNodes =  mRects.GetLeafList();
  tRectTreeNodeListIt it;
  for(it = lstNodes.begin();it!=lstNodes.end();++it)
  {
    BMPMAPTRACE((_A("Checking node %d:\n"),node++));
    tRectTreeNode *TopNode = *it;
    cBitmapRect* pData = TopNode->GetData();

    //Check if the space is free
    if (pData->mlHandle<0) {
      BMPMAPTRACE(("Found free node\n"));
      bFoundEmptyNode = true; //An empty node was found.. bitmap not full yet.

      //Check if the Image fits in the rect
      const sRecti NewRect = sRecti(
          pData->mRect.x, pData->mRect.y,
          aRect.GetWidth()+(aPadding*2),
          aRect.GetHeight()+(aPadding*2));
      BMPMAPTRACE(("Fit: %s in %s\n",NewRect,pData->mRect));

      if (BoxFit(NewRect, pData->mRect))
      {
        BMPMAPTRACE(("The node fits!\n"));

        // If the bitmap fits perfectly add the node without splitting
        if (MinimumFit(NewRect,pData->mRect)) {
          BMPMAPTRACE(("Minimum fit!\n"));
          pData->mRect = NewRect;
          pData->mlHandle = 1;
        }
        // If there is still space left, make new nodes.
        else {
          BMPMAPTRACE(("Normal fit!\n"));
          //Insert 2 children for the top node (lower and upper part.
          tRectTreeNode* UpperNode;
          //Upper
          UpperNode = mRects.InsertAt(
              cBitmapRect(NewRect.x,NewRect.y,
                          pData->mRect.GetWidth(),NewRect.GetHeight(),-2),
              TopNode,
              eBinTreeNode_Left);

          //Lower
          mRects.InsertAt(
              cBitmapRect(NewRect.x,NewRect.y+NewRect.GetHeight(),
                          pData->mRect.GetWidth(),pData->mRect.GetHeight()-NewRect.GetHeight(),-3),
              TopNode,
              eBinTreeNode_Right);

          //Split the Upper Node into 2 nodes.
          pData = UpperNode->GetData();//Get the data for the upper node.
          //Upper split, this is the new bitmap
          mRects.InsertAt(
              cBitmapRect(NewRect.x,NewRect.y,
                          NewRect.GetWidth(),NewRect.GetHeight(),2),
              UpperNode,
              eBinTreeNode_Left);

          //Lower split, this is empty
          mRects.InsertAt(
              cBitmapRect(NewRect.x+NewRect.GetWidth(),NewRect.y,
                          pData->mRect.GetWidth()-NewRect.GetWidth(),NewRect.GetHeight(),-4),
              UpperNode,
              eBinTreeNode_Right);
        }

        if (apSrc) {
          DrawBitmap(apSrc,Vec2i(NewRect.x+aPadding,NewRect.y+aPadding),abFiltered);
        }

        mvRects.push_back(
            sRecti(NewRect.x+aPadding,NewRect.y+aPadding,
                   aRect.GetWidth(),aRect.GetHeight()));
        return mvRects.back();
      }
    }
  }

  if (!bFoundEmptyNode) {
    mbIsFull = true;
  }
  return sRecti::Null();
}

//-----------------------------------------------------------------------

tBool cImagePacker::MinimumFit(sRecti aSrc,sRecti aDest)
{
  if ((aDest.GetWidth()-aSrc.GetWidth())    < mlMinHole &&
      (aDest.GetHeight()-aSrc.GetHeight())  < mlMinHole)
    return true;

  return false;
}

//-----------------------------------------------------------------------

tBool cImagePacker::GetIsFull() const
{
  return mbIsFull;
}
