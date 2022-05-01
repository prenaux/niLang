#include "../API/niLang/ICollection.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iIterator **/
IDLC_BEGIN_INTF(ni,iIterator)
/** ni -> iIterator::GetCollection/0 **/
IDLC_METH_BEGIN(ni,iIterator,GetCollection,0)
	IDLC_DECL_RETVAR(iCollection*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iIterator,GetCollection,0,())
	IDLC_RET_FROM_INTF(iCollection,_Ret)
IDLC_METH_END(ni,iIterator,GetCollection,0)

/** ni -> iIterator::HasNext/0 **/
IDLC_METH_BEGIN(ni,iIterator,HasNext,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iIterator,HasNext,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iIterator,HasNext,0)

/** ni -> iIterator::Next/0 **/
IDLC_METH_BEGIN(ni,iIterator,Next,0)
	IDLC_DECL_RETREFVAR(Var&,_Ret,Var)
	IDLC_METH_CALL(_Ret,ni,iIterator,Next,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iIterator,Next,0)

/** ni -> iIterator::Key/0 **/
IDLC_METH_BEGIN(ni,iIterator,Key,0)
	IDLC_DECL_RETREFVAR(Var&,_Ret,Var)
	IDLC_METH_CALL(_Ret,ni,iIterator,Key,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iIterator,Key,0)

/** ni -> iIterator::Value/0 **/
IDLC_METH_BEGIN(ni,iIterator,Value,0)
	IDLC_DECL_RETREFVAR(Var&,_Ret,Var)
	IDLC_METH_CALL(_Ret,ni,iIterator,Value,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iIterator,Value,0)

IDLC_END_INTF(ni,iIterator)

/** interface : iCollection **/
IDLC_BEGIN_INTF(ni,iCollection)
/** ni -> iCollection::GetKeyType/0 **/
IDLC_METH_BEGIN(ni,iCollection,GetKeyType,0)
	IDLC_DECL_RETVAR(tType,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCollection,GetKeyType,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iCollection,GetKeyType,0)

/** ni -> iCollection::GetValueType/0 **/
IDLC_METH_BEGIN(ni,iCollection,GetValueType,0)
	IDLC_DECL_RETVAR(tType,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCollection,GetValueType,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iCollection,GetValueType,0)

/** ni -> iCollection::IsEmpty/0 **/
IDLC_METH_BEGIN(ni,iCollection,IsEmpty,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCollection,IsEmpty,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iCollection,IsEmpty,0)

/** ni -> iCollection::GetSize/0 **/
IDLC_METH_BEGIN(ni,iCollection,GetSize,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCollection,GetSize,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iCollection,GetSize,0)

/** ni -> iCollection::Contains/1 **/
IDLC_METH_BEGIN(ni,iCollection,Contains,1)
	IDLC_DECL_VAR(Var,aVar)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,aVar)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCollection,Contains,1,(aVar))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iCollection,Contains,1)

/** ni -> iCollection::ContainsAll/1 **/
IDLC_METH_BEGIN(ni,iCollection,ContainsAll,1)
	IDLC_DECL_VAR(iCollection*,apCollection)
	IDLC_BUF_TO_INTF(iCollection,apCollection)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCollection,ContainsAll,1,(apCollection))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iCollection,ContainsAll,1)

/** ni -> iCollection::Iterator/0 **/
IDLC_METH_BEGIN(ni,iCollection,Iterator,0)
	IDLC_DECL_RETVAR(iIterator*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCollection,Iterator,0,())
	IDLC_RET_FROM_INTF(iIterator,_Ret)
IDLC_METH_END(ni,iCollection,Iterator,0)

/** ni -> iCollection::Find/1 **/
IDLC_METH_BEGIN(ni,iCollection,Find,1)
	IDLC_DECL_VAR(Var,aVar)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,aVar)
	IDLC_DECL_RETVAR(iIterator*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCollection,Find,1,(aVar))
	IDLC_RET_FROM_INTF(iIterator,_Ret)
IDLC_METH_END(ni,iCollection,Find,1)

/** ni -> iCollection::Get/1 **/
IDLC_METH_BEGIN(ni,iCollection,Get,1)
	IDLC_DECL_VAR(Var,aKey)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,aKey)
	IDLC_DECL_RETVAR(Var,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCollection,Get,1,(aKey))
	IDLC_RET_FROM_BASE(ni::eType_Variant,_Ret)
IDLC_METH_END(ni,iCollection,Get,1)

/** ni -> iCollection::GetFirst/0 **/
IDLC_METH_BEGIN(ni,iCollection,GetFirst,0)
	IDLC_DECL_RETVAR(Var,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCollection,GetFirst,0,())
	IDLC_RET_FROM_BASE(ni::eType_Variant,_Ret)
IDLC_METH_END(ni,iCollection,GetFirst,0)

/** ni -> iCollection::GetLast/0 **/
IDLC_METH_BEGIN(ni,iCollection,GetLast,0)
	IDLC_DECL_RETVAR(Var,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCollection,GetLast,0,())
	IDLC_RET_FROM_BASE(ni::eType_Variant,_Ret)
IDLC_METH_END(ni,iCollection,GetLast,0)

IDLC_END_INTF(ni,iCollection)

