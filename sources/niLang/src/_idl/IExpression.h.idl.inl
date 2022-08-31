#include "../API/niLang/IExpression.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iExpressionVariable **/
IDLC_BEGIN_INTF(ni,iExpressionVariable)
/** ni -> iExpressionVariable::SetName/1 **/
IDLC_METH_BEGIN(ni,iExpressionVariable,SetName,1)
	IDLC_DECL_VAR(iHString*,ahspString)
	IDLC_BUF_TO_INTF(iHString,ahspString)
	IDLC_METH_CALL_VOID(ni,iExpressionVariable,SetName,1,(ahspString))
IDLC_METH_END(ni,iExpressionVariable,SetName,1)

/** ni -> iExpressionVariable::GetName/0 **/
IDLC_METH_BEGIN(ni,iExpressionVariable,GetName,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionVariable,GetName,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iExpressionVariable,GetName,0)

/** ni -> iExpressionVariable::Copy/1 **/
IDLC_METH_BEGIN(ni,iExpressionVariable,Copy,1)
	IDLC_DECL_VAR(iExpressionVariable*,apVar)
	IDLC_BUF_TO_INTF(iExpressionVariable,apVar)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionVariable,Copy,1,(apVar))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iExpressionVariable,Copy,1)

/** ni -> iExpressionVariable::Clone/0 **/
IDLC_METH_BEGIN(ni,iExpressionVariable,Clone,0)
	IDLC_DECL_RETVAR(iExpressionVariable*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionVariable,Clone,0,())
	IDLC_RET_FROM_INTF(iExpressionVariable,_Ret)
IDLC_METH_END(ni,iExpressionVariable,Clone,0)

/** ni -> iExpressionVariable::GetType/0 **/
IDLC_METH_BEGIN(ni,iExpressionVariable,GetType,0)
	IDLC_DECL_RETVAR(eExpressionVariableType,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionVariable,GetType,0,())
	IDLC_RET_FROM_ENUM(eExpressionVariableType,_Ret)
IDLC_METH_END(ni,iExpressionVariable,GetType,0)

/** ni -> iExpressionVariable::GetFlags/0 **/
IDLC_METH_BEGIN(ni,iExpressionVariable,GetFlags,0)
	IDLC_DECL_RETVAR(tExpressionVariableFlags,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionVariable,GetFlags,0,())
	IDLC_RET_FROM_ENUM(tExpressionVariableFlags,_Ret)
IDLC_METH_END(ni,iExpressionVariable,GetFlags,0)

/** ni -> iExpressionVariable::SetFloat/1 **/
IDLC_METH_BEGIN(ni,iExpressionVariable,SetFloat,1)
	IDLC_DECL_VAR(tF64,aV)
	IDLC_BUF_TO_BASE(ni::eType_F64,aV)
	IDLC_METH_CALL_VOID(ni,iExpressionVariable,SetFloat,1,(aV))
IDLC_METH_END(ni,iExpressionVariable,SetFloat,1)

/** ni -> iExpressionVariable::GetFloat/0 **/
IDLC_METH_BEGIN(ni,iExpressionVariable,GetFloat,0)
	IDLC_DECL_RETVAR(tF64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionVariable,GetFloat,0,())
	IDLC_RET_FROM_BASE(ni::eType_F64,_Ret)
IDLC_METH_END(ni,iExpressionVariable,GetFloat,0)

/** ni -> iExpressionVariable::SetVec2/1 **/
IDLC_METH_BEGIN(ni,iExpressionVariable,SetVec2,1)
	IDLC_DECL_VAR(sVec2f,aV)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,aV)
	IDLC_METH_CALL_VOID(ni,iExpressionVariable,SetVec2,1,(aV))
IDLC_METH_END(ni,iExpressionVariable,SetVec2,1)

/** ni -> iExpressionVariable::GetVec2/0 **/
IDLC_METH_BEGIN(ni,iExpressionVariable,GetVec2,0)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionVariable,GetVec2,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iExpressionVariable,GetVec2,0)

