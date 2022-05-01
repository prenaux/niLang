#ifndef __MATHRECTANGLE_26862902_H__
#define __MATHRECTANGLE_26862902_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Math
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
//! Rect template class.
template <typename T>
struct sRect : public sVec4<T>
{
  sRect();
  sRect(const sVec4<T>& avVec);
  explicit sRect(T aLeft, T aTop, T aWidth = 0, T aHeight = 0);
  explicit sRect(const sVec2<T>& aTopLeft, const sVec2<T>& aBottomRight);
  explicit sRect(const sVec2<T>& aTopLeft, T aWidth = 0, T aHeight = 0);

  //! Set the Top Left corner and the size of the rectangle.
  inline void __stdcall Set(T aLeft, T aTop, T aWidth = 0, T aHeight = 0);
  //! Set the Top Left corner and the size of the rectangle.
  inline void __stdcall Set(const sVec2<T>& aTopLeft, T aWidth, T aHeight);

  //! Set the rectangle corners.
  inline void __stdcall SetCorners(const sVec2<T>& aTopLeft, const sVec2<T>& aBottomRight);
  //! Set a rectangle corner. Adapt the others accordingly.
  inline void __stdcall SetCorner(eRectCorners aCorner, const sVec2<T>& aCornerPos);
  //! Get a rectangle corner.
  inline sVec2<T> __stdcall GetCorner(eRectCorners aCorner) const;
  //! Set the Top Left corner.
  inline void __stdcall SetTopLeft(const sVec2<T>& v) { this->x = v.x; this->y = v.y; }
  //! Get the Top Left corner.
  inline sVec2<T> __stdcall GetTopLeft() const { sVec2<T> v = {this->x,this->y}; return v; }
  //! Set the Top Right corner.
  inline void __stdcall SetTopRight(const sVec2<T>& v) { this->z = v.x; this->y = v.y; }
  //! Get the Top Right corner.
  inline sVec2<T> __stdcall GetTopRight() const { sVec2<T> v = {this->z,this->y}; return v; }
  //! Set the Bottom Right corner.
  inline void __stdcall SetBottomRight(const sVec2<T>& v) { this->z = v.x; this->w = v.y; }
  //! Get the Bottom Right corner.
  inline sVec2<T> __stdcall GetBottomRight() const { sVec2<T> v = {this->z,this->w}; return v; }
  //! Set the Bottom Left corner.
  inline void __stdcall SetBottomLeft(const sVec2<T>& v) { this->x = v.x; this->w = v.y; }
  //! Get the Bottom Left corner.
  inline sVec2<T> __stdcall GetBottomLeft() const { sVec2<T> v = {this->x,this->w}; return v; }

  //! Set the Left edge.
  inline void __stdcall SetLeft(T aV);
  //! Get the Left edge.
  inline T __stdcall GetLeft() const;
  //! Set the Right edge.
  inline void __stdcall SetRight(T aV);
  //! Get the Right edge.
  inline T __stdcall GetRight() const;
  //! Set the Top edge.
  inline void __stdcall SetTop(T aV);
  //! Get the Top edge.
  inline T __stdcall GetTop() const;
  //! Set the Bottom edge.
  inline void __stdcall SetBottom(T aV);
  //! Get the Bottom edge.
  inline T __stdcall GetBottom() const;

  //! Set the width of the rectangle.
  inline void __stdcall SetWidth(T aWidth);
  //! Get the width of the rectangle.
  inline T __stdcall GetWidth() const;
  //! Set the height of the rectangle.
  inline void __stdcall SetHeight(T aHeight);
  //! Get the height of the rectangle.
  inline T __stdcall GetHeight() const;

  //! Set the width and height of the rectangle.
  inline void __stdcall SetSize(T aWidth, T aHeight);
  //! Set the size of the rectangle.
  inline void __stdcall SetSize(const sVec2<T>& aSize);
  //! Get the size of the rectangle.
  inline sVec2<T> __stdcall GetSize() const;

  //! Move the rectangle of the given offsets (relative movement).
  inline void __stdcall Move(const sVec2<T>& aMoveOffset);
  //! Move the rectangle to the given position (absolute movement).
  //! \param aPos indicates where to move to.
  //! \param aCorner is indicates the anchor point.
  inline void __stdcall MoveTo(const sVec2<T>& aPos, eRectCorners aCorner = eRectCorners_TopLeft);

