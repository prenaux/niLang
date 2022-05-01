// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "PixelFormatStd.h"
#include <niLang/Utils/Float16.h>

#pragma niTodo("Make a template version of cPixelFormatStd to replace the mpBase thing...")

//////////////////////////////////////////////////////////////////////////////////////////////
// 8 bits pixel format

///////////////////////////////////////////////
tPtr cPixelFormatStd::s8::BuildPixelul(tPtr pOut, tU32 r, tU32 g, tU32 b, tU32 a) const
{
  *((tU8*)(pOut)) = tU8((((a>>ulARShift)&ulAMask) << ulALShift) |
                        (((r>>ulRRShift)&ulRMask) << ulRLShift) |
                        (((g>>ulGRShift)&ulGMask) << ulGLShift) |
                        (((b>>ulBRShift)&ulBMask) << ulBLShift));
  return pOut;
}

///////////////////////////////////////////////
tPtr cPixelFormatStd::s8::BuildPixelf(tPtr pOut, tF32 r, tF32 g, tF32 b, tF32 a) const
{
  *((tU8*)(pOut)) = tU8(((tU32(a*fAMul)&ulAMask) << ulALShift) |
                        ((tU32(r*fRMul)&ulRMask) << ulRLShift) |
                        ((tU32(g*fGMul)&ulGMask) << ulGLShift) |
                        ((tU32(b*fBMul)&ulBMask) << ulBLShift));
  return pOut;
}

///////////////////////////////////////////////
void cPixelFormatStd::s8::UnpackPixelf(tPtr pC, sColor4f& col) const
{
  if (ulRSigned)  col.x = tF32(tI32(((*((tU8*)(pC))>>ulRLShift)&ulRMask)<<ulRRShift))/2147483648.0f;
  else      col.x = tF32((*((tU8*)(pC))>>ulRLShift)&ulRMask)/fRMul;

  if (ulGSigned)  col.y = tF32(tI32(((*((tU8*)(pC))>>ulGLShift)&ulGMask)<<ulGRShift))/2147483648.0f;
  else      col.y = tF32((*((tU8*)(pC))>>ulGLShift)&ulGMask)/fGMul;

  if (ulBSigned)  col.z = tF32(tI32(((*((tU8*)(pC))>>ulBLShift)&ulBMask)<<ulBRShift))/2147483648.0f;
  else      col.z = tF32((*((tU8*)(pC))>>ulBLShift)&ulBMask)/fBMul;

  if (ulASigned)  col.w = tF32(tI32(((*((tU8*)(pC))>>ulALShift)&ulAMask)<<ulARShift))/2147483648.0f;
  else      col.w = tF32((*((tU8*)(pC))>>ulALShift)&ulAMask)/fAMul;
}

///////////////////////////////////////////////
void cPixelFormatStd::s8::UnpackPixelul(tPtr pC, sColor4ul& col) const
{
  col.x = ((*((tU8*)(pC))>>ulRLShift)&ulRMask)<<ulRRShift;
  col.y = ((*((tU8*)(pC))>>ulGLShift)&ulGMask)<<ulGRShift;
  col.z = ((*((tU8*)(pC))>>ulBLShift)&ulBMask)<<ulBRShift;
  col.w = ((*((tU8*)(pC))>>ulALShift)&ulAMask)<<ulARShift;
}

///////////////////////////////////////////////
void cPixelFormatStd::s8::CopyOne(tPtr pDst, tPtr pSrc) const
{
  *((tU8*)(pDst)) = *((tU8*)(pSrc));
}

//////////////////////////////////////////////////////////////////////////////////////////////
// 16 bits pixel format

///////////////////////////////////////////////
tPtr cPixelFormatStd::s16::BuildPixelul(tPtr pOut, tU32 r, tU32 g, tU32 b, tU32 a) const
{
  *((tU16*)(pOut)) = tU16((((a>>ulARShift)&ulAMask) << ulALShift) |
                          (((r>>ulRRShift)&ulRMask) << ulRLShift) |
                          (((g>>ulGRShift)&ulGMask) << ulGLShift) |
                          (((b>>ulBRShift)&ulBMask) << ulBLShift));
  return pOut;
}

///////////////////////////////////////////////
tPtr cPixelFormatStd::s16::BuildPixelf(tPtr pOut, tF32 r, tF32 g, tF32 b, tF32 a) const
{
  *((tU16*)(pOut)) = tU16(((tU32(a*fAMul)&ulAMask) << ulALShift) |
                          ((tU32(r*fRMul)&ulRMask) << ulRLShift) |
                          ((tU32(g*fGMul)&ulGMask) << ulGLShift) |
                          ((tU32(b*fBMul)&ulBMask) << ulBLShift));
  return pOut;
}

///////////////////////////////////////////////
void cPixelFormatStd::s16::UnpackPixelf(tPtr pC, sColor4f& col) const
{
  if (ulRSigned)  col.x = tF32(tI32(((*((tU16*)(pC))>>ulRLShift)&ulRMask)<<ulRRShift))/2147483648.0f;
  else      col.x = tF32((*((tU16*)(pC))>>ulRLShift)&ulRMask)/fRMul;

  if (ulGSigned)  col.y = tF32(tI32(((*((tU16*)(pC))>>ulGLShift)&ulGMask)<<ulGRShift))/2147483648.0f;
  else      col.y = tF32((*((tU16*)(pC))>>ulGLShift)&ulGMask)/fGMul;

  if (ulBSigned)  col.z = tF32(tI32(((*((tU16*)(pC))>>ulBLShift)&ulBMask)<<ulBRShift))/2147483648.0f;
  else      col.z = tF32((*((tU16*)(pC))>>ulBLShift)&ulBMask)/fBMul;

  if (ulASigned)  col.w = tF32(tI32(((*((tU16*)(pC))>>ulALShift)&ulAMask)<<ulARShift))/2147483648.0f;
  else      col.w = tF32((*((tU16*)(pC))>>ulALShift)&ulAMask)/fAMul;
}

///////////////////////////////////////////////
void cPixelFormatStd::s16::UnpackPixelul(tPtr pC, sColor4ul& col) const
{
  col.x = ((*((tU16*)(pC))>>ulRLShift)&ulRMask)<<ulRRShift;
  col.y = ((*((tU16*)(pC))>>ulGLShift)&ulGMask)<<ulGRShift;
  col.z = ((*((tU16*)(pC))>>ulBLShift)&ulBMask)<<ulBRShift;
  col.w = ((*((tU16*)(pC))>>ulALShift)&ulAMask)<<ulARShift;
}

///////////////////////////////////////////////
void cPixelFormatStd::s16::CopyOne(tPtr pDst, tPtr pSrc) const
{
  *((tU16*)(pDst)) = *((tU16*)(pSrc));
}

//////////////////////////////////////////////////////////////////////////////////////////////
// 24 bits pixel format

///////////////////////////////////////////////
tPtr cPixelFormatStd::s24::BuildPixelul(tPtr pOut, tU32 r, tU32 g, tU32 b, tU32 a) const
{
  tU32 col = ((((a>>ulARShift)&ulAMask) << ulALShift) |
              (((r>>ulRRShift)&ulRMask) << ulRLShift) |
              (((g>>ulGRShift)&ulGMask) << ulGLShift) |
              (((b>>ulBRShift)&ulBMask) << ulBLShift));
  *((tU24*)(pOut)) = *((tU24*)(&col));
  return pOut;
}

///////////////////////////////////////////////
tPtr cPixelFormatStd::s24::BuildPixelf(tPtr pOut, tF32 r, tF32 g, tF32 b, tF32 a) const
{
  tU32 col = ((tU32(a*fAMul)&ulAMask) << ulALShift) |
      ((tU32(r*fRMul)&ulRMask) << ulRLShift) |
      ((tU32(g*fGMul)&ulGMask) << ulGLShift) |
      ((tU32(b*fBMul)&ulBMask) << ulBLShift);
  *((tU24*)(pOut)) = *((tU24*)(&col));
  return pOut;
}

///////////////////////////////////////////////
void cPixelFormatStd::s24::UnpackPixelf(tPtr pC, sColor4f& col) const
{
  tU32 ulV = bit_castz<tU32>(*(tU24*)pC);

  if (ulRSigned)  col.x = tF32(tI32(((ulV>>ulRLShift)&ulRMask)<<ulRRShift))/2147483648.0f;
  else      col.x = tF32((ulV>>ulRLShift)&ulRMask)/fRMul;

  if (ulGSigned)  col.y = tF32(tI32(((ulV>>ulGLShift)&ulGMask)<<ulGRShift))/2147483648.0f;
  else      col.y = tF32((ulV>>ulGLShift)&ulGMask)/fGMul;

  if (ulBSigned)  col.z = tF32(tI32(((ulV>>ulBLShift)&ulBMask)<<ulBRShift))/2147483648.0f;
  else      col.z = tF32((ulV>>ulBLShift)&ulBMask)/fBMul;

  if (ulASigned)  col.w = tF32(tI32(((ulV>>ulALShift)&ulAMask)<<ulARShift))/2147483648.0f;
  else      col.w = tF32((ulV>>ulALShift)&ulAMask)/fAMul;
}

