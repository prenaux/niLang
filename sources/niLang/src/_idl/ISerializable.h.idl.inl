#include "../API/niLang/ISerializable.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iSerializable **/
IDLC_BEGIN_INTF(ni,iSerializable)
/** ni -> iSerializable::GetSerializeObjectTypeID/0 **/
IDLC_METH_BEGIN(ni,iSerializable,GetSerializeObjectTypeID,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSerializable,GetSerializeObjectTypeID,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iSerializable,GetSerializeObjectTypeID,0)

/** ni -> iSerializable::Serialize/2 **/
IDLC_METH_BEGIN(ni,iSerializable,Serialize,2)
	IDLC_DECL_VAR(iFile*,apFile)
	IDLC_BUF_TO_INTF(iFile,apFile)
	IDLC_DECL_VAR(eSerializeMode,aMode)
	IDLC_BUF_TO_ENUM(eSerializeMode,aMode)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iSerializable,Serialize,2,(apFile,aMode))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iSerializable,Serialize,2)

IDLC_END_INTF(ni,iSerializable)

IDLC_END_NAMESPACE()
// EOF //
