#include "../API/niUI/IImage.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iImage **/
IDLC_BEGIN_INTF(ni,iImage)
/** ni -> iImage::Copy/1 **/
IDLC_METH_BEGIN(ni,iImage,Copy,1)
	IDLC_DECL_VAR(iImage*,apImage)
	IDLC_BUF_TO_INTF(iImage,apImage)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImage,Copy,1,(apImage))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iImage,Copy,1)

/** ni -> iImage::Clone/0 **/
IDLC_METH_BEGIN(ni,iImage,Clone,0)
	IDLC_DECL_RETVAR(iImage*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImage,Clone,0,())
	IDLC_RET_FROM_INTF(iImage,_Ret)
IDLC_METH_END(ni,iImage,Clone,0)

/** ni -> iImage::GetHasBitmap/0 **/
IDLC_METH_BEGIN(ni,iImage,GetHasBitmap,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImage,GetHasBitmap,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iImage,GetHasBitmap,0)

/** ni -> iImage::GrabBitmap/2 **/
IDLC_METH_BEGIN(ni,iImage,GrabBitmap,2)
	IDLC_DECL_VAR(eImageUsage,aLock)
	IDLC_BUF_TO_ENUM(eImageUsage,aLock)
	IDLC_DECL_VAR(sRecti,aDirtyRect)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4i|ni::eTypeFlags_Pointer,aDirtyRect)
	IDLC_DECL_RETVAR(iBitmap2D*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImage,GrabBitmap,2,(aLock,aDirtyRect))
	IDLC_RET_FROM_INTF(iBitmap2D,_Ret)
IDLC_METH_END(ni,iImage,GrabBitmap,2)

/** ni -> iImage::GetHasTexture/0 **/
IDLC_METH_BEGIN(ni,iImage,GetHasTexture,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImage,GetHasTexture,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iImage,GetHasTexture,0)

/** ni -> iImage::GetHasDepthStencil/0 **/
IDLC_METH_BEGIN(ni,iImage,GetHasDepthStencil,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImage,GetHasDepthStencil,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iImage,GetHasDepthStencil,0)

/** ni -> iImage::GrabTexture/2 **/
IDLC_METH_BEGIN(ni,iImage,GrabTexture,2)
	IDLC_DECL_VAR(eImageUsage,aLock)
	IDLC_BUF_TO_ENUM(eImageUsage,aLock)
	IDLC_DECL_VAR(sRecti,aDirtyRect)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4i|ni::eTypeFlags_Pointer,aDirtyRect)
	IDLC_DECL_RETVAR(iTexture*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImage,GrabTexture,2,(aLock,aDirtyRect))
	IDLC_RET_FROM_INTF(iTexture,_Ret)
IDLC_METH_END(ni,iImage,GrabTexture,2)

/** ni -> iImage::GetWidth/0 **/
IDLC_METH_BEGIN(ni,iImage,GetWidth,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImage,GetWidth,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iImage,GetWidth,0)

/** ni -> iImage::GetHeight/0 **/
IDLC_METH_BEGIN(ni,iImage,GetHeight,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImage,GetHeight,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iImage,GetHeight,0)

/** ni -> iImage::GetSize/0 **/
IDLC_METH_BEGIN(ni,iImage,GetSize,0)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImage,GetSize,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iImage,GetSize,0)

/** ni -> iImage::RecomputeBitmapMipmapsBeforeCopyToTexture/0 **/
IDLC_METH_BEGIN(ni,iImage,RecomputeBitmapMipmapsBeforeCopyToTexture,0)
	IDLC_METH_CALL_VOID(ni,iImage,RecomputeBitmapMipmapsBeforeCopyToTexture,0,())
IDLC_METH_END(ni,iImage,RecomputeBitmapMipmapsBeforeCopyToTexture,0)

IDLC_END_INTF(ni,iImage)

IDLC_END_NAMESPACE()
// EOF //
