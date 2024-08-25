#include "stdafx.h"

#include "sqconfig.h"
#include "sqopcodes.h"
#include "sqfuncproto.h"
#include "sqvm.h"
#include "sqclosure.h"
#include "sqtable.h"
#include "sqarray.h"
#include "ScriptVM.h"
#include "ScriptTypes.h"
#include "ScriptAutomation.h"
#include <niLang/Utils/ObjectInterfaceCast.h>

//#define EXCEPTION_DEBUG
// #define LAST_CALLSTACK

#ifdef EXCEPTION_DEBUG
#pragma niTodoX("Exception Debug enabled.")
#endif

#ifdef LAST_CALLSTACK
cString _strLastCallStack;
#endif

cString sq_getcallinfo_string(HSQUIRRELVM v, int level);

#pragma niTodo("DeepClone can get in infinite loop because of some internal cycles, fix this.")

SQVM::SQVM(SQSharedState* aSS)
{
  niAssert(aSS != NULL);
  _ss = aSS;
  _foreignptr = NULL;
  _nnativecalls = 0;
  _lasterror = _null_;
  _errorhandler = _null_;
  _debughook = _null_;
  _raiseErrorMode = eScriptRaiseErrorMode_ShortCallstack;
#ifdef _DEBUG
  _logRaiseError = eLogFlags_Debug;
#else
  _logRaiseError = 0;
#endif
  INIT_CHAIN();
  ADD_TO_CHAIN(this);
  _emptystring = _H("");
  _execDumpStack = !!ni::GetLang()->GetProperty("niScript.ExecDumpStack").Bool();
}

void SQVM::Invalidate()
{
  _lasterror = _null_;
  _errorhandler = _null_;
  _debughook = _null_;
  int size = _stack.size();
  for (int i = size-1; i>=0; --i)
    _stack[i] = _null_;
  _roottable = _null_;
}

SQVM::~SQVM()
{
  Invalidate();
  REMOVE_FROM_CHAIN(this);
}

SQObjectPtr SQVM::PrintObjVal(const SQObject &o)
{
  switch(_sqtype(o)) {
    case OT_STRING: {
      return _stringobj(o);
    }
    case OT_INTEGER: {
      return _H(niFmt(_A("%d"),_int(o)));
    }
    case OT_FLOAT: {
      cString out;
      SQVM_CatFloatToString(out,_float(o));
      return _H(out);
    }
    default:
      return _ss->GetTypeNameObj(o);
  }
}

void SQVM::Raise_ObjError(const SQObjectPtr &desc, bool aPrint)
{
  _lasterror = desc;
  if (aPrint && (_raiseErrorMode > eScriptRaiseErrorMode_Error)) {
    cString str = AEOL;
    sqFormatError(this, _sqtype(desc)==OT_STRING?_stringval(desc):NULL,str,0);
    niLog(Raw,str.Chars());
  }
}

void SQVM::Raise_MsgError(const cString& s)
{
  if (_raiseErrorMode > eScriptRaiseErrorMode_Error) {
    cString str = AEOL;
    sqFormatError(this,s.Chars(),str,0);
    niLog(Raw,str.Chars());
    _lasterror = _H(str);
  }
  else {
    _lasterror = _H(s);
  }
}

void SQVM::Raise_IdxError(const SQObject &o)
{
  SQObjectPtr oval = PrintObjVal(o);
  Raise_MsgError(niFmt("the index '%.50s' does not exist", _stringval(oval)));
}

void SQVM::Raise_CompareError(const SQObject &o1, const SQObject &o2)
{
  SQObjectPtr oval1 = PrintObjVal(o1), oval2 = PrintObjVal(o2);
  Raise_MsgError(niFmt(_A("compare between '%.50s' and '%.50s'"), _stringval(oval1), _stringval(oval2)));
}

void SQVM::Raise_ParamTypeError(int nparam,int typemask,int type)
{
  Raise_MsgError(niFmt("parameter %d has an invalid type '%s' ; expected: '%s'",
                       nparam,
                       sqa_gettypestring(type),
                       sqa_gettypestring(typemask)));
}

