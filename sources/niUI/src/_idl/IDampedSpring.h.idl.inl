#include "../API/niUI/IDampedSpring.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iDampedSpring1 **/
IDLC_BEGIN_INTF(ni,iDampedSpring1)
/** ni -> iDampedSpring1::SetKd/1 **/
IDLC_METH_BEGIN(ni,iDampedSpring1,SetKd,1)
	IDLC_DECL_VAR(tF32,afD)
	IDLC_BUF_TO_BASE(ni::eType_F32,afD)
	IDLC_METH_CALL_VOID(ni,iDampedSpring1,SetKd,1,(afD))
IDLC_METH_END(ni,iDampedSpring1,SetKd,1)

/** ni -> iDampedSpring1::GetKd/0 **/
IDLC_METH_BEGIN(ni,iDampedSpring1,GetKd,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpring1,GetKd,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpring1,GetKd,0)

/** ni -> iDampedSpring1::SetKs/1 **/
IDLC_METH_BEGIN(ni,iDampedSpring1,SetKs,1)
	IDLC_DECL_VAR(tF32,afD)
	IDLC_BUF_TO_BASE(ni::eType_F32,afD)
	IDLC_METH_CALL_VOID(ni,iDampedSpring1,SetKs,1,(afD))
IDLC_METH_END(ni,iDampedSpring1,SetKs,1)

/** ni -> iDampedSpring1::GetKs/0 **/
IDLC_METH_BEGIN(ni,iDampedSpring1,GetKs,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpring1,GetKs,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpring1,GetKs,0)

/** ni -> iDampedSpring1::SetStiffnessAndDampingRatio/2 **/
IDLC_METH_BEGIN(ni,iDampedSpring1,SetStiffnessAndDampingRatio,2)
	IDLC_DECL_VAR(tF32,afKs)
	IDLC_BUF_TO_BASE(ni::eType_F32,afKs)
	IDLC_DECL_VAR(tF32,afE)
	IDLC_BUF_TO_BASE(ni::eType_F32,afE)
	IDLC_METH_CALL_VOID(ni,iDampedSpring1,SetStiffnessAndDampingRatio,2,(afKs,afE))
IDLC_METH_END(ni,iDampedSpring1,SetStiffnessAndDampingRatio,2)

/** ni -> iDampedSpring1::SetDampingRatio/1 **/
IDLC_METH_BEGIN(ni,iDampedSpring1,SetDampingRatio,1)
	IDLC_DECL_VAR(tF32,afE)
	IDLC_BUF_TO_BASE(ni::eType_F32,afE)
	IDLC_METH_CALL_VOID(ni,iDampedSpring1,SetDampingRatio,1,(afE))
IDLC_METH_END(ni,iDampedSpring1,SetDampingRatio,1)

/** ni -> iDampedSpring1::GetDampingRatio/0 **/
IDLC_METH_BEGIN(ni,iDampedSpring1,GetDampingRatio,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpring1,GetDampingRatio,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpring1,GetDampingRatio,0)

/** ni -> iDampedSpring1::SetVelocity/1 **/
IDLC_METH_BEGIN(ni,iDampedSpring1,SetVelocity,1)
	IDLC_DECL_VAR(tF32,avVel)
	IDLC_BUF_TO_BASE(ni::eType_F32,avVel)
	IDLC_METH_CALL_VOID(ni,iDampedSpring1,SetVelocity,1,(avVel))
IDLC_METH_END(ni,iDampedSpring1,SetVelocity,1)

/** ni -> iDampedSpring1::GetVelocity/0 **/
IDLC_METH_BEGIN(ni,iDampedSpring1,GetVelocity,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpring1,GetVelocity,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpring1,GetVelocity,0)

/** ni -> iDampedSpring1::ComputeAcceleration/1 **/
IDLC_METH_BEGIN(ni,iDampedSpring1,ComputeAcceleration,1)
	IDLC_DECL_VAR(tF32,avD)
	IDLC_BUF_TO_BASE(ni::eType_F32,avD)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpring1,ComputeAcceleration,1,(avD))
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpring1,ComputeAcceleration,1)

/** ni -> iDampedSpring1::UpdateVelocity/2 **/
IDLC_METH_BEGIN(ni,iDampedSpring1,UpdateVelocity,2)
	IDLC_DECL_VAR(ni::tF32,afDeltaTime)
	IDLC_BUF_TO_BASE(ni::eType_F32,afDeltaTime)
	IDLC_DECL_VAR(tF32,avD)
	IDLC_BUF_TO_BASE(ni::eType_F32,avD)
	IDLC_METH_CALL_VOID(ni,iDampedSpring1,UpdateVelocity,2,(afDeltaTime,avD))
