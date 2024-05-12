#ifndef __DATATABLEWRITESTACK_32651726_H__
#define __DATATABLEWRITESTACK_32651726_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "API/niLang/IDataTable.h"
#include <niLang/STL/stack.h>

namespace ni {

//////////////////////////////////////////////////////////////////////////////////////////////
// cDataTableWriteStack declaration.
class cDataTableWriteStack : public ni::ImplRC<ni::iDataTableWriteStack,ni::eImplFlags_Default>
{
  niBeginClass(cDataTableWriteStack);

 public:
  //! Constructor.
  cDataTableWriteStack(iDataTable* apDT);
  //! Constructor.
  cDataTableWriteStack(const achar* aaszName);
  //! Destructor.
  ~cDataTableWriteStack();

  //! Zeros all the class members.
  void ZeroMembers();
  //! Sanity check.
  ni::tBool __stdcall IsOK() const;

  //// ni::iDataTableWriteStack ////////////////
  ni::iDataTable * __stdcall GetTop() const;
  void __stdcall SetName(const achar* aaszName) const {
    if (mStack.top().IsOK()) {
      mStack.top()->SetName(aaszName);
    }
  }
  const achar * __stdcall GetName() const;
  tU32 __stdcall GetNumChildren() const;
  tBool __stdcall PushChild(tU32 anIndex);
  tBool __stdcall PushChildFail(tU32 anIndex);
  tBool __stdcall Push(const achar *aaszName);
  tBool __stdcall PushFail(const achar *aaszName);
  tBool __stdcall PushNew(const achar *aaszName);
  tBool __stdcall PushAppend(ni::iDataTable *apDT);
  tBool __stdcall PushEx(const achar* aaszName, const achar* aaszProp, const achar* aaszVal);
  tBool __stdcall PushFailEx(const achar* aaszName, const achar* aaszProp, const achar* aaszVal);
  tBool __stdcall Pop();
  tBool __stdcall RemoveProperty(const achar *aaszName);
  void __stdcall SetString(const achar *aaszName, const achar *v);
  void __stdcall SetHString(const achar *aaszName, iHString *v);
  void __stdcall SetInt(const achar *aaszName, tI64 v);
  void __stdcall SetBool(const achar *aaszName, tBool v);
  void __stdcall SetFloat(const achar *aaszName, tF64 v);
  void __stdcall SetVec2(const achar *aaszName, const sVec2f &v);
  void __stdcall SetVec3(const achar *aaszName, const sVec3f &v);
  void __stdcall SetVec4(const achar *aaszName, const sVec4f &v);
  void __stdcall SetMatrix(const achar *aaszName, const sMatrixf &v);
  void __stdcall SetIUnknown(const achar *aaszName, iUnknown *v);
  void __stdcall SetEnum(const achar *aaszName, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags, tU32 anVal);
  tBool __stdcall SetMetadata(const achar* aaszProp, iHString* ahspData) {
    if (mStack.empty() || !mStack.top().IsOK())
      return eFalse;
    return mStack.top()->SetMetadata(aaszProp,ahspData);
  }
  tBool __stdcall HasProperty(const achar* aaszName) const {
    if (mStack.empty() || !mStack.top().IsOK())
      return eFalse;
    return mStack.top()->HasProperty(aaszName);
  }
  //// ni::iDataTableWriteStack ////////////////

 private:
  astl::stack<Ptr<iDataTable> > mStack;
  niEndClass(cDataTableWriteStack);
};

}
/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __DATATABLEWRITESTACK_32651726_H__
