#ifndef __WIDGETZMAP_57626044_H__
#define __WIDGETZMAP_57626044_H__
#pragma once
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

class cWidget;

typedef astl::list<cWidget*>          tWidgetPtrDeq;
typedef tWidgetPtrDeq::iterator         tWidgetPtrDeqIt;
typedef tWidgetPtrDeq::const_iterator     tWidgetPtrDeqCIt;
typedef tWidgetPtrDeq::reverse_iterator     tWidgetPtrDeqRIt;
typedef tWidgetPtrDeq::const_reverse_iterator tWidgetPtrDeqCRIt;

typedef astl::map<tU32,tWidgetPtrDeq> tWidgetZMap;
typedef tWidgetZMap::iterator     tWidgetZMapIt;
typedef tWidgetZMap::const_iterator   tWidgetZMapCIt;
typedef tWidgetZMap::reverse_iterator     tWidgetZMapRIt;
typedef tWidgetZMap::const_reverse_iterator   tWidgetZMapCRIt;

struct iWidgetZMap : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetZMap,0xf265b538,0xe1bc,0x4811,0xbe,0xf3,0x0f,0xae,0x0b,0x13,0x6d,0xea);
  virtual tBool __stdcall RemoveOfZMap(cWidget* apW) = 0;
  virtual void __stdcall SetZOrder(cWidget* apW, eWidgetZOrder aZOrder) = 0;
  virtual void __stdcall SetZOrderAbove(cWidget* apW, cWidget* apAbove) = 0;
};

class cWidgetZMap : public cIUnknownImpl<iWidgetZMap,eIUnknownImplFlags_NoRefCount>
{
 public:
  cWidgetZMap() {
    mnTouchCount = 0;
  }

  ///////////////////////////////////////////////
  void Clear() {
    mmapZOrder.clear();
  }

  ///////////////////////////////////////////////
  tBool IsEmpty() const {
    return mmapZOrder.empty();
  }

  ///////////////////////////////////////////////
  cWidget* GetTargetByPos(const sVec2f &pos,
                          tBool (*apfnExcludeWidget)(const cWidget* w, tIntPtr apUserData),
                          tIntPtr apUserData) const;

  ///////////////////////////////////////////////
  void AddToZMap(cWidget* apW, eWidgetZOrder aZOrder)
  {
    tBool bBottom = eFalse;
    tU32 nMap = 0;
    switch (aZOrder) {
      case eWidgetZOrder_BackgroundBottom: bBottom = eTrue;
      case eWidgetZOrder_Background:
        {
          nMap = 3;
          break;
        }
      case eWidgetZOrder_Bottom: bBottom = eTrue;
      case eWidgetZOrder_Top:
        {
          nMap = 2;
          break;
        }
      case eWidgetZOrder_TopMostBottom: bBottom = eTrue;
      case eWidgetZOrder_TopMost:
        {
          nMap = 1;
          break;
        }
      case eWidgetZOrder_OverlayBottom: bBottom = eTrue;
      case eWidgetZOrder_Overlay:
        {
          nMap = 0;
          break;
        }
    }

    tWidgetZMapIt it = mmapZOrder.find(nMap);
    if (it == mmapZOrder.end()) {
      tWidgetPtrDeq deq;
      deq.push_back(apW);
      astl::upsert(mmapZOrder,nMap,deq);
    }
    else {
      if (bBottom)  it->second.push_front(apW);
      else      it->second.push_back(apW);
    }

    ++mnTouchCount;
  }

  ///////////////////////////////////////////////
  void __stdcall SetZOrder(cWidget* apW, eWidgetZOrder aZOrder)
  {
    RemoveOfZMap(apW);
    AddToZMap(apW,aZOrder);
  }

  ///////////////////////////////////////////////
  void __stdcall SetZOrderAbove(cWidget* apW, cWidget* apAbove)
  {
    RemoveOfZMap(apW);
    for (tWidgetZMapIt it = mmapZOrder.begin(); it != mmapZOrder.end(); ++it) {
      for (tWidgetPtrDeqIt lit = it->second.begin(); lit != it->second.end(); ++lit)
      {
        if (*lit == (cWidget*)apAbove) {
          ++lit;
          it->second.insert(lit,apW);
          ++mnTouchCount;
          return;
        }
      }
    }
  }

