#include "../API/niUI/IVGPolygonTesselator.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iVGPolygonTesselator **/
/** iVGPolygonTesselator -> MIN FEATURES '20' **/
#if niMinFeatures(20)
IDLC_BEGIN_INTF(ni,iVGPolygonTesselator)
/** ni -> iVGPolygonTesselator::AddVertexF32/2 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPolygonTesselator::AddVertexF32/2 **/
IDLC_METH_BEGIN(ni,iVGPolygonTesselator,AddVertexF32,2)
	IDLC_DECL_VAR(tF32,x)
	IDLC_BUF_TO_BASE(ni::eType_F32,x)
	IDLC_DECL_VAR(tF32,y)
	IDLC_BUF_TO_BASE(ni::eType_F32,y)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPolygonTesselator,AddVertexF32,2,(x,y))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGPolygonTesselator,AddVertexF32,2)
#endif // niMinFeatures(20)

/** ni -> iVGPolygonTesselator::AddVertexF64/2 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPolygonTesselator::AddVertexF64/2 **/
IDLC_METH_BEGIN(ni,iVGPolygonTesselator,AddVertexF64,2)
	IDLC_DECL_VAR(tF64,x)
	IDLC_BUF_TO_BASE(ni::eType_F64,x)
	IDLC_DECL_VAR(tF64,y)
	IDLC_BUF_TO_BASE(ni::eType_F64,y)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPolygonTesselator,AddVertexF64,2,(x,y))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGPolygonTesselator,AddVertexF64,2)
#endif // niMinFeatures(20)

/** ni -> iVGPolygonTesselator::AddVertex/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPolygonTesselator::AddVertex/1 **/
IDLC_METH_BEGIN(ni,iVGPolygonTesselator,AddVertex,1)
	IDLC_DECL_VAR(sVec2f,vertex)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,vertex)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPolygonTesselator,AddVertex,1,(vertex))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGPolygonTesselator,AddVertex,1)
#endif // niMinFeatures(20)

/** ni -> iVGPolygonTesselator::GetNumVertices/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPolygonTesselator::GetNumVertices/0 **/
IDLC_METH_BEGIN(ni,iVGPolygonTesselator,GetNumVertices,0)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPolygonTesselator,GetNumVertices,0,())
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iVGPolygonTesselator,GetNumVertices,0)
#endif // niMinFeatures(20)

/** ni -> iVGPolygonTesselator::SubmitContour/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPolygonTesselator::SubmitContour/0 **/
IDLC_METH_BEGIN(ni,iVGPolygonTesselator,SubmitContour,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPolygonTesselator,SubmitContour,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGPolygonTesselator,SubmitContour,0)
#endif // niMinFeatures(20)

/** ni -> iVGPolygonTesselator::BeginPolygon/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPolygonTesselator::BeginPolygon/1 **/
IDLC_METH_BEGIN(ni,iVGPolygonTesselator,BeginPolygon,1)
	IDLC_DECL_VAR(tBool,abEvenOdd)
	IDLC_BUF_TO_BASE(ni::eType_I8,abEvenOdd)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPolygonTesselator,BeginPolygon,1,(abEvenOdd))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGPolygonTesselator,BeginPolygon,1)
#endif // niMinFeatures(20)

/** ni -> iVGPolygonTesselator::EndPolygon/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPolygonTesselator::EndPolygon/0 **/
IDLC_METH_BEGIN(ni,iVGPolygonTesselator,EndPolygon,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPolygonTesselator,EndPolygon,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVGPolygonTesselator,EndPolygon,0)
#endif // niMinFeatures(20)

/** ni -> iVGPolygonTesselator::GetTesselatedVertices/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iVGPolygonTesselator::GetTesselatedVertices/0 **/
IDLC_METH_BEGIN(ni,iVGPolygonTesselator,GetTesselatedVertices,0)
	IDLC_DECL_RETVAR(const tVec2fCVec*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVGPolygonTesselator,GetTesselatedVertices,0,())
	IDLC_RET_FROM_INTF(const tVec2fCVec,_Ret)
IDLC_METH_END(ni,iVGPolygonTesselator,GetTesselatedVertices,0)
#endif // niMinFeatures(20)

IDLC_END_INTF(ni,iVGPolygonTesselator)

#endif // if niMinFeatures(20)
IDLC_END_NAMESPACE()
// EOF //