///////////////////////////////////////////////
void cPixelFormatStd::s24::UnpackPixelul(tPtr pC, sColor4ul& col) const
{
  tU32 ulV = bit_castz<tU32>(*(tU24*)pC);
  col.x = ((ulV>>ulRLShift)&ulRMask)<<ulRRShift;
  col.y = ((ulV>>ulGLShift)&ulGMask)<<ulGRShift;
  col.z = ((ulV>>ulBLShift)&ulBMask)<<ulBRShift;
  col.w = ((ulV>>ulALShift)&ulAMask)<<ulARShift;
}

///////////////////////////////////////////////
void cPixelFormatStd::s24::CopyOne(tPtr pDst, tPtr pSrc) const
{
  tU24 v = *((tU24*)(pSrc));
  *((tU24*)(pDst)) = v;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// 32 bits pixel format

///////////////////////////////////////////////
tPtr cPixelFormatStd::s32::BuildPixelul(tPtr pOut, tU32 r, tU32 g, tU32 b, tU32 a) const
{
  *((tU32*)(pOut)) = tU32((((a>>ulARShift) & ulAMask) << ulALShift) |
                          (((r>>ulRRShift) & ulRMask) << ulRLShift) |
                          (((g>>ulGRShift) & ulGMask) << ulGLShift) |
                          (((b>>ulBRShift) & ulBMask) << ulBLShift));
  return pOut;
}

///////////////////////////////////////////////
tPtr cPixelFormatStd::s32::BuildPixelf(tPtr pOut, tF32 r, tF32 g, tF32 b, tF32 a) const
{
  *((tU32*)(pOut)) = tU32(((tU32(a*fAMul)&ulAMask) << ulALShift) |
                          ((tU32(r*fRMul)&ulRMask) << ulRLShift) |
                          ((tU32(g*fGMul)&ulGMask) << ulGLShift) |
                          ((tU32(b*fBMul)&ulBMask) << ulBLShift));
  return pOut;
}

///////////////////////////////////////////////
void cPixelFormatStd::s32::UnpackPixelf(tPtr pC, sColor4f& col) const
{
  if (ulRSigned)  col.x = tF32(tI32(((*((tU32*)(pC))>>ulRLShift)&ulRMask)<<ulRRShift))/2147483648.0f;
  else      col.x = tF32((*((tU32*)(pC))>>ulRLShift)&ulRMask)/fRMul;

  if (ulGSigned)  col.y = tF32(tI32(((*((tU32*)(pC))>>ulGLShift)&ulGMask)<<ulGRShift))/2147483648.0f;
  else      col.y = tF32((*((tU32*)(pC))>>ulGLShift)&ulGMask)/fGMul;

  if (ulBSigned)  col.z = tF32(tI32(((*((tU32*)(pC))>>ulBLShift)&ulBMask)<<ulBRShift))/2147483648.0f;
  else      col.z = tF32((*((tU32*)(pC))>>ulBLShift)&ulBMask)/fBMul;

  if (ulASigned)  col.w = tF32(tI32(((*((tU32*)(pC))>>ulALShift)&ulAMask)<<ulARShift))/2147483648.0f;
  else      col.w = tF32((*((tU32*)(pC))>>ulALShift)&ulAMask)/fAMul;
}

///////////////////////////////////////////////
void cPixelFormatStd::s32::UnpackPixelul(tPtr pC, sColor4ul& col) const
{
  col.x = ((*((tU32*)(pC))>>ulRLShift)&ulRMask)<<ulRRShift;
  col.y = ((*((tU32*)(pC))>>ulGLShift)&ulGMask)<<ulGRShift;
  col.z = ((*((tU32*)(pC))>>ulBLShift)&ulBMask)<<ulBRShift;
  col.w = ((*((tU32*)(pC))>>ulALShift)&ulAMask)<<ulARShift;
}

///////////////////////////////////////////////
void cPixelFormatStd::s32::CopyOne(tPtr pDst, tPtr pSrc) const
{
  *((tU32*)(pDst)) = *((tU32*)(pSrc));
}

//////////////////////////////////////////////////////////////////////////////////////////////
// 40 bits pixel format

///////////////////////////////////////////////
tPtr cPixelFormatStd::s40::BuildPixelul(tPtr pOut, tU32 r, tU32 g, tU32 b, tU32 a) const
{
  tU64 col = ((((a>>ulARShift)&qAMask) << ulALShift) |
              (((r>>ulRRShift)&qRMask) << ulRLShift) |
              (((g>>ulGRShift)&qGMask) << ulGLShift) |
              (((b>>ulBRShift)&qBMask) << ulBLShift));
  *((tU40*)(pOut)) = *((tU40*)(&col));
  return pOut;
}

///////////////////////////////////////////////
tPtr cPixelFormatStd::s40::BuildPixelf(tPtr pOut, tF32 r, tF32 g, tF32 b, tF32 a) const
{
  tU64 col =  ((tU64(a*fAMul)&qAMask) << ulALShift) |
      ((tU64(r*fRMul)&qRMask) << ulRLShift) |
      ((tU64(g*fGMul)&qGMask) << ulGLShift) |
      ((tU64(b*fBMul)&qBMask) << ulBLShift);
  *((tU40*)(pOut)) = *((tU40*)(&col));
  return pOut;
}

///////////////////////////////////////////////
void cPixelFormatStd::s40::UnpackPixelf(tPtr pC, sColor4f& col) const
{
  tU64 ulV = bit_castz<tU64>(*(tU40*)pC);

  if (ulRSigned)  col.x = tF32(tI64(((ulV>>ulRLShift)&qRMask)<<ulRRShift))/2147483648.0f;

  if (ulGSigned)  col.y = tF32(tI64(((ulV>>ulGLShift)&qGMask)<<ulGRShift))/2147483648.0f;
  else      col.y = tF32(tI64((ulV>>ulGLShift)&qGMask))/fGMul;

  if (ulBSigned)  col.z = tF32(tI64(((ulV>>ulBLShift)&qBMask)<<ulBRShift))/2147483648.0f;
  else      col.z = tF32(tI64((ulV>>ulBLShift)&qBMask))/fBMul;

  if (ulASigned)  col.w = tF32(tI64(((ulV>>ulALShift)&qAMask)<<ulARShift))/2147483648.0f;
  else      col.w = tF32(tI64((ulV>>ulALShift)&qAMask))/fAMul;
}

///////////////////////////////////////////////
void cPixelFormatStd::s40::UnpackPixelul(tPtr pC, sColor4ul& col) const
{
  tU64 ulV = bit_castz<tU64>(*(tU40*)pC);
  col.x = tU32(((ulV>>ulRLShift)&qRMask)<<ulRRShift);
  col.y = tU32(((ulV>>ulGLShift)&qGMask)<<ulGRShift);
  col.z = tU32(((ulV>>ulBLShift)&qBMask)<<ulBRShift);
  col.w = tU32(((ulV>>ulALShift)&qAMask)<<ulARShift);
}

///////////////////////////////////////////////
void cPixelFormatStd::s40::CopyOne(tPtr pDst, tPtr pSrc) const
{
  *((tU40*)(pDst)) = *((tU40*)(pSrc));
}

//////////////////////////////////////////////////////////////////////////////////////////////
// 48 bits pixel format

///////////////////////////////////////////////
tPtr cPixelFormatStd::s48::BuildPixelul(tPtr pOut, tU32 r, tU32 g, tU32 b, tU32 a) const
{
  tU64 col = ((((a>>ulARShift)&qAMask) << ulALShift) |
              (((r>>ulRRShift)&qRMask) << ulRLShift) |
              (((g>>ulGRShift)&qGMask) << ulGLShift) |
              (((b>>ulBRShift)&qBMask) << ulBLShift));
  *((tU48*)(pOut)) = *((tU48*)(&col));
  return pOut;
}

///////////////////////////////////////////////
tPtr cPixelFormatStd::s48::BuildPixelf(tPtr pOut, tF32 r, tF32 g, tF32 b, tF32 a) const
{
  tU64 col =  ((tU64(a*fAMul)&qAMask) << ulALShift) |
      ((tU64(r*fRMul)&qRMask) << ulRLShift) |
      ((tU64(g*fGMul)&qGMask) << ulGLShift) |
      ((tU64(b*fBMul)&qBMask) << ulBLShift);
  *((tU48*)(pOut)) = *((tU48*)(&col));
  return pOut;
}

///////////////////////////////////////////////
void cPixelFormatStd::s48::UnpackPixelf(tPtr pC, sColor4f& col) const
{
  tU64 ulV = bit_castz<tU64>(*(tU48*)pC);

  if (ulRSigned)  col.x = tF32(tI64(((ulV>>ulRLShift)&qRMask)<<ulRRShift))/2147483648.0f;
  else      col.x = tF32(tI64((ulV>>ulRLShift)&qRMask))/fRMul;

  if (ulGSigned)  col.y = tF32(tI64(((ulV>>ulGLShift)&qGMask)<<ulGRShift))/2147483648.0f;
  else      col.y = tF32(tI64((ulV>>ulGLShift)&qGMask))/fGMul;

  if (ulBSigned)  col.z = tF32(tI64(((ulV>>ulBLShift)&qBMask)<<ulBRShift))/2147483648.0f;
  else      col.z = tF32(tI64((ulV>>ulBLShift)&qBMask))/fBMul;

  if (ulASigned)  col.w = tF32(tI64(((ulV>>ulALShift)&qAMask)<<ulARShift))/2147483648.0f;
  else      col.w = tF32(tI64((ulV>>ulALShift)&qAMask))/fAMul;
}

///////////////////////////////////////////////
void cPixelFormatStd::s48::UnpackPixelul(tPtr pC, sColor4ul& col) const
{
  tU64 ulV = bit_castz<tU64>(*(tU48*)pC);
  col.x = tU32(((ulV>>ulRLShift)&qRMask)<<ulRRShift);
  col.y = tU32(((ulV>>ulGLShift)&qGMask)<<ulGRShift);
  col.z = tU32(((ulV>>ulBLShift)&qBMask)<<ulBRShift);
  col.w = tU32(((ulV>>ulALShift)&qAMask)<<ulARShift);
}

///////////////////////////////////////////////
void cPixelFormatStd::s48::CopyOne(tPtr pDst, tPtr pSrc) const
{
  *((tU48*)(pDst)) = *((tU48*)(pSrc));
}

//////////////////////////////////////////////////////////////////////////////////////////////
// 56 bits pixel format

///////////////////////////////////////////////
tPtr cPixelFormatStd::s56::BuildPixelul(tPtr pOut, tU32 r, tU32 g, tU32 b, tU32 a) const
{
  tU64 col = ((((a>>ulARShift)&qAMask) << ulALShift) |
              (((r>>ulRRShift)&qRMask) << ulRLShift) |
              (((g>>ulGRShift)&qGMask) << ulGLShift) |
              (((b>>ulBRShift)&qBMask) << ulBLShift));
  *((tU56*)(pOut)) = *((tU56*)(&col));
  return pOut;
}

///////////////////////////////////////////////
tPtr cPixelFormatStd::s56::BuildPixelf(tPtr pOut, tF32 r, tF32 g, tF32 b, tF32 a) const
{
  tU64 col =  ((tU64(a*fAMul)&qAMask) << ulALShift) |
      ((tU64(r*fRMul)&qRMask) << ulRLShift) |
      ((tU64(g*fGMul)&qGMask) << ulGLShift) |
      ((tU64(b*fBMul)&qBMask) << ulBLShift);
  *((tU56*)(pOut)) = *((tU56*)(&col));
  return pOut;
}

///////////////////////////////////////////////
void cPixelFormatStd::s56::UnpackPixelf(tPtr pC, sColor4f& col) const
{
  tU64 ulV = bit_castz<tU64>(*(tU56*)pC);

  if (ulRSigned)  col.x = tF32(tI64(((ulV>>ulRLShift)&qRMask)<<ulRRShift))/2147483648.0f;
  else      col.x = tF32(tI64((ulV>>ulRLShift)&qRMask))/fRMul;

  if (ulGSigned)  col.y = tF32(tI64(((ulV>>ulGLShift)&qGMask)<<ulGRShift))/2147483648.0f;
  else      col.y = tF32(tI64((ulV>>ulGLShift)&qGMask))/fGMul;

  if (ulBSigned)  col.z = tF32(tI64(((ulV>>ulBLShift)&qBMask)<<ulBRShift))/2147483648.0f;
  else      col.z = tF32(tI64((ulV>>ulBLShift)&qBMask))/fBMul;

  if (ulASigned)  col.w = tF32(tI64(((ulV>>ulALShift)&qAMask)<<ulARShift))/2147483648.0f;
  else      col.w = tF32(tI64((ulV>>ulALShift)&qAMask))/fAMul;
}

///////////////////////////////////////////////
void cPixelFormatStd::s56::UnpackPixelul(tPtr pC, sColor4ul& col) const
{
  tU64 ulV = bit_castz<tU64>(*(tU56*)pC);
  col.x = tU32(((ulV>>ulRLShift)&qRMask)<<ulRRShift);
  col.y = tU32(((ulV>>ulGLShift)&qGMask)<<ulGRShift);
  col.z = tU32(((ulV>>ulBLShift)&qBMask)<<ulBRShift);
  col.w = tU32(((ulV>>ulALShift)&qAMask)<<ulARShift);
}

///////////////////////////////////////////////
void cPixelFormatStd::s56::CopyOne(tPtr pDst, tPtr pSrc) const
{
  *((tU56*)(pDst)) = *((tU56*)(pSrc));
}

//////////////////////////////////////////////////////////////////////////////////////////////
// 64 bits pixel format

///////////////////////////////////////////////
tPtr cPixelFormatStd::s64::BuildPixelul(tPtr pOut, tU32 r, tU32 g, tU32 b, tU32 a) const
{
  *((tU64*)(pOut)) = tU64((((a>>ulARShift)&qAMask) << ulALShift) |
                          (((r>>ulRRShift)&qRMask) << ulRLShift) |
                          (((g>>ulGRShift)&qGMask) << ulGLShift) |
                          (((b>>ulBRShift)&qBMask) << ulBLShift));
  return pOut;
}

///////////////////////////////////////////////
tPtr cPixelFormatStd::s64::BuildPixelf(tPtr pOut, tF32 r, tF32 g, tF32 b, tF32 a) const
{
  *((tU64*)(pOut)) = tU64(((tU64(a*fAMul)&qAMask) << ulALShift) |
                          ((tU64(r*fRMul)&qRMask) << ulRLShift) |
                          ((tU64(g*fGMul)&qGMask) << ulGLShift) |
                          ((tU64(b*fBMul)&qBMask) << ulBLShift));
  return pOut;
}

///////////////////////////////////////////////
void cPixelFormatStd::s64::UnpackPixelf(tPtr pC, sColor4f& col) const
{
  if (ulASigned)  col.w = tF32(tI32(((*((tU64*)pC)>>ulALShift)&qAMask)<<ulARShift))/2147483648.0f;
  else      col.w = tF32((*((tI64*)(pC))>>ulALShift)&qAMask)/fAMul;

  if (ulRSigned)  col.x = tF32(tI32(((*((tU64*)pC)>>ulRLShift)&qRMask)<<ulRRShift))/2147483648.0f;
  else      col.x = tF32((*((tI64*)(pC))>>ulRLShift)&qRMask)/fRMul;

  if (ulGSigned)  col.y = tF32(tI32(((*((tU64*)pC)>>ulGLShift)&qGMask)<<ulGRShift))/2147483648.0f;
  else      col.y = tF32((*((tI64*)(pC))>>ulGLShift)&qGMask)/fGMul;

  if (ulBSigned)  col.z = tF32(tI32(((*((tU64*)pC)>>ulBLShift)&qBMask)<<ulBRShift))/2147483648.0f;
  else      col.z = tF32((*((tI64*)(pC))>>ulBLShift)&qBMask)/fBMul;
}

///////////////////////////////////////////////
void cPixelFormatStd::s64::UnpackPixelul(tPtr pC, sColor4ul& col) const
{
  col.x = tU32(((*((tU64*)pC)>>ulRLShift)&qRMask)<<ulRRShift);
  col.y = tU32(((*((tU64*)pC)>>ulGLShift)&qGMask)<<ulGRShift);
  col.z = tU32(((*((tU64*)pC)>>ulBLShift)&qBMask)<<ulBRShift);
  col.w = tU32(((*((tU64*)pC)>>ulALShift)&qAMask)<<ulARShift);
}

///////////////////////////////////////////////
void cPixelFormatStd::s64::CopyOne(tPtr pDst, tPtr pSrc) const
{
  *((tU64*)(pDst)) = *((tU64*)(pSrc));
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Floating point pixel format

///////////////////////////////////////////////
tPtr cPixelFormatStd::sFloat::BuildPixelul(tPtr pOut, tU32 r, tU32 g, tU32 b, tU32 a) const
{
  if (niFlagIs(RFlags,PXFFLOAT_HASCHANNEL)) {
    if (niFlagIs(RFlags,PXFFLOAT_16BITS))
      *((tF16*)(pOut+PXFFLOAT_GETINDEX(RFlags))) = tF32(r>>16)/65535.0f;
    else
      *((tF32*)(pOut+PXFFLOAT_GETINDEX(RFlags))) = tF32(r>>16)/65535.0f;
  }
  if (niFlagIs(GFlags,PXFFLOAT_HASCHANNEL)) {
    if (niFlagIs(RFlags,PXFFLOAT_16BITS))
      *((tF16*)(pOut+PXFFLOAT_GETINDEX(GFlags))) = tF32(g>>16)/65535.0f;
    else
      *((tF32*)(pOut+PXFFLOAT_GETINDEX(GFlags))) = tF32(g>>16)/65535.0f;
  }
  if (niFlagIs(BFlags,PXFFLOAT_HASCHANNEL)) {
    if (niFlagIs(RFlags,PXFFLOAT_16BITS))
      *((tF16*)(pOut+PXFFLOAT_GETINDEX(BFlags))) = tF32(b>>16)/65535.0f;
    else
      *((tF32*)(pOut+PXFFLOAT_GETINDEX(BFlags))) = tF32(b>>16)/65535.0f;
  }
  if (niFlagIs(AFlags,PXFFLOAT_HASCHANNEL)) {
    if (niFlagIs(RFlags,PXFFLOAT_16BITS))
      *((tF16*)(pOut+PXFFLOAT_GETINDEX(AFlags))) = tF32(a>>16)/65535.0f;
    else
      *((tF32*)(pOut+PXFFLOAT_GETINDEX(AFlags))) = tF32(a>>16)/65535.0f;
  }
  return pOut;
}

///////////////////////////////////////////////
tPtr cPixelFormatStd::sFloat::BuildPixelf(tPtr pOut, tF32 r, tF32 g, tF32 b, tF32 a) const
{
  if (niFlagIs(RFlags,PXFFLOAT_HASCHANNEL)) {
    if (niFlagIs(RFlags,PXFFLOAT_16BITS))
      *((tF16*)(pOut+PXFFLOAT_GETINDEX(RFlags))) = r;
    else
      *((tF32*)(pOut+PXFFLOAT_GETINDEX(RFlags))) = r;
  }
  if (niFlagIs(GFlags,PXFFLOAT_HASCHANNEL)) {
    if (niFlagIs(GFlags,PXFFLOAT_16BITS))
      *((tF16*)(pOut+PXFFLOAT_GETINDEX(GFlags))) = g;
    else
      *((tF32*)(pOut+PXFFLOAT_GETINDEX(GFlags))) = g;
  }
  if (niFlagIs(BFlags,PXFFLOAT_HASCHANNEL)) {
    if (niFlagIs(BFlags,PXFFLOAT_16BITS))
      *((tF16*)(pOut+PXFFLOAT_GETINDEX(BFlags))) = b;
    else
      *((tF32*)(pOut+PXFFLOAT_GETINDEX(BFlags))) = b;
  }
  if (niFlagIs(AFlags,PXFFLOAT_HASCHANNEL)) {
    if (niFlagIs(AFlags,PXFFLOAT_16BITS))
      *((tF16*)(pOut+PXFFLOAT_GETINDEX(AFlags))) = a;
    else
      *((tF32*)(pOut+PXFFLOAT_GETINDEX(AFlags))) = a;
  }
  return pOut;
}

///////////////////////////////////////////////
void cPixelFormatStd::sFloat::UnpackPixelf(tPtr pColor, sColor4f& col) const
{
  col.x = niFlagIsNot(RFlags,PXFFLOAT_HASCHANNEL)?0.0f:
      niFlagIs(RFlags,PXFFLOAT_16BITS) ?
      (tF32)*((tF16*)(pColor+PXFFLOAT_GETINDEX(RFlags))) :
      *((tF32*)(pColor+PXFFLOAT_GETINDEX(RFlags)));
  col.y = niFlagIsNot(GFlags,PXFFLOAT_HASCHANNEL)?0.0f:
      niFlagIs(GFlags,PXFFLOAT_16BITS) ?
      (tF32)*((tF16*)(pColor+PXFFLOAT_GETINDEX(GFlags))) :
      *((tF32*)(pColor+PXFFLOAT_GETINDEX(GFlags)));
  col.z = niFlagIsNot(BFlags,PXFFLOAT_HASCHANNEL)?0.0f:
      niFlagIs(BFlags,PXFFLOAT_16BITS) ?
      (tF32)*((tF16*)(pColor+PXFFLOAT_GETINDEX(BFlags))) :
      *((tF32*)(pColor+PXFFLOAT_GETINDEX(BFlags)));
  col.w = niFlagIsNot(AFlags,PXFFLOAT_HASCHANNEL)?0.0f:
      niFlagIs(AFlags,PXFFLOAT_16BITS) ?
      (tF32)*((tF16*)(pColor+PXFFLOAT_GETINDEX(AFlags))) :
      *((tF32*)(pColor+PXFFLOAT_GETINDEX(AFlags)));
}

///////////////////////////////////////////////
void cPixelFormatStd::sFloat::UnpackPixelul(tPtr pColor, sColor4ul& col) const
{
  col.x = niFlagIsNot(RFlags,PXFFLOAT_HASCHANNEL)?0:
      tU32(ni::Clamp<tF32>(niFlagIs(RFlags,PXFFLOAT_16BITS) ?
                           (tF32)*((tF16*)(pColor+PXFFLOAT_GETINDEX(RFlags))) :
                           *((tF32*)(pColor+PXFFLOAT_GETINDEX(RFlags))),0.0f,1.0f)*65535.0f)<<16;
  col.y = niFlagIsNot(GFlags,PXFFLOAT_HASCHANNEL)?0:
      tU32(ni::Clamp<tF32>(niFlagIs(GFlags,PXFFLOAT_16BITS) ?
                           (tF32)*((tF16*)(pColor+PXFFLOAT_GETINDEX(GFlags))) :
                           *((tF32*)(pColor+PXFFLOAT_GETINDEX(GFlags))),0.0f,1.0f)*65535.0f)<<16;
  col.z = niFlagIsNot(BFlags,PXFFLOAT_HASCHANNEL)?0:
      tU32(ni::Clamp<tF32>(niFlagIs(BFlags,PXFFLOAT_16BITS) ?
                           (tF32)*((tF16*)(pColor+PXFFLOAT_GETINDEX(BFlags))) :
                           *((tF32*)(pColor+PXFFLOAT_GETINDEX(BFlags))),0.0f,1.0f)*65535.0f)<<16;
  col.w = niFlagIsNot(AFlags,PXFFLOAT_HASCHANNEL)?0:
      tU32(ni::Clamp<tF32>(niFlagIs(AFlags,PXFFLOAT_16BITS) ?
                           (tF32)*((tF16*)(pColor+PXFFLOAT_GETINDEX(AFlags))) :
                           *((tF32*)(pColor+PXFFLOAT_GETINDEX(AFlags))),0.0f,1.0f)*65535.0f)<<16;
}

///////////////////////////////////////////////
void cPixelFormatStd::sFloat::CopyOne(tPtr pDst, tPtr pSrc) const
{
  if (niFlagIs(RFlags,PXFFLOAT_HASCHANNEL)) {
    if (niFlagIs(RFlags,PXFFLOAT_16BITS))
      *((tF16*)(pDst+PXFFLOAT_GETINDEX(RFlags))) =
          *((tF16*)(pSrc+PXFFLOAT_GETINDEX(RFlags)));
    else
      *((tF32*)(pDst+PXFFLOAT_GETINDEX(RFlags))) =
          *((tF32*)(pSrc+PXFFLOAT_GETINDEX(RFlags)));
  }
  if (niFlagIs(GFlags,PXFFLOAT_HASCHANNEL)) {
    if (niFlagIs(GFlags,PXFFLOAT_16BITS))
      *((tF16*)(pDst+PXFFLOAT_GETINDEX(GFlags))) =
          *((tF16*)(pSrc+PXFFLOAT_GETINDEX(GFlags)));
    else
      *((tF32*)(pDst+PXFFLOAT_GETINDEX(GFlags))) =
          *((tF32*)(pSrc+PXFFLOAT_GETINDEX(GFlags)));
  }
  if (niFlagIs(BFlags,PXFFLOAT_HASCHANNEL)) {
    if (niFlagIs(BFlags,PXFFLOAT_16BITS))
      *((tF16*)(pDst+PXFFLOAT_GETINDEX(BFlags))) =
          *((tF16*)(pSrc+PXFFLOAT_GETINDEX(BFlags)));
    else
      *((tF32*)(pDst+PXFFLOAT_GETINDEX(BFlags))) =
          *((tF32*)(pSrc+PXFFLOAT_GETINDEX(BFlags)));
  }
  if (niFlagIs(AFlags,PXFFLOAT_HASCHANNEL)) {
    if (niFlagIs(AFlags,PXFFLOAT_16BITS))
      *((tF16*)(pDst+PXFFLOAT_GETINDEX(AFlags))) =
          *((tF16*)(pSrc+PXFFLOAT_GETINDEX(AFlags)));
    else
      *((tF32*)(pDst+PXFFLOAT_GETINDEX(AFlags))) =
          *((tF32*)(pSrc+PXFFLOAT_GETINDEX(AFlags)));
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
// cPixelFormatStd implementation

///////////////////////////////////////////////
cPixelFormatStd::cPixelFormatStd(const achar* aszFormat)
{
  ZeroMembers();

  mstrFormat = aszFormat;

  mCaps = ePixelFormatCaps( ePixelFormatCaps_BuildPixel       |
                            ePixelFormatCaps_UnpackPixel      |
                            ePixelFormatCaps_Blit         |
                            ePixelFormatCaps_BlitMirrorLeftRight  |
                            ePixelFormatCaps_BlitMirrorUpDown   |
                            ePixelFormatCaps_BlitStretch);

  // Parse the format and fill mComps
  const achar* p = aszFormat;
  if (*p == 'F')
  {
    sFloat* pFloatFmt = niNew sFloat();
    tU8* pChannel;
    ++p;
    tU32 i;
    tU32 nCurByte = 0;
    for (i = 0; i < 4 && *p; ++i)
    {
      achar channel = *p;
      switch (channel)
      {
        case 'D':
        case 'R': pChannel = &pFloatFmt->RFlags; break;
        case 'G': pChannel = &pFloatFmt->GFlags; break;
        case 'B': pChannel = &pFloatFmt->BFlags; break;
        case 'A':
        case 'S':
        case 'P': // for backward compatibility with paletted format
        case 'X': pChannel = &pFloatFmt->AFlags; break;
        default:  pChannel = NULL;
      }
      if (pChannel == NULL)
      {
        niError(niFmt(_A("Channel '%c' unknown, format '%s'."),*p,aszFormat));
        niDelete pFloatFmt;
        return;
      }

      if (*pChannel != 0)
      {
        niError(niFmt(_A("Channel '%c' already defined, format '%s'."),*p,aszFormat));
        niDelete pFloatFmt;
        return;
      }

      cString str;
      ++p;
      if (*p == 0)
      {
        niError(niFmt(_A("Unexpected end of format '%s'."),aszFormat));
        niDelete pFloatFmt;
        return;
      }
      str.appendChar(*p);
      ++p;
      if (*p == 0)
      {
        niError(niFmt(_A("Unexpected end of format '%s'."),aszFormat));
        niDelete pFloatFmt;
        return;
      }
      str.appendChar(*p);
      ++p;

      tU32 nNumBits = str.Long();
      if (nNumBits != 16 && nNumBits != 32)
      {
        niError(niFmt(_A("Invalid number of bits '%d' after channel '%c', format '%s'."),nNumBits,channel,aszFormat));
        niDelete pFloatFmt;
        return;
      }
      *pChannel |= PXFFLOAT_HASCHANNEL|PXFFLOAT_BUILDINDEX(nCurByte)|((nNumBits==16)?PXFFLOAT_16BITS:0);
      mulBitsPerPixel += nNumBits;
      nCurByte += nNumBits==16?2:4;
    }

    if (*p != 0)
    {
      niError(niFmt(_A("Expected end of format '%s', from '%s'."),aszFormat,p-1));
      niDelete pFloatFmt;
      return;
    }

    mulComp = i;
    mulBytesPerPixel = niBytesPerPixel(mulBitsPerPixel);
    mpBase = pFloatFmt;
    mulNumRBits = niFlagIs(pFloatFmt->RFlags,PXFFLOAT_16BITS)?16:32;
    mulNumGBits = niFlagIs(pFloatFmt->GFlags,PXFFLOAT_16BITS)?16:32;
    mulNumBBits = niFlagIs(pFloatFmt->BFlags,PXFFLOAT_16BITS)?16:32;
    mulNumABits = niFlagIs(pFloatFmt->AFlags,PXFFLOAT_16BITS)?16:32;
  }
  else
  {
    tI32 lComp = 0;
    tU32 ulLShift = 0;
    achar aLastChar = 0;
    cString strCurrentDigit;
    sInfo64 inf;
    memset(&inf,0,sizeof(inf));

    while (1)
    {
      if (!*p || !ni::StrIsDigit(*p))
      {
        if (strCurrentDigit.IsNotEmpty())
        {
          tBool bSigned = eFalse;

          aLastChar = StrToUpper(aLastChar);
          switch (aLastChar) {
            case _A('A'): case _A('Q'): case _A('X'): case _A('S'): lComp = 0;  break;
            case _A('R'): case _A('W'): case _A('L'): case _A('P'): case _A('D'): lComp = 1;  break;
            case _A('G'): case _A('V'):               lComp = 2;  break;
            case _A('B'): case _A('U'):               lComp = 3;  break;
            default: lComp = -1;
          }

          if (aLastChar == _A('Q') || aLastChar == _A('W') ||
              aLastChar == _A('V') || aLastChar == _A('U'))
          {
            bSigned = eTrue;
            mCaps |= ePixelFormatCaps_Signed;
          }

          if (lComp >= 0)
          {
            sInfo64::sComponent* pComp = &inf.Comps[lComp];
            tU64 ulNumBit = strCurrentDigit.Long();
            switch (lComp)
            {
              case 0: mulNumABits = (tU32)ulNumBit; break;
              case 1: mulNumRBits = (tU32)ulNumBit; break;
              case 2: mulNumGBits = (tU32)ulNumBit; break;
              case 3: mulNumBBits = (tU32)ulNumBit; break;
            }
            mulBitsPerPixel += (tU32)ulNumBit;
            pComp->ulRShift = (tU32)(32-ulNumBit);
            pComp->ulLShift = ulLShift;
            pComp->qMask = (1LL<<(tI64)ulNumBit)-1LL;
            pComp->fMul = tF32(pComp->qMask)*(bSigned?0.5f:1.0f);
            pComp->ulSigned = bSigned ? 1 : 0;
            ulLShift += (tU32)ulNumBit;
            strCurrentDigit.Clear();
            mulComp++;
          }
        }
        if (!*p)
          break;
        aLastChar = *p;
      }
      else
      {
        strCurrentDigit.appendChar(*p);
      }
      ++p;
    }

    mulBytesPerPixel = niBytesPerPixel(mulBitsPerPixel);
    switch (mulBytesPerPixel)
    {
      case 1: mpBase = niNew s8();   *((s8*)(mpBase)) = inf; break;
      case 2: mpBase = niNew s16();  *((s16*)(mpBase)) = inf; break;
      case 3: mpBase = niNew s24();  *((s24*)(mpBase)) = inf; break;
      case 4: mpBase = niNew s32();  *((s32*)(mpBase)) = inf; break;
      case 5: mpBase = niNew s40();  *((s40*)(mpBase)) = inf; break;
      case 6: mpBase = niNew s48();  *((s48*)(mpBase)) = inf; break;
      case 7: mpBase = niNew s56();  *((s56*)(mpBase)) = inf; break;
      case 8: mpBase = niNew s64();  *((s64*)(mpBase)) = inf; break;
      default: break;
    }
  }
}

///////////////////////////////////////////////
cPixelFormatStd::~cPixelFormatStd()
{
  niSafeDelete(mpBase);
}

///////////////////////////////////////////////
void cPixelFormatStd::ZeroMembers()
{
  mCaps = ePixelFormatCaps(0);
  mulBitsPerPixel = 0;
  mulBytesPerPixel = 0;
  mulComp = 0;
  mpBase = NULL;
  mulNumRBits = mulNumGBits = mulNumBBits = mulNumABits = 0;
}

///////////////////////////////////////////////
tBool cPixelFormatStd::IsOK() const
{
  return mpBase != NULL;
}

///////////////////////////////////////////////
tBool cPixelFormatStd::IsSamePixelFormat(const iPixelFormat* pPixFmt) const
{
  return ni::StrICmp(pPixFmt->GetFormat(), GetFormat()) == 0;
}

///////////////////////////////////////////////
iPixelFormat* cPixelFormatStd::Clone() const
{
  return niThis(cPixelFormatStd);
}

///////////////////////////////////////////////
const achar* cPixelFormatStd::GetFormat() const
{
  return mstrFormat.Chars();
}

///////////////////////////////////////////////
ePixelFormatCaps cPixelFormatStd::GetCaps() const
{
  return (ePixelFormatCaps)mCaps;
}

///////////////////////////////////////////////
tU32 cPixelFormatStd::GetBitsPerPixel() const
{
  return mulBitsPerPixel;
}

///////////////////////////////////////////////
tU32 cPixelFormatStd::GetBytesPerPixel() const
{
  return mulBytesPerPixel;
}

///////////////////////////////////////////////
tU32 cPixelFormatStd::GetSize(tU32 ulW, tU32 ulH, tU32 ulD) const
{
  if (ulD)
  {
    return ulW*ulH*ulD*GetBytesPerPixel();
  }
  else
  {
    return ulW*ulH*GetBytesPerPixel();
  }
}

///////////////////////////////////////////////
tU32 cPixelFormatStd::GetNumComponents() const
{
  return mulComp;
}

///////////////////////////////////////////////
tU32 cPixelFormatStd::GetNumRBits() const
{
  return mulNumRBits;
}

///////////////////////////////////////////////
tU32 cPixelFormatStd::GetNumGBits() const
{
  return mulNumGBits;
}

///////////////////////////////////////////////
tU32 cPixelFormatStd::GetNumBBits() const
{
  return mulNumBBits;
}

///////////////////////////////////////////////
tU32 cPixelFormatStd::GetNumABits() const
{
  return mulNumABits;
}

///////////////////////////////////////////////
tPtr cPixelFormatStd::BuildPixelul(tPtr pOut, tU32 r, tU32 g, tU32 b, tU32 a) const
{
  return mpBase->BuildPixelul(pOut, r, g, b, a);
}

///////////////////////////////////////////////
tPtr cPixelFormatStd::BuildPixelf(tPtr pOut, tF32 r, tF32 g, tF32 b, tF32 a) const
{
  return mpBase->BuildPixelf(pOut, r, g, b, a);
}

///////////////////////////////////////////////
tPtr cPixelFormatStd::BeginUnpackPixels(tPtr pSurface, tU32 ulPitch, tU32 ulX, tU32 ulY, tU32 ulW, tU32 ulH)
{
  return pSurface;
}

///////////////////////////////////////////////
void cPixelFormatStd::EndUnpackPixels()
{
}

///////////////////////////////////////////////
sColor4f cPixelFormatStd::UnpackPixelf(tPtr pColor) const
{
  sColor4f col;
  mpBase->UnpackPixelf(pColor, col);
  if (!mulNumABits) {
    col.w = 1.0f;
  }
  return col;
}

///////////////////////////////////////////////
sVec4i cPixelFormatStd::UnpackPixelul(tPtr pColor) const
{
  sVec4i col;
  mpBase->UnpackPixelul(pColor, (sColor4ul&)col);
  if (!mulNumABits) {
    col.w = ~0;
  }
  return col;
}

///////////////////////////////////////////////
sColor4ub cPixelFormatStd::UnpackPixelub(tPtr pColor) const
{
  sColor4ul c;
  mpBase->UnpackPixelul(pColor, c);
  sColor4ub col;
  col.x = tU8(c.x >> 24);
  col.y = tU8(c.y >> 24);
  col.z = tU8(c.z >> 24);
  if (!mulNumABits) {
    col.w = 255;
  }
  else {
    col.w = tU8(c.w >> 24);
  }
  return col;
}

///////////////////////////////////////////////
sVec4i  cPixelFormatStd::UnpackPixelus(tPtr pColor) const
{
  sVec4i col;
  mpBase->UnpackPixelul(pColor, (sColor4ul&)col);
  col.x >>= 16;
  col.y >>= 16;
  col.z >>= 16;
  if (!mulNumABits) {
    col.w = 0xFFFF;
  }
  else {
    col.w >>= 16;
  }
  return col;
}

///////////////////////////////////////////////
static unsigned char dither_table[8] = { 0, 16, 68, 146, 170, 109, 187, 239 };
static unsigned char dither_ytable[8] = { 1, 5, 2, 7, 4, 0, 6, 3 };
static __forceinline void dither_for_4444(const int r, const int g, const int b/*, const int a*/,
                                          const int x, const int _y,
                                          int& returned_r, int& returned_g,
                                          int& returned_b/*, int& returned_a*/)
{
  const int y = dither_ytable[_y&7];
  int bpos = (x+y)&7;

  returned_r = r>>4;
  returned_g = g>>4;
  returned_b = b>>4;
  // returned_a = a>>4;

  returned_r += (dither_table[r&7] >> bpos) & 1;

  bpos = (bpos+3)&7;
  returned_b += (dither_table[b&7] >> bpos) & 1;

  bpos = (bpos+7)&7;
  returned_g += (dither_table[g&7] >> bpos) & 1;
  // alpha channel is dithered like the green channel...
  // returned_a += (dither_table[g&7] >> bpos) & 1;

  returned_r -= returned_r>>4;
  returned_g -= returned_g>>4;
  returned_b -= returned_b>>4;
  // returned_a -= returned_a>>4;
}
static __forceinline void dither_for_555(const int r, const int g, const int b,
                                         const int x, const int _y,
                                         int& returned_r, int& returned_g, int& returned_b)
{
  const int y = dither_ytable[_y&7];
  int bpos = (x+y)&7;

  returned_r = r>>3;
  returned_g = g>>3;
  returned_b = b>>3;

  returned_r += (dither_table[r&7] >> bpos) & 1;

  bpos = (bpos+3)&7;
  returned_b += (dither_table[b&7] >> bpos) & 1;

  bpos = (bpos+7)&7;
  returned_g += (dither_table[g&7] >> bpos) & 1;

  returned_r -= returned_r>>5;
  returned_g -= returned_g>>5;
  returned_b -= returned_b>>5;
}
static __forceinline void dither_for_565(const int r, const int g, const int b,
                                         const int x, const int _y,
                                         int& returned_r, int& returned_g, int& returned_b)
{
  const int y = dither_ytable[_y&7];
  int bpos = (x+y)&7;

  returned_r = r>>3;
  returned_g = g>>2;
  returned_b = b>>3;

  returned_r += (dither_table[r&7] >> bpos) & 1;

  bpos = (bpos+3)&7;
  returned_b += (dither_table[b&7] >> bpos) & 1;

  bpos = (bpos+7)&7;
  returned_g += (dither_table[(g&3)*2] >> bpos) & 1;

  returned_r -= returned_r>>5;
  returned_g -= returned_g>>6;
  returned_b -= returned_b>>5;
}

static __forceinline tU32 _ShouldDither(const iPixelFormat* apSrcFmt, const iPixelFormat* apDstFmt)
{
  const tU32 srcBpp = apSrcFmt->GetBitsPerPixel();
  if (srcBpp <= 16)
    return 0;

  const tU32 destBpp = apDstFmt->GetBitsPerPixel();
  const tU32 abits = apDstFmt->GetNumABits();
  if (destBpp == 16) {
    if (abits == 4) {
      return 12;
    }
    else if (abits) {
      return 15;
    }
    else {
      return 16;
    }
  }

  return 0;
}
static __forceinline void _DitherColor12(sColor4ub& col, tU32 x, tU32 y)
{
  int returned_r, returned_g, returned_b/*, returned_a*/;
  dither_for_4444(col.x,col.y,col.z/*,col.w*/,x,y,returned_r,returned_g,returned_b/*,returned_a*/);
  col.x = (tU8)(returned_r<<4);
  col.y = (tU8)(returned_g<<4);
  col.z = (tU8)(returned_b<<4);
  // col.w = (tU8)(returned_a<<4);
}
static __forceinline void _DitherColor15(sColor4ub& col, tU32 x, tU32 y)
{
  int returned_r, returned_g, returned_b;
  dither_for_555(col.x,col.y,col.z,x,y,returned_r,returned_g,returned_b);
  col.x = (tU8)(returned_r<<3);
  col.y = (tU8)(returned_g<<3);
  col.z = (tU8)(returned_b<<3);
}
static __forceinline void _DitherColor16(sColor4ub& col, tU32 x, tU32 y)
{
  int returned_r, returned_g, returned_b;
  dither_for_565(col.x,col.y,col.z,x,y,returned_r,returned_g,returned_b);
  col.x = (tU8)(returned_r<<3);
  col.y = (tU8)(returned_g<<2);
  col.z = (tU8)(returned_b<<3);
}
static __forceinline void _DitherColor(tU32 anBpp, sColor4ub& col, tU32 x, tU32 y) {
  switch (anBpp) {
    case 12: _DitherColor12(col,x,y); break;
    case 15: _DitherColor15(col,x,y); break;
    case 16: _DitherColor16(col,x,y); break;
  }
}

///////////////////////////////////////////////
tBool cPixelFormatStd::Blit(tPtr pDst, tU32 ulDestPitch, tU32 dx, tU32 dy,
                            tPtr pSrc, tU32 ulSrcPitch, iPixelFormat* pSrcFmt, tU32 sx, tU32 sy,
                            tU32 w, tU32 h, ePixelFormatBlit blitFlags) const
{

  // Normal blitting
  if (blitFlags == ePixelFormatBlit_Normal)
  {
    if (IsSamePixelFormat(pSrcFmt))
    {
      tU32 y;

      w *= mulBytesPerPixel;
      sx *= mulBytesPerPixel;
      dx *= mulBytesPerPixel;

      for(y = 0; y < h; y++)
      {
        memcpy(((tU8*)(pDst))+((y+dy)*ulDestPitch)+dx,
               ((tU8*)(pSrc))+((y+sy)*ulSrcPitch)+sx,
               w);
      }
    }
    else
    {
      if (!(pSrcFmt->GetCaps() & ePixelFormatCaps_UnpackPixel))
        return eFalse;

      if ((pSrcFmt->GetCaps() & ePixelFormatCaps_Signed))
      {
        if (!(mCaps & ePixelFormatCaps_Signed)) {
          const tU32 ulSrcBpp = pSrcFmt->GetBytesPerPixel();
          tU8  *pubDst, *pubSrc;
          tU32 x, y;
          pSrc = pSrcFmt->BeginUnpackPixels(pSrc, ulSrcPitch, sx, sy, w, h);
          if (pSrc) {
            for (y = 0; y < h; ++y) {
              pubSrc = ((tU8*)(pSrc))+((y+sy)*ulSrcPitch)+(sx*ulSrcBpp);
              pubDst = ((tU8*)(pDst))+((y+dy)*ulDestPitch)+(dx*mulBytesPerPixel);
              for (x = 0; x < w; ++x) {
                sColor4f c = pSrcFmt->UnpackPixelf(tPtr(pubSrc));
                BuildPixelus(tPtr(pubDst),
                             tU16(c.x*32767.0f + 32768.0f),
                             tU16(c.y*32767.0f + 32768.0f),
                             tU16(c.z*32767.0f + 32768.0f),
                             tU16(c.w*32767.0f + 32768.0f));
                pubSrc += ulSrcBpp;
                pubDst += mulBytesPerPixel;
              }
            }
            pSrcFmt->EndUnpackPixels();
          }
        }
        else
        {
          pSrc = pSrcFmt->BeginUnpackPixels(pSrc, ulSrcPitch, sx, sy, w, h);
          if (pSrc) {
            const tU32 ulSrcBpp = pSrcFmt->GetBytesPerPixel();
            tU8  *pubDst, *pubSrc;
            tU32 x, y;
            for (y = 0; y < h; ++y) {
              pubSrc = ((tU8*)(pSrc))+((y+sy)*ulSrcPitch)+(sx*ulSrcBpp);
              pubDst = ((tU8*)(pDst))+((y+dy)*ulDestPitch)+(dx*mulBytesPerPixel);
              for (x = 0; x < w; ++x) {
                sColor4f c = pSrcFmt->UnpackPixelf(tPtr(pubSrc));
                BuildPixelf(tPtr(pubDst), c.x, c.y, c.z, c.w);
                pubSrc += ulSrcBpp;
                pubDst += mulBytesPerPixel;
              }
            }
            pSrcFmt->EndUnpackPixels();
          }
        }
      }
      else // if not signed
      {
        pSrc = pSrcFmt->BeginUnpackPixels(pSrc, ulSrcPitch, sx, sy, w, h);
        if (pSrc)
        {
          const tU32 ulSrcBpp = pSrcFmt->GetBytesPerPixel();
          tU8  *pubDst, *pubSrc;
          tU32 x, y;
          const tU32 ditherBpp = _ShouldDither(pSrcFmt,this);
          if (ditherBpp) {
            for (y = 0; y < h; ++y) {
              pubSrc = ((tU8*)(pSrc))+((y+sy)*ulSrcPitch)+(sx*ulSrcBpp);
              pubDst = ((tU8*)(pDst))+((y+dy)*ulDestPitch)+(dx*mulBytesPerPixel);
              for (x = 0; x < w; ++x) {
                sColor4ub c = pSrcFmt->UnpackPixelub(tPtr(pubSrc));
                _DitherColor(ditherBpp,c,x+sx,y+sy);
                BuildPixelub(tPtr(pubDst), c.x, c.y, c.z, c.w);
                pubSrc += ulSrcBpp;
                pubDst += mulBytesPerPixel;
              }
            }
          }
          else {
            for (y = 0; y < h; ++y) {
              pubSrc = ((tU8*)(pSrc))+((y+sy)*ulSrcPitch)+(sx*ulSrcBpp);
              pubDst = ((tU8*)(pDst))+((y+dy)*ulDestPitch)+(dx*mulBytesPerPixel);
              for (x = 0; x < w; ++x) {
                sVec4i c = pSrcFmt->UnpackPixelus(tPtr(pubSrc));
                BuildPixelus(tPtr(pubDst), (tU16)c.x, (tU16)c.y, (tU16)c.z, (tU16)c.w);
                pubSrc += ulSrcBpp;
                pubDst += mulBytesPerPixel;
              }
            }
          }
          pSrcFmt->EndUnpackPixels();
        }
      }
    }
  }
  else
  {
    if (!(pSrcFmt->GetCaps() & ePixelFormatCaps_UnpackPixel))
      return eFalse;

    // Mirrored left-right and up-down
    if (niFlagTest(blitFlags,ePixelFormatBlit_MirrorLeftRight|ePixelFormatBlit_MirrorUpDown))
    {
      pSrc = pSrcFmt->BeginUnpackPixels(pSrc, ulSrcPitch, sx, sy, w, h);
      if (pSrc)
      {
        tU32 ulSrcBpp = pSrcFmt->GetBytesPerPixel();
        tU8  *pubDst, *pubSrc;
        tU32 x, y, yd;
        const tU32 ditherBpp = _ShouldDither(pSrcFmt,this);
        if (ditherBpp) {
          for (y = 0, yd = dy+h-1; y < h; ++y, --yd) {
            pubSrc = ((tU8*)(pSrc))+((y+sy)*ulSrcPitch)+(sx*ulSrcBpp);
            pubDst = ((tU8*)(pDst))+(yd*ulDestPitch)+(dx*mulBytesPerPixel)+(w*mulBytesPerPixel)-mulBytesPerPixel;
            for (x = 0; x < w; ++x) {
              sColor4ub c = pSrcFmt->UnpackPixelub(tPtr(pubSrc));
              _DitherColor(ditherBpp,c,x+sx,y+sy);
              BuildPixelub(tPtr(pubDst), c.x, c.y, c.z, c.w);
              pubSrc += ulSrcBpp;
              pubDst -= mulBytesPerPixel;
            }
          }
        }
        else {
          for (y = 0, yd = dy+h-1; y < h; ++y, --yd) {
            pubSrc = ((tU8*)(pSrc))+((y+sy)*ulSrcPitch)+(sx*ulSrcBpp);
            pubDst = ((tU8*)(pDst))+(yd*ulDestPitch)+(dx*mulBytesPerPixel)+(w*mulBytesPerPixel)-mulBytesPerPixel;
            for (x = 0; x < w; ++x) {
              sVec4i c = pSrcFmt->UnpackPixelus(tPtr(pubSrc));
              BuildPixelus(tPtr(pubDst), (tU16)c.x, (tU16)c.y, (tU16)c.z, (tU16)c.w);
              pubSrc += ulSrcBpp;
              pubDst -= mulBytesPerPixel;
            }
          }
        }
        pSrcFmt->EndUnpackPixels();
      }
    }
    // Mirrored up-down
    else if (niFlagTest(blitFlags,ePixelFormatBlit_MirrorUpDown))
    {
      if (IsSamePixelFormat(pSrcFmt)) {
        w *= mulBytesPerPixel;
        sx *= mulBytesPerPixel;
        dx *= mulBytesPerPixel;
        for (tU32 y = 0, yd = dy+h-1; y < h; ++y, --yd) {
          memcpy(((tU8*)(pDst))+((yd)*ulDestPitch)+dx,
                 ((tU8*)(pSrc))+((y+sy)*ulSrcPitch)+sx,
                 w);
        }
      }
      else {
        pSrc = pSrcFmt->BeginUnpackPixels(pSrc, ulSrcPitch, sx, sy, w, h);
        if (pSrc) {
          const tU32 ulSrcBpp = pSrcFmt->GetBytesPerPixel();
          tU8  *pubDst, *pubSrc;
          tU32 x, y, yd;
          const tU32 ditherBpp = _ShouldDither(pSrcFmt,this);
          if (ditherBpp) {
            for (y = 0, yd = dy+h-1; y < h; ++y, --yd) {
              pubSrc = ((tU8*)(pSrc))+((y+sy)*ulSrcPitch)+(sx*ulSrcBpp);
              pubDst = ((tU8*)(pDst))+(yd*ulDestPitch)+(dx*mulBytesPerPixel);
              for (x = 0; x < w; ++x) {
                sColor4ub c = pSrcFmt->UnpackPixelub(tPtr(pubSrc));
                _DitherColor(ditherBpp,c,x+sx,y+sy);
                BuildPixelub(tPtr(pubDst), c.x, c.y, c.z, c.w);
                pubSrc += ulSrcBpp;
                pubDst += mulBytesPerPixel;
              }
            }
          }
          else {
            for (y = 0, yd = dy+h-1; y < h; ++y, --yd) {
              pubSrc = ((tU8*)(pSrc))+((y+sy)*ulSrcPitch)+(sx*ulSrcBpp);
              pubDst = ((tU8*)(pDst))+(yd*ulDestPitch)+(dx*mulBytesPerPixel);
              for (x = 0; x < w; ++x)
              {
                sVec4i c = pSrcFmt->UnpackPixelus(tPtr(pubSrc));
                BuildPixelus(tPtr(pubDst), (tU16)c.x, (tU16)c.y, (tU16)c.z, (tU16)c.w);
                pubSrc += ulSrcBpp;
                pubDst += mulBytesPerPixel;
              }
            }
          }
          pSrcFmt->EndUnpackPixels();
        }
      }
    }
    // Mirrored left-right
    else if (niFlagTest(blitFlags,ePixelFormatBlit_MirrorLeftRight))
    {
      pSrc = pSrcFmt->BeginUnpackPixels(pSrc, ulSrcPitch, sx, sy, w, h);
      if (pSrc) {
        const tU32 ulSrcBpp = pSrcFmt->GetBytesPerPixel();
        tU8  *pubDst, *pubSrc;
        tU32 x, y;
        const tU32 ditherBpp = _ShouldDither(pSrcFmt,this);
        if (ditherBpp) {
          for (y = 0; y < h; ++y) {
            pubSrc = ((tU8*)(pSrc))+((y+sy)*ulSrcPitch)+(sx*ulSrcBpp);
            pubDst = ((tU8*)(pDst))+((y+dy)*ulDestPitch)+(dx*mulBytesPerPixel)+(w*mulBytesPerPixel)-mulBytesPerPixel;
            for (x = 0; x < w; ++x) {
              sColor4ub c = pSrcFmt->UnpackPixelub(tPtr(pubSrc));
              _DitherColor(ditherBpp,c,x+sx,y+sy);
              BuildPixelub(tPtr(pubDst), c.x, c.y, c.z, c.w);
              pubSrc += ulSrcBpp;
              pubDst -= mulBytesPerPixel;
            }
          }
        }
        else {
          for (y = 0; y < h; ++y) {
            pubSrc = ((tU8*)(pSrc))+((y+sy)*ulSrcPitch)+(sx*ulSrcBpp);
            pubDst = ((tU8*)(pDst))+((y+dy)*ulDestPitch)+(dx*mulBytesPerPixel)+(w*mulBytesPerPixel)-mulBytesPerPixel;
            for (x = 0; x < w; ++x) {
              sVec4i c = pSrcFmt->UnpackPixelus(tPtr(pubSrc));
              BuildPixelus(tPtr(pubDst), (tU16)c.x, (tU16)c.y, (tU16)c.z, (tU16)c.w);
              pubSrc += ulSrcBpp;
              pubDst -= mulBytesPerPixel;
            }
          }
        }
        pSrcFmt->EndUnpackPixels();
      }
    }
  }

  return eTrue;
}

///////////////////////////////////////////////
tBool cPixelFormatStd::BlitStretch(
    tPtr pDst, tU32 ulDestPitch, tU32 ulDestWidth, tU32 ulDestHeight, tU32 dx, tU32 dy, tU32 dw, tU32 dh,
    tPtr pSrc, tU32 ulSrcPitch, iPixelFormat* pSrcFmt, tU32 sx, tU32 sy, tU32 sw, tU32 sh) const
{
  if (!(pSrcFmt->GetCaps() & ePixelFormatCaps_UnpackPixel))
    return eFalse;

  // no scaling
  if (sw == dw && sh == dh) {
    return Blit(pDst,ulDestPitch,dx,dy,pSrc,ulSrcPitch,pSrcFmt,sx,sy,sw,sh,ePixelFormatBlit_Normal);
  }

  // half scaling, most likely creating mipmaps, use a bilinear filter
#pragma niTodo("This should go in the blit flags")
  if (!(sw&1) && !(sh&1) && dw == sw/2 && dh == sh/2 && dx == 0 && dy == 0 && sx == 0 && sy == 0)
  { // Want to create an inferior mip map level...
    pSrc = pSrcFmt->BeginUnpackPixels(pSrc, ulSrcPitch, sx, sy, sw, sh);
    if (pSrc)
    {
      tU8* pCol;
      tU32 x, y;
      tU32 ulSrcByPP = pSrcFmt->GetBytesPerPixel();

      for (y = 0; y < dh; ++y)
      {
        for (x = 0; x < dw; ++x)
        {
          pCol = ((tU8*)pSrc)+(ulSrcPitch*(y<<1))+((x<<1)*ulSrcByPP);
          sColor4ub ca = pSrcFmt->UnpackPixelub(tPtr(pCol));

          pCol += ulSrcByPP;  // x+1
          sColor4ub cb = pSrcFmt->UnpackPixelub(tPtr(pCol));

          pCol += ulSrcPitch;           // y+1, x+1
          sColor4ub cc = pSrcFmt->UnpackPixelub(tPtr(pCol));

          pCol -= ulSrcByPP;  // y+1, x
          sColor4ub cd = pSrcFmt->UnpackPixelub(tPtr(pCol));

          BuildPixelub(tPtr(((tU8*)(pDst))+(ulDestPitch*y)+(x*mulBytesPerPixel)),
                       (ca.x+cb.x+cc.x+cd.x)>>2,
                       (ca.y+cb.y+cc.y+cd.y)>>2,
                       (ca.z+cb.z+cc.z+cd.z)>>2,
                       (ca.w+cb.w+cc.w+cd.w)>>2);
        }
      }

      pSrcFmt->EndUnpackPixels();
    }

    return eTrue;
  }

  int x, y;
  int i1, i2, dd;
  int xinc, yinc;
  int dxbeg, dxend;
  int dybeg, dyend;
  int si_sx, si_dx;
  int si_i1, si_i2;
  int si_dd, si_dw;
  tU8 *s, *d, *dend;

  if((sw <= 0) || (sh <= 0) || (dw <= 0) || (dh <= 0))
    return eFalse;

  dybeg = (dy > 0) ? dy : 0;
  dyend = ((dy + dh) < ulDestHeight) ? (dy + dh) : ulDestHeight;
  if(dybeg >= dyend)
    return eFalse;

  dxbeg = (dx > 0) ? dx : 0;
  dxend = ((dx + dw) < ulDestWidth) ? (dx + dw) : ulDestWidth;
  if(dxbeg >= dxend)
    return eFalse;

  --sw; --sh;
  --dw; --dh;

  if(dw == 0)
  {
    xinc = 0;
  }
  else
  {
    xinc = sw / dw;
    sw %= dw;
  }

  if(dh == 0)
  {
    yinc = 0;
  }
  else
  {
    yinc = sh / dh;
    sh %= dh;
  }

  si_dw = dxend - dxbeg;
  si_i1 = 2 * sw;
  si_dd = si_i1 - si_dw;
  si_i2 = si_dd - si_dw;
  for(si_dx = dx, si_sx = sx; si_dx < dxbeg; si_dx++, si_sx += xinc)
  {
    if(si_dd >= 0)
    {
      si_sx++;
      si_dd += si_i2;
    }
    else
    {
      si_dd += si_i1;
    }
  }
  si_dx *= GetBytesPerPixel();
  si_sx *= pSrcFmt->GetBytesPerPixel();
  si_dw *= GetBytesPerPixel();
  xinc *= pSrcFmt->GetBytesPerPixel();

  i2 = (dd = (i1 = 2 * sh) - dh) - dh;
  for(x = dy, y = sy; x < dybeg; x++, y += yinc)
  {
    if(dd >= 0)
      y++, dd += i2;
    else
      dd += i1;
  }

  if (IsSamePixelFormat(pSrcFmt))
  {
    for(; x < dyend; x++, y += yinc)
    {
      // Do stretch line
      s = ((tU8*)(pSrc)) + (y*ulSrcPitch) + si_sx;
      d = ((tU8*)(pDst)) + (x*ulDestPitch) + si_dx;
      dend = d + si_dw;

      for(; d < dend; d += GetBytesPerPixel(), s += xinc)
      {
        mpBase->CopyOne(tPtr(d),tPtr(s));

        if(si_dd >= 0)
        {
          s += pSrcFmt->GetBytesPerPixel();
          si_dd += si_i2;
        }
        else
        {
          si_dd += si_i1;
        }
      }


      if(dd >= 0)
      {
        ++y;
        dd += i2;
      }
      else
      {
        dd += i1;
      }
    }
  }
  else
  {
    pSrc = pSrcFmt->BeginUnpackPixels(pSrc, ulSrcPitch, sx, sy, sw, sh);
    if (pSrc)
    {
      const tU32 ditherBpp = _ShouldDither(pSrcFmt,this);
      if (ditherBpp) {
        for (; x < dyend; ++x, y += yinc) {
          // Do stretch line
          s = ((tU8*)(pSrc)) + (y*ulSrcPitch) + si_sx;
          d = ((tU8*)(pDst)) + (x*ulDestPitch) + si_dx;
          dend = d + si_dw;

          for (tU32 ditherX = si_dx; d < dend; d += GetBytesPerPixel(), s += xinc, ++ditherX) {
            sColor4ub c = pSrcFmt->UnpackPixelub(tPtr(s));
            _DitherColor(ditherBpp,c,ditherX,x);
            BuildPixelub(tPtr(d), c.x, c.y, c.z, c.w);
            if (si_dd >= 0) {
              s += pSrcFmt->GetBytesPerPixel();
              si_dd += si_i2;
            }
            else {
              si_dd += si_i1;
            }
          }
          if (dd >= 0) {
            ++y;
            dd += i2;
          }
          else {
            dd += i1;
          }
        }
      }
      else {
        for (; x < dyend; ++x, y += yinc) {
          // Do stretch line
          s = ((tU8*)(pSrc)) + (y*ulSrcPitch) + si_sx;
          d = ((tU8*)(pDst)) + (x*ulDestPitch) + si_dx;
          dend = d + si_dw;

          for(; d < dend; d += GetBytesPerPixel(), s += xinc)
          {
            sVec4i c = pSrcFmt->UnpackPixelus(tPtr(s));
            BuildPixelus(tPtr(d), (tU16)c.x, (tU16)c.y, (tU16)c.z, (tU16)c.w);

            if (si_dd >= 0) {
              s += pSrcFmt->GetBytesPerPixel();
              si_dd += si_i2;
            }
            else {
              si_dd += si_i1;
            }
          }

          if (dd >= 0) {
            ++y;
            dd += i2;
          }
          else {
            dd += i1;
          }
        }
      }

      pSrcFmt->EndUnpackPixels();
    }
  }

  return eTrue;
}
