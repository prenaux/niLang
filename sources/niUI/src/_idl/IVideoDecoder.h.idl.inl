// clang-format off
//
// Autogenerated IDL definition for 'API/niUI/IVideoDecoder.h'.
//
#include "../API/niUI/IVideoDecoder.h"

#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif

IDLC_BEGIN_NAMESPACE()

/** NAMESPACE : ni **/
/** interface : iVideoDecoder **/
IDLC_BEGIN_INTF(ni,iVideoDecoder)
/** ni -> iVideoDecoder::GetVideoDecoderName/0 **/
IDLC_METH_BEGIN(ni,iVideoDecoder,GetVideoDecoderName,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVideoDecoder,GetVideoDecoderName,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iVideoDecoder,GetVideoDecoderName,0)

/** ni -> iVideoDecoder::GetVideoFps/0 **/
IDLC_METH_BEGIN(ni,iVideoDecoder,GetVideoFps,0)
	IDLC_DECL_RETVAR(tF64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVideoDecoder,GetVideoFps,0,())
	IDLC_RET_FROM_BASE(ni::eType_F64,_Ret)
IDLC_METH_END(ni,iVideoDecoder,GetVideoFps,0)

/** ni -> iVideoDecoder::GetFlags/0 **/
IDLC_METH_BEGIN(ni,iVideoDecoder,GetFlags,0)
	IDLC_DECL_RETVAR(tVideoDecoderFlags,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVideoDecoder,GetFlags,0,())
	IDLC_RET_FROM_ENUM(tVideoDecoderFlags,_Ret)
IDLC_METH_END(ni,iVideoDecoder,GetFlags,0)

/** ni -> iVideoDecoder::GetLength/0 **/
IDLC_METH_BEGIN(ni,iVideoDecoder,GetLength,0)
	IDLC_DECL_RETVAR(tF64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVideoDecoder,GetLength,0,())
	IDLC_RET_FROM_BASE(ni::eType_F64,_Ret)
IDLC_METH_END(ni,iVideoDecoder,GetLength,0)

/** ni -> iVideoDecoder::SetTime/1 **/
IDLC_METH_BEGIN(ni,iVideoDecoder,SetTime,1)
	IDLC_DECL_VAR(tF64,afTime)
	IDLC_BUF_TO_BASE(ni::eType_F64,afTime)
	IDLC_METH_CALL_VOID(ni,iVideoDecoder,SetTime,1,(afTime))
IDLC_METH_END(ni,iVideoDecoder,SetTime,1)

/** ni -> iVideoDecoder::GetTime/0 **/
IDLC_METH_BEGIN(ni,iVideoDecoder,GetTime,0)
	IDLC_DECL_RETVAR(tF64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVideoDecoder,GetTime,0,())
	IDLC_RET_FROM_BASE(ni::eType_F64,_Ret)
IDLC_METH_END(ni,iVideoDecoder,GetTime,0)

/** ni -> iVideoDecoder::SetPause/1 **/
IDLC_METH_BEGIN(ni,iVideoDecoder,SetPause,1)
	IDLC_DECL_VAR(tBool,abPause)
	IDLC_BUF_TO_BASE(ni::eType_I8,abPause)
	IDLC_METH_CALL_VOID(ni,iVideoDecoder,SetPause,1,(abPause))
IDLC_METH_END(ni,iVideoDecoder,SetPause,1)

/** ni -> iVideoDecoder::GetPause/0 **/
IDLC_METH_BEGIN(ni,iVideoDecoder,GetPause,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVideoDecoder,GetPause,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVideoDecoder,GetPause,0)

/** ni -> iVideoDecoder::SetSpeed/1 **/
IDLC_METH_BEGIN(ni,iVideoDecoder,SetSpeed,1)
	IDLC_DECL_VAR(tF32,afSpeed)
	IDLC_BUF_TO_BASE(ni::eType_F32,afSpeed)
	IDLC_METH_CALL_VOID(ni,iVideoDecoder,SetSpeed,1,(afSpeed))
IDLC_METH_END(ni,iVideoDecoder,SetSpeed,1)

/** ni -> iVideoDecoder::GetSpeed/0 **/
IDLC_METH_BEGIN(ni,iVideoDecoder,GetSpeed,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVideoDecoder,GetSpeed,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iVideoDecoder,GetSpeed,0)