IDLC_METH_END(ni,iDampedSpring1,UpdateVelocity,2)

/** ni -> iDampedSpring1::UpdateVelocityWithAcceleration/2 **/
IDLC_METH_BEGIN(ni,iDampedSpring1,UpdateVelocityWithAcceleration,2)
	IDLC_DECL_VAR(ni::tF32,afDeltaTime)
	IDLC_BUF_TO_BASE(ni::eType_F32,afDeltaTime)
	IDLC_DECL_VAR(tF32,avAcc)
	IDLC_BUF_TO_BASE(ni::eType_F32,avAcc)
	IDLC_METH_CALL_VOID(ni,iDampedSpring1,UpdateVelocityWithAcceleration,2,(afDeltaTime,avAcc))
IDLC_METH_END(ni,iDampedSpring1,UpdateVelocityWithAcceleration,2)

IDLC_END_INTF(ni,iDampedSpring1)

/** interface : iDampedSpring2 **/
IDLC_BEGIN_INTF(ni,iDampedSpring2)
/** ni -> iDampedSpring2::SetKd/1 **/
IDLC_METH_BEGIN(ni,iDampedSpring2,SetKd,1)
	IDLC_DECL_VAR(tF32,afD)
	IDLC_BUF_TO_BASE(ni::eType_F32,afD)
	IDLC_METH_CALL_VOID(ni,iDampedSpring2,SetKd,1,(afD))
IDLC_METH_END(ni,iDampedSpring2,SetKd,1)

/** ni -> iDampedSpring2::GetKd/0 **/
IDLC_METH_BEGIN(ni,iDampedSpring2,GetKd,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpring2,GetKd,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpring2,GetKd,0)

/** ni -> iDampedSpring2::SetKs/1 **/
IDLC_METH_BEGIN(ni,iDampedSpring2,SetKs,1)
	IDLC_DECL_VAR(tF32,afD)
	IDLC_BUF_TO_BASE(ni::eType_F32,afD)
	IDLC_METH_CALL_VOID(ni,iDampedSpring2,SetKs,1,(afD))
IDLC_METH_END(ni,iDampedSpring2,SetKs,1)

/** ni -> iDampedSpring2::GetKs/0 **/
IDLC_METH_BEGIN(ni,iDampedSpring2,GetKs,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpring2,GetKs,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpring2,GetKs,0)

/** ni -> iDampedSpring2::SetStiffnessAndDampingRatio/2 **/
IDLC_METH_BEGIN(ni,iDampedSpring2,SetStiffnessAndDampingRatio,2)
	IDLC_DECL_VAR(tF32,afKs)
	IDLC_BUF_TO_BASE(ni::eType_F32,afKs)
	IDLC_DECL_VAR(tF32,afE)
	IDLC_BUF_TO_BASE(ni::eType_F32,afE)
	IDLC_METH_CALL_VOID(ni,iDampedSpring2,SetStiffnessAndDampingRatio,2,(afKs,afE))
IDLC_METH_END(ni,iDampedSpring2,SetStiffnessAndDampingRatio,2)

/** ni -> iDampedSpring2::SetDampingRatio/1 **/
IDLC_METH_BEGIN(ni,iDampedSpring2,SetDampingRatio,1)
	IDLC_DECL_VAR(tF32,afE)
	IDLC_BUF_TO_BASE(ni::eType_F32,afE)
	IDLC_METH_CALL_VOID(ni,iDampedSpring2,SetDampingRatio,1,(afE))
IDLC_METH_END(ni,iDampedSpring2,SetDampingRatio,1)

/** ni -> iDampedSpring2::GetDampingRatio/0 **/
IDLC_METH_BEGIN(ni,iDampedSpring2,GetDampingRatio,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpring2,GetDampingRatio,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpring2,GetDampingRatio,0)

/** ni -> iDampedSpring2::SetVelocity/1 **/
IDLC_METH_BEGIN(ni,iDampedSpring2,SetVelocity,1)
	IDLC_DECL_VAR(sVec2f,avVel)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,avVel)
	IDLC_METH_CALL_VOID(ni,iDampedSpring2,SetVelocity,1,(avVel))
IDLC_METH_END(ni,iDampedSpring2,SetVelocity,1)

/** ni -> iDampedSpring2::GetVelocity/0 **/
IDLC_METH_BEGIN(ni,iDampedSpring2,GetVelocity,0)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpring2,GetVelocity,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iDampedSpring2,GetVelocity,0)

