#include "../API/niLang/IDataTable.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iDataTableSink **/
IDLC_BEGIN_INTF(ni,iDataTableSink)
/** ni -> iDataTableSink::OnDataTableSink_SetName/1 **/
IDLC_METH_BEGIN(ni,iDataTableSink,OnDataTableSink_SetName,1)
	IDLC_DECL_VAR(iDataTable*,apDT)
	IDLC_BUF_TO_INTF(iDataTable,apDT)
	IDLC_METH_CALL_VOID(ni,iDataTableSink,OnDataTableSink_SetName,1,(apDT))
IDLC_METH_END(ni,iDataTableSink,OnDataTableSink_SetName,1)

/** ni -> iDataTableSink::OnDataTableSink_AddChild/2 **/
IDLC_METH_BEGIN(ni,iDataTableSink,OnDataTableSink_AddChild,2)
	IDLC_DECL_VAR(iDataTable*,apDT)
	IDLC_BUF_TO_INTF(iDataTable,apDT)
	IDLC_DECL_VAR(iDataTable*,apChild)
	IDLC_BUF_TO_INTF(iDataTable,apChild)
	IDLC_METH_CALL_VOID(ni,iDataTableSink,OnDataTableSink_AddChild,2,(apDT,apChild))
IDLC_METH_END(ni,iDataTableSink,OnDataTableSink_AddChild,2)

/** ni -> iDataTableSink::OnDataTableSink_RemoveChild/2 **/
IDLC_METH_BEGIN(ni,iDataTableSink,OnDataTableSink_RemoveChild,2)
	IDLC_DECL_VAR(iDataTable*,apDT)
	IDLC_BUF_TO_INTF(iDataTable,apDT)
	IDLC_DECL_VAR(iDataTable*,apChild)
	IDLC_BUF_TO_INTF(iDataTable,apChild)
	IDLC_METH_CALL_VOID(ni,iDataTableSink,OnDataTableSink_RemoveChild,2,(apDT,apChild))
IDLC_METH_END(ni,iDataTableSink,OnDataTableSink_RemoveChild,2)

/** ni -> iDataTableSink::OnDataTableSink_SetProperty/2 **/
IDLC_METH_BEGIN(ni,iDataTableSink,OnDataTableSink_SetProperty,2)
	IDLC_DECL_VAR(iDataTable*,apDT)
	IDLC_BUF_TO_INTF(iDataTable,apDT)
	IDLC_DECL_VAR(tU32,anProperty)
	IDLC_BUF_TO_BASE(ni::eType_U32,anProperty)
	IDLC_METH_CALL_VOID(ni,iDataTableSink,OnDataTableSink_SetProperty,2,(apDT,anProperty))
IDLC_METH_END(ni,iDataTableSink,OnDataTableSink_SetProperty,2)

/** ni -> iDataTableSink::OnDataTableSink_SetMetadata/2 **/
IDLC_METH_BEGIN(ni,iDataTableSink,OnDataTableSink_SetMetadata,2)
	IDLC_DECL_VAR(iDataTable*,apDT)
	IDLC_BUF_TO_INTF(iDataTable,apDT)
	IDLC_DECL_VAR(tU32,anProperty)
	IDLC_BUF_TO_BASE(ni::eType_U32,anProperty)
	IDLC_METH_CALL_VOID(ni,iDataTableSink,OnDataTableSink_SetMetadata,2,(apDT,anProperty))
IDLC_METH_END(ni,iDataTableSink,OnDataTableSink_SetMetadata,2)

/** ni -> iDataTableSink::OnDataTableSink_RemoveProperty/2 **/
IDLC_METH_BEGIN(ni,iDataTableSink,OnDataTableSink_RemoveProperty,2)
	IDLC_DECL_VAR(iDataTable*,apDT)
	IDLC_BUF_TO_INTF(iDataTable,apDT)
	IDLC_DECL_VAR(tU32,anProperty)
	IDLC_BUF_TO_BASE(ni::eType_U32,anProperty)
	IDLC_METH_CALL_VOID(ni,iDataTableSink,OnDataTableSink_RemoveProperty,2,(apDT,anProperty))
IDLC_METH_END(ni,iDataTableSink,OnDataTableSink_RemoveProperty,2)

/** ni -> iDataTableSink::OnDataTableSink_GetProperty/2 **/
IDLC_METH_BEGIN(ni,iDataTableSink,OnDataTableSink_GetProperty,2)
	IDLC_DECL_VAR(iDataTable*,apDT)
	IDLC_BUF_TO_INTF(iDataTable,apDT)
	IDLC_DECL_VAR(tU32,anProperty)
	IDLC_BUF_TO_BASE(ni::eType_U32,anProperty)
	IDLC_DECL_RETVAR(Var,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableSink,OnDataTableSink_GetProperty,2,(apDT,anProperty))
	IDLC_RET_FROM_BASE(ni::eType_Variant,_Ret)
IDLC_METH_END(ni,iDataTableSink,OnDataTableSink_GetProperty,2)

IDLC_END_INTF(ni,iDataTableSink)

