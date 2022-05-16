#include "stdafx.h"

#include "sqconfig.h"
#include "sqvm.h"
#include "sqobject.h"
#include "sqtable.h"

SQUserData::SQUserData() {
  mptrDelegate = _null_;
}
SQUserData::~SQUserData() {
}

void SQUserData::SetDelegate(SQTable *mt)
{
  mptrDelegate = mt;
}
SQTable* SQUserData::GetDelegate() const {
  return _table(mptrDelegate);
}
SQUserData* SQUserData::Clone(SQVM* apVM, tSQDeepCloneGuardSet* apDeepClone) {
  SQUserData* ud = this->CloneData(*apVM->_ss,apDeepClone);
  if (!mptrDelegate.IsNull()) {
    ud->SetDelegate(_table(mptrDelegate));
  }
  return ud;
}

#ifndef NO_GARBAGE_COLLECTOR
void __stdcall SQUserData::Mark(SQCollectable**) {
}
#endif

void __stdcall SQUserData::Invalidate() {
  SetDelegate(NULL);
}
