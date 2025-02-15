// clang-format off
//
// Autogenerated IDL definition for 'API/niUI/IGraphicsDriver.h'.
//
#include "../API/niUI/IGraphicsDriver.h"

#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif

IDLC_BEGIN_NAMESPACE()

/** NAMESPACE : ni **/
/** interface : iGraphicsDrawOpCapture **/
IDLC_BEGIN_INTF(ni,iGraphicsDrawOpCapture)
/** ni -> iGraphicsDrawOpCapture::BeginCapture/0 **/
IDLC_METH_BEGIN(ni,iGraphicsDrawOpCapture,BeginCapture,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDrawOpCapture,BeginCapture,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGraphicsDrawOpCapture,BeginCapture,0)

/** ni -> iGraphicsDrawOpCapture::EndCapture/0 **/
IDLC_METH_BEGIN(ni,iGraphicsDrawOpCapture,EndCapture,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDrawOpCapture,EndCapture,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iGraphicsDrawOpCapture,EndCapture,0)

/** ni -> iGraphicsDrawOpCapture::GetIsCapturing/0 **/
IDLC_METH_BEGIN(ni,iGraphicsDrawOpCapture,GetIsCapturing,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDrawOpCapture,GetIsCapturing,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGraphicsDrawOpCapture,GetIsCapturing,0)

/** ni -> iGraphicsDrawOpCapture::ClearCapture/0 **/
IDLC_METH_BEGIN(ni,iGraphicsDrawOpCapture,ClearCapture,0)
	IDLC_METH_CALL_VOID(ni,iGraphicsDrawOpCapture,ClearCapture,0,())
IDLC_METH_END(ni,iGraphicsDrawOpCapture,ClearCapture,0)

/** ni -> iGraphicsDrawOpCapture::SetCaptureFlags/1 **/
IDLC_METH_BEGIN(ni,iGraphicsDrawOpCapture,SetCaptureFlags,1)
	IDLC_DECL_VAR(tGraphicsCaptureFlags,aFlags)
	IDLC_BUF_TO_ENUM(tGraphicsCaptureFlags,aFlags)
	IDLC_METH_CALL_VOID(ni,iGraphicsDrawOpCapture,SetCaptureFlags,1,(aFlags))
IDLC_METH_END(ni,iGraphicsDrawOpCapture,SetCaptureFlags,1)

/** ni -> iGraphicsDrawOpCapture::GetCaptureFlags/0 **/
IDLC_METH_BEGIN(ni,iGraphicsDrawOpCapture,GetCaptureFlags,0)
	IDLC_DECL_RETVAR(tGraphicsCaptureFlags,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDrawOpCapture,GetCaptureFlags,0,())
	IDLC_RET_FROM_ENUM(tGraphicsCaptureFlags,_Ret)
IDLC_METH_END(ni,iGraphicsDrawOpCapture,GetCaptureFlags,0)

/** ni -> iGraphicsDrawOpCapture::SetCaptureStopAt/1 **/
IDLC_METH_BEGIN(ni,iGraphicsDrawOpCapture,SetCaptureStopAt,1)
	IDLC_DECL_VAR(tU32,anStopAt)
	IDLC_BUF_TO_BASE(ni::eType_U32,anStopAt)
	IDLC_METH_CALL_VOID(ni,iGraphicsDrawOpCapture,SetCaptureStopAt,1,(anStopAt))
IDLC_METH_END(ni,iGraphicsDrawOpCapture,SetCaptureStopAt,1)

/** ni -> iGraphicsDrawOpCapture::GetCaptureStopAt/0 **/
IDLC_METH_BEGIN(ni,iGraphicsDrawOpCapture,GetCaptureStopAt,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDrawOpCapture,GetCaptureStopAt,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iGraphicsDrawOpCapture,GetCaptureStopAt,0)

/** ni -> iGraphicsDrawOpCapture::GetNumCaptured/0 **/
IDLC_METH_BEGIN(ni,iGraphicsDrawOpCapture,GetNumCaptured,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDrawOpCapture,GetNumCaptured,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iGraphicsDrawOpCapture,GetNumCaptured,0)

