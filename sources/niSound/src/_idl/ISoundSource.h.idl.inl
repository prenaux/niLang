#include "../API/niSound/ISoundSource.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iSoundSource **/
IDLC_BEGIN_INTF(ni,iSoundSource)
/** ni -> iSoundSource::SetSoundBuffer/1 **/
IDLC_METH_BEGIN(ni,iSoundSource,SetSoundBuffer,1)
	IDLC_DECL_VAR(iSoundBuffer*,apBuffer)
	IDLC_BUF_TO_INTF(iSoundBuffer,apBuffer)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundSource,SetSoundBuffer,1,(apBuffer))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iSoundSource,SetSoundBuffer,1)

/** ni -> iSoundSource::GetSoundBuffer/0 **/
IDLC_METH_BEGIN(ni,iSoundSource,GetSoundBuffer,0)
	IDLC_DECL_RETVAR(iSoundBuffer*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundSource,GetSoundBuffer,0,())
	IDLC_RET_FROM_INTF(iSoundBuffer,_Ret)
IDLC_METH_END(ni,iSoundSource,GetSoundBuffer,0)

/** ni -> iSoundSource::SetMode/1 **/
IDLC_METH_BEGIN(ni,iSoundSource,SetMode,1)
	IDLC_DECL_VAR(eSoundMode,aMode)
	IDLC_BUF_TO_ENUM(eSoundMode,aMode)
	IDLC_METH_CALL_VOID(ni,iSoundSource,SetMode,1,(aMode))
IDLC_METH_END(ni,iSoundSource,SetMode,1)

/** ni -> iSoundSource::GetMode/0 **/
IDLC_METH_BEGIN(ni,iSoundSource,GetMode,0)
	IDLC_DECL_RETVAR(eSoundMode,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundSource,GetMode,0,())
	IDLC_RET_FROM_ENUM(eSoundMode,_Ret)
IDLC_METH_END(ni,iSoundSource,GetMode,0)

/** ni -> iSoundSource::SetPosition/1 **/
IDLC_METH_BEGIN(ni,iSoundSource,SetPosition,1)
	IDLC_DECL_VAR(sVec3f,avPos)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,avPos)
	IDLC_METH_CALL_VOID(ni,iSoundSource,SetPosition,1,(avPos))
IDLC_METH_END(ni,iSoundSource,SetPosition,1)

/** ni -> iSoundSource::GetPosition/0 **/
IDLC_METH_BEGIN(ni,iSoundSource,GetPosition,0)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundSource,GetPosition,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iSoundSource,GetPosition,0)

/** ni -> iSoundSource::SetVelocity/1 **/
IDLC_METH_BEGIN(ni,iSoundSource,SetVelocity,1)
	IDLC_DECL_VAR(sVec3f,avVel)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,avVel)
	IDLC_METH_CALL_VOID(ni,iSoundSource,SetVelocity,1,(avVel))
IDLC_METH_END(ni,iSoundSource,SetVelocity,1)

/** ni -> iSoundSource::GetVelocity/0 **/
IDLC_METH_BEGIN(ni,iSoundSource,GetVelocity,0)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundSource,GetVelocity,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iSoundSource,GetVelocity,0)

/** ni -> iSoundSource::SetMinDistance/1 **/
IDLC_METH_BEGIN(ni,iSoundSource,SetMinDistance,1)
	IDLC_DECL_VAR(tF32,fMin)
	IDLC_BUF_TO_BASE(ni::eType_F32,fMin)
	IDLC_METH_CALL_VOID(ni,iSoundSource,SetMinDistance,1,(fMin))
IDLC_METH_END(ni,iSoundSource,SetMinDistance,1)

/** ni -> iSoundSource::GetMinDistance/0 **/
IDLC_METH_BEGIN(ni,iSoundSource,GetMinDistance,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundSource,GetMinDistance,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iSoundSource,GetMinDistance,0)

/** ni -> iSoundSource::SetMaxDistance/1 **/
IDLC_METH_BEGIN(ni,iSoundSource,SetMaxDistance,1)
	IDLC_DECL_VAR(tF32,fMax)
	IDLC_BUF_TO_BASE(ni::eType_F32,fMax)
	IDLC_METH_CALL_VOID(ni,iSoundSource,SetMaxDistance,1,(fMax))
IDLC_METH_END(ni,iSoundSource,SetMaxDistance,1)

/** ni -> iSoundSource::GetMaxDistance/0 **/
IDLC_METH_BEGIN(ni,iSoundSource,GetMaxDistance,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundSource,GetMaxDistance,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iSoundSource,GetMaxDistance,0)

/** ni -> iSoundSource::SetPan/1 **/
IDLC_METH_BEGIN(ni,iSoundSource,SetPan,1)
	IDLC_DECL_VAR(tF32,afPanning)
	IDLC_BUF_TO_BASE(ni::eType_F32,afPanning)
	IDLC_METH_CALL_VOID(ni,iSoundSource,SetPan,1,(afPanning))
IDLC_METH_END(ni,iSoundSource,SetPan,1)

/** ni -> iSoundSource::GetPan/0 **/
IDLC_METH_BEGIN(ni,iSoundSource,GetPan,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundSource,GetPan,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iSoundSource,GetPan,0)

