#include "../API/niUI/IMaterial.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iMaterial **/
IDLC_BEGIN_INTF(ni,iMaterial)
/** ni -> iMaterial::GetGraphics/0 **/
IDLC_METH_BEGIN(ni,iMaterial,GetGraphics,0)
	IDLC_DECL_RETVAR(iGraphics*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetGraphics,0,())
	IDLC_RET_FROM_INTF(iGraphics,_Ret)
IDLC_METH_END(ni,iMaterial,GetGraphics,0)

/** ni -> iMaterial::SetName/1 **/
IDLC_METH_BEGIN(ni,iMaterial,SetName,1)
	IDLC_DECL_VAR(iHString*,ahspName)
	IDLC_BUF_TO_INTF(iHString,ahspName)
	IDLC_METH_CALL_VOID(ni,iMaterial,SetName,1,(ahspName))
IDLC_METH_END(ni,iMaterial,SetName,1)

/** ni -> iMaterial::GetName/0 **/
IDLC_METH_BEGIN(ni,iMaterial,GetName,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetName,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iMaterial,GetName,0)

/** ni -> iMaterial::SetClass/1 **/
IDLC_METH_BEGIN(ni,iMaterial,SetClass,1)
	IDLC_DECL_VAR(iHString*,ahspName)
	IDLC_BUF_TO_INTF(iHString,ahspName)
	IDLC_METH_CALL_VOID(ni,iMaterial,SetClass,1,(ahspName))
IDLC_METH_END(ni,iMaterial,SetClass,1)

/** ni -> iMaterial::GetClass/0 **/
IDLC_METH_BEGIN(ni,iMaterial,GetClass,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetClass,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iMaterial,GetClass,0)

/** ni -> iMaterial::GetWidth/0 **/
IDLC_METH_BEGIN(ni,iMaterial,GetWidth,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetWidth,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iMaterial,GetWidth,0)

/** ni -> iMaterial::GetHeight/0 **/
IDLC_METH_BEGIN(ni,iMaterial,GetHeight,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetHeight,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iMaterial,GetHeight,0)

/** ni -> iMaterial::GetDepth/0 **/
IDLC_METH_BEGIN(ni,iMaterial,GetDepth,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetDepth,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iMaterial,GetDepth,0)

/** ni -> iMaterial::SetFlags/1 **/
IDLC_METH_BEGIN(ni,iMaterial,SetFlags,1)
	IDLC_DECL_VAR(tMaterialFlags,aFlags)
	IDLC_BUF_TO_ENUM(tMaterialFlags,aFlags)
	IDLC_METH_CALL_VOID(ni,iMaterial,SetFlags,1,(aFlags))
IDLC_METH_END(ni,iMaterial,SetFlags,1)

/** ni -> iMaterial::GetFlags/0 **/
IDLC_METH_BEGIN(ni,iMaterial,GetFlags,0)
	IDLC_DECL_RETVAR(tMaterialFlags,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetFlags,0,())
	IDLC_RET_FROM_ENUM(tMaterialFlags,_Ret)
IDLC_METH_END(ni,iMaterial,GetFlags,0)

/** ni -> iMaterial::SetBlendMode/1 **/
IDLC_METH_BEGIN(ni,iMaterial,SetBlendMode,1)
	IDLC_DECL_VAR(eBlendMode,aBlendMode)
	IDLC_BUF_TO_ENUM(eBlendMode,aBlendMode)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,SetBlendMode,1,(aBlendMode))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMaterial,SetBlendMode,1)

/** ni -> iMaterial::GetBlendMode/0 **/
IDLC_METH_BEGIN(ni,iMaterial,GetBlendMode,0)
	IDLC_DECL_RETVAR(eBlendMode,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetBlendMode,0,())
	IDLC_RET_FROM_ENUM(eBlendMode,_Ret)
IDLC_METH_END(ni,iMaterial,GetBlendMode,0)