/** ni -> iGraphicsDrawOpCapture::GetCapturedClear/1 **/
IDLC_METH_BEGIN(ni,iGraphicsDrawOpCapture,GetCapturedClear,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(sVec4i,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDrawOpCapture,GetCapturedClear,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_Vec4i,_Ret)
IDLC_METH_END(ni,iGraphicsDrawOpCapture,GetCapturedClear,1)

/** ni -> iGraphicsDrawOpCapture::GetCapturedDrawOp/1 **/
IDLC_METH_BEGIN(ni,iGraphicsDrawOpCapture,GetCapturedDrawOp,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(iDrawOperation*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDrawOpCapture,GetCapturedDrawOp,1,(anIndex))
	IDLC_RET_FROM_INTF(iDrawOperation,_Ret)
IDLC_METH_END(ni,iGraphicsDrawOpCapture,GetCapturedDrawOp,1)

/** ni -> iGraphicsDrawOpCapture::GetCapturedDrawOpTime/1 **/
IDLC_METH_BEGIN(ni,iGraphicsDrawOpCapture,GetCapturedDrawOpTime,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDrawOpCapture,GetCapturedDrawOpTime,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iGraphicsDrawOpCapture,GetCapturedDrawOpTime,1)

/** ni -> iGraphicsDrawOpCapture::GetCapturedDrawOpContext/1 **/
IDLC_METH_BEGIN(ni,iGraphicsDrawOpCapture,GetCapturedDrawOpContext,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(iGraphicsContext*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDrawOpCapture,GetCapturedDrawOpContext,1,(anIndex))
	IDLC_RET_FROM_INTF(iGraphicsContext,_Ret)
IDLC_METH_END(ni,iGraphicsDrawOpCapture,GetCapturedDrawOpContext,1)

/** ni -> iGraphicsDrawOpCapture::BeginCaptureDrawOp/3 **/
IDLC_METH_BEGIN(ni,iGraphicsDrawOpCapture,BeginCaptureDrawOp,3)
	IDLC_DECL_VAR(iGraphicsContext*,apContext)
	IDLC_BUF_TO_INTF(iGraphicsContext,apContext)
	IDLC_DECL_VAR(iDrawOperation*,apDrawOp)
	IDLC_BUF_TO_INTF(iDrawOperation,apDrawOp)
	IDLC_DECL_VAR(sVec4i,aClearParams)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4i|ni::eTypeFlags_Pointer,aClearParams)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDrawOpCapture,BeginCaptureDrawOp,3,(apContext,apDrawOp,aClearParams))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGraphicsDrawOpCapture,BeginCaptureDrawOp,3)

/** ni -> iGraphicsDrawOpCapture::EndCaptureDrawOp/3 **/
IDLC_METH_BEGIN(ni,iGraphicsDrawOpCapture,EndCaptureDrawOp,3)
	IDLC_DECL_VAR(iGraphicsContext*,apContext)
	IDLC_BUF_TO_INTF(iGraphicsContext,apContext)
	IDLC_DECL_VAR(iDrawOperation*,apDrawOp)
	IDLC_BUF_TO_INTF(iDrawOperation,apDrawOp)
	IDLC_DECL_VAR(sVec4i,aClearParams)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4i|ni::eTypeFlags_Pointer,aClearParams)
	IDLC_METH_CALL_VOID(ni,iGraphicsDrawOpCapture,EndCaptureDrawOp,3,(apContext,apDrawOp,aClearParams))
IDLC_METH_END(ni,iGraphicsDrawOpCapture,EndCaptureDrawOp,3)

IDLC_END_INTF(ni,iGraphicsDrawOpCapture)

/** interface : iGraphicsDriver **/
IDLC_BEGIN_INTF(ni,iGraphicsDriver)
/** ni -> iGraphicsDriver::GetGraphics/0 **/
IDLC_METH_BEGIN(ni,iGraphicsDriver,GetGraphics,0)
	IDLC_DECL_RETVAR(iGraphics*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDriver,GetGraphics,0,())
	IDLC_RET_FROM_INTF(iGraphics,_Ret)
IDLC_METH_END(ni,iGraphicsDriver,GetGraphics,0)

/** ni -> iGraphicsDriver::GetName/0 **/
IDLC_METH_BEGIN(ni,iGraphicsDriver,GetName,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDriver,GetName,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iGraphicsDriver,GetName,0)

/** ni -> iGraphicsDriver::GetDesc/0 **/
IDLC_METH_BEGIN(ni,iGraphicsDriver,GetDesc,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDriver,GetDesc,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iGraphicsDriver,GetDesc,0)

/** ni -> iGraphicsDriver::GetDeviceName/0 **/
IDLC_METH_BEGIN(ni,iGraphicsDriver,GetDeviceName,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDriver,GetDeviceName,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iGraphicsDriver,GetDeviceName,0)

