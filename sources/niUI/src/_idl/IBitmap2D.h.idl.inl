// clang-format off
//
// Autogenerated IDL definition for 'API/niUI/IBitmap2D.h'.
//
#include "../API/niUI/IBitmap2D.h"

#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif

IDLC_BEGIN_NAMESPACE()

/** NAMESPACE : ni **/
/** interface : iBitmap2D **/
IDLC_BEGIN_INTF(ni,iBitmap2D)
/** ni -> iBitmap2D::SetMemoryAddress/3 -> NO AUTOMATION **/
/** ni -> iBitmap2D::GetPitch/0 **/
IDLC_METH_BEGIN(ni,iBitmap2D,GetPitch,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iBitmap2D,GetPitch,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iBitmap2D,GetPitch,0)

/** ni -> iBitmap2D::GetData/0 **/
IDLC_METH_BEGIN(ni,iBitmap2D,GetData,0)
	IDLC_DECL_RETVAR(tPtr,_Ret)
	IDLC_METH_CALL(_Ret,ni,iBitmap2D,GetData,0,())
	IDLC_RET_FROM_PTR(ni::eType_Ptr,_Ret)
IDLC_METH_END(ni,iBitmap2D,GetData,0)

/** ni -> iBitmap2D::GetSize/0 **/
IDLC_METH_BEGIN(ni,iBitmap2D,GetSize,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iBitmap2D,GetSize,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iBitmap2D,GetSize,0)

/** ni -> iBitmap2D::CreateResized/2 **/
IDLC_METH_BEGIN(ni,iBitmap2D,CreateResized,2)
	IDLC_DECL_VAR(tI32,nW)
	IDLC_BUF_TO_BASE(ni::eType_I32,nW)
	IDLC_DECL_VAR(tI32,nH)
	IDLC_BUF_TO_BASE(ni::eType_I32,nH)
	IDLC_DECL_RETVAR(iBitmap2D*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iBitmap2D,CreateResized,2,(nW,nH))
	IDLC_RET_FROM_INTF(iBitmap2D,_Ret)
IDLC_METH_END(ni,iBitmap2D,CreateResized,2)

/** ni -> iBitmap2D::CreateResizedEx/3 **/
IDLC_METH_BEGIN(ni,iBitmap2D,CreateResizedEx,3)
	IDLC_DECL_VAR(tI32,nW)
	IDLC_BUF_TO_BASE(ni::eType_I32,nW)
	IDLC_DECL_VAR(tI32,nH)
	IDLC_BUF_TO_BASE(ni::eType_I32,nH)
	IDLC_DECL_VAR(tBool,abPreserveMipMaps)
	IDLC_BUF_TO_BASE(ni::eType_I8,abPreserveMipMaps)
	IDLC_DECL_RETVAR(iBitmap2D*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iBitmap2D,CreateResizedEx,3,(nW,nH,abPreserveMipMaps))
	IDLC_RET_FROM_INTF(iBitmap2D,_Ret)
IDLC_METH_END(ni,iBitmap2D,CreateResizedEx,3)

