#include "../API/niTestInterop/ITestInterop.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iTestInteropObject **/
IDLC_BEGIN_INTF(ni,iTestInteropObject)
/** ni -> iTestInteropObject::SetNumItems/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetNumItems,1)
	IDLC_DECL_VAR(tSize,aNumItems)
	IDLC_BUF_TO_BASE(ni::eType_Size,aNumItems)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetNumItems,1,(aNumItems))
IDLC_METH_END(ni,iTestInteropObject,SetNumItems,1)

/** ni -> iTestInteropObject::GetNumItems/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetNumItems,0)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetNumItems,0,())
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetNumItems,0)

/** ni -> iTestInteropObject::SetTestInterop/1 **/
IDLC_METH_INVALID_BEGIN(ni,iTestInteropObject,SetTestInterop,1)
IDLC_METH_INVALID_END(ni,iTestInteropObject,SetTestInterop,1)

/** ni -> iTestInteropObject::GetTestInterop/0 **/
IDLC_METH_INVALID_BEGIN(ni,iTestInteropObject,GetTestInterop,0)
IDLC_METH_INVALID_END(ni,iTestInteropObject,GetTestInterop,0)

/** ni -> iTestInteropObject::SetTestInteropConst/1 **/
IDLC_METH_INVALID_BEGIN(ni,iTestInteropObject,SetTestInteropConst,1)
IDLC_METH_INVALID_END(ni,iTestInteropObject,SetTestInteropConst,1)

/** ni -> iTestInteropObject::GetTestInteropConst/0 **/
IDLC_METH_INVALID_BEGIN(ni,iTestInteropObject,GetTestInteropConst,0)
IDLC_METH_INVALID_END(ni,iTestInteropObject,GetTestInteropConst,0)

/** ni -> iTestInteropObject::SetTestInteropRef/1 **/
IDLC_METH_INVALID_BEGIN(ni,iTestInteropObject,SetTestInteropRef,1)
IDLC_METH_INVALID_END(ni,iTestInteropObject,SetTestInteropRef,1)

/** ni -> iTestInteropObject::GetTestInteropRef/0 **/
IDLC_METH_INVALID_BEGIN(ni,iTestInteropObject,GetTestInteropRef,0)
IDLC_METH_INVALID_END(ni,iTestInteropObject,GetTestInteropRef,0)

/** ni -> iTestInteropObject::SetTestInteropConstRef/1 **/
IDLC_METH_INVALID_BEGIN(ni,iTestInteropObject,SetTestInteropConstRef,1)
IDLC_METH_INVALID_END(ni,iTestInteropObject,SetTestInteropConstRef,1)

/** ni -> iTestInteropObject::GetTestInteropConstRef/0 **/
IDLC_METH_INVALID_BEGIN(ni,iTestInteropObject,GetTestInteropConstRef,0)
IDLC_METH_INVALID_END(ni,iTestInteropObject,GetTestInteropConstRef,0)