#define CALL_SYSTEM_DELEGATE_MM(TYPE,MM,NUM_ARGS,DEST)                  \
  CallMetaMethod(_table(_ss->_##TYPE##_default_delegate),MM,NUM_ARGS,DEST)

bool SQVM::ArithMetaMethod(int op, const SQObjectPtr & o1, const SQObjectPtr & o2, SQObjectPtr& dest)
{
  SQMetaMethod mm;
  switch (op) {
    case _A('+'): mm = MT_ARITH_ADD; break;
    case _A('-'): mm = MT_ARITH_SUB; break;
    case _A('/'): mm = MT_ARITH_DIV; break;
    case _A('*'): mm = MT_ARITH_MUL; break;
    default:
      niAssertUnreachable("Unreachable");
      return false;
  }
  switch (_sqtype(o1)) {
    case OT_TABLE:
      if (_table(o1)->CanCallMetaMethod()) {
        Push(o1);
        Push(o2);
        if (CallMetaMethod(_table(o1), mm, 2, dest)) {
          return true;
        }
      }
      Push(o1);
      Push(o2);
      return CALL_SYSTEM_DELEGATE_MM(table,mm,2,dest);
    case OT_ARRAY:
      Push(o1);
      Push(o2);
      return CALL_SYSTEM_DELEGATE_MM(array,mm,2,dest);
    case OT_STRING:
      Push(o1);
      Push(o2);
      return CALL_SYSTEM_DELEGATE_MM(string,mm,2,dest);
    case OT_USERDATA:
      if (_userdata(o1)->GetDelegate()) {
        Push(o1);
        Push(o2);
        return CallMetaMethod(_userdata(o1)->GetDelegate(), mm, 2, dest);
      }
      break;
    case OT_IUNKNOWN: {
      Push(o1);
      Push(o2);
      return CallIUnknownMetaMethod(_iunknown(o1), mm, 2, dest);
    }
  }
  return false;
}

bool SQVM::Modulo(const SQObjectPtr & o1, const SQObjectPtr & o2, SQObjectPtr& dest)
{
  if (sq_isnumeric(o1) && sq_isnumeric(o2))
  {
    if ((_sqtype(o1) == OT_INTEGER) && (_sqtype(o2) == OT_INTEGER))
    {
      if (_int(o2) == 0) {
        VM_ERRORB(_A("int modulo by zero"));
      }
      dest = SQInt(_int(o1)% _int(o2));
    }
    else
    {
      const SQFloat a = tofloat(o1);
      const SQFloat b = tofloat(o2);
      if (b == 0) {
        // dest = SQFloat(0);
        VM_ERRORB(_A("float modulo by zero"));
      }
      dest = SQFloat(ni::FMod(a,b));
    }
  }
  else
  {
    switch (_sqtype(o1)) {
      case OT_TABLE:
        if (_table(o1)->CanCallMetaMethod()) {
          Push(o1);
          Push(o2);
          if (CallMetaMethod(_table(o1), MT_ARITH_MODULO, 2, dest))
            return true;
        }
        Push(o1);
        Push(o2);
        if (CALL_SYSTEM_DELEGATE_MM(table,MT_ARITH_MODULO,2,dest))
          return true;
        break;
      case OT_ARRAY:
        Push(o1);
        Push(o2);
        if (CALL_SYSTEM_DELEGATE_MM(array,MT_ARITH_MODULO,2,dest))
          return true;
        break;
      case OT_STRING:
        Push(o1);
        Push(o2);
        if (CALL_SYSTEM_DELEGATE_MM(string,MT_ARITH_MODULO,2,dest))
          return true;
        break;
      case OT_USERDATA:
        if (_userdata(o1)->GetDelegate()) {
          Push(o1);
          Push(o2);
          if (CallMetaMethod(_userdata(o1)->GetDelegate(), MT_ARITH_MODULO, 2, dest))
            return true;
        }
        break;
      case OT_IUNKNOWN: {
        Push(o1);
        Push(o2);
        if (CallIUnknownMetaMethod(_iunknown(o1), MT_ARITH_MODULO, 2, dest))
          return true;
        break;
      }
    }
    VM_ERRORB(niFmt(_A("modulo between '%s' and '%s'"),
                    _ss->GetTypeNameStr(o1),
                    _ss->GetTypeNameStr(o2)));
  }
  return true;
}

bool SQVM::Div(const SQObjectPtr & o1, const SQObjectPtr & o2, SQObjectPtr& dest)
{
  if (sq_isnumeric(o1) && sq_isnumeric(o2))
  {
    if ((_sqtype(o1) == OT_INTEGER) && (_sqtype(o2) == OT_INTEGER))
    {
      if (_int(o2) == 0) {
        VM_ERRORB(_A("int division by zero"));
      }
      dest = SQInt(_int(o1) / _int(o2));
    }
    else
    {
      const double a = tofloat(o1);
      const double b = tofloat(o2);
      if (b == 0) {
        VM_ERRORB(_A("float division by zero"));
        //                 dest = SQFloat(1);
      }
      dest = SQFloat(a/b);
    }
  }
  else
  {
    switch (_sqtype(o1))
    {
      case OT_TABLE:
        if (_table(o1)->CanCallMetaMethod()) {
          Push(o1);
          Push(o2);
          if (CallMetaMethod(_table(o1), MT_ARITH_DIV, 2, dest))
            return true;
        }
        Push(o1);
        Push(o2);
        if (CALL_SYSTEM_DELEGATE_MM(table,MT_ARITH_DIV,2,dest))
          return true;
        break;
      case OT_ARRAY:
        Push(o1);
        Push(o2);
        if (CALL_SYSTEM_DELEGATE_MM(array,MT_ARITH_DIV,2,dest))
          return true;
        break;
      case OT_STRING:
        Push(o1);
        Push(o2);
        if (CALL_SYSTEM_DELEGATE_MM(string,MT_ARITH_DIV,2,dest))
          return true;
        break;
      case OT_USERDATA:
        if (_userdata(o1)->GetDelegate()) {
          Push(o1);
          Push(o2);
          if (CallMetaMethod(_userdata(o1)->GetDelegate(), MT_ARITH_DIV, 2, dest))
            return true;
        }
        break;
      case OT_IUNKNOWN: {
        Push(o1);
        Push(o2);
        if (CallIUnknownMetaMethod(_iunknown(o1), MT_ARITH_DIV, 2, dest))
          return true;
      }
    }
    VM_ERRORB(niFmt(_A("division between '%s' and '%s'"),
                    _ss->GetTypeNameStr(o1),
                    _ss->GetTypeNameStr(o2)));
  }
  return true;
}

bool SQVM::ObjEq(const SQObjectPtr & o1, const SQObjectPtr & o2, int& result)
{
  if (_sqtype(o1) == _sqtype(o2))
  {
    SQObjectPtr res = SQObjectPtr();
    switch (_sqtype(o1)) {
      case OT_NULL:
        result = 0;
        return true;
      case OT_STRING:
        result = (_stringhval(o1) == _stringhval(o2))?0:1;
        return true;
      case OT_USERDATA:
        result = (_userdata(o1)->Eq(_userdata(o2)))?0:1;
        return true;
      case OT_IUNKNOWN:
        if (_iunknown(o1) == _iunknown(o2))
          return true;
        // if reference (pointer) not equal we have to try to call the metamethod...
        return ObjCmp(o1,o2,result);
    }
  }
  return ObjCmp(o1,o2,result);
}

bool SQVM::ObjCmp(const SQObjectPtr & o1, const SQObjectPtr & o2, int& result)
{
  if (_sqtype(o1) == OT_IUNKNOWN) {
    SQObjectPtr closure;
    if (GetIUnknownMetaMethod(_iunknown(o1), MT_STD_CMP, closure)) {
      SQObjectPtr res = SQObjectPtr();
      Push(o1);
      Push(o2);
      if (!CallIUnknownMetaMethod(_iunknown(o1), closure, 2, res) ||
          _sqtype(res) != OT_INTEGER)
      {
        Raise_CompareError(o1, o2);
        return false;
      }
      result = _int(res);
      return true;
    }
    else {
      result = CmpByVal((tIntPtr)_iunknown(o1), (tIntPtr)_iunknown(o2));
      return true;
    }
  }
  else if (_sqtype(o1) == _sqtype(o2))
  {
    SQObjectPtr res = SQObjectPtr();
    switch (_sqtype(o1)) {
      case OT_NULL:
        result = 0;
        return true;
      case OT_STRING: {
        result = StrCmp(_stringval(o1), _stringval(o2));
        return true;
      }
      case OT_INTEGER:
        result = CmpByVal(_int(o1), _int(o2));
        return true;
      case OT_FLOAT:
        result = CmpByVal(_float(o1), _float(o2));
        return true;
      case OT_TABLE:
        if (_table(o1)->CanCallMetaMethod()) {
          Push(o1);
          Push(o2);
          CallMetaMethod(_table(o1), MT_STD_CMP, 2, res);
        }
        if (_sqtype(res) != OT_INTEGER) {
          res = CmpByVal(_table(o1), _table(o2));
        }
        break;
      case OT_USERDATA:
        Push(o1);
        Push(o2);
        if (_userdata(o1)->GetDelegate()) {
          CallMetaMethod(_userdata(o1)->GetDelegate(), MT_STD_CMP, 2, res);
        }
        if (_sqtype(res) != OT_INTEGER) {
          res = _userdata(o1)->Cmp(_userdata(o2));
        }
        break;
    }
    if (_sqtype(res) != OT_INTEGER) {
      Raise_CompareError(o1, o2);
      return false;
    }
    result = _int(res);
    return true;
  }
  else
  {
    if (sq_isnumeric(o1) && sq_isnumeric(o2)) {
      if ((_sqtype(o1) == OT_INTEGER) && (_sqtype(o2) == OT_FLOAT)) {
        result = CmpByVal((double)_int(o1),(double)_float(o2));
        return true;
      }
      else {
        result = CmpByVal((double)_float(o1),(double)_int(o2));
        return true;
      }
    }
    else if (_sqtype(o1) == OT_NULL) {
      result = -1;
      return true;
    }
    else if (_sqtype(o2) == OT_NULL) {
      result = 1;
      return true;
    }
    else {
      result = CmpByVal(_sqtype(o1), _sqtype(o2));
      return true;
    }
  }
}

bool SQVM::StringCat(const SQObjectPtr & str, const SQObjectPtr & obj, SQObjectPtr& dest)
{
  switch (_sqtype(obj))
  {
    case OT_STRING:
      {
        cString newStr;
        switch (_sqtype(str))
        {
          case OT_STRING:
            {
              newStr  = _stringval(str);
              newStr += _stringval(obj);
              break;
            }
          case OT_FLOAT: {
            SQVM_CatFloatToString(newStr, _float(str));
            newStr += _stringval(obj);
            break;
          }
          case OT_INTEGER:
            newStr.Format(_A("%d%s"), _int(str), _stringval(obj));
            break;
          case OT_NULL:
            newStr.Format(_A("(null)%s"), _stringval(obj));
            break;
          default:
            VM_ERRORB(niFmt(_A("string concatenation between '%s' and '%s'"),
                            _ss->GetTypeNameStr(str),
                            _ss->GetTypeNameStr(obj)));
        }
        dest = _H(newStr);
        break;
      }
    case OT_NULL:
      dest = _H(niFmt("%s(null)",_stringval(str)));
      break;
    case OT_FLOAT: {
      cString newStr;
      newStr += _stringval(str);
      SQVM_CatFloatToString(newStr, _float(obj));
      dest = _H(newStr);
      break;
    }
    case OT_INTEGER:
      dest = _H(niFmt(_A("%s%d"),_stringval(str),_int(obj)));
      break;
    case OT_IUNKNOWN: {
      cString newStr;
      newStr  = _stringval(str);
      iUnknown* p = _iunknown(obj);
      iHString* hsp = niGetIUnknownHString(p);
      if (hsp) {
        newStr += niHStr(hsp);
      }
      else {
        Ptr<iToString> ptrToString = ni::QueryInterface<iToString>(p);
        if (ptrToString.IsOK()) {
          newStr << ptrToString.ptr();
        }
        else {
          SQObjectPtr res;
          Push(obj);
          if (CallIUnknownMetaMethod(_iunknown(obj), MT_STD_TOSTRING, 1, res) &&
              _sqtype(res) == OT_STRING)
          {
            newStr << _stringval(res);
          }
          else {
            cString strUnknown;
            iunknown_gettype_concat(strUnknown,this,_iunknown(obj));
            newStr << niFmt("<%s>@%p",strUnknown.Chars(),(tIntPtr)_iunknown(obj));
          }
        }
      }
      dest = _H(newStr);
      break;
    }
    case OT_USERDATA:
      if (_userdata(obj)->GetDelegate())
      {
        SQObjectPtr res;
        Push(obj);
        if (CallMetaMethod(_userdata(obj)->GetDelegate(), MT_STD_TOSTRING, 1, res))
        {
          if (_sqtype(res) != OT_STRING)
          {
            VM_ERRORB(niFmt(_A("string concatenation between '%s' and userdata, _tostring returned a '%s' not a string."), _ss->GetTypeNameStr(str), _ss->GetTypeNameStr(obj), _ss->GetTypeNameStr(res)));
          }
          else
          {
            cString newStr;
            newStr  = _stringval(str);
            newStr += _stringval(res);
            dest = _H(newStr);
            break;
          }
        }
      }
      VM_ERRORB(niFmt(_A("string concatenation between '%s' and userdata without _tostring "), _ss->GetTypeNameStr(str), _ss->GetTypeNameStr(obj)));
      break;
    case OT_TABLE: {
      if (_table(obj)->CanCallMetaMethod()) {
        SQObjectPtr res;
        Push(obj);
        if (CallMetaMethod(_table(obj), MT_STD_TOSTRING, 1, res)) {
          if (_sqtype(res) != OT_STRING) {
            VM_ERRORB(niFmt(_A("string concatenation between '%s' and table, _tostring returned a '%s' not a string."), _ss->GetTypeNameStr(str), _ss->GetTypeNameStr(obj), _ss->GetTypeNameStr(res)));
          }
          else
          {
            cString newStr;
            newStr  = _stringval(str);
            newStr += _stringval(res);
            dest = _H(newStr);
          }
          break;
        }
      }
      VM_ERRORB(niFmt(_A("string concatenation between '%s' and table"),
                      _ss->GetTypeNameStr(str)));
      break;
    }
    default:
      VM_ERRORB(niFmt(_A("string concatenation between '%s' and '%s'"),
                      _ss->GetTypeNameStr(str), _ss->GetTypeNameStr(obj)));
  }
  return true;
}

void SQVM::TypeOf(const SQObjectPtr & obj1, SQObjectPtr& dest)
{
  switch (_sqtype(obj1))
  {
    case OT_TABLE:
      if (_table(obj1)->CanCallMetaMethod()) {
        Push(obj1);
        if (CallMetaMethod(_table(obj1), MT_STD_TYPEOF, 1, dest))
          return;
      }
      break;
    case OT_USERDATA:
      if (_userdata(obj1)->GetDelegate())
      {
        Push(obj1);
        if (CallMetaMethod(_userdata(obj1)->GetDelegate(), MT_STD_TYPEOF, 1, dest))
          return;
      }
  }
  dest = _ss->GetTypeNameObj(obj1);
}

bool SQVM::Init(bool abInitRootTable)
{
  _stack.resize(16,_null_);
  _callsstack.reserve(16);
  _stackbase = 0;
  _top = 0;
  if (abInitRootTable) {
    _roottable = SQTable::Create();
    RegisterSQRegFunctions(
      as_nn(_table(_roottable)),
      SQSharedState::_base_funcs);
  }
  else {
    _roottable = _null_;
  }
  return true;
}

__forceinline tBool _IsClonableType(const tU32 aType) {
  return aType == OT_TABLE || aType == OT_ARRAY || aType == OT_STRING || aType == OT_USERDATA;
}

bool SQVM::Clone(const SQObjectPtr &self,SQObjectPtr &target, tSQDeepCloneGuardSet* apDeepClone)
{
  SQObjectPtr temp_reg;

  if (apDeepClone && (_sqtype(self) == OT_TABLE || _sqtype(self) == OT_ARRAY)) {
    ++apDeepClone->_depth;
    if (apDeepClone->_depth > 50) {
      niAssertUnreachable("Deep clone maximum depth");
      VM_ERRORB("Deep clone maximum depth");
    }
  }
  bool ret = true;

  switch(_sqtype(self))
  {
    case OT_TABLE: {
      target = _table(self)->Clone(this,apDeepClone);
      if (apDeepClone) {
        for (SQTable::tHMapIt it = _table(target)->GetHMap().begin();
             it != _table(target)->GetHMap().end(); ++it)
        {
          SQObjectPtr& p = it->second;
          if (!_IsClonableType(_sqtype(p)))
            continue;
          if (!Clone(p,p,apDeepClone)) {
            VM_ERRORB(_A("Can't deep clone table member."));
          }
          if (_sqtype(p) == OT_TABLE) _table(p)->SetParent(_table(target));
        }
      }
      ret = true;
      break;
    }
    case OT_USERDATA: {
      target = _userdata(self)->Clone(this,apDeepClone);
      ret = true;
      break;
    }
    case OT_ARRAY: {
      target = _array(self)->Clone(this,apDeepClone);
      if (apDeepClone) {
        for (SQObjectPtrVec::iterator it = _array(target)->_values.begin();
             it != _array(target)->_values.end(); ++it)
        {
          SQObjectPtr& p = *it;
          if (!_IsClonableType(_sqtype(p)))
            continue;
          if (!Clone(p,p,apDeepClone)) {
            VM_ERRORB(_A("Can't deep clone array member."));
          }
        }
      }
      ret = true;
      break;
    }
    case OT_STRING: {
      target = self;
      ret = true;
      break;
    }
    case OT_IUNKNOWN: {
      Push(self);
      if (CallIUnknownMetaMethod(_iunknown(self), MT_IUNKNOWN_CLONE, 1, temp_reg)) {
        target = temp_reg;
        ret = true;
      }
      else {
        ret = false;
      }
      break;
    }
    default:
      ret = false;
      break;
  }

  if (apDeepClone && (_sqtype(self) == OT_TABLE || _sqtype(self) == OT_ARRAY)) {
    --apDeepClone->_depth;
  }
  return ret;
}

bool SQVM::NewSlot(const SQObjectPtr &self,const SQObjectPtr &key,const SQObjectPtr &val, int opExt)
{
  niLet noNewSlot = [&]() -> bool {
    if (opExt & _OPEXT_GET_SAFE) {
      return true;
    }
    VM_ERRORB(niFmt(_A("newslot indexing %s with %s"),_ss->GetTypeNameStr(self),_ss->GetTypeNameStr(key)));
  };

  switch(_sqtype(self)){
    case OT_TABLE: {
      if (_sqtype(key) == OT_NULL) {
        VM_ERRORB(_A("null cannot be used as index"));
      }
      _table(self)->NewSlot(key,val);
      break;
    }
    case OT_USERDATA: {
      if (_userdata(self)->GetDelegate()) {
        SQObjectPtr res;
        if (DoGet(self,key,res,NULL,0)) {
          return DoSet(self,key,val,0);
        }
        else {
          Push(self);
          Push(key);
          Push(val);
          return CallMetaMethod(_userdata(self)->GetDelegate(),MT_USERDATA_NEWSLOT,3,res);
        }
      }
      return noNewSlot();
    }
    case OT_IUNKNOWN: {
      // TODO: Review. What's going on here is unclear.
      SQObjectPtr t;
      Push(self); Push(key); Push(val); Push(opExt);
      if (!CallIUnknownMetaMethod(_iunknown(self),MT_USERDATA_NEWSLOT,4,t)) {
        return Set(self,key,val,opExt);
      }
      return noNewSlot();
    }
    default: {
      return noNewSlot();
    }
  }
  return true;
}

bool SQVM::DeleteSlot(const SQObjectPtr &self,const SQObjectPtr &key,SQObjectPtr &res, int opExt)
{
  switch(_sqtype(self)) {
    case OT_TABLE: {
      SQObjectPtr t;
      if (_table(self)->Get(key,t)) {
        _table(self)->Remove(key);
      }
      else {
        if (opExt & _OPEXT_GET_SAFE) {
          t = _null_;
        }
        else {
          Raise_IdxError((SQObject &)key);
          return false;
        }
      }
      res = t;
      break;
    }
    case OT_USERDATA:
      if (_userdata(self)->GetDelegate()) {
        Push(self);Push(key);
        SQObjectPtr t;
        if(!CallMetaMethod(_userdata(self)->GetDelegate(),MT_USERDATA_DELSLOT,2,t)) {
          VM_ERRORB(_A("cannot delete a slot from a userdata"));
        }
        res = t;
      }
      break;
    default:
      if (opExt & _OPEXT_GET_SAFE) {
        return true;
      }
      VM_ERRORB(niFmt(_A("attempt to delete a slot from a %s"),_ss->GetTypeNameStr(self)));
  }
  return true;
}

SQObjectPtr& SQVM::GetEmptyString() {
  return _emptystring;
}

void SQVM::CallErrorHandler(const SQObjectPtr& error)
{
  if (_sqtype(_errorhandler) != OT_NULL) {
    SQObjectPtr out;
    Push(_roottable); Push(error);
    Call(_errorhandler, 2, _top-2, out);
    Pop(2);
  }
}

void SQVM::CallDebugHook(int type,int forcedline)
{
  SQObjectPtr temp_reg;
  int nparams=5;
  SQFunctionProto *func = _funcproto(_closure(_ci->_closurePtr)->_function);
  Push(_roottable);
  Push(type);
  Push(func->_sourcename);
  Push(forcedline?forcedline:(int)func->GetLineCol(_ci->_ip).x);
  Push(func->_name);
  Call(_debughook,nparams,_top-nparams,temp_reg);
  Pop(nparams);
}

bool SQVM::CallNative(SQNativeClosure *nclosure, int nargs, int stackbase, SQObjectPtr &retval)
{
  if (_nnativecalls + 1 > MAX_NATIVE_CALLS) {
    VM_ERRORB(_A("Native stack overflow"));
  }

  int nparamscheck = nclosure->_nparamscheck;
  if (((nparamscheck > 0) && (nparamscheck != nargs))
      || ((nparamscheck < 0) && (nargs < (-nparamscheck))))
  {
    VM_ERRORB(niFmt("wrong number of parameters, native closure '%s' (0x%x), expected %d, passed %d",nclosure->_name,nclosure,nparamscheck,nargs));
  }

  int tcs = nclosure->_typecheck.size();
  if (tcs) {
    for(int i = 0; i < nargs && i < tcs; i++) {
      if((nclosure->_typecheck[i] != -1) &&
         !(_sqtype(_stack[stackbase+i]) & nclosure->_typecheck[i]))
      {
        Raise_ParamTypeError(i,nclosure->_typecheck[i],_sqtype(_stack[stackbase+i]));
        return false;
      }
    }
  }

  _nnativecalls++;
  if ((_top + MIN_STACK_OVERHEAD) > (int)_stack.size()) {
    _stack.resize(_stack.size() + (MIN_STACK_OVERHEAD<<1));
  }
  int oldtop = _top;
  int oldstackbase = _stackbase;
  _top = stackbase + nargs;
  push_callinfo(this, SQCallInfo());
  _ci->_etraps = 0;
  _ci->_closurePtr = nclosure;
  // _ci->_closure._var.mpIUnknown = nclosure;
  // _ci->_closure._var.mType = OT_NATIVECLOSURE;
  _ci->_prevstkbase = stackbase - _stackbase;
  _ci->_ncalls = 1;
  _stackbase = stackbase;
  _ci->_prevtop = (oldtop - oldstackbase);
  int ret = (nclosure->_function)(this);
  _nnativecalls--;

  if (ret < 0) {
    _stackbase = oldstackbase;
    _top = oldtop;
    pop_callinfo(this);
    return false;
  }

  if (ret != 0){ retval = VM_STACK_TOP(); }
  else { retval = _null_; }
  _stackbase = oldstackbase;
  _top = oldtop;
  pop_callinfo(this);
  return true;
}

bool SQVM::GetIUnknownMetaMethod(iUnknown* o, SQMetaMethod mm, SQObjectPtr &closure)
{
  if (((cScriptVM*)_foreignptr)->GetAutomation()->Get(
          this,o,_sq_metamethods[mm],closure,_OPEXT_GET_SAFE)) {
    return true;
  }
  return false;
}

bool SQVM::CallIUnknownMetaMethod(iUnknown* o, SQMetaMethod mm, int nparams, SQObjectPtr &outres)
{
  SQObjectPtr closure;
  if (((cScriptVM*)_foreignptr)->GetAutomation()->Get(
          this,o,_sq_metamethods[mm],closure,_OPEXT_GET_SAFE))
  {
#ifdef LAST_CALLSTACK
    _strLastCallStack.clear();
    _raiseErrorMode = eScriptRaiseErrorMode_Locals;
    sqFormatError(this, NULL, _strLastCallStack,0);
    niDebugFmt((_A("LAST CALLSTACK : \n%s"),_strLastCallStack));
#endif
    if (_sqtype(closure) == OT_USERDATA) {
      SQUserData* ud = _userdata(closure);
      if (ud->GetType() == eScriptType_MethodDef) {
        sScriptTypeMethodDef* pMethodDef = (sScriptTypeMethodDef*)ud;
        const int stackbase = _top-nparams;
        if (CallMethodDef(pMethodDef,nparams,stackbase,outres)) {
          Pop(nparams);
          return true;
        }
      }
    }
    else {
      if (Call(closure, nparams, _top-nparams, outres)) {
        Pop(nparams);
        return true;
      }
    }
  }
  Pop(nparams);
  return false;
}

bool SQVM::CallIUnknownMetaMethod(iUnknown* o, SQObjectPtr& closure, int nparams, SQObjectPtr &outres)
{
#ifdef LAST_CALLSTACK
  _strLastCallStack.clear();
  _raiseErrorMode = eScriptRaiseErrorMode_Locals;
  sqFormatError(this, NULL, _strLastCallStack,0);
  niDebugFmt((_A("LAST CALLSTACK : \n%s"),_strLastCallStack));
#endif
  if (_sqtype(closure) == OT_USERDATA) {
    SQUserData* ud = _userdata(closure);
    if (ud->GetType() == eScriptType_MethodDef) {
      sScriptTypeMethodDef* pMethodDef = (sScriptTypeMethodDef*)ud;
      const int stackbase = _top-nparams;
      if (CallMethodDef(pMethodDef,nparams,stackbase,outres)) {
        Pop(nparams);
        return true;
      }
    }
    else {
      if (Call(closure, nparams, _top-nparams, outres)) {
        Pop(nparams);
        return true;
      }
    }
  }

  Pop(nparams);
  return false;
}

bool SQVM::DoGet(const SQObjectPtr &self, const SQObjectPtr &key, SQObjectPtr &dest, const SQObjectPtr* root, int opExt)
{
  switch(_sqtype(self)){
    case OT_TABLE:
      {
        if (_table(self)->Get(key,dest))
          return true;
        // delegation
        if (_table(self)->GetDelegate()) {
          return DoGet(SQObjectPtr(_table(self)->GetDelegate()),key,dest,root?root:&self,opExt);
        }
        if (opExt & _OPEXT_GET_RAW) {
          return false;
        }
        return _ddel(*_ss,table)->Get(key,dest);
      }
    case OT_IUNKNOWN:
      return ((cScriptVM*)_foreignptr)->GetAutomation()->Get(this,_iunknown(self),key,dest,opExt);
    case OT_ARRAY:
      if (sq_isnumeric(key)) {
        return _array(self)->Get(toint(key),dest);
      }
      else {
        return _ddel(*_ss,array)->Get(key,dest);
      }
    case OT_STRING:
      if(sq_isnumeric(key)){
        SQInt n=toint(key);
        if (abs(n) < (int)_stringlen(self)) {
          if(n<0)n=_stringlen(self)-n;
          dest=SQInt(_stringval(self)[n]);
          return true;
        }
        return false;
      }
      else return _ddel(*_ss,string)->Get(key,dest);
      break;
    case OT_USERDATA:
      {
        bool getRetVal = false;
        if (_userdata(self)->GetDelegate()) {
          getRetVal = DoGet(SQObjectPtr(_userdata(self)->GetDelegate()),
                            key,dest,root,opExt|_OPEXT_GET_RAW);
          if (!getRetVal) {
            if (opExt & _OPEXT_GET_RAW)
              return false;
            Push(root?*root:self);
            Push(key);
            if (!CallMetaMethod(_userdata(self)->GetDelegate(),MT_USERDATA_GET,2,dest)) {
              return false;
            }
            return true;
          }
        }
        return getRetVal;
      }
      break;
    case OT_INTEGER:case OT_FLOAT:
      return _ddel(*_ss,number)->Get(key,dest);
    case OT_CLOSURE: case OT_NATIVECLOSURE:
      return _ddel(*_ss,closure)->Get(key,dest);
    default:
      if (!(opExt & _OPEXT_GET_SAFE)) {
        VM_ERRORB(niFmt(_A("indexing a %s, with key '%s' (%s)"),
                        _ss->GetTypeNameStr(self),
                        (_sqtype(key)==OT_STRING)?_stringval(key):"",
                        _ss->GetTypeNameStr(key)));
      }
      return false;
  }
}

bool SQVM::DoSet(const SQObjectPtr &self,const SQObjectPtr &key,const SQObjectPtr &val, int opExt)
{
  switch(_sqtype(self)) {
    case OT_TABLE: {
      if (!_table(self)->Set(key,val)) {
        if (_table(self)->GetDelegate()) {
          SQObjectPtr v;
          if (!DoGet(self,key,v,NULL,0)) {
            return false;
          }
          return _table(self)->NewSlot(key,val);
        }
        return false;
      }
      return true;
    }
    case OT_IUNKNOWN:
      return ((cScriptVM*)_foreignptr)->GetAutomation()->Set(this,_iunknown(self),key,val,opExt);
    case OT_ARRAY:
      if (!sq_isnumeric(key)) {
        VM_ERRORB(niFmt(_A("indexing '%s' with '%s'"),
                        _ss->GetTypeNameStr(self),_ss->GetTypeNameStr(key)));
      }
      return _array(self)->Set(toint(key),val);
    case OT_USERDATA:
      if(_userdata(self)->GetDelegate()){
        SQObjectPtr t;
        Push(self);Push(key);Push(val);
        return CallMetaMethod(_userdata(self)->GetDelegate(),MT_USERDATA_SET,3,t);
      }
      break;
  }
  return false;

}

static _HDecl(_args_);
bool SQVM::StartCall(SQClosure* closure, int target, int nargs, int stackbase, bool tailcall)
{
  SQFunctionProto* func = _funcproto(closure->_function);
  niAssert(func);
  if (!func) {
    VM_ERRORB("Calling an invalid closure object, no function prototype.");
  }

  const int outerssize = (int)func->_outervalues.size();
  const int paramssize = (int)(func->_parameters.size() - outerssize);
  const int newtop = stackbase + func->_stacksize;
  //     niDebugFmt((_A("CALL: [%d] %d/%d"),stackbase,nargs,paramssize));

  if ((paramssize-1) == 1 && _stringobj(func->_parameters[1]._name) == _HC(_args_)) {
    // niDebugFmt(("...VARARGS..."));
    const tU32 pbase = stackbase+paramssize-1;
    const tU32 numVarArgs = nargs-1;
    Ptr<SQArray> args = SQArray::Create(numVarArgs);
    niLoop(i,numVarArgs) {
      args->_values[i] = _stack[pbase+i];
      _stack[pbase+i] = _null_;
    }
    _stack[pbase] = args.ptr();
  }
  /*
    else if (nargs > paramssize) {
    VM_ERRORB(niFmt(_A("too many parameters (%d) for closure '%s/%d' (0x%x)"),
    nargs,
    _stringobj(func->_name),
    paramssize,
    closure));
    }
  */
  /*
    if (paramssize != nargs) {
    VM_ERRORB(niFmt(_A("invalid number of parameters (%d) for closure '%s/%d' (0x%x)"),
    nargs,
    _stringobj(func->_name),
    paramssize,
    closure));
    }
  */

  if (!tailcall) {
    push_callinfo(this, SQCallInfo());
    _ci->_etraps = 0;
    _ci->_prevstkbase = stackbase - _stackbase;
    _ci->_target = target;
    _ci->_prevtop = _top - _stackbase;
    _ci->_ncalls = 1;
    _ci->_root = false;
  }
  else {
    _ci->_ncalls++;
  }
  _ci->_closurePtr = closure;
  // _ci->_closure._var.mpIUnknown = closure;
  // _ci->_closure._var.mType = OT_CLOSURE;
  _ci->_iv = &func->_instructions;
  _ci->_literals = &func->_literals;
  //grows the stack if needed
  if (((unsigned int)newtop + (func->_stacksize << 1) + outerssize) > _stack.size()) {
    _stack.resize(_stack.size() + (func->_stacksize << 1) + outerssize);
  }
  if (nargs < paramssize) {
    // set unspecified parameters to null...
    int base = stackbase + nargs;
    for (int i = 0; i < (paramssize-nargs); ++i)
      _stack[base + i] = _null_;
  }
  if ((outerssize)>0) {
    // set outer variables, they'll override extra parameters...
    int base = stackbase + paramssize;
    for (int i = 0; i < outerssize; ++i)
      _stack[base + i] = closure->_outervalues[i];
  }
  _top = newtop;
  _stackbase = stackbase;
  _ci->_ip = &(*_ci->_iv)[0];
  return true;
}

void SQVM::dumpstack(int stackbase,bool dumpall)
{
  int size=dumpall?_stack.size():_top;
  int n=0;
  cString o;
  o << niFmt(_A("\n>>>>stack dump<<<<\n"));
  SQCallInfo &ci=_callsstack.back();
  o << niFmt(_A("IP: %d\n"),ci._ip);
  o << niFmt(_A("prev stack base: %d\n"),ci._prevstkbase);
  o << niFmt(_A("prev top: %d\n"),ci._prevtop);
  for(int i=0;i<size;i++){
    SQObjectPtr &obj=_stack[i];
    if (stackbase==i) {
      o << niFmt(_A(">"));
    }
    else {
      o << _A(" ");
    }
    o << niFmt(_A("[%d]:"),n);
    switch(_sqtype(obj)){
      case OT_FLOAT:      o << niFmt(_A("FLOAT %.3f"),_float(obj));break;
      case OT_INTEGER:    o << niFmt(_A("INTEGER %d"),_int(obj));break;
      case OT_STRING:     o << niFmt(_A("STRING %s"),_stringval(obj));break;
      case OT_NULL:     o << niFmt(_A("NULL")); break;
      case OT_TABLE:      o << niFmt(_A("TABLE %p[%p]"),_table(obj),_table(obj)->GetDelegate());break;
      case OT_ARRAY:      o << niFmt(_A("ARRAY %p"),_array(obj));break;
      case OT_CLOSURE:    o << niFmt(_A("CLOSURE [%p]"),_closure(obj));break;
      case OT_NATIVECLOSURE:  o << niFmt(_A("NATIVECLOSURE"));break;
      case OT_USERDATA:   o << niFmt(_A("USERDATA %p[%p]"),_userdata(obj),_userdata(obj)->GetDelegate());break;
      case OT_IUNKNOWN:   o << niFmt(_A("IUNKNOWN %p [%d refs]"),
                                     (tIntPtr)_iunknown(obj),
                                     _iunknown(obj)?_iunknown(obj)->GetNumRefs():0);
        break;
      default:
        niAssertUnreachable("Unknown object type.");
        break;
    };
    o << _A("\n");
    ++n;
  }
  niDebugFmt((o.Chars()));
}
