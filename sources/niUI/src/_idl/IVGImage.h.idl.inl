#include "../API/niUI/IVGImage.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iVGImage **/
IDLC_BEGIN_INTF(ni,iVGImage)
/** ni -> iVGImage::Copy/1 **/
IDLC_METH_BEGIN(ni,iVGImage,Copy,1)
	IDLC_DECL_VAR(iVGImage*,apImage)
	IDLC_BUF_TO_INTF(iVGImage,apImage)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGImage,Copy,1,(apImage))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGImage,Copy,1)

/** ni -> iVGImage::Clone/0 **/
IDLC_METH_BEGIN(ni,iVGImage,Clone,0)
	IDLC_DECL_RETVAR(Ptr<iVGImage>,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGImage,Clone,0,())
	IDLC_RET_FROM_INTF(,_Ret)
IDLC_METH_END(ni,iVGImage,Clone,0)

/** ni -> iVGImage::GetHasBitmap/0 **/
IDLC_METH_BEGIN(ni,iVGImage,GetHasBitmap,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGImage,GetHasBitmap,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGImage,GetHasBitmap,0)

/** ni -> iVGImage::GrabBitmap/2 **/
IDLC_METH_BEGIN(ni,iVGImage,GrabBitmap,2)
	IDLC_DECL_VAR(eVGImageUsage,aLock)
	IDLC_BUF_TO_ENUM(eVGImageUsage,aLock)
	IDLC_DECL_VAR(sRecti,aDirtyRect)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4i|ni::eTypeFlags_Pointer,aDirtyRect)
	IDLC_DECL_RETVAR(iBitmap2D*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGImage,GrabBitmap,2,(aLock,aDirtyRect))
	IDLC_RET_FROM_INTF(iBitmap2D,_Ret)
IDLC_METH_END(ni,iVGImage,GrabBitmap,2)

/** ni -> iVGImage::GetHasTexture/0 **/
IDLC_METH_BEGIN(ni,iVGImage,GetHasTexture,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGImage,GetHasTexture,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGImage,GetHasTexture,0)

/** ni -> iVGImage::GetHasDepthStencil/0 **/
IDLC_METH_BEGIN(ni,iVGImage,GetHasDepthStencil,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGImage,GetHasDepthStencil,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGImage,GetHasDepthStencil,0)

/** ni -> iVGImage::GrabTexture/2 **/
IDLC_METH_BEGIN(ni,iVGImage,GrabTexture,2)
	IDLC_DECL_VAR(eVGImageUsage,aLock)
	IDLC_BUF_TO_ENUM(eVGImageUsage,aLock)
	IDLC_DECL_VAR(sRecti,aDirtyRect)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4i|ni::eTypeFlags_Pointer,aDirtyRect)
	IDLC_DECL_RETVAR(iTexture*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGImage,GrabTexture,2,(aLock,aDirtyRect))
	IDLC_RET_FROM_INTF(iTexture,_Ret)
IDLC_METH_END(ni,iVGImage,GrabTexture,2)

/** ni -> iVGImage::GetWidth/0 **/
IDLC_METH_BEGIN(ni,iVGImage,GetWidth,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGImage,GetWidth,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iVGImage,GetWidth,0)

/** ni -> iVGImage::GetHeight/0 **/
IDLC_METH_BEGIN(ni,iVGImage,GetHeight,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGImage,GetHeight,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iVGImage,GetHeight,0)

/** ni -> iVGImage::GetSize/0 **/
IDLC_METH_BEGIN(ni,iVGImage,GetSize,0)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGImage,GetSize,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iVGImage,GetSize,0)

/** ni -> iVGImage::Resize/2 **/
IDLC_METH_BEGIN(ni,iVGImage,Resize,2)
	IDLC_DECL_VAR(tU32,anNewWidth)
	IDLC_BUF_TO_BASE(ni::eType_U32,anNewWidth)
	IDLC_DECL_VAR(tU32,anNewHeight)
	IDLC_BUF_TO_BASE(ni::eType_U32,anNewHeight)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGImage,Resize,2,(anNewWidth,anNewHeight))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGImage,Resize,2)

IDLC_END_INTF(ni,iVGImage)

IDLC_END_NAMESPACE()
// EOF //
