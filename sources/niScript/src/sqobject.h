#ifndef _SQOBJECT_H_
#define _SQOBJECT_H_

#include "sqvector.h"
#include "sqrefcounted.h"

#define SQ_CLOSURESTREAM_HEAD niFourCC('C','L','O','1')

struct SQSharedState;

enum SQMetaMethod{

#define MM_STD_TYPEOF _A("_typeof")
  MT_STD_TYPEOF = 0,
#define MM_STD_TOSTRING _A("_tostring")
  MT_STD_TOSTRING = 1,
#define MM_STD_CMP    _A("_cmp")
  MT_STD_CMP = 2,
#define MM_STD_CALL   _A("_call")
  MT_STD_CALL = 3,
#define MM_STD_NEXTI  _A("_nexti")
  MT_STD_NEXTI = 4,

#define MM_ARITH_ADD  _A("_add")
  MT_ARITH_ADD = 5,
#define MM_ARITH_SUB  _A("_sub")
  MT_ARITH_SUB = 6,
#define MM_ARITH_MUL  _A("_mul")
  MT_ARITH_MUL = 7,
#define MM_ARITH_DIV  _A("_div")
  MT_ARITH_DIV = 8,
#define MM_ARITH_UNM  _A("_unm")
  MT_ARITH_UNM = 9,
#define MM_ARITH_MODULO _A("_modulo")
  MT_ARITH_MODULO = 10,

#define MM_TABLE_INVALIDATE _A("_invalidate")
  MT_TABLE_INVALIDATE = 11,

#define MM_USERDATA_SET   _A("_set")
  MT_USERDATA_SET = 12,
#define MM_USERDATA_GET   _A("_get")
  MT_USERDATA_GET = 13,
#define MM_USERDATA_NEWSLOT _A("_newslot")
  MT_USERDATA_NEWSLOT = 14,
#define MM_USERDATA_DELSLOT _A("_delslot")
  MT_USERDATA_DELSLOT = 15,

#define MM_IUNKNOWN_CLONE _A("_clone")
  MT_IUNKNOWN_CLONE = 16,

  MT__LAST
};

#define MINPOWER2 4

#define _sqtype(obj) ((obj)._var.mType)

#define _int(obj) (obj)._var.mI32
#define _float(obj) (obj)._var.mF64
#define _collectable(obj) static_cast<SQCollectable*>((obj)._var.mpIUnknown)
#define _table(obj) static_cast<SQTable*>(_collectable(obj))
#define _array(obj) static_cast<SQArray*>(_collectable(obj))
#define _closure(obj) static_cast<SQClosure*>(_collectable(obj))
#define _nativeclosure(obj) static_cast<SQNativeClosure*>(_collectable(obj))
#define _userdata(obj) static_cast<SQUserData*>(_collectable(obj))
#define _funcproto(obj) static_cast<SQFunctionProto*>(_collectable(obj))
#define _rawval(obj) (obj)._var.mpPointer
#define _iunknown(obj) (obj)._var.mpIUnknown
#define _intptr(obj) (obj)._var.mIntPtr

#define tofloat(num)   ((_sqtype(num)==OT_INTEGER)?(SQFloat)_int(num):_float(num))
#define toint(num) ((_sqtype(num)==OT_FLOAT)?(SQInt)_float(num):_int(num))

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
#define _stringobj(obj)  static_cast<iHString*>(_iunknown(obj))
#define _stringhval(obj) _stringobj(obj)
#define _stringeq(a,b)   (_stringhval(a) == _stringhval(b))
#define _stringlen(obj)  _stringobj(obj)->GetLength()
#define _stringval(obj)  _stringobj(obj)->GetChars()
#define _stringle(a,b)   (_stringobj(a)->Cmp(_stringobj(b)) < 0)

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
struct SQObjectPtr : public SQObject
{
 private:
  SQObjectPtr(SQCollectable*) {}
  SQObjectPtr(ni::iCollectable*) {}

