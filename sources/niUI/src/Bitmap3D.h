#ifndef __BITMAP3D_30629840_H__
#define __BITMAP3D_30629840_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

namespace ni {
//////////////////////////////////////////////////////////////////////////////////////////////
// cBitmap3D declaration
class cBitmap3D : public cIUnknownImpl<iBitmap3D,
                                       eIUnknownImplFlags_DontInherit1|
                                       eIUnknownImplFlags_DontInherit3,
                                       iBitmapBase,iTexture,iDeviceResource>
{
 public:
  cBitmap3D(cGraphics* pGraphics, iHString* ahspName,
            tU32 ulW, tU32 ulH, tU32 ulD, iPixelFormat* pPixFmt,
            tBool abAllocateData);
  ~cBitmap3D();

  void ZeroMembers();

  //// iUnknown /////////////////////////////////
  tBool __stdcall IsOK() const;
  //// iUnknown /////////////////////////////////

  //// iBitmapBase //////////////////////////////
  eBitmapType __stdcall GetType() const { return eBitmapType_3D; }
  tU32 __stdcall GetWidth() const;
  tU32 __stdcall GetHeight() const;
  tU32 __stdcall GetDepth() const;
  iPixelFormat* __stdcall GetPixelFormat() const;
  tBool __stdcall CreateMipMaps(tU32 anNumMipMap, tBool abCompute);
  void __stdcall RemoveMipMaps();
  tU32 __stdcall GetNumMipMaps() const;
  iBitmapBase* __stdcall Clone(ePixelFormatBlit eBlit) const;
  iBitmapBase* __stdcall CreateConvertedFormat(const iPixelFormat* pFmt) const;
  iBitmapBase* __stdcall CreateGammaCorrected(float factor) const;
  tBool __stdcall GammaCorrect(float factor);
  //// iBitmapBase //////////////////////////////

  //// iTexture /////////////////////////////////
  iHString* __stdcall GetDeviceResourceName() const {
    return ni::HStringIsEmpty(mhspName)?(iHString*)NULL:mhspName.ptr();
  }
  tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) { return eFalse; }
  tBool __stdcall ResetDeviceResource() { return eFalse; }
  tTextureFlags __stdcall GetFlags() const {
    return eTextureFlags_SystemMemory|
        eTextureFlags_Dynamic|
        (GetNumMipMaps()?eTextureFlags_MipMaps:0);
  }
  virtual iTexture* __stdcall GetSubTexture(tU32 anIndex) const {
    return NULL;
  }
  virtual iDeviceResource* __stdcall Bind(iUnknown*) { return this; }
  //// iTexture /////////////////////////////////

  //// iBitmap3D ////////////////////////////////
  tBool __stdcall SetMemoryAddress(tPtr apAddr, tBool abFreeAddr, tU32 anRowPitch, tU32 anSlicePitch);
  tU32  __stdcall GetRowPitch() const;
  tU32  __stdcall GetSlicePitch() const;
  tPtr  __stdcall GetData() const;
  tU32  __stdcall GetSize() const;
  tPtr  __stdcall GetSlicePtr(tU32 anSlice) const {
    return _GetSlicePtr(anSlice);
  }
  iBitmap2D*  __stdcall CreateSliceBitmap(tU32 anSlice) const {
    return _GetSliceBmp(anSlice);
  }
  iBitmap3D* __stdcall CreateResized(tU32 anW, tU32 anH, tU32 anD) const;
  iBitmap3D* __stdcall GetMipMap(tU32 ulIdx) const;
  iBitmap3D* __stdcall GetLevel(tU32 anIndex) const;
  tBool __stdcall BeginUnpackPixels();
  void  __stdcall EndUnpackPixels();
  tBool __stdcall Blit(const iBitmap3D* src, const sVec3i& avSrcMin, const sVec3i& avDestMin, const sVec3i& avSize);
  void __stdcall PutPixel(const sVec3i& avPos, tPtr col);
  tPtr __stdcall GetPixel(const sVec3i& avPos, tPtr pOut) const;
  void __stdcall Clear(tPtr pColor = NULL);
  void __stdcall ClearBox(const sVec3i& avMin, const sVec3i& avSize, tPtr pColor);
  void __stdcall PutPixelf(const sVec3i& avPos, const sColor4f& avCol);
  sColor4f __stdcall GetPixelf(const sVec3i& avPos) const;
  void __stdcall Clearf(const sColor4f& avCol);
  void __stdcall ClearBoxf(const sVec3i& avMin, const sVec3i& avSize, const sColor4f& avCol);
  //// iBitmap3D ////////////////////////////////

 private:
  tBool   mbOK;
  tHStringPtr mhspName;
  cGraphics*  mpGraphics;
  tU32    mulWidth;
  tU32    mulHeight;
  tU32    mulDepth;
  tU32        mnSize;
  tPtr        mptrData;
  tBool       mbFreeData;
  tU32        mnRowPitch;
  tU32        mnSlicePitch;
  Ptr<iPixelFormat> mptrPxf;
  astl::vector<Ptr<iBitmap3D> > mvMipMaps;

  tPtr        _GetSlicePtr(tU32 anSlice) const;
  iBitmap2D*  _GetSliceBmp(tU32 anSlice) const;
  void _ResizeMipMapsVector(tU32 aulNumMipMaps);
};
} // end of namespace ni

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __BITMAP3D_30629840_H__
