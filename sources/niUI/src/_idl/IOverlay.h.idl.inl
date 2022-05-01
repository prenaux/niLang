#include "../API/niUI/IOverlay.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iOverlay **/
IDLC_BEGIN_INTF(ni,iOverlay)
/** ni -> iOverlay::Clone/0 **/
IDLC_METH_BEGIN(ni,iOverlay,Clone,0)
	IDLC_DECL_RETVAR(iOverlay*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOverlay,Clone,0,())
	IDLC_RET_FROM_INTF(iOverlay,_Ret)
IDLC_METH_END(ni,iOverlay,Clone,0)

/** ni -> iOverlay::GetMaterial/0 **/
IDLC_METH_BEGIN(ni,iOverlay,GetMaterial,0)
	IDLC_DECL_RETVAR(iMaterial*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOverlay,GetMaterial,0,())
	IDLC_RET_FROM_INTF(iMaterial,_Ret)
IDLC_METH_END(ni,iOverlay,GetMaterial,0)

/** ni -> iOverlay::GetImage/0 **/
IDLC_METH_BEGIN(ni,iOverlay,GetImage,0)
	IDLC_DECL_RETVAR(iImage*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOverlay,GetImage,0,())
	IDLC_RET_FROM_INTF(iImage,_Ret)
IDLC_METH_END(ni,iOverlay,GetImage,0)

/** ni -> iOverlay::GetBaseSize/0 **/
IDLC_METH_BEGIN(ni,iOverlay,GetBaseSize,0)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOverlay,GetBaseSize,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iOverlay,GetBaseSize,0)

/** ni -> iOverlay::SetPivot/1 **/
IDLC_METH_BEGIN(ni,iOverlay,SetPivot,1)
	IDLC_DECL_VAR(sVec2f,avPivot)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,avPivot)
	IDLC_METH_CALL_VOID(ni,iOverlay,SetPivot,1,(avPivot))
IDLC_METH_END(ni,iOverlay,SetPivot,1)

/** ni -> iOverlay::GetPivot/0 **/
IDLC_METH_BEGIN(ni,iOverlay,GetPivot,0)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOverlay,GetPivot,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iOverlay,GetPivot,0)

/** ni -> iOverlay::SetSize/1 **/
IDLC_METH_BEGIN(ni,iOverlay,SetSize,1)
	IDLC_DECL_VAR(sVec2f,avSize)
	IDLC_BUF_TO_BASE(ni::eType_Vec2f,avSize)
	IDLC_METH_CALL_VOID(ni,iOverlay,SetSize,1,(avSize))
IDLC_METH_END(ni,iOverlay,SetSize,1)

/** ni -> iOverlay::GetSize/0 **/
IDLC_METH_BEGIN(ni,iOverlay,GetSize,0)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOverlay,GetSize,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iOverlay,GetSize,0)

/** ni -> iOverlay::SetBlendMode/1 **/
IDLC_METH_BEGIN(ni,iOverlay,SetBlendMode,1)
	IDLC_DECL_VAR(eBlendMode,aBlendMode)
	IDLC_BUF_TO_ENUM(eBlendMode,aBlendMode)
	IDLC_METH_CALL_VOID(ni,iOverlay,SetBlendMode,1,(aBlendMode))
IDLC_METH_END(ni,iOverlay,SetBlendMode,1)

/** ni -> iOverlay::GetBlendMode/0 **/
IDLC_METH_BEGIN(ni,iOverlay,GetBlendMode,0)
	IDLC_DECL_RETVAR(eBlendMode,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOverlay,GetBlendMode,0,())
	IDLC_RET_FROM_ENUM(eBlendMode,_Ret)
IDLC_METH_END(ni,iOverlay,GetBlendMode,0)

/** ni -> iOverlay::SetFiltering/1 **/
IDLC_METH_BEGIN(ni,iOverlay,SetFiltering,1)
	IDLC_DECL_VAR(tBool,abEnabled)
	IDLC_BUF_TO_BASE(ni::eType_I8,abEnabled)
	IDLC_METH_CALL_VOID(ni,iOverlay,SetFiltering,1,(abEnabled))
IDLC_METH_END(ni,iOverlay,SetFiltering,1)

/** ni -> iOverlay::GetFiltering/0 **/
IDLC_METH_BEGIN(ni,iOverlay,GetFiltering,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOverlay,GetFiltering,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iOverlay,GetFiltering,0)

/** ni -> iOverlay::SetColor/1 **/
IDLC_METH_BEGIN(ni,iOverlay,SetColor,1)
	IDLC_DECL_VAR(sColor4f,aColor)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,aColor)
	IDLC_METH_CALL_VOID(ni,iOverlay,SetColor,1,(aColor))
