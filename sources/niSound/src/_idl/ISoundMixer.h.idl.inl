#include "../API/niSound/ISoundMixer.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iSoundMixer **/
IDLC_BEGIN_INTF(ni,iSoundMixer)
/** ni -> iSoundMixer::SwitchIn/0 **/
IDLC_METH_BEGIN(ni,iSoundMixer,SwitchIn,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer,SwitchIn,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iSoundMixer,SwitchIn,0)

/** ni -> iSoundMixer::SwitchOut/0 **/
IDLC_METH_BEGIN(ni,iSoundMixer,SwitchOut,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer,SwitchOut,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iSoundMixer,SwitchOut,0)

/** ni -> iSoundMixer::SetAmplification/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer,SetAmplification,1)
	IDLC_DECL_VAR(tI32,ampli)
	IDLC_BUF_TO_BASE(ni::eType_I32,ampli)
	IDLC_METH_CALL_VOID(ni,iSoundMixer,SetAmplification,1,(ampli))
IDLC_METH_END(ni,iSoundMixer,SetAmplification,1)

/** ni -> iSoundMixer::GetAmplification/0 **/
IDLC_METH_BEGIN(ni,iSoundMixer,GetAmplification,0)
	IDLC_DECL_RETVAR(tI32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer,GetAmplification,0,())
	IDLC_RET_FROM_BASE(ni::eType_I32,_Ret)
IDLC_METH_END(ni,iSoundMixer,GetAmplification,0)

/** ni -> iSoundMixer::SetSaturationCheck/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer,SetSaturationCheck,1)
	IDLC_DECL_VAR(tBool,check)
	IDLC_BUF_TO_BASE(ni::eType_I8,check)
	IDLC_METH_CALL_VOID(ni,iSoundMixer,SetSaturationCheck,1,(check))
IDLC_METH_END(ni,iSoundMixer,SetSaturationCheck,1)

/** ni -> iSoundMixer::GetSaturationCheck/0 **/
IDLC_METH_BEGIN(ni,iSoundMixer,GetSaturationCheck,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer,GetSaturationCheck,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iSoundMixer,GetSaturationCheck,0)

/** ni -> iSoundMixer::SetReverseStereo/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer,SetReverseStereo,1)
	IDLC_DECL_VAR(tBool,reverse)
	IDLC_BUF_TO_BASE(ni::eType_I8,reverse)
	IDLC_METH_CALL_VOID(ni,iSoundMixer,SetReverseStereo,1,(reverse))
IDLC_METH_END(ni,iSoundMixer,SetReverseStereo,1)

/** ni -> iSoundMixer::GetReverseStereo/0 **/
IDLC_METH_BEGIN(ni,iSoundMixer,GetReverseStereo,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer,GetReverseStereo,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iSoundMixer,GetReverseStereo,0)

/** ni -> iSoundMixer::SetMasterVolume/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer,SetMasterVolume,1)
	IDLC_DECL_VAR(tI32,vol)
	IDLC_BUF_TO_BASE(ni::eType_I32,vol)
	IDLC_METH_CALL_VOID(ni,iSoundMixer,SetMasterVolume,1,(vol))
IDLC_METH_END(ni,iSoundMixer,SetMasterVolume,1)

/** ni -> iSoundMixer::GetMasterVolume/0 **/
IDLC_METH_BEGIN(ni,iSoundMixer,GetMasterVolume,0)
	IDLC_DECL_RETVAR(tI32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer,GetMasterVolume,0,())
	IDLC_RET_FROM_BASE(ni::eType_I32,_Ret)
IDLC_METH_END(ni,iSoundMixer,GetMasterVolume,0)

/** ni -> iSoundMixer::GetNumChannels/0 **/
IDLC_METH_BEGIN(ni,iSoundMixer,GetNumChannels,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer,GetNumChannels,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iSoundMixer,GetNumChannels,0)

