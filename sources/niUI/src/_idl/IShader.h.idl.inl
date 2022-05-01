#include "../API/niUI/IShader.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iShaderConstants **/
IDLC_BEGIN_INTF(ni,iShaderConstants)
/** ni -> iShaderConstants::Clone/0 **/
IDLC_METH_BEGIN(ni,iShaderConstants,Clone,0)
	IDLC_DECL_RETVAR(iShaderConstants*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShaderConstants,Clone,0,())
	IDLC_RET_FROM_INTF(iShaderConstants,_Ret)
IDLC_METH_END(ni,iShaderConstants,Clone,0)

/** ni -> iShaderConstants::GetMaxNumRegisters/0 **/
IDLC_METH_BEGIN(ni,iShaderConstants,GetMaxNumRegisters,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShaderConstants,GetMaxNumRegisters,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iShaderConstants,GetMaxNumRegisters,0)

/** ni -> iShaderConstants::AddConstant/3 **/
IDLC_METH_BEGIN(ni,iShaderConstants,AddConstant,3)
	IDLC_DECL_VAR(iHString*,ahspName)
	IDLC_BUF_TO_INTF(iHString,ahspName)
	IDLC_DECL_VAR(eShaderRegisterType,aType)
	IDLC_BUF_TO_ENUM(eShaderRegisterType,aType)
	IDLC_DECL_VAR(tU32,anSize)
	IDLC_BUF_TO_BASE(ni::eType_U32,anSize)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShaderConstants,AddConstant,3,(ahspName,aType,anSize))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iShaderConstants,AddConstant,3)

/** ni -> iShaderConstants::GetNumConstants/0 **/
IDLC_METH_BEGIN(ni,iShaderConstants,GetNumConstants,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShaderConstants,GetNumConstants,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iShaderConstants,GetNumConstants,0)

/** ni -> iShaderConstants::GetConstantIndex/1 **/
IDLC_METH_BEGIN(ni,iShaderConstants,GetConstantIndex,1)
	IDLC_DECL_VAR(iHString*,ahspName)
	IDLC_BUF_TO_INTF(iHString,ahspName)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShaderConstants,GetConstantIndex,1,(ahspName))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iShaderConstants,GetConstantIndex,1)

/** ni -> iShaderConstants::GetConstantName/1 **/
IDLC_METH_BEGIN(ni,iShaderConstants,GetConstantName,1)
	IDLC_DECL_VAR(tU32,anConstIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anConstIndex)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShaderConstants,GetConstantName,1,(anConstIndex))
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iShaderConstants,GetConstantName,1)

/** ni -> iShaderConstants::GetConstantSize/1 **/
IDLC_METH_BEGIN(ni,iShaderConstants,GetConstantSize,1)
	IDLC_DECL_VAR(tU32,anConstIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anConstIndex)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShaderConstants,GetConstantSize,1,(anConstIndex))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iShaderConstants,GetConstantSize,1)

/** ni -> iShaderConstants::GetConstantType/1 **/
IDLC_METH_BEGIN(ni,iShaderConstants,GetConstantType,1)
	IDLC_DECL_VAR(tU32,anConstIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anConstIndex)
	IDLC_DECL_RETVAR(eShaderRegisterType,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShaderConstants,GetConstantType,1,(anConstIndex))
	IDLC_RET_FROM_ENUM(eShaderRegisterType,_Ret)
IDLC_METH_END(ni,iShaderConstants,GetConstantType,1)

/** ni -> iShaderConstants::SetHwIndex/2 **/
IDLC_METH_BEGIN(ni,iShaderConstants,SetHwIndex,2)
	IDLC_DECL_VAR(tU32,anConstIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anConstIndex)
	IDLC_DECL_VAR(tU32,anRegisterIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anRegisterIndex)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShaderConstants,SetHwIndex,2,(anConstIndex,anRegisterIndex))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iShaderConstants,SetHwIndex,2)

/** ni -> iShaderConstants::GetHwIndex/1 **/
IDLC_METH_BEGIN(ni,iShaderConstants,GetHwIndex,1)
	IDLC_DECL_VAR(tU32,anConstIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anConstIndex)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShaderConstants,GetHwIndex,1,(anConstIndex))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iShaderConstants,GetHwIndex,1)

/** ni -> iShaderConstants::SetFloatArray/3 **/
IDLC_METH_BEGIN(ni,iShaderConstants,SetFloatArray,3)
	IDLC_DECL_VAR(tU32,anConstIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anConstIndex)
	IDLC_DECL_VAR(tVec4fCVec*,apV)
	IDLC_BUF_TO_INTF(tVec4fCVec,apV)
	IDLC_DECL_VAR(tU32,anSize)
	IDLC_BUF_TO_BASE(ni::eType_U32,anSize)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShaderConstants,SetFloatArray,3,(anConstIndex,apV,anSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iShaderConstants,SetFloatArray,3)

