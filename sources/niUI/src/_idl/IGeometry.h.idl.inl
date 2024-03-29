// clang-format off
//
// Autogenerated IDL definition for 'API/niUI/IGeometry.h'.
//
#include "../API/niUI/IGeometry.h"

#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif

IDLC_BEGIN_NAMESPACE()

/** NAMESPACE : ni **/
/** interface : iGeometrySubset **/
IDLC_BEGIN_INTF(ni,iGeometrySubset)
/** ni -> iGeometrySubset::SetID/1 **/
IDLC_METH_BEGIN(ni,iGeometrySubset,SetID,1)
	IDLC_DECL_VAR(tU32,anID)
	IDLC_BUF_TO_BASE(ni::eType_U32,anID)
	IDLC_METH_CALL_VOID(ni,iGeometrySubset,SetID,1,(anID))
IDLC_METH_END(ni,iGeometrySubset,SetID,1)

/** ni -> iGeometrySubset::GetID/0 **/
IDLC_METH_BEGIN(ni,iGeometrySubset,GetID,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGeometrySubset,GetID,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iGeometrySubset,GetID,0)

/** ni -> iGeometrySubset::SetFirstIndex/1 **/
IDLC_METH_BEGIN(ni,iGeometrySubset,SetFirstIndex,1)
	IDLC_DECL_VAR(tU32,anFirstIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anFirstIndex)
	IDLC_METH_CALL_VOID(ni,iGeometrySubset,SetFirstIndex,1,(anFirstIndex))
IDLC_METH_END(ni,iGeometrySubset,SetFirstIndex,1)

/** ni -> iGeometrySubset::GetFirstIndex/0 **/
IDLC_METH_BEGIN(ni,iGeometrySubset,GetFirstIndex,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGeometrySubset,GetFirstIndex,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iGeometrySubset,GetFirstIndex,0)

/** ni -> iGeometrySubset::SetNumIndices/1 **/
IDLC_METH_BEGIN(ni,iGeometrySubset,SetNumIndices,1)
	IDLC_DECL_VAR(tU32,anNumIndices)
	IDLC_BUF_TO_BASE(ni::eType_U32,anNumIndices)
	IDLC_METH_CALL_VOID(ni,iGeometrySubset,SetNumIndices,1,(anNumIndices))
IDLC_METH_END(ni,iGeometrySubset,SetNumIndices,1)

/** ni -> iGeometrySubset::GetNumIndices/0 **/
IDLC_METH_BEGIN(ni,iGeometrySubset,GetNumIndices,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGeometrySubset,GetNumIndices,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iGeometrySubset,GetNumIndices,0)

/** ni -> iGeometrySubset::SetMaterial/1 **/
IDLC_METH_BEGIN(ni,iGeometrySubset,SetMaterial,1)
	IDLC_DECL_VAR(tU32,anMaterial)
	IDLC_BUF_TO_BASE(ni::eType_U32,anMaterial)
	IDLC_METH_CALL_VOID(ni,iGeometrySubset,SetMaterial,1,(anMaterial))
IDLC_METH_END(ni,iGeometrySubset,SetMaterial,1)

/** ni -> iGeometrySubset::GetMaterial/0 **/
IDLC_METH_BEGIN(ni,iGeometrySubset,GetMaterial,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGeometrySubset,GetMaterial,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iGeometrySubset,GetMaterial,0)

IDLC_END_INTF(ni,iGeometrySubset)

/** interface : iGeometry **/
IDLC_BEGIN_INTF(ni,iGeometry)
/** ni -> iGeometry::GetType/0 **/
IDLC_METH_BEGIN(ni,iGeometry,GetType,0)
	IDLC_DECL_RETVAR(eGeometryType,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGeometry,GetType,0,())
	IDLC_RET_FROM_ENUM(eGeometryType,_Ret)
IDLC_METH_END(ni,iGeometry,GetType,0)

/** ni -> iGeometry::GetVertexArray/0 **/
IDLC_METH_BEGIN(ni,iGeometry,GetVertexArray,0)
	IDLC_DECL_RETVAR(iVertexArray*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGeometry,GetVertexArray,0,())
	IDLC_RET_FROM_INTF(iVertexArray,_Ret)