  //! Inflate the size of the rectangle.
  //! \remark Negative number will "deflate" the rectangle.
  inline void __stdcall Inflate(const sVec2<T>& aV);

  //! Normalize the rectangle. Make sure that width and height are positive.
  inline void __stdcall Normalize();

  //! Set the center point of the rectangle.
  inline void __stdcall SetCenter(const sVec2<T>& avPos);
  //! Get the center point of the rectangle.
  inline sVec2<T> __stdcall GetCenter() const;

  //! Return eTrue if the given rectangle intersect this rectangle.
  //! \remark Intersection test doesnt include the Bottom and Right edges.
  inline tBool __stdcall IntersectRect(const sRect<T>& aRect) const;
  //! Return eTrue if the given point intersect/is inside this rectangle.
  //! \remark Intersection test doesnt include the Bottom and Right edges.
  inline tBool __stdcall Intersect(const sVec2<T>& aPoint) const;

  //! clips the rectangle passed against himself and returns the resulting rectangle
  inline sRect __stdcall ClipRect(const sRect<T>& aRect) const;

  //! Get the center of the a frame with the specified borders.
  inline sRect __stdcall ComputeFrameCenter(T Left, T Right, T Top, T Bottom) const;

  //! Get the center of the a frame with the specified borders.
  inline sRect __stdcall ComputeFrameCenter(const sVec4<T>& aFrameBorder) const {
    return ComputeFrameCenter(aFrameBorder.Left(),aFrameBorder.Right(),aFrameBorder.Top(),aFrameBorder.Bottom());
  }

  //! Get the size of the Left frame border.
  inline T __stdcall GetFrameLeftBorder(const sRect<T>& aFrame) const {
    return aFrame.Left()-this->Left();
  }
  //! Get the size of the Right frame border.
  inline T __stdcall GetFrameRightBorder(const sRect<T>& aFrame) const {
    return this->Right()-aFrame.Right();
  }
  //! Get the size of the Top frame border.
  inline T __stdcall GetFrameTopBorder(const sRect<T>& aFrame) const {
    return aFrame.Top()-this->Top();
  }
  //! Get the size of the Bottom frame border.
  inline T __stdcall GetFrameBottomBorder(const sRect<T>& aFrame) const {
    return this->Bottom()-aFrame.Bottom();
  }
  //! Return a vector that contains the border sizes.
  inline sVec4<T> __stdcall GetFrameBorder(const sRect<T>& aFrame) const {
    return Vec4<T>(GetFrameLeftBorder(aFrame),
                   GetFrameTopBorder(aFrame),
                   GetFrameRightBorder(aFrame),
                   GetFrameBottomBorder(aFrame));
  }

  //! += operator, move the rectangle.
  inline sRect& __stdcall operator += (const sVec2<T>& v);
  //! += operator, move the rectangle.
  inline sRect __stdcall operator + (const sVec2<T>& v) const;

  //! -= operator, move the rectangle.
  inline sRect& __stdcall operator -= (const sVec2<T>& v);
  //! -= operator, move the rectangle.
  inline sRect __stdcall operator - (const sVec2<T>& v) const;

  //! = operator, move the rectangle to the specified position, Top Left is the anchor corner.
  inline sRect& __stdcall operator = (const sVec2<T>& v);

  static inline const sRect& __stdcall Null() {
    static sRect _v(0,0);
    return _v;
  }

  inline sRect<tI32> __stdcall ToInt() const {
    return sRect<tI32>((tI32)GetLeft(),(tI32)GetTop(),
                       (tU32)GetWidth(),(tI32)GetHeight());
  }
  inline sRect<tF32> __stdcall ToFloat() const {
    return sRect<tF32>((tF32)GetLeft(),(tF32)GetTop(),
                       (tF32)GetWidth(),(tF32)GetHeight());
  }

  sRect<T> FitInto(const sRect<T>& aDestRect, const tBool abCenter = ni::eTrue) const {
    sRect<T> rect = aDestRect;
    const sVec2<T> imgSize = {
      this->GetWidth(),
      this->GetHeight()
    };
    sVec2<T> size = imgSize;
    const T rectRatio = ni::FDiv(rect.GetHeight(), rect.GetWidth());
    const T imgRatio = ni::FDiv(size.y,size.x);
    size *= (imgRatio < rectRatio) ?
        (rect.GetWidth() / size.x) :
        (rect.GetHeight() / size.y);
    T xcenter = 0, ycenter = 0;
    if (abCenter) {
      xcenter = ((rect.GetWidth() - size.x) * ((T)(0.5)));
      ycenter = ((rect.GetHeight() - size.y) * ((T)(0.5)));
    }
    return sRect<T>(rect.Left()+xcenter,rect.Top()+ycenter,size.x,size.y);
  }

