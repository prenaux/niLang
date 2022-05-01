#include "../API/niUI/IGraphicsContext.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iGraphicsContext **/
IDLC_BEGIN_INTF(ni,iGraphicsContext)
/** ni -> iGraphicsContext::GetGraphics/0 **/
IDLC_METH_BEGIN(ni,iGraphicsContext,GetGraphics,0)
	IDLC_DECL_RETVAR(iGraphics*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsContext,GetGraphics,0,())
	IDLC_RET_FROM_INTF(iGraphics,_Ret)
IDLC_METH_END(ni,iGraphicsContext,GetGraphics,0)

/** ni -> iGraphicsContext::GetDriver/0 **/
IDLC_METH_BEGIN(ni,iGraphicsContext,GetDriver,0)
	IDLC_DECL_RETVAR(iGraphicsDriver*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsContext,GetDriver,0,())
	IDLC_RET_FROM_INTF(iGraphicsDriver,_Ret)
IDLC_METH_END(ni,iGraphicsContext,GetDriver,0)

/** ni -> iGraphicsContext::GetWidth/0 **/
IDLC_METH_BEGIN(ni,iGraphicsContext,GetWidth,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsContext,GetWidth,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iGraphicsContext,GetWidth,0)

/** ni -> iGraphicsContext::GetHeight/0 **/
IDLC_METH_BEGIN(ni,iGraphicsContext,GetHeight,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsContext,GetHeight,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iGraphicsContext,GetHeight,0)

/** ni -> iGraphicsContext::ClearBuffers/4 **/
IDLC_METH_BEGIN(ni,iGraphicsContext,ClearBuffers,4)
	IDLC_DECL_VAR(tClearBuffersFlags,clearBuffer)
	IDLC_BUF_TO_ENUM(tClearBuffersFlags,clearBuffer)
	IDLC_DECL_VAR(tU32,anColor)
	IDLC_BUF_TO_BASE(ni::eType_U32,anColor)
	IDLC_DECL_VAR(tF32,afDepth)
	IDLC_BUF_TO_BASE(ni::eType_F32,afDepth)
	IDLC_DECL_VAR(tI32,anStencil)
	IDLC_BUF_TO_BASE(ni::eType_I32,anStencil)
	IDLC_METH_CALL_VOID(ni,iGraphicsContext,ClearBuffers,4,(clearBuffer,anColor,afDepth,anStencil))
IDLC_METH_END(ni,iGraphicsContext,ClearBuffers,4)

/** ni -> iGraphicsContext::GetRenderTarget/1 **/
IDLC_METH_BEGIN(ni,iGraphicsContext,GetRenderTarget,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(iTexture*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsContext,GetRenderTarget,1,(anIndex))
	IDLC_RET_FROM_INTF(iTexture,_Ret)
IDLC_METH_END(ni,iGraphicsContext,GetRenderTarget,1)

/** ni -> iGraphicsContext::GetDepthStencil/0 **/
IDLC_METH_BEGIN(ni,iGraphicsContext,GetDepthStencil,0)
	IDLC_DECL_RETVAR(iTexture*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsContext,GetDepthStencil,0,())
	IDLC_RET_FROM_INTF(iTexture,_Ret)
IDLC_METH_END(ni,iGraphicsContext,GetDepthStencil,0)

/** ni -> iGraphicsContext::Display/2 **/
IDLC_METH_BEGIN(ni,iGraphicsContext,Display,2)
	IDLC_DECL_VAR(tGraphicsDisplayFlags,aFlags)
	IDLC_BUF_TO_ENUM(tGraphicsDisplayFlags,aFlags)
	IDLC_DECL_VAR(sRecti,aRect)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4i|ni::eTypeFlags_Pointer,aRect)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsContext,Display,2,(aFlags,aRect))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGraphicsContext,Display,2)

/** ni -> iGraphicsContext::DrawOperation/1 **/
IDLC_METH_BEGIN(ni,iGraphicsContext,DrawOperation,1)
	IDLC_DECL_VAR(iDrawOperation*,apDrawOp)
	IDLC_BUF_TO_INTF(iDrawOperation,apDrawOp)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsContext,DrawOperation,1,(apDrawOp))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGraphicsContext,DrawOperation,1)

/** ni -> iGraphicsContext::CaptureFrontBuffer/0 **/
IDLC_METH_BEGIN(ni,iGraphicsContext,CaptureFrontBuffer,0)
	IDLC_DECL_RETVAR(iBitmap2D*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsContext,CaptureFrontBuffer,0,())
	IDLC_RET_FROM_INTF(iBitmap2D,_Ret)