/** ni -> iTestInteropObject::SetEnum/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetEnum,1)
	IDLC_DECL_VAR(eTestInterop,aV)
	IDLC_BUF_TO_ENUM(eTestInterop,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetEnum,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetEnum,1)

/** ni -> iTestInteropObject::GetEnum/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetEnum,0)
	IDLC_DECL_RETVAR(eTestInterop,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetEnum,0,())
	IDLC_RET_FROM_ENUM(eTestInterop,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetEnum,0)

/** ni -> iTestInteropObject::SetFlags/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetFlags,1)
	IDLC_DECL_VAR(tTestInteropFlags,aV)
	IDLC_BUF_TO_ENUM(tTestInteropFlags,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetFlags,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetFlags,1)

/** ni -> iTestInteropObject::GetFlags/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetFlags,0)
	IDLC_DECL_RETVAR(tTestInteropFlags,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetFlags,0,())
	IDLC_RET_FROM_ENUM(tTestInteropFlags,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetFlags,0)

/** ni -> iTestInteropObject::SetIntPtr/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetIntPtr,1)
	IDLC_DECL_VAR(tIntPtr,aV)
	IDLC_BUF_TO_BASE(ni::eType_IntPtr,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetIntPtr,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetIntPtr,1)

/** ni -> iTestInteropObject::GetIntPtr/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetIntPtr,0)
	IDLC_DECL_RETVAR(tIntPtr,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetIntPtr,0,())
	IDLC_RET_FROM_BASE(ni::eType_IntPtr,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetIntPtr,0)

/** ni -> iTestInteropObject::SetUIntPtr/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetUIntPtr,1)
	IDLC_DECL_VAR(tIntPtr,aV)
	IDLC_BUF_TO_BASE(ni::eType_IntPtr,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetUIntPtr,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetUIntPtr,1)

/** ni -> iTestInteropObject::GetUIntPtr/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetUIntPtr,0)
	IDLC_DECL_RETVAR(tIntPtr,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetUIntPtr,0,())
	IDLC_RET_FROM_BASE(ni::eType_IntPtr,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetUIntPtr,0)

/** ni -> iTestInteropObject::SetI8/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetI8,1)
	IDLC_DECL_VAR(tI8,aV)
	IDLC_BUF_TO_BASE(ni::eType_I8,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetI8,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetI8,1)

/** ni -> iTestInteropObject::GetI8/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetI8,0)
	IDLC_DECL_RETVAR(tI8,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetI8,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetI8,0)

/** ni -> iTestInteropObject::SetU8/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetU8,1)
	IDLC_DECL_VAR(tU8,aV)
	IDLC_BUF_TO_BASE(ni::eType_U8,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetU8,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetU8,1)

/** ni -> iTestInteropObject::GetU8/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetU8,0)
	IDLC_DECL_RETVAR(tU8,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetU8,0,())
	IDLC_RET_FROM_BASE(ni::eType_U8,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetU8,0)

/** ni -> iTestInteropObject::SetI16/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetI16,1)
	IDLC_DECL_VAR(tI16,aV)
	IDLC_BUF_TO_BASE(ni::eType_I16,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetI16,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetI16,1)

/** ni -> iTestInteropObject::GetI16/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetI16,0)
	IDLC_DECL_RETVAR(tI16,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetI16,0,())
	IDLC_RET_FROM_BASE(ni::eType_I16,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetI16,0)

/** ni -> iTestInteropObject::SetU16/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetU16,1)
	IDLC_DECL_VAR(tU16,aV)
	IDLC_BUF_TO_BASE(ni::eType_U16,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetU16,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetU16,1)

/** ni -> iTestInteropObject::GetU16/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetU16,0)
	IDLC_DECL_RETVAR(tU16,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetU16,0,())
	IDLC_RET_FROM_BASE(ni::eType_U16,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetU16,0)

/** ni -> iTestInteropObject::SetI32/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetI32,1)
	IDLC_DECL_VAR(tI32,aV)
	IDLC_BUF_TO_BASE(ni::eType_I32,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetI32,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetI32,1)

/** ni -> iTestInteropObject::GetI32/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetI32,0)
	IDLC_DECL_RETVAR(tI32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetI32,0,())
	IDLC_RET_FROM_BASE(ni::eType_I32,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetI32,0)

/** ni -> iTestInteropObject::SetU32/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetU32,1)
	IDLC_DECL_VAR(tU32,aV)
	IDLC_BUF_TO_BASE(ni::eType_U32,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetU32,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetU32,1)

/** ni -> iTestInteropObject::GetU32/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetU32,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetU32,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetU32,0)

/** ni -> iTestInteropObject::SetI64/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetI64,1)
	IDLC_DECL_VAR(tI64,aV)
	IDLC_BUF_TO_BASE(ni::eType_I64,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetI64,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetI64,1)

/** ni -> iTestInteropObject::GetI64/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetI64,0)
	IDLC_DECL_RETVAR(tI64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetI64,0,())
	IDLC_RET_FROM_BASE(ni::eType_I64,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetI64,0)

/** ni -> iTestInteropObject::SetU64/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetU64,1)
	IDLC_DECL_VAR(tU64,aV)
	IDLC_BUF_TO_BASE(ni::eType_U64,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetU64,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetU64,1)

/** ni -> iTestInteropObject::GetU64/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetU64,0)
	IDLC_DECL_RETVAR(tU64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetU64,0,())
	IDLC_RET_FROM_BASE(ni::eType_U64,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetU64,0)

/** ni -> iTestInteropObject::SetF32/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetF32,1)
	IDLC_DECL_VAR(tF32,aV)
	IDLC_BUF_TO_BASE(ni::eType_F32,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetF32,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetF32,1)

/** ni -> iTestInteropObject::GetF32/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetF32,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetF32,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetF32,0)

/** ni -> iTestInteropObject::SetF64/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetF64,1)
	IDLC_DECL_VAR(tF64,aV)
	IDLC_BUF_TO_BASE(ni::eType_F64,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetF64,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetF64,1)

/** ni -> iTestInteropObject::GetF64/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetF64,0)
	IDLC_DECL_RETVAR(tF64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetF64,0,())
	IDLC_RET_FROM_BASE(ni::eType_F64,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetF64,0)

/** ni -> iTestInteropObject::SetVar/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetVar,1)
	IDLC_DECL_VAR(Var,aV)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetVar,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetVar,1)

/** ni -> iTestInteropObject::GetVar/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetVar,0)
	IDLC_DECL_RETVAR(Var,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetVar,0,())
	IDLC_RET_FROM_BASE(ni::eType_Variant,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetVar,0)

/** ni -> iTestInteropObject::SetVec2l/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetVec2l,1)
	IDLC_DECL_VAR(sVec2i,aV)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2i|ni::eTypeFlags_Pointer,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetVec2l,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetVec2l,1)

/** ni -> iTestInteropObject::GetVec2l/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetVec2l,0)
	IDLC_DECL_RETVAR(sVec2i,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetVec2l,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2i,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetVec2l,0)

/** ni -> iTestInteropObject::SetVec2f/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetVec2f,1)
	IDLC_DECL_VAR(sVec2f,aV)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetVec2f,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetVec2f,1)

/** ni -> iTestInteropObject::GetVec2f/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetVec2f,0)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetVec2f,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetVec2f,0)

/** ni -> iTestInteropObject::SetVec3l/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetVec3l,1)
	IDLC_DECL_VAR(sVec3i,aV)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3i|ni::eTypeFlags_Pointer,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetVec3l,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetVec3l,1)

/** ni -> iTestInteropObject::GetVec3l/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetVec3l,0)
	IDLC_DECL_RETVAR(sVec3i,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetVec3l,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec3i,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetVec3l,0)

/** ni -> iTestInteropObject::SetVec3f/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetVec3f,1)
	IDLC_DECL_VAR(sVec3f,aV)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetVec3f,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetVec3f,1)

/** ni -> iTestInteropObject::GetVec3f/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetVec3f,0)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetVec3f,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetVec3f,0)

/** ni -> iTestInteropObject::SetVec4l/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetVec4l,1)
	IDLC_DECL_VAR(sVec4i,aV)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4i|ni::eTypeFlags_Pointer,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetVec4l,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetVec4l,1)

/** ni -> iTestInteropObject::GetVec4l/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetVec4l,0)
	IDLC_DECL_RETVAR(sVec4i,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetVec4l,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4i,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetVec4l,0)

/** ni -> iTestInteropObject::SetVec4f/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetVec4f,1)
	IDLC_DECL_VAR(sVec4f,aV)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetVec4f,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetVec4f,1)

/** ni -> iTestInteropObject::GetVec4f/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetVec4f,0)
	IDLC_DECL_RETVAR(sVec4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetVec4f,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetVec4f,0)

/** ni -> iTestInteropObject::SetMatrixf/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetMatrixf,1)
	IDLC_DECL_VAR(sMatrixf,aV)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Matrixf|ni::eTypeFlags_Pointer,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetMatrixf,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetMatrixf,1)

/** ni -> iTestInteropObject::GetMatrixf/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetMatrixf,0)
	IDLC_DECL_RETVAR(sMatrixf,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetMatrixf,0,())
	IDLC_RET_FROM_BASE(ni::eType_Matrixf,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetMatrixf,0)

/** ni -> iTestInteropObject::SetMatrixfValues/16 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetMatrixfValues,16)
	IDLC_DECL_VAR(ni::tF32,a00)
	IDLC_BUF_TO_BASE(ni::eType_F32,a00)
	IDLC_DECL_VAR(ni::tF32,a01)
	IDLC_BUF_TO_BASE(ni::eType_F32,a01)
	IDLC_DECL_VAR(ni::tF32,a02)
	IDLC_BUF_TO_BASE(ni::eType_F32,a02)
	IDLC_DECL_VAR(ni::tF32,a03)
	IDLC_BUF_TO_BASE(ni::eType_F32,a03)
	IDLC_DECL_VAR(ni::tF32,a10)
	IDLC_BUF_TO_BASE(ni::eType_F32,a10)
	IDLC_DECL_VAR(ni::tF32,a11)
	IDLC_BUF_TO_BASE(ni::eType_F32,a11)
	IDLC_DECL_VAR(ni::tF32,a12)
	IDLC_BUF_TO_BASE(ni::eType_F32,a12)
	IDLC_DECL_VAR(ni::tF32,a13)
	IDLC_BUF_TO_BASE(ni::eType_F32,a13)
	IDLC_DECL_VAR(ni::tF32,a20)
	IDLC_BUF_TO_BASE(ni::eType_F32,a20)
	IDLC_DECL_VAR(ni::tF32,a21)
	IDLC_BUF_TO_BASE(ni::eType_F32,a21)
	IDLC_DECL_VAR(ni::tF32,a22)
	IDLC_BUF_TO_BASE(ni::eType_F32,a22)
	IDLC_DECL_VAR(ni::tF32,a23)
	IDLC_BUF_TO_BASE(ni::eType_F32,a23)
	IDLC_DECL_VAR(ni::tF32,a30)
	IDLC_BUF_TO_BASE(ni::eType_F32,a30)
	IDLC_DECL_VAR(ni::tF32,a31)
	IDLC_BUF_TO_BASE(ni::eType_F32,a31)
	IDLC_DECL_VAR(ni::tF32,a32)
	IDLC_BUF_TO_BASE(ni::eType_F32,a32)
	IDLC_DECL_VAR(ni::tF32,a33)
	IDLC_BUF_TO_BASE(ni::eType_F32,a33)
	IDLC_DECL_RETVAR(ni::tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,SetMatrixfValues,16,(a00,a01,a02,a03,a10,a11,a12,a13,a20,a21,a22,a23,a30,a31,a32,a33))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iTestInteropObject,SetMatrixfValues,16)

/** ni -> iTestInteropObject::SetACharStrZ/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetACharStrZ,1)
	IDLC_DECL_VAR(achar*,aV)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetACharStrZ,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetACharStrZ,1)

/** ni -> iTestInteropObject::GetACharStrZ/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetACharStrZ,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetACharStrZ,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetACharStrZ,0)

/** ni -> iTestInteropObject::SetString/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetString,1)
	IDLC_DECL_VAR(cString,aStr)
	IDLC_BUF_TO_BASE(ni::eType_String,aStr)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetString,1,(aStr))
IDLC_METH_END(ni,iTestInteropObject,SetString,1)

/** ni -> iTestInteropObject::GetString/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetString,0)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetString,0,())
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetString,0)

/** ni -> iTestInteropObject::SetStringCRef/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetStringCRef,1)
	IDLC_DECL_VAR(cString,aStr)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_String|ni::eTypeFlags_Pointer,aStr)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetStringCRef,1,(aStr))
IDLC_METH_END(ni,iTestInteropObject,SetStringCRef,1)

/** ni -> iTestInteropObject::GetStringCRef/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetStringCRef,0)
	IDLC_DECL_RETREFVAR(cString&,_Ret,cString)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetStringCRef,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_String|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetStringCRef,0)

/** ni -> iTestInteropObject::SetHString/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetHString,1)
	IDLC_DECL_VAR(iHString*,ahspStr)
	IDLC_BUF_TO_INTF(iHString,ahspStr)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetHString,1,(ahspStr))
IDLC_METH_END(ni,iTestInteropObject,SetHString,1)

/** ni -> iTestInteropObject::GetHString/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetHString,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetHString,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetHString,0)

/** ni -> iTestInteropObject::SetIndexedU32/2 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetIndexedU32,2)
	IDLC_DECL_VAR(tSize,aIndex)
	IDLC_BUF_TO_BASE(ni::eType_Size,aIndex)
	IDLC_DECL_VAR(tU32,aV)
	IDLC_BUF_TO_BASE(ni::eType_U32,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetIndexedU32,2,(aIndex,aV))
IDLC_METH_END(ni,iTestInteropObject,SetIndexedU32,2)

/** ni -> iTestInteropObject::GetIndexedU32/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetIndexedU32,1)
	IDLC_DECL_VAR(tSize,aIndex)
	IDLC_BUF_TO_BASE(ni::eType_Size,aIndex)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetIndexedU32,1,(aIndex))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetIndexedU32,1)

/** ni -> iTestInteropObject::SetMappedU32/2 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetMappedU32,2)
	IDLC_DECL_VAR(achar*,aaszKey)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszKey)
	IDLC_DECL_VAR(tU32,aV)
	IDLC_BUF_TO_BASE(ni::eType_U32,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetMappedU32,2,(aaszKey,aV))
IDLC_METH_END(ni,iTestInteropObject,SetMappedU32,2)

/** ni -> iTestInteropObject::GetMappedU32/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetMappedU32,1)
	IDLC_DECL_VAR(achar*,aaszKey)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszKey)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetMappedU32,1,(aaszKey))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetMappedU32,1)

/** ni -> iTestInteropObject::SetPtr/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetPtr,1)
	IDLC_DECL_VAR(tPtr,aV)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetPtr,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetPtr,1)

/** ni -> iTestInteropObject::GetPtr/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetPtr,0)
	IDLC_DECL_RETVAR(tPtr,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetPtr,0,())
	IDLC_RET_FROM_PTR(ni::eType_Ptr,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetPtr,0)

/** ni -> iTestInteropObject::WriteRawBuffer/2 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,WriteRawBuffer,2)
	IDLC_DECL_VAR(tPtr,aData)
	IDLC_BUF_TO_PTR(ni::eTypeFlags_Constant|ni::eType_Ptr,aData)
	IDLC_DECL_VAR(tSize,anSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,anSize)
	IDLC_DECL_RETVAR(ni::tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,WriteRawBuffer,2,(aData,anSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iTestInteropObject,WriteRawBuffer,2)

/** ni -> iTestInteropObject::ReadRawBuffer/2 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,ReadRawBuffer,2)
	IDLC_DECL_VAR(tPtr,aData)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,aData)
	IDLC_DECL_VAR(tSize,anSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,anSize)
	IDLC_DECL_RETVAR(ni::tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,ReadRawBuffer,2,(aData,anSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iTestInteropObject,ReadRawBuffer,2)

/** ni -> iTestInteropObject::WriteRawBufferVoid/2 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,WriteRawBufferVoid,2)
	IDLC_DECL_VAR(void*,apIn)
	IDLC_BUF_TO_PTR(ni::eTypeFlags_Constant|ni::eType_Ptr,apIn)
	IDLC_DECL_VAR(tSize,anSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,anSize)
	IDLC_DECL_RETVAR(ni::tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,WriteRawBufferVoid,2,(apIn,anSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iTestInteropObject,WriteRawBufferVoid,2)

/** ni -> iTestInteropObject::ReadRawBufferVoid/2 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,ReadRawBufferVoid,2)
	IDLC_DECL_VAR(void*,apOut)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,apOut)
	IDLC_DECL_VAR(tSize,anSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,anSize)
	IDLC_DECL_RETVAR(ni::tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,ReadRawBufferVoid,2,(apOut,anSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iTestInteropObject,ReadRawBufferVoid,2)

/** ni -> iTestInteropObject::WriteRawBufferU32/2 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,WriteRawBufferU32,2)
	IDLC_DECL_VAR(tU32*,apIn)
	IDLC_BUF_TO_NULL(ni::eTypeFlags_Constant|ni::eType_U32|ni::eTypeFlags_Pointer,apIn)
	IDLC_DECL_VAR(tSize,anSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,anSize)
	IDLC_DECL_RETVAR(ni::tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,WriteRawBufferU32,2,(apIn,anSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iTestInteropObject,WriteRawBufferU32,2)

/** ni -> iTestInteropObject::ReadRawBufferU32/2 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,ReadRawBufferU32,2)
	IDLC_DECL_VAR(tU32*,apOut)
	IDLC_BUF_TO_NULL(ni::eType_U32|ni::eTypeFlags_Pointer,apOut)
	IDLC_DECL_VAR(tSize,anSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,anSize)
	IDLC_DECL_RETVAR(ni::tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,ReadRawBufferU32,2,(apOut,anSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iTestInteropObject,ReadRawBufferU32,2)

/** ni -> iTestInteropObject::SetIndexedIUnknown/2 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetIndexedIUnknown,2)
	IDLC_DECL_VAR(tSize,aIndex)
	IDLC_BUF_TO_BASE(ni::eType_Size,aIndex)
	IDLC_DECL_VAR(ni::iUnknown*,aV)
	IDLC_BUF_TO_INTF(ni::iUnknown,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetIndexedIUnknown,2,(aIndex,aV))
IDLC_METH_END(ni,iTestInteropObject,SetIndexedIUnknown,2)

/** ni -> iTestInteropObject::GetIndexedIUnknown/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetIndexedIUnknown,1)
	IDLC_DECL_VAR(tSize,aIndex)
	IDLC_BUF_TO_BASE(ni::eType_Size,aIndex)
	IDLC_DECL_RETVAR(ni::iUnknown*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetIndexedIUnknown,1,(aIndex))
	IDLC_RET_FROM_INTF(ni::iUnknown,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetIndexedIUnknown,1)

/** ni -> iTestInteropObject::SetMappedIUnknown/2 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetMappedIUnknown,2)
	IDLC_DECL_VAR(achar*,aaszKey)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszKey)
	IDLC_DECL_VAR(ni::iUnknown*,aV)
	IDLC_BUF_TO_INTF(ni::iUnknown,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetMappedIUnknown,2,(aaszKey,aV))
IDLC_METH_END(ni,iTestInteropObject,SetMappedIUnknown,2)

/** ni -> iTestInteropObject::GetMappedIUnknown/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetMappedIUnknown,1)
	IDLC_DECL_VAR(achar*,aaszKey)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszKey)
	IDLC_DECL_RETVAR(ni::iUnknown*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetMappedIUnknown,1,(aaszKey))
	IDLC_RET_FROM_INTF(ni::iUnknown,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetMappedIUnknown,1)

/** ni -> iTestInteropObject::SetTestInteropObject/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetTestInteropObject,1)
	IDLC_DECL_VAR(iTestInteropObject*,aV)
	IDLC_BUF_TO_INTF(iTestInteropObject,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetTestInteropObject,1,(aV))
IDLC_METH_END(ni,iTestInteropObject,SetTestInteropObject,1)

/** ni -> iTestInteropObject::GetTestInteropObject/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetTestInteropObject,0)
	IDLC_DECL_RETVAR(iTestInteropObject*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetTestInteropObject,0,())
	IDLC_RET_FROM_INTF(iTestInteropObject,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetTestInteropObject,0)

/** ni -> iTestInteropObject::SetIndexedTestInteropObject/2 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetIndexedTestInteropObject,2)
	IDLC_DECL_VAR(tSize,aIndex)
	IDLC_BUF_TO_BASE(ni::eType_Size,aIndex)
	IDLC_DECL_VAR(iTestInteropObject*,aV)
	IDLC_BUF_TO_INTF(iTestInteropObject,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetIndexedTestInteropObject,2,(aIndex,aV))
IDLC_METH_END(ni,iTestInteropObject,SetIndexedTestInteropObject,2)

/** ni -> iTestInteropObject::GetIndexedTestInteropObject/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetIndexedTestInteropObject,1)
	IDLC_DECL_VAR(tSize,aIndex)
	IDLC_BUF_TO_BASE(ni::eType_Size,aIndex)
	IDLC_DECL_RETVAR(iTestInteropObject*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetIndexedTestInteropObject,1,(aIndex))
	IDLC_RET_FROM_INTF(iTestInteropObject,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetIndexedTestInteropObject,1)

/** ni -> iTestInteropObject::SetMappedTestInteropObject/2 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetMappedTestInteropObject,2)
	IDLC_DECL_VAR(achar*,aaszKey)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszKey)
	IDLC_DECL_VAR(iTestInteropObject*,aV)
	IDLC_BUF_TO_INTF(iTestInteropObject,aV)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetMappedTestInteropObject,2,(aaszKey,aV))
IDLC_METH_END(ni,iTestInteropObject,SetMappedTestInteropObject,2)

/** ni -> iTestInteropObject::GetMappedTestInteropObject/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetMappedTestInteropObject,1)
	IDLC_DECL_VAR(achar*,aaszKey)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszKey)
	IDLC_DECL_RETVAR(iTestInteropObject*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetMappedTestInteropObject,1,(aaszKey))
	IDLC_RET_FROM_INTF(iTestInteropObject,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetMappedTestInteropObject,1)

/** ni -> iTestInteropObject::SetU32Vec/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetU32Vec,1)
	IDLC_DECL_VAR(tU32CVec*,apVec)
	IDLC_BUF_TO_INTF(tU32CVec,apVec)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetU32Vec,1,(apVec))
IDLC_METH_END(ni,iTestInteropObject,SetU32Vec,1)

/** ni -> iTestInteropObject::GetU32Vec/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetU32Vec,0)
	IDLC_DECL_RETVAR(tU32CVec*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetU32Vec,0,())
	IDLC_RET_FROM_INTF(tU32CVec,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetU32Vec,0)

/** ni -> iTestInteropObject::SetStringMap/1 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,SetStringMap,1)
	IDLC_DECL_VAR(tStringCMap*,apMap)
	IDLC_BUF_TO_INTF(tStringCMap,apMap)
	IDLC_METH_CALL_VOID(ni,iTestInteropObject,SetStringMap,1,(apMap))
IDLC_METH_END(ni,iTestInteropObject,SetStringMap,1)

/** ni -> iTestInteropObject::GetStringMap/0 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,GetStringMap,0)
	IDLC_DECL_RETVAR(tStringCMap*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,GetStringMap,0,())
	IDLC_RET_FROM_INTF(tStringCMap,_Ret)
IDLC_METH_END(ni,iTestInteropObject,GetStringMap,0)

/** ni -> iTestInteropObject::Func32/32 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,Func32,32)
	IDLC_DECL_VAR(ni::tU32,a00)
	IDLC_BUF_TO_BASE(ni::eType_U32,a00)
	IDLC_DECL_VAR(ni::tU32,a01)
	IDLC_BUF_TO_BASE(ni::eType_U32,a01)
	IDLC_DECL_VAR(ni::tU32,a02)
	IDLC_BUF_TO_BASE(ni::eType_U32,a02)
	IDLC_DECL_VAR(ni::tU32,a03)
	IDLC_BUF_TO_BASE(ni::eType_U32,a03)
	IDLC_DECL_VAR(ni::tU32,a10)
	IDLC_BUF_TO_BASE(ni::eType_U32,a10)
	IDLC_DECL_VAR(ni::tU32,a11)
	IDLC_BUF_TO_BASE(ni::eType_U32,a11)
	IDLC_DECL_VAR(ni::tU32,a12)
	IDLC_BUF_TO_BASE(ni::eType_U32,a12)
	IDLC_DECL_VAR(ni::tU32,a13)
	IDLC_BUF_TO_BASE(ni::eType_U32,a13)
	IDLC_DECL_VAR(ni::tU32,a20)
	IDLC_BUF_TO_BASE(ni::eType_U32,a20)
	IDLC_DECL_VAR(ni::tU32,a21)
	IDLC_BUF_TO_BASE(ni::eType_U32,a21)
	IDLC_DECL_VAR(ni::tU32,a22)
	IDLC_BUF_TO_BASE(ni::eType_U32,a22)
	IDLC_DECL_VAR(ni::tU32,a23)
	IDLC_BUF_TO_BASE(ni::eType_U32,a23)
	IDLC_DECL_VAR(ni::tU32,a30)
	IDLC_BUF_TO_BASE(ni::eType_U32,a30)
	IDLC_DECL_VAR(ni::tU32,a31)
	IDLC_BUF_TO_BASE(ni::eType_U32,a31)
	IDLC_DECL_VAR(ni::tU32,a32)
	IDLC_BUF_TO_BASE(ni::eType_U32,a32)
	IDLC_DECL_VAR(ni::tU32,a33)
	IDLC_BUF_TO_BASE(ni::eType_U32,a33)
	IDLC_DECL_VAR(ni::tU32,a40)
	IDLC_BUF_TO_BASE(ni::eType_U32,a40)
	IDLC_DECL_VAR(ni::tU32,a41)
	IDLC_BUF_TO_BASE(ni::eType_U32,a41)
	IDLC_DECL_VAR(ni::tU32,a42)
	IDLC_BUF_TO_BASE(ni::eType_U32,a42)
	IDLC_DECL_VAR(ni::tU32,a43)
	IDLC_BUF_TO_BASE(ni::eType_U32,a43)
	IDLC_DECL_VAR(ni::tU32,a50)
	IDLC_BUF_TO_BASE(ni::eType_U32,a50)
	IDLC_DECL_VAR(ni::tU32,a51)
	IDLC_BUF_TO_BASE(ni::eType_U32,a51)
	IDLC_DECL_VAR(ni::tU32,a52)
	IDLC_BUF_TO_BASE(ni::eType_U32,a52)
	IDLC_DECL_VAR(ni::tU32,a53)
	IDLC_BUF_TO_BASE(ni::eType_U32,a53)
	IDLC_DECL_VAR(ni::tU32,a60)
	IDLC_BUF_TO_BASE(ni::eType_U32,a60)
	IDLC_DECL_VAR(ni::tU32,a61)
	IDLC_BUF_TO_BASE(ni::eType_U32,a61)
	IDLC_DECL_VAR(ni::tU32,a62)
	IDLC_BUF_TO_BASE(ni::eType_U32,a62)
	IDLC_DECL_VAR(ni::tU32,a63)
	IDLC_BUF_TO_BASE(ni::eType_U32,a63)
	IDLC_DECL_VAR(ni::tU32,a70)
	IDLC_BUF_TO_BASE(ni::eType_U32,a70)
	IDLC_DECL_VAR(ni::tU32,a71)
	IDLC_BUF_TO_BASE(ni::eType_U32,a71)
	IDLC_DECL_VAR(ni::tU32,a72)
	IDLC_BUF_TO_BASE(ni::eType_U32,a72)
	IDLC_DECL_VAR(ni::tU32,a73)
	IDLC_BUF_TO_BASE(ni::eType_U32,a73)
	IDLC_DECL_RETVAR(ni::tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,Func32,32,(a00,a01,a02,a03,a10,a11,a12,a13,a20,a21,a22,a23,a30,a31,a32,a33,a40,a41,a42,a43,a50,a51,a52,a53,a60,a61,a62,a63,a70,a71,a72,a73))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iTestInteropObject,Func32,32)

/** ni -> iTestInteropObject::Func33/33 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,Func33,33)
	IDLC_DECL_VAR(ni::tU32,a00)
	IDLC_BUF_TO_BASE(ni::eType_U32,a00)
	IDLC_DECL_VAR(ni::tU32,a01)
	IDLC_BUF_TO_BASE(ni::eType_U32,a01)
	IDLC_DECL_VAR(ni::tU32,a02)
	IDLC_BUF_TO_BASE(ni::eType_U32,a02)
	IDLC_DECL_VAR(ni::tU32,a03)
	IDLC_BUF_TO_BASE(ni::eType_U32,a03)
	IDLC_DECL_VAR(ni::tU32,a10)
	IDLC_BUF_TO_BASE(ni::eType_U32,a10)
	IDLC_DECL_VAR(ni::tU32,a11)
	IDLC_BUF_TO_BASE(ni::eType_U32,a11)
	IDLC_DECL_VAR(ni::tU32,a12)
	IDLC_BUF_TO_BASE(ni::eType_U32,a12)
	IDLC_DECL_VAR(ni::tU32,a13)
	IDLC_BUF_TO_BASE(ni::eType_U32,a13)
	IDLC_DECL_VAR(ni::tU32,a20)
	IDLC_BUF_TO_BASE(ni::eType_U32,a20)
	IDLC_DECL_VAR(ni::tU32,a21)
	IDLC_BUF_TO_BASE(ni::eType_U32,a21)
	IDLC_DECL_VAR(ni::tU32,a22)
	IDLC_BUF_TO_BASE(ni::eType_U32,a22)
	IDLC_DECL_VAR(ni::tU32,a23)
	IDLC_BUF_TO_BASE(ni::eType_U32,a23)
	IDLC_DECL_VAR(ni::tU32,a30)
	IDLC_BUF_TO_BASE(ni::eType_U32,a30)
	IDLC_DECL_VAR(ni::tU32,a31)
	IDLC_BUF_TO_BASE(ni::eType_U32,a31)
	IDLC_DECL_VAR(ni::tU32,a32)
	IDLC_BUF_TO_BASE(ni::eType_U32,a32)
	IDLC_DECL_VAR(ni::tU32,a33)
	IDLC_BUF_TO_BASE(ni::eType_U32,a33)
	IDLC_DECL_VAR(ni::tU32,a40)
	IDLC_BUF_TO_BASE(ni::eType_U32,a40)
	IDLC_DECL_VAR(ni::tU32,a41)
	IDLC_BUF_TO_BASE(ni::eType_U32,a41)
	IDLC_DECL_VAR(ni::tU32,a42)
	IDLC_BUF_TO_BASE(ni::eType_U32,a42)
	IDLC_DECL_VAR(ni::tU32,a43)
	IDLC_BUF_TO_BASE(ni::eType_U32,a43)
	IDLC_DECL_VAR(ni::tU32,a50)
	IDLC_BUF_TO_BASE(ni::eType_U32,a50)
	IDLC_DECL_VAR(ni::tU32,a51)
	IDLC_BUF_TO_BASE(ni::eType_U32,a51)
	IDLC_DECL_VAR(ni::tU32,a52)
	IDLC_BUF_TO_BASE(ni::eType_U32,a52)
	IDLC_DECL_VAR(ni::tU32,a53)
	IDLC_BUF_TO_BASE(ni::eType_U32,a53)
	IDLC_DECL_VAR(ni::tU32,a60)
	IDLC_BUF_TO_BASE(ni::eType_U32,a60)
	IDLC_DECL_VAR(ni::tU32,a61)
	IDLC_BUF_TO_BASE(ni::eType_U32,a61)
	IDLC_DECL_VAR(ni::tU32,a62)
	IDLC_BUF_TO_BASE(ni::eType_U32,a62)
	IDLC_DECL_VAR(ni::tU32,a63)
	IDLC_BUF_TO_BASE(ni::eType_U32,a63)
	IDLC_DECL_VAR(ni::tU32,a70)
	IDLC_BUF_TO_BASE(ni::eType_U32,a70)
	IDLC_DECL_VAR(ni::tU32,a71)
	IDLC_BUF_TO_BASE(ni::eType_U32,a71)
	IDLC_DECL_VAR(ni::tU32,a72)
	IDLC_BUF_TO_BASE(ni::eType_U32,a72)
	IDLC_DECL_VAR(ni::tU32,a73)
	IDLC_BUF_TO_BASE(ni::eType_U32,a73)
	IDLC_DECL_VAR(ni::tU32,firstOops)
	IDLC_BUF_TO_BASE(ni::eType_U32,firstOops)
	IDLC_DECL_RETVAR(ni::tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,Func33,33,(a00,a01,a02,a03,a10,a11,a12,a13,a20,a21,a22,a23,a30,a31,a32,a33,a40,a41,a42,a43,a50,a51,a52,a53,a60,a61,a62,a63,a70,a71,a72,a73,firstOops))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iTestInteropObject,Func33,33)

/** ni -> iTestInteropObject::Func48/40 **/
IDLC_METH_BEGIN(ni,iTestInteropObject,Func48,40)
	IDLC_DECL_VAR(ni::tU32,a00)
	IDLC_BUF_TO_BASE(ni::eType_U32,a00)
	IDLC_DECL_VAR(ni::tU32,a01)
	IDLC_BUF_TO_BASE(ni::eType_U32,a01)
	IDLC_DECL_VAR(ni::tU32,a02)
	IDLC_BUF_TO_BASE(ni::eType_U32,a02)
	IDLC_DECL_VAR(ni::tU32,a03)
	IDLC_BUF_TO_BASE(ni::eType_U32,a03)
	IDLC_DECL_VAR(ni::tU32,a10)
	IDLC_BUF_TO_BASE(ni::eType_U32,a10)
	IDLC_DECL_VAR(ni::tU32,a11)
	IDLC_BUF_TO_BASE(ni::eType_U32,a11)
	IDLC_DECL_VAR(ni::tU32,a12)
	IDLC_BUF_TO_BASE(ni::eType_U32,a12)
	IDLC_DECL_VAR(ni::tU32,a13)
	IDLC_BUF_TO_BASE(ni::eType_U32,a13)
	IDLC_DECL_VAR(ni::tU32,a20)
	IDLC_BUF_TO_BASE(ni::eType_U32,a20)
	IDLC_DECL_VAR(ni::tU32,a21)
	IDLC_BUF_TO_BASE(ni::eType_U32,a21)
	IDLC_DECL_VAR(ni::tU32,a22)
	IDLC_BUF_TO_BASE(ni::eType_U32,a22)
	IDLC_DECL_VAR(ni::tU32,a23)
	IDLC_BUF_TO_BASE(ni::eType_U32,a23)
	IDLC_DECL_VAR(ni::tU32,a30)
	IDLC_BUF_TO_BASE(ni::eType_U32,a30)
	IDLC_DECL_VAR(ni::tU32,a31)
	IDLC_BUF_TO_BASE(ni::eType_U32,a31)
	IDLC_DECL_VAR(ni::tU32,a32)
	IDLC_BUF_TO_BASE(ni::eType_U32,a32)
	IDLC_DECL_VAR(ni::tU32,a33)
	IDLC_BUF_TO_BASE(ni::eType_U32,a33)
	IDLC_DECL_VAR(ni::tU32,a40)
	IDLC_BUF_TO_BASE(ni::eType_U32,a40)
	IDLC_DECL_VAR(ni::tU32,a41)
	IDLC_BUF_TO_BASE(ni::eType_U32,a41)
	IDLC_DECL_VAR(ni::tU32,a42)
	IDLC_BUF_TO_BASE(ni::eType_U32,a42)
	IDLC_DECL_VAR(ni::tU32,a43)
	IDLC_BUF_TO_BASE(ni::eType_U32,a43)
	IDLC_DECL_VAR(ni::tU32,a50)
	IDLC_BUF_TO_BASE(ni::eType_U32,a50)
	IDLC_DECL_VAR(ni::tU32,a51)
	IDLC_BUF_TO_BASE(ni::eType_U32,a51)
	IDLC_DECL_VAR(ni::tU32,a52)
	IDLC_BUF_TO_BASE(ni::eType_U32,a52)
	IDLC_DECL_VAR(ni::tU32,a53)
	IDLC_BUF_TO_BASE(ni::eType_U32,a53)
	IDLC_DECL_VAR(ni::tU32,a60)
	IDLC_BUF_TO_BASE(ni::eType_U32,a60)
	IDLC_DECL_VAR(ni::tU32,a61)
	IDLC_BUF_TO_BASE(ni::eType_U32,a61)
	IDLC_DECL_VAR(ni::tU32,a62)
	IDLC_BUF_TO_BASE(ni::eType_U32,a62)
	IDLC_DECL_VAR(ni::tU32,a63)
	IDLC_BUF_TO_BASE(ni::eType_U32,a63)
	IDLC_DECL_VAR(ni::tU32,a70)
	IDLC_BUF_TO_BASE(ni::eType_U32,a70)
	IDLC_DECL_VAR(ni::tU32,a71)
	IDLC_BUF_TO_BASE(ni::eType_U32,a71)
	IDLC_DECL_VAR(ni::tU32,a72)
	IDLC_BUF_TO_BASE(ni::eType_U32,a72)
	IDLC_DECL_VAR(ni::tU32,a73)
	IDLC_BUF_TO_BASE(ni::eType_U32,a73)
	IDLC_DECL_VAR(ni::tU32,a80)
	IDLC_BUF_TO_BASE(ni::eType_U32,a80)
	IDLC_DECL_VAR(ni::tU32,a81)
	IDLC_BUF_TO_BASE(ni::eType_U32,a81)
	IDLC_DECL_VAR(ni::tU32,a82)
	IDLC_BUF_TO_BASE(ni::eType_U32,a82)
	IDLC_DECL_VAR(ni::tU32,a83)
	IDLC_BUF_TO_BASE(ni::eType_U32,a83)
	IDLC_DECL_VAR(ni::tU32,a90)
	IDLC_BUF_TO_BASE(ni::eType_U32,a90)
	IDLC_DECL_VAR(ni::tU32,a91)
	IDLC_BUF_TO_BASE(ni::eType_U32,a91)
	IDLC_DECL_VAR(ni::tU32,a92)
	IDLC_BUF_TO_BASE(ni::eType_U32,a92)
	IDLC_DECL_VAR(ni::tU32,a93)
	IDLC_BUF_TO_BASE(ni::eType_U32,a93)
	IDLC_DECL_RETVAR(ni::tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropObject,Func48,40,(a00,a01,a02,a03,a10,a11,a12,a13,a20,a21,a22,a23,a30,a31,a32,a33,a40,a41,a42,a43,a50,a51,a52,a53,a60,a61,a62,a63,a70,a71,a72,a73,a80,a81,a82,a83,a90,a91,a92,a93))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iTestInteropObject,Func48,40)