/** ni -> iDampedSpring2::ComputeAcceleration/1 **/
IDLC_METH_BEGIN(ni,iDampedSpring2,ComputeAcceleration,1)
	IDLC_DECL_VAR(sVec2f,avD)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,avD)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpring2,ComputeAcceleration,1,(avD))
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iDampedSpring2,ComputeAcceleration,1)

/** ni -> iDampedSpring2::UpdateVelocity/2 **/
IDLC_METH_BEGIN(ni,iDampedSpring2,UpdateVelocity,2)
	IDLC_DECL_VAR(ni::tF32,afDeltaTime)
	IDLC_BUF_TO_BASE(ni::eType_F32,afDeltaTime)
	IDLC_DECL_VAR(sVec2f,avD)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,avD)
	IDLC_METH_CALL_VOID(ni,iDampedSpring2,UpdateVelocity,2,(afDeltaTime,avD))
IDLC_METH_END(ni,iDampedSpring2,UpdateVelocity,2)

/** ni -> iDampedSpring2::UpdateVelocityWithAcceleration/2 **/
IDLC_METH_BEGIN(ni,iDampedSpring2,UpdateVelocityWithAcceleration,2)
	IDLC_DECL_VAR(ni::tF32,afDeltaTime)
	IDLC_BUF_TO_BASE(ni::eType_F32,afDeltaTime)
	IDLC_DECL_VAR(sVec2f,avAcc)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,avAcc)
	IDLC_METH_CALL_VOID(ni,iDampedSpring2,UpdateVelocityWithAcceleration,2,(afDeltaTime,avAcc))
IDLC_METH_END(ni,iDampedSpring2,UpdateVelocityWithAcceleration,2)

IDLC_END_INTF(ni,iDampedSpring2)

/** interface : iDampedSpring3 **/
IDLC_BEGIN_INTF(ni,iDampedSpring3)
/** ni -> iDampedSpring3::SetKd/1 **/
IDLC_METH_BEGIN(ni,iDampedSpring3,SetKd,1)
	IDLC_DECL_VAR(tF32,afD)
	IDLC_BUF_TO_BASE(ni::eType_F32,afD)
	IDLC_METH_CALL_VOID(ni,iDampedSpring3,SetKd,1,(afD))
IDLC_METH_END(ni,iDampedSpring3,SetKd,1)

/** ni -> iDampedSpring3::GetKd/0 **/
IDLC_METH_BEGIN(ni,iDampedSpring3,GetKd,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpring3,GetKd,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpring3,GetKd,0)

/** ni -> iDampedSpring3::SetKs/1 **/
IDLC_METH_BEGIN(ni,iDampedSpring3,SetKs,1)
	IDLC_DECL_VAR(tF32,afD)
	IDLC_BUF_TO_BASE(ni::eType_F32,afD)
	IDLC_METH_CALL_VOID(ni,iDampedSpring3,SetKs,1,(afD))
IDLC_METH_END(ni,iDampedSpring3,SetKs,1)

/** ni -> iDampedSpring3::GetKs/0 **/
IDLC_METH_BEGIN(ni,iDampedSpring3,GetKs,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpring3,GetKs,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpring3,GetKs,0)

/** ni -> iDampedSpring3::SetStiffnessAndDampingRatio/2 **/
IDLC_METH_BEGIN(ni,iDampedSpring3,SetStiffnessAndDampingRatio,2)
	IDLC_DECL_VAR(tF32,afKs)
	IDLC_BUF_TO_BASE(ni::eType_F32,afKs)
	IDLC_DECL_VAR(tF32,afE)
	IDLC_BUF_TO_BASE(ni::eType_F32,afE)
	IDLC_METH_CALL_VOID(ni,iDampedSpring3,SetStiffnessAndDampingRatio,2,(afKs,afE))
IDLC_METH_END(ni,iDampedSpring3,SetStiffnessAndDampingRatio,2)

/** ni -> iDampedSpring3::SetDampingRatio/1 **/
IDLC_METH_BEGIN(ni,iDampedSpring3,SetDampingRatio,1)
	IDLC_DECL_VAR(tF32,afE)
	IDLC_BUF_TO_BASE(ni::eType_F32,afE)
	IDLC_METH_CALL_VOID(ni,iDampedSpring3,SetDampingRatio,1,(afE))
IDLC_METH_END(ni,iDampedSpring3,SetDampingRatio,1)

/** ni -> iDampedSpring3::GetDampingRatio/0 **/
IDLC_METH_BEGIN(ni,iDampedSpring3,GetDampingRatio,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpring3,GetDampingRatio,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpring3,GetDampingRatio,0)