/** ni -> iExpressionVariable::SetVec3/1 **/
IDLC_METH_BEGIN(ni,iExpressionVariable,SetVec3,1)
	IDLC_DECL_VAR(sVec3f,aV)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,aV)
	IDLC_METH_CALL_VOID(ni,iExpressionVariable,SetVec3,1,(aV))
IDLC_METH_END(ni,iExpressionVariable,SetVec3,1)

/** ni -> iExpressionVariable::GetVec3/0 **/
IDLC_METH_BEGIN(ni,iExpressionVariable,GetVec3,0)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionVariable,GetVec3,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iExpressionVariable,GetVec3,0)

/** ni -> iExpressionVariable::SetVec4/1 **/
IDLC_METH_BEGIN(ni,iExpressionVariable,SetVec4,1)
	IDLC_DECL_VAR(sVec4f,aV)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,aV)
	IDLC_METH_CALL_VOID(ni,iExpressionVariable,SetVec4,1,(aV))
IDLC_METH_END(ni,iExpressionVariable,SetVec4,1)

/** ni -> iExpressionVariable::GetVec4/0 **/
IDLC_METH_BEGIN(ni,iExpressionVariable,GetVec4,0)
	IDLC_DECL_RETVAR(sVec4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionVariable,GetVec4,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iExpressionVariable,GetVec4,0)

/** ni -> iExpressionVariable::SetMatrix/1 **/
IDLC_METH_BEGIN(ni,iExpressionVariable,SetMatrix,1)
	IDLC_DECL_VAR(sMatrixf,aV)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Matrixf|ni::eTypeFlags_Pointer,aV)
	IDLC_METH_CALL_VOID(ni,iExpressionVariable,SetMatrix,1,(aV))
IDLC_METH_END(ni,iExpressionVariable,SetMatrix,1)

/** ni -> iExpressionVariable::GetMatrix/0 **/
IDLC_METH_BEGIN(ni,iExpressionVariable,GetMatrix,0)
	IDLC_DECL_RETVAR(sMatrixf,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionVariable,GetMatrix,0,())
	IDLC_RET_FROM_BASE(ni::eType_Matrixf,_Ret)
IDLC_METH_END(ni,iExpressionVariable,GetMatrix,0)

/** ni -> iExpressionVariable::SetString/1 **/
IDLC_METH_BEGIN(ni,iExpressionVariable,SetString,1)
	IDLC_DECL_VAR(cString,aV)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_String|ni::eTypeFlags_Pointer,aV)
	IDLC_METH_CALL_VOID(ni,iExpressionVariable,SetString,1,(aV))
IDLC_METH_END(ni,iExpressionVariable,SetString,1)

/** ni -> iExpressionVariable::GetString/0 **/
IDLC_METH_BEGIN(ni,iExpressionVariable,GetString,0)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionVariable,GetString,0,())
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iExpressionVariable,GetString,0)

IDLC_END_INTF(ni,iExpressionVariable)

/** interface : iExpression **/
IDLC_BEGIN_INTF(ni,iExpression)
/** ni -> iExpression::Eval/0 **/
IDLC_METH_BEGIN(ni,iExpression,Eval,0)
	IDLC_DECL_RETVAR(Ptr<iExpressionVariable>,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpression,Eval,0,())
	IDLC_RET_FROM_INTF(,_Ret)
IDLC_METH_END(ni,iExpression,Eval,0)

/** ni -> iExpression::GetEvalResult/0 **/
IDLC_METH_BEGIN(ni,iExpression,GetEvalResult,0)
	IDLC_DECL_RETVAR(iExpressionVariable*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpression,GetEvalResult,0,())
	IDLC_RET_FROM_INTF(iExpressionVariable,_Ret)
IDLC_METH_END(ni,iExpression,GetEvalResult,0)

/** ni -> iExpression::GetContext/0 **/
IDLC_METH_BEGIN(ni,iExpression,GetContext,0)
	IDLC_DECL_RETVAR(iExpressionContext*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpression,GetContext,0,())
	IDLC_RET_FROM_INTF(iExpressionContext,_Ret)
IDLC_METH_END(ni,iExpression,GetContext,0)

IDLC_END_INTF(ni,iExpression)