IDLC_END_INTF(ni,iTestInteropObject)

/** interface : iTestInteropSink **/
IDLC_BEGIN_INTF(ni,iTestInteropSink)
/** ni -> iTestInteropSink::OnTestInteropSink_Void/0 **/
IDLC_METH_BEGIN(ni,iTestInteropSink,OnTestInteropSink_Void,0)
	IDLC_METH_CALL_VOID(ni,iTestInteropSink,OnTestInteropSink_Void,0,())
IDLC_METH_END(ni,iTestInteropSink,OnTestInteropSink_Void,0)

/** ni -> iTestInteropSink::OnTestInteropSink_Void_I32/1 **/
IDLC_METH_BEGIN(ni,iTestInteropSink,OnTestInteropSink_Void_I32,1)
	IDLC_DECL_VAR(ni::tI32,v)
	IDLC_BUF_TO_BASE(ni::eType_I32,v)
	IDLC_METH_CALL_VOID(ni,iTestInteropSink,OnTestInteropSink_Void_I32,1,(v))
IDLC_METH_END(ni,iTestInteropSink,OnTestInteropSink_Void_I32,1)

/** ni -> iTestInteropSink::OnTestInteropSink_Void_I64/1 **/
IDLC_METH_BEGIN(ni,iTestInteropSink,OnTestInteropSink_Void_I64,1)
	IDLC_DECL_VAR(ni::tI64,v)
	IDLC_BUF_TO_BASE(ni::eType_I64,v)
	IDLC_METH_CALL_VOID(ni,iTestInteropSink,OnTestInteropSink_Void_I64,1,(v))
