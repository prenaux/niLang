#include "../API/niLang/IHString.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iHString **/
IDLC_BEGIN_INTF(ni,iHString)
/** ni -> iHString::GetChars/0 **/
IDLC_METH_BEGIN(ni,iHString,GetChars,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHString,GetChars,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iHString,GetChars,0)

/** ni -> iHString::GetLength/0 **/
IDLC_METH_BEGIN(ni,iHString,GetLength,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHString,GetLength,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iHString,GetLength,0)

/** ni -> iHString::Cmp/1 **/
IDLC_METH_BEGIN(ni,iHString,Cmp,1)
	IDLC_DECL_VAR(iHString*,ahspString)
	IDLC_BUF_TO_INTF(iHString,ahspString)
	IDLC_DECL_RETVAR(tI32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHString,Cmp,1,(ahspString))
	IDLC_RET_FROM_BASE(ni::eType_I32,_Ret)
IDLC_METH_END(ni,iHString,Cmp,1)

/** ni -> iHString::ICmp/1 **/
IDLC_METH_BEGIN(ni,iHString,ICmp,1)
	IDLC_DECL_VAR(iHString*,ahspString)
	IDLC_BUF_TO_INTF(iHString,ahspString)
	IDLC_DECL_RETVAR(tI32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHString,ICmp,1,(ahspString))
	IDLC_RET_FROM_BASE(ni::eType_I32,_Ret)
IDLC_METH_END(ni,iHString,ICmp,1)

/** ni -> iHString::GetLocalized/0 **/
IDLC_METH_BEGIN(ni,iHString,GetLocalized,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHString,GetLocalized,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iHString,GetLocalized,0)

/** ni -> iHString::GetLocalizedEx/1 **/
IDLC_METH_BEGIN(ni,iHString,GetLocalizedEx,1)
	IDLC_DECL_VAR(iHString*,locale)
	IDLC_BUF_TO_INTF(iHString,locale)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHString,GetLocalizedEx,1,(locale))
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iHString,GetLocalizedEx,1)

/** ni -> iHString::IsLocalized/1 **/
IDLC_METH_BEGIN(ni,iHString,IsLocalized,1)
	IDLC_DECL_VAR(iHString*,locale)
	IDLC_BUF_TO_INTF(iHString,locale)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHString,IsLocalized,1,(locale))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iHString,IsLocalized,1)

/** ni -> iHString::CreateCharIt/1 **/
IDLC_METH_BEGIN(ni,iHString,CreateCharIt,1)
	IDLC_DECL_VAR(tU32,offset)
	IDLC_BUF_TO_BASE(ni::eType_U32,offset)
	IDLC_DECL_RETVAR(iHStringCharIt*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHString,CreateCharIt,1,(offset))
	IDLC_RET_FROM_INTF(iHStringCharIt,_Ret)
IDLC_METH_END(ni,iHString,CreateCharIt,1)

/** ni -> iHString::CreateRangeIt/2 **/
IDLC_METH_BEGIN(ni,iHString,CreateRangeIt,2)
	IDLC_DECL_VAR(tU32,offset)
	IDLC_BUF_TO_BASE(ni::eType_U32,offset)
	IDLC_DECL_VAR(tU32,size)
	IDLC_BUF_TO_BASE(ni::eType_U32,size)
	IDLC_DECL_RETVAR(iHStringCharIt*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHString,CreateRangeIt,2,(offset,size))
	IDLC_RET_FROM_INTF(iHStringCharIt,_Ret)
IDLC_METH_END(ni,iHString,CreateRangeIt,2)

IDLC_END_INTF(ni,iHString)

/** interface : iHStringCharIt **/
IDLC_BEGIN_INTF(ni,iHStringCharIt)
/** ni -> iHStringCharIt::GetString/0 **/
IDLC_METH_BEGIN(ni,iHStringCharIt,GetString,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHStringCharIt,GetString,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iHStringCharIt,GetString,0)

/** ni -> iHStringCharIt::Clone/0 **/
IDLC_METH_BEGIN(ni,iHStringCharIt,Clone,0)
	IDLC_DECL_RETVAR(iHStringCharIt*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHStringCharIt,Clone,0,())
	IDLC_RET_FROM_INTF(iHStringCharIt,_Ret)
IDLC_METH_END(ni,iHStringCharIt,Clone,0)

/** ni -> iHStringCharIt::GetIsStart/0 **/
IDLC_METH_BEGIN(ni,iHStringCharIt,GetIsStart,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHStringCharIt,GetIsStart,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iHStringCharIt,GetIsStart,0)

