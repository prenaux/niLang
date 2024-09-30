// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"

#include "sqvm.h"
#include "sqobject.h"
#include "sqtable.h"
#include "sqfuncproto.h"
#include "sqclosure.h"
#include "sqarray.h"

///////////////////////////////////////////////
SQArray::SQArray(int nsize)
{
  _values.resize(nsize);
  INIT_CHAIN();
  ADD_TO_CHAIN(this);
  mnFlags = 0;
}

///////////////////////////////////////////////
SQArray::~SQArray()
{
  REMOVE_FROM_CHAIN(this);
}

///////////////////////////////////////////////
SQArray* SQArray::Create(int nInitialSize)
{
  return niNew SQArray(nInitialSize);
}

///////////////////////////////////////////////
void SQArray::Invalidate()
{
  mnFlags = 0;
  _values.clear();
  _values.resize(0);
}

///////////////////////////////////////////////
bool SQArray::Get(const int nidx,SQObjectPtr &val)
{
  if (nidx >= 0 && nidx < (int)_values.size())
  {
    val = _values[nidx];
    return true;
  }
  else
    return false;
}

///////////////////////////////////////////////
bool SQArray::Set(const int nidx,const SQObjectPtr &val)
{
  if (nidx >= 0 && nidx < (int)_values.size())
  {
    _values[nidx]=val;
    return true;
  }
  else
    return false;
}

///////////////////////////////////////////////
int SQArray::Next(const SQObjectPtr &refpos, SQObjectPtr &outkey, SQObjectPtr &outval)
{
  unsigned int idx = TranslateIndex(refpos);
  while (idx < _values.size())
  {
    //first found
    outkey=(SQInt)idx;
    outval=_values[idx];
    //return idx for the next iteration
    return ++idx;
  }

  //nothing to iterate anymore
  return -1;
}

///////////////////////////////////////////////
SQArray *SQArray::Clone(tSQDeepCloneGuardSet* apDeepClone)
{
  SQArray *anew = Create(Size());
  anew->_values = _values;
  return anew;
}

///////////////////////////////////////////////
int SQArray::Size() const
{
  return _values.size();
}

///////////////////////////////////////////////
int SQArray::Capacity() const
{
  return _values.capacity();
}

///////////////////////////////////////////////
void SQArray::Resize(int size,SQObjectPtr &fill)
{
  _values.resize(size,fill);
}

///////////////////////////////////////////////
void SQArray::Reserve(int size)
{
  _values.reserve(size);
}

///////////////////////////////////////////////
void SQArray::Append(const SQObject &o)
{
  _values.push_back(o);
}

///////////////////////////////////////////////
void SQArray::Extend(const SQArray *a)
{
  int xlen = a->Size();
  if (xlen)
  {
    for (int i = 0; i < xlen; ++i)
      Append(a->_values[i]);
  }
}

///////////////////////////////////////////////
#ifndef NO_GARBAGE_COLLECTOR
void SQArray::Mark(SQCollectable** chain)
{
  START_MARK();
  int len = _values.size();
  for (int i = 0;i < len; i++) {
    SQGarbageCollector::MarkObject(_values[i], chain);
  }
  END_MARK(chain);
}
#endif

///////////////////////////////////////////////
SQObjectPtr& SQArray::Top()
{
  return _values.back();
}

///////////////////////////////////////////////
void SQArray::Pop()
{
  _values.pop_back();
}

///////////////////////////////////////////////
void SQArray::Insert(const SQObject& idx,const SQObject &val)
{
  astl::insert_at(_values,toint(idx),val);
}

///////////////////////////////////////////////
void SQArray::Remove(unsigned int idx)
{
  astl::remove_at(_values,idx);
}

///////////////////////////////////////////////
void SQArray::Clear()
{
  _values.clear();
}