  const tBool IsNull() const {
    return *this == sRect<T>::Null();
  }

  void Add(const sVec2<T>& aTL, const sVec2<T>& aBR) {
    if (this->IsNull()) {
      *this = sRect<T>(aTL, aBR);
    }
    else {
      sVec4<T> rect = {
        Min(this->x, aTL.x),
        Min(this->y, aTL.y),
        Max(this->z, aBR.x),
        Max(this->w, aBR.y),
      };
      *this = rect;
    }
  }

  void Add(const sRect<T>& aRight) {
    this->Add(aRight.GetTopLeft(), aRight.GetBottomRight());
  }
};

//! Int Rect.
typedef sRect<tI32> sRecti;
//! Float Rect.
typedef sRect<tF32> sRectf;

static inline sRectf Rectf(tF32 x, tF32 y, tF32 w, tF32 h) {
  return sRectf(x,y,w,h);
}
static inline sRectf Rectf(tF32 w = 0, tF32 h = 0) {
  return sRectf(0,0,w,h);
}
static inline sRectf Rectf(tF32 x, tF32 y, const sVec2f& aSize) {
  return sRectf(x,y,aSize.x,aSize.y);
}
static inline sRectf Rectf(const sVec2f& aPos, const sVec2f& aSize) {
  return sRectf(aPos.x,aPos.y,aSize.x,aSize.y);
}

static inline sRecti Recti(tI32 x, tI32 y, tI32 w, tI32 h) {
  return sRecti(x,y,w,h);
}
static inline sRecti Recti(tI32 w = 0, tI32 h = 0) {
  return sRecti(0,0,w,h);
}
static inline sRecti Recti(tI32 x, tI32 y, const sVec2i& aSize) {
  return sRecti(x,y,aSize.x,aSize.y);
}
static inline sRecti Recti(const sVec2i& aPos, const sVec2i& aSize) {
  return sRecti(aPos.x,aPos.y,aSize.x,aSize.y);
}

template <typename T>
static inline sRect<T> CenterRect(const sRect<T>& aRect, const sVec2<T>& aSize) {
  sRect<T> r = sRect<T>::Null();
  r.SetSize(aSize);
  r.MoveTo(Vec2<T>((aRect.GetWidth() - r.GetWidth()) / 2,
                   (aRect.GetHeight() - r.GetHeight()) / 2) +
           aRect.GetTopLeft());
  return r;
}

//--------------------------------------------------------------------------------------------
//
//
//
//--------------------------------------------------------------------------------------------
///////////////////////////////////////////////
template<typename T>
inline sRect<T>::sRect() {
}

///////////////////////////////////////////////
template<typename T>
sRect<T>::sRect(const sVec4<T>& avVec) {
  this->Left() = avVec.Left();
  this->Right() = avVec.Right();
  this->Top() = avVec.Top();
  this->Bottom() = avVec.Bottom();
}

///////////////////////////////////////////////
template<typename T>
inline sRect<T>::sRect(const sVec2<T>& aTopLeft, const sVec2<T>& aBottomRight)
{
  SetCorners(aTopLeft, aBottomRight);
}

///////////////////////////////////////////////
template<typename T>
inline sRect<T>::sRect(T aLeft, T aTop, T aWidth, T aHeight)
{
  Set(aLeft, aTop, aWidth, aHeight);
}

///////////////////////////////////////////////
template<typename T>
inline sRect<T>::sRect(const sVec2<T>& aTopLeft, T aWidth, T aHeight)
{
  Set(aTopLeft, aWidth, aHeight);
}

///////////////////////////////////////////////
template<typename T>
inline void sRect<T>::Set(const sVec2<T>& aTopLeft, T aWidth, T aHeight)
{
  this->SetTopLeft(aTopLeft);
  this->Right() = this->Left() + aWidth;
  this->Bottom() = this->Top() + aHeight;
}

///////////////////////////////////////////////
template<typename T>
inline void sRect<T>::Set(T aLeft, T aTop, T aWidth, T aHeight)
{
  this->Left() = aLeft;
  this->Top() = aTop;
  this->Right() = this->Left() + aWidth;
  this->Bottom() = this->Top() + aHeight;
}