/** ni -> iBitmap2D::CreateCubeBitmap/13 **/
IDLC_METH_BEGIN(ni,iBitmap2D,CreateCubeBitmap,13)
	IDLC_DECL_VAR(tU32,anWidth)
	IDLC_BUF_TO_BASE(ni::eType_U32,anWidth)
	IDLC_DECL_VAR(sVec2i,avPX)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2i|ni::eTypeFlags_Pointer,avPX)
	IDLC_DECL_VAR(ePixelFormatBlit,aBlitPX)
	IDLC_BUF_TO_ENUM(ePixelFormatBlit,aBlitPX)
	IDLC_DECL_VAR(sVec2i,avNX)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2i|ni::eTypeFlags_Pointer,avNX)
	IDLC_DECL_VAR(ePixelFormatBlit,aBlitNX)
	IDLC_BUF_TO_ENUM(ePixelFormatBlit,aBlitNX)
	IDLC_DECL_VAR(sVec2i,avPY)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2i|ni::eTypeFlags_Pointer,avPY)
	IDLC_DECL_VAR(ePixelFormatBlit,aBlitPY)
	IDLC_BUF_TO_ENUM(ePixelFormatBlit,aBlitPY)
	IDLC_DECL_VAR(sVec2i,avNY)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2i|ni::eTypeFlags_Pointer,avNY)
	IDLC_DECL_VAR(ePixelFormatBlit,aBlitNY)
	IDLC_BUF_TO_ENUM(ePixelFormatBlit,aBlitNY)
	IDLC_DECL_VAR(sVec2i,avPZ)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2i|ni::eTypeFlags_Pointer,avPZ)
	IDLC_DECL_VAR(ePixelFormatBlit,aBlitPZ)
	IDLC_BUF_TO_ENUM(ePixelFormatBlit,aBlitPZ)
	IDLC_DECL_VAR(sVec2i,avNZ)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2i|ni::eTypeFlags_Pointer,avNZ)
	IDLC_DECL_VAR(ePixelFormatBlit,aBlitNZ)
	IDLC_BUF_TO_ENUM(ePixelFormatBlit,aBlitNZ)
	IDLC_DECL_RETVAR(iBitmapCube*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iBitmap2D,CreateCubeBitmap,13,(anWidth,avPX,aBlitPX,avNX,aBlitNX,avPY,aBlitPY,avNY,aBlitNY,avPZ,aBlitPZ,avNZ,aBlitNZ))
	IDLC_RET_FROM_INTF(iBitmapCube,_Ret)
IDLC_METH_END(ni,iBitmap2D,CreateCubeBitmap,13)

/** ni -> iBitmap2D::CreateCubeBitmapCross/0 **/
IDLC_METH_BEGIN(ni,iBitmap2D,CreateCubeBitmapCross,0)
	IDLC_DECL_RETVAR(iBitmapCube*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iBitmap2D,CreateCubeBitmapCross,0,())
	IDLC_RET_FROM_INTF(iBitmapCube,_Ret)
IDLC_METH_END(ni,iBitmap2D,CreateCubeBitmapCross,0)

/** ni -> iBitmap2D::BeginUnpackPixels/0 **/
IDLC_METH_BEGIN(ni,iBitmap2D,BeginUnpackPixels,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iBitmap2D,BeginUnpackPixels,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iBitmap2D,BeginUnpackPixels,0)

/** ni -> iBitmap2D::EndUnpackPixels/0 **/
IDLC_METH_BEGIN(ni,iBitmap2D,EndUnpackPixels,0)
	IDLC_METH_CALL_VOID(ni,iBitmap2D,EndUnpackPixels,0,())
IDLC_METH_END(ni,iBitmap2D,EndUnpackPixels,0)

/** ni -> iBitmap2D::GetMipMap/1 **/
IDLC_METH_BEGIN(ni,iBitmap2D,GetMipMap,1)
	IDLC_DECL_VAR(tU32,ulIdx)
	IDLC_BUF_TO_BASE(ni::eType_U32,ulIdx)
	IDLC_DECL_RETVAR(iBitmap2D*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iBitmap2D,GetMipMap,1,(ulIdx))
	IDLC_RET_FROM_INTF(iBitmap2D,_Ret)
IDLC_METH_END(ni,iBitmap2D,GetMipMap,1)

/** ni -> iBitmap2D::GetLevel/1 **/
IDLC_METH_BEGIN(ni,iBitmap2D,GetLevel,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(iBitmap2D*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iBitmap2D,GetLevel,1,(anIndex))
	IDLC_RET_FROM_INTF(iBitmap2D,_Ret)
IDLC_METH_END(ni,iBitmap2D,GetLevel,1)

/** ni -> iBitmap2D::Blit/8 **/
IDLC_METH_BEGIN(ni,iBitmap2D,Blit,8)
	IDLC_DECL_VAR(iBitmap2D*,src)
	IDLC_BUF_TO_INTF(iBitmap2D,src)
	IDLC_DECL_VAR(tI32,xs)
	IDLC_BUF_TO_BASE(ni::eType_I32,xs)
	IDLC_DECL_VAR(tI32,ys)
	IDLC_BUF_TO_BASE(ni::eType_I32,ys)
	IDLC_DECL_VAR(tI32,xd)
	IDLC_BUF_TO_BASE(ni::eType_I32,xd)
	IDLC_DECL_VAR(tI32,yd)
	IDLC_BUF_TO_BASE(ni::eType_I32,yd)
	IDLC_DECL_VAR(tI32,w)
	IDLC_BUF_TO_BASE(ni::eType_I32,w)
	IDLC_DECL_VAR(tI32,h)
	IDLC_BUF_TO_BASE(ni::eType_I32,h)
	IDLC_DECL_VAR(ePixelFormatBlit,eBlit)
	IDLC_BUF_TO_ENUM(ePixelFormatBlit,eBlit)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iBitmap2D,Blit,8,(src,xs,ys,xd,yd,w,h,eBlit))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iBitmap2D,Blit,8)

