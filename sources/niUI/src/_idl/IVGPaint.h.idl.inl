#include "../API/niUI/IVGPaint.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iVGPaint **/
/** iVGPaint -> MIN FEATURES '20' **/
#if niMinFeatures(20)
IDLC_BEGIN_INTF(ni,iVGPaint)
/** ni -> iVGPaint::Copy/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaint::Copy/1 **/
IDLC_METH_BEGIN(ni,iVGPaint,Copy,1)
	IDLC_DECL_VAR(iVGPaint*,apSrc)
	IDLC_BUF_TO_INTF(iVGPaint,apSrc)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPaint,Copy,1,(apSrc))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGPaint,Copy,1)
#endif // niMinFeatures(20)

/** ni -> iVGPaint::Clone/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaint::Clone/0 **/
IDLC_METH_BEGIN(ni,iVGPaint,Clone,0)
	IDLC_DECL_RETVAR(iVGPaint*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPaint,Clone,0,())
	IDLC_RET_FROM_INTF(iVGPaint,_Ret)
IDLC_METH_END(ni,iVGPaint,Clone,0)
#endif // niMinFeatures(20)

/** ni -> iVGPaint::GetType/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaint::GetType/0 **/
IDLC_METH_BEGIN(ni,iVGPaint,GetType,0)
	IDLC_DECL_RETVAR(eVGPaintType,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPaint,GetType,0,())
	IDLC_RET_FROM_ENUM(eVGPaintType,_Ret)
IDLC_METH_END(ni,iVGPaint,GetType,0)
#endif // niMinFeatures(20)

/** ni -> iVGPaint::SetColor/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaint::SetColor/1 **/
IDLC_METH_BEGIN(ni,iVGPaint,SetColor,1)
	IDLC_DECL_VAR(sColor4f,aColor)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,aColor)
	IDLC_METH_CALL_VOID(ni,iVGPaint,SetColor,1,(aColor))
IDLC_METH_END(ni,iVGPaint,SetColor,1)
#endif // niMinFeatures(20)

/** ni -> iVGPaint::GetColor/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaint::GetColor/0 **/
IDLC_METH_BEGIN(ni,iVGPaint,GetColor,0)
	IDLC_DECL_RETREFVAR(sColor4f&,_Ret,sColor4f)
	IDLC_METH_CALL(_Ret,ni,iVGPaint,GetColor,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iVGPaint,GetColor,0)
#endif // niMinFeatures(20)

IDLC_END_INTF(ni,iVGPaint)

#endif // if niMinFeatures(20)
/** interface : iVGPaintImage **/
/** iVGPaintImage -> MIN FEATURES '20' **/
#if niMinFeatures(20)
IDLC_BEGIN_INTF(ni,iVGPaintImage)
/** ni -> iVGPaintImage::GetImage/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintImage::GetImage/0 **/
IDLC_METH_BEGIN(ni,iVGPaintImage,GetImage,0)
	IDLC_DECL_RETVAR(iVGImage*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPaintImage,GetImage,0,())
	IDLC_RET_FROM_INTF(iVGImage,_Ret)
IDLC_METH_END(ni,iVGPaintImage,GetImage,0)
#endif // niMinFeatures(20)

/** ni -> iVGPaintImage::SetFilterType/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintImage::SetFilterType/1 **/
IDLC_METH_BEGIN(ni,iVGPaintImage,SetFilterType,1)
	IDLC_DECL_VAR(eVGImageFilter,aType)
	IDLC_BUF_TO_ENUM(eVGImageFilter,aType)
	IDLC_METH_CALL_VOID(ni,iVGPaintImage,SetFilterType,1,(aType))
IDLC_METH_END(ni,iVGPaintImage,SetFilterType,1)
#endif // niMinFeatures(20)

/** ni -> iVGPaintImage::GetFilterType/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintImage::GetFilterType/0 **/
IDLC_METH_BEGIN(ni,iVGPaintImage,GetFilterType,0)
	IDLC_DECL_RETVAR(eVGImageFilter,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPaintImage,GetFilterType,0,())
	IDLC_RET_FROM_ENUM(eVGImageFilter,_Ret)