/** ni -> iShaderConstants::SetFloatPointer/3 -> NO AUTOMATION **/
/** ni -> iShaderConstants::SetFloat/2 **/
IDLC_METH_BEGIN(ni,iShaderConstants,SetFloat,2)
	IDLC_DECL_VAR(tU32,anConstIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anConstIndex)
	IDLC_DECL_VAR(sVec4f,aV)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,aV)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShaderConstants,SetFloat,2,(anConstIndex,aV))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iShaderConstants,SetFloat,2)

/** ni -> iShaderConstants::SetFloatMatrixArray/3 **/
IDLC_METH_BEGIN(ni,iShaderConstants,SetFloatMatrixArray,3)
	IDLC_DECL_VAR(tU32,anConstIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anConstIndex)
	IDLC_DECL_VAR(tMatrixfCVec*,apV)
	IDLC_BUF_TO_INTF(tMatrixfCVec,apV)
	IDLC_DECL_VAR(tU32,anSize)
	IDLC_BUF_TO_BASE(ni::eType_U32,anSize)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShaderConstants,SetFloatMatrixArray,3,(anConstIndex,apV,anSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iShaderConstants,SetFloatMatrixArray,3)

/** ni -> iShaderConstants::SetFloatMatrix/2 **/
IDLC_METH_BEGIN(ni,iShaderConstants,SetFloatMatrix,2)
	IDLC_DECL_VAR(tU32,anConstIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anConstIndex)
	IDLC_DECL_VAR(sMatrixf,aV)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Matrixf|ni::eTypeFlags_Pointer,aV)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShaderConstants,SetFloatMatrix,2,(anConstIndex,aV))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iShaderConstants,SetFloatMatrix,2)

/** ni -> iShaderConstants::SetIntArray/3 **/
IDLC_METH_BEGIN(ni,iShaderConstants,SetIntArray,3)
	IDLC_DECL_VAR(tU32,anConstIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anConstIndex)
	IDLC_DECL_VAR(tVec4iCVec*,apV)
	IDLC_BUF_TO_INTF(tVec4iCVec,apV)
	IDLC_DECL_VAR(tU32,anSize)
	IDLC_BUF_TO_BASE(ni::eType_U32,anSize)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShaderConstants,SetIntArray,3,(anConstIndex,apV,anSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iShaderConstants,SetIntArray,3)

/** ni -> iShaderConstants::SetIntPointer/3 -> NO AUTOMATION **/
/** ni -> iShaderConstants::SetInt/2 **/
IDLC_METH_BEGIN(ni,iShaderConstants,SetInt,2)
	IDLC_DECL_VAR(tU32,anConstIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anConstIndex)
	IDLC_DECL_VAR(sVec4i,aV)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4i|ni::eTypeFlags_Pointer,aV)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShaderConstants,SetInt,2,(anConstIndex,aV))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iShaderConstants,SetInt,2)

/** ni -> iShaderConstants::GetFloat/2 **/
IDLC_METH_BEGIN(ni,iShaderConstants,GetFloat,2)
	IDLC_DECL_VAR(tU32,anConstIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anConstIndex)
	IDLC_DECL_VAR(tU32,anOffset)
	IDLC_BUF_TO_BASE(ni::eType_U32,anOffset)
	IDLC_DECL_RETVAR(sVec4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShaderConstants,GetFloat,2,(anConstIndex,anOffset))
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iShaderConstants,GetFloat,2)

/** ni -> iShaderConstants::GetFloatMatrix/1 **/
IDLC_METH_BEGIN(ni,iShaderConstants,GetFloatMatrix,1)
	IDLC_DECL_VAR(tU32,anConstIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anConstIndex)
	IDLC_DECL_RETVAR(sMatrixf,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShaderConstants,GetFloatMatrix,1,(anConstIndex))
	IDLC_RET_FROM_BASE(ni::eType_Matrixf,_Ret)
IDLC_METH_END(ni,iShaderConstants,GetFloatMatrix,1)