/** ni -> iSoundMixer::StartChannel/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer,StartChannel,1)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer,StartChannel,1,(nchan))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iSoundMixer,StartChannel,1)

/** ni -> iSoundMixer::StopChannel/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer,StopChannel,1)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer,StopChannel,1,(nchan))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iSoundMixer,StopChannel,1)

/** ni -> iSoundMixer::StopAllChannels/0 **/
IDLC_METH_BEGIN(ni,iSoundMixer,StopAllChannels,0)
	IDLC_METH_CALL_VOID(ni,iSoundMixer,StopAllChannels,0,())
IDLC_METH_END(ni,iSoundMixer,StopAllChannels,0)

/** ni -> iSoundMixer::SetChannelMasterVolume/2 **/
IDLC_METH_BEGIN(ni,iSoundMixer,SetChannelMasterVolume,2)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_VAR(tI32,vol)
	IDLC_BUF_TO_BASE(ni::eType_I32,vol)
	IDLC_METH_CALL_VOID(ni,iSoundMixer,SetChannelMasterVolume,2,(nchan,vol))
IDLC_METH_END(ni,iSoundMixer,SetChannelMasterVolume,2)

/** ni -> iSoundMixer::GetChannelMasterVolume/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer,GetChannelMasterVolume,1)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_RETVAR(tI32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer,GetChannelMasterVolume,1,(nchan))
	IDLC_RET_FROM_BASE(ni::eType_I32,_Ret)
IDLC_METH_END(ni,iSoundMixer,GetChannelMasterVolume,1)

/** ni -> iSoundMixer::SetChannelMasterPan/2 **/
IDLC_METH_BEGIN(ni,iSoundMixer,SetChannelMasterPan,2)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_VAR(tI32,pan)
	IDLC_BUF_TO_BASE(ni::eType_I32,pan)
	IDLC_METH_CALL_VOID(ni,iSoundMixer,SetChannelMasterPan,2,(nchan,pan))
IDLC_METH_END(ni,iSoundMixer,SetChannelMasterPan,2)

/** ni -> iSoundMixer::GetChannelMasterPan/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer,GetChannelMasterPan,1)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_RETVAR(tI32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer,GetChannelMasterPan,1,(nchan))
	IDLC_RET_FROM_BASE(ni::eType_I32,_Ret)
IDLC_METH_END(ni,iSoundMixer,GetChannelMasterPan,1)

/** ni -> iSoundMixer::SetChannelBuffer/2 **/
IDLC_METH_BEGIN(ni,iSoundMixer,SetChannelBuffer,2)
	IDLC_DECL_VAR(tU32,anChan)
	IDLC_BUF_TO_BASE(ni::eType_U32,anChan)
	IDLC_DECL_VAR(iSoundBuffer*,apBuffer)
	IDLC_BUF_TO_INTF(iSoundBuffer,apBuffer)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer,SetChannelBuffer,2,(anChan,apBuffer))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iSoundMixer,SetChannelBuffer,2)

/** ni -> iSoundMixer::GetChannelBuffer/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer,GetChannelBuffer,1)
	IDLC_DECL_VAR(tU32,anChan)
	IDLC_BUF_TO_BASE(ni::eType_U32,anChan)
	IDLC_DECL_RETVAR(iSoundBuffer*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer,GetChannelBuffer,1,(anChan))
	IDLC_RET_FROM_INTF(iSoundBuffer,_Ret)
IDLC_METH_END(ni,iSoundMixer,GetChannelBuffer,1)

/** ni -> iSoundMixer::SetChannelVolume/2 **/
IDLC_METH_BEGIN(ni,iSoundMixer,SetChannelVolume,2)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_VAR(tI32,vol)
	IDLC_BUF_TO_BASE(ni::eType_I32,vol)
	IDLC_METH_CALL_VOID(ni,iSoundMixer,SetChannelVolume,2,(nchan,vol))
IDLC_METH_END(ni,iSoundMixer,SetChannelVolume,2)

