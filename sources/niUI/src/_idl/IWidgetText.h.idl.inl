// clang-format off
//
// Autogenerated IDL definition for 'API/niUI/IWidgetText.h'.
//
#include "../API/niUI/IWidgetText.h"

#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif

IDLC_BEGIN_NAMESPACE()

/** NAMESPACE : ni **/
/** interface : iWidgetText **/
/** iWidgetText -> MIN FEATURES '20' **/
#if niMinFeatures(20)
IDLC_BEGIN_INTF(ni,iWidgetText)
/** ni -> iWidgetText::GetTextObject/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iWidgetText::GetTextObject/0 **/
IDLC_METH_BEGIN(ni,iWidgetText,GetTextObject,0)
	IDLC_DECL_RETVAR(iTextObject*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetText,GetTextObject,0,())
	IDLC_RET_FROM_INTF(iTextObject,_Ret)
IDLC_METH_END(ni,iWidgetText,GetTextObject,0)
#endif // niMinFeatures(20)

IDLC_END_INTF(ni,iWidgetText)

#endif // if niMinFeatures(20)
IDLC_END_NAMESPACE()
// EOF //
