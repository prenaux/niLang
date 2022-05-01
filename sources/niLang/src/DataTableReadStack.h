#ifndef __DATATABLEREADSTACK_31180938_H__
#define __DATATABLEREADSTACK_31180938_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "API/niLang/IDataTable.h"
#include <niLang/STL/stack.h>

namespace ni {

//////////////////////////////////////////////////////////////////////////////////////////////
// cDataTableReadStack declaration.
class cDataTableReadStack : public ni::cIUnknownImpl<ni::iDataTableReadStack,ni::eIUnknownImplFlags_Default>
{
  niBeginClass(cDataTableReadStack);

 public:
  //! Constructor.
  cDataTableReadStack(iDataTable* apDT);
  //! Destructor.
  ~cDataTableReadStack();

  //! Zeros all the class members.
  void ZeroMembers();
  //! Sanity check.
  ni::tBool __stdcall IsOK() const;

  //// ni::iDataTableReadStack /////////////////
  ni::iDataTable * __stdcall GetTop() const;
  const achar * __stdcall GetName() const;
  tU32 __stdcall GetNumChildren() const;
  tBool __stdcall PushChild(tU32 anIndex);
  tBool __stdcall PushChildFail(tU32 anIndex);
  tBool __stdcall Push(const achar *aaszName);
  tBool __stdcall PushFail(const achar *aaszName);
  tBool __stdcall PushEx(const achar* aaszName, const achar* aaszProp, const achar* aaszVal);
  tBool __stdcall PushFailEx(const achar* aaszName, const achar* aaszProp, const achar* aaszVal);
  tBool __stdcall Pop();
  tBool __stdcall RemoveProperty(const achar *aaszName);
  cString __stdcall GetString(const achar *aaszName) const;
  cString __stdcall GetStringDefault(const achar *aaszName, const achar *v) const;
  Ptr<iHString> __stdcall GetHString(const achar *aaszName) const;
  Ptr<iHString> __stdcall GetHStringDefault(const achar *aaszName, iHString *v) const;
  tI64 __stdcall GetInt(const achar *aaszName) const;
  tI64 __stdcall GetIntDefault(const achar *aaszName, tI64 v) const;
  tBool __stdcall GetBool(const achar *aaszName) const;
  tBool __stdcall GetBoolDefault(const achar *aaszName, tBool v) const;
  tF64 __stdcall GetFloat(const achar *aaszName) const;
  tF64 __stdcall GetFloatDefault(const achar *aaszName, tF64 v) const;
  sVec2f __stdcall GetVec2(const achar *aaszName) const;
  sVec2f __stdcall GetVec2Default(const achar *aaszName, const sVec2f &v) const;
  sVec3f __stdcall GetVec3(const achar *aaszName) const;
  sVec3f __stdcall GetVec3Default(const achar *aaszName, const sVec3f &v) const;
  sVec4f __stdcall GetVec4(const achar *aaszName) const;
  sVec4f __stdcall GetVec4Default(const achar *aaszName, const sVec4f &v) const;
  sVec3f __stdcall GetCol3(const achar *aaszName) const;
  sVec4f __stdcall GetCol4(const achar *aaszName) const;
  sMatrixf __stdcall GetMatrix(const achar *aaszName) const;
  sMatrixf __stdcall GetMatrixDefault(const achar *aaszName, const sMatrixf &v) const;
  iUnknown * __stdcall GetIUnknown(const achar *aaszName) const;
  iUnknown * __stdcall GetIUnknownDefault(const achar *aaszName, iHString *v) const;
  tU32 __stdcall GetEnum(const achar *aaszName, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags) const;
  tU32 __stdcall GetEnumDefault(const achar *aaszName, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags, tU32 v) const;
  Ptr<iHString> __stdcall GetMetadata(const achar* aaszProp) const {
    if (mStack.empty() || !mStack.top().IsOK())
      return NULL;
    return mStack.top()->GetMetadata(aaszProp);
  }
  tBool __stdcall HasProperty(const achar* aaszName) const {
    if (mStack.empty() || !mStack.top().IsOK())
      return eFalse;
    return mStack.top()->HasProperty(aaszName);
  }
  //// ni::iDataTableReadStack /////////////////

 private:
  astl::stack<Ptr<iDataTable> > mStack;
  niEndClass(cDataTableReadStack);
};

}
/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __DATATABLEREADSTACK_31180938_H__