IDLC_METH_END(ni,iTestInteropSink,OnTestInteropSink_Void_I64,1)

/** ni -> iTestInteropSink::OnTestInteropSink_Void_Ptr/1 **/
IDLC_METH_BEGIN(ni,iTestInteropSink,OnTestInteropSink_Void_Ptr,1)
	IDLC_DECL_VAR(ni::tIntPtr,v)
	IDLC_BUF_TO_BASE(ni::eType_IntPtr,v)
	IDLC_METH_CALL_VOID(ni,iTestInteropSink,OnTestInteropSink_Void_Ptr,1,(v))
IDLC_METH_END(ni,iTestInteropSink,OnTestInteropSink_Void_Ptr,1)

/** ni -> iTestInteropSink::OnTestInteropSink_Var_Var/1 **/
IDLC_METH_BEGIN(ni,iTestInteropSink,OnTestInteropSink_Var_Var,1)
	IDLC_DECL_VAR(ni::Var,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,v)
	IDLC_DECL_RETVAR(ni::Var,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropSink,OnTestInteropSink_Var_Var,1,(v))
	IDLC_RET_FROM_BASE(ni::eType_Variant,_Ret)
IDLC_METH_END(ni,iTestInteropSink,OnTestInteropSink_Var_Var,1)

/** ni -> iTestInteropSink::OnTestInteropSink_Vec3f_String/1 **/
IDLC_METH_BEGIN(ni,iTestInteropSink,OnTestInteropSink_Vec3f_String,1)
	IDLC_DECL_VAR(ni::achar*,aaszStr)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszStr)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropSink,OnTestInteropSink_Vec3f_String,1,(aaszStr))
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iTestInteropSink,OnTestInteropSink_Vec3f_String,1)