/** ni -> iDampedSpring3::SetVelocity/1 **/
IDLC_METH_BEGIN(ni,iDampedSpring3,SetVelocity,1)
	IDLC_DECL_VAR(sVec3f,avVel)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,avVel)
	IDLC_METH_CALL_VOID(ni,iDampedSpring3,SetVelocity,1,(avVel))
IDLC_METH_END(ni,iDampedSpring3,SetVelocity,1)

/** ni -> iDampedSpring3::GetVelocity/0 **/
IDLC_METH_BEGIN(ni,iDampedSpring3,GetVelocity,0)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpring3,GetVelocity,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iDampedSpring3,GetVelocity,0)

/** ni -> iDampedSpring3::ComputeAcceleration/1 **/
IDLC_METH_BEGIN(ni,iDampedSpring3,ComputeAcceleration,1)
	IDLC_DECL_VAR(sVec3f,avD)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,avD)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpring3,ComputeAcceleration,1,(avD))
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iDampedSpring3,ComputeAcceleration,1)

/** ni -> iDampedSpring3::UpdateVelocity/2 **/
IDLC_METH_BEGIN(ni,iDampedSpring3,UpdateVelocity,2)
	IDLC_DECL_VAR(ni::tF32,afDeltaTime)
	IDLC_BUF_TO_BASE(ni::eType_F32,afDeltaTime)
	IDLC_DECL_VAR(sVec3f,avD)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,avD)
	IDLC_METH_CALL_VOID(ni,iDampedSpring3,UpdateVelocity,2,(afDeltaTime,avD))
IDLC_METH_END(ni,iDampedSpring3,UpdateVelocity,2)

/** ni -> iDampedSpring3::UpdateVelocityWithAcceleration/2 **/
IDLC_METH_BEGIN(ni,iDampedSpring3,UpdateVelocityWithAcceleration,2)
	IDLC_DECL_VAR(ni::tF32,afDeltaTime)
	IDLC_BUF_TO_BASE(ni::eType_F32,afDeltaTime)
	IDLC_DECL_VAR(sVec3f,avAcc)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,avAcc)
	IDLC_METH_CALL_VOID(ni,iDampedSpring3,UpdateVelocityWithAcceleration,2,(afDeltaTime,avAcc))
IDLC_METH_END(ni,iDampedSpring3,UpdateVelocityWithAcceleration,2)

IDLC_END_INTF(ni,iDampedSpring3)

/** interface : iDampedSpring4 **/
IDLC_BEGIN_INTF(ni,iDampedSpring4)
/** ni -> iDampedSpring4::SetKd/1 **/
IDLC_METH_BEGIN(ni,iDampedSpring4,SetKd,1)
	IDLC_DECL_VAR(tF32,afD)
	IDLC_BUF_TO_BASE(ni::eType_F32,afD)
	IDLC_METH_CALL_VOID(ni,iDampedSpring4,SetKd,1,(afD))
IDLC_METH_END(ni,iDampedSpring4,SetKd,1)

/** ni -> iDampedSpring4::GetKd/0 **/
IDLC_METH_BEGIN(ni,iDampedSpring4,GetKd,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpring4,GetKd,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpring4,GetKd,0)

/** ni -> iDampedSpring4::SetKs/1 **/
IDLC_METH_BEGIN(ni,iDampedSpring4,SetKs,1)
	IDLC_DECL_VAR(tF32,afD)
	IDLC_BUF_TO_BASE(ni::eType_F32,afD)
	IDLC_METH_CALL_VOID(ni,iDampedSpring4,SetKs,1,(afD))
IDLC_METH_END(ni,iDampedSpring4,SetKs,1)

/** ni -> iDampedSpring4::GetKs/0 **/
IDLC_METH_BEGIN(ni,iDampedSpring4,GetKs,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpring4,GetKs,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpring4,GetKs,0)

/** ni -> iDampedSpring4::SetStiffnessAndDampingRatio/2 **/
IDLC_METH_BEGIN(ni,iDampedSpring4,SetStiffnessAndDampingRatio,2)
	IDLC_DECL_VAR(tF32,afKs)
	IDLC_BUF_TO_BASE(ni::eType_F32,afKs)
	IDLC_DECL_VAR(tF32,afE)
	IDLC_BUF_TO_BASE(ni::eType_F32,afE)
	IDLC_METH_CALL_VOID(ni,iDampedSpring4,SetStiffnessAndDampingRatio,2,(afKs,afE))
IDLC_METH_END(ni,iDampedSpring4,SetStiffnessAndDampingRatio,2)

/** ni -> iDampedSpring4::SetDampingRatio/1 **/
IDLC_METH_BEGIN(ni,iDampedSpring4,SetDampingRatio,1)
	IDLC_DECL_VAR(tF32,afE)
	IDLC_BUF_TO_BASE(ni::eType_F32,afE)
	IDLC_METH_CALL_VOID(ni,iDampedSpring4,SetDampingRatio,1,(afE))