/** interface : iExpressionURLResolver **/
IDLC_BEGIN_INTF(ni,iExpressionURLResolver)
/** ni -> iExpressionURLResolver::ResolveURL/1 **/
IDLC_METH_BEGIN(ni,iExpressionURLResolver,ResolveURL,1)
	IDLC_DECL_VAR(achar*,aURL)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aURL)
	IDLC_DECL_RETVAR(Var,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionURLResolver,ResolveURL,1,(aURL))
	IDLC_RET_FROM_BASE(ni::eType_Variant,_Ret)
IDLC_METH_END(ni,iExpressionURLResolver,ResolveURL,1)

IDLC_END_INTF(ni,iExpressionURLResolver)

/** interface : iExpressionContext **/
IDLC_BEGIN_INTF(ni,iExpressionContext)
/** ni -> iExpressionContext::GetParentContext/0 **/
IDLC_METH_BEGIN(ni,iExpressionContext,GetParentContext,0)
	IDLC_DECL_RETVAR(iExpressionContext*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionContext,GetParentContext,0,())
	IDLC_RET_FROM_INTF(iExpressionContext,_Ret)
IDLC_METH_END(ni,iExpressionContext,GetParentContext,0)

/** ni -> iExpressionContext::CreateContext/0 **/
IDLC_METH_BEGIN(ni,iExpressionContext,CreateContext,0)
	IDLC_DECL_RETVAR(iExpressionContext*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionContext,CreateContext,0,())
	IDLC_RET_FROM_INTF(iExpressionContext,_Ret)
IDLC_METH_END(ni,iExpressionContext,CreateContext,0)

/** ni -> iExpressionContext::CreateVariable/3 **/
IDLC_METH_BEGIN(ni,iExpressionContext,CreateVariable,3)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(eExpressionVariableType,aType)
	IDLC_BUF_TO_ENUM(eExpressionVariableType,aType)
	IDLC_DECL_VAR(tExpressionVariableFlags,aFlags)
	IDLC_BUF_TO_ENUM(tExpressionVariableFlags,aFlags)
	IDLC_DECL_RETVAR(iExpressionVariable*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionContext,CreateVariable,3,(aaszName,aType,aFlags))
	IDLC_RET_FROM_INTF(iExpressionVariable,_Ret)
IDLC_METH_END(ni,iExpressionContext,CreateVariable,3)

/** ni -> iExpressionContext::CreateVariableFromExpr/3 **/
IDLC_METH_BEGIN(ni,iExpressionContext,CreateVariableFromExpr,3)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(achar*,aaszExpr)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszExpr)
	IDLC_DECL_VAR(tExpressionVariableFlags,aFlags)
	IDLC_BUF_TO_ENUM(tExpressionVariableFlags,aFlags)
	IDLC_DECL_RETVAR(iExpressionVariable*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionContext,CreateVariableFromExpr,3,(aaszName,aaszExpr,aFlags))
	IDLC_RET_FROM_INTF(iExpressionVariable,_Ret)
IDLC_METH_END(ni,iExpressionContext,CreateVariableFromExpr,3)

/** ni -> iExpressionContext::CreateVariableFromRunnable/4 **/
IDLC_METH_BEGIN(ni,iExpressionContext,CreateVariableFromRunnable,4)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(eExpressionVariableType,aType)
	IDLC_BUF_TO_ENUM(eExpressionVariableType,aType)
	IDLC_DECL_VAR(iRunnable*,apRunnable)
	IDLC_BUF_TO_INTF(iRunnable,apRunnable)
	IDLC_DECL_VAR(tExpressionVariableFlags,aFlags)
	IDLC_BUF_TO_ENUM(tExpressionVariableFlags,aFlags)
	IDLC_DECL_RETVAR(iExpressionVariable*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionContext,CreateVariableFromRunnable,4,(aaszName,aType,apRunnable,aFlags))
	IDLC_RET_FROM_INTF(iExpressionVariable,_Ret)
IDLC_METH_END(ni,iExpressionContext,CreateVariableFromRunnable,4)