/** ni -> iTestInteropSink::OnTestInteropSink_Vec3f_CObject/1 **/
IDLC_METH_BEGIN(ni,iTestInteropSink,OnTestInteropSink_Vec3f_CObject,1)
	IDLC_DECL_VAR(iTestInteropObject*,apObj)
	IDLC_BUF_TO_INTF(iTestInteropObject,apObj)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropSink,OnTestInteropSink_Vec3f_CObject,1,(apObj))
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iTestInteropSink,OnTestInteropSink_Vec3f_CObject,1)

/** ni -> iTestInteropSink::OnTestInteropSink_Vec3f_Object/1 **/
IDLC_METH_BEGIN(ni,iTestInteropSink,OnTestInteropSink_Vec3f_Object,1)
	IDLC_DECL_VAR(iTestInteropObject*,apObj)
	IDLC_BUF_TO_INTF(iTestInteropObject,apObj)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropSink,OnTestInteropSink_Vec3f_Object,1,(apObj))
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iTestInteropSink,OnTestInteropSink_Vec3f_Object,1)

/** ni -> iTestInteropSink::OnTestInteropSink_Object_CObject/1 **/
IDLC_METH_BEGIN(ni,iTestInteropSink,OnTestInteropSink_Object_CObject,1)
	IDLC_DECL_VAR(iTestInteropObject*,apObj)
	IDLC_BUF_TO_INTF(iTestInteropObject,apObj)
	IDLC_DECL_RETVAR(iTestInteropObject*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropSink,OnTestInteropSink_Object_CObject,1,(apObj))
	IDLC_RET_FROM_INTF(iTestInteropObject,_Ret)