/** interface : iMutableCollection **/
IDLC_BEGIN_INTF(ni,iMutableCollection)
/** ni -> iMutableCollection::Clear/0 **/
IDLC_METH_BEGIN(ni,iMutableCollection,Clear,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMutableCollection,Clear,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMutableCollection,Clear,0)

/** ni -> iMutableCollection::Copy/1 **/
IDLC_METH_BEGIN(ni,iMutableCollection,Copy,1)
	IDLC_DECL_VAR(iCollection*,apCollection)
	IDLC_BUF_TO_INTF(iCollection,apCollection)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMutableCollection,Copy,1,(apCollection))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMutableCollection,Copy,1)

/** ni -> iMutableCollection::Reserve/1 **/
IDLC_METH_BEGIN(ni,iMutableCollection,Reserve,1)
	IDLC_DECL_VAR(tU32,anSize)
	IDLC_BUF_TO_BASE(ni::eType_U32,anSize)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMutableCollection,Reserve,1,(anSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMutableCollection,Reserve,1)

/** ni -> iMutableCollection::Resize/1 **/
IDLC_METH_BEGIN(ni,iMutableCollection,Resize,1)
	IDLC_DECL_VAR(tU32,anSize)
	IDLC_BUF_TO_BASE(ni::eType_U32,anSize)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMutableCollection,Resize,1,(anSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMutableCollection,Resize,1)

/** ni -> iMutableCollection::Add/1 **/
IDLC_METH_BEGIN(ni,iMutableCollection,Add,1)
	IDLC_DECL_VAR(Var,aVar)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,aVar)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMutableCollection,Add,1,(aVar))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMutableCollection,Add,1)

/** ni -> iMutableCollection::AddAll/1 **/
IDLC_METH_BEGIN(ni,iMutableCollection,AddAll,1)
	IDLC_DECL_VAR(iCollection*,apCollection)
	IDLC_BUF_TO_INTF(iCollection,apCollection)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMutableCollection,AddAll,1,(apCollection))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMutableCollection,AddAll,1)

/** ni -> iMutableCollection::Remove/1 **/
IDLC_METH_BEGIN(ni,iMutableCollection,Remove,1)
	IDLC_DECL_VAR(Var,aVar)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,aVar)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMutableCollection,Remove,1,(aVar))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMutableCollection,Remove,1)

/** ni -> iMutableCollection::RemoveIterator/1 **/
IDLC_METH_BEGIN(ni,iMutableCollection,RemoveIterator,1)
	IDLC_DECL_VAR(iIterator*,apIterator)
	IDLC_BUF_TO_INTF(iIterator,apIterator)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMutableCollection,RemoveIterator,1,(apIterator))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMutableCollection,RemoveIterator,1)

/** ni -> iMutableCollection::RemoveAll/1 **/
IDLC_METH_BEGIN(ni,iMutableCollection,RemoveAll,1)
	IDLC_DECL_VAR(iCollection*,apCollection)
	IDLC_BUF_TO_INTF(iCollection,apCollection)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMutableCollection,RemoveAll,1,(apCollection))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMutableCollection,RemoveAll,1)

/** ni -> iMutableCollection::RemoveFirst/0 **/
IDLC_METH_BEGIN(ni,iMutableCollection,RemoveFirst,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMutableCollection,RemoveFirst,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMutableCollection,RemoveFirst,0)

/** ni -> iMutableCollection::RemoveLast/0 **/
IDLC_METH_BEGIN(ni,iMutableCollection,RemoveLast,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMutableCollection,RemoveLast,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMutableCollection,RemoveLast,0)

/** ni -> iMutableCollection::Put/2 **/
IDLC_METH_BEGIN(ni,iMutableCollection,Put,2)
	IDLC_DECL_VAR(Var,aKey)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,aKey)
	IDLC_DECL_VAR(Var,aValue)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,aValue)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMutableCollection,Put,2,(aKey,aValue))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMutableCollection,Put,2)

/** ni -> iMutableCollection::SetFirst/1 **/
IDLC_METH_BEGIN(ni,iMutableCollection,SetFirst,1)
	IDLC_DECL_VAR(Var,aValue)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,aValue)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMutableCollection,SetFirst,1,(aValue))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMutableCollection,SetFirst,1)

/** ni -> iMutableCollection::SetLast/1 **/
IDLC_METH_BEGIN(ni,iMutableCollection,SetLast,1)
	IDLC_DECL_VAR(Var,aValue)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,aValue)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMutableCollection,SetLast,1,(aValue))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iMutableCollection,SetLast,1)

/** ni -> iMutableCollection::GetDataPtr/0 **/
IDLC_METH_BEGIN(ni,iMutableCollection,GetDataPtr,0)
	IDLC_DECL_RETVAR(tPtr,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMutableCollection,GetDataPtr,0,())
	IDLC_RET_FROM_PTR(ni::eType_Ptr,_Ret)
IDLC_METH_END(ni,iMutableCollection,GetDataPtr,0)

/** ni -> iMutableCollection::GetDataSize/0 **/
IDLC_METH_BEGIN(ni,iMutableCollection,GetDataSize,0)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iMutableCollection,GetDataSize,0,())
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iMutableCollection,GetDataSize,0)

IDLC_END_INTF(ni,iMutableCollection)

IDLC_END_NAMESPACE()
// EOF //
