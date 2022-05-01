#ifndef _SQARRAY_H_
#define _SQARRAY_H_

#define SQ_ARRAY_FLAGS_WRITTING niBit(0)

struct SQArray : public SQCollectable, public SQ_ALLOCATOR(SQArray)
{
  virtual ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) {
    return this->_DoQueryInterface(this,aIID);
  }

private:
  SQArray(int nsize);
  ~SQArray();

public:
  static SQArray* Create(int nInitialSize);
#ifndef NO_GARBAGE_COLLECTOR
  void __stdcall Mark(SQCollectable **chain);
#endif
  void __stdcall Invalidate();
  bool Get(const int nidx,SQObjectPtr &val);
  bool Set(const int nidx,const SQObjectPtr &val);
  int Next(const SQObjectPtr &refpos,SQObjectPtr &outkey,SQObjectPtr &outval);
  SQArray *Clone(SQVM* apVM, tSQDeepCloneGuardSet* apDeepClone);
  int Size() const;
  int Capacity() const;
  void Resize(int size,SQObjectPtr &fill = _null_);
  void Reserve(int size);
  void Append(const SQObject &o);
  void Extend(const SQArray *a);
  SQObjectPtr &Top();
  void Pop();
  void Insert(const SQObject& idx,const SQObject &val);
  void Remove(unsigned int idx);
  void Clear();
  SQObjectPtrVec _values;

  tU32 mnFlags;
  __forceinline tBool __stdcall SerializeWriteLock() {
    if (niFlagIs(mnFlags,SQ_ARRAY_FLAGS_WRITTING))
      return ni::eFalse;
    niFlagOn(mnFlags,SQ_ARRAY_FLAGS_WRITTING);
    return ni::eTrue;
  }
  __forceinline tBool __stdcall SerializeWriteUnlock() {
    if (niFlagIsNot(mnFlags,SQ_ARRAY_FLAGS_WRITTING))
      return ni::eFalse;
    niFlagOff(mnFlags,SQ_ARRAY_FLAGS_WRITTING);
    return ni::eTrue;
  }
};
#endif //_SQARRAY_H_