IDLC_METH_END(ni,iTestInteropSink,OnTestInteropSink_Object_CObject,1)

/** ni -> iTestInteropSink::OnTestInteropSink_Object_Object/1 **/
IDLC_METH_BEGIN(ni,iTestInteropSink,OnTestInteropSink_Object_Object,1)
	IDLC_DECL_VAR(iTestInteropObject*,apObj)
	IDLC_BUF_TO_INTF(iTestInteropObject,apObj)
	IDLC_DECL_RETVAR(iTestInteropObject*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropSink,OnTestInteropSink_Object_Object,1,(apObj))
	IDLC_RET_FROM_INTF(iTestInteropObject,_Ret)
IDLC_METH_END(ni,iTestInteropSink,OnTestInteropSink_Object_Object,1)

/** ni -> iTestInteropSink::OnTestInteropSink_Object_Object2/3 **/
IDLC_METH_BEGIN(ni,iTestInteropSink,OnTestInteropSink_Object_Object2,3)
	IDLC_DECL_VAR(ni::tU32,a)
	IDLC_BUF_TO_BASE(ni::eType_U32,a)
	IDLC_DECL_VAR(ni::tF32,b)
	IDLC_BUF_TO_BASE(ni::eType_F32,b)
	IDLC_DECL_VAR(iTestInteropObject*,apObj)
	IDLC_BUF_TO_INTF(iTestInteropObject,apObj)
	IDLC_DECL_RETVAR(iTestInteropObject*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropSink,OnTestInteropSink_Object_Object2,3,(a,b,apObj))
	IDLC_RET_FROM_INTF(iTestInteropObject,_Ret)