IDLC_METH_END(ni,iGeometry,GetVertexArray,0)

/** ni -> iGeometry::GetIndexArray/0 **/
IDLC_METH_BEGIN(ni,iGeometry,GetIndexArray,0)
	IDLC_DECL_RETVAR(iIndexArray*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGeometry,GetIndexArray,0,())
	IDLC_RET_FROM_INTF(iIndexArray,_Ret)
IDLC_METH_END(ni,iGeometry,GetIndexArray,0)

/** ni -> iGeometry::Generate/2 **/
IDLC_METH_BEGIN(ni,iGeometry,Generate,2)
	IDLC_DECL_VAR(eGeometryGenerate,aGenerate)
	IDLC_BUF_TO_ENUM(eGeometryGenerate,aGenerate)
	IDLC_DECL_VAR(tF32,fEpsilon)
	IDLC_BUF_TO_BASE(ni::eType_F32,fEpsilon)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGeometry,Generate,2,(aGenerate,fEpsilon))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGeometry,Generate,2)

/** ni -> iGeometry::GetNumFaces/0 **/
IDLC_METH_BEGIN(ni,iGeometry,GetNumFaces,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGeometry,GetNumFaces,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iGeometry,GetNumFaces,0)

/** ni -> iGeometry::SetFacesSubsetsIDs/1 **/
IDLC_METH_BEGIN(ni,iGeometry,SetFacesSubsetsIDs,1)
	IDLC_DECL_VAR(tU32*,apIDs)
	IDLC_BUF_TO_NULL(ni::eTypeFlags_Constant|ni::eType_U32|ni::eTypeFlags_Pointer,apIDs)
	IDLC_METH_CALL_VOID(ni,iGeometry,SetFacesSubsetsIDs,1,(apIDs))
IDLC_METH_END(ni,iGeometry,SetFacesSubsetsIDs,1)

/** ni -> iGeometry::GetFacesSubsetsIDs/0 -> NO AUTOMATION **/
/** ni -> iGeometry::GetAdjacencyArray/0 -> NO AUTOMATION **/
/** ni -> iGeometry::GetNumSubsets/0 **/
IDLC_METH_BEGIN(ni,iGeometry,GetNumSubsets,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGeometry,GetNumSubsets,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iGeometry,GetNumSubsets,0)

/** ni -> iGeometry::GetSubset/1 **/
IDLC_METH_BEGIN(ni,iGeometry,GetSubset,1)
	IDLC_DECL_VAR(tU32,aulIdx)
	IDLC_BUF_TO_BASE(ni::eType_U32,aulIdx)
	IDLC_DECL_RETVAR(iGeometrySubset*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGeometry,GetSubset,1,(aulIdx))
	IDLC_RET_FROM_INTF(iGeometrySubset,_Ret)
IDLC_METH_END(ni,iGeometry,GetSubset,1)

/** ni -> iGeometry::GetSubsetIndex/1 **/
IDLC_METH_BEGIN(ni,iGeometry,GetSubsetIndex,1)
	IDLC_DECL_VAR(tU32,aulID)
	IDLC_BUF_TO_BASE(ni::eType_U32,aulID)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGeometry,GetSubsetIndex,1,(aulID))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iGeometry,GetSubsetIndex,1)

/** ni -> iGeometry::AddSubset/4 **/
IDLC_METH_BEGIN(ni,iGeometry,AddSubset,4)
	IDLC_DECL_VAR(tU32,anID)
	IDLC_BUF_TO_BASE(ni::eType_U32,anID)
	IDLC_DECL_VAR(tU32,anFirstIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anFirstIndex)
	IDLC_DECL_VAR(tU32,anNumIndices)
	IDLC_BUF_TO_BASE(ni::eType_U32,anNumIndices)
	IDLC_DECL_VAR(tU32,anMaterial)
	IDLC_BUF_TO_BASE(ni::eType_U32,anMaterial)
	IDLC_DECL_RETVAR(iGeometrySubset*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGeometry,AddSubset,4,(anID,anFirstIndex,anNumIndices,anMaterial))
	IDLC_RET_FROM_INTF(iGeometrySubset,_Ret)