/** ni -> iBitmap2D::BlitStretch/9 **/
IDLC_METH_BEGIN(ni,iBitmap2D,BlitStretch,9)
	IDLC_DECL_VAR(iBitmap2D*,src)
	IDLC_BUF_TO_INTF(iBitmap2D,src)
	IDLC_DECL_VAR(tI32,xs)
	IDLC_BUF_TO_BASE(ni::eType_I32,xs)
	IDLC_DECL_VAR(tI32,ys)
	IDLC_BUF_TO_BASE(ni::eType_I32,ys)
	IDLC_DECL_VAR(tI32,xd)
	IDLC_BUF_TO_BASE(ni::eType_I32,xd)
	IDLC_DECL_VAR(tI32,yd)
	IDLC_BUF_TO_BASE(ni::eType_I32,yd)
	IDLC_DECL_VAR(tI32,ws)
	IDLC_BUF_TO_BASE(ni::eType_I32,ws)
	IDLC_DECL_VAR(tI32,hs)
	IDLC_BUF_TO_BASE(ni::eType_I32,hs)
	IDLC_DECL_VAR(tI32,wd)
	IDLC_BUF_TO_BASE(ni::eType_I32,wd)
	IDLC_DECL_VAR(tI32,hd)
	IDLC_BUF_TO_BASE(ni::eType_I32,hd)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iBitmap2D,BlitStretch,9,(src,xs,ys,xd,yd,ws,hs,wd,hd))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iBitmap2D,BlitStretch,9)

/** ni -> iBitmap2D::BlitAlpha/11 **/
IDLC_METH_BEGIN(ni,iBitmap2D,BlitAlpha,11)
	IDLC_DECL_VAR(iBitmap2D*,src)
	IDLC_BUF_TO_INTF(iBitmap2D,src)
	IDLC_DECL_VAR(tI32,xs)
	IDLC_BUF_TO_BASE(ni::eType_I32,xs)
	IDLC_DECL_VAR(tI32,ys)
	IDLC_BUF_TO_BASE(ni::eType_I32,ys)
	IDLC_DECL_VAR(tI32,xd)
	IDLC_BUF_TO_BASE(ni::eType_I32,xd)
	IDLC_DECL_VAR(tI32,yd)
	IDLC_BUF_TO_BASE(ni::eType_I32,yd)
	IDLC_DECL_VAR(tI32,w)
	IDLC_BUF_TO_BASE(ni::eType_I32,w)
	IDLC_DECL_VAR(tI32,h)
	IDLC_BUF_TO_BASE(ni::eType_I32,h)
	IDLC_DECL_VAR(ePixelFormatBlit,eBlit)
	IDLC_BUF_TO_ENUM(ePixelFormatBlit,eBlit)
	IDLC_DECL_VAR(sColor4f,acolSource)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,acolSource)
	IDLC_DECL_VAR(sColor4f,acolDest)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,acolDest)
	IDLC_DECL_VAR(eBlendMode,aBlendMode)
	IDLC_BUF_TO_ENUM(eBlendMode,aBlendMode)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iBitmap2D,BlitAlpha,11,(src,xs,ys,xd,yd,w,h,eBlit,acolSource,acolDest,aBlendMode))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iBitmap2D,BlitAlpha,11)