 public:
  __forceinline SQObjectPtr() {
    _VarDataSetType(_var,eType_Null);
  }
  __forceinline ~SQObjectPtr() {
    _VarDataRelease(_var);
  }
  __forceinline SQObjectPtr(SQTable *pTable)
  {
    niAssert(pTable != NULL);
    _VarDataSetType(_var,OT_TABLE);
    _VarDataSetAddRef(_var,(ni::iUnknown*)pTable);
  }
  __forceinline SQObjectPtr(SQArray *pArray)
  {
    niAssert(pArray != NULL);
    _VarDataSetType(_var, OT_ARRAY);
    _VarDataSetAddRef(_var,(ni::iUnknown*)pArray);
  }
  __forceinline SQObjectPtr(SQClosure *pClosure)
  {
    niAssert(pClosure != NULL);
    _VarDataSetType(_var, OT_CLOSURE);
    _VarDataSetAddRef(_var,(ni::iUnknown*)pClosure);
  }
  __forceinline SQObjectPtr(SQNativeClosure *pNativeClosure)
  {
    niAssert(pNativeClosure != NULL);
    _VarDataSetType(_var,OT_NATIVECLOSURE);
    _VarDataSetAddRef(_var,(ni::iUnknown*)pNativeClosure);
  }
  __forceinline SQObjectPtr(const tHStringPtr& ahspString)
  {
    niAssert(ahspString.IsOK());
    _VarDataSetType(_var,OT_STRING);
    _VarDataSetAddRef(_var,(ni::iUnknown*)ahspString.ptr());
  }
  __forceinline SQObjectPtr(const tHStringNN& ahspString)
  {
    _VarDataSetType(_var,OT_STRING);
    _VarDataSetAddRef(_var,(ni::iUnknown*)ahspString.raw_ptr());
  }
  __forceinline SQObjectPtr(const iHString* pString)
  {
    niAssert(pString != NULL);
    _VarDataSetType(_var,OT_STRING);
    _VarDataSetAddRef(_var,(ni::iUnknown*)pString);
  }
  __forceinline SQObjectPtr(SQUserData *pUserData)
  {
    niAssert(pUserData != NULL);
    _VarDataSetType(_var, OT_USERDATA);
    _VarDataSetAddRef(_var,(ni::iUnknown*)pUserData);
  }
  __forceinline SQObjectPtr(SQFunctionProto *pFunctionProto)
  {
    niAssert(pFunctionProto != NULL);
    _VarDataSetType(_var, OT_FUNCPROTO);
    _VarDataSetAddRef(_var,(ni::iUnknown*)pFunctionProto);
  }
  __forceinline SQObjectPtr(SQCollectable* apCollectable, tBool)
  {
    niAssert(apCollectable != NULL);
    _VarDataSetType(_var, OT_IUNKNOWN | eTypeFlags_Collectable);
    _VarDataSetAddRef(_var,apCollectable);
  }
  __forceinline SQObjectPtr(ni::iUnknown* apIUnknown)
  {
    niAssert(apIUnknown != NULL);
    _VarDataSetType(_var,OT_IUNKNOWN);
    _VarDataSetAddRef(_var,apIUnknown);
  }
  __forceinline SQObjectPtr(SQInt nInteger)
  {
    _VarDataSetType(_var,OT_INTEGER);
    _var.mI32 = nInteger;
  }
  __forceinline SQObjectPtr(SQFloat fFloat)
  {
    _VarDataSetType(_var,OT_FLOAT);
    niCAssert(sizeof(SQFloat) == sizeof(_var.mF64));
    _var.mF64 = fFloat;
  }