/** ni -> iSoundSource::SetVolume/1 **/
IDLC_METH_BEGIN(ni,iSoundSource,SetVolume,1)
	IDLC_DECL_VAR(tF32,afVolume)
	IDLC_BUF_TO_BASE(ni::eType_F32,afVolume)
	IDLC_METH_CALL_VOID(ni,iSoundSource,SetVolume,1,(afVolume))
IDLC_METH_END(ni,iSoundSource,SetVolume,1)

/** ni -> iSoundSource::GetVolume/0 **/
IDLC_METH_BEGIN(ni,iSoundSource,GetVolume,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundSource,GetVolume,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iSoundSource,GetVolume,0)

/** ni -> iSoundSource::SetSpeed/1 **/
IDLC_METH_BEGIN(ni,iSoundSource,SetSpeed,1)
	IDLC_DECL_VAR(tF32,afPitch)
	IDLC_BUF_TO_BASE(ni::eType_F32,afPitch)
	IDLC_METH_CALL_VOID(ni,iSoundSource,SetSpeed,1,(afPitch))
IDLC_METH_END(ni,iSoundSource,SetSpeed,1)

/** ni -> iSoundSource::GetSpeed/0 **/
IDLC_METH_BEGIN(ni,iSoundSource,GetSpeed,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundSource,GetSpeed,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iSoundSource,GetSpeed,0)

/** ni -> iSoundSource::SetLoop/1 **/
IDLC_METH_BEGIN(ni,iSoundSource,SetLoop,1)
	IDLC_DECL_VAR(tBool,abLoop)
	IDLC_BUF_TO_BASE(ni::eType_I8,abLoop)
	IDLC_METH_CALL_VOID(ni,iSoundSource,SetLoop,1,(abLoop))
IDLC_METH_END(ni,iSoundSource,SetLoop,1)

/** ni -> iSoundSource::GetLoop/0 **/
IDLC_METH_BEGIN(ni,iSoundSource,GetLoop,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundSource,GetLoop,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iSoundSource,GetLoop,0)

/** ni -> iSoundSource::Play/0 **/
IDLC_METH_BEGIN(ni,iSoundSource,Play,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundSource,Play,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iSoundSource,Play,0)

/** ni -> iSoundSource::Stop/0 **/
IDLC_METH_BEGIN(ni,iSoundSource,Stop,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundSource,Stop,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iSoundSource,Stop,0)

/** ni -> iSoundSource::Pause/0 **/
IDLC_METH_BEGIN(ni,iSoundSource,Pause,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundSource,Pause,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iSoundSource,Pause,0)

/** ni -> iSoundSource::GetIsPlaying/0 **/
IDLC_METH_BEGIN(ni,iSoundSource,GetIsPlaying,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundSource,GetIsPlaying,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iSoundSource,GetIsPlaying,0)

/** ni -> iSoundSource::GetIsPaused/0 **/
IDLC_METH_BEGIN(ni,iSoundSource,GetIsPaused,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundSource,GetIsPaused,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iSoundSource,GetIsPaused,0)

/** ni -> iSoundSource::Play2D/3 **/
IDLC_METH_BEGIN(ni,iSoundSource,Play2D,3)
	IDLC_DECL_VAR(ni::tF32,afVolume)
	IDLC_BUF_TO_BASE(ni::eType_F32,afVolume)
	IDLC_DECL_VAR(ni::tF32,afSpeed)
	IDLC_BUF_TO_BASE(ni::eType_F32,afSpeed)
	IDLC_DECL_VAR(ni::tF32,afPan)
	IDLC_BUF_TO_BASE(ni::eType_F32,afPan)
	IDLC_DECL_RETVAR(ni::tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundSource,Play2D,3,(afVolume,afSpeed,afPan))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iSoundSource,Play2D,3)

/** ni -> iSoundSource::Play3D/6 **/
IDLC_METH_BEGIN(ni,iSoundSource,Play3D,6)
	IDLC_DECL_VAR(ni::tF32,afVolume)
	IDLC_BUF_TO_BASE(ni::eType_F32,afVolume)
	IDLC_DECL_VAR(ni::tF32,afSpeed)
	IDLC_BUF_TO_BASE(ni::eType_F32,afSpeed)
	IDLC_DECL_VAR(ni::sVec3f,avPosition)
	IDLC_BUF_TO_BASE(ni::eType_Vec3f,avPosition)
	IDLC_DECL_VAR(ni::tBool,abListenerRelative)
	IDLC_BUF_TO_BASE(ni::eType_I8,abListenerRelative)
	IDLC_DECL_VAR(ni::tF32,afMinDistance)
	IDLC_BUF_TO_BASE(ni::eType_F32,afMinDistance)
	IDLC_DECL_VAR(ni::tF32,afMaxDistance)
	IDLC_BUF_TO_BASE(ni::eType_F32,afMaxDistance)
	IDLC_DECL_RETVAR(ni::tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundSource,Play3D,6,(afVolume,afSpeed,avPosition,abListenerRelative,afMinDistance,afMaxDistance))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iSoundSource,Play3D,6)

IDLC_END_INTF(ni,iSoundSource)

IDLC_END_NAMESPACE()
// EOF //
