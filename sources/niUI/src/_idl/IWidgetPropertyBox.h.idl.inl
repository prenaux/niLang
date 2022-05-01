#include "../API/niUI/IWidgetPropertyBox.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iWidgetPropertyBox **/
/** iWidgetPropertyBox -> MIN FEATURES '20' **/
#if niMinFeatures(20)
IDLC_BEGIN_INTF(ni,iWidgetPropertyBox)
/** ni -> iWidgetPropertyBox::SetDataTable/1 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iWidgetPropertyBox::SetDataTable/1 **/
IDLC_METH_BEGIN(ni,iWidgetPropertyBox,SetDataTable,1)
	IDLC_DECL_VAR(iDataTable*,apDT)
	IDLC_BUF_TO_INTF(iDataTable,apDT)
	IDLC_METH_CALL_VOID(ni,iWidgetPropertyBox,SetDataTable,1,(apDT))
IDLC_METH_END(ni,iWidgetPropertyBox,SetDataTable,1)
#endif // niMinFeatures(20)

/** ni -> iWidgetPropertyBox::GetDataTable/0 -> MIN FEATURES '20' **/
#if niMinFeatures(20)
/** ni -> iWidgetPropertyBox::GetDataTable/0 **/
IDLC_METH_BEGIN(ni,iWidgetPropertyBox,GetDataTable,0)
	IDLC_DECL_RETVAR(iDataTable*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetPropertyBox,GetDataTable,0,())
	IDLC_RET_FROM_INTF(iDataTable,_Ret)
IDLC_METH_END(ni,iWidgetPropertyBox,GetDataTable,0)
#endif // niMinFeatures(20)

IDLC_END_INTF(ni,iWidgetPropertyBox)

#endif // if niMinFeatures(20)
IDLC_END_NAMESPACE()
// EOF //
