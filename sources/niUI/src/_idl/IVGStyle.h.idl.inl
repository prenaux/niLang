#include "../API/niUI/IVGStyle.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iVGStyle **/
/** iVGStyle -> MIN FEATURES '20' **/
#if niMinFeatures(20)
IDLC_BEGIN_INTF(ni,iVGStyle)
/** ni -> iVGStyle::Copy/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::Copy/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,Copy,1)
	IDLC_DECL_VAR(iVGStyle*,apStyle)
	IDLC_BUF_TO_INTF(iVGStyle,apStyle)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,Copy,1,(apStyle))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGStyle,Copy,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::Clone/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::Clone/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,Clone,0)
	IDLC_DECL_RETVAR(iVGStyle*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,Clone,0,())
	IDLC_RET_FROM_INTF(iVGStyle,_Ret)
IDLC_METH_END(ni,iVGStyle,Clone,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::Push/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::Push/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,Push,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,Push,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGStyle,Push,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::Pop/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::Pop/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,Pop,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,Pop,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGStyle,Pop,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetDefault/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetDefault/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetDefault,0)
	IDLC_METH_CALL_VOID(ni,iVGStyle,SetDefault,0,())
IDLC_METH_END(ni,iVGStyle,SetDefault,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetOpacity/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetOpacity/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetOpacity,1)
	IDLC_DECL_VAR(tF32,afOpacity)
	IDLC_BUF_TO_BASE(ni::eType_F32,afOpacity)
	IDLC_METH_CALL_VOID(ni,iVGStyle,SetOpacity,1,(afOpacity))
IDLC_METH_END(ni,iVGStyle,SetOpacity,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetOpacity/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetOpacity/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetOpacity,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetOpacity,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iVGStyle,GetOpacity,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetLineCap/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetLineCap/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetLineCap,1)
	IDLC_DECL_VAR(eVGLineCap,aCap)
	IDLC_BUF_TO_ENUM(eVGLineCap,aCap)
	IDLC_METH_CALL_VOID(ni,iVGStyle,SetLineCap,1,(aCap))
IDLC_METH_END(ni,iVGStyle,SetLineCap,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetLineCap/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetLineCap/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetLineCap,0)
	IDLC_DECL_RETVAR(eVGLineCap,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetLineCap,0,())
	IDLC_RET_FROM_ENUM(eVGLineCap,_Ret)
IDLC_METH_END(ni,iVGStyle,GetLineCap,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetLineJoin/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetLineJoin/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetLineJoin,1)
	IDLC_DECL_VAR(eVGLineJoin,aJoin)
	IDLC_BUF_TO_ENUM(eVGLineJoin,aJoin)
	IDLC_METH_CALL_VOID(ni,iVGStyle,SetLineJoin,1,(aJoin))
IDLC_METH_END(ni,iVGStyle,SetLineJoin,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetLineJoin/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetLineJoin/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetLineJoin,0)
	IDLC_DECL_RETVAR(eVGLineJoin,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetLineJoin,0,())
	IDLC_RET_FROM_ENUM(eVGLineJoin,_Ret)
IDLC_METH_END(ni,iVGStyle,GetLineJoin,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetInnerJoin/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetInnerJoin/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetInnerJoin,1)
	IDLC_DECL_VAR(eVGInnerJoin,aInnerJoin)
	IDLC_BUF_TO_ENUM(eVGInnerJoin,aInnerJoin)
	IDLC_METH_CALL_VOID(ni,iVGStyle,SetInnerJoin,1,(aInnerJoin))
IDLC_METH_END(ni,iVGStyle,SetInnerJoin,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetInnerJoin/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetInnerJoin/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetInnerJoin,0)
	IDLC_DECL_RETVAR(eVGInnerJoin,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetInnerJoin,0,())
	IDLC_RET_FROM_ENUM(eVGInnerJoin,_Ret)
IDLC_METH_END(ni,iVGStyle,GetInnerJoin,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetMiterLimit/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetMiterLimit/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetMiterLimit,1)
	IDLC_DECL_VAR(tF32,afMiterLimit)
	IDLC_BUF_TO_BASE(ni::eType_F32,afMiterLimit)
	IDLC_METH_CALL_VOID(ni,iVGStyle,SetMiterLimit,1,(afMiterLimit))
IDLC_METH_END(ni,iVGStyle,SetMiterLimit,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetMiterLimit/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetMiterLimit/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetMiterLimit,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetMiterLimit,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iVGStyle,GetMiterLimit,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetSmooth/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetSmooth/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetSmooth,1)
	IDLC_DECL_VAR(tF32,afSmooth)
	IDLC_BUF_TO_BASE(ni::eType_F32,afSmooth)
	IDLC_METH_CALL_VOID(ni,iVGStyle,SetSmooth,1,(afSmooth))
IDLC_METH_END(ni,iVGStyle,SetSmooth,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetSmooth/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetSmooth/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetSmooth,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetSmooth,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iVGStyle,GetSmooth,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetCurrentColor/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetCurrentColor/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetCurrentColor,1)
	IDLC_DECL_VAR(sColor4f,avColor)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,avColor)
	IDLC_METH_CALL_VOID(ni,iVGStyle,SetCurrentColor,1,(avColor))
IDLC_METH_END(ni,iVGStyle,SetCurrentColor,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetCurrentColor/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetCurrentColor/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetCurrentColor,0)
	IDLC_DECL_RETREFVAR(sColor4f&,_Ret,sColor4f)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetCurrentColor,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iVGStyle,GetCurrentColor,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetAntiAliasing/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetAntiAliasing/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetAntiAliasing,1)
	IDLC_DECL_VAR(tBool,abAntiAliasing)
	IDLC_BUF_TO_BASE(ni::eType_I8,abAntiAliasing)
	IDLC_METH_CALL_VOID(ni,iVGStyle,SetAntiAliasing,1,(abAntiAliasing))
IDLC_METH_END(ni,iVGStyle,SetAntiAliasing,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetAntiAliasing/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetAntiAliasing/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetAntiAliasing,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetAntiAliasing,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGStyle,GetAntiAliasing,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetRasterizerApproximationScale/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetRasterizerApproximationScale/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetRasterizerApproximationScale,1)
	IDLC_DECL_VAR(tF32,afRasterizerApproximationScale)
	IDLC_BUF_TO_BASE(ni::eType_F32,afRasterizerApproximationScale)
	IDLC_METH_CALL_VOID(ni,iVGStyle,SetRasterizerApproximationScale,1,(afRasterizerApproximationScale))
IDLC_METH_END(ni,iVGStyle,SetRasterizerApproximationScale,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetRasterizerApproximationScale/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetRasterizerApproximationScale/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetRasterizerApproximationScale,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetRasterizerApproximationScale,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iVGStyle,GetRasterizerApproximationScale,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetTesselatorApproximationScale/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetTesselatorApproximationScale/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetTesselatorApproximationScale,1)
	IDLC_DECL_VAR(tF32,afTesselatorApproximationScale)
	IDLC_BUF_TO_BASE(ni::eType_F32,afTesselatorApproximationScale)
	IDLC_METH_CALL_VOID(ni,iVGStyle,SetTesselatorApproximationScale,1,(afTesselatorApproximationScale))
IDLC_METH_END(ni,iVGStyle,SetTesselatorApproximationScale,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetTesselatorApproximationScale/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetTesselatorApproximationScale/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetTesselatorApproximationScale,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetTesselatorApproximationScale,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iVGStyle,GetTesselatorApproximationScale,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetStroke/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetStroke/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetStroke,1)
	IDLC_DECL_VAR(tBool,abStroke)
	IDLC_BUF_TO_BASE(ni::eType_I8,abStroke)
	IDLC_METH_CALL_VOID(ni,iVGStyle,SetStroke,1,(abStroke))
IDLC_METH_END(ni,iVGStyle,SetStroke,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetStroke/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetStroke/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetStroke,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetStroke,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGStyle,GetStroke,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetStrokeWidth/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetStrokeWidth/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetStrokeWidth,1)
	IDLC_DECL_VAR(tF32,afWidth)
	IDLC_BUF_TO_BASE(ni::eType_F32,afWidth)
	IDLC_METH_CALL_VOID(ni,iVGStyle,SetStrokeWidth,1,(afWidth))
IDLC_METH_END(ni,iVGStyle,SetStrokeWidth,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetStrokeWidth/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetStrokeWidth/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetStrokeWidth,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetStrokeWidth,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iVGStyle,GetStrokeWidth,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetStrokeTransformed/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetStrokeTransformed/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetStrokeTransformed,1)
	IDLC_DECL_VAR(tBool,abStrokeTransformed)
	IDLC_BUF_TO_BASE(ni::eType_I8,abStrokeTransformed)
	IDLC_METH_CALL_VOID(ni,iVGStyle,SetStrokeTransformed,1,(abStrokeTransformed))
IDLC_METH_END(ni,iVGStyle,SetStrokeTransformed,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetStrokeTransformed/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetStrokeTransformed/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetStrokeTransformed,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetStrokeTransformed,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGStyle,GetStrokeTransformed,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetStrokePaint/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetStrokePaint/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetStrokePaint,1)
	IDLC_DECL_VAR(iVGPaint*,apPaint)
	IDLC_BUF_TO_INTF(iVGPaint,apPaint)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,SetStrokePaint,1,(apPaint))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGStyle,SetStrokePaint,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetStrokePaint/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetStrokePaint/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetStrokePaint,0)
	IDLC_DECL_RETVAR(iVGPaint*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetStrokePaint,0,())
	IDLC_RET_FROM_INTF(iVGPaint,_Ret)
IDLC_METH_END(ni,iVGStyle,GetStrokePaint,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetStrokeColor/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetStrokeColor/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetStrokeColor,1)
	IDLC_DECL_VAR(sColor3f,avColor)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,avColor)
	IDLC_METH_CALL_VOID(ni,iVGStyle,SetStrokeColor,1,(avColor))
IDLC_METH_END(ni,iVGStyle,SetStrokeColor,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetStrokeColor/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetStrokeColor/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetStrokeColor,0)
	IDLC_DECL_RETVAR(sColor3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetStrokeColor,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iVGStyle,GetStrokeColor,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetStrokeColor4/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetStrokeColor4/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetStrokeColor4,1)
	IDLC_DECL_VAR(sColor4f,avColor)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,avColor)
	IDLC_METH_CALL_VOID(ni,iVGStyle,SetStrokeColor4,1,(avColor))
IDLC_METH_END(ni,iVGStyle,SetStrokeColor4,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetStrokeColor4/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetStrokeColor4/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetStrokeColor4,0)
	IDLC_DECL_RETVAR(sColor4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetStrokeColor4,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iVGStyle,GetStrokeColor4,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetStrokeOpacity/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetStrokeOpacity/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetStrokeOpacity,1)
	IDLC_DECL_VAR(tF32,afOpacity)
	IDLC_BUF_TO_BASE(ni::eType_F32,afOpacity)
	IDLC_METH_CALL_VOID(ni,iVGStyle,SetStrokeOpacity,1,(afOpacity))
IDLC_METH_END(ni,iVGStyle,SetStrokeOpacity,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetStrokeOpacity/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetStrokeOpacity/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetStrokeOpacity,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetStrokeOpacity,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iVGStyle,GetStrokeOpacity,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetNumDashes/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetNumDashes/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetNumDashes,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetNumDashes,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iVGStyle,GetNumDashes,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::ClearDashes/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::ClearDashes/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,ClearDashes,0)
	IDLC_METH_CALL_VOID(ni,iVGStyle,ClearDashes,0,())
IDLC_METH_END(ni,iVGStyle,ClearDashes,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::AddDash/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::AddDash/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,AddDash,1)
	IDLC_DECL_VAR(sVec2f,aV)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,aV)
	IDLC_METH_CALL_VOID(ni,iVGStyle,AddDash,1,(aV))
IDLC_METH_END(ni,iVGStyle,AddDash,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::RemoveDash/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::RemoveDash/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,RemoveDash,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,RemoveDash,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGStyle,RemoveDash,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetDash/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetDash/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetDash,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetDash,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iVGStyle,GetDash,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetDashStart/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetDashStart/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetDashStart,1)
	IDLC_DECL_VAR(tF32,afDashStart)
	IDLC_BUF_TO_BASE(ni::eType_F32,afDashStart)
	IDLC_METH_CALL_VOID(ni,iVGStyle,SetDashStart,1,(afDashStart))
IDLC_METH_END(ni,iVGStyle,SetDashStart,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetDashStart/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetDashStart/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetDashStart,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetDashStart,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iVGStyle,GetDashStart,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetFill/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetFill/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetFill,1)
	IDLC_DECL_VAR(tBool,abFill)
	IDLC_BUF_TO_BASE(ni::eType_I8,abFill)
	IDLC_METH_CALL_VOID(ni,iVGStyle,SetFill,1,(abFill))
IDLC_METH_END(ni,iVGStyle,SetFill,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetFill/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetFill/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetFill,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetFill,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGStyle,GetFill,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetFillEvenOdd/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetFillEvenOdd/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetFillEvenOdd,1)
	IDLC_DECL_VAR(tBool,abFillEvenOdd)
	IDLC_BUF_TO_BASE(ni::eType_I8,abFillEvenOdd)
	IDLC_METH_CALL_VOID(ni,iVGStyle,SetFillEvenOdd,1,(abFillEvenOdd))
IDLC_METH_END(ni,iVGStyle,SetFillEvenOdd,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetFillEvenOdd/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetFillEvenOdd/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetFillEvenOdd,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetFillEvenOdd,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGStyle,GetFillEvenOdd,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetFillExpand/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetFillExpand/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetFillExpand,1)
	IDLC_DECL_VAR(tF32,afExpand)
	IDLC_BUF_TO_BASE(ni::eType_F32,afExpand)
	IDLC_METH_CALL_VOID(ni,iVGStyle,SetFillExpand,1,(afExpand))
IDLC_METH_END(ni,iVGStyle,SetFillExpand,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetFillExpand/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetFillExpand/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetFillExpand,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetFillExpand,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iVGStyle,GetFillExpand,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetFillPaint/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetFillPaint/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetFillPaint,1)
	IDLC_DECL_VAR(iVGPaint*,apPaint)
	IDLC_BUF_TO_INTF(iVGPaint,apPaint)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,SetFillPaint,1,(apPaint))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGStyle,SetFillPaint,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetFillPaint/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetFillPaint/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetFillPaint,0)
	IDLC_DECL_RETVAR(iVGPaint*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetFillPaint,0,())
	IDLC_RET_FROM_INTF(iVGPaint,_Ret)
IDLC_METH_END(ni,iVGStyle,GetFillPaint,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetFillColor/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetFillColor/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetFillColor,1)
	IDLC_DECL_VAR(sColor3f,avColor)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,avColor)
	IDLC_METH_CALL_VOID(ni,iVGStyle,SetFillColor,1,(avColor))
IDLC_METH_END(ni,iVGStyle,SetFillColor,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetFillColor/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetFillColor/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetFillColor,0)
	IDLC_DECL_RETVAR(sColor3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetFillColor,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iVGStyle,GetFillColor,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetFillColor4/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetFillColor4/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetFillColor4,1)
	IDLC_DECL_VAR(sColor4f,avColor)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,avColor)
	IDLC_METH_CALL_VOID(ni,iVGStyle,SetFillColor4,1,(avColor))
IDLC_METH_END(ni,iVGStyle,SetFillColor4,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetFillColor4/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetFillColor4/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetFillColor4,0)
	IDLC_DECL_RETVAR(sColor4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetFillColor4,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iVGStyle,GetFillColor4,0)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::SetFillOpacity/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::SetFillOpacity/1 **/
IDLC_METH_BEGIN(ni,iVGStyle,SetFillOpacity,1)
	IDLC_DECL_VAR(tF32,afOpacity)
	IDLC_BUF_TO_BASE(ni::eType_F32,afOpacity)
	IDLC_METH_CALL_VOID(ni,iVGStyle,SetFillOpacity,1,(afOpacity))
IDLC_METH_END(ni,iVGStyle,SetFillOpacity,1)
#endif // niMinFeatures(20)

/** ni -> iVGStyle::GetFillOpacity/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGStyle::GetFillOpacity/0 **/
IDLC_METH_BEGIN(ni,iVGStyle,GetFillOpacity,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGStyle,GetFillOpacity,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iVGStyle,GetFillOpacity,0)
#endif // niMinFeatures(20)

IDLC_END_INTF(ni,iVGStyle)

#endif // if niMinFeatures(20)
IDLC_END_NAMESPACE()
// EOF //