IDLC_METH_END(ni,iVGPaintImage,GetFilterType,0)
#endif // niMinFeatures(20)

/** ni -> iVGPaintImage::SetFilterRadius/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintImage::SetFilterRadius/1 **/
IDLC_METH_BEGIN(ni,iVGPaintImage,SetFilterRadius,1)
	IDLC_DECL_VAR(tF32,afRadius)
	IDLC_BUF_TO_BASE(ni::eType_F32,afRadius)
	IDLC_METH_CALL_VOID(ni,iVGPaintImage,SetFilterRadius,1,(afRadius))
IDLC_METH_END(ni,iVGPaintImage,SetFilterRadius,1)
#endif // niMinFeatures(20)

/** ni -> iVGPaintImage::GetFilterRadius/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintImage::GetFilterRadius/0 **/
IDLC_METH_BEGIN(ni,iVGPaintImage,GetFilterRadius,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPaintImage,GetFilterRadius,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iVGPaintImage,GetFilterRadius,0)
#endif // niMinFeatures(20)

/** ni -> iVGPaintImage::SetFilterNormalize/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintImage::SetFilterNormalize/1 **/
IDLC_METH_BEGIN(ni,iVGPaintImage,SetFilterNormalize,1)
	IDLC_DECL_VAR(tBool,abNormalize)
	IDLC_BUF_TO_BASE(ni::eType_I8,abNormalize)
	IDLC_METH_CALL_VOID(ni,iVGPaintImage,SetFilterNormalize,1,(abNormalize))
IDLC_METH_END(ni,iVGPaintImage,SetFilterNormalize,1)
#endif // niMinFeatures(20)

/** ni -> iVGPaintImage::GetFilterNormalize/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintImage::GetFilterNormalize/0 **/
IDLC_METH_BEGIN(ni,iVGPaintImage,GetFilterNormalize,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPaintImage,GetFilterNormalize,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGPaintImage,GetFilterNormalize,0)
#endif // niMinFeatures(20)

/** ni -> iVGPaintImage::SetWrapType/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintImage::SetWrapType/1 **/
IDLC_METH_BEGIN(ni,iVGPaintImage,SetWrapType,1)
	IDLC_DECL_VAR(eVGWrapType,aWrapType)
	IDLC_BUF_TO_ENUM(eVGWrapType,aWrapType)
	IDLC_METH_CALL_VOID(ni,iVGPaintImage,SetWrapType,1,(aWrapType))
IDLC_METH_END(ni,iVGPaintImage,SetWrapType,1)
#endif // niMinFeatures(20)

/** ni -> iVGPaintImage::GetWrapType/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintImage::GetWrapType/0 **/
IDLC_METH_BEGIN(ni,iVGPaintImage,GetWrapType,0)
	IDLC_DECL_RETVAR(eVGWrapType,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPaintImage,GetWrapType,0,())
	IDLC_RET_FROM_ENUM(eVGWrapType,_Ret)
IDLC_METH_END(ni,iVGPaintImage,GetWrapType,0)
#endif // niMinFeatures(20)

/** ni -> iVGPaintImage::SetRectangle/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintImage::SetRectangle/1 **/
IDLC_METH_BEGIN(ni,iVGPaintImage,SetRectangle,1)
	IDLC_DECL_VAR(sRectf,aRect)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,aRect)
	IDLC_METH_CALL_VOID(ni,iVGPaintImage,SetRectangle,1,(aRect))
IDLC_METH_END(ni,iVGPaintImage,SetRectangle,1)
#endif // niMinFeatures(20)

/** ni -> iVGPaintImage::GetRectangle/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintImage::GetRectangle/0 **/
IDLC_METH_BEGIN(ni,iVGPaintImage,GetRectangle,0)
	IDLC_DECL_RETVAR(sRectf,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPaintImage,GetRectangle,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iVGPaintImage,GetRectangle,0)