/** interface : iDataTable **/
IDLC_BEGIN_INTF(ni,iDataTable)
/** ni -> iDataTable::SetName/1 **/
IDLC_METH_BEGIN(ni,iDataTable,SetName,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_METH_CALL_VOID(ni,iDataTable,SetName,1,(aaszName))
IDLC_METH_END(ni,iDataTable,SetName,1)

/** ni -> iDataTable::GetName/0 **/
IDLC_METH_BEGIN(ni,iDataTable,GetName,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetName,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iDataTable,GetName,0)

/** ni -> iDataTable::GetRoot/0 **/
IDLC_METH_BEGIN(ni,iDataTable,GetRoot,0)
	IDLC_DECL_RETVAR(iDataTable*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetRoot,0,())
	IDLC_RET_FROM_INTF(iDataTable,_Ret)
IDLC_METH_END(ni,iDataTable,GetRoot,0)

/** ni -> iDataTable::GetParent/0 **/
IDLC_METH_BEGIN(ni,iDataTable,GetParent,0)
	IDLC_DECL_RETVAR(iDataTable*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetParent,0,())
	IDLC_RET_FROM_INTF(iDataTable,_Ret)
IDLC_METH_END(ni,iDataTable,GetParent,0)

/** ni -> iDataTable::GetPrevSibling/0 **/
IDLC_METH_BEGIN(ni,iDataTable,GetPrevSibling,0)
	IDLC_DECL_RETVAR(iDataTable*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetPrevSibling,0,())
	IDLC_RET_FROM_INTF(iDataTable,_Ret)
IDLC_METH_END(ni,iDataTable,GetPrevSibling,0)

/** ni -> iDataTable::GetNextSibling/0 **/
IDLC_METH_BEGIN(ni,iDataTable,GetNextSibling,0)
	IDLC_DECL_RETVAR(iDataTable*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetNextSibling,0,())
	IDLC_RET_FROM_INTF(iDataTable,_Ret)
IDLC_METH_END(ni,iDataTable,GetNextSibling,0)

/** ni -> iDataTable::GetIndex/0 **/
IDLC_METH_BEGIN(ni,iDataTable,GetIndex,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetIndex,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTable,GetIndex,0)

/** ni -> iDataTable::Clone/0 **/
IDLC_METH_BEGIN(ni,iDataTable,Clone,0)
	IDLC_DECL_RETVAR(iDataTable*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,Clone,0,())
	IDLC_RET_FROM_INTF(iDataTable,_Ret)
IDLC_METH_END(ni,iDataTable,Clone,0)

/** ni -> iDataTable::CloneEx/1 **/
IDLC_METH_BEGIN(ni,iDataTable,CloneEx,1)
	IDLC_DECL_VAR(tDataTableCopyFlags,aMode)
	IDLC_BUF_TO_ENUM(tDataTableCopyFlags,aMode)
	IDLC_DECL_RETVAR(iDataTable*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,CloneEx,1,(aMode))
	IDLC_RET_FROM_INTF(iDataTable,_Ret)
IDLC_METH_END(ni,iDataTable,CloneEx,1)

/** ni -> iDataTable::Copy/2 **/
IDLC_METH_BEGIN(ni,iDataTable,Copy,2)
	IDLC_DECL_VAR(iDataTable*,apSource)
	IDLC_BUF_TO_INTF(iDataTable,apSource)
	IDLC_DECL_VAR(tDataTableCopyFlags,aMode)
	IDLC_BUF_TO_ENUM(tDataTableCopyFlags,aMode)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,Copy,2,(apSource,aMode))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTable,Copy,2)

/** ni -> iDataTable::Clear/0 **/
IDLC_METH_BEGIN(ni,iDataTable,Clear,0)
	IDLC_METH_CALL_VOID(ni,iDataTable,Clear,0,())
IDLC_METH_END(ni,iDataTable,Clear,0)

/** ni -> iDataTable::GetHasSink/0 **/
IDLC_METH_BEGIN(ni,iDataTable,GetHasSink,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetHasSink,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTable,GetHasSink,0)

/** ni -> iDataTable::GetSinkList/0 **/
IDLC_METH_BEGIN(ni,iDataTable,GetSinkList,0)
	IDLC_DECL_RETVAR(tDataTableSinkLst*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetSinkList,0,())
	IDLC_RET_FROM_INTF(tDataTableSinkLst,_Ret)
IDLC_METH_END(ni,iDataTable,GetSinkList,0)

/** ni -> iDataTable::ClearChildren/0 **/
IDLC_METH_BEGIN(ni,iDataTable,ClearChildren,0)
	IDLC_METH_CALL_VOID(ni,iDataTable,ClearChildren,0,())
IDLC_METH_END(ni,iDataTable,ClearChildren,0)

/** ni -> iDataTable::GetNumChildren/0 **/
IDLC_METH_BEGIN(ni,iDataTable,GetNumChildren,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetNumChildren,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTable,GetNumChildren,0)

/** ni -> iDataTable::GetChildIndex/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetChildIndex,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetChildIndex,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTable,GetChildIndex,1)

/** ni -> iDataTable::GetChild/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetChild,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(iDataTable*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetChild,1,(aaszName))
	IDLC_RET_FROM_INTF(iDataTable,_Ret)
IDLC_METH_END(ni,iDataTable,GetChild,1)

/** ni -> iDataTable::GetChildFromIndex/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetChildFromIndex,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(iDataTable*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetChildFromIndex,1,(anIndex))
	IDLC_RET_FROM_INTF(iDataTable,_Ret)
IDLC_METH_END(ni,iDataTable,GetChildFromIndex,1)

/** ni -> iDataTable::AddChild/1 **/
IDLC_METH_BEGIN(ni,iDataTable,AddChild,1)
	IDLC_DECL_VAR(iDataTable*,apTable)
	IDLC_BUF_TO_INTF(iDataTable,apTable)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,AddChild,1,(apTable))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTable,AddChild,1)

/** ni -> iDataTable::RemoveChild/1 **/
IDLC_METH_BEGIN(ni,iDataTable,RemoveChild,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,RemoveChild,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTable,RemoveChild,1)

/** ni -> iDataTable::ClearProperties/0 **/
IDLC_METH_BEGIN(ni,iDataTable,ClearProperties,0)
	IDLC_METH_CALL_VOID(ni,iDataTable,ClearProperties,0,())
IDLC_METH_END(ni,iDataTable,ClearProperties,0)

/** ni -> iDataTable::RemoveProperty/1 **/
IDLC_METH_BEGIN(ni,iDataTable,RemoveProperty,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,RemoveProperty,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTable,RemoveProperty,1)

/** ni -> iDataTable::RemovePropertyFromIndex/1 **/
IDLC_METH_BEGIN(ni,iDataTable,RemovePropertyFromIndex,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,RemovePropertyFromIndex,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTable,RemovePropertyFromIndex,1)

/** ni -> iDataTable::GetNumProperties/0 **/
IDLC_METH_BEGIN(ni,iDataTable,GetNumProperties,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetNumProperties,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTable,GetNumProperties,0)

/** ni -> iDataTable::GetPropertyIndex/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetPropertyIndex,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetPropertyIndex,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTable,GetPropertyIndex,1)

/** ni -> iDataTable::GetPropertyName/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetPropertyName,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetPropertyName,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iDataTable,GetPropertyName,1)

/** ni -> iDataTable::GetPropertyType/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetPropertyType,1)
	IDLC_DECL_VAR(achar*,aaszProp)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszProp)
	IDLC_DECL_RETVAR(eDataTablePropertyType,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetPropertyType,1,(aaszProp))
	IDLC_RET_FROM_ENUM(eDataTablePropertyType,_Ret)
IDLC_METH_END(ni,iDataTable,GetPropertyType,1)

/** ni -> iDataTable::GetPropertyTypeFromIndex/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetPropertyTypeFromIndex,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(eDataTablePropertyType,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetPropertyTypeFromIndex,1,(anIndex))
	IDLC_RET_FROM_ENUM(eDataTablePropertyType,_Ret)
IDLC_METH_END(ni,iDataTable,GetPropertyTypeFromIndex,1)

/** ni -> iDataTable::HasProperty/1 **/
IDLC_METH_BEGIN(ni,iDataTable,HasProperty,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,HasProperty,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTable,HasProperty,1)