/** ni -> iMaterial::SetRasterizerStates/1 **/
IDLC_METH_BEGIN(ni,iMaterial,SetRasterizerStates,1)
	IDLC_DECL_VAR(tIntPtr,aHandle)
	IDLC_BUF_TO_BASE(ni::eType_IntPtr,aHandle)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,SetRasterizerStates,1,(aHandle))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMaterial,SetRasterizerStates,1)

/** ni -> iMaterial::GetRasterizerStates/0 **/
IDLC_METH_BEGIN(ni,iMaterial,GetRasterizerStates,0)
	IDLC_DECL_RETVAR(tIntPtr,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetRasterizerStates,0,())
	IDLC_RET_FROM_BASE(ni::eType_IntPtr,_Ret)
IDLC_METH_END(ni,iMaterial,GetRasterizerStates,0)

/** ni -> iMaterial::SetDepthStencilStates/1 **/
IDLC_METH_BEGIN(ni,iMaterial,SetDepthStencilStates,1)
	IDLC_DECL_VAR(tIntPtr,aHandle)
	IDLC_BUF_TO_BASE(ni::eType_IntPtr,aHandle)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,SetDepthStencilStates,1,(aHandle))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMaterial,SetDepthStencilStates,1)

/** ni -> iMaterial::GetDepthStencilStates/0 **/
IDLC_METH_BEGIN(ni,iMaterial,GetDepthStencilStates,0)
	IDLC_DECL_RETVAR(tIntPtr,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetDepthStencilStates,0,())
	IDLC_RET_FROM_BASE(ni::eType_IntPtr,_Ret)
IDLC_METH_END(ni,iMaterial,GetDepthStencilStates,0)

/** ni -> iMaterial::SetPolygonOffset/1 **/
IDLC_METH_BEGIN(ni,iMaterial,SetPolygonOffset,1)
	IDLC_DECL_VAR(ni::sVec2f,avOffset)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,avOffset)
	IDLC_METH_CALL_VOID(ni,iMaterial,SetPolygonOffset,1,(avOffset))
IDLC_METH_END(ni,iMaterial,SetPolygonOffset,1)

/** ni -> iMaterial::GetPolygonOffset/0 **/
IDLC_METH_BEGIN(ni,iMaterial,GetPolygonOffset,0)
	IDLC_DECL_RETVAR(ni::sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetPolygonOffset,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iMaterial,GetPolygonOffset,0)

/** ni -> iMaterial::GetHasShader/0 **/
IDLC_METH_BEGIN(ni,iMaterial,GetHasShader,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetHasShader,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMaterial,GetHasShader,0)

/** ni -> iMaterial::SetShader/2 **/
IDLC_METH_BEGIN(ni,iMaterial,SetShader,2)
	IDLC_DECL_VAR(eShaderUnit,aUnit)
	IDLC_BUF_TO_ENUM(eShaderUnit,aUnit)
	IDLC_DECL_VAR(iShader*,apProgram)
	IDLC_BUF_TO_INTF(iShader,apProgram)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,SetShader,2,(aUnit,apProgram))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMaterial,SetShader,2)

/** ni -> iMaterial::GetShader/1 **/
IDLC_METH_BEGIN(ni,iMaterial,GetShader,1)
	IDLC_DECL_VAR(eShaderUnit,aUnit)
	IDLC_BUF_TO_ENUM(eShaderUnit,aUnit)
	IDLC_DECL_RETVAR(iShader*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetShader,1,(aUnit))
	IDLC_RET_FROM_INTF(iShader,_Ret)
IDLC_METH_END(ni,iMaterial,GetShader,1)

/** ni -> iMaterial::SetShaderConstants/1 **/
IDLC_METH_BEGIN(ni,iMaterial,SetShaderConstants,1)
	IDLC_DECL_VAR(iShaderConstants*,apBuffer)
	IDLC_BUF_TO_INTF(iShaderConstants,apBuffer)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,SetShaderConstants,1,(apBuffer))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMaterial,SetShaderConstants,1)

