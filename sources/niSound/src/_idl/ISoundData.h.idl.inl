#include "../API/niSound/ISoundData.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iSoundData **/
IDLC_BEGIN_INTF(ni,iSoundData)
/** ni -> iSoundData::Clone/0 **/
IDLC_METH_BEGIN(ni,iSoundData,Clone,0)
	IDLC_DECL_RETVAR(iSoundData*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundData,Clone,0,())
	IDLC_RET_FROM_INTF(iSoundData,_Ret)
IDLC_METH_END(ni,iSoundData,Clone,0)

/** ni -> iSoundData::GetFormat/0 **/
IDLC_METH_BEGIN(ni,iSoundData,GetFormat,0)
	IDLC_DECL_RETVAR(eSoundFormat,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundData,GetFormat,0,())
	IDLC_RET_FROM_ENUM(eSoundFormat,_Ret)
IDLC_METH_END(ni,iSoundData,GetFormat,0)

/** ni -> iSoundData::GetFrequency/0 **/
IDLC_METH_BEGIN(ni,iSoundData,GetFrequency,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundData,GetFrequency,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iSoundData,GetFrequency,0)

/** ni -> iSoundData::ReadRaw/2 **/
IDLC_METH_BEGIN(ni,iSoundData,ReadRaw,2)
	IDLC_DECL_VAR(tPtr,apOut)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,apOut)
	IDLC_DECL_VAR(tU32,anSize)
	IDLC_BUF_TO_BASE(ni::eType_U32,anSize)
	IDLC_DECL_RETVAR(tI32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundData,ReadRaw,2,(apOut,anSize))
	IDLC_RET_FROM_BASE(ni::eType_I32,_Ret)
IDLC_METH_END(ni,iSoundData,ReadRaw,2)

/** ni -> iSoundData::Reset/0 **/
IDLC_METH_BEGIN(ni,iSoundData,Reset,0)
	IDLC_METH_CALL_VOID(ni,iSoundData,Reset,0,())
IDLC_METH_END(ni,iSoundData,Reset,0)

/** ni -> iSoundData::GetLength/0 **/
IDLC_METH_BEGIN(ni,iSoundData,GetLength,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundData,GetLength,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iSoundData,GetLength,0)

IDLC_END_INTF(ni,iSoundData)

/** interface : iSoundDataLoader **/
IDLC_BEGIN_INTF(ni,iSoundDataLoader)
/** ni -> iSoundDataLoader::LoadSoundData/1 **/
IDLC_METH_BEGIN(ni,iSoundDataLoader,LoadSoundData,1)
	IDLC_DECL_VAR(iFile*,apFile)
	IDLC_BUF_TO_INTF(iFile,apFile)
	IDLC_DECL_RETVAR(iSoundData*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSoundDataLoader,LoadSoundData,1,(apFile))
	IDLC_RET_FROM_INTF(iSoundData,_Ret)
IDLC_METH_END(ni,iSoundDataLoader,LoadSoundData,1)

IDLC_END_INTF(ni,iSoundDataLoader)

IDLC_END_NAMESPACE()
// EOF //