/** ni -> iHStringCharIt::GetIsEnd/0 **/
IDLC_METH_BEGIN(ni,iHStringCharIt,GetIsEnd,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHStringCharIt,GetIsEnd,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iHStringCharIt,GetIsEnd,0)

/** ni -> iHStringCharIt::GetPosition/0 **/
IDLC_METH_BEGIN(ni,iHStringCharIt,GetPosition,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHStringCharIt,GetPosition,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iHStringCharIt,GetPosition,0)

/** ni -> iHStringCharIt::ToStart/0 **/
IDLC_METH_BEGIN(ni,iHStringCharIt,ToStart,0)
	IDLC_METH_CALL_VOID(ni,iHStringCharIt,ToStart,0,())
IDLC_METH_END(ni,iHStringCharIt,ToStart,0)

/** ni -> iHStringCharIt::ToEnd/0 **/
IDLC_METH_BEGIN(ni,iHStringCharIt,ToEnd,0)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHStringCharIt,ToEnd,0,())
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iHStringCharIt,ToEnd,0)

/** ni -> iHStringCharIt::GetNumChars/0 **/
IDLC_METH_BEGIN(ni,iHStringCharIt,GetNumChars,0)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHStringCharIt,GetNumChars,0,())
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iHStringCharIt,GetNumChars,0)

/** ni -> iHStringCharIt::GetNumBytes/0 **/
IDLC_METH_BEGIN(ni,iHStringCharIt,GetNumBytes,0)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHStringCharIt,GetNumBytes,0,())
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iHStringCharIt,GetNumBytes,0)

/** ni -> iHStringCharIt::PeekNext/0 **/
IDLC_METH_BEGIN(ni,iHStringCharIt,PeekNext,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHStringCharIt,PeekNext,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iHStringCharIt,PeekNext,0)

/** ni -> iHStringCharIt::Next/0 **/
IDLC_METH_BEGIN(ni,iHStringCharIt,Next,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHStringCharIt,Next,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iHStringCharIt,Next,0)

/** ni -> iHStringCharIt::PeekPrior/0 **/
IDLC_METH_BEGIN(ni,iHStringCharIt,PeekPrior,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHStringCharIt,PeekPrior,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iHStringCharIt,PeekPrior,0)

/** ni -> iHStringCharIt::Prior/0 **/
IDLC_METH_BEGIN(ni,iHStringCharIt,Prior,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHStringCharIt,Prior,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iHStringCharIt,Prior,0)

/** ni -> iHStringCharIt::PeekAdvance/1 **/
IDLC_METH_BEGIN(ni,iHStringCharIt,PeekAdvance,1)
	IDLC_DECL_VAR(tU32,fwd)
	IDLC_BUF_TO_BASE(ni::eType_U32,fwd)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHStringCharIt,PeekAdvance,1,(fwd))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iHStringCharIt,PeekAdvance,1)

/** ni -> iHStringCharIt::Advance/1 **/
IDLC_METH_BEGIN(ni,iHStringCharIt,Advance,1)
	IDLC_DECL_VAR(tU32,n)
	IDLC_BUF_TO_BASE(ni::eType_U32,n)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHStringCharIt,Advance,1,(n))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iHStringCharIt,Advance,1)

/** ni -> iHStringCharIt::PeekRewind/1 **/
IDLC_METH_BEGIN(ni,iHStringCharIt,PeekRewind,1)
	IDLC_DECL_VAR(tU32,back)
	IDLC_BUF_TO_BASE(ni::eType_U32,back)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHStringCharIt,PeekRewind,1,(back))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iHStringCharIt,PeekRewind,1)

/** ni -> iHStringCharIt::Rewind/1 **/
IDLC_METH_BEGIN(ni,iHStringCharIt,Rewind,1)
	IDLC_DECL_VAR(tU32,n)
	IDLC_BUF_TO_BASE(ni::eType_U32,n)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iHStringCharIt,Rewind,1,(n))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iHStringCharIt,Rewind,1)

/** ni -> iHStringCharIt::ToPosition/1 **/
IDLC_METH_BEGIN(ni,iHStringCharIt,ToPosition,1)
	IDLC_DECL_VAR(tU32,anOffsetInBytes)
	IDLC_BUF_TO_BASE(ni::eType_U32,anOffsetInBytes)
	IDLC_METH_CALL_VOID(ni,iHStringCharIt,ToPosition,1,(anOffsetInBytes))
IDLC_METH_END(ni,iHStringCharIt,ToPosition,1)

IDLC_END_INTF(ni,iHStringCharIt)

/** NAMESPACE : astl **/
/** NAMESPACE : eastl **/
IDLC_END_NAMESPACE()
// EOF //