  // Assignment: both SQObjectPtr & SQObject constructors and
  // assignment operators are necessary otherwise the compiler uses
  // the default compiler generated bitwise copy version for
  // SQObjectPtr
  __forceinline SQObjectPtr(const SQObjectPtr &o) {
    _VarDataBitCopy(_var,o._var);
    _VarDataAddRef(_var);
  }
  __forceinline SQObjectPtr(const SQObject &o) {
    _VarDataBitCopy(_var,o._var);
    _VarDataAddRef(_var);
  }
  __forceinline SQObjectPtr& operator=(const SQObjectPtr& obj) {
    *this = (const SQObject&)obj;
    return *this;
  }
  __forceinline SQObjectPtr& operator=(const SQObject& obj) {
    if ((ni::tIntPtr)&obj != (ni::tIntPtr)this) {
      _VarDataAddRef((ni::VarData&)obj._var);
      _VarDataRelease(_var);
      _VarDataBitCopy(_var,obj._var);
    }
    return *this;
  }

  // Comparison
  __forceinline bool operator == (const SQObject& obj) const;
  __forceinline bool operator != (const SQObject& obj) const {
    return !(*this == obj);
  }

  // Some old C++ compilers are brain damaged and need this to not complain
  // about ambiguous conversions.
  __forceinline bool operator == (const SQObjectPtr& obj) const {
    return (*this == static_cast<const SQObject&>(obj));
  }
  __forceinline bool operator != (const SQObjectPtr& obj) const {
    return (*this != static_cast<const SQObject&>(obj));
  }

  __forceinline bool IsNull() const {
    return niType(_var.mType) == eType_Null;
  }

 private:
  SQObjectPtr(const SQChar *){} //safety

  static __forceinline void _VarDataSetType(ni::VarData& v, tType aType) {
    v.mType = aType;
  }
  static __forceinline void _VarDataSetAddRef(ni::VarData& v, ni::iUnknown* apObj) {
    v.mpIUnknown = apObj;
    if (v.mpIUnknown) {
      if (v.mType & eTypeFlags_Collectable) {
        down_cast<SQCollectable*>(v.mpIUnknown)->_CollectableAddRef();
      }
      else {
        v.mpIUnknown->AddRef();
      }
    }
  }
  static __forceinline void _VarDataAddRef(ni::VarData& v) {
    if (niType(v.mType) == eType_IUnknown) {
      niAssert(v.mType & eTypeFlags_Pointer);
      if (v.mpIUnknown) {
        if (v.mType & eTypeFlags_Collectable) {
          down_cast<SQCollectable*>(v.mpIUnknown)->_CollectableAddRef();
        }
        else {
          v.mpIUnknown->AddRef();
        }
      }
    }
  }
  static __forceinline void _VarDataRelease(ni::VarData& v) {
    if (niType(v.mType) == eType_IUnknown) {
      niAssert(v.mType & eTypeFlags_Pointer);
      if (v.mpIUnknown) {
        if (v.mType & eTypeFlags_Collectable) {
          down_cast<SQCollectable*>(v.mpIUnknown)->_CollectableRelease();
        }
        else {
          v.mpIUnknown->Release();
        }
      }
    }
  }
  static __forceinline void _VarDataBitCopy(ni::VarData& d, const ni::VarData& s) {
    d.mType = s.mType;
    d.mF64 = s.mF64; // copy the largest member, 8 bytes...
  }
};

niCAssert(sizeof(SQObjectPtr) == sizeof(ni::Var));
niCAssert(sizeof(SQObjectPtr) == sizeof(SQObject));

/////////////////////////////////////////////////////////////////////////////////////
struct SQUserData : public SQCollectable
{
  SQUserData();
  virtual ~SQUserData();

  void SetDelegate(SQTable *mt);
  SQTable* GetDelegate() const;
  SQUserData* Clone(SQVM* apVM, tSQDeepCloneGuardSet* apDeepClone);
  int CmpType(SQUserData* r) const {
    return CmpByVal(this->GetType(),r->GetType());
  }

  virtual int __stdcall GetSize() const = 0;
  virtual int __stdcall GetType() const = 0;
  virtual size_t __stdcall Hash() const = 0;
  virtual bool __stdcall Eq(SQUserData* r) const = 0;
  virtual int __stdcall Cmp(SQUserData* r) const = 0;
  virtual SQUserData* __stdcall CloneData(SQSharedState& aSS, tSQDeepCloneGuardSet* apDeepClone) const = 0;
  virtual cString __stdcall GetTypeString() const = 0;