/** ni -> iDataTable::SetMetadata/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetMetadata,2)
	IDLC_DECL_VAR(achar*,aaszProp)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszProp)
	IDLC_DECL_VAR(iHString*,ahspData)
	IDLC_BUF_TO_INTF(iHString,ahspData)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,SetMetadata,2,(aaszProp,ahspData))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTable,SetMetadata,2)

/** ni -> iDataTable::GetMetadata/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetMetadata,1)
	IDLC_DECL_VAR(achar*,aaszProp)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszProp)
	IDLC_DECL_RETVAR(Ptr<iHString>,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetMetadata,1,(aaszProp))
	IDLC_RET_FROM_INTF(,_Ret)
IDLC_METH_END(ni,iDataTable,GetMetadata,1)

/** ni -> iDataTable::SetMetadataFromIndex/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetMetadataFromIndex,2)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_VAR(iHString*,ahspData)
	IDLC_BUF_TO_INTF(iHString,ahspData)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,SetMetadataFromIndex,2,(anIndex,ahspData))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTable,SetMetadataFromIndex,2)

/** ni -> iDataTable::GetMetadataFromIndex/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetMetadataFromIndex,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(Ptr<iHString>,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetMetadataFromIndex,1,(anIndex))
	IDLC_RET_FROM_INTF(,_Ret)
IDLC_METH_END(ni,iDataTable,GetMetadataFromIndex,1)

/** ni -> iDataTable::GetVar/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetVar,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(Var,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetVar,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_Variant,_Ret)
IDLC_METH_END(ni,iDataTable,GetVar,1)

/** ni -> iDataTable::GetVarFromIndex/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetVarFromIndex,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(Var,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetVarFromIndex,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_Variant,_Ret)
IDLC_METH_END(ni,iDataTable,GetVarFromIndex,1)

/** ni -> iDataTable::GetVarDefault/2 **/
IDLC_METH_BEGIN(ni,iDataTable,GetVarDefault,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(Var,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,v)
	IDLC_DECL_RETVAR(Var,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetVarDefault,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_Variant,_Ret)
IDLC_METH_END(ni,iDataTable,GetVarDefault,2)

/** ni -> iDataTable::GetString/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetString,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetString,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iDataTable,GetString,1)

/** ni -> iDataTable::GetStringFromIndex/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetStringFromIndex,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetStringFromIndex,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iDataTable,GetStringFromIndex,1)

/** ni -> iDataTable::GetStringDefault/2 **/
IDLC_METH_BEGIN(ni,iDataTable,GetStringDefault,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(achar*,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,v)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetStringDefault,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iDataTable,GetStringDefault,2)

/** ni -> iDataTable::GetHString/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetHString,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(Ptr<iHString>,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetHString,1,(aaszName))
	IDLC_RET_FROM_INTF(,_Ret)
IDLC_METH_END(ni,iDataTable,GetHString,1)

/** ni -> iDataTable::GetHStringFromIndex/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetHStringFromIndex,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(Ptr<iHString>,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetHStringFromIndex,1,(anIndex))
	IDLC_RET_FROM_INTF(,_Ret)
IDLC_METH_END(ni,iDataTable,GetHStringFromIndex,1)

/** ni -> iDataTable::GetHStringDefault/2 **/
IDLC_METH_BEGIN(ni,iDataTable,GetHStringDefault,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(iHString*,v)
	IDLC_BUF_TO_INTF(iHString,v)
	IDLC_DECL_RETVAR(Ptr<iHString>,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetHStringDefault,2,(aaszName,v))
	IDLC_RET_FROM_INTF(,_Ret)
IDLC_METH_END(ni,iDataTable,GetHStringDefault,2)

/** ni -> iDataTable::GetInt/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetInt,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(tI64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetInt,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_I64,_Ret)
IDLC_METH_END(ni,iDataTable,GetInt,1)

/** ni -> iDataTable::GetIntFromIndex/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetIntFromIndex,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(tI64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetIntFromIndex,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_I64,_Ret)
IDLC_METH_END(ni,iDataTable,GetIntFromIndex,1)

/** ni -> iDataTable::GetIntDefault/2 **/
IDLC_METH_BEGIN(ni,iDataTable,GetIntDefault,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(tI64,v)
	IDLC_BUF_TO_BASE(ni::eType_I64,v)
	IDLC_DECL_RETVAR(tI64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetIntDefault,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_I64,_Ret)
IDLC_METH_END(ni,iDataTable,GetIntDefault,2)

/** ni -> iDataTable::GetBool/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetBool,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetBool,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTable,GetBool,1)

/** ni -> iDataTable::GetBoolFromIndex/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetBoolFromIndex,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetBoolFromIndex,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTable,GetBoolFromIndex,1)

/** ni -> iDataTable::GetBoolDefault/2 **/
IDLC_METH_BEGIN(ni,iDataTable,GetBoolDefault,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(tBool,v)
	IDLC_BUF_TO_BASE(ni::eType_I8,v)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetBoolDefault,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTable,GetBoolDefault,2)

/** ni -> iDataTable::GetFloat/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetFloat,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(tF64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetFloat,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_F64,_Ret)
IDLC_METH_END(ni,iDataTable,GetFloat,1)

/** ni -> iDataTable::GetFloatFromIndex/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetFloatFromIndex,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(tF64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetFloatFromIndex,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_F64,_Ret)
IDLC_METH_END(ni,iDataTable,GetFloatFromIndex,1)

/** ni -> iDataTable::GetFloatDefault/2 **/
IDLC_METH_BEGIN(ni,iDataTable,GetFloatDefault,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(tF64,v)
	IDLC_BUF_TO_BASE(ni::eType_F64,v)
	IDLC_DECL_RETVAR(tF64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetFloatDefault,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_F64,_Ret)
IDLC_METH_END(ni,iDataTable,GetFloatDefault,2)

/** ni -> iDataTable::GetVec2/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetVec2,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetVec2,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iDataTable,GetVec2,1)

/** ni -> iDataTable::GetVec2FromIndex/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetVec2FromIndex,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetVec2FromIndex,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iDataTable,GetVec2FromIndex,1)

/** ni -> iDataTable::GetVec2Default/2 **/
IDLC_METH_BEGIN(ni,iDataTable,GetVec2Default,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(sVec2f,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,v)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetVec2Default,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iDataTable,GetVec2Default,2)

/** ni -> iDataTable::GetVec3/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetVec3,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetVec3,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iDataTable,GetVec3,1)

/** ni -> iDataTable::GetVec3FromIndex/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetVec3FromIndex,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetVec3FromIndex,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iDataTable,GetVec3FromIndex,1)

/** ni -> iDataTable::GetVec3Default/2 **/
IDLC_METH_BEGIN(ni,iDataTable,GetVec3Default,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(sVec3f,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,v)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetVec3Default,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iDataTable,GetVec3Default,2)

/** ni -> iDataTable::GetVec4/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetVec4,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(sVec4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetVec4,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iDataTable,GetVec4,1)

/** ni -> iDataTable::GetVec4FromIndex/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetVec4FromIndex,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(sVec4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetVec4FromIndex,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iDataTable,GetVec4FromIndex,1)

/** ni -> iDataTable::GetVec4Default/2 **/
IDLC_METH_BEGIN(ni,iDataTable,GetVec4Default,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(sVec4f,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,v)
	IDLC_DECL_RETVAR(sVec4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetVec4Default,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iDataTable,GetVec4Default,2)

/** ni -> iDataTable::GetCol3/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetCol3,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetCol3,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iDataTable,GetCol3,1)

/** ni -> iDataTable::GetCol3FromIndex/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetCol3FromIndex,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetCol3FromIndex,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iDataTable,GetCol3FromIndex,1)

/** ni -> iDataTable::GetCol4/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetCol4,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(sVec4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetCol4,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iDataTable,GetCol4,1)

/** ni -> iDataTable::GetCol4FromIndex/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetCol4FromIndex,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(sVec4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetCol4FromIndex,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iDataTable,GetCol4FromIndex,1)

/** ni -> iDataTable::GetMatrix/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetMatrix,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(sMatrixf,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetMatrix,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_Matrixf,_Ret)
IDLC_METH_END(ni,iDataTable,GetMatrix,1)

/** ni -> iDataTable::GetMatrixFromIndex/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetMatrixFromIndex,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(sMatrixf,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetMatrixFromIndex,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_Matrixf,_Ret)
IDLC_METH_END(ni,iDataTable,GetMatrixFromIndex,1)

/** ni -> iDataTable::GetMatrixDefault/2 **/
IDLC_METH_BEGIN(ni,iDataTable,GetMatrixDefault,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(sMatrixf,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Matrixf|ni::eTypeFlags_Pointer,v)
	IDLC_DECL_RETVAR(sMatrixf,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetMatrixDefault,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_Matrixf,_Ret)
IDLC_METH_END(ni,iDataTable,GetMatrixDefault,2)

/** ni -> iDataTable::GetIUnknown/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetIUnknown,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(iUnknown*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetIUnknown,1,(aaszName))
	IDLC_RET_FROM_INTF(iUnknown,_Ret)
IDLC_METH_END(ni,iDataTable,GetIUnknown,1)

/** ni -> iDataTable::GetIUnknownFromIndex/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetIUnknownFromIndex,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(iUnknown*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetIUnknownFromIndex,1,(anIndex))
	IDLC_RET_FROM_INTF(iUnknown,_Ret)
IDLC_METH_END(ni,iDataTable,GetIUnknownFromIndex,1)

/** ni -> iDataTable::GetIUnknownDefault/2 **/
IDLC_METH_BEGIN(ni,iDataTable,GetIUnknownDefault,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(iUnknown*,v)
	IDLC_BUF_TO_INTF(iUnknown,v)
	IDLC_DECL_RETVAR(iUnknown*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetIUnknownDefault,2,(aaszName,v))
	IDLC_RET_FROM_INTF(iUnknown,_Ret)
IDLC_METH_END(ni,iDataTable,GetIUnknownDefault,2)

/** ni -> iDataTable::GetEnum/3 **/
IDLC_METH_BEGIN(ni,iDataTable,GetEnum,3)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(sEnumDef*,apEnumDef)
	IDLC_BUF_TO_PTR(ni::eTypeFlags_Constant|ni::eType_Ptr|ni::eTypeFlags_Pointer,apEnumDef)
	IDLC_DECL_VAR(tEnumToStringFlags,aFlags)
	IDLC_BUF_TO_ENUM(tEnumToStringFlags,aFlags)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetEnum,3,(aaszName,apEnumDef,aFlags))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTable,GetEnum,3)

/** ni -> iDataTable::GetEnumFromIndex/3 **/
IDLC_METH_BEGIN(ni,iDataTable,GetEnumFromIndex,3)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_VAR(sEnumDef*,apEnumDef)
	IDLC_BUF_TO_PTR(ni::eTypeFlags_Constant|ni::eType_Ptr|ni::eTypeFlags_Pointer,apEnumDef)
	IDLC_DECL_VAR(tEnumToStringFlags,aFlags)
	IDLC_BUF_TO_ENUM(tEnumToStringFlags,aFlags)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetEnumFromIndex,3,(anIndex,apEnumDef,aFlags))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTable,GetEnumFromIndex,3)

/** ni -> iDataTable::GetEnumDefault/4 **/
IDLC_METH_BEGIN(ni,iDataTable,GetEnumDefault,4)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(sEnumDef*,apEnumDef)
	IDLC_BUF_TO_PTR(ni::eTypeFlags_Constant|ni::eType_Ptr|ni::eTypeFlags_Pointer,apEnumDef)
	IDLC_DECL_VAR(tEnumToStringFlags,aFlags)
	IDLC_BUF_TO_ENUM(tEnumToStringFlags,aFlags)
	IDLC_DECL_VAR(tU32,v)
	IDLC_BUF_TO_BASE(ni::eType_U32,v)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetEnumDefault,4,(aaszName,apEnumDef,aFlags,v))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTable,GetEnumDefault,4)

/** ni -> iDataTable::SetVar/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetVar,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(Var,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,v)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,SetVar,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTable,SetVar,2)

/** ni -> iDataTable::SetVarFromIndex/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetVarFromIndex,2)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_VAR(Var,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,v)
	IDLC_METH_CALL_VOID(ni,iDataTable,SetVarFromIndex,2,(anIndex,v))
IDLC_METH_END(ni,iDataTable,SetVarFromIndex,2)

/** ni -> iDataTable::SetString/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetString,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(achar*,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,v)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,SetString,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTable,SetString,2)

/** ni -> iDataTable::SetStringFromIndex/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetStringFromIndex,2)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_VAR(achar*,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,v)
	IDLC_METH_CALL_VOID(ni,iDataTable,SetStringFromIndex,2,(anIndex,v))
IDLC_METH_END(ni,iDataTable,SetStringFromIndex,2)

/** ni -> iDataTable::SetHString/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetHString,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(iHString*,v)
	IDLC_BUF_TO_INTF(iHString,v)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,SetHString,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTable,SetHString,2)

/** ni -> iDataTable::SetHStringFromIndex/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetHStringFromIndex,2)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_VAR(iHString*,v)
	IDLC_BUF_TO_INTF(iHString,v)
	IDLC_METH_CALL_VOID(ni,iDataTable,SetHStringFromIndex,2,(anIndex,v))
IDLC_METH_END(ni,iDataTable,SetHStringFromIndex,2)

/** ni -> iDataTable::SetInt/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetInt,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(tI64,v)
	IDLC_BUF_TO_BASE(ni::eType_I64,v)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,SetInt,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTable,SetInt,2)

/** ni -> iDataTable::SetIntFromIndex/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetIntFromIndex,2)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_VAR(tI64,v)
	IDLC_BUF_TO_BASE(ni::eType_I64,v)
	IDLC_METH_CALL_VOID(ni,iDataTable,SetIntFromIndex,2,(anIndex,v))
IDLC_METH_END(ni,iDataTable,SetIntFromIndex,2)

/** ni -> iDataTable::SetBool/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetBool,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(tBool,v)
	IDLC_BUF_TO_BASE(ni::eType_I8,v)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,SetBool,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTable,SetBool,2)

/** ni -> iDataTable::SetBoolFromIndex/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetBoolFromIndex,2)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_VAR(tBool,v)
	IDLC_BUF_TO_BASE(ni::eType_I8,v)
	IDLC_METH_CALL_VOID(ni,iDataTable,SetBoolFromIndex,2,(anIndex,v))
IDLC_METH_END(ni,iDataTable,SetBoolFromIndex,2)

/** ni -> iDataTable::SetFloat/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetFloat,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(tF64,v)
	IDLC_BUF_TO_BASE(ni::eType_F64,v)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,SetFloat,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTable,SetFloat,2)

/** ni -> iDataTable::SetFloatFromIndex/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetFloatFromIndex,2)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_VAR(tF64,v)
	IDLC_BUF_TO_BASE(ni::eType_F64,v)
	IDLC_METH_CALL_VOID(ni,iDataTable,SetFloatFromIndex,2,(anIndex,v))
IDLC_METH_END(ni,iDataTable,SetFloatFromIndex,2)

/** ni -> iDataTable::SetVec2/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetVec2,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(sVec2f,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,v)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,SetVec2,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTable,SetVec2,2)

/** ni -> iDataTable::SetVec2FromIndex/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetVec2FromIndex,2)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_VAR(sVec2f,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,v)
	IDLC_METH_CALL_VOID(ni,iDataTable,SetVec2FromIndex,2,(anIndex,v))
IDLC_METH_END(ni,iDataTable,SetVec2FromIndex,2)

/** ni -> iDataTable::SetVec3/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetVec3,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(sVec3f,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,v)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,SetVec3,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTable,SetVec3,2)

/** ni -> iDataTable::SetVec3FromIndex/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetVec3FromIndex,2)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_VAR(sVec3f,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,v)
	IDLC_METH_CALL_VOID(ni,iDataTable,SetVec3FromIndex,2,(anIndex,v))
IDLC_METH_END(ni,iDataTable,SetVec3FromIndex,2)

/** ni -> iDataTable::SetVec4/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetVec4,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(sVec4f,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,v)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,SetVec4,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTable,SetVec4,2)

/** ni -> iDataTable::SetVec4FromIndex/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetVec4FromIndex,2)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_VAR(sVec4f,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,v)
	IDLC_METH_CALL_VOID(ni,iDataTable,SetVec4FromIndex,2,(anIndex,v))
IDLC_METH_END(ni,iDataTable,SetVec4FromIndex,2)

/** ni -> iDataTable::SetMatrix/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetMatrix,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(sMatrixf,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Matrixf|ni::eTypeFlags_Pointer,v)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,SetMatrix,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTable,SetMatrix,2)

/** ni -> iDataTable::SetMatrixFromIndex/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetMatrixFromIndex,2)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_VAR(sMatrixf,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Matrixf|ni::eTypeFlags_Pointer,v)
	IDLC_METH_CALL_VOID(ni,iDataTable,SetMatrixFromIndex,2,(anIndex,v))
IDLC_METH_END(ni,iDataTable,SetMatrixFromIndex,2)

/** ni -> iDataTable::SetIUnknown/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetIUnknown,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(iUnknown*,v)
	IDLC_BUF_TO_INTF(iUnknown,v)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,SetIUnknown,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTable,SetIUnknown,2)

/** ni -> iDataTable::SetIUnknownFromIndex/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetIUnknownFromIndex,2)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_VAR(iUnknown*,v)
	IDLC_BUF_TO_INTF(iUnknown,v)
	IDLC_METH_CALL_VOID(ni,iDataTable,SetIUnknownFromIndex,2,(anIndex,v))
IDLC_METH_END(ni,iDataTable,SetIUnknownFromIndex,2)

/** ni -> iDataTable::SetEnum/4 **/
IDLC_METH_BEGIN(ni,iDataTable,SetEnum,4)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(sEnumDef*,apEnumDef)
	IDLC_BUF_TO_PTR(ni::eTypeFlags_Constant|ni::eType_Ptr|ni::eTypeFlags_Pointer,apEnumDef)
	IDLC_DECL_VAR(tEnumToStringFlags,aFlags)
	IDLC_BUF_TO_ENUM(tEnumToStringFlags,aFlags)
	IDLC_DECL_VAR(tU32,anVal)
	IDLC_BUF_TO_BASE(ni::eType_U32,anVal)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,SetEnum,4,(aaszName,apEnumDef,aFlags,anVal))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTable,SetEnum,4)