/** ni -> iMaterial::GetShaderConstants/0 **/
IDLC_METH_BEGIN(ni,iMaterial,GetShaderConstants,0)
	IDLC_DECL_RETVAR(iShaderConstants*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetShaderConstants,0,())
	IDLC_RET_FROM_INTF(iShaderConstants,_Ret)
IDLC_METH_END(ni,iMaterial,GetShaderConstants,0)

/** ni -> iMaterial::HasChannelTexture/1 **/
IDLC_METH_BEGIN(ni,iMaterial,HasChannelTexture,1)
	IDLC_DECL_VAR(eMaterialChannel,aChannel)
	IDLC_BUF_TO_ENUM(eMaterialChannel,aChannel)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,HasChannelTexture,1,(aChannel))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMaterial,HasChannelTexture,1)

/** ni -> iMaterial::SetChannelTexture/2 **/
IDLC_METH_BEGIN(ni,iMaterial,SetChannelTexture,2)
	IDLC_DECL_VAR(eMaterialChannel,aChannel)
	IDLC_BUF_TO_ENUM(eMaterialChannel,aChannel)
	IDLC_DECL_VAR(iTexture*,apTexture)
	IDLC_BUF_TO_INTF(iTexture,apTexture)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,SetChannelTexture,2,(aChannel,apTexture))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMaterial,SetChannelTexture,2)

/** ni -> iMaterial::GetChannelTexture/1 **/
IDLC_METH_BEGIN(ni,iMaterial,GetChannelTexture,1)
	IDLC_DECL_VAR(eMaterialChannel,aChannel)
	IDLC_BUF_TO_ENUM(eMaterialChannel,aChannel)
	IDLC_DECL_RETVAR(iTexture*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetChannelTexture,1,(aChannel))
	IDLC_RET_FROM_INTF(iTexture,_Ret)
IDLC_METH_END(ni,iMaterial,GetChannelTexture,1)

/** ni -> iMaterial::SetChannelColor/2 **/
IDLC_METH_BEGIN(ni,iMaterial,SetChannelColor,2)
	IDLC_DECL_VAR(eMaterialChannel,aChannel)
	IDLC_BUF_TO_ENUM(eMaterialChannel,aChannel)
	IDLC_DECL_VAR(sColor4f,aColor)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,aColor)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,SetChannelColor,2,(aChannel,aColor))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMaterial,SetChannelColor,2)

/** ni -> iMaterial::GetChannelColor/1 **/
IDLC_METH_BEGIN(ni,iMaterial,GetChannelColor,1)
	IDLC_DECL_VAR(eMaterialChannel,aChannel)
	IDLC_BUF_TO_ENUM(eMaterialChannel,aChannel)
	IDLC_DECL_RETREFVAR(sColor4f&,_Ret,sColor4f)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetChannelColor,1,(aChannel))
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iMaterial,GetChannelColor,1)

/** ni -> iMaterial::SetChannelSamplerStates/2 **/
IDLC_METH_BEGIN(ni,iMaterial,SetChannelSamplerStates,2)
	IDLC_DECL_VAR(eMaterialChannel,aChannel)
	IDLC_BUF_TO_ENUM(eMaterialChannel,aChannel)
	IDLC_DECL_VAR(tIntPtr,aHandle)
	IDLC_BUF_TO_BASE(ni::eType_IntPtr,aHandle)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,SetChannelSamplerStates,2,(aChannel,aHandle))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMaterial,SetChannelSamplerStates,2)

/** ni -> iMaterial::GetChannelSamplerStates/1 **/
IDLC_METH_BEGIN(ni,iMaterial,GetChannelSamplerStates,1)
	IDLC_DECL_VAR(eMaterialChannel,aChannel)
	IDLC_BUF_TO_ENUM(eMaterialChannel,aChannel)
	IDLC_DECL_RETVAR(tIntPtr,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetChannelSamplerStates,1,(aChannel))
	IDLC_RET_FROM_BASE(ni::eType_IntPtr,_Ret)
IDLC_METH_END(ni,iMaterial,GetChannelSamplerStates,1)