///////////////////////////////////////////////
template<typename T>
inline void sRect<T>::SetCorners(const sVec2<T>& aTopLeft, const sVec2<T>& aBottomRight)
{
  this->SetTopLeft(aTopLeft);
  this->SetBottomRight(aBottomRight);
}

///////////////////////////////////////////////
template<typename T>
void sRect<T>::SetCorner(eRectCorners aCorner, const sVec2<T>& aCornerPos)
{
  switch (aCorner) {
    case eRectCorners_TopLeft: this->SetTopLeft(aCornerPos); break;
    case eRectCorners_TopRight: this->SetTopRight(aCornerPos); break;
    case eRectCorners_BottomRight: this->SetBottomRight(aCornerPos); break;
    case eRectCorners_BottomLeft: this->SetBottomLeft(aCornerPos); break;
    default: break;
  }
}

///////////////////////////////////////////////
template<typename T>
sVec2<T> sRect<T>::GetCorner(eRectCorners aCorner) const
{
  switch (aCorner) {
    case eRectCorners_TopLeft:   return this->GetTopLeft();
    case eRectCorners_TopRight:  return this->GetTopRight();
    case eRectCorners_BottomRight: return this->GetBottomRight();
    case eRectCorners_BottomLeft:  return this->GetBottomLeft();
    default: break;
  }
  return Vec2<T>(0,0);
}

///////////////////////////////////////////////
template<typename T>
inline void sRect<T>::SetLeft(T aV) {
  this->Left() = aV;
}
template<typename T>
inline T sRect<T>::GetLeft() const {
  return this->Left();
}

///////////////////////////////////////////////
template<typename T>
inline void sRect<T>::SetRight(T aV) {
  this->Right() = aV;
}
template<typename T>
inline T sRect<T>::GetRight() const {
  return this->Right();
}

///////////////////////////////////////////////
template<typename T>
inline void sRect<T>::SetTop(T aV) {
  this->Top() = aV;
}
template<typename T>
inline T sRect<T>::GetTop() const {
  return this->Top();
}

///////////////////////////////////////////////
template<typename T>
inline void sRect<T>::SetBottom(T aV) {
  this->Bottom() = aV;
}
template<typename T>
inline T sRect<T>::GetBottom() const {
  return this->Bottom();
}

///////////////////////////////////////////////
template<typename T>
void sRect<T>::SetWidth(T aWidth) {
  this->Right() = this->Left() + aWidth;
}
template<typename T>
T sRect<T>::GetWidth() const {
  return this->Right() - this->Left();
}

///////////////////////////////////////////////
template<typename T>
void sRect<T>::SetHeight(T aHeight) {
  this->Bottom() = this->Top() + aHeight;
}
template<typename T>
T sRect<T>::GetHeight() const {
  return this->Bottom() - this->Top();
}

///////////////////////////////////////////////
template<typename T>
inline void sRect<T>::SetSize(T aWidth, T aHeight) {
  SetWidth(aWidth);
  SetHeight(aHeight);
}
template<typename T>
inline void sRect<T>::SetSize(const sVec2<T>& aSize) {
  SetWidth(aSize.x);
  SetHeight(aSize.y);
}
template<typename T>
inline sVec2<T> sRect<T>::GetSize() const {
  return Vec2<T>(GetWidth(),GetHeight());
}

///////////////////////////////////////////////
template<typename T>
inline void sRect<T>::Move(const sVec2<T>& aMoveOffset)
{
  this->Left() += aMoveOffset.x;
  this->Top() += aMoveOffset.y;
  this->Right() += aMoveOffset.x;
  this->Bottom() += aMoveOffset.y;
}

///////////////////////////////////////////////
template<typename T>
inline void sRect<T>::MoveTo(const sVec2<T>& aPos, eRectCorners aCorner)
{
  Normalize();
  T width = GetWidth();
  T height = GetHeight();
  SetCorner(aCorner, aPos);
  switch (aCorner) {
    case eRectCorners_TopLeft: {
      SetSize(width, height);
      break;
    }
    case eRectCorners_TopRight: {
      SetSize(-width, height);
      break;
    }
    case eRectCorners_BottomRight: {
      SetSize(-width, -height);
      break;
    }
    case eRectCorners_BottomLeft: {
      SetSize(width, -height);
      break;
    }
    default:
      break;
  }
  Normalize();
}