/** ni -> iGraphicsDriver::GetCaps/1 **/
IDLC_METH_BEGIN(ni,iGraphicsDriver,GetCaps,1)
	IDLC_DECL_VAR(eGraphicsCaps,aCaps)
	IDLC_BUF_TO_ENUM(eGraphicsCaps,aCaps)
	IDLC_DECL_RETVAR(tInt,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDriver,GetCaps,1,(aCaps))
	IDLC_RET_FROM_BASE(ni::eType_Int,_Ret)
IDLC_METH_END(ni,iGraphicsDriver,GetCaps,1)

/** ni -> iGraphicsDriver::GetGraphicsDriverImplFlags/0 **/
IDLC_METH_BEGIN(ni,iGraphicsDriver,GetGraphicsDriverImplFlags,0)
	IDLC_DECL_RETVAR(tGraphicsDriverImplFlags,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDriver,GetGraphicsDriverImplFlags,0,())
	IDLC_RET_FROM_ENUM(tGraphicsDriverImplFlags,_Ret)
IDLC_METH_END(ni,iGraphicsDriver,GetGraphicsDriverImplFlags,0)

/** ni -> iGraphicsDriver::SetDrawOpCapture/1 **/
IDLC_METH_BEGIN(ni,iGraphicsDriver,SetDrawOpCapture,1)
	IDLC_DECL_VAR(iGraphicsDrawOpCapture*,apCapture)
	IDLC_BUF_TO_INTF(iGraphicsDrawOpCapture,apCapture)
	IDLC_METH_CALL_VOID(ni,iGraphicsDriver,SetDrawOpCapture,1,(apCapture))
IDLC_METH_END(ni,iGraphicsDriver,SetDrawOpCapture,1)

/** ni -> iGraphicsDriver::GetDrawOpCapture/0 **/
IDLC_METH_BEGIN(ni,iGraphicsDriver,GetDrawOpCapture,0)
	IDLC_DECL_RETVAR(iGraphicsDrawOpCapture*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDriver,GetDrawOpCapture,0,())
	IDLC_RET_FROM_INTF(iGraphicsDrawOpCapture,_Ret)
IDLC_METH_END(ni,iGraphicsDriver,GetDrawOpCapture,0)

/** ni -> iGraphicsDriver::CreateContextForWindow/5 **/
IDLC_METH_BEGIN(ni,iGraphicsDriver,CreateContextForWindow,5)
	IDLC_DECL_VAR(iOSWindow*,apWindow)
	IDLC_BUF_TO_INTF(iOSWindow,apWindow)
	IDLC_DECL_VAR(achar*,aaszBBFormat)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszBBFormat)
	IDLC_DECL_VAR(achar*,aaszDSFormat)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszDSFormat)
	IDLC_DECL_VAR(tU32,anSwapInterval)
	IDLC_BUF_TO_BASE(ni::eType_U32,anSwapInterval)
	IDLC_DECL_VAR(tTextureFlags,aBackBufferFlags)
	IDLC_BUF_TO_ENUM(tTextureFlags,aBackBufferFlags)
	IDLC_DECL_RETVAR(iGraphicsContext*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDriver,CreateContextForWindow,5,(apWindow,aaszBBFormat,aaszDSFormat,anSwapInterval,aBackBufferFlags))
	IDLC_RET_FROM_INTF(iGraphicsContext,_Ret)
IDLC_METH_END(ni,iGraphicsDriver,CreateContextForWindow,5)

/** ni -> iGraphicsDriver::CreateContextForRenderTargets/5 **/
IDLC_METH_BEGIN(ni,iGraphicsDriver,CreateContextForRenderTargets,5)
	IDLC_DECL_VAR(iTexture*,apRT0)
	IDLC_BUF_TO_INTF(iTexture,apRT0)
	IDLC_DECL_VAR(iTexture*,apRT1)
	IDLC_BUF_TO_INTF(iTexture,apRT1)
	IDLC_DECL_VAR(iTexture*,apRT2)
	IDLC_BUF_TO_INTF(iTexture,apRT2)
	IDLC_DECL_VAR(iTexture*,apRT3)
	IDLC_BUF_TO_INTF(iTexture,apRT3)
	IDLC_DECL_VAR(iTexture*,apDS)
	IDLC_BUF_TO_INTF(iTexture,apDS)
	IDLC_DECL_RETVAR(iGraphicsContextRT*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDriver,CreateContextForRenderTargets,5,(apRT0,apRT1,apRT2,apRT3,apDS))
	IDLC_RET_FROM_INTF(iGraphicsContextRT,_Ret)