/** ni -> iVideoDecoder::SetNumLoops/1 **/
IDLC_METH_BEGIN(ni,iVideoDecoder,SetNumLoops,1)
	IDLC_DECL_VAR(ni::tU32,anLoop)
	IDLC_BUF_TO_BASE(ni::eType_U32,anLoop)
	IDLC_METH_CALL_VOID(ni,iVideoDecoder,SetNumLoops,1,(anLoop))
IDLC_METH_END(ni,iVideoDecoder,SetNumLoops,1)

/** ni -> iVideoDecoder::GetNumLoops/0 **/
IDLC_METH_BEGIN(ni,iVideoDecoder,GetNumLoops,0)
	IDLC_DECL_RETVAR(ni::tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVideoDecoder,GetNumLoops,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iVideoDecoder,GetNumLoops,0)

/** ni -> iVideoDecoder::Update/2 **/
IDLC_METH_BEGIN(ni,iVideoDecoder,Update,2)
	IDLC_DECL_VAR(tBool,abUpdateTarget)
	IDLC_BUF_TO_BASE(ni::eType_I8,abUpdateTarget)
	IDLC_DECL_VAR(tF32,afFrameTime)
	IDLC_BUF_TO_BASE(ni::eType_F32,afFrameTime)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVideoDecoder,Update,2,(abUpdateTarget,afFrameTime))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVideoDecoder,Update,2)

/** ni -> iVideoDecoder::GetTargetTexture/0 **/
IDLC_METH_BEGIN(ni,iVideoDecoder,GetTargetTexture,0)
	IDLC_DECL_RETVAR(iTexture*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVideoDecoder,GetTargetTexture,0,())
	IDLC_RET_FROM_INTF(iTexture,_Ret)
IDLC_METH_END(ni,iVideoDecoder,GetTargetTexture,0)

/** ni -> iVideoDecoder::GetTargetBitmap/0 **/
IDLC_METH_BEGIN(ni,iVideoDecoder,GetTargetBitmap,0)
	IDLC_DECL_RETVAR(iBitmap2D*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVideoDecoder,GetTargetBitmap,0,())
	IDLC_RET_FROM_INTF(iBitmap2D,_Ret)
IDLC_METH_END(ni,iVideoDecoder,GetTargetBitmap,0)

/** ni -> iVideoDecoder::GetNumSoundTracks/0 **/
IDLC_METH_BEGIN(ni,iVideoDecoder,GetNumSoundTracks,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVideoDecoder,GetNumSoundTracks,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iVideoDecoder,GetNumSoundTracks,0)

/** ni -> iVideoDecoder::GetSoundTrackData/1 **/
IDLC_METH_BEGIN(ni,iVideoDecoder,GetSoundTrackData,1)
	IDLC_DECL_VAR(tU32,anNumTrack)
	IDLC_BUF_TO_BASE(ni::eType_U32,anNumTrack)
	IDLC_DECL_RETVAR(iUnknown*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVideoDecoder,GetSoundTrackData,1,(anNumTrack))
	IDLC_RET_FROM_INTF(iUnknown,_Ret)
IDLC_METH_END(ni,iVideoDecoder,GetSoundTrackData,1)

/** ni -> iVideoDecoder::SetUpdateOnBind/1 **/
IDLC_METH_BEGIN(ni,iVideoDecoder,SetUpdateOnBind,1)
	IDLC_DECL_VAR(tBool,abUpdateOnBind)
	IDLC_BUF_TO_BASE(ni::eType_I8,abUpdateOnBind)
	IDLC_METH_CALL_VOID(ni,iVideoDecoder,SetUpdateOnBind,1,(abUpdateOnBind))
IDLC_METH_END(ni,iVideoDecoder,SetUpdateOnBind,1)

/** ni -> iVideoDecoder::GetUpdateOnBind/0 **/
IDLC_METH_BEGIN(ni,iVideoDecoder,GetUpdateOnBind,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iVideoDecoder,GetUpdateOnBind,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iVideoDecoder,GetUpdateOnBind,0)

IDLC_END_INTF(ni,iVideoDecoder)

IDLC_END_NAMESPACE()
// EOF //