/** ni -> iDataTable::SetEnumFromIndex/4 **/
IDLC_METH_BEGIN(ni,iDataTable,SetEnumFromIndex,4)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_VAR(sEnumDef*,apEnumDef)
	IDLC_BUF_TO_PTR(ni::eTypeFlags_Constant|ni::eType_Ptr|ni::eTypeFlags_Pointer,apEnumDef)
	IDLC_DECL_VAR(tEnumToStringFlags,aFlags)
	IDLC_BUF_TO_ENUM(tEnumToStringFlags,aFlags)
	IDLC_DECL_VAR(tU32,anVal)
	IDLC_BUF_TO_BASE(ni::eType_U32,anVal)
	IDLC_METH_CALL_VOID(ni,iDataTable,SetEnumFromIndex,4,(anIndex,apEnumDef,aFlags,anVal))
IDLC_METH_END(ni,iDataTable,SetEnumFromIndex,4)

/** ni -> iDataTable::SetRawVar/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetRawVar,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(Var,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,v)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,SetRawVar,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTable,SetRawVar,2)

/** ni -> iDataTable::SetRawVarFromIndex/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetRawVarFromIndex,2)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_VAR(Var,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,v)
	IDLC_METH_CALL_VOID(ni,iDataTable,SetRawVarFromIndex,2,(anIndex,v))
IDLC_METH_END(ni,iDataTable,SetRawVarFromIndex,2)

/** ni -> iDataTable::GetRawVar/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetRawVar,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(Var,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetRawVar,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_Variant,_Ret)
IDLC_METH_END(ni,iDataTable,GetRawVar,1)

/** ni -> iDataTable::GetRawVarFromIndex/1 **/
IDLC_METH_BEGIN(ni,iDataTable,GetRawVarFromIndex,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(Var,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetRawVarFromIndex,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_Variant,_Ret)
IDLC_METH_END(ni,iDataTable,GetRawVarFromIndex,1)

/** ni -> iDataTable::GetRawVarDefault/2 **/
IDLC_METH_BEGIN(ni,iDataTable,GetRawVarDefault,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(Var,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,v)
	IDLC_DECL_RETVAR(Var,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetRawVarDefault,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_Variant,_Ret)
IDLC_METH_END(ni,iDataTable,GetRawVarDefault,2)

/** ni -> iDataTable::NewVarFromPath/2 **/
IDLC_METH_BEGIN(ni,iDataTable,NewVarFromPath,2)
	IDLC_DECL_VAR(achar*,aPath)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aPath)
	IDLC_DECL_VAR(Var,aVal)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,aVal)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,NewVarFromPath,2,(aPath,aVal))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTable,NewVarFromPath,2)