/** ni -> iBitmap2D::BlitAlphaStretch/12 **/
IDLC_METH_BEGIN(ni,iBitmap2D,BlitAlphaStretch,12)
	IDLC_DECL_VAR(iBitmap2D*,src)
	IDLC_BUF_TO_INTF(iBitmap2D,src)
	IDLC_DECL_VAR(tI32,xs)
	IDLC_BUF_TO_BASE(ni::eType_I32,xs)
	IDLC_DECL_VAR(tI32,ys)
	IDLC_BUF_TO_BASE(ni::eType_I32,ys)
	IDLC_DECL_VAR(tI32,xd)
	IDLC_BUF_TO_BASE(ni::eType_I32,xd)
	IDLC_DECL_VAR(tI32,yd)
	IDLC_BUF_TO_BASE(ni::eType_I32,yd)
	IDLC_DECL_VAR(tI32,ws)
	IDLC_BUF_TO_BASE(ni::eType_I32,ws)
	IDLC_DECL_VAR(tI32,hs)
	IDLC_BUF_TO_BASE(ni::eType_I32,hs)
	IDLC_DECL_VAR(tI32,wd)
	IDLC_BUF_TO_BASE(ni::eType_I32,wd)
	IDLC_DECL_VAR(tI32,hd)
	IDLC_BUF_TO_BASE(ni::eType_I32,hd)
	IDLC_DECL_VAR(sColor4f,acolSource)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,acolSource)
	IDLC_DECL_VAR(sColor4f,acolDest)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,acolDest)
	IDLC_DECL_VAR(eBlendMode,aBlendMode)
	IDLC_BUF_TO_ENUM(eBlendMode,aBlendMode)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iBitmap2D,BlitAlphaStretch,12,(src,xs,ys,xd,yd,ws,hs,wd,hd,acolSource,acolDest,aBlendMode))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iBitmap2D,BlitAlphaStretch,12)

/** ni -> iBitmap2D::PutPixel/3 -> NO AUTOMATION **/
/** ni -> iBitmap2D::GetPixel/3 -> NO AUTOMATION **/
/** ni -> iBitmap2D::Clear/1 -> NO AUTOMATION **/
/** ni -> iBitmap2D::ClearRect/2 -> NO AUTOMATION **/
/** ni -> iBitmap2D::PutPixelf/3 **/
IDLC_METH_BEGIN(ni,iBitmap2D,PutPixelf,3)
	IDLC_DECL_VAR(tI32,x)
	IDLC_BUF_TO_BASE(ni::eType_I32,x)
	IDLC_DECL_VAR(tI32,y)
	IDLC_BUF_TO_BASE(ni::eType_I32,y)
	IDLC_DECL_VAR(sColor4f,avCol)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,avCol)
	IDLC_METH_CALL_VOID(ni,iBitmap2D,PutPixelf,3,(x,y,avCol))
IDLC_METH_END(ni,iBitmap2D,PutPixelf,3)

/** ni -> iBitmap2D::GetPixelf/2 **/
IDLC_METH_BEGIN(ni,iBitmap2D,GetPixelf,2)
	IDLC_DECL_VAR(tI32,x)
	IDLC_BUF_TO_BASE(ni::eType_I32,x)
	IDLC_DECL_VAR(tI32,y)
	IDLC_BUF_TO_BASE(ni::eType_I32,y)
	IDLC_DECL_RETVAR(sColor4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iBitmap2D,GetPixelf,2,(x,y))
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iBitmap2D,GetPixelf,2)

/** ni -> iBitmap2D::Clearf/1 **/
IDLC_METH_BEGIN(ni,iBitmap2D,Clearf,1)
	IDLC_DECL_VAR(sColor4f,avCol)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,avCol)
	IDLC_METH_CALL_VOID(ni,iBitmap2D,Clearf,1,(avCol))
IDLC_METH_END(ni,iBitmap2D,Clearf,1)

/** ni -> iBitmap2D::ClearRectf/2 **/
IDLC_METH_BEGIN(ni,iBitmap2D,ClearRectf,2)
	IDLC_DECL_VAR(sRecti,aRect)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4i|ni::eTypeFlags_Pointer,aRect)
	IDLC_DECL_VAR(sColor4f,avCol)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,avCol)
	IDLC_METH_CALL_VOID(ni,iBitmap2D,ClearRectf,2,(aRect,avCol))
IDLC_METH_END(ni,iBitmap2D,ClearRectf,2)

IDLC_END_INTF(ni,iBitmap2D)

IDLC_END_NAMESPACE()
// EOF //