IDLC_METH_END(ni,iOverlay,SetColor,1)

/** ni -> iOverlay::GetColor/0 **/
IDLC_METH_BEGIN(ni,iOverlay,GetColor,0)
	IDLC_DECL_RETVAR(sColor4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOverlay,GetColor,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iOverlay,GetColor,0)

/** ni -> iOverlay::SetCornerColor/2 **/
IDLC_METH_BEGIN(ni,iOverlay,SetCornerColor,2)
	IDLC_DECL_VAR(eRectCorners,aCorner)
	IDLC_BUF_TO_ENUM(eRectCorners,aCorner)
	IDLC_DECL_VAR(sColor4f,aColor)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,aColor)
	IDLC_METH_CALL_VOID(ni,iOverlay,SetCornerColor,2,(aCorner,aColor))
IDLC_METH_END(ni,iOverlay,SetCornerColor,2)

/** ni -> iOverlay::GetCornerColor/1 **/
IDLC_METH_BEGIN(ni,iOverlay,GetCornerColor,1)
	IDLC_DECL_VAR(eRectCorners,aCorner)
	IDLC_BUF_TO_ENUM(eRectCorners,aCorner)
	IDLC_DECL_RETVAR(sColor4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOverlay,GetCornerColor,1,(aCorner))
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iOverlay,GetCornerColor,1)

/** ni -> iOverlay::SetMapping/1 **/
IDLC_METH_BEGIN(ni,iOverlay,SetMapping,1)
	IDLC_DECL_VAR(sRectf,aRect)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,aRect)
	IDLC_METH_CALL_VOID(ni,iOverlay,SetMapping,1,(aRect))
IDLC_METH_END(ni,iOverlay,SetMapping,1)

/** ni -> iOverlay::GetMapping/0 **/
IDLC_METH_BEGIN(ni,iOverlay,GetMapping,0)
	IDLC_DECL_RETVAR(sRectf,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOverlay,GetMapping,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iOverlay,GetMapping,0)

/** ni -> iOverlay::SetFrame/1 **/
IDLC_METH_BEGIN(ni,iOverlay,SetFrame,1)
	IDLC_DECL_VAR(sVec4f,aFrameBorder)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,aFrameBorder)
	IDLC_METH_CALL_VOID(ni,iOverlay,SetFrame,1,(aFrameBorder))
IDLC_METH_END(ni,iOverlay,SetFrame,1)

/** ni -> iOverlay::GetFrame/0 **/
IDLC_METH_BEGIN(ni,iOverlay,GetFrame,0)
	IDLC_DECL_RETVAR(sVec4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOverlay,GetFrame,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iOverlay,GetFrame,0)

/** ni -> iOverlay::GetIsFrame/0 **/
IDLC_METH_BEGIN(ni,iOverlay,GetIsFrame,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOverlay,GetIsFrame,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iOverlay,GetIsFrame,0)

/** ni -> iOverlay::ComputeFrameCenter/1 **/
IDLC_METH_BEGIN(ni,iOverlay,ComputeFrameCenter,1)
	IDLC_DECL_VAR(sRectf,aDest)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,aDest)
	IDLC_DECL_RETVAR(sRectf,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOverlay,ComputeFrameCenter,1,(aDest))
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iOverlay,ComputeFrameCenter,1)

/** ni -> iOverlay::Draw/3 **/
IDLC_METH_BEGIN(ni,iOverlay,Draw,3)
	IDLC_DECL_VAR(iCanvas*,apCanvas)
	IDLC_BUF_TO_INTF(iCanvas,apCanvas)
	IDLC_DECL_VAR(sVec2f,aPos)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,aPos)
	IDLC_DECL_VAR(sVec2f,aSize)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,aSize)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOverlay,Draw,3,(apCanvas,aPos,aSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iOverlay,Draw,3)

/** ni -> iOverlay::DrawFrame/4 **/
IDLC_METH_BEGIN(ni,iOverlay,DrawFrame,4)
	IDLC_DECL_VAR(iCanvas*,apCanvas)
	IDLC_BUF_TO_INTF(iCanvas,apCanvas)
	IDLC_DECL_VAR(tRectFrameFlags,aFrame)
	IDLC_BUF_TO_ENUM(tRectFrameFlags,aFrame)
	IDLC_DECL_VAR(sVec2f,aPos)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,aPos)
	IDLC_DECL_VAR(sVec2f,aSize)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,aSize)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOverlay,DrawFrame,4,(apCanvas,aFrame,aPos,aSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iOverlay,DrawFrame,4)

IDLC_END_INTF(ni,iOverlay)

IDLC_END_NAMESPACE()
// EOF //
