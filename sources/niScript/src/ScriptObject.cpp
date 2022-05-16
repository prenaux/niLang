// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"

#if niMinFeatures(15)

#include "ScriptVM.h"
#include "ScriptObject.h"
#include "ScriptTypes.h"
#include "sqvm.h"
#include "sqtable.h"

#ifdef SCRIPT_OBJECT_DEBUGID
tU32 gScriptObjectDebugID_Counter = 0;
tU32 gScriptObjectDebugID_Break = 0;
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// cScriptObject implementation.

///////////////////////////////////////////////
cScriptObject::cScriptObject(cScriptVM* apVM, int idx, int numpop, tBool abHoldRef)
    : mbHoldRef(abHoldRef)
{

  ZeroMembers();

#ifdef SCRIPT_OBJECT_DEBUGID
  ++gScriptObjectDebugID_Counter;
  if (gScriptObjectDebugID_Counter == 0) gScriptObjectDebugID_Counter = 1;
  mnDebugID = gScriptObjectDebugID_Counter;
  if (mnDebugID == gScriptObjectDebugID_Break)
  {
    ni_debug_break();
  }
#endif

  mpVM = apVM;

  if (!Get(idx,numpop))
  {
    niError(_A("Can't get the object."));
    mpVM = NULL;
    return;
  }
#ifdef SCRIPTOBJECT_INVALIDATEPOLICY
  mpVM->RegisterScriptObject(this);
#endif
}

///////////////////////////////////////////////
cScriptObject::~cScriptObject()
{
  Invalidate();
}

///////////////////////////////////////////////
void cScriptObject::ZeroMembers()
{
  sq_resetobject(&mObject);
  mType = eScriptObjectType_Last;
  mpVM = NULL;
  mnNumFreeVars = 0;
  mnNumParameters = 0;
}

///////////////////////////////////////////////
tBool __stdcall cScriptObject::IsOK() const
{
  niClassIsOK(cScriptObject);
  return niIsOK(mpVM);
}

///////////////////////////////////////////////
void __stdcall cScriptObject::Invalidate()
{
  if (!mpVM) return;
#ifdef SCRIPTOBJECT_INVALIDATEPOLICY
  mpVM->UnregisterScriptObject(this);
#endif
  Get(eInvalidHandle,0);
  mpVM = NULL;
}

///////////////////////////////////////////////
iScriptVM* cScriptObject::GetVM() const
{
  return mpVM;
}

///////////////////////////////////////////////
eScriptObjectType cScriptObject::GetType() const
{
  if (!IsOK()) return eScriptObjectType_Unknown;
  return mType;
}

///////////////////////////////////////////////
tBool cScriptObject::Push()
{
  if (!IsOK()) return eFalse;
  sq_pushobject(*mpVM, mObject);
  return eTrue;
}

///////////////////////////////////////////////
iScriptObject* __stdcall cScriptObject::Clone(tBool abHoldRef) const
{
  if (!IsOK()) return NULL;

  niSqGuard(*mpVM);

  sq_pushobject(*mpVM,mObject);
  iScriptObject* pObject = niNew cScriptObject(mpVM, -1, 1, abHoldRef);
  if (!niIsOK(pObject)) {
    niSqUnGuard(*mpVM);
    niSafeRelease(pObject);
    niError(_A("Can't get the cloned object."));
    return NULL;
  }

  niSqUnGuard(*mpVM);
  return pObject;
}