IDLC_METH_END(ni,iGraphicsDriver,CreateContextForRenderTargets,5)

/** ni -> iGraphicsDriver::ResetAllCaches/0 **/
IDLC_METH_BEGIN(ni,iGraphicsDriver,ResetAllCaches,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDriver,ResetAllCaches,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGraphicsDriver,ResetAllCaches,0)

/** ni -> iGraphicsDriver::CheckTextureFormat/2 **/
IDLC_METH_BEGIN(ni,iGraphicsDriver,CheckTextureFormat,2)
	IDLC_DECL_VAR(iBitmapFormat*,apFormat)
	IDLC_BUF_TO_INTF(iBitmapFormat,apFormat)
	IDLC_DECL_VAR(tTextureFlags,aFlags)
	IDLC_BUF_TO_ENUM(tTextureFlags,aFlags)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDriver,CheckTextureFormat,2,(apFormat,aFlags))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGraphicsDriver,CheckTextureFormat,2)

/** ni -> iGraphicsDriver::CreateTexture/8 **/
IDLC_METH_BEGIN(ni,iGraphicsDriver,CreateTexture,8)
	IDLC_DECL_VAR(iHString*,ahspName)
	IDLC_BUF_TO_INTF(iHString,ahspName)
	IDLC_DECL_VAR(eBitmapType,aType)
	IDLC_BUF_TO_ENUM(eBitmapType,aType)
	IDLC_DECL_VAR(achar*,aaszFormat)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszFormat)
	IDLC_DECL_VAR(tU32,anNumMipMaps)
	IDLC_BUF_TO_BASE(ni::eType_U32,anNumMipMaps)
	IDLC_DECL_VAR(tU32,anWidth)
	IDLC_BUF_TO_BASE(ni::eType_U32,anWidth)
	IDLC_DECL_VAR(tU32,anHeight)
	IDLC_BUF_TO_BASE(ni::eType_U32,anHeight)
	IDLC_DECL_VAR(tU32,anDepth)
	IDLC_BUF_TO_BASE(ni::eType_U32,anDepth)
	IDLC_DECL_VAR(tTextureFlags,aFlags)
	IDLC_BUF_TO_ENUM(tTextureFlags,aFlags)
	IDLC_DECL_RETVAR(iTexture*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDriver,CreateTexture,8,(ahspName,aType,aaszFormat,anNumMipMaps,anWidth,anHeight,anDepth,aFlags))
	IDLC_RET_FROM_INTF(iTexture,_Ret)
IDLC_METH_END(ni,iGraphicsDriver,CreateTexture,8)

/** ni -> iGraphicsDriver::BlitBitmapToTexture/6 **/
IDLC_METH_BEGIN(ni,iGraphicsDriver,BlitBitmapToTexture,6)
	IDLC_DECL_VAR(iBitmap2D*,apSrc)
	IDLC_BUF_TO_INTF(iBitmap2D,apSrc)
	IDLC_DECL_VAR(iTexture*,apDest)
	IDLC_BUF_TO_INTF(iTexture,apDest)
	IDLC_DECL_VAR(tU32,anDestLevel)
	IDLC_BUF_TO_BASE(ni::eType_U32,anDestLevel)
	IDLC_DECL_VAR(sRecti,aSrcRect)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4i|ni::eTypeFlags_Pointer,aSrcRect)
	IDLC_DECL_VAR(sRecti,aDestRect)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4i|ni::eTypeFlags_Pointer,aDestRect)
	IDLC_DECL_VAR(eTextureBlitFlags,aFlags)
	IDLC_BUF_TO_ENUM(eTextureBlitFlags,aFlags)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDriver,BlitBitmapToTexture,6,(apSrc,apDest,anDestLevel,aSrcRect,aDestRect,aFlags))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGraphicsDriver,BlitBitmapToTexture,6)

/** ni -> iGraphicsDriver::BlitTextureToBitmap/6 **/
IDLC_METH_BEGIN(ni,iGraphicsDriver,BlitTextureToBitmap,6)
	IDLC_DECL_VAR(iTexture*,apSrc)
	IDLC_BUF_TO_INTF(iTexture,apSrc)
	IDLC_DECL_VAR(tU32,anSrcLevel)
	IDLC_BUF_TO_BASE(ni::eType_U32,anSrcLevel)
	IDLC_DECL_VAR(iBitmap2D*,apDest)
	IDLC_BUF_TO_INTF(iBitmap2D,apDest)
	IDLC_DECL_VAR(sRecti,aSrcRect)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4i|ni::eTypeFlags_Pointer,aSrcRect)
	IDLC_DECL_VAR(sRecti,aDestRect)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4i|ni::eTypeFlags_Pointer,aDestRect)
	IDLC_DECL_VAR(eTextureBlitFlags,aFlags)
	IDLC_BUF_TO_ENUM(eTextureBlitFlags,aFlags)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDriver,BlitTextureToBitmap,6,(apSrc,anSrcLevel,apDest,aSrcRect,aDestRect,aFlags))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGraphicsDriver,BlitTextureToBitmap,6)