///////////////////////////////////////////////
template<typename T>
inline void __stdcall sRect<T>::Inflate(const sVec2<T>& aV)
{
  SetSize(GetWidth()+aV.x,GetHeight()+aV.y);
}

///////////////////////////////////////////////
template<typename T>
inline void sRect<T>::Normalize()
{
  if (this->Left() > this->Right())
    ni::Swap(this->Left(), this->Right());

  if (this->Top() > this->Bottom())
    ni::Swap(this->Top(), this->Bottom());
}

///////////////////////////////////////////////
template<typename T>
inline void __stdcall sRect<T>::SetCenter(const sVec2<T>& avPos)
{
  MoveTo(Vec2<T>(avPos.x-(GetWidth()/2),avPos.y-(GetHeight()/2)));
}

///////////////////////////////////////////////
template<typename T>
inline sVec2<T> sRect<T>::GetCenter() const
{
  return Vec2<T>(this->Left()+(GetWidth()/2), this->Top()+(GetHeight()/2));
}

///////////////////////////////////////////////
template<typename T>
inline tBool sRect<T>::IntersectRect(const sRect<T>& aRect) const
{
  if (aRect.Right() < this->Left()) return eFalse;
  if (aRect.Bottom() < this->Top()) return eFalse;
  if (aRect.Left() > this->Right()) return eFalse;
  if (aRect.Top() > this->Bottom())  return eFalse;
  return eTrue;
}

///////////////////////////////////////////////
template<typename T>
inline tBool sRect<T>::Intersect(const sVec2<T>& aPoint) const
{
  if (aPoint.x >= this->Left() && aPoint.x < this->Right() &&
      aPoint.y >= this->Top() && aPoint.y < this->Bottom())
    return eTrue;

  return eFalse;
}

///////////////////////////////////////////////
template<typename T>
inline sRect<T> sRect<T>::ClipRect(const sRect<T>& aRect) const
{
  sRect<T> ret;
  ret.Left() = aRect.Left()<this->Left()?this->Left():
      (aRect.Left()>this->Right())?this->Right():aRect.Left();
  ret.Right()=aRect.Right()<this->Left()?this->Left():
      (aRect.Right()>this->Right())?this->Right():aRect.Right();
  ret.Top()=aRect.Top()<this->Top()?this->Top():
      (aRect.Top()>this->Bottom())?this->Bottom():aRect.Top();
  ret.Bottom()=aRect.Bottom()<this->Top()?this->Top():
      (aRect.Bottom()>this->Bottom())?this->Bottom():aRect.Bottom();
  return ret;
}

///////////////////////////////////////////////
template<typename T>
inline sRect<T> __stdcall sRect<T>::ComputeFrameCenter(T aLeft, T aRight, T aTop, T aBottom) const
{
  sRect<T> r;
  r.Left() = this->Left()+aLeft;
  r.Right() = this->Right()-aRight;
  r.Top() = this->Top()+aTop;
  r.Bottom() = this->Bottom()-aBottom;
  return r;
}

///////////////////////////////////////////////
template<typename T>
inline sRect<T>& sRect<T>::operator += (const sVec2<T>& v)
{
  this->Left() += v.x;
  this->Top() += v.y;
  this->Right() += v.x;
  this->Bottom() += v.y;
  return *this;
}

///////////////////////////////////////////////
template<typename T>
inline sRect<T> sRect<T>::operator + (const sVec2<T>& v) const
{
  return sRect<T>(this->GetTopLeft()+v, this->GetBottomRight()+v);
}

///////////////////////////////////////////////
template<typename T>
inline sRect<T>& sRect<T>::operator -= (const sVec2<T>& v)
{
  this->Left() -= v.x;
  this->Top() -= v.y;
  this->Right() -= v.x;
  this->Bottom() -= v.y;
  return *this;
}

///////////////////////////////////////////////
template<typename T>
inline sRect<T> sRect<T>::operator - (const sVec2<T>& v) const
{
  return sRect<T>(this->GetTopLeft()-v, this->GetBottomRight()-v);
}

///////////////////////////////////////////////
template<typename T>
inline sRect<T>& sRect<T>::operator = (const sVec2<T>& v)
{
  MoveTo(v);
  return *this;
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __MATHRECTANGLE_26862902_H__
