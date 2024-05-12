#ifndef __IMAGEPACKER_H_6D6895F2_4AE0_46E7_AC89_A2871D2C87FC__
#define __IMAGEPACKER_H_6D6895F2_4AE0_46E7_AC89_A2871D2C87FC__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include <niLang/Utils/BinTree.h>

///
/// cImagePacker
///
class cImagePacker : public ImplRC<iUnknown>
{
  // The frames bitmap + rect class
  class cBitmapRect
  {
   public:
    cBitmapRect(){mlHandle=-1;}
    cBitmapRect(int x,int y,int w,int h, int alHandle){
      mRect = sRecti(x,y,w,h);
      mlHandle = alHandle;
    }

    sRecti mRect;
    int mlHandle;
  };

  typedef BinTree<cBitmapRect>    tRectTree;
  typedef BinTreeNode<cBitmapRect>  tRectTreeNode;
  typedef astl::list<tRectTreeNode*>  tRectTreeNodeList;
  typedef tRectTreeNodeList::iterator tRectTreeNodeListIt;
  typedef astl::vector<sRecti>   tRectVec;

 public:
  cImagePacker(iGraphics* apGraphics,
               tU32 anWidth, tU32 anHeight,
               tBool abUseMipMaps,
               const char* aaszBaseName,
               const char* aaszPxf);
  ~cImagePacker();

  void __stdcall Clear();
  void __stdcall Invalidate();
  tBool __stdcall IsOK() const;

  //// iBitmapPacker ////////////////////////////////////////
  sRecti __stdcall InsertRect(const sRecti& aRect, const int aPadding) {
    return _DoInsert(aRect,NULL,eFalse,aPadding);
  }
  sRecti __stdcall InsertBitmap(iBitmap2D *apSrc, tBool abFiltered, const int aPadding) {
    return _DoInsert(sRecti(0,0,apSrc->GetWidth(),apSrc->GetHeight()),apSrc,abFiltered,aPadding);
  }
  tBool __stdcall GetIsFull() const;
  tU32 __stdcall GetWidth() const;
  tU32 __stdcall GetHeight() const;
  tU32 __stdcall GetNumMipMaps() const;
  iBitmap2D* __stdcall _GetSourceBitmap() const {
    return mptrImage->GrabBitmap(eImageUsage_Source,sRecti::Null());
  }
  iTexture* __stdcall _GetSourceTexture() const {
    return mptrImage->GrabTexture(eImageUsage_Source,sRecti::Null());
  }
  iImage* __stdcall GetImage() const {
    return mptrImage;
  }
  tU32 __stdcall GetNumRects() const { return mvRects.size(); }
  sRecti __stdcall GetRect(tU32 anIndex) const {
    if (anIndex >= mvRects.size()) return sRecti::Null();
    return mvRects[anIndex];
  }
  //// iBitmapPacker ////////////////////////////////////////

  iBitmap2D* __stdcall _GetTargetBitmap(const sRecti& aRect) {
    return mptrImage->GrabBitmap(eImageUsage_Target,aRect);
  }

 private:
  sRecti _DoInsert(const sRecti& aRect, iBitmap2D* apSrc, tBool abFiltered, const int aPadding);

  tBool MinimumFit(sRecti aSrc,sRecti aDest);

  void DrawOne(iBitmap2D* apBmp, const sVec2i& aDestPos);
  void DrawBitmap(iBitmap2D* apBmp, const sVec2i& aDestPos, tBool abFiltered);

  bool BoxFit(const sRecti& aRectSrc, const sRecti& aRectDest)
  {
    //check is size is smaller and doesn't overlap
    if ((aRectSrc.GetWidth()  > aRectDest.GetWidth()) ||
        (aRectSrc.GetHeight() > aRectDest.GetHeight()) ||
        (aRectSrc.x+aRectSrc.GetWidth() > aRectDest.x+aRectDest.GetWidth()) ||
        (aRectSrc.y+aRectSrc.GetHeight()> aRectDest.y+aRectDest.GetHeight()))
      return false;

    //check if x,y is in borders
    if (aRectSrc.x  < aRectDest.x ||
        aRectSrc.y  < aRectDest.y ||
        aRectSrc.x  > aRectDest.x+aRectDest.GetWidth() ||
        aRectSrc.y  > aRectDest.y+aRectDest.GetHeight())
      return false;

    return true;
  }

 public:
  Ptr<iImage>  mptrImage;
  tRectTree      mRects;
  tRectVec       mvRects;
  tU32           mnNumMipMaps;
  tI32           mlMinHole;
  tBool          mbIsFull;
};

#endif // __IMAGEPACKER_H_6D6895F2_4AE0_46E7_AC89_A2871D2C87FC__