IDLC_METH_END(ni,iDampedSpring4,SetDampingRatio,1)

/** ni -> iDampedSpring4::GetDampingRatio/0 **/
IDLC_METH_BEGIN(ni,iDampedSpring4,GetDampingRatio,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpring4,GetDampingRatio,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpring4,GetDampingRatio,0)

/** ni -> iDampedSpring4::SetVelocity/1 **/
IDLC_METH_BEGIN(ni,iDampedSpring4,SetVelocity,1)
	IDLC_DECL_VAR(sVec4f,avVel)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,avVel)
	IDLC_METH_CALL_VOID(ni,iDampedSpring4,SetVelocity,1,(avVel))
IDLC_METH_END(ni,iDampedSpring4,SetVelocity,1)

/** ni -> iDampedSpring4::GetVelocity/0 **/
IDLC_METH_BEGIN(ni,iDampedSpring4,GetVelocity,0)
	IDLC_DECL_RETVAR(sVec4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpring4,GetVelocity,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iDampedSpring4,GetVelocity,0)

/** ni -> iDampedSpring4::ComputeAcceleration/1 **/
IDLC_METH_BEGIN(ni,iDampedSpring4,ComputeAcceleration,1)
	IDLC_DECL_VAR(sVec4f,avD)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,avD)
	IDLC_DECL_RETVAR(sVec4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpring4,ComputeAcceleration,1,(avD))
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iDampedSpring4,ComputeAcceleration,1)

/** ni -> iDampedSpring4::UpdateVelocity/2 **/
IDLC_METH_BEGIN(ni,iDampedSpring4,UpdateVelocity,2)
	IDLC_DECL_VAR(ni::tF32,afDeltaTime)
	IDLC_BUF_TO_BASE(ni::eType_F32,afDeltaTime)
	IDLC_DECL_VAR(sVec4f,avD)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,avD)
	IDLC_METH_CALL_VOID(ni,iDampedSpring4,UpdateVelocity,2,(afDeltaTime,avD))
IDLC_METH_END(ni,iDampedSpring4,UpdateVelocity,2)

/** ni -> iDampedSpring4::UpdateVelocityWithAcceleration/2 **/
IDLC_METH_BEGIN(ni,iDampedSpring4,UpdateVelocityWithAcceleration,2)
	IDLC_DECL_VAR(ni::tF32,afDeltaTime)
	IDLC_BUF_TO_BASE(ni::eType_F32,afDeltaTime)
	IDLC_DECL_VAR(sVec4f,avAcc)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,avAcc)
	IDLC_METH_CALL_VOID(ni,iDampedSpring4,UpdateVelocityWithAcceleration,2,(afDeltaTime,avAcc))
IDLC_METH_END(ni,iDampedSpring4,UpdateVelocityWithAcceleration,2)

IDLC_END_INTF(ni,iDampedSpring4)

