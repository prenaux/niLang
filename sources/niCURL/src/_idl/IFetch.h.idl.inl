#include "../API/niCURL/IFetch.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iFetchSink **/
IDLC_BEGIN_INTF(ni,iFetchSink)
/** ni -> iFetchSink::OnFetchSink_Success/1 **/
IDLC_METH_BEGIN(ni,iFetchSink,OnFetchSink_Success,1)
	IDLC_DECL_VAR(iFetchRequest*,apFetch)
	IDLC_BUF_TO_INTF(iFetchRequest,apFetch)
	IDLC_METH_CALL_VOID(ni,iFetchSink,OnFetchSink_Success,1,(apFetch))
IDLC_METH_END(ni,iFetchSink,OnFetchSink_Success,1)

/** ni -> iFetchSink::OnFetchSink_Error/1 **/
IDLC_METH_BEGIN(ni,iFetchSink,OnFetchSink_Error,1)
	IDLC_DECL_VAR(iFetchRequest*,apFetch)
	IDLC_BUF_TO_INTF(iFetchRequest,apFetch)
	IDLC_METH_CALL_VOID(ni,iFetchSink,OnFetchSink_Error,1,(apFetch))
IDLC_METH_END(ni,iFetchSink,OnFetchSink_Error,1)

/** ni -> iFetchSink::OnFetchSink_Progress/1 **/
IDLC_METH_BEGIN(ni,iFetchSink,OnFetchSink_Progress,1)
	IDLC_DECL_VAR(iFetchRequest*,apFetch)
	IDLC_BUF_TO_INTF(iFetchRequest,apFetch)
	IDLC_METH_CALL_VOID(ni,iFetchSink,OnFetchSink_Progress,1,(apFetch))
IDLC_METH_END(ni,iFetchSink,OnFetchSink_Progress,1)

/** ni -> iFetchSink::OnFetchSink_ReadyStateChange/1 **/
IDLC_METH_BEGIN(ni,iFetchSink,OnFetchSink_ReadyStateChange,1)
	IDLC_DECL_VAR(iFetchRequest*,apFetch)
	IDLC_BUF_TO_INTF(iFetchRequest,apFetch)
	IDLC_METH_CALL_VOID(ni,iFetchSink,OnFetchSink_ReadyStateChange,1,(apFetch))
IDLC_METH_END(ni,iFetchSink,OnFetchSink_ReadyStateChange,1)

IDLC_END_INTF(ni,iFetchSink)

/** interface : iFetchRequest **/
IDLC_BEGIN_INTF(ni,iFetchRequest)
/** ni -> iFetchRequest::GetMethod/0 **/
IDLC_METH_BEGIN(ni,iFetchRequest,GetMethod,0)
	IDLC_DECL_RETVAR(eFetchMethod,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFetchRequest,GetMethod,0,())
	IDLC_RET_FROM_ENUM(eFetchMethod,_Ret)
IDLC_METH_END(ni,iFetchRequest,GetMethod,0)

/** ni -> iFetchRequest::GetReadyState/0 **/
IDLC_METH_BEGIN(ni,iFetchRequest,GetReadyState,0)
	IDLC_DECL_RETVAR(eFetchReadyState,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFetchRequest,GetReadyState,0,())
	IDLC_RET_FROM_ENUM(eFetchReadyState,_Ret)
IDLC_METH_END(ni,iFetchRequest,GetReadyState,0)

/** ni -> iFetchRequest::GetStatus/0 **/
IDLC_METH_BEGIN(ni,iFetchRequest,GetStatus,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFetchRequest,GetStatus,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iFetchRequest,GetStatus,0)

/** ni -> iFetchRequest::GetReceivedHeaders/0 **/
IDLC_METH_BEGIN(ni,iFetchRequest,GetReceivedHeaders,0)
	IDLC_DECL_RETVAR(iFile*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFetchRequest,GetReceivedHeaders,0,())
	IDLC_RET_FROM_INTF(iFile,_Ret)
IDLC_METH_END(ni,iFetchRequest,GetReceivedHeaders,0)

/** ni -> iFetchRequest::GetReceivedData/0 **/
IDLC_METH_BEGIN(ni,iFetchRequest,GetReceivedData,0)
	IDLC_DECL_RETVAR(iFile*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFetchRequest,GetReceivedData,0,())
	IDLC_RET_FROM_INTF(iFile,_Ret)
IDLC_METH_END(ni,iFetchRequest,GetReceivedData,0)

/** ni -> iFetchRequest::GetHasFailed/0 **/
IDLC_METH_BEGIN(ni,iFetchRequest,GetHasFailed,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFetchRequest,GetHasFailed,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFetchRequest,GetHasFailed,0)

IDLC_END_INTF(ni,iFetchRequest)

IDLC_END_NAMESPACE()
// EOF //