/** ni -> iShaderConstants::GetFloatPointer/1 -> NO AUTOMATION **/
/** ni -> iShaderConstants::GetInt/2 **/
IDLC_METH_BEGIN(ni,iShaderConstants,GetInt,2)
	IDLC_DECL_VAR(tU32,anConstIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anConstIndex)
	IDLC_DECL_VAR(tU32,anOffset)
	IDLC_BUF_TO_BASE(ni::eType_U32,anOffset)
	IDLC_DECL_RETVAR(sVec4i,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShaderConstants,GetInt,2,(anConstIndex,anOffset))
	IDLC_RET_FROM_BASE(ni::eType_Vec4i,_Ret)
IDLC_METH_END(ni,iShaderConstants,GetInt,2)

/** ni -> iShaderConstants::GetIntPointer/1 -> NO AUTOMATION **/
/** ni -> iShaderConstants::GetDescStructPtr/0 **/
IDLC_METH_BEGIN(ni,iShaderConstants,GetDescStructPtr,0)
	IDLC_DECL_RETVAR(tPtr,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShaderConstants,GetDescStructPtr,0,())
	IDLC_RET_FROM_PTR(ni::eType_Ptr,_Ret)
IDLC_METH_END(ni,iShaderConstants,GetDescStructPtr,0)

/** ni -> iShaderConstants::SetConstantMetadata/2 **/
IDLC_METH_BEGIN(ni,iShaderConstants,SetConstantMetadata,2)
	IDLC_DECL_VAR(tU32,anConstIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anConstIndex)
	IDLC_DECL_VAR(iHString*,ahspMetadata)
	IDLC_BUF_TO_INTF(iHString,ahspMetadata)
	IDLC_METH_CALL_VOID(ni,iShaderConstants,SetConstantMetadata,2,(anConstIndex,ahspMetadata))
IDLC_METH_END(ni,iShaderConstants,SetConstantMetadata,2)

/** ni -> iShaderConstants::GetConstantMetadata/1 **/
IDLC_METH_BEGIN(ni,iShaderConstants,GetConstantMetadata,1)
	IDLC_DECL_VAR(tU32,anConstIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anConstIndex)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShaderConstants,GetConstantMetadata,1,(anConstIndex))
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iShaderConstants,GetConstantMetadata,1)

IDLC_END_INTF(ni,iShaderConstants)

/** interface : iShader **/
IDLC_BEGIN_INTF(ni,iShader)
/** ni -> iShader::GetUnit/0 **/
IDLC_METH_BEGIN(ni,iShader,GetUnit,0)
	IDLC_DECL_RETVAR(eShaderUnit,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShader,GetUnit,0,())
	IDLC_RET_FROM_ENUM(eShaderUnit,_Ret)
IDLC_METH_END(ni,iShader,GetUnit,0)

/** ni -> iShader::GetProfile/0 **/
IDLC_METH_BEGIN(ni,iShader,GetProfile,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShader,GetProfile,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iShader,GetProfile,0)

/** ni -> iShader::GetConstants/0 **/
IDLC_METH_BEGIN(ni,iShader,GetConstants,0)
	IDLC_DECL_RETVAR(const iShaderConstants*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShader,GetConstants,0,())
	IDLC_RET_FROM_INTF(const iShaderConstants,_Ret)
IDLC_METH_END(ni,iShader,GetConstants,0)

/** ni -> iShader::GetCode/0 **/
IDLC_METH_BEGIN(ni,iShader,GetCode,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShader,GetCode,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iShader,GetCode,0)

/** ni -> iShader::GetDescStructPtr/0 **/
IDLC_METH_BEGIN(ni,iShader,GetDescStructPtr,0)
	IDLC_DECL_RETVAR(tPtr,_Ret)
	IDLC_METH_CALL(_Ret,ni,iShader,GetDescStructPtr,0,())
	IDLC_RET_FROM_PTR(ni::eType_Ptr,_Ret)
IDLC_METH_END(ni,iShader,GetDescStructPtr,0)

IDLC_END_INTF(ni,iShader)

/** interface : iGLShader **/
IDLC_BEGIN_INTF(ni,iGLShader)
/** ni -> iGLShader::GetVertexAttributeLocationArray/0 -> NO AUTOMATION **/
/** ni -> iGLShader::BeforeDraw/0 **/
IDLC_METH_BEGIN(ni,iGLShader,BeforeDraw,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGLShader,BeforeDraw,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGLShader,BeforeDraw,0)

/** ni -> iGLShader::AfterDraw/0 **/
IDLC_METH_BEGIN(ni,iGLShader,AfterDraw,0)
	IDLC_METH_CALL_VOID(ni,iGLShader,AfterDraw,0,())
IDLC_METH_END(ni,iGLShader,AfterDraw,0)

IDLC_END_INTF(ni,iGLShader)

IDLC_END_NAMESPACE()
// EOF //
