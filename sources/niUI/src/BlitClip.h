
template <typename T>
inline tBool ClipDim(T& p, T& s, const T aP, const T aSz, const T aMaxSz) {
  p = aP; s = aSz==0?aMaxSz:aSz;
  if (p >= aMaxSz) return eFalse; // outsize of size
  if (p < 0) {
    T d = 0-p;
    p = 0;
    s -= d;
  }
  if (p+s > aMaxSz) {
    T d = (p+s)-aMaxSz;
    s -= d;
  }
  if (s <= 0) return eFalse;
  return eTrue;
}

template <typename T>
inline tBool ClipBlitRect(sRecti& aClipped, T* apSrc, const tU32 anLevel, const sRecti& aRect) {
  tU32 nWidth = apSrc->GetWidth();
  tU32 nHeight = apSrc->GetHeight();
  if (anLevel != eInvalidHandle && anLevel != 0) {
    nWidth = ni::Max(nWidth>>anLevel,1);
    nHeight = ni::Max(nHeight>>anLevel,1);
  }
  sRecti clipBounds(0,0,nWidth,nHeight);

  sRecti toClip(aRect.GetTopLeft().x, aRect.GetTopLeft().y,
                     (aRect.GetWidth() == 0) ? nWidth : aRect.GetWidth(),
                     (aRect.GetHeight() == 0) ? nHeight : aRect.GetHeight());
  if (!toClip.IntersectRect(clipBounds))
    return eFalse;

  aClipped = toClip.ClipRect(clipBounds);
  if (aClipped.GetWidth() <= 0 || aClipped.GetHeight() <= 0)
    return eFalse;

  return eTrue;
}

template <typename T>
inline tBool ClipBlitRectf(sRectf& aClipped, T* apSrc, const tU32 anLevel, const sRectf& aRect) {
  tU32 nWidth = apSrc->GetWidth();
  tU32 nHeight = apSrc->GetHeight();
  if (anLevel != eInvalidHandle && anLevel != 0) {
    nWidth = ni::Max(nWidth>>anLevel,1);
    nHeight = ni::Max(nHeight>>anLevel,1);
  }
  sRectf clipBounds(0,0,(tF32)nWidth,(tF32)nHeight);

  sRectf toClip(aRect.GetTopLeft().x, aRect.GetTopLeft().y,
                     (aRect.GetWidth() == 0) ? nWidth : aRect.GetWidth(),
                     (aRect.GetHeight() == 0) ? nHeight : aRect.GetHeight());
  if (!toClip.IntersectRect(clipBounds))
    return eFalse;

  aClipped = toClip.ClipRect(clipBounds);
  if (aClipped.GetWidth() <= 0 || aClipped.GetHeight() <= 0)
    return eFalse;

  return eTrue;
}

template <typename T>
inline tBool ClipBlitBox(sVec3i& aClPos, sVec3i& aClSz,
                         T* apSrc, const tU32 anLevel,
                         const sVec3i& aPos, const sVec3i& aSz) {
  tI32 nWidth = (tI32)apSrc->GetWidth();
  tI32 nHeight = (tI32)apSrc->GetHeight();
  tI32 nDepth = (tI32)apSrc->GetDepth();
  if (anLevel != eInvalidHandle && anLevel != 0) {
    nWidth = ni::Max(nWidth>>anLevel,1);
    nHeight = ni::Max(nHeight>>anLevel,1);
    nDepth = ni::Max(nDepth>>anLevel,1);
  }

  if (!ClipDim(aClPos.x,aClSz.x,aPos.x,aSz.x,nWidth)) return eFalse;
  if (!ClipDim(aClPos.y,aClSz.y,aPos.y,aSz.y,nHeight)) return eFalse;
  if (!ClipDim(aClPos.z,aClSz.z,aPos.z,aSz.z,nDepth)) return eFalse;

  return eTrue;
}