/** ni -> iDataTable::SetVarFromPath/2 **/
IDLC_METH_BEGIN(ni,iDataTable,SetVarFromPath,2)
	IDLC_DECL_VAR(achar*,aPath)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aPath)
	IDLC_DECL_VAR(Var,aVal)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,aVal)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,SetVarFromPath,2,(aPath,aVal))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTable,SetVarFromPath,2)

/** ni -> iDataTable::GetVarFromPath/2 **/
IDLC_METH_BEGIN(ni,iDataTable,GetVarFromPath,2)
	IDLC_DECL_VAR(achar*,aPath)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aPath)
	IDLC_DECL_VAR(Var,aDefault)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,aDefault)
	IDLC_DECL_RETVAR(Var,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTable,GetVarFromPath,2,(aPath,aDefault))
	IDLC_RET_FROM_BASE(ni::eType_Variant,_Ret)
IDLC_METH_END(ni,iDataTable,GetVarFromPath,2)

IDLC_END_INTF(ni,iDataTable)

/** interface : iDataTableReadStack **/
IDLC_BEGIN_INTF(ni,iDataTableReadStack)
/** ni -> iDataTableReadStack::GetTop/0 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetTop,0)
	IDLC_DECL_RETVAR(iDataTable*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetTop,0,())
	IDLC_RET_FROM_INTF(iDataTable,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetTop,0)

/** ni -> iDataTableReadStack::GetName/0 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetName,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetName,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetName,0)

/** ni -> iDataTableReadStack::GetNumChildren/0 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetNumChildren,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetNumChildren,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetNumChildren,0)

/** ni -> iDataTableReadStack::PushChild/1 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,PushChild,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,PushChild,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,PushChild,1)

/** ni -> iDataTableReadStack::PushChildFail/1 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,PushChildFail,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,PushChildFail,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,PushChildFail,1)

/** ni -> iDataTableReadStack::Push/1 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,Push,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,Push,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,Push,1)

/** ni -> iDataTableReadStack::PushFail/1 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,PushFail,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,PushFail,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,PushFail,1)

/** ni -> iDataTableReadStack::PushEx/3 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,PushEx,3)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(achar*,aaszProp)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszProp)
	IDLC_DECL_VAR(achar*,aaszVal)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszVal)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,PushEx,3,(aaszName,aaszProp,aaszVal))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,PushEx,3)

/** ni -> iDataTableReadStack::PushFailEx/3 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,PushFailEx,3)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(achar*,aaszProp)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszProp)
	IDLC_DECL_VAR(achar*,aaszVal)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszVal)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,PushFailEx,3,(aaszName,aaszProp,aaszVal))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,PushFailEx,3)

/** ni -> iDataTableReadStack::Pop/0 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,Pop,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,Pop,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,Pop,0)

/** ni -> iDataTableReadStack::HasProperty/1 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,HasProperty,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,HasProperty,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,HasProperty,1)

/** ni -> iDataTableReadStack::RemoveProperty/1 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,RemoveProperty,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,RemoveProperty,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,RemoveProperty,1)

/** ni -> iDataTableReadStack::GetString/1 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetString,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetString,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetString,1)

/** ni -> iDataTableReadStack::GetStringDefault/2 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetStringDefault,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(achar*,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,v)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetStringDefault,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetStringDefault,2)

/** ni -> iDataTableReadStack::GetHString/1 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetHString,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(Ptr<iHString>,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetHString,1,(aaszName))
	IDLC_RET_FROM_INTF(,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetHString,1)

/** ni -> iDataTableReadStack::GetHStringDefault/2 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetHStringDefault,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(iHString*,v)
	IDLC_BUF_TO_INTF(iHString,v)
	IDLC_DECL_RETVAR(Ptr<iHString>,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetHStringDefault,2,(aaszName,v))
	IDLC_RET_FROM_INTF(,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetHStringDefault,2)

/** ni -> iDataTableReadStack::GetInt/1 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetInt,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(tI64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetInt,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_I64,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetInt,1)

/** ni -> iDataTableReadStack::GetIntDefault/2 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetIntDefault,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(tI64,v)
	IDLC_BUF_TO_BASE(ni::eType_I64,v)
	IDLC_DECL_RETVAR(tI64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetIntDefault,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_I64,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetIntDefault,2)

/** ni -> iDataTableReadStack::GetBool/1 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetBool,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetBool,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetBool,1)

/** ni -> iDataTableReadStack::GetBoolDefault/2 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetBoolDefault,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(tBool,v)
	IDLC_BUF_TO_BASE(ni::eType_I8,v)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetBoolDefault,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetBoolDefault,2)

/** ni -> iDataTableReadStack::GetFloat/1 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetFloat,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(tF64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetFloat,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_F64,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetFloat,1)

/** ni -> iDataTableReadStack::GetFloatDefault/2 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetFloatDefault,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(tF64,v)
	IDLC_BUF_TO_BASE(ni::eType_F64,v)
	IDLC_DECL_RETVAR(tF64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetFloatDefault,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_F64,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetFloatDefault,2)

/** ni -> iDataTableReadStack::GetVec2/1 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetVec2,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetVec2,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetVec2,1)

/** ni -> iDataTableReadStack::GetVec2Default/2 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetVec2Default,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(sVec2f,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,v)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetVec2Default,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetVec2Default,2)

/** ni -> iDataTableReadStack::GetVec3/1 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetVec3,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetVec3,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetVec3,1)

/** ni -> iDataTableReadStack::GetVec3Default/2 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetVec3Default,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(sVec3f,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,v)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetVec3Default,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetVec3Default,2)

/** ni -> iDataTableReadStack::GetVec4/1 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetVec4,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(sVec4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetVec4,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetVec4,1)

/** ni -> iDataTableReadStack::GetVec4Default/2 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetVec4Default,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(sVec4f,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,v)
	IDLC_DECL_RETVAR(sVec4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetVec4Default,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetVec4Default,2)

/** ni -> iDataTableReadStack::GetCol3/1 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetCol3,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetCol3,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetCol3,1)

/** ni -> iDataTableReadStack::GetCol4/1 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetCol4,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(sVec4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetCol4,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetCol4,1)

/** ni -> iDataTableReadStack::GetMatrix/1 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetMatrix,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(sMatrixf,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetMatrix,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_Matrixf,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetMatrix,1)

/** ni -> iDataTableReadStack::GetMatrixDefault/2 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetMatrixDefault,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(sMatrixf,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Matrixf|ni::eTypeFlags_Pointer,v)
	IDLC_DECL_RETVAR(sMatrixf,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetMatrixDefault,2,(aaszName,v))
	IDLC_RET_FROM_BASE(ni::eType_Matrixf,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetMatrixDefault,2)

/** ni -> iDataTableReadStack::GetIUnknown/1 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetIUnknown,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(iUnknown*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetIUnknown,1,(aaszName))
	IDLC_RET_FROM_INTF(iUnknown,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetIUnknown,1)

/** ni -> iDataTableReadStack::GetIUnknownDefault/2 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetIUnknownDefault,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(iHString*,v)
	IDLC_BUF_TO_INTF(iHString,v)
	IDLC_DECL_RETVAR(iUnknown*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetIUnknownDefault,2,(aaszName,v))
	IDLC_RET_FROM_INTF(iUnknown,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetIUnknownDefault,2)

/** ni -> iDataTableReadStack::GetEnum/3 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetEnum,3)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(sEnumDef*,apEnumDef)
	IDLC_BUF_TO_PTR(ni::eTypeFlags_Constant|ni::eType_Ptr|ni::eTypeFlags_Pointer,apEnumDef)
	IDLC_DECL_VAR(tEnumToStringFlags,aFlags)
	IDLC_BUF_TO_ENUM(tEnumToStringFlags,aFlags)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetEnum,3,(aaszName,apEnumDef,aFlags))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetEnum,3)

/** ni -> iDataTableReadStack::GetEnumDefault/4 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetEnumDefault,4)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(sEnumDef*,apEnumDef)
	IDLC_BUF_TO_PTR(ni::eTypeFlags_Constant|ni::eType_Ptr|ni::eTypeFlags_Pointer,apEnumDef)
	IDLC_DECL_VAR(tEnumToStringFlags,aFlags)
	IDLC_BUF_TO_ENUM(tEnumToStringFlags,aFlags)
	IDLC_DECL_VAR(tU32,v)
	IDLC_BUF_TO_BASE(ni::eType_U32,v)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetEnumDefault,4,(aaszName,apEnumDef,aFlags,v))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetEnumDefault,4)

/** ni -> iDataTableReadStack::GetMetadata/1 **/
IDLC_METH_BEGIN(ni,iDataTableReadStack,GetMetadata,1)
	IDLC_DECL_VAR(achar*,aaszProp)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszProp)
	IDLC_DECL_RETVAR(Ptr<iHString>,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableReadStack,GetMetadata,1,(aaszProp))
	IDLC_RET_FROM_INTF(,_Ret)