  ///////////////////////////////////////////////
  tBool __stdcall RemoveOfZMap(cWidget* apW)
  {
    for (tWidgetZMapIt it = mmapZOrder.begin(); it != mmapZOrder.end(); ++it) {
      for (tWidgetPtrDeqIt lit = it->second.begin(); lit != it->second.end(); ++lit)
      {
        if (*lit == apW) {
          it->second.erase(lit);
          if (it->second.empty())
            mmapZOrder.erase(it);
          ++mnTouchCount;
          return eTrue;
        }
      }
    }
    return eFalse;
  }

  ///////////////////////////////////////////////
  tBool HasWidget(cWidget* apW) const {
    for (tWidgetZMapCIt it = mmapZOrder.begin(); it != mmapZOrder.end(); ++it) {
      for (tWidgetPtrDeqCIt lit = it->second.begin(); lit != it->second.end(); ++lit) {
        if (*lit == apW) {
          return eTrue;
        }
      }
    }
    return eFalse;
  }

  ///////////////////////////////////////////////
  ni::tU32 GetNumDrawable() const
  {
    tU32 nCount = 0;
    for (tWidgetZMapCRIt zit = mmapZOrder.rbegin(); zit != mmapZOrder.rend(); ++zit) {
      nCount += zit->second.size();
    }
    return nCount;
  }

  ///////////////////////////////////////////////
  tU32 GetDrawOrder(const iWidget* apW) const
  {
    tU32 nCount = 0;

    for (tWidgetZMapCRIt zit = mmapZOrder.rbegin(); zit != mmapZOrder.rend(); ++zit)
    {
      for (tWidgetPtrDeqCIt lit = zit->second.begin(); lit != zit->second.end(); ++lit)
      {
        if (*lit == (cWidget*)apW)
          return nCount;
        ++nCount;
      }
    }

    return eInvalidHandle;
  }

  ///////////////////////////////////////////////
  cWidget* GetFromDrawOrder(tU32 anDrawOrder) const
  {
    tU32 nCount = 0;
    for (tWidgetZMapCRIt zit = mmapZOrder.rbegin(); zit != mmapZOrder.rend(); ++zit)
    {
      for (tWidgetPtrDeqCIt lit = zit->second.begin(); lit != zit->second.end(); ++lit)
      {
        if (nCount == anDrawOrder)
          return (cWidget*)*lit;
        ++nCount;
      }
    }
    return NULL;
  }

  ///////////////////////////////////////////////
  tWidgetZMapRIt RBegin() {
    return mmapZOrder.rbegin();
  }
  tWidgetZMapCRIt RBegin() const {
    return mmapZOrder.rbegin();
  }
  tWidgetZMapRIt REnd() {
    return mmapZOrder.rend();
  }
  tWidgetZMapCRIt REnd() const {
    return mmapZOrder.rend();
  }

  ///////////////////////////////////////////////
  void GetReverseList(astl::list<cWidget*>& aOut) const {
    for (tWidgetZMapCRIt zit = mmapZOrder.rbegin(); zit != mmapZOrder.rend(); ++zit) {
      for (tWidgetPtrDeqCIt lit = zit->second.begin(); lit != zit->second.end(); ++lit) {
        cWidget* w = (cWidget*)*lit;
        aOut.push_back(w);
      }
    }
  }

  ///////////////////////////////////////////////
  void GetList(astl::list<cWidget*>& aOut) const {
    for (tWidgetZMapCIt zit = mmapZOrder.begin(); zit != mmapZOrder.end(); ++zit) {
      for (tWidgetPtrDeqCIt lit = zit->second.begin(); lit != zit->second.end(); ++lit) {
        cWidget* w = (cWidget*)*lit;
        aOut.push_back(w);
      }
    }
  }

  ///////////////////////////////////////////////
  tU32 __stdcall GetTouchCount() const {
    return mnTouchCount;
  }

 private:
  tWidgetZMap mmapZOrder;
  tU32 mnTouchCount;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __WIDGETZMAP_57626044_H__