///////////////////////////////////////////////
//! Get the object from the specified index.
tBool __stdcall cScriptObject::Get(int idx, int numpop)
{
  if (!IsOK())
    return eFalse;

  HSQUIRRELVM vm = *mpVM;

  if (mType != eScriptObjectType_Last) {
    if (mbHoldRef)
      sq_release(*vm->_ss,&mObject);
  }

  if (idx == eInvalidHandle)
  {
    mType = eScriptObjectType_Last;
    return eTrue;
  }

  if (SQ_FAILED(sq_getstackobj(*mpVM, idx, &mObject)))
  {
    niError(_A("Can't get stack object."));
    return eFalse;
  }

  eScriptType sqType = sqa_getscripttype(*mpVM,idx);
  switch (sqType)
  {
    case eScriptType_Null:
      {
        mType = eScriptObjectType_Null;
        break;
      }
    case eScriptType_Int:
      {
        mType = eScriptObjectType_Int;
        break;
      }
    case eScriptType_Float:
      {
        mType = eScriptObjectType_Float;
        break;
      }
    case eScriptType_String:
      {
        mType = eScriptObjectType_String;
        break;
      }
    case eScriptType_Table:
      {
        mType = eScriptObjectType_Table;
        break;
      }
    case eScriptType_Array:
      {
        mType = eScriptObjectType_Array;
        break;
      }
    case eScriptType_Closure:
    case eScriptType_NativeClosure:
      {
        mType = eScriptObjectType_Function;
        sq_getclosureinfo(*mpVM,idx,(tU32*)&mnNumParameters,(tU32*)&mnNumFreeVars);
        niAssert(mnNumParameters >= 0);
        break;
      }
    case eScriptType_Vec2:
      {
        mType = eScriptObjectType_Vec2;
        break;
      }
    case eScriptType_Vec3:
      {
        mType = eScriptObjectType_Vec3;
        break;
      }
    case eScriptType_Vec4:
      {
        mType = eScriptObjectType_Vec4;
        break;
      }
    case eScriptType_Matrix:
      {
        mType = eScriptObjectType_Matrix;
        break;
      }
    case eScriptType_IUnknown:
      {
        mType = eScriptObjectType_IUnknown;
        break;
      }
    case eScriptType_UserData:
      {
        mType = eScriptObjectType_UserData;
        break;
      }
    case eScriptType_UUID: {
      mType = eScriptObjectType_UUID;
      break;
    }
  }

  if (mbHoldRef) {
    sq_addref(*vm->_ss,&mObject);
  }

  if (numpop > 0) {
    sq_pop(*mpVM, numpop);
  }

  return eTrue;
}

///////////////////////////////////////////////
//! Get the integer value.
//! \return niMaxI32 if the type of the object is not integer.
tI32 __stdcall cScriptObject::GetInt() const
{
  if (!IsInt()) return niMaxI32;
  return _int(mObject);
}

///////////////////////////////////////////////
//! Get the float value.
tF64 __stdcall cScriptObject::GetFloat() const
{
  if (!IsFloat()) return niMaxF64;
  return _float(mObject);
}

///////////////////////////////////////////////
const achar* __stdcall cScriptObject::GetString() const
{
  if (!IsString()) return AZEROSTR;
  return _stringval(mObject);
}

///////////////////////////////////////////////
iHString* __stdcall cScriptObject::GetHString() const
{
  if (!IsString()) return NULL;
  return _stringhval(mObject);
}

///////////////////////////////////////////////
iUnknown* __stdcall cScriptObject::GetIUnknown() const
{
  if (!IsIUnknown()) return NULL;
  return _iunknown(mObject);
  // sScriptTypeIUnknown* pClass = niStaticCast(sScriptTypeIUnknown*,_userdata(mObject));
  // return pClass->pObject;
}

///////////////////////////////////////////////
//! Get the IUnknown object, will get the interface also from iDispatch table.
iUnknown* __stdcall cScriptObject::GetIUnknownEx(const tUUID& aIID) const
{
  HSQUIRRELVM v = *mpVM;
  iUnknown* pObject = NULL;
  sq_pushobject(v,mObject);
  if (SQ_FAILED(sqa_getIUnknown(v,-1,&pObject,aIID)))
    return NULL;
  sq_pop(v,1);
  return pObject;
}

///////////////////////////////////////////////
sVec2f __stdcall cScriptObject::GetVec2() const
{
  if (!IsVec2()) return sVec2f::Zero();
  return static_cast<sScriptTypeVec2f*>(_userdata(mObject))->_val;
}