IDLC_METH_END(ni,iGraphicsContext,CaptureFrontBuffer,0)

/** ni -> iGraphicsContext::SetViewport/1 **/
IDLC_METH_BEGIN(ni,iGraphicsContext,SetViewport,1)
	IDLC_DECL_VAR(sRecti,aVal)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4i|ni::eTypeFlags_Pointer,aVal)
	IDLC_METH_CALL_VOID(ni,iGraphicsContext,SetViewport,1,(aVal))
IDLC_METH_END(ni,iGraphicsContext,SetViewport,1)

/** ni -> iGraphicsContext::GetViewport/0 **/
IDLC_METH_BEGIN(ni,iGraphicsContext,GetViewport,0)
	IDLC_DECL_RETVAR(sRecti,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsContext,GetViewport,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4i,_Ret)
IDLC_METH_END(ni,iGraphicsContext,GetViewport,0)

/** ni -> iGraphicsContext::SetScissorRect/1 **/
IDLC_METH_BEGIN(ni,iGraphicsContext,SetScissorRect,1)
	IDLC_DECL_VAR(sRecti,aVal)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4i|ni::eTypeFlags_Pointer,aVal)
	IDLC_METH_CALL_VOID(ni,iGraphicsContext,SetScissorRect,1,(aVal))
IDLC_METH_END(ni,iGraphicsContext,SetScissorRect,1)

/** ni -> iGraphicsContext::GetScissorRect/0 **/
IDLC_METH_BEGIN(ni,iGraphicsContext,GetScissorRect,0)
	IDLC_DECL_RETVAR(sRecti,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsContext,GetScissorRect,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4i,_Ret)
IDLC_METH_END(ni,iGraphicsContext,GetScissorRect,0)

/** ni -> iGraphicsContext::SetFixedStates/1 **/
IDLC_METH_BEGIN(ni,iGraphicsContext,SetFixedStates,1)
	IDLC_DECL_VAR(iFixedStates*,apStates)
	IDLC_BUF_TO_INTF(iFixedStates,apStates)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsContext,SetFixedStates,1,(apStates))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGraphicsContext,SetFixedStates,1)

/** ni -> iGraphicsContext::GetFixedStates/0 **/
IDLC_METH_BEGIN(ni,iGraphicsContext,GetFixedStates,0)
	IDLC_DECL_RETVAR(iFixedStates*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsContext,GetFixedStates,0,())
	IDLC_RET_FROM_INTF(iFixedStates,_Ret)
IDLC_METH_END(ni,iGraphicsContext,GetFixedStates,0)

/** ni -> iGraphicsContext::SetMaterial/1 **/
IDLC_METH_BEGIN(ni,iGraphicsContext,SetMaterial,1)
	IDLC_DECL_VAR(iMaterial*,apMat)
	IDLC_BUF_TO_INTF(iMaterial,apMat)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsContext,SetMaterial,1,(apMat))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGraphicsContext,SetMaterial,1)

/** ni -> iGraphicsContext::GetMaterial/0 **/
IDLC_METH_BEGIN(ni,iGraphicsContext,GetMaterial,0)
	IDLC_DECL_RETVAR(const iMaterial*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsContext,GetMaterial,0,())
	IDLC_RET_FROM_INTF(const iMaterial,_Ret)
IDLC_METH_END(ni,iGraphicsContext,GetMaterial,0)

IDLC_END_INTF(ni,iGraphicsContext)

/** interface : iGraphicsContextRT **/
IDLC_BEGIN_INTF(ni,iGraphicsContextRT)
/** ni -> iGraphicsContextRT::ChangeRenderTarget/2 **/
IDLC_METH_BEGIN(ni,iGraphicsContextRT,ChangeRenderTarget,2)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_VAR(iTexture*,apRT)
	IDLC_BUF_TO_INTF(iTexture,apRT)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsContextRT,ChangeRenderTarget,2,(anIndex,apRT))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGraphicsContextRT,ChangeRenderTarget,2)

/** ni -> iGraphicsContextRT::ChangeDepthStencil/1 **/
IDLC_METH_BEGIN(ni,iGraphicsContextRT,ChangeDepthStencil,1)
	IDLC_DECL_VAR(iTexture*,apDS)
	IDLC_BUF_TO_INTF(iTexture,apDS)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGraphicsContextRT,ChangeDepthStencil,1,(apDS))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGraphicsContextRT,ChangeDepthStencil,1)

IDLC_END_INTF(ni,iGraphicsContextRT)

IDLC_END_NAMESPACE()
// EOF //