/** ni -> iGraphicsDriver::BlitTextureToTexture/7 **/
IDLC_METH_BEGIN(ni,iGraphicsDriver,BlitTextureToTexture,7)
	IDLC_DECL_VAR(iTexture*,apSrc)
	IDLC_BUF_TO_INTF(iTexture,apSrc)
	IDLC_DECL_VAR(tU32,anSrcLevel)
	IDLC_BUF_TO_BASE(ni::eType_U32,anSrcLevel)
	IDLC_DECL_VAR(iTexture*,apDest)
	IDLC_BUF_TO_INTF(iTexture,apDest)
	IDLC_DECL_VAR(tU32,anDestLevel)
	IDLC_BUF_TO_BASE(ni::eType_U32,anDestLevel)
	IDLC_DECL_VAR(sRecti,aSrcRect)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4i|ni::eTypeFlags_Pointer,aSrcRect)
	IDLC_DECL_VAR(sRecti,aDestRect)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4i|ni::eTypeFlags_Pointer,aDestRect)
	IDLC_DECL_VAR(eTextureBlitFlags,aFlags)
	IDLC_BUF_TO_ENUM(eTextureBlitFlags,aFlags)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDriver,BlitTextureToTexture,7,(apSrc,anSrcLevel,apDest,anDestLevel,aSrcRect,aDestRect,aFlags))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGraphicsDriver,BlitTextureToTexture,7)

/** ni -> iGraphicsDriver::BlitBitmap3DToTexture/7 **/
IDLC_METH_BEGIN(ni,iGraphicsDriver,BlitBitmap3DToTexture,7)
	IDLC_DECL_VAR(iBitmap3D*,apSrc)
	IDLC_BUF_TO_INTF(iBitmap3D,apSrc)
	IDLC_DECL_VAR(iTexture*,apDest)
	IDLC_BUF_TO_INTF(iTexture,apDest)
	IDLC_DECL_VAR(tU32,anDestLevel)
	IDLC_BUF_TO_BASE(ni::eType_U32,anDestLevel)
	IDLC_DECL_VAR(sVec3i,aSrcMin)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3i|ni::eTypeFlags_Pointer,aSrcMin)
	IDLC_DECL_VAR(sVec3i,aDestMin)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3i|ni::eTypeFlags_Pointer,aDestMin)
	IDLC_DECL_VAR(sVec3i,avSize)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3i|ni::eTypeFlags_Pointer,avSize)
	IDLC_DECL_VAR(eTextureBlitFlags,aFlags)
	IDLC_BUF_TO_ENUM(eTextureBlitFlags,aFlags)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDriver,BlitBitmap3DToTexture,7,(apSrc,apDest,anDestLevel,aSrcMin,aDestMin,avSize,aFlags))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGraphicsDriver,BlitBitmap3DToTexture,7)

/** ni -> iGraphicsDriver::BlitTextureToBitmap3D/7 **/
IDLC_METH_BEGIN(ni,iGraphicsDriver,BlitTextureToBitmap3D,7)
	IDLC_DECL_VAR(iTexture*,apSrc)
	IDLC_BUF_TO_INTF(iTexture,apSrc)
	IDLC_DECL_VAR(tU32,anSrcLevel)
	IDLC_BUF_TO_BASE(ni::eType_U32,anSrcLevel)
	IDLC_DECL_VAR(iBitmap3D*,apDest)
	IDLC_BUF_TO_INTF(iBitmap3D,apDest)
	IDLC_DECL_VAR(sVec3i,aSrcMin)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3i|ni::eTypeFlags_Pointer,aSrcMin)
	IDLC_DECL_VAR(sVec3i,aDestMin)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3i|ni::eTypeFlags_Pointer,aDestMin)
	IDLC_DECL_VAR(sVec3i,avSize)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3i|ni::eTypeFlags_Pointer,avSize)
	IDLC_DECL_VAR(eTextureBlitFlags,aFlags)
	IDLC_BUF_TO_ENUM(eTextureBlitFlags,aFlags)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDriver,BlitTextureToBitmap3D,7,(apSrc,anSrcLevel,apDest,aSrcMin,aDestMin,avSize,aFlags))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGraphicsDriver,BlitTextureToBitmap3D,7)