/** ni -> iMaterial::CopyChannel/3 **/
IDLC_METH_BEGIN(ni,iMaterial,CopyChannel,3)
	IDLC_DECL_VAR(eMaterialChannel,aDestChannel)
	IDLC_BUF_TO_ENUM(eMaterialChannel,aDestChannel)
	IDLC_DECL_VAR(iMaterial*,apSource)
	IDLC_BUF_TO_INTF(iMaterial,apSource)
	IDLC_DECL_VAR(eMaterialChannel,aSrcChannel)
	IDLC_BUF_TO_ENUM(eMaterialChannel,aSrcChannel)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,CopyChannel,3,(aDestChannel,apSource,aSrcChannel))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMaterial,CopyChannel,3)

/** ni -> iMaterial::Copy/1 **/
IDLC_METH_BEGIN(ni,iMaterial,Copy,1)
	IDLC_DECL_VAR(iMaterial*,apMat)
	IDLC_BUF_TO_INTF(iMaterial,apMat)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,Copy,1,(apMat))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMaterial,Copy,1)

/** ni -> iMaterial::Clone/0 **/
IDLC_METH_BEGIN(ni,iMaterial,Clone,0)
	IDLC_DECL_RETVAR(iMaterial*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,Clone,0,())
	IDLC_RET_FROM_INTF(iMaterial,_Ret)
IDLC_METH_END(ni,iMaterial,Clone,0)

/** ni -> iMaterial::Serialize/3 **/
IDLC_METH_BEGIN(ni,iMaterial,Serialize,3)
	IDLC_DECL_VAR(eSerializeMode,aMode)
	IDLC_BUF_TO_ENUM(eSerializeMode,aMode)
	IDLC_DECL_VAR(iDataTable*,apDT)
	IDLC_BUF_TO_INTF(iDataTable,apDT)
	IDLC_DECL_VAR(iHString*,ahspBasePath)
	IDLC_BUF_TO_INTF(iHString,ahspBasePath)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,Serialize,3,(aMode,apDT,ahspBasePath))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMaterial,Serialize,3)

/** ni -> iMaterial::GetDescStructPtr/0 **/
IDLC_METH_BEGIN(ni,iMaterial,GetDescStructPtr,0)
	IDLC_DECL_RETVAR(tPtr,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetDescStructPtr,0,())
	IDLC_RET_FROM_PTR(ni::eType_Ptr,_Ret)
IDLC_METH_END(ni,iMaterial,GetDescStructPtr,0)

/** ni -> iMaterial::SetExpression/2 **/
IDLC_METH_BEGIN(ni,iMaterial,SetExpression,2)
	IDLC_DECL_VAR(eMaterialExpression,aExpr)
	IDLC_BUF_TO_ENUM(eMaterialExpression,aExpr)
	IDLC_DECL_VAR(iHString*,ahspExpr)
	IDLC_BUF_TO_INTF(iHString,ahspExpr)
	IDLC_METH_CALL_VOID(ni,iMaterial,SetExpression,2,(aExpr,ahspExpr))
IDLC_METH_END(ni,iMaterial,SetExpression,2)

/** ni -> iMaterial::GetExpression/1 **/
IDLC_METH_BEGIN(ni,iMaterial,GetExpression,1)
	IDLC_DECL_VAR(eMaterialExpression,aExpr)
	IDLC_BUF_TO_ENUM(eMaterialExpression,aExpr)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetExpression,1,(aExpr))
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iMaterial,GetExpression,1)

/** ni -> iMaterial::GetExpressionValueMatrix/1 **/
IDLC_METH_BEGIN(ni,iMaterial,GetExpressionValueMatrix,1)
	IDLC_DECL_VAR(eMaterialExpression,aExpr)
	IDLC_BUF_TO_ENUM(eMaterialExpression,aExpr)
	IDLC_DECL_RETVAR(sMatrixf,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetExpressionValueMatrix,1,(aExpr))
	IDLC_RET_FROM_BASE(ni::eType_Matrixf,_Ret)