IDLC_METH_END(ni,iDataTableReadStack,GetMetadata,1)

IDLC_END_INTF(ni,iDataTableReadStack)

/** interface : iDataTableWriteStack **/
IDLC_BEGIN_INTF(ni,iDataTableWriteStack)
/** ni -> iDataTableWriteStack::GetTop/0 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,GetTop,0)
	IDLC_DECL_RETVAR(iDataTable*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableWriteStack,GetTop,0,())
	IDLC_RET_FROM_INTF(iDataTable,_Ret)
IDLC_METH_END(ni,iDataTableWriteStack,GetTop,0)

/** ni -> iDataTableWriteStack::SetName/1 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,SetName,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_METH_CALL_VOID(ni,iDataTableWriteStack,SetName,1,(aaszName))
IDLC_METH_END(ni,iDataTableWriteStack,SetName,1)

/** ni -> iDataTableWriteStack::GetName/0 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,GetName,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableWriteStack,GetName,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iDataTableWriteStack,GetName,0)

/** ni -> iDataTableWriteStack::GetNumChildren/0 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,GetNumChildren,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableWriteStack,GetNumChildren,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDataTableWriteStack,GetNumChildren,0)

/** ni -> iDataTableWriteStack::PushChild/1 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,PushChild,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableWriteStack,PushChild,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTableWriteStack,PushChild,1)

/** ni -> iDataTableWriteStack::PushChildFail/1 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,PushChildFail,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableWriteStack,PushChildFail,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTableWriteStack,PushChildFail,1)

/** ni -> iDataTableWriteStack::Push/1 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,Push,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableWriteStack,Push,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTableWriteStack,Push,1)

/** ni -> iDataTableWriteStack::PushFail/1 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,PushFail,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableWriteStack,PushFail,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTableWriteStack,PushFail,1)

/** ni -> iDataTableWriteStack::PushNew/1 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,PushNew,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableWriteStack,PushNew,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTableWriteStack,PushNew,1)

/** ni -> iDataTableWriteStack::PushAppend/1 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,PushAppend,1)
	IDLC_DECL_VAR(iDataTable*,apDT)
	IDLC_BUF_TO_INTF(iDataTable,apDT)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableWriteStack,PushAppend,1,(apDT))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTableWriteStack,PushAppend,1)

/** ni -> iDataTableWriteStack::PushEx/3 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,PushEx,3)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(achar*,aaszProp)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszProp)
	IDLC_DECL_VAR(achar*,aaszVal)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszVal)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableWriteStack,PushEx,3,(aaszName,aaszProp,aaszVal))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTableWriteStack,PushEx,3)

/** ni -> iDataTableWriteStack::PushFailEx/3 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,PushFailEx,3)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(achar*,aaszProp)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszProp)
	IDLC_DECL_VAR(achar*,aaszVal)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszVal)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableWriteStack,PushFailEx,3,(aaszName,aaszProp,aaszVal))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTableWriteStack,PushFailEx,3)

/** ni -> iDataTableWriteStack::Pop/0 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,Pop,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableWriteStack,Pop,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTableWriteStack,Pop,0)

/** ni -> iDataTableWriteStack::HasProperty/1 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,HasProperty,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableWriteStack,HasProperty,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTableWriteStack,HasProperty,1)

/** ni -> iDataTableWriteStack::RemoveProperty/1 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,RemoveProperty,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableWriteStack,RemoveProperty,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTableWriteStack,RemoveProperty,1)

/** ni -> iDataTableWriteStack::SetString/2 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,SetString,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(achar*,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,v)
	IDLC_METH_CALL_VOID(ni,iDataTableWriteStack,SetString,2,(aaszName,v))
IDLC_METH_END(ni,iDataTableWriteStack,SetString,2)

/** ni -> iDataTableWriteStack::SetHString/2 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,SetHString,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(iHString*,v)
	IDLC_BUF_TO_INTF(iHString,v)
	IDLC_METH_CALL_VOID(ni,iDataTableWriteStack,SetHString,2,(aaszName,v))
IDLC_METH_END(ni,iDataTableWriteStack,SetHString,2)

/** ni -> iDataTableWriteStack::SetInt/2 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,SetInt,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(tI64,v)
	IDLC_BUF_TO_BASE(ni::eType_I64,v)
	IDLC_METH_CALL_VOID(ni,iDataTableWriteStack,SetInt,2,(aaszName,v))
IDLC_METH_END(ni,iDataTableWriteStack,SetInt,2)

/** ni -> iDataTableWriteStack::SetBool/2 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,SetBool,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(tBool,v)
	IDLC_BUF_TO_BASE(ni::eType_I8,v)
	IDLC_METH_CALL_VOID(ni,iDataTableWriteStack,SetBool,2,(aaszName,v))
IDLC_METH_END(ni,iDataTableWriteStack,SetBool,2)

/** ni -> iDataTableWriteStack::SetFloat/2 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,SetFloat,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(tF64,v)
	IDLC_BUF_TO_BASE(ni::eType_F64,v)
	IDLC_METH_CALL_VOID(ni,iDataTableWriteStack,SetFloat,2,(aaszName,v))
IDLC_METH_END(ni,iDataTableWriteStack,SetFloat,2)

/** ni -> iDataTableWriteStack::SetVec2/2 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,SetVec2,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(sVec2f,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,v)
	IDLC_METH_CALL_VOID(ni,iDataTableWriteStack,SetVec2,2,(aaszName,v))
IDLC_METH_END(ni,iDataTableWriteStack,SetVec2,2)

/** ni -> iDataTableWriteStack::SetVec3/2 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,SetVec3,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(sVec3f,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec3f|ni::eTypeFlags_Pointer,v)
	IDLC_METH_CALL_VOID(ni,iDataTableWriteStack,SetVec3,2,(aaszName,v))
IDLC_METH_END(ni,iDataTableWriteStack,SetVec3,2)

/** ni -> iDataTableWriteStack::SetVec4/2 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,SetVec4,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(sVec4f,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,v)
	IDLC_METH_CALL_VOID(ni,iDataTableWriteStack,SetVec4,2,(aaszName,v))
IDLC_METH_END(ni,iDataTableWriteStack,SetVec4,2)

/** ni -> iDataTableWriteStack::SetMatrix/2 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,SetMatrix,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(sMatrixf,v)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Matrixf|ni::eTypeFlags_Pointer,v)
	IDLC_METH_CALL_VOID(ni,iDataTableWriteStack,SetMatrix,2,(aaszName,v))
IDLC_METH_END(ni,iDataTableWriteStack,SetMatrix,2)

/** ni -> iDataTableWriteStack::SetIUnknown/2 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,SetIUnknown,2)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(iUnknown*,v)
	IDLC_BUF_TO_INTF(iUnknown,v)
	IDLC_METH_CALL_VOID(ni,iDataTableWriteStack,SetIUnknown,2,(aaszName,v))
IDLC_METH_END(ni,iDataTableWriteStack,SetIUnknown,2)

/** ni -> iDataTableWriteStack::SetEnum/4 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,SetEnum,4)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(sEnumDef*,apEnumDef)
	IDLC_BUF_TO_PTR(ni::eTypeFlags_Constant|ni::eType_Ptr|ni::eTypeFlags_Pointer,apEnumDef)
	IDLC_DECL_VAR(tEnumToStringFlags,aFlags)
	IDLC_BUF_TO_ENUM(tEnumToStringFlags,aFlags)
	IDLC_DECL_VAR(tU32,anVal)
	IDLC_BUF_TO_BASE(ni::eType_U32,anVal)
	IDLC_METH_CALL_VOID(ni,iDataTableWriteStack,SetEnum,4,(aaszName,apEnumDef,aFlags,anVal))
IDLC_METH_END(ni,iDataTableWriteStack,SetEnum,4)

/** ni -> iDataTableWriteStack::SetMetadata/2 **/
IDLC_METH_BEGIN(ni,iDataTableWriteStack,SetMetadata,2)
	IDLC_DECL_VAR(achar*,aaszProp)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszProp)
	IDLC_DECL_VAR(iHString*,ahspData)
	IDLC_BUF_TO_INTF(iHString,ahspData)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDataTableWriteStack,SetMetadata,2,(aaszProp,ahspData))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDataTableWriteStack,SetMetadata,2)

IDLC_END_INTF(ni,iDataTableWriteStack)

IDLC_END_NAMESPACE()
// EOF //