/** ni -> iExpressionContext::CreateVariableFromVar/3 **/
IDLC_METH_BEGIN(ni,iExpressionContext,CreateVariableFromVar,3)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(Var,aInitialValue)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,aInitialValue)
	IDLC_DECL_VAR(tExpressionVariableFlags,aFlags)
	IDLC_BUF_TO_ENUM(tExpressionVariableFlags,aFlags)
	IDLC_DECL_RETVAR(iExpressionVariable*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionContext,CreateVariableFromVar,3,(aaszName,aInitialValue,aFlags))
	IDLC_RET_FROM_INTF(iExpressionVariable,_Ret)
IDLC_METH_END(ni,iExpressionContext,CreateVariableFromVar,3)

/** ni -> iExpressionContext::AddVariable/1 **/
IDLC_METH_BEGIN(ni,iExpressionContext,AddVariable,1)
	IDLC_DECL_VAR(iExpressionVariable*,apVar)
	IDLC_BUF_TO_INTF(iExpressionVariable,apVar)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionContext,AddVariable,1,(apVar))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iExpressionContext,AddVariable,1)

/** ni -> iExpressionContext::RemoveVariable/1 **/
IDLC_METH_BEGIN(ni,iExpressionContext,RemoveVariable,1)
	IDLC_DECL_VAR(iExpressionVariable*,apVariable)
	IDLC_BUF_TO_INTF(iExpressionVariable,apVariable)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionContext,RemoveVariable,1,(apVariable))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iExpressionContext,RemoveVariable,1)

/** ni -> iExpressionContext::FindVariable/1 **/
IDLC_METH_BEGIN(ni,iExpressionContext,FindVariable,1)
	IDLC_DECL_VAR(iHString*,ahspName)
	IDLC_BUF_TO_INTF(iHString,ahspName)
	IDLC_DECL_RETVAR(iExpressionVariable*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionContext,FindVariable,1,(ahspName))
	IDLC_RET_FROM_INTF(iExpressionVariable,_Ret)
IDLC_METH_END(ni,iExpressionContext,FindVariable,1)

/** ni -> iExpressionContext::CreateExpression/1 **/
IDLC_METH_BEGIN(ni,iExpressionContext,CreateExpression,1)
	IDLC_DECL_VAR(achar*,aaszExpr)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszExpr)
	IDLC_DECL_RETVAR(iExpression*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionContext,CreateExpression,1,(aaszExpr))
	IDLC_RET_FROM_INTF(iExpression,_Ret)
IDLC_METH_END(ni,iExpressionContext,CreateExpression,1)

/** ni -> iExpressionContext::Eval/1 **/
IDLC_METH_BEGIN(ni,iExpressionContext,Eval,1)
	IDLC_DECL_VAR(achar*,aaszExpr)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszExpr)
	IDLC_DECL_RETVAR(Ptr<iExpressionVariable>,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionContext,Eval,1,(aaszExpr))
	IDLC_RET_FROM_INTF(,_Ret)
IDLC_METH_END(ni,iExpressionContext,Eval,1)

/** ni -> iExpressionContext::GetUnknownSymbols/2 **/
IDLC_METH_BEGIN(ni,iExpressionContext,GetUnknownSymbols,2)
	IDLC_DECL_VAR(achar*,aaszExpr)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszExpr)
	IDLC_DECL_VAR(tStringCVec*,apList)
	IDLC_BUF_TO_INTF(tStringCVec,apList)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionContext,GetUnknownSymbols,2,(aaszExpr,apList))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iExpressionContext,GetUnknownSymbols,2)

/** ni -> iExpressionContext::SetGlobalEnumSearch/1 **/
IDLC_METH_BEGIN(ni,iExpressionContext,SetGlobalEnumSearch,1)
	IDLC_DECL_VAR(tBool,abEnabled)
	IDLC_BUF_TO_BASE(ni::eType_I8,abEnabled)
	IDLC_METH_CALL_VOID(ni,iExpressionContext,SetGlobalEnumSearch,1,(abEnabled))
IDLC_METH_END(ni,iExpressionContext,SetGlobalEnumSearch,1)

/** ni -> iExpressionContext::GetGlobalEnumSearch/0 **/
IDLC_METH_BEGIN(ni,iExpressionContext,GetGlobalEnumSearch,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionContext,GetGlobalEnumSearch,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iExpressionContext,GetGlobalEnumSearch,0)