#endif // niMinFeatures(20)

/** ni -> iVGPaintImage::SetUnits/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintImage::SetUnits/1 **/
IDLC_METH_BEGIN(ni,iVGPaintImage,SetUnits,1)
	IDLC_DECL_VAR(eVGPaintUnits,aUnits)
	IDLC_BUF_TO_ENUM(eVGPaintUnits,aUnits)
	IDLC_METH_CALL_VOID(ni,iVGPaintImage,SetUnits,1,(aUnits))
IDLC_METH_END(ni,iVGPaintImage,SetUnits,1)
#endif // niMinFeatures(20)

/** ni -> iVGPaintImage::GetUnits/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintImage::GetUnits/0 **/
IDLC_METH_BEGIN(ni,iVGPaintImage,GetUnits,0)
	IDLC_DECL_RETVAR(eVGPaintUnits,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPaintImage,GetUnits,0,())
	IDLC_RET_FROM_ENUM(eVGPaintUnits,_Ret)
IDLC_METH_END(ni,iVGPaintImage,GetUnits,0)
#endif // niMinFeatures(20)

/** ni -> iVGPaintImage::SetSource/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintImage::SetSource/1 **/
IDLC_METH_BEGIN(ni,iVGPaintImage,SetSource,1)
	IDLC_DECL_VAR(iUnknown*,apSource)
	IDLC_BUF_TO_INTF(iUnknown,apSource)
	IDLC_METH_CALL_VOID(ni,iVGPaintImage,SetSource,1,(apSource))
IDLC_METH_END(ni,iVGPaintImage,SetSource,1)
#endif // niMinFeatures(20)

/** ni -> iVGPaintImage::GetSource/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintImage::GetSource/0 **/
IDLC_METH_BEGIN(ni,iVGPaintImage,GetSource,0)
	IDLC_DECL_RETVAR(iUnknown*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPaintImage,GetSource,0,())
	IDLC_RET_FROM_INTF(iUnknown,_Ret)
IDLC_METH_END(ni,iVGPaintImage,GetSource,0)
#endif // niMinFeatures(20)

/** ni -> iVGPaintImage::SetSourceUnits/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintImage::SetSourceUnits/1 **/
IDLC_METH_BEGIN(ni,iVGPaintImage,SetSourceUnits,1)
	IDLC_DECL_VAR(eVGPaintUnits,aUnits)
	IDLC_BUF_TO_ENUM(eVGPaintUnits,aUnits)
	IDLC_METH_CALL_VOID(ni,iVGPaintImage,SetSourceUnits,1,(aUnits))
IDLC_METH_END(ni,iVGPaintImage,SetSourceUnits,1)
#endif // niMinFeatures(20)

/** ni -> iVGPaintImage::GetSourceUnits/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintImage::GetSourceUnits/0 **/
IDLC_METH_BEGIN(ni,iVGPaintImage,GetSourceUnits,0)
	IDLC_DECL_RETVAR(eVGPaintUnits,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPaintImage,GetSourceUnits,0,())
	IDLC_RET_FROM_ENUM(eVGPaintUnits,_Ret)
IDLC_METH_END(ni,iVGPaintImage,GetSourceUnits,0)
#endif // niMinFeatures(20)

IDLC_END_INTF(ni,iVGPaintImage)

#endif // if niMinFeatures(20)
/** interface : iVGPaintGradient **/
/** iVGPaintGradient -> MIN FEATURES '20' **/
#if niMinFeatures(20)
IDLC_BEGIN_INTF(ni,iVGPaintGradient)
/** ni -> iVGPaintGradient::SetGradientType/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintGradient::SetGradientType/1 **/
IDLC_METH_BEGIN(ni,iVGPaintGradient,SetGradientType,1)
	IDLC_DECL_VAR(eVGGradientType,aType)
	IDLC_BUF_TO_ENUM(eVGGradientType,aType)
	IDLC_METH_CALL_VOID(ni,iVGPaintGradient,SetGradientType,1,(aType))
