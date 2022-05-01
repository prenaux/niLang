#include "../API/niUI/IVGGradientTable.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iVGGradientTable **/
/** iVGGradientTable -> MIN FEATURES '20' **/
#if niMinFeatures(20)
IDLC_BEGIN_INTF(ni,iVGGradientTable)
/** ni -> iVGGradientTable::Copy/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGGradientTable::Copy/1 **/
IDLC_METH_BEGIN(ni,iVGGradientTable,Copy,1)
	IDLC_DECL_VAR(iVGGradientTable*,apSrc)
	IDLC_BUF_TO_INTF(iVGGradientTable,apSrc)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGGradientTable,Copy,1,(apSrc))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGGradientTable,Copy,1)
#endif // niMinFeatures(20)

/** ni -> iVGGradientTable::Clone/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGGradientTable::Clone/0 **/
IDLC_METH_BEGIN(ni,iVGGradientTable,Clone,0)
	IDLC_DECL_RETVAR(iVGGradientTable*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGGradientTable,Clone,0,())
	IDLC_RET_FROM_INTF(iVGGradientTable,_Ret)
IDLC_METH_END(ni,iVGGradientTable,Clone,0)
#endif // niMinFeatures(20)

/** ni -> iVGGradientTable::GetSize/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGGradientTable::GetSize/0 **/
IDLC_METH_BEGIN(ni,iVGGradientTable,GetSize,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGGradientTable,GetSize,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iVGGradientTable,GetSize,0)
#endif // niMinFeatures(20)

/** ni -> iVGGradientTable::SetColor/2 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGGradientTable::SetColor/2 **/
IDLC_METH_BEGIN(ni,iVGGradientTable,SetColor,2)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_VAR(sColor4f,aColor)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,aColor)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGGradientTable,SetColor,2,(anIndex,aColor))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGGradientTable,SetColor,2)
#endif // niMinFeatures(20)

/** ni -> iVGGradientTable::GetColor/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGGradientTable::GetColor/1 **/
IDLC_METH_BEGIN(ni,iVGGradientTable,GetColor,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(sColor4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGGradientTable,GetColor,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iVGGradientTable,GetColor,1)
#endif // niMinFeatures(20)

/** ni -> iVGGradientTable::SetColorRange/3 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGGradientTable::SetColorRange/3 **/
IDLC_METH_BEGIN(ni,iVGGradientTable,SetColorRange,3)
	IDLC_DECL_VAR(tU32,anStart)
	IDLC_BUF_TO_BASE(ni::eType_U32,anStart)
	IDLC_DECL_VAR(tU32,anEnd)
	IDLC_BUF_TO_BASE(ni::eType_U32,anEnd)
	IDLC_DECL_VAR(sColor4f,avColor)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,avColor)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGGradientTable,SetColorRange,3,(anStart,anEnd,avColor))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGGradientTable,SetColorRange,3)
#endif // niMinFeatures(20)

/** ni -> iVGGradientTable::GenerateTwoColors/2 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGGradientTable::GenerateTwoColors/2 **/
IDLC_METH_BEGIN(ni,iVGGradientTable,GenerateTwoColors,2)
	IDLC_DECL_VAR(sColor4f,aStartColor)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,aStartColor)
	IDLC_DECL_VAR(sColor4f,aEndColor)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,aEndColor)
	IDLC_METH_CALL_VOID(ni,iVGGradientTable,GenerateTwoColors,2,(aStartColor,aEndColor))
IDLC_METH_END(ni,iVGGradientTable,GenerateTwoColors,2)
#endif // niMinFeatures(20)

/** ni -> iVGGradientTable::GenerateStops/2 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGGradientTable::GenerateStops/2 **/
IDLC_METH_BEGIN(ni,iVGGradientTable,GenerateStops,2)
	IDLC_DECL_VAR(tF32CVec*,apOffsets)
	IDLC_BUF_TO_INTF(tF32CVec,apOffsets)
	IDLC_DECL_VAR(tVec4fCVec*,apColors)
	IDLC_BUF_TO_INTF(tVec4fCVec,apColors)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGGradientTable,GenerateStops,2,(apOffsets,apColors))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGGradientTable,GenerateStops,2)
#endif // niMinFeatures(20)

/** ni -> iVGGradientTable::CreateImage/7 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGGradientTable::CreateImage/7 **/
IDLC_METH_BEGIN(ni,iVGGradientTable,CreateImage,7)
	IDLC_DECL_VAR(eVGGradientType,aType)
	IDLC_BUF_TO_ENUM(eVGGradientType,aType)
	IDLC_DECL_VAR(eVGWrapType,aWrapType)
	IDLC_BUF_TO_ENUM(eVGWrapType,aWrapType)
	IDLC_DECL_VAR(iVGTransform*,apTransform)
	IDLC_BUF_TO_INTF(iVGTransform,apTransform)
	IDLC_DECL_VAR(tU32,anWidth)
	IDLC_BUF_TO_BASE(ni::eType_U32,anWidth)
	IDLC_DECL_VAR(tU32,anHeight)
	IDLC_BUF_TO_BASE(ni::eType_U32,anHeight)
	IDLC_DECL_VAR(tI32,anD1)
	IDLC_BUF_TO_BASE(ni::eType_I32,anD1)
	IDLC_DECL_VAR(tI32,anD2)
	IDLC_BUF_TO_BASE(ni::eType_I32,anD2)
	IDLC_DECL_RETVAR(iVGImage*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGGradientTable,CreateImage,7,(aType,aWrapType,apTransform,anWidth,anHeight,anD1,anD2))
	IDLC_RET_FROM_INTF(iVGImage,_Ret)
IDLC_METH_END(ni,iVGGradientTable,CreateImage,7)
#endif // niMinFeatures(20)

IDLC_END_INTF(ni,iVGGradientTable)

#endif // if niMinFeatures(20)
IDLC_END_NAMESPACE()
// EOF //