/** ni -> iExpressionContext::AddEnumDef/1 **/
IDLC_METH_BEGIN(ni,iExpressionContext,AddEnumDef,1)
	IDLC_DECL_VAR(sEnumDef*,apEnumDef)
	IDLC_BUF_TO_PTR(ni::eTypeFlags_Constant|ni::eType_Ptr|ni::eTypeFlags_Pointer,apEnumDef)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionContext,AddEnumDef,1,(apEnumDef))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iExpressionContext,AddEnumDef,1)

/** ni -> iExpressionContext::GetEnumDef/1 **/
IDLC_METH_INVALID_BEGIN(ni,iExpressionContext,GetEnumDef,1)
IDLC_METH_INVALID_END(ni,iExpressionContext,GetEnumDef,1)

/** ni -> iExpressionContext::SetDefaultEnumDef/1 **/
IDLC_METH_BEGIN(ni,iExpressionContext,SetDefaultEnumDef,1)
	IDLC_DECL_VAR(sEnumDef*,apEnumDef)
	IDLC_BUF_TO_PTR(ni::eTypeFlags_Constant|ni::eType_Ptr|ni::eTypeFlags_Pointer,apEnumDef)
	IDLC_METH_CALL_VOID(ni,iExpressionContext,SetDefaultEnumDef,1,(apEnumDef))
IDLC_METH_END(ni,iExpressionContext,SetDefaultEnumDef,1)

/** ni -> iExpressionContext::GetDefaultEnumDef/0 **/
IDLC_METH_INVALID_BEGIN(ni,iExpressionContext,GetDefaultEnumDef,0)
IDLC_METH_INVALID_END(ni,iExpressionContext,GetDefaultEnumDef,0)

/** ni -> iExpressionContext::GetEnumValueString/1 **/
IDLC_METH_BEGIN(ni,iExpressionContext,GetEnumValueString,1)
	IDLC_DECL_VAR(tU32,anValue)
	IDLC_BUF_TO_BASE(ni::eType_U32,anValue)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionContext,GetEnumValueString,1,(anValue))
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iExpressionContext,GetEnumValueString,1)

/** ni -> iExpressionContext::GetEnumFlagsString/1 **/
IDLC_METH_BEGIN(ni,iExpressionContext,GetEnumFlagsString,1)
	IDLC_DECL_VAR(tU32,anValue)
	IDLC_BUF_TO_BASE(ni::eType_U32,anValue)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionContext,GetEnumFlagsString,1,(anValue))
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iExpressionContext,GetEnumFlagsString,1)

/** ni -> iExpressionContext::RegisterURLResolver/2 **/
IDLC_METH_BEGIN(ni,iExpressionContext,RegisterURLResolver,2)
	IDLC_DECL_VAR(achar*,aaszProtocol)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszProtocol)
	IDLC_DECL_VAR(iExpressionURLResolver*,apResolver)
	IDLC_BUF_TO_INTF(iExpressionURLResolver,apResolver)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionContext,RegisterURLResolver,2,(aaszProtocol,apResolver))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iExpressionContext,RegisterURLResolver,2)

/** ni -> iExpressionContext::UnregisterURLResolver/1 **/
IDLC_METH_BEGIN(ni,iExpressionContext,UnregisterURLResolver,1)
	IDLC_DECL_VAR(achar*,aaszProtocol)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszProtocol)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionContext,UnregisterURLResolver,1,(aaszProtocol))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iExpressionContext,UnregisterURLResolver,1)

/** ni -> iExpressionContext::FindURLResolver/1 **/
IDLC_METH_BEGIN(ni,iExpressionContext,FindURLResolver,1)
	IDLC_DECL_VAR(achar*,aaszProtocol)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszProtocol)
	IDLC_DECL_RETVAR(iExpressionURLResolver*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iExpressionContext,FindURLResolver,1,(aaszProtocol))
	IDLC_RET_FROM_INTF(iExpressionURLResolver,_Ret)
IDLC_METH_END(ni,iExpressionContext,FindURLResolver,1)

IDLC_END_INTF(ni,iExpressionContext)

IDLC_END_NAMESPACE()
// EOF //