/** ni -> iSoundMixer::GetChannelVolume/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer,GetChannelVolume,1)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_RETVAR(tI32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer,GetChannelVolume,1,(nchan))
	IDLC_RET_FROM_BASE(ni::eType_I32,_Ret)
IDLC_METH_END(ni,iSoundMixer,GetChannelVolume,1)

/** ni -> iSoundMixer::SetChannelPan/2 **/
IDLC_METH_BEGIN(ni,iSoundMixer,SetChannelPan,2)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_VAR(tI32,pan)
	IDLC_BUF_TO_BASE(ni::eType_I32,pan)
	IDLC_METH_CALL_VOID(ni,iSoundMixer,SetChannelPan,2,(nchan,pan))
IDLC_METH_END(ni,iSoundMixer,SetChannelPan,2)

/** ni -> iSoundMixer::GetChannelPan/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer,GetChannelPan,1)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_RETVAR(tI32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer,GetChannelPan,1,(nchan))
	IDLC_RET_FROM_BASE(ni::eType_I32,_Ret)
IDLC_METH_END(ni,iSoundMixer,GetChannelPan,1)

/** ni -> iSoundMixer::SetChannelFrequency/2 **/
IDLC_METH_BEGIN(ni,iSoundMixer,SetChannelFrequency,2)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_VAR(tI32,frequency)
	IDLC_BUF_TO_BASE(ni::eType_I32,frequency)
	IDLC_METH_CALL_VOID(ni,iSoundMixer,SetChannelFrequency,2,(nchan,frequency))
IDLC_METH_END(ni,iSoundMixer,SetChannelFrequency,2)

/** ni -> iSoundMixer::GetChannelFrequency/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer,GetChannelFrequency,1)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_RETVAR(tI32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer,GetChannelFrequency,1,(nchan))
	IDLC_RET_FROM_BASE(ni::eType_I32,_Ret)
IDLC_METH_END(ni,iSoundMixer,GetChannelFrequency,1)

/** ni -> iSoundMixer::SetChannelLoop/2 **/
IDLC_METH_BEGIN(ni,iSoundMixer,SetChannelLoop,2)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_VAR(tBool,loop)
	IDLC_BUF_TO_BASE(ni::eType_I8,loop)
	IDLC_METH_CALL_VOID(ni,iSoundMixer,SetChannelLoop,2,(nchan,loop))
IDLC_METH_END(ni,iSoundMixer,SetChannelLoop,2)

/** ni -> iSoundMixer::GetChannelLoop/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer,GetChannelLoop,1)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer,GetChannelLoop,1,(nchan))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iSoundMixer,GetChannelLoop,1)

/** ni -> iSoundMixer::SetChannelPause/2 **/
IDLC_METH_BEGIN(ni,iSoundMixer,SetChannelPause,2)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_VAR(tBool,pause)
	IDLC_BUF_TO_BASE(ni::eType_I8,pause)
	IDLC_METH_CALL_VOID(ni,iSoundMixer,SetChannelPause,2,(nchan,pause))
IDLC_METH_END(ni,iSoundMixer,SetChannelPause,2)

/** ni -> iSoundMixer::GetChannelPause/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer,GetChannelPause,1)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer,GetChannelPause,1,(nchan))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iSoundMixer,GetChannelPause,1)

/** ni -> iSoundMixer::GetIsChannelBusy/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer,GetIsChannelBusy,1)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer,GetIsChannelBusy,1,(nchan))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iSoundMixer,GetIsChannelBusy,1)

/** ni -> iSoundMixer::SetChannelUserID/2 **/
IDLC_METH_BEGIN(ni,iSoundMixer,SetChannelUserID,2)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_VAR(tIntPtr,anUserId)
	IDLC_BUF_TO_BASE(ni::eType_IntPtr,anUserId)
	IDLC_METH_CALL_VOID(ni,iSoundMixer,SetChannelUserID,2,(nchan,anUserId))
IDLC_METH_END(ni,iSoundMixer,SetChannelUserID,2)