///////////////////////////////////////////////
sVec3f __stdcall cScriptObject::GetVec3() const
{
  if (!IsVec3()) return sVec3f::Zero();
  return static_cast<sScriptTypeVec3f*>(_userdata(mObject))->_val;
}

///////////////////////////////////////////////
sVec4f __stdcall cScriptObject::GetVec4() const
{
  if (!IsVec4()) return sVec4f::Zero();
  return static_cast<sScriptTypeVec4f*>(_userdata(mObject))->_val;
}

///////////////////////////////////////////////
sMatrixf __stdcall cScriptObject::GetMatrix() const
{
  if (!IsMatrix()) return sMatrixf::Identity();
  return static_cast<sScriptTypeMatrixf*>(_userdata(mObject))->_val;
}

///////////////////////////////////////////////
const tUUID& __stdcall cScriptObject::GetUUID() const
{
  if (!IsUUID()) return ni::kuuidZero;
  return static_cast<sScriptTypeUUID*>(_userdata(mObject))->mUUID;
}

///////////////////////////////////////////////
//! Get the number of parameters of the function.
tU32 __stdcall cScriptObject::GetNumParameters() const
{
  return mnNumParameters;
}

///////////////////////////////////////////////
//! Get the number of free variables of the function.
tU32 __stdcall cScriptObject::GetNumFreeVars() const
{
  return mnNumFreeVars;
}

///////////////////////////////////////////////
tU32 __stdcall cScriptObject::GetNumCallParameters() const
{
  niAssert(mnNumParameters>=mnNumFreeVars);
  return mnNumParameters-mnNumFreeVars;
}

///////////////////////////////////////////////
//! Enumerate all objects in this table.
tScriptObjectPtrCVec* __stdcall cScriptObject::EnumObjects() const
{
#pragma niTodo("Add a parameter to specify if it's required to enumerate delegate's objects")
#pragma niTodo("Add a parameter to specify if it's the hidden (starting with __) keys should be enumerated")
#pragma niTodo("Add delegate objects only if they arent already in the list")

  if (GetType() != eScriptObjectType_Table)
    return NULL;

  tScriptObjectPtrCVec* pLst = tScriptObjectPtrCVec::Create();
  niAssert(pLst != NULL);

  HSQUIRRELVM v = *mpVM;

  sq_pushobject(v,mObject); // Push this
  sq_pushnull(v); // NULL iterator
  while (SQ_SUCCEEDED(sq_next(v,-2)))
  {
    const SQChar* aszKey = NULL;
    if (sq_gettype(v,-2) == OT_STRING &&
        SQ_SUCCEEDED(sq_getstring(v,-2,&aszKey)) &&
        StrNCmp(aszKey,_A("__"),2) == 0)
    {
      sq_pop(v,2);  // pop key and value
      continue;
    }
    pLst->Add(niNew cScriptObject(mpVM,-2,0));
    pLst->Add(niNew cScriptObject(mpVM,-1,0));
    sq_pop(v,2);  // pop key and value
  }
  sq_pop(v,1);  // Pop the NULL iterator

  int numDelegateToPop = 0;
  while (SQ_SUCCEEDED(sq_getdelegate(v,-1)))
  {
    ++numDelegateToPop;
    sq_pushnull(v); // NULL iterator
    while (SQ_SUCCEEDED(sq_next(v,-2)))
    {
      const SQChar* aszKey = NULL;
      if (SQ_SUCCEEDED(sq_getstring(v,-2,&aszKey)) && StrNCmp(aszKey,_A("__"),2) == 0)
      {
        sq_pop(v,2);  // pop key and value
        continue;
      }
      pLst->Add(niNew cScriptObject(mpVM,-2,0));
      pLst->Add(niNew cScriptObject(mpVM,-1,0));
      sq_pop(v,2);  // pop key and value
    }
    sq_pop(v,1);  // Pop the NULL iterator
  }

  sq_pop(v,1+numDelegateToPop); // Pop the delegates and the table
  return pLst;
}

#endif // niMinFeatures
