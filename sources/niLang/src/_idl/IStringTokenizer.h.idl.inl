#include "../API/niLang/IStringTokenizer.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iStringTokenizer **/
IDLC_BEGIN_INTF(ni,iStringTokenizer)
/** ni -> iStringTokenizer::GetCharType/1 **/
IDLC_METH_BEGIN(ni,iStringTokenizer,GetCharType,1)
	IDLC_DECL_VAR(tU32,c)
	IDLC_BUF_TO_BASE(ni::eType_U32,c)
	IDLC_DECL_RETVAR(eStringTokenizerCharType,_Ret)
	IDLC_METH_CALL(_Ret,ni,iStringTokenizer,GetCharType,1,(c))
	IDLC_RET_FROM_ENUM(eStringTokenizerCharType,_Ret)
IDLC_METH_END(ni,iStringTokenizer,GetCharType,1)

/** ni -> iStringTokenizer::OnNewLine/0 **/
IDLC_METH_BEGIN(ni,iStringTokenizer,OnNewLine,0)
	IDLC_METH_CALL_VOID(ni,iStringTokenizer,OnNewLine,0,())
IDLC_METH_END(ni,iStringTokenizer,OnNewLine,0)

IDLC_END_INTF(ni,iStringTokenizer)

IDLC_END_NAMESPACE()
// EOF //