IDLC_METH_END(ni,iTestInteropSink,OnTestInteropSink_Object_Object2,3)

/** ni -> iTestInteropSink::OnTestInteropSink_Object_ObjectVec3f/2 **/
IDLC_METH_BEGIN(ni,iTestInteropSink,OnTestInteropSink_Object_ObjectVec3f,2)
	IDLC_DECL_VAR(iTestInteropObject*,apObj)
	IDLC_BUF_TO_INTF(iTestInteropObject,apObj)
	IDLC_DECL_VAR(ni::sVec3f,c)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,c)
	IDLC_DECL_RETVAR(iTestInteropObject*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropSink,OnTestInteropSink_Object_ObjectVec3f,2,(apObj,c))
	IDLC_RET_FROM_INTF(iTestInteropObject,_Ret)
IDLC_METH_END(ni,iTestInteropSink,OnTestInteropSink_Object_ObjectVec3f,2)

IDLC_END_INTF(ni,iTestInteropSink)

/** interface : iTestInteropForEach **/
IDLC_BEGIN_INTF(ni,iTestInteropForEach)
/** ni -> iTestInteropForEach::ForEachObject_OnTestInteropSink_Void/1 **/
IDLC_METH_BEGIN(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Void,1)
	IDLC_DECL_VAR(iTestInteropSink*,apSink)
	IDLC_BUF_TO_INTF(iTestInteropSink,apSink)
	IDLC_METH_CALL_VOID(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Void,1,(apSink))
IDLC_METH_END(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Void,1)

/** ni -> iTestInteropForEach::ForEachObject_OnTestInteropSink_Void_I32/2 **/
IDLC_METH_BEGIN(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Void_I32,2)
	IDLC_DECL_VAR(iTestInteropSink*,apSink)
	IDLC_BUF_TO_INTF(iTestInteropSink,apSink)
	IDLC_DECL_VAR(ni::tI32,v)
	IDLC_BUF_TO_BASE(ni::eType_I32,v)
	IDLC_METH_CALL_VOID(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Void_I32,2,(apSink,v))
IDLC_METH_END(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Void_I32,2)

/** ni -> iTestInteropForEach::ForEachObject_OnTestInteropSink_Void_I64/2 **/
IDLC_METH_BEGIN(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Void_I64,2)
	IDLC_DECL_VAR(iTestInteropSink*,apSink)
	IDLC_BUF_TO_INTF(iTestInteropSink,apSink)
	IDLC_DECL_VAR(ni::tI64,v)
	IDLC_BUF_TO_BASE(ni::eType_I64,v)
	IDLC_METH_CALL_VOID(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Void_I64,2,(apSink,v))
IDLC_METH_END(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Void_I64,2)

/** ni -> iTestInteropForEach::ForEachObject_OnTestInteropSink_Void_Ptr/2 **/
IDLC_METH_BEGIN(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Void_Ptr,2)
	IDLC_DECL_VAR(iTestInteropSink*,apSink)
	IDLC_BUF_TO_INTF(iTestInteropSink,apSink)
	IDLC_DECL_VAR(ni::tPtr,v)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,v)
	IDLC_METH_CALL_VOID(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Void_Ptr,2,(apSink,v))
IDLC_METH_END(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Void_Ptr,2)

/** ni -> iTestInteropForEach::ForEachObject_OnTestInteropSink_Var_Var/2 **/
IDLC_METH_BEGIN(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Var_Var,2)
	IDLC_DECL_VAR(iTestInteropSink*,apSink)
	IDLC_BUF_TO_INTF(iTestInteropSink,apSink)
	IDLC_DECL_VAR(Var,v)
	IDLC_BUF_TO_BASE(ni::eType_Variant,v)
	IDLC_DECL_RETVAR(Var,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Var_Var,2,(apSink,v))
	IDLC_RET_FROM_BASE(ni::eType_Variant,_Ret)
IDLC_METH_END(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Var_Var,2)

/** ni -> iTestInteropForEach::ForEachObject_OnTestInteropSink_Vec3f_String/2 **/
IDLC_METH_BEGIN(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Vec3f_String,2)
	IDLC_DECL_VAR(iTestInteropSink*,apSink)
	IDLC_BUF_TO_INTF(iTestInteropSink,apSink)
	IDLC_DECL_VAR(ni::achar*,aaszStr)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszStr)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Vec3f_String,2,(apSink,aaszStr))
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Vec3f_String,2)

/** ni -> iTestInteropForEach::ForEachObject_OnTestInteropSink_Vec3f_CObject/1 **/
IDLC_METH_BEGIN(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Vec3f_CObject,1)
	IDLC_DECL_VAR(iTestInteropSink*,apSink)
	IDLC_BUF_TO_INTF(iTestInteropSink,apSink)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Vec3f_CObject,1,(apSink))
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Vec3f_CObject,1)