/** ni -> iGraphicsDriver::GetNumShaderProfile/1 **/
IDLC_METH_BEGIN(ni,iGraphicsDriver,GetNumShaderProfile,1)
	IDLC_DECL_VAR(eShaderUnit,aUnit)
	IDLC_BUF_TO_ENUM(eShaderUnit,aUnit)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDriver,GetNumShaderProfile,1,(aUnit))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iGraphicsDriver,GetNumShaderProfile,1)

/** ni -> iGraphicsDriver::GetShaderProfile/2 **/
IDLC_METH_BEGIN(ni,iGraphicsDriver,GetShaderProfile,2)
	IDLC_DECL_VAR(eShaderUnit,aUnit)
	IDLC_BUF_TO_ENUM(eShaderUnit,aUnit)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDriver,GetShaderProfile,2,(aUnit,anIndex))
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iGraphicsDriver,GetShaderProfile,2)

/** ni -> iGraphicsDriver::CreateShader/2 **/
IDLC_METH_BEGIN(ni,iGraphicsDriver,CreateShader,2)
	IDLC_DECL_VAR(iHString*,ahspName)
	IDLC_BUF_TO_INTF(iHString,ahspName)
	IDLC_DECL_VAR(iFile*,apByteCode)
	IDLC_BUF_TO_INTF(iFile,apByteCode)
	IDLC_DECL_RETVAR(iShader*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDriver,CreateShader,2,(ahspName,apByteCode))
	IDLC_RET_FROM_INTF(iShader,_Ret)
IDLC_METH_END(ni,iGraphicsDriver,CreateShader,2)

/** ni -> iGraphicsDriver::CreateOcclusionQuery/0 **/
IDLC_METH_BEGIN(ni,iGraphicsDriver,CreateOcclusionQuery,0)
	IDLC_DECL_RETVAR(iOcclusionQuery*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDriver,CreateOcclusionQuery,0,())
	IDLC_RET_FROM_INTF(iOcclusionQuery,_Ret)
IDLC_METH_END(ni,iGraphicsDriver,CreateOcclusionQuery,0)

/** ni -> iGraphicsDriver::CreateVertexArray/3 **/
IDLC_METH_BEGIN(ni,iGraphicsDriver,CreateVertexArray,3)
	IDLC_DECL_VAR(tU32,anNumVertices)
	IDLC_BUF_TO_BASE(ni::eType_U32,anNumVertices)
	IDLC_DECL_VAR(tFVF,anFVF)
	IDLC_BUF_TO_BASE(ni::eType_U32,anFVF)
	IDLC_DECL_VAR(eArrayUsage,aUsage)
	IDLC_BUF_TO_ENUM(eArrayUsage,aUsage)
	IDLC_DECL_RETVAR(iVertexArray*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDriver,CreateVertexArray,3,(anNumVertices,anFVF,aUsage))
	IDLC_RET_FROM_INTF(iVertexArray,_Ret)
IDLC_METH_END(ni,iGraphicsDriver,CreateVertexArray,3)

/** ni -> iGraphicsDriver::CreateIndexArray/4 **/
IDLC_METH_BEGIN(ni,iGraphicsDriver,CreateIndexArray,4)
	IDLC_DECL_VAR(eGraphicsPrimitiveType,aPrimitiveType)
	IDLC_BUF_TO_ENUM(eGraphicsPrimitiveType,aPrimitiveType)
	IDLC_DECL_VAR(tU32,anNumIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anNumIndex)
	IDLC_DECL_VAR(tU32,anMaxVertexIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anMaxVertexIndex)
	IDLC_DECL_VAR(eArrayUsage,aUsage)
	IDLC_BUF_TO_ENUM(eArrayUsage,aUsage)
	IDLC_DECL_RETVAR(iIndexArray*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsDriver,CreateIndexArray,4,(aPrimitiveType,anNumIndex,anMaxVertexIndex,aUsage))
	IDLC_RET_FROM_INTF(iIndexArray,_Ret)
IDLC_METH_END(ni,iGraphicsDriver,CreateIndexArray,4)

IDLC_END_INTF(ni,iGraphicsDriver)

IDLC_END_NAMESPACE()
// EOF //