/** ni -> iSoundMixer::GetChannelUserID/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer,GetChannelUserID,1)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_RETVAR(tIntPtr,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer,GetChannelUserID,1,(nchan))
	IDLC_RET_FROM_BASE(ni::eType_IntPtr,_Ret)
IDLC_METH_END(ni,iSoundMixer,GetChannelUserID,1)

/** ni -> iSoundMixer::UpdateMixer/0 **/
IDLC_METH_BEGIN(ni,iSoundMixer,UpdateMixer,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer,UpdateMixer,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iSoundMixer,UpdateMixer,0)

IDLC_END_INTF(ni,iSoundMixer)

/** interface : iSoundMixer3D **/
IDLC_BEGIN_INTF(ni,iSoundMixer3D)
/** ni -> iSoundMixer3D::GetFirst3DChannel/0 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,GetFirst3DChannel,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer3D,GetFirst3DChannel,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iSoundMixer3D,GetFirst3DChannel,0)

/** ni -> iSoundMixer3D::GetIsChannel3D/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,GetIsChannel3D,1)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer3D,GetIsChannel3D,1,(nchan))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iSoundMixer3D,GetIsChannel3D,1)

/** ni -> iSoundMixer3D::SetChannelMode/2 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,SetChannelMode,2)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_VAR(eSoundMode,aMode)
	IDLC_BUF_TO_ENUM(eSoundMode,aMode)
	IDLC_METH_CALL_VOID(ni,iSoundMixer3D,SetChannelMode,2,(nchan,aMode))
IDLC_METH_END(ni,iSoundMixer3D,SetChannelMode,2)

/** ni -> iSoundMixer3D::GetChannelMode/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,GetChannelMode,1)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_RETVAR(eSoundMode,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer3D,GetChannelMode,1,(nchan))
	IDLC_RET_FROM_ENUM(eSoundMode,_Ret)
IDLC_METH_END(ni,iSoundMixer3D,GetChannelMode,1)

/** ni -> iSoundMixer3D::SetChannelPosition/2 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,SetChannelPosition,2)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_VAR(sVec3f,avPosition)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,avPosition)
	IDLC_METH_CALL_VOID(ni,iSoundMixer3D,SetChannelPosition,2,(nchan,avPosition))
IDLC_METH_END(ni,iSoundMixer3D,SetChannelPosition,2)

/** ni -> iSoundMixer3D::GetChannelPosition/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,GetChannelPosition,1)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer3D,GetChannelPosition,1,(nchan))
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iSoundMixer3D,GetChannelPosition,1)

/** ni -> iSoundMixer3D::SetChannelVelocity/2 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,SetChannelVelocity,2)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_VAR(sVec3f,avVelocity)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,avVelocity)
	IDLC_METH_CALL_VOID(ni,iSoundMixer3D,SetChannelVelocity,2,(nchan,avVelocity))
IDLC_METH_END(ni,iSoundMixer3D,SetChannelVelocity,2)

/** ni -> iSoundMixer3D::GetChannelVelocity/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,GetChannelVelocity,1)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer3D,GetChannelVelocity,1,(nchan))
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iSoundMixer3D,GetChannelVelocity,1)

/** ni -> iSoundMixer3D::SetChannelMinDistance/2 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,SetChannelMinDistance,2)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_VAR(tF32,afMinDistance)
	IDLC_BUF_TO_BASE(ni::eType_F32,afMinDistance)
	IDLC_METH_CALL_VOID(ni,iSoundMixer3D,SetChannelMinDistance,2,(nchan,afMinDistance))
IDLC_METH_END(ni,iSoundMixer3D,SetChannelMinDistance,2)

/** ni -> iSoundMixer3D::GetChannelMinDistance/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,GetChannelMinDistance,1)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer3D,GetChannelMinDistance,1,(nchan))
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iSoundMixer3D,GetChannelMinDistance,1)

/** ni -> iSoundMixer3D::SetChannelMaxDistance/2 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,SetChannelMaxDistance,2)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_VAR(tF32,afMaxDistance)
	IDLC_BUF_TO_BASE(ni::eType_F32,afMaxDistance)
	IDLC_METH_CALL_VOID(ni,iSoundMixer3D,SetChannelMaxDistance,2,(nchan,afMaxDistance))
IDLC_METH_END(ni,iSoundMixer3D,SetChannelMaxDistance,2)

/** ni -> iSoundMixer3D::GetChannelMaxDistance/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,GetChannelMaxDistance,1)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer3D,GetChannelMaxDistance,1,(nchan))
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iSoundMixer3D,GetChannelMaxDistance,1)

/** ni -> iSoundMixer3D::SetChannelConeInner/2 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,SetChannelConeInner,2)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_VAR(tF32,afInsideAngle)
	IDLC_BUF_TO_BASE(ni::eType_F32,afInsideAngle)
	IDLC_METH_CALL_VOID(ni,iSoundMixer3D,SetChannelConeInner,2,(nchan,afInsideAngle))
IDLC_METH_END(ni,iSoundMixer3D,SetChannelConeInner,2)

/** ni -> iSoundMixer3D::GetChannelConeInner/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,GetChannelConeInner,1)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer3D,GetChannelConeInner,1,(nchan))
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iSoundMixer3D,GetChannelConeInner,1)

/** ni -> iSoundMixer3D::SetChannelConeOuter/2 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,SetChannelConeOuter,2)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_VAR(tF32,afOutsideAngle)
	IDLC_BUF_TO_BASE(ni::eType_F32,afOutsideAngle)
	IDLC_METH_CALL_VOID(ni,iSoundMixer3D,SetChannelConeOuter,2,(nchan,afOutsideAngle))
IDLC_METH_END(ni,iSoundMixer3D,SetChannelConeOuter,2)

/** ni -> iSoundMixer3D::GetChannelConeOuter/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,GetChannelConeOuter,1)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer3D,GetChannelConeOuter,1,(nchan))
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iSoundMixer3D,GetChannelConeOuter,1)

/** ni -> iSoundMixer3D::SetChannelConeOuterVolume/2 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,SetChannelConeOuterVolume,2)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_VAR(tU32,anOuterVolume)
	IDLC_BUF_TO_BASE(ni::eType_U32,anOuterVolume)
	IDLC_METH_CALL_VOID(ni,iSoundMixer3D,SetChannelConeOuterVolume,2,(nchan,anOuterVolume))
IDLC_METH_END(ni,iSoundMixer3D,SetChannelConeOuterVolume,2)

/** ni -> iSoundMixer3D::GetChannelConeOuterVolume/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,GetChannelConeOuterVolume,1)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer3D,GetChannelConeOuterVolume,1,(nchan))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iSoundMixer3D,GetChannelConeOuterVolume,1)

/** ni -> iSoundMixer3D::SetChannelConeDirection/2 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,SetChannelConeDirection,2)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_VAR(sVec3f,avDir)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,avDir)
	IDLC_METH_CALL_VOID(ni,iSoundMixer3D,SetChannelConeDirection,2,(nchan,avDir))
IDLC_METH_END(ni,iSoundMixer3D,SetChannelConeDirection,2)

/** ni -> iSoundMixer3D::GetChannelConeDirection/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,GetChannelConeDirection,1)
	IDLC_DECL_VAR(tU32,nchan)
	IDLC_BUF_TO_BASE(ni::eType_U32,nchan)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer3D,GetChannelConeDirection,1,(nchan))
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iSoundMixer3D,GetChannelConeDirection,1)

/** ni -> iSoundMixer3D::SetListenerPosition/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,SetListenerPosition,1)
	IDLC_DECL_VAR(sVec3f,aV)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,aV)
	IDLC_METH_CALL_VOID(ni,iSoundMixer3D,SetListenerPosition,1,(aV))
IDLC_METH_END(ni,iSoundMixer3D,SetListenerPosition,1)

/** ni -> iSoundMixer3D::GetListenerPosition/0 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,GetListenerPosition,0)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer3D,GetListenerPosition,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iSoundMixer3D,GetListenerPosition,0)

/** ni -> iSoundMixer3D::SetListenerVelocity/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,SetListenerVelocity,1)
	IDLC_DECL_VAR(sVec3f,aV)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,aV)
	IDLC_METH_CALL_VOID(ni,iSoundMixer3D,SetListenerVelocity,1,(aV))
IDLC_METH_END(ni,iSoundMixer3D,SetListenerVelocity,1)

/** ni -> iSoundMixer3D::GetListenerVelocity/0 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,GetListenerVelocity,0)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer3D,GetListenerVelocity,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iSoundMixer3D,GetListenerVelocity,0)

/** ni -> iSoundMixer3D::SetListenerForward/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,SetListenerForward,1)
	IDLC_DECL_VAR(sVec3f,aV)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,aV)
	IDLC_METH_CALL_VOID(ni,iSoundMixer3D,SetListenerForward,1,(aV))
IDLC_METH_END(ni,iSoundMixer3D,SetListenerForward,1)

/** ni -> iSoundMixer3D::GetListenerForward/0 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,GetListenerForward,0)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer3D,GetListenerForward,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iSoundMixer3D,GetListenerForward,0)

/** ni -> iSoundMixer3D::SetListenerUp/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,SetListenerUp,1)
	IDLC_DECL_VAR(sVec3f,aV)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,aV)
	IDLC_METH_CALL_VOID(ni,iSoundMixer3D,SetListenerUp,1,(aV))
IDLC_METH_END(ni,iSoundMixer3D,SetListenerUp,1)

/** ni -> iSoundMixer3D::GetListenerUp/0 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,GetListenerUp,0)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer3D,GetListenerUp,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iSoundMixer3D,GetListenerUp,0)

/** ni -> iSoundMixer3D::SetListenerDistanceScale/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,SetListenerDistanceScale,1)
	IDLC_DECL_VAR(tF32,afScale)
	IDLC_BUF_TO_BASE(ni::eType_F32,afScale)
	IDLC_METH_CALL_VOID(ni,iSoundMixer3D,SetListenerDistanceScale,1,(afScale))
IDLC_METH_END(ni,iSoundMixer3D,SetListenerDistanceScale,1)

/** ni -> iSoundMixer3D::GetListenerDistanceScale/0 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,GetListenerDistanceScale,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer3D,GetListenerDistanceScale,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iSoundMixer3D,GetListenerDistanceScale,0)

/** ni -> iSoundMixer3D::SetListenerRolloffScale/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,SetListenerRolloffScale,1)
	IDLC_DECL_VAR(tF32,afScale)
	IDLC_BUF_TO_BASE(ni::eType_F32,afScale)
	IDLC_METH_CALL_VOID(ni,iSoundMixer3D,SetListenerRolloffScale,1,(afScale))
IDLC_METH_END(ni,iSoundMixer3D,SetListenerRolloffScale,1)

/** ni -> iSoundMixer3D::GetListenerRolloffScale/0 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,GetListenerRolloffScale,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer3D,GetListenerRolloffScale,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iSoundMixer3D,GetListenerRolloffScale,0)

/** ni -> iSoundMixer3D::SetListenerDopplerScale/1 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,SetListenerDopplerScale,1)
	IDLC_DECL_VAR(tF32,afScale)
	IDLC_BUF_TO_BASE(ni::eType_F32,afScale)
	IDLC_METH_CALL_VOID(ni,iSoundMixer3D,SetListenerDopplerScale,1,(afScale))
IDLC_METH_END(ni,iSoundMixer3D,SetListenerDopplerScale,1)

/** ni -> iSoundMixer3D::GetListenerDopplerScale/0 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,GetListenerDopplerScale,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer3D,GetListenerDopplerScale,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iSoundMixer3D,GetListenerDopplerScale,0)

/** ni -> iSoundMixer3D::UpdateMixer3D/0 **/
IDLC_METH_BEGIN(ni,iSoundMixer3D,UpdateMixer3D,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundMixer3D,UpdateMixer3D,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iSoundMixer3D,UpdateMixer3D,0)

IDLC_END_INTF(ni,iSoundMixer3D)

IDLC_END_NAMESPACE()
// EOF //