IDLC_METH_END(ni,iGeometry,AddSubset,4)

/** ni -> iGeometry::RemoveSubset/1 **/
IDLC_METH_BEGIN(ni,iGeometry,RemoveSubset,1)
	IDLC_DECL_VAR(tU32,aulIdx)
	IDLC_BUF_TO_BASE(ni::eType_U32,aulIdx)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGeometry,RemoveSubset,1,(aulIdx))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGeometry,RemoveSubset,1)

/** ni -> iGeometry::Clone/2 **/
IDLC_METH_BEGIN(ni,iGeometry,Clone,2)
	IDLC_DECL_VAR(tGeometryCreateFlags,aFlags)
	IDLC_BUF_TO_ENUM(tGeometryCreateFlags,aFlags)
	IDLC_DECL_VAR(tFVF,aFVF)
	IDLC_BUF_TO_BASE(ni::eType_U32,aFVF)
	IDLC_DECL_RETVAR(iGeometry*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGeometry,Clone,2,(aFlags,aFVF))
	IDLC_RET_FROM_INTF(iGeometry,_Ret)
IDLC_METH_END(ni,iGeometry,Clone,2)

/** ni -> iGeometry::SetDrawOp/2 **/
IDLC_METH_BEGIN(ni,iGeometry,SetDrawOp,2)
	IDLC_DECL_VAR(iDrawOperation*,apDrawOp)
	IDLC_BUF_TO_INTF(iDrawOperation,apDrawOp)
	IDLC_DECL_VAR(tU32,aulSubsetIdx)
	IDLC_BUF_TO_BASE(ni::eType_U32,aulSubsetIdx)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGeometry,SetDrawOp,2,(apDrawOp,aulSubsetIdx))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGeometry,SetDrawOp,2)

/** ni -> iGeometry::Optimize/1 **/
IDLC_METH_BEGIN(ni,iGeometry,Optimize,1)
	IDLC_DECL_VAR(tGeometryOptimizeFlags,aFlags)
	IDLC_BUF_TO_ENUM(tGeometryOptimizeFlags,aFlags)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGeometry,Optimize,1,(aFlags))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGeometry,Optimize,1)

IDLC_END_INTF(ni,iGeometry)

/** interface : iGeometryModifier **/
IDLC_BEGIN_INTF(ni,iGeometryModifier)
/** ni -> iGeometryModifier::SetGeometry/1 **/
IDLC_METH_BEGIN(ni,iGeometryModifier,SetGeometry,1)
	IDLC_DECL_VAR(iGeometry*,pGeometry)
	IDLC_BUF_TO_INTF(iGeometry,pGeometry)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGeometryModifier,SetGeometry,1,(pGeometry))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGeometryModifier,SetGeometry,1)

/** ni -> iGeometryModifier::GetGeometry/0 **/
IDLC_METH_BEGIN(ni,iGeometryModifier,GetGeometry,0)
	IDLC_DECL_RETVAR(iGeometry*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGeometryModifier,GetGeometry,0,())
	IDLC_RET_FROM_INTF(iGeometry,_Ret)
IDLC_METH_END(ni,iGeometryModifier,GetGeometry,0)

/** ni -> iGeometryModifier::GetModifiedGeometry/0 **/
IDLC_METH_BEGIN(ni,iGeometryModifier,GetModifiedGeometry,0)
	IDLC_DECL_RETVAR(iGeometry*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGeometryModifier,GetModifiedGeometry,0,())
	IDLC_RET_FROM_INTF(iGeometry,_Ret)
IDLC_METH_END(ni,iGeometryModifier,GetModifiedGeometry,0)

/** ni -> iGeometryModifier::Update/0 **/
IDLC_METH_BEGIN(ni,iGeometryModifier,Update,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGeometryModifier,Update,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGeometryModifier,Update,0)

IDLC_END_INTF(ni,iGeometryModifier)

IDLC_END_NAMESPACE()
// EOF //