IDLC_METH_END(ni,iVGPaintGradient,SetGradientType,1)
#endif // niMinFeatures(20)

/** ni -> iVGPaintGradient::GetGradientType/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintGradient::GetGradientType/0 **/
IDLC_METH_BEGIN(ni,iVGPaintGradient,GetGradientType,0)
	IDLC_DECL_RETVAR(eVGGradientType,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPaintGradient,GetGradientType,0,())
	IDLC_RET_FROM_ENUM(eVGGradientType,_Ret)
IDLC_METH_END(ni,iVGPaintGradient,GetGradientType,0)
#endif // niMinFeatures(20)

/** ni -> iVGPaintGradient::SetD1/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintGradient::SetD1/1 **/
IDLC_METH_BEGIN(ni,iVGPaintGradient,SetD1,1)
	IDLC_DECL_VAR(tI32,anD1)
	IDLC_BUF_TO_BASE(ni::eType_I32,anD1)
	IDLC_METH_CALL_VOID(ni,iVGPaintGradient,SetD1,1,(anD1))
IDLC_METH_END(ni,iVGPaintGradient,SetD1,1)
#endif // niMinFeatures(20)

/** ni -> iVGPaintGradient::GetD1/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintGradient::GetD1/0 **/
IDLC_METH_BEGIN(ni,iVGPaintGradient,GetD1,0)
	IDLC_DECL_RETVAR(tI32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPaintGradient,GetD1,0,())
	IDLC_RET_FROM_BASE(ni::eType_I32,_Ret)
IDLC_METH_END(ni,iVGPaintGradient,GetD1,0)
#endif // niMinFeatures(20)

/** ni -> iVGPaintGradient::SetD2/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintGradient::SetD2/1 **/
IDLC_METH_BEGIN(ni,iVGPaintGradient,SetD2,1)
	IDLC_DECL_VAR(tI32,anD2)
	IDLC_BUF_TO_BASE(ni::eType_I32,anD2)
	IDLC_METH_CALL_VOID(ni,iVGPaintGradient,SetD2,1,(anD2))
IDLC_METH_END(ni,iVGPaintGradient,SetD2,1)
#endif // niMinFeatures(20)

/** ni -> iVGPaintGradient::GetD2/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintGradient::GetD2/0 **/
IDLC_METH_BEGIN(ni,iVGPaintGradient,GetD2,0)
	IDLC_DECL_RETVAR(tI32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPaintGradient,GetD2,0,())
	IDLC_RET_FROM_BASE(ni::eType_I32,_Ret)
IDLC_METH_END(ni,iVGPaintGradient,GetD2,0)
#endif // niMinFeatures(20)

/** ni -> iVGPaintGradient::SetWrapType/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintGradient::SetWrapType/1 **/
IDLC_METH_BEGIN(ni,iVGPaintGradient,SetWrapType,1)
	IDLC_DECL_VAR(eVGWrapType,aWrapType)
	IDLC_BUF_TO_ENUM(eVGWrapType,aWrapType)
	IDLC_METH_CALL_VOID(ni,iVGPaintGradient,SetWrapType,1,(aWrapType))
IDLC_METH_END(ni,iVGPaintGradient,SetWrapType,1)
#endif // niMinFeatures(20)

/** ni -> iVGPaintGradient::GetWrapType/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintGradient::GetWrapType/0 **/
IDLC_METH_BEGIN(ni,iVGPaintGradient,GetWrapType,0)
	IDLC_DECL_RETVAR(eVGWrapType,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPaintGradient,GetWrapType,0,())
	IDLC_RET_FROM_ENUM(eVGWrapType,_Ret)
IDLC_METH_END(ni,iVGPaintGradient,GetWrapType,0)
#endif // niMinFeatures(20)