/** interface : iDampedSpringPosition1 **/
IDLC_BEGIN_INTF(ni,iDampedSpringPosition1)
/** ni -> iDampedSpringPosition1::SetIdealPosition/1 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition1,SetIdealPosition,1)
	IDLC_DECL_VAR(tF32,avPos)
	IDLC_BUF_TO_BASE(ni::eType_F32,avPos)
	IDLC_METH_CALL_VOID(ni,iDampedSpringPosition1,SetIdealPosition,1,(avPos))
IDLC_METH_END(ni,iDampedSpringPosition1,SetIdealPosition,1)

/** ni -> iDampedSpringPosition1::GetIdealPosition/0 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition1,GetIdealPosition,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition1,GetIdealPosition,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition1,GetIdealPosition,0)

/** ni -> iDampedSpringPosition1::SetCurrentPosition/1 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition1,SetCurrentPosition,1)
	IDLC_DECL_VAR(tF32,avPos)
	IDLC_BUF_TO_BASE(ni::eType_F32,avPos)
	IDLC_METH_CALL_VOID(ni,iDampedSpringPosition1,SetCurrentPosition,1,(avPos))
IDLC_METH_END(ni,iDampedSpringPosition1,SetCurrentPosition,1)

/** ni -> iDampedSpringPosition1::GetCurrentPosition/0 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition1,GetCurrentPosition,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition1,GetCurrentPosition,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition1,GetCurrentPosition,0)

/** ni -> iDampedSpringPosition1::UpdatePosition/1 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition1,UpdatePosition,1)
	IDLC_DECL_VAR(ni::tF32,afDeltaTime)
	IDLC_BUF_TO_BASE(ni::eType_F32,afDeltaTime)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition1,UpdatePosition,1,(afDeltaTime))
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition1,UpdatePosition,1)

/** ni -> iDampedSpringPosition1::SetStep/1 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition1,SetStep,1)
	IDLC_DECL_VAR(tF32,afD)
	IDLC_BUF_TO_BASE(ni::eType_F32,afD)
	IDLC_METH_CALL_VOID(ni,iDampedSpringPosition1,SetStep,1,(afD))
IDLC_METH_END(ni,iDampedSpringPosition1,SetStep,1)

/** ni -> iDampedSpringPosition1::GetStep/0 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition1,GetStep,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition1,GetStep,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition1,GetStep,0)

/** ni -> iDampedSpringPosition1::SetSpeed/1 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition1,SetSpeed,1)
	IDLC_DECL_VAR(tF32,afD)
	IDLC_BUF_TO_BASE(ni::eType_F32,afD)
	IDLC_METH_CALL_VOID(ni,iDampedSpringPosition1,SetSpeed,1,(afD))
IDLC_METH_END(ni,iDampedSpringPosition1,SetSpeed,1)

/** ni -> iDampedSpringPosition1::GetSpeed/0 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition1,GetSpeed,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition1,GetSpeed,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition1,GetSpeed,0)

/** ni -> iDampedSpringPosition1::SetEndThreshold/1 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition1,SetEndThreshold,1)
	IDLC_DECL_VAR(tF32,afD)
	IDLC_BUF_TO_BASE(ni::eType_F32,afD)
	IDLC_METH_CALL_VOID(ni,iDampedSpringPosition1,SetEndThreshold,1,(afD))
IDLC_METH_END(ni,iDampedSpringPosition1,SetEndThreshold,1)

/** ni -> iDampedSpringPosition1::GetEndThreshold/0 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition1,GetEndThreshold,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition1,GetEndThreshold,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition1,GetEndThreshold,0)

/** ni -> iDampedSpringPosition1::GetIsEnded/0 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition1,GetIsEnded,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition1,GetIsEnded,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition1,GetIsEnded,0)

IDLC_END_INTF(ni,iDampedSpringPosition1)

/** interface : iDampedSpringPosition2 **/
IDLC_BEGIN_INTF(ni,iDampedSpringPosition2)
/** ni -> iDampedSpringPosition2::SetIdealPosition/1 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition2,SetIdealPosition,1)
	IDLC_DECL_VAR(sVec2f,avPos)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,avPos)
	IDLC_METH_CALL_VOID(ni,iDampedSpringPosition2,SetIdealPosition,1,(avPos))
IDLC_METH_END(ni,iDampedSpringPosition2,SetIdealPosition,1)

/** ni -> iDampedSpringPosition2::GetIdealPosition/0 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition2,GetIdealPosition,0)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition2,GetIdealPosition,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition2,GetIdealPosition,0)

/** ni -> iDampedSpringPosition2::SetCurrentPosition/1 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition2,SetCurrentPosition,1)
	IDLC_DECL_VAR(sVec2f,avPos)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,avPos)
	IDLC_METH_CALL_VOID(ni,iDampedSpringPosition2,SetCurrentPosition,1,(avPos))
IDLC_METH_END(ni,iDampedSpringPosition2,SetCurrentPosition,1)

/** ni -> iDampedSpringPosition2::GetCurrentPosition/0 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition2,GetCurrentPosition,0)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition2,GetCurrentPosition,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition2,GetCurrentPosition,0)

/** ni -> iDampedSpringPosition2::UpdatePosition/1 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition2,UpdatePosition,1)
	IDLC_DECL_VAR(ni::tF32,afDeltaTime)
	IDLC_BUF_TO_BASE(ni::eType_F32,afDeltaTime)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition2,UpdatePosition,1,(afDeltaTime))
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition2,UpdatePosition,1)

/** ni -> iDampedSpringPosition2::SetStep/1 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition2,SetStep,1)
	IDLC_DECL_VAR(tF32,afD)
	IDLC_BUF_TO_BASE(ni::eType_F32,afD)
	IDLC_METH_CALL_VOID(ni,iDampedSpringPosition2,SetStep,1,(afD))
IDLC_METH_END(ni,iDampedSpringPosition2,SetStep,1)

/** ni -> iDampedSpringPosition2::GetStep/0 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition2,GetStep,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition2,GetStep,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition2,GetStep,0)

/** ni -> iDampedSpringPosition2::SetSpeed/1 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition2,SetSpeed,1)
	IDLC_DECL_VAR(tF32,afD)
	IDLC_BUF_TO_BASE(ni::eType_F32,afD)
	IDLC_METH_CALL_VOID(ni,iDampedSpringPosition2,SetSpeed,1,(afD))
IDLC_METH_END(ni,iDampedSpringPosition2,SetSpeed,1)

/** ni -> iDampedSpringPosition2::GetSpeed/0 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition2,GetSpeed,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition2,GetSpeed,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition2,GetSpeed,0)

/** ni -> iDampedSpringPosition2::SetEndThreshold/1 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition2,SetEndThreshold,1)
	IDLC_DECL_VAR(tF32,afD)
	IDLC_BUF_TO_BASE(ni::eType_F32,afD)
	IDLC_METH_CALL_VOID(ni,iDampedSpringPosition2,SetEndThreshold,1,(afD))
IDLC_METH_END(ni,iDampedSpringPosition2,SetEndThreshold,1)

/** ni -> iDampedSpringPosition2::GetEndThreshold/0 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition2,GetEndThreshold,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition2,GetEndThreshold,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition2,GetEndThreshold,0)

/** ni -> iDampedSpringPosition2::GetIsEnded/0 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition2,GetIsEnded,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition2,GetIsEnded,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition2,GetIsEnded,0)

IDLC_END_INTF(ni,iDampedSpringPosition2)

/** interface : iDampedSpringPosition3 **/
IDLC_BEGIN_INTF(ni,iDampedSpringPosition3)
/** ni -> iDampedSpringPosition3::SetIdealPosition/1 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition3,SetIdealPosition,1)
	IDLC_DECL_VAR(sVec3f,avPos)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,avPos)
	IDLC_METH_CALL_VOID(ni,iDampedSpringPosition3,SetIdealPosition,1,(avPos))
IDLC_METH_END(ni,iDampedSpringPosition3,SetIdealPosition,1)

/** ni -> iDampedSpringPosition3::GetIdealPosition/0 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition3,GetIdealPosition,0)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition3,GetIdealPosition,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition3,GetIdealPosition,0)

/** ni -> iDampedSpringPosition3::SetCurrentPosition/1 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition3,SetCurrentPosition,1)
	IDLC_DECL_VAR(sVec3f,avPos)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,avPos)
	IDLC_METH_CALL_VOID(ni,iDampedSpringPosition3,SetCurrentPosition,1,(avPos))
IDLC_METH_END(ni,iDampedSpringPosition3,SetCurrentPosition,1)

/** ni -> iDampedSpringPosition3::GetCurrentPosition/0 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition3,GetCurrentPosition,0)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition3,GetCurrentPosition,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition3,GetCurrentPosition,0)

/** ni -> iDampedSpringPosition3::UpdatePosition/1 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition3,UpdatePosition,1)
	IDLC_DECL_VAR(ni::tF32,afDeltaTime)
	IDLC_BUF_TO_BASE(ni::eType_F32,afDeltaTime)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition3,UpdatePosition,1,(afDeltaTime))
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition3,UpdatePosition,1)

/** ni -> iDampedSpringPosition3::SetStep/1 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition3,SetStep,1)
	IDLC_DECL_VAR(tF32,afD)
	IDLC_BUF_TO_BASE(ni::eType_F32,afD)
	IDLC_METH_CALL_VOID(ni,iDampedSpringPosition3,SetStep,1,(afD))
IDLC_METH_END(ni,iDampedSpringPosition3,SetStep,1)

/** ni -> iDampedSpringPosition3::GetStep/0 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition3,GetStep,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition3,GetStep,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition3,GetStep,0)

/** ni -> iDampedSpringPosition3::SetSpeed/1 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition3,SetSpeed,1)
	IDLC_DECL_VAR(tF32,afD)
	IDLC_BUF_TO_BASE(ni::eType_F32,afD)
	IDLC_METH_CALL_VOID(ni,iDampedSpringPosition3,SetSpeed,1,(afD))
IDLC_METH_END(ni,iDampedSpringPosition3,SetSpeed,1)

/** ni -> iDampedSpringPosition3::GetSpeed/0 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition3,GetSpeed,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition3,GetSpeed,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition3,GetSpeed,0)

/** ni -> iDampedSpringPosition3::SetEndThreshold/1 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition3,SetEndThreshold,1)
	IDLC_DECL_VAR(tF32,afD)
	IDLC_BUF_TO_BASE(ni::eType_F32,afD)
	IDLC_METH_CALL_VOID(ni,iDampedSpringPosition3,SetEndThreshold,1,(afD))
IDLC_METH_END(ni,iDampedSpringPosition3,SetEndThreshold,1)

/** ni -> iDampedSpringPosition3::GetEndThreshold/0 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition3,GetEndThreshold,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition3,GetEndThreshold,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition3,GetEndThreshold,0)

/** ni -> iDampedSpringPosition3::GetIsEnded/0 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition3,GetIsEnded,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition3,GetIsEnded,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition3,GetIsEnded,0)

IDLC_END_INTF(ni,iDampedSpringPosition3)

/** interface : iDampedSpringPosition4 **/
IDLC_BEGIN_INTF(ni,iDampedSpringPosition4)
/** ni -> iDampedSpringPosition4::SetIdealPosition/1 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition4,SetIdealPosition,1)
	IDLC_DECL_VAR(sVec4f,avPos)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,avPos)
	IDLC_METH_CALL_VOID(ni,iDampedSpringPosition4,SetIdealPosition,1,(avPos))
IDLC_METH_END(ni,iDampedSpringPosition4,SetIdealPosition,1)

/** ni -> iDampedSpringPosition4::GetIdealPosition/0 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition4,GetIdealPosition,0)
	IDLC_DECL_RETVAR(sVec4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition4,GetIdealPosition,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition4,GetIdealPosition,0)

/** ni -> iDampedSpringPosition4::SetCurrentPosition/1 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition4,SetCurrentPosition,1)
	IDLC_DECL_VAR(sVec4f,avPos)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,avPos)
	IDLC_METH_CALL_VOID(ni,iDampedSpringPosition4,SetCurrentPosition,1,(avPos))
IDLC_METH_END(ni,iDampedSpringPosition4,SetCurrentPosition,1)

/** ni -> iDampedSpringPosition4::GetCurrentPosition/0 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition4,GetCurrentPosition,0)
	IDLC_DECL_RETVAR(sVec4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition4,GetCurrentPosition,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition4,GetCurrentPosition,0)

/** ni -> iDampedSpringPosition4::UpdatePosition/1 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition4,UpdatePosition,1)
	IDLC_DECL_VAR(ni::tF32,afDeltaTime)
	IDLC_BUF_TO_BASE(ni::eType_F32,afDeltaTime)
	IDLC_DECL_RETVAR(sVec4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition4,UpdatePosition,1,(afDeltaTime))
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition4,UpdatePosition,1)

/** ni -> iDampedSpringPosition4::SetStep/1 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition4,SetStep,1)
	IDLC_DECL_VAR(tF32,afD)
	IDLC_BUF_TO_BASE(ni::eType_F32,afD)
	IDLC_METH_CALL_VOID(ni,iDampedSpringPosition4,SetStep,1,(afD))
IDLC_METH_END(ni,iDampedSpringPosition4,SetStep,1)

/** ni -> iDampedSpringPosition4::GetStep/0 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition4,GetStep,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition4,GetStep,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition4,GetStep,0)

/** ni -> iDampedSpringPosition4::SetSpeed/1 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition4,SetSpeed,1)
	IDLC_DECL_VAR(tF32,afD)
	IDLC_BUF_TO_BASE(ni::eType_F32,afD)
	IDLC_METH_CALL_VOID(ni,iDampedSpringPosition4,SetSpeed,1,(afD))
IDLC_METH_END(ni,iDampedSpringPosition4,SetSpeed,1)

/** ni -> iDampedSpringPosition4::GetSpeed/0 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition4,GetSpeed,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition4,GetSpeed,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition4,GetSpeed,0)

/** ni -> iDampedSpringPosition4::SetEndThreshold/1 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition4,SetEndThreshold,1)
	IDLC_DECL_VAR(tF32,afD)
	IDLC_BUF_TO_BASE(ni::eType_F32,afD)
	IDLC_METH_CALL_VOID(ni,iDampedSpringPosition4,SetEndThreshold,1,(afD))
IDLC_METH_END(ni,iDampedSpringPosition4,SetEndThreshold,1)

/** ni -> iDampedSpringPosition4::GetEndThreshold/0 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition4,GetEndThreshold,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition4,GetEndThreshold,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition4,GetEndThreshold,0)

/** ni -> iDampedSpringPosition4::GetIsEnded/0 **/
IDLC_METH_BEGIN(ni,iDampedSpringPosition4,GetIsEnded,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDampedSpringPosition4,GetIsEnded,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDampedSpringPosition4,GetIsEnded,0)

IDLC_END_INTF(ni,iDampedSpringPosition4)

IDLC_END_NAMESPACE()
// EOF //
