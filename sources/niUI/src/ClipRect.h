#ifndef __CLIPRECT_H_1E0ED548_EA92_482F_A8CE_ABEB2D9A57C8__
#define __CLIPRECT_H_1E0ED548_EA92_482F_A8CE_ABEB2D9A57C8__

static inline tBool _ClipBlitDestRectf(sRectf& aDestClipped,
                                       const sRectf& aDestRect,
                                       const sRectf& aClipRect)
{
  // Clip destination
  sRectf toClip = aDestRect;
  if (!toClip.IntersectRect(aClipRect))
    return eFalse;

  toClip = toClip.ClipRect(aClipRect);
  if (toClip.GetWidth() <= 0 || toClip.GetHeight() <= 0)
    return eFalse;

  // Copy to dest rect
  aDestClipped = toClip;
  return eTrue;
}

static inline tBool _ClipBlitDestRecti(sRecti& aDestClipped,
                                       const sRectf& aDestRect,
                                       const sRectf& aClipRect)
{
  sRectf d;
  if (!_ClipBlitDestRectf(d,aDestRect,aClipRect))
    return eFalse;
  aDestClipped = d.ToInt();
  return eTrue;
}

static inline tBool _ClipBlitRectf(sRectf& aDestClipped,
                                   const sRectf& aDestRect,
                                   sRectf& aSrcClipped,
                                   const sRectf& aSrcRect,
                                   const sRectf& aClipRect)
{
  // Clip destination
  sRectf toClip = aDestRect;
  if (!toClip.IntersectRect(aClipRect))
    return eFalse;

  toClip = toClip.ClipRect(aClipRect);
  if (toClip.GetWidth() <= 0 || toClip.GetHeight() <= 0)
    return eFalse;

  // Adjust source rect...
  tF32 diffPosX = toClip.x - aDestRect.x;
  tF32 diffPosY = toClip.y - aDestRect.y;
  aSrcClipped.SetTopLeft(Vec2<tF32>(
      aSrcRect.x + diffPosX,
      aSrcRect.y + diffPosY));
  tF32 diffSizeX = toClip.GetWidth() / aDestRect.GetWidth();
  tF32 diffSizeY = toClip.GetHeight() / aDestRect.GetHeight();
  aSrcClipped.SetSize(
      aSrcRect.GetWidth() * diffSizeX,
      aSrcRect.GetHeight() * diffSizeY);

  // Copy to dest rect
  aDestClipped = toClip;
  return eTrue;
}

static inline tBool _ClipBlitRecti(sRecti& aDestClipped,
                                   const sRectf& aDestRect,
                                   sRecti& aSrcClipped,
                                   const sRectf& aSrcRect,
                                   const sRectf& aClipRect)
{
  sRectf d,s;
  if (!_ClipBlitRectf(d,aDestRect,s,aSrcRect,aClipRect))
    return eFalse;
  aDestClipped = d.ToInt();
  aSrcClipped = s.ToInt();
  return eTrue;
}

static tBool _BlitBlend(iBitmap2D* apDestBmp,
                        const sRectf& aDestRect,
                        iBitmap2D* apSrcBmp,
                        const sRectf& aSrcRect,
                        const sRectf& aClipRect,
                        eBlendMode aBlendMode,
                        const sColor4f& colFront,
                        const sColor4f& colBack = sColor4f::White())
{
  sRecti rectDst, rectSrc;
  if (_ClipBlitRecti(rectDst,aDestRect,rectSrc,aSrcRect,aClipRect)) {
    if (rectDst.GetSize() != rectSrc.GetSize()) {
      if (aBlendMode != eBlendMode_NoBlending) {
        return apDestBmp->BlitAlphaStretch(
            apSrcBmp,
            rectSrc.Left(),
            rectSrc.Top(),
            rectDst.Left(),
            rectDst.Top(),
            rectSrc.GetWidth(),
            rectSrc.GetHeight(),
            rectDst.GetWidth(),
            rectDst.GetHeight(),
            colFront,
            colBack,
            eBlendMode_Translucent);
      }
      else {
        return apDestBmp->BlitStretch(
            apSrcBmp,
            rectSrc.Left(),
            rectSrc.Top(),
            rectDst.Left(),
            rectDst.Top(),
            rectSrc.GetWidth(),
            rectSrc.GetHeight(),
            rectDst.GetWidth(),
            rectDst.GetHeight());
      }
    }
    else {
      if (aBlendMode != eBlendMode_NoBlending) {
        return apDestBmp->BlitAlpha(
            apSrcBmp,
            rectSrc.Left(),
            rectSrc.Top(),
            rectDst.Left(),
            rectDst.Top(),
            rectSrc.GetWidth(),
            rectSrc.GetHeight(),
            ePixelFormatBlit_Normal,
            colFront,
            colBack,
            eBlendMode_Translucent);
      }
      else {
        return apDestBmp->Blit(
            apSrcBmp,
            rectSrc.Left(),
            rectSrc.Top(),
            rectDst.Left(),
            rectDst.Top(),
            rectSrc.GetWidth(),
            rectSrc.GetHeight(),
            ePixelFormatBlit_Normal);
      }
    }
  }

  return eTrue;
}

static __forceinline tBool _BlitBlend(iBitmap2D* apDestBmp,
                                      const sRectf& aDestRect,
                                      iBitmap2D* apSrcBmp,
                                      const sRectf& aSrcRect,
                                      const sRectf& aClipRect,
                                      eBlendMode aBlendMode,
                                      tU32 anColor)
{
  const sColor4f colFront = {ULColorGetRf(anColor),
                             ULColorGetGf(anColor),
                             ULColorGetBf(anColor),
                             ULColorGetAf(anColor)};
  return _BlitBlend(apDestBmp,aDestRect,apSrcBmp,aSrcRect,aClipRect,aBlendMode,colFront);
}

#endif // __CLIPRECT_H_1E0ED548_EA92_482F_A8CE_ABEB2D9A57C8__