IDLC_METH_END(ni,iMaterial,GetExpressionValueMatrix,1)

/** ni -> iMaterial::GetExpressionValueVector/1 **/
IDLC_METH_BEGIN(ni,iMaterial,GetExpressionValueVector,1)
	IDLC_DECL_VAR(eMaterialExpression,aExpr)
	IDLC_BUF_TO_ENUM(eMaterialExpression,aExpr)
	IDLC_DECL_RETVAR(sVec4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetExpressionValueVector,1,(aExpr))
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iMaterial,GetExpressionValueVector,1)

/** ni -> iMaterial::SetExpressionObject/3 **/
IDLC_METH_BEGIN(ni,iMaterial,SetExpressionObject,3)
	IDLC_DECL_VAR(eMaterialExpression,aExpr)
	IDLC_BUF_TO_ENUM(eMaterialExpression,aExpr)
	IDLC_DECL_VAR(iHString*,ahspExpr)
	IDLC_BUF_TO_INTF(iHString,ahspExpr)
	IDLC_DECL_VAR(iExpression*,apExpr)
	IDLC_BUF_TO_INTF(iExpression,apExpr)
	IDLC_METH_CALL_VOID(ni,iMaterial,SetExpressionObject,3,(aExpr,ahspExpr,apExpr))
IDLC_METH_END(ni,iMaterial,SetExpressionObject,3)

/** ni -> iMaterial::GetExpressionObject/1 **/
IDLC_METH_BEGIN(ni,iMaterial,GetExpressionObject,1)
	IDLC_DECL_VAR(eMaterialExpression,aExpr)
	IDLC_BUF_TO_ENUM(eMaterialExpression,aExpr)
	IDLC_DECL_RETVAR(iExpression*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetExpressionObject,1,(aExpr))
	IDLC_RET_FROM_INTF(iExpression,_Ret)
IDLC_METH_END(ni,iMaterial,GetExpressionObject,1)

/** ni -> iMaterial::SetUserdata/2 **/
IDLC_METH_BEGIN(ni,iMaterial,SetUserdata,2)
	IDLC_DECL_VAR(iHString*,ahspID)
	IDLC_BUF_TO_INTF(iHString,ahspID)
	IDLC_DECL_VAR(iUnknown*,apUserdata)
	IDLC_BUF_TO_INTF(iUnknown,apUserdata)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,SetUserdata,2,(ahspID,apUserdata))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMaterial,SetUserdata,2)

/** ni -> iMaterial::GetUserdata/1 **/
IDLC_METH_BEGIN(ni,iMaterial,GetUserdata,1)
	IDLC_DECL_VAR(iHString*,ahspID)
	IDLC_BUF_TO_INTF(iHString,ahspID)
	IDLC_DECL_RETVAR(iUnknown*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetUserdata,1,(ahspID))
	IDLC_RET_FROM_INTF(iUnknown,_Ret)
IDLC_METH_END(ni,iMaterial,GetUserdata,1)

/** ni -> iMaterial::GetNumUserdata/0 **/
IDLC_METH_BEGIN(ni,iMaterial,GetNumUserdata,0)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetNumUserdata,0,())
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iMaterial,GetNumUserdata,0)

/** ni -> iMaterial::GetUserdataName/1 **/
IDLC_METH_BEGIN(ni,iMaterial,GetUserdataName,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetUserdataName,1,(anIndex))
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iMaterial,GetUserdataName,1)

/** ni -> iMaterial::GetUserdataFromIndex/1 **/
IDLC_METH_BEGIN(ni,iMaterial,GetUserdataFromIndex,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(iUnknown*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMaterial,GetUserdataFromIndex,1,(anIndex))
	IDLC_RET_FROM_INTF(iUnknown,_Ret)
IDLC_METH_END(ni,iMaterial,GetUserdataFromIndex,1)

IDLC_END_INTF(ni,iMaterial)

IDLC_END_NAMESPACE()
// EOF //