/** ni -> iVGPaintGradient::SetA/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintGradient::SetA/1 **/
IDLC_METH_BEGIN(ni,iVGPaintGradient,SetA,1)
	IDLC_DECL_VAR(sVec2f,aV)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,aV)
	IDLC_METH_CALL_VOID(ni,iVGPaintGradient,SetA,1,(aV))
IDLC_METH_END(ni,iVGPaintGradient,SetA,1)
#endif // niMinFeatures(20)

/** ni -> iVGPaintGradient::GetA/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintGradient::GetA/0 **/
IDLC_METH_BEGIN(ni,iVGPaintGradient,GetA,0)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPaintGradient,GetA,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iVGPaintGradient,GetA,0)
#endif // niMinFeatures(20)

/** ni -> iVGPaintGradient::SetB/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintGradient::SetB/1 **/
IDLC_METH_BEGIN(ni,iVGPaintGradient,SetB,1)
	IDLC_DECL_VAR(sVec2f,aV)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,aV)
	IDLC_METH_CALL_VOID(ni,iVGPaintGradient,SetB,1,(aV))
IDLC_METH_END(ni,iVGPaintGradient,SetB,1)
#endif // niMinFeatures(20)

/** ni -> iVGPaintGradient::GetB/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintGradient::GetB/0 **/
IDLC_METH_BEGIN(ni,iVGPaintGradient,GetB,0)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPaintGradient,GetB,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iVGPaintGradient,GetB,0)
#endif // niMinFeatures(20)

/** ni -> iVGPaintGradient::SetR/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintGradient::SetR/1 **/
IDLC_METH_BEGIN(ni,iVGPaintGradient,SetR,1)
	IDLC_DECL_VAR(tF32,afV)
	IDLC_BUF_TO_BASE(ni::eType_F32,afV)
	IDLC_METH_CALL_VOID(ni,iVGPaintGradient,SetR,1,(afV))
IDLC_METH_END(ni,iVGPaintGradient,SetR,1)
#endif // niMinFeatures(20)

/** ni -> iVGPaintGradient::GetR/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintGradient::GetR/0 **/
IDLC_METH_BEGIN(ni,iVGPaintGradient,GetR,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPaintGradient,GetR,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iVGPaintGradient,GetR,0)
#endif // niMinFeatures(20)

/** ni -> iVGPaintGradient::SetUnits/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintGradient::SetUnits/1 **/
IDLC_METH_BEGIN(ni,iVGPaintGradient,SetUnits,1)
	IDLC_DECL_VAR(eVGPaintUnits,aUnits)
	IDLC_BUF_TO_ENUM(eVGPaintUnits,aUnits)
	IDLC_METH_CALL_VOID(ni,iVGPaintGradient,SetUnits,1,(aUnits))
IDLC_METH_END(ni,iVGPaintGradient,SetUnits,1)
#endif // niMinFeatures(20)

/** ni -> iVGPaintGradient::GetUnits/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintGradient::GetUnits/0 **/
IDLC_METH_BEGIN(ni,iVGPaintGradient,GetUnits,0)
	IDLC_DECL_RETVAR(eVGPaintUnits,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPaintGradient,GetUnits,0,())
	IDLC_RET_FROM_ENUM(eVGPaintUnits,_Ret)
IDLC_METH_END(ni,iVGPaintGradient,GetUnits,0)
#endif // niMinFeatures(20)

/** ni -> iVGPaintGradient::GetGradientTable/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPaintGradient::GetGradientTable/0 **/
IDLC_METH_BEGIN(ni,iVGPaintGradient,GetGradientTable,0)
	IDLC_DECL_RETVAR(iVGGradientTable*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPaintGradient,GetGradientTable,0,())
	IDLC_RET_FROM_INTF(iVGGradientTable,_Ret)
IDLC_METH_END(ni,iVGPaintGradient,GetGradientTable,0)
#endif // niMinFeatures(20)

IDLC_END_INTF(ni,iVGPaintGradient)

#endif // if niMinFeatures(20)
IDLC_END_NAMESPACE()
// EOF //