 private:
#ifndef NO_GARBAGE_COLLECTOR
  virtual void __stdcall Mark(SQCollectable**);
#endif
  virtual void __stdcall Invalidate();

  SQObjectPtr mptrDelegate;
};

#define SQ_USERDATA_ALLOC(TYPE) ni::Impl_HeapAlloc
#define SQ_USERDATA_BASE(TYPE) public SQUserData, public SQ_USERDATA_ALLOC(TYPE)

/////////////////////////////////////////////////////////////////////////////////////
struct SQException
{
  SQException(const SQChar *str);
  SQException(const SQObjectPtr &desc);
  SQException(const SQException &b);
  SQObjectPtr _description;
};

ni::tU32 TranslateIndex(const SQObjectPtr &idx);

SQ_VECTOR_TYPEDEF(SQObjectPtr,SQObjectPtrVec);

SQ_VECTOR_TYPEDEF(int,SQIntVec);

ASTL_RAW_ALLOCATOR_IMPL(sqobjectptr);
typedef astl::list<SQObjectPtr,ASTL_ALLOCATOR(SQObjectPtr,sqobjectptr)>  SQObjectPtrLst;

__forceinline bool SQObjectPtr::operator == (const SQObject& obj) const
{
  if (_var.mType != obj._var.mType) {
    return false;
  }
  if (_var.mType == OT_STRING) {
    return _stringeq(*this,obj);
  }
  if (_var.mType == OT_USERDATA) {
    return _userdata(*this)->Eq(_userdata(obj));
  }
  return (ni::Var&)_var == (ni::Var&)obj._var;
}

niExportFunc(SQRESULT) sq_getiunknown(HSQUIRRELVM v,int idx,iUnknown** i);

tBool vm_string_nexti(iHString* hsp, const SQObjectPtr &refpos, SQObjectPtr &outkey, SQObjectPtr &outval, SQObjectPtr& itr);

struct SQObjectPtrSortByPtr {
  bool operator()(const SQObjectPtr& o1, const SQObjectPtr& o2) const {
    if (_sqtype(o1) == _sqtype(o2)) {
      return _intptr(o1) < _intptr(o2);
    }
    else {
      return _sqtype(o1) < _sqtype(o2);
    }
  }
};

struct SQObjectPtrSortLogical {
  bool operator()(const SQObjectPtr& o1, const SQObjectPtr& o2) const {
    if (_sqtype(o1) == _sqtype(o2)) {
      switch (_sqtype(o1)) {
        case OT_STRING:   return _stringle(o1,o2);
        case OT_INTEGER:  return _int(o1) < _int(o2);
        case OT_FLOAT:    return _float(o1) < _float(o2);
        case OT_USERDATA: return _userdata(o1)->Cmp(_userdata(o2)) < 0;
        default:          return _intptr(o1) < _intptr(o2);
      }
    }
    else {
      return _sqtype(o1) < _sqtype(o2);
    }
  }
};

ni::cString sq_typeof(HSQUIRRELVM v,int idx);

extern SQObjectPtr _null_;
extern SQObjectPtr _notnull_;
extern SQObjectPtr _one_;
extern SQObjectPtr _minusone_;

bool WriteObject(HSQUIRRELVM v,ni::tPtr up,SQWRITEFUNC write,const SQObjectPtr &o);
bool WriteSQClosure(SQClosure* _this, SQVM *v, ni::tPtr up, SQWRITEFUNC write);
bool WriteSQFunctionProto(SQFunctionProto* _this, SQVM *v, ni::tPtr up, SQWRITEFUNC write);

bool ReadObject(HSQUIRRELVM v,ni::tPtr up,SQREADFUNC read,SQObjectPtr &o);
bool ReadSQClosure(SQClosure* _this, SQVM *v,ni::tPtr up,SQREADFUNC read);
bool ReadSQFunctionProto(SQFunctionProto* _this, SQVM *v,ni::tPtr up,SQREADFUNC read);

#endif //_SQOBJECT_H_