/** ni -> iTestInteropForEach::ForEachObject_OnTestInteropSink_Vec3f_Object/1 **/
IDLC_METH_BEGIN(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Vec3f_Object,1)
	IDLC_DECL_VAR(iTestInteropSink*,apSink)
	IDLC_BUF_TO_INTF(iTestInteropSink,apSink)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Vec3f_Object,1,(apSink))
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Vec3f_Object,1)

/** ni -> iTestInteropForEach::ForEachObject_OnTestInteropSink_Object_CObject/1 **/
IDLC_METH_BEGIN(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Object_CObject,1)
	IDLC_DECL_VAR(iTestInteropSink*,apSink)
	IDLC_BUF_TO_INTF(iTestInteropSink,apSink)
	IDLC_DECL_RETVAR(iTestInteropObject*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Object_CObject,1,(apSink))
	IDLC_RET_FROM_INTF(iTestInteropObject,_Ret)
IDLC_METH_END(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Object_CObject,1)

/** ni -> iTestInteropForEach::ForEachObject_OnTestInteropSink_Object_Object/1 **/
IDLC_METH_BEGIN(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Object_Object,1)
	IDLC_DECL_VAR(iTestInteropSink*,apSink)
	IDLC_BUF_TO_INTF(iTestInteropSink,apSink)
	IDLC_DECL_RETVAR(iTestInteropObject*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Object_Object,1,(apSink))
	IDLC_RET_FROM_INTF(iTestInteropObject,_Ret)
IDLC_METH_END(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Object_Object,1)

/** ni -> iTestInteropForEach::ForEachObject_OnTestInteropSink_Object_Object2/3 **/
IDLC_METH_BEGIN(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Object_Object2,3)
	IDLC_DECL_VAR(iTestInteropSink*,apSink)
	IDLC_BUF_TO_INTF(iTestInteropSink,apSink)
	IDLC_DECL_VAR(ni::tU32,a)
	IDLC_BUF_TO_BASE(ni::eType_U32,a)
	IDLC_DECL_VAR(ni::tF32,b)
	IDLC_BUF_TO_BASE(ni::eType_F32,b)
	IDLC_DECL_RETVAR(iTestInteropObject*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Object_Object2,3,(apSink,a,b))
	IDLC_RET_FROM_INTF(iTestInteropObject,_Ret)
IDLC_METH_END(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Object_Object2,3)

/** ni -> iTestInteropForEach::ForEachObject_OnTestInteropSink_Object_ObjectVec3f/2 **/
IDLC_METH_BEGIN(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Object_ObjectVec3f,2)
	IDLC_DECL_VAR(iTestInteropSink*,apSink)
	IDLC_BUF_TO_INTF(iTestInteropSink,apSink)
	IDLC_DECL_VAR(ni::sVec3f,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,v)
	IDLC_DECL_RETVAR(iTestInteropObject*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Object_ObjectVec3f,2,(apSink,v))
	IDLC_RET_FROM_INTF(iTestInteropObject,_Ret)
IDLC_METH_END(ni,iTestInteropForEach,ForEachObject_OnTestInteropSink_Object_ObjectVec3f,2)

IDLC_END_INTF(ni,iTestInteropForEach)

/** interface : iTestInterop **/
IDLC_BEGIN_INTF(ni,iTestInterop)
/** ni -> iTestInterop::FeatureSetNeverThere/0 -> MIN FEATURES '200' **/
#if niMinFeatures(200)
/** ni -> iTestInterop::FeatureSetNeverThere/0 **/
IDLC_METH_BEGIN(ni,iTestInterop,FeatureSetNeverThere,0)
	IDLC_METH_CALL_VOID(ni,iTestInterop,FeatureSetNeverThere,0,())
IDLC_METH_END(ni,iTestInterop,FeatureSetNeverThere,0)
#endif // niMinFeatures(200)

/** ni -> iTestInterop::FeatureSetFull/0 -> MIN FEATURES '30' **/
#if niMinFeatures(30)
/** ni -> iTestInterop::FeatureSetFull/0 **/
IDLC_METH_BEGIN(ni,iTestInterop,FeatureSetFull,0)
	IDLC_METH_CALL_VOID(ni,iTestInterop,FeatureSetFull,0,())
IDLC_METH_END(ni,iTestInterop,FeatureSetFull,0)
#endif // niMinFeatures(30)

/** ni -> iTestInterop::FeatureSetEmbedded/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iTestInterop::FeatureSetEmbedded/0 **/
IDLC_METH_BEGIN(ni,iTestInterop,FeatureSetEmbedded,0)
	IDLC_METH_CALL_VOID(ni,iTestInterop,FeatureSetEmbedded,0,())
IDLC_METH_END(ni,iTestInterop,FeatureSetEmbedded,0)
#endif // niMinFeatures(20)

/** ni -> iTestInterop::FeatureSetLight/0 **/
IDLC_METH_BEGIN(ni,iTestInterop,FeatureSetLight,0)
	IDLC_METH_CALL_VOID(ni,iTestInterop,FeatureSetLight,0,())
IDLC_METH_END(ni,iTestInterop,FeatureSetLight,0)

/** ni -> iTestInterop::GenerateCrash/1 **/
IDLC_METH_BEGIN(ni,iTestInterop,GenerateCrash,1)
	IDLC_DECL_VAR(ni::tU32,anExceptionType)
	IDLC_BUF_TO_BASE(ni::eType_U32,anExceptionType)
	IDLC_DECL_RETVAR(tInt,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInterop,GenerateCrash,1,(anExceptionType))
	IDLC_RET_FROM_BASE(ni::eType_Int,_Ret)
IDLC_METH_END(ni,iTestInterop,GenerateCrash,1)

/** ni -> iTestInterop::GetNumTestInteropObjects/0 **/
IDLC_METH_BEGIN(ni,iTestInterop,GetNumTestInteropObjects,0)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInterop,GetNumTestInteropObjects,0,())
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iTestInterop,GetNumTestInteropObjects,0)

/** ni -> iTestInterop::GetTestInteropObject/1 **/
IDLC_METH_BEGIN(ni,iTestInterop,GetTestInteropObject,1)
	IDLC_DECL_VAR(ni::tSize,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_Size,anIndex)
	IDLC_DECL_RETVAR(iTestInteropObject*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInterop,GetTestInteropObject,1,(anIndex))
	IDLC_RET_FROM_INTF(iTestInteropObject,_Ret)
IDLC_METH_END(ni,iTestInterop,GetTestInteropObject,1)

/** ni -> iTestInterop::CreateTestInteropObject/1 **/
IDLC_METH_BEGIN(ni,iTestInterop,CreateTestInteropObject,1)
	IDLC_DECL_VAR(ni::tU32,anNumItems)
	IDLC_BUF_TO_BASE(ni::eType_U32,anNumItems)
	IDLC_DECL_RETVAR(iTestInteropObject*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInterop,CreateTestInteropObject,1,(anNumItems))
	IDLC_RET_FROM_INTF(iTestInteropObject,_Ret)
IDLC_METH_END(ni,iTestInterop,CreateTestInteropObject,1)

/** ni -> iTestInterop::CreateTestInteropObjectPtr/1 **/
IDLC_METH_BEGIN(ni,iTestInterop,CreateTestInteropObjectPtr,1)
	IDLC_DECL_VAR(ni::tU32,anNumItems)
	IDLC_BUF_TO_BASE(ni::eType_U32,anNumItems)
	IDLC_DECL_RETVAR(Ptr<iTestInteropObject>,_Ret)
	IDLC_METH_CALL(_Ret,ni,iTestInterop,CreateTestInteropObjectPtr,1,(anNumItems))
	IDLC_RET_FROM_INTF(,_Ret)
IDLC_METH_END(ni,iTestInterop,CreateTestInteropObjectPtr,1)

IDLC_END_INTF(ni,iTestInterop)

IDLC_END_NAMESPACE()
// EOF //
