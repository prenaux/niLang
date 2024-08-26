#include "stdafx.h"

#include <niLang/Utils/Trace.h>
#include <niLang/STL/hash_set.h>
#include "sqconfig.h"
#include "sqobject.h"
#include "sqclosure.h"
#include "sqfuncproto.h"
#include "sqarray.h"
#include "sqtable.h"
#include "sqopcodes.h"
#include "sqfuncstate.h"
#include "sq_hstring.h"
#include "sqstate.h"
#include "ScriptTypes.h"
#include "ScriptVM.h"

#define SQLINTER_LOG_INLINE

static bool _ShouldKeepName(ain<tChars> aFilter, ain<tChars> aName, ain<bool> aDefault) {
  if (aFilter && *aFilter) {
    if (ni::StrCmp(aFilter,"*") == 0 ||
        ni::StrCmp(aFilter,"1") == 0 ||
        ni::StrICmp(aFilter,"all") == 0 ||
        ni::StrICmp(aName,aFilter) == 0) {
      return true;
    }

    if (ni::StrCmp(aFilter,"0") == 0 ||
        ni::StrICmp(aFilter,"none") == 0)
    {
      return false;
    }

    return ni::afilepattern_match(aFilter, aName) > 0;
  }

  return aDefault;
}

static const char* const _InstrDesc[]={
  "_OP_LINE",
  "_OP_LOAD",
  "_OP_LOADNULL",
  "_OP_NEWSLOT",
  "_OP_SET",
  "_OP_GET",
  "_OP_LOADROOTTABLE",
  "_OP_PREPCALL",
  "_OP_CALL",
  "_OP_MOVE",
  "_OP_ADD",
  "_OP_SUB",
  "_OP_MUL",
  "_OP_DIV",
  "_OP_MODULO",
  "_OP_EQ",
  "_OP_NE",
  "_OP_G",
  "_OP_GE",
  "_OP_L",
  "_OP_LE",
  "_OP_EXISTS",
  "_OP_NEWTABLE",
  "_OP_JMP",
  "_OP_JNZ",
  "_OP_JZ",
  "_OP_RETURN",
  "_OP_CLOSURE",
  "_OP_FOREACH",
  "_OP_TYPEOF",
  "_OP_PUSHTRAP",
  "_OP_POPTRAP",
  "_OP_THROW",
  "_OP_NEWARRAY",
  "_OP_APPENDARRAY",
  "_OP_AND",
  "_OP_OR",
  "_OP_NEG",
  "_OP_NOT",
  "_OP_DELETE",
  "_OP_BWNOT",
  "_OP_BWAND",
  "_OP_BWOR",
  "_OP_BWXOR",
  "_OP_MINUSEQ",
  "_OP_PLUSEQ",
  "_OP_MULEQ",
  "_OP_DIVEQ",
  "_OP_TAILCALL",
  "_OP_SHIFTL",
  "_OP_SHIFTR",
  "_OP_INC",
  "_OP_DEC",
  "_OP_PINC",
  "_OP_PDEC",
  "_OP_GETK",
  "_OP_PREPCALLK",
  "_OP_DMOVE",
  "_OP_LOADNULLS",
  "_OP_USHIFTR",
  "_OP_SHIFTLEQ",
  "_OP_SHIFTREQ",
  "_OP_USHIFTREQ",
  "_OP_BWANDEQ",
  "_OP_BWOREQ",
  "_OP_BWXOREQ",
  "_OP_MODULOEQ",
  "_OP_SPACESHIP",
  "_OP_THROW_SILENT",
};
niCAssert(niCountOf(_InstrDesc) == __OP_LAST);
extern "C" const achar* _GetOpDesc(int op) {
  return (op >= 0 && op < __OP_LAST) ? _InstrDesc[op] : "_OP_???";
}

static cString& _AddOpExt(cString& o, int opExt) {
  int added = 0;
  o << "{";
  if (opExt) {
    if (opExt & _OPEXT_GET_RAW) {
      if (added++ > 0) { o << "|"; }
      o << "GET_RAW";
    }
    if (opExt & _OPEXT_GET_SAFE) {
      if (added++ > 0) { o << "|"; }
      o << "GET_SAFE";
    }
    if (opExt & _OPEXT_EXPLICIT_THIS) {
      if (added++ > 0) { o << "|"; }
      o << "EXPLICIT_THIS";
    }
  }
  o << "}";
  return o;
}

namespace ni {
niExportFuncCPP(cString&) Base32AppendToString(cString& r, const tPtr src, tInt slen, const tBool abUpperCase) {
  niAssert(src != NULL);
  const tInt sz = Base32EncodeOutputSize(slen);
  if (sz <= 0) {
    return r;
  }
  const tOffset startSize = r.size();
  r.resize(startSize + sz);
  tInt effectiveLen = Base32Encode((tPtr)r.data()+startSize, sz, src, slen, abUpperCase);
  // the string's size need to be set to the effective string len, unused chars are set to 0
  r.resize(startSize + effectiveLen);
  return r;
}
}

static cString& _PtrToString(cString& r, const tIntPtr aPtr) {
  ni::Base32AppendToString(r, (tPtr)&aPtr, sizeof(aPtr), eTrue);
  const int rsize = r.size()-1;
  const char* rp = r.data() + (rsize - 1);
  int trailingZeros = 0;
  // niDebugFmt(("... _PtrToString: Before: %s", r));
  niLoop(i,ni::Max(8,rsize)) {
    if (*rp-- == '0') {
      ++trailingZeros;
    }
    else {
      break;
    }
  }
  if (trailingZeros > 0) {
    r.resize(rsize - trailingZeros);
  }
  // niDebugFmt(("... _PtrToString: After[%d]: %s (%d)", trailingZeros, r));
  return r;
}
static cString _PtrToString(const tIntPtr aPtr) {
  cString r;
  return _PtrToString(r,aPtr);
}

static const achar* _ObjCharsOrZeroStr(const SQObjectPtr& obj) {
  switch (_sqtype(obj)) {
    case OT_STRING: {
      return _stringval(obj);
    }
    default: {
      return AZEROSTR;
    }
  }
}

static cString _ObjTypeString(const SQObjectPtr& obj)
{
  niLet type = _RAW_TYPE(_sqtype(obj));
  switch (type) {
    case _RT_NULL: return "Null";
    case _RT_INTEGER: return "I32";
    case _RT_FLOAT: return "F64";
    case _RT_STRING: return "STRING";
    case _RT_TABLE: return "TABLE";
    case _RT_ARRAY: return "ARRAY";
    case _RT_USERDATA: {
      SQUserData* ud = _userdata(obj);
      return ud->GetTypeString();
    }
    case _RT_CLOSURE: return "CLOSURE";
    case _RT_NATIVECLOSURE: return "NATIVECLOSURE";
    case _RT_FUNCPROTO: return "FUNCPROTO";
    case _RT_IUNKNOWN: return "IUnknownPtr";
    default: return ni::GetTypeString(type);
  }
}

static cString _ObjToString(const SQObjectPtr& obj) {
  switch (_sqtype(obj)) {
    case OT_STRING: {
      return niFmt("\"%s\"",_stringval(obj));
    }
    case OT_IUNKNOWN: {
      return niFmt("{%s}::IUnknown",_PtrToString((tIntPtr)_iunknown(obj)));
    }
    case OT_FUNCPROTO: {
      return niFmt("function %s",_ObjToString(_funcproto(obj)->_name));
    }
    case OT_TABLE: {
      return niFmt("%s{%s}::TABLE",
                   _table(obj)->GetDebugName(),
                   _PtrToString((tIntPtr)_table(obj)));
    }
    case OT_USERDATA: {
      return niFmt("%s", _userdata(obj)->GetTypeString());
    }
    default:
      return niFmt("%s::%s",(Var&)obj._var,_ObjTypeString(obj));
  }
}

typedef astl::hash_set<tIntPtr> tExpandedObjectsSet;

static cString _ObjToRecString(const SQObjectPtr& obj, const cString& ind, tExpandedObjectsSet& expandedObjects);

static cString _TableToRecString(const SQObjectPtr& obj, const cString& ind, tExpandedObjectsSet& expandedObjects)
{
  if (_sqtype(obj) != OT_TABLE) {
    return _ASTR("{NOT A TABLE}");
  }

  const cString nextInd = ind + "  ";

  cString o;
  o << _ObjToString(obj) << " {\n";
  SQObjectPtr val,key,refo,ito;
  while (_table(obj)->Next(refo,key,val,ito))
  {
    o << nextInd
      << _ObjToString(key)
      << " = "
      << _ObjToRecString(val, nextInd, expandedObjects)
      << "\n";
    refo = ito;
  }
  o << ind << "}\n";
  return o;
}

static cString _ObjToRecString(const SQObjectPtr& obj, const cString& ind, tExpandedObjectsSet& expandedObjects) {
  if (_sqtype(obj) == OT_TABLE) {
    if (astl::contains(expandedObjects,_intptr(obj))) {
      return _ObjToString(obj) + _ASTR(" <CYCLE>");
    }
    else {
      expandedObjects.insert(_intptr(obj));
      return _TableToRecString(obj, ind, expandedObjects);
    }
  }
  else {
    return _ObjToString(obj);
  }
}

static cString _RootObjToRecString(const SQObjectPtr& obj, const cString& ind) {
  tExpandedObjectsSet expandedObjects;
  return _ObjToRecString(obj,ind,expandedObjects);
}

static cString _RootObjToRecString(const SQObjectPtr& obj) {
  return _RootObjToRecString(obj,"");
}

static SQObjectPtr _GetKeyFromArg(const SQObjectPtr& _literals, int arg) {
  SQObjectPtr val,key,refo,ito;
  while (_table(_literals)->Next(refo,key,val,ito)
         && (_int(val) != arg))
  {
    refo = ito;
  }
  return key;
}

static cString _FuncProtoToString(const SQFunctionProto& func) {
  const int outerssize = (int)func._outervalues.size();
  const int paramssize = (int)(func._parameters.size() - outerssize);
  const int arity = paramssize-1; // number of parameter - 1 for the implicit 'this' argument
  return niFmt("%s/%d(%d)[%s:%d]",
               func.GetName(),
               arity,
               outerssize,
               func.GetSourceName(),
               func._sourceline);
}

static cString _NativeClosureToString(const SQNativeClosure& func) {
  const int paramssize = func._nparamscheck;
  const int arity = paramssize-1; // number of parameter - 1 for the implicit 'this' argument
  return niFmt("%s/%d", func._name, arity);
}

enum eLintFlags {
  eLintFlags_None = 0,
  eLintFlags_IsError = niBit(31),
  eLintFlags_IsWarning = niBit(30),
  eLintFlags_IsInfo = niBit(29),
  eLintFlags_IsInternal = niBit(28),
  eLintFlags_IsPedantic = niBit(27),
  eLintFlags_IsExperimental = niBit(26),
  // The lint is disabled unless explicitly enabled, even 'all' won't implicitly enable it
  eLintFlags_IsExplicit = niBit(25),
};

struct sLint {
  const tU32 key;
  const tHStringPtr name;
};

struct LintClosure : public ImplRC<iUnknown>
{
  LintClosure(SQFunctionProto *func, SQTable* rootTable, SQTable* thisTable) {
    _func = func;
    _root = rootTable;
    _this = thisTable;
  }

  Ptr<SQFunctionProto> _func;
  SQObjectPtrVec _outervalues;
  SQObjectPtr _root;
  SQObjectPtr _this;
};

typedef astl::hash_map<const SQFunctionProto*,Ptr<LintClosure> > tFuncClosureMap;

#define _LOBJ(KIND) (kLint_##KIND)
#define _LKEY(KIND) (kLint_##KIND.key)
#define _LNAME(KIND) (kLint_##KIND.name.ptr())
#define _LTRACE(MSG) if (shouldLintTrace) { niDebugFmt(MSG); }
#define _LINTERNAL_ERROR(MSG) aLinter.Log(_LKEY(internal_error), _LNAME(internal_error), *thisfunc, thisfunc->GetSourceLineCol(), MSG)
#define _LINTERNAL_WARNING(MSG) aLinter.Log(_LKEY(internal_warning), _LNAME(internal_warning), *thisfunc, thisfunc->GetSourceLineCol(), MSG)
#define _LENABLED(KIND) aLinter.IsEnabled(_LKEY(KIND))
#define _LINT(KIND,MSG) aLinter.Log(_LKEY(KIND), _LNAME(KIND), *thisfunc, getlinecol(inst), MSG)

static tU32 _lintKeyGen = 0;
#define _DEF_LINT(NAME,CAT1,CAT2)                                       \
  static const sLint kLint_##NAME = { ++_lintKeyGen | eLintFlags_##CAT1 | eLintFlags_##CAT2, _H(#NAME) };

_DEF_LINT(internal_error,IsError,IsInternal);
_DEF_LINT(internal_warning,IsWarning,IsInternal);
_DEF_LINT(implicit_this_getk,IsWarning,IsPedantic);
_DEF_LINT(implicit_this_callk,IsWarning,IsPedantic);
_DEF_LINT(this_key_notfound_getk,IsError,None);
_DEF_LINT(this_key_notfound_callk,IsError,None);
_DEF_LINT(this_key_notfound_outer,IsError,None);
_DEF_LINT(key_notfound_getk,IsError,IsExperimental);
_DEF_LINT(key_notfound_callk,IsError,IsExperimental);
_DEF_LINT(this_set_key_notfound,IsError,None);
_DEF_LINT(set_key_notfound,IsError,IsExperimental);
_DEF_LINT(call_null,IsWarning,IsExplicit);
_DEF_LINT(getk_null,IsWarning,IsExplicit);
_DEF_LINT(call_error,IsError,None);
_DEF_LINT(call_num_args,IsError,None);
_DEF_LINT(ret_type_is_null,IsWarning,IsExplicit);
_DEF_LINT(ret_type_cant_assign,IsError,None);

#undef _DEF_LINT

struct sLinter;

struct iLintFuncCall : public iUnknown {
  niDeclareInterfaceUUID(iLintFuncCall,0xfeee9127,0x5c61,0xef11,0x8a,0x5c,0x97,0x69,0xc8,0x3a,0xa9,0xff);

  virtual nn<iHString> __stdcall GetName() const = 0;
  // return -1 for varargs
  virtual tI32 __stdcall GetArity() const = 0;
  virtual SQObjectPtr __stdcall LintCall(const sLinter& aLinter, ain<astl::vector<SQObjectPtr>> aCallArgs) = 0;
};

struct sLinter {
 private:
  sLinter(const sLinter& aLinter);

 public:
  typedef astl::hash_map<tU32,tI32> tLintKindMap;
  tLintKindMap _lintEnabled;
  SQSharedState _ss;
  SQObjectPtr _vmroot;
  SQObjectPtr _typedefs;
  SQObjectPtr _lintFuncCallQueryInterface;

#if !defined SQLINTER_LOG_INLINE
  astl::vector<cString> _logs;
#endif

#define _REG_LINT(KIND) astl::upsert(_lintEnabled,_LKEY(KIND),          \
                                     niFlagIsNot(_LKEY(KIND),eLintFlags_IsPedantic) && \
                                     niFlagIsNot(_LKEY(KIND),eLintFlags_IsExperimental) && \
                                     niFlagIsNot(_LKEY(KIND),eLintFlags_IsExplicit))

  sLinter() {
    _vmroot = SQTable::Create();
    _table(_vmroot)->SetDebugName("__vmroot__");
    this->RegisterBuiltinFuncs(_table(_vmroot));

    _typedefs = SQTable::Create();
    _table(_typedefs)->SetDebugName("__typedefs__");

    _REG_LINT(internal_error);
    _REG_LINT(internal_warning);
    _REG_LINT(implicit_this_getk);
    _REG_LINT(implicit_this_callk);
    _REG_LINT(this_key_notfound_getk);
    _REG_LINT(this_key_notfound_callk);
    _REG_LINT(this_key_notfound_outer);
    _REG_LINT(key_notfound_getk);
    _REG_LINT(key_notfound_callk);
    _REG_LINT(this_set_key_notfound);
    _REG_LINT(set_key_notfound);
    _REG_LINT(call_null);
    _REG_LINT(getk_null);
    _REG_LINT(call_error);
    _REG_LINT(call_num_args);
    _REG_LINT(ret_type_is_null);
    _REG_LINT(ret_type_cant_assign);
  }
#undef _REG_LINT

  tBool EnableFromSlot(const SQFunctionProto* thisfunc, ain<SQObjectPtr> table, ain<SQObjectPtr> k, ain<SQObjectPtr> v) {
    sLinter& aLinter = *this;
    if (sq_type(table) != OT_TABLE) {
      _LINTERNAL_WARNING("__lint table is not a table.");
      return eFalse;
    }
    tHStringPtr tableName = _table(table)->GetDebugHName();
    if (tableName == _HC(__lint)) {
      if (sq_type(k) != OT_STRING) {
        _LINTERNAL_WARNING("__lint key is not a string.");
        return eFalse;
      }
      if (sq_type(v) != OT_INTEGER) {
        _LINTERNAL_WARNING("__lint value is not an integer or boolean.");
        return eFalse;
      }
      return EnableFromName(
        thisfunc, _stringhval(k), _int(v));
    }
    else {
      return eFalse;
    }
  }

  tBool EnableFromName(const SQFunctionProto* thisfunc, const iHString* aName, tI32 aValue) {
    sLinter& aLinter = *this;

#define _E(NAME)                                        \
    else if (aName == _LNAME(NAME)) {                   \
      astl::upsert(_lintEnabled, _LKEY(NAME), aValue);  \
    }

    // applies to all the non explicit lints
    if (aName == _HC(_all)) {
      niLoopit(tLintKindMap::iterator, it, _lintEnabled) {
        const tU32 lint = it->first;
        if (!niFlagIs(lint,eLintFlags_IsExplicit)) {
          it->second = aValue;
        }
      }
    }
    // applies to all the pedantic lints
    else if (aName == _HC(_pedantic)) {
      niLoopit(tLintKindMap::iterator, it, _lintEnabled) {
        const tU32 lint = it->first;
        if (niFlagIs(lint,eLintFlags_IsPedantic)) {
          it->second = aValue;
        }
      }
    }
    // applies to all the experimental lints
    else if (aName == _HC(_experimental)) {
      niLoopit(tLintKindMap::iterator, it, _lintEnabled) {
        const tU32 lint = it->first;
        if (niFlagIs(lint,eLintFlags_IsExperimental)) {
          it->second = aValue;
        }
      }
    }
    // applies to all the explicit lints
    else if (aName == _HC(_explicit)) {
      niLoopit(tLintKindMap::iterator, it, _lintEnabled) {
        const tU32 lint = it->first;
        if (niFlagIs(lint,eLintFlags_IsExplicit)) {
          it->second = aValue;
        }
      }
    }

    _E(implicit_this_getk)
    _E(implicit_this_callk)
    _E(this_key_notfound_getk)
    _E(this_key_notfound_callk)
    _E(this_key_notfound_outer)
    _E(key_notfound_callk)
    _E(key_notfound_getk)
    _E(this_set_key_notfound)
    _E(set_key_notfound)
    _E(call_null)
    _E(getk_null)
    _E(call_error)
    _E(call_num_args)
    _E(ret_type_is_null)
    _E(ret_type_cant_assign)
    else {
      _LINTERNAL_WARNING(niFmt("__lint unknown lint kind '%s'.", aName));
      return eFalse;
    }

#undef _E
    return eTrue;
  }

  tBool IsEnabled(tU32 aLint) const {
    astl::hash_map<tU32,tI32>::const_iterator it = _lintEnabled.find((tU32)aLint);
    if (it == _lintEnabled.end())
      return eTrue; // all lint are enabled by default;
    return it->second;
  }

  void Log(
    const tU32 aLint,
    const iHString* aLintName,
    const SQFunctionProto& aProto,
    const sVec2i aLineCol,
    const cString& aMsg)
  {
#if defined SQLINTER_LOG_INLINE
    cString o;
#else
    cString& o = _logs.push_back();
#endif
    o << "Lint: ";
    if (niFlagIs(aLint,eLintFlags_IsError)) {
      o << "Error: ";
    }
    else if (niFlagIs(aLint,eLintFlags_IsWarning)) {
      o << "Warning: ";
    }
    else if (niFlagIs(aLint,eLintFlags_IsInfo)) {
      o << "Info: ";
    }

    if (niFlagIs(aLint,eLintFlags_IsExperimental)) {
      o << "Experimental: ";
    }
    if (niFlagIs(aLint,eLintFlags_IsExplicit)) {
      o << "Explicit: ";
    }

    // if (aLintName != NULL) {
    o << niHStr(aLintName) << ": ";
    // }
    o << aMsg;

    if (aLineCol.x == aProto._sourceline) {
      o << niFmt(" (in %s)", aProto.GetName());
    }
    else {
      o << niFmt(" (in %s:%s)", aProto.GetName(), aProto._sourceline);
    }
    o << niFmt(" [%s:%s:%s]\n", aProto.GetSourceName(), aLineCol.x, aLineCol.y);
#ifdef SQLINTER_LOG_INLINE
    niPrintln(o);
#endif
  }

  tFuncClosureMap _funcClosureMap;
  tBool AddClosure(LintClosure* apClosure) {
    if (GetClosure(apClosure->_func) != NULL)
      return eFalse;
    astl::upsert(_funcClosureMap,apClosure->_func.ptr(),apClosure);
    return eTrue;
  }
  LintClosure* GetClosure(const SQFunctionProto* apProto) {
    tFuncClosureMap::iterator it = _funcClosureMap.find(apProto);
    if (it == _funcClosureMap.end())
      return NULL;
    return it->second;
  }

  void RegisterFuncNull(SQTable* table, const achar* name) {
    table->NewSlot(_H(name), _null_);
  }

  void RegisterFunc(SQTable* table, const achar* name, ain<SQObjectPtr> aLintFunc) {
    table->NewSlot(_H(name), aLintFunc);
  }

  void RegisterFunc(SQTable* table, const iHString* ahspName, ain<SQObjectPtr> aLintFunc) {
    table->NewSlot(ahspName, aLintFunc);
  }

  void RegisterLintFunc(SQTable* table, ain_nn_mut<iLintFuncCall> aLintFunc) {
    table->NewSlot(aLintFunc->GetName().raw_ptr(), aLintFunc.raw_ptr());
  }

  int RegisterSQRegFunctions(SQTable* table, const SQRegFunction* regs) {
    int i = 0;
    while (regs[i].name != NULL) {
      RegisterFuncNull(table, regs[i].name);
      ++i;
    }
    return i;
  }

  void RegisterBuiltinFuncs(SQTable* table);

  SQObjectPtr ResolveTypeUUID(const achar* aTypeName, const tUUID& aTypeUUID) const {
    // TODO: Cache returned values
    niLoop(mi, ni::GetLang()->GetNumModuleDefs()) {
      niLet mdef = ni::GetLang()->GetModuleDef(mi);
      niLoop(ii, mdef->GetNumInterfaces()) {
        niLet idef = mdef->GetInterface(ii);
        if (*idef->mUUID == aTypeUUID) {
          return niNew sScriptTypeInterfaceDef(
            this->_ss, idef);
        }
      }
    }
    return niNew sScriptTypeErrorCode(
      this->_ss,
      _HC(error_code_cant_find_type_uuid),
      niFmt("type_uuid<%s,%s>",aTypeName,aTypeUUID));
  }

  astl::optional<const sInterfaceDef*> FindInterfaceDef(ain_nn_mut<iHString> aInterfaceName) const
  {
    niLet interfaceUUID = ni::GetLang()->GetInterfaceUUID(aInterfaceName);
    if (interfaceUUID == kuuidZero)
      return astl::nullopt;

    niLet interfaceDef = ni::GetLang()->GetInterfaceDefFromUUID(interfaceUUID);
    if (interfaceDef == nullptr)
      return astl::nullopt;

    return interfaceDef;
  }

  astl::optional<const iObjectTypeDef*> FindObjectTypeDef(const achar* aObjectTypeName) const {
    // TODO: Cache returned values
    niLoop(mi, ni::GetLang()->GetNumModuleDefs()) {
      niLet mdef = ni::GetLang()->GetModuleDef(mi);
      niLoop(oi,mdef->GetNumObjectTypes()) {
        const iObjectTypeDef* odef = mdef->GetObjectType(oi);
        if (StrEq(odef->GetName(), aObjectTypeName)) {
          return odef;
        }
      }
    }
    return astl::nullopt;
  }


  SQObjectPtr ResolveType(const SQObjectPtr& aType) {
    if (sq_isnull(aType))
      return _null_;

    SQObjectPtr typeDef;
    if (LintGet(_typedefs,aType,typeDef,0)) {
      return typeDef;
    }

    if (!sq_isstring(aType)) {
      return _null_;
    }

    SQObjectPtr resolvedType = _null_;

    niLet hspType = as_NN(_stringhval(aType));
    niLet typeStrChars = hspType->GetChars();
    niLet typeStrLen = hspType->GetLength();
    // niDebugFmt(("... ResolveType: %s", hspType));

    // Interface types
    if (typeStrLen >= 2 && typeStrChars[0] == 'i' && StrIsUpper(typeStrChars[1])) {
      niLetMut foundInterfaceDef = this->FindInterfaceDef(hspType);
      if (!foundInterfaceDef.has_value()) {
        resolvedType = niNew sScriptTypeErrorCode(
          this->_ss, _HC(error_code_cant_find_interface_def), niHStr(hspType));
      }
      else {
        resolvedType = niNew sScriptTypeInterfaceDef(
          this->_ss, foundInterfaceDef.value());
      }
    }

    // Base types
    else if (hspType == _ss._typeStr_string) {
      resolvedType = niNew sScriptTypeResolvedType(
        this->_ss, eScriptType_String);
      _userdata(resolvedType)->SetDelegate(_ddel(_ss,string));
    }
    else if (hspType == _ss._typeStr_int ||
             hspType == _HC(typestr_bool))
    {
      resolvedType = niNew sScriptTypeResolvedType(
        this->_ss, eScriptType_Int);
      _userdata(resolvedType)->SetDelegate(_ddel(_ss,number));
    }
    else if (hspType == _ss._typeStr_float) {
      resolvedType = niNew sScriptTypeResolvedType(
        this->_ss, eScriptType_Float);
      _userdata(resolvedType)->SetDelegate(_ddel(_ss,number));
    }
    else if (hspType == _ss._typeStr_array) {
      resolvedType = niNew sScriptTypeResolvedType(
        this->_ss, eScriptType_Array);
      _userdata(resolvedType)->SetDelegate(_ddel(_ss,array));
    }
    else if (hspType == _ss._typeStr_table) {
      resolvedType = niNew sScriptTypeResolvedType(
        this->_ss, eScriptType_Table);
      _userdata(resolvedType)->SetDelegate(_ddel(_ss,table));
    }
    else if (hspType == _ss._typeStr_closure) {
      resolvedType = niNew sScriptTypeResolvedType(
        this->_ss, eScriptType_Closure);
      _userdata(resolvedType)->SetDelegate(_ddel(_ss,closure));
    }
    else if (hspType == _HC(typestr_void)) {
      resolvedType = niNew sScriptTypeResolvedType(
        this->_ss, eScriptType_Null);
    }

    // Userdata based types
    else if (hspType == _HC(Vec2)) {
      resolvedType = niNew sScriptTypeResolvedType(
        this->_ss, eScriptType_Vec2);
      _userdata(resolvedType)->SetDelegate(_ddel(_ss,vec2f));
    }
    else if (hspType == _HC(Vec3)) {
      resolvedType = niNew sScriptTypeResolvedType(
        this->_ss, eScriptType_Vec3);
      _userdata(resolvedType)->SetDelegate(_ddel(_ss,vec3f));
    }
    else if (hspType == _HC(Vec4)) {
      resolvedType = niNew sScriptTypeResolvedType(
        this->_ss, eScriptType_Vec4);
      _userdata(resolvedType)->SetDelegate(_ddel(_ss,vec4f));
    }
    else if (hspType == _HC(Matrix)) {
      resolvedType = niNew sScriptTypeResolvedType(
        this->_ss, eScriptType_Matrix);
      _userdata(resolvedType)->SetDelegate(_ddel(_ss,matrixf));
    }
    else if (hspType == _HC(UUID)) {
      resolvedType = niNew sScriptTypeResolvedType(
        this->_ss, eScriptType_UUID);
      _userdata(resolvedType)->SetDelegate(_ddel(_ss,uuid));
    }
    else {
      resolvedType = niNew sScriptTypeErrorCode(
        this->_ss, _HC(error_code_unknown_typedef), niHStr(hspType));
    }

    niPanicAssert(resolvedType != _null_);
    niPanicAssert(this->LintNewSlot(_typedefs,aType,resolvedType));
    return resolvedType;
  }

  bool LintGet(const SQObjectPtr &self, const SQObjectPtr &key, SQObjectPtr &dest, int opExt)
  {
    const SQSharedState& ss = this->_ss;
    switch(_sqtype(self)){
      case OT_TABLE:
        {
          if (_table(self)->Get(key,dest))
            return true;
          // delegation
          if (_table(self)->GetDelegate()) {
            return LintGet(SQObjectPtr(_table(self)->GetDelegate()),key,dest,opExt);
          }
          if (opExt & _OPEXT_GET_RAW) {
            return false;
          }
          // niDebugFmt(("... _table_ddel: %s", _TableToString(_table_ddel,"")));
          return _ddel(ss,table)->Get(key,dest);
        }
      case OT_IUNKNOWN:
        // TODO: This should not happen, it should be a sInterfaceDef
        return false;
      case OT_ARRAY:
        if (sq_isnumeric(key)) {
          return _array(self)->Get(toint(key),dest);
        }
        else {
          return _ddel(ss,array)->Get(key,dest);
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
        else return _ddel(ss,string)->Get(key,dest);
        break;

      case OT_USERDATA:
        {
          niLet ud = _userdata(self);
          switch (ud->GetType()) {
            case eScriptType_InterfaceDef: {
              niLet idef = as_nn(((sScriptTypeInterfaceDef*)ud)->pInterfaceDef);
              if (!sq_isstring(key)) {
                dest = niNew sScriptTypeErrorCode(
                  _ss,
                  _HC(error_code_cant_find_method_def),
                  niFmt("Looking in interface_def '%s', key '%s' isn't a string.",
                        idef->maszName, _ObjToString(key)));
                return false;
              }
              else if (_stringhval(key) == _HC(QueryInterface)) {
                dest = _lintFuncCallQueryInterface;
                return true;
              }
              else {
                niLet intfDel = _ss.GetInterfaceDelegate(*idef->mUUID);
                if (intfDel == _null_) {
                  dest = niNew sScriptTypeErrorCode(
                    _ss,
                    _HC(error_code_cant_find_method_def),
                    niFmt("Cant find delegate for interface_def '%s'.",
                          idef->maszName));
                }
                else {
                  if (_table(intfDel)->Get(key,dest)) {
                    return true;
                  }
                  else {
                    dest = niNew sScriptTypeErrorCode(
                      _ss,
                      _HC(error_code_cant_find_method_def),
                      niFmt("method_def<%s::%s>", idef->maszName, _stringhval(key)));
                  }
                }
              }
              return false;
            }
          }

          bool getRetVal = false;
          if (ud->GetDelegate()) {
            getRetVal = LintGet(
              SQObjectPtr(ud->GetDelegate()),
              key,dest,opExt|_OPEXT_GET_RAW);
            if (!getRetVal) {
              if (opExt & _OPEXT_GET_RAW)
                return false;
              // TODO: Lint call _get metamethod? Probably overkill.
             return false;
            }
          }
          return getRetVal;
        }
        break;

      case OT_INTEGER:case OT_FLOAT:
        return _ddel(ss,number)->Get(key,dest);
      case OT_CLOSURE: case OT_NATIVECLOSURE:
        return _ddel(ss,closure)->Get(key,dest);
      default:
        return false;
    }
  }

  bool LintSet(const SQObjectPtr &self,const SQObjectPtr &key,const SQObjectPtr &val, int opExt)
  {
    switch(_sqtype(self)) {
      case OT_TABLE: {
        SQObjectPtr v;
        // niDebugFmt(("... _LintSet: '%s' in %s", _ObjToString(key), _ExpandedObjToString(self)));
        if (!LintGet(self,key,v,0)) {
          return false;
        }
        return _table(self)->Set(key,val);
      }
      case OT_IUNKNOWN:
        // TODO: Ideally we'd lookup the methods in the interface, etc...
        return false;
      case OT_ARRAY:
        if (!sq_isnumeric(key)) {
          return false;
        }
        return _array(self)->Set(toint(key),val);
      case OT_USERDATA:
        // TODO: We could get the delegate and somehow have it tell us whether
        // we can set what's asked...
        return false;
    }
    return false;
  }

  bool LintNewSlot(const SQObjectPtr &self,const SQObjectPtr &key,const SQObjectPtr &val)
  {
    switch(_sqtype(self)) {
      case OT_TABLE: {
        return _table(self)->NewSlot(key,val);
      }
      default: {
        return false;
      }
    }
    return false;
  }

  static bool _LintTypeCanAssign(ain<eScriptType> aFromType, ain<eScriptType> aToType) {
    if (aFromType != aToType) {
      return eFalse;
    }
    return eTrue;
  }

  static eScriptType _GetResolvedObjType(const SQObjectPtr& aObj) {
    niLet type = sqa_getscriptobjtype(aObj);
    if (type == eScriptType_ResolvedType) {
      return ((sScriptTypeResolvedType*)_userdata(aObj))->_scriptType;
    }
    return type;
  }

  static bool LintTypeObjCanAssign(const SQObjectPtr& aFromTypeObj, const SQObjectPtr& aToTypeObj) {
    niLetMut fromType = _GetResolvedObjType(aFromTypeObj);
    niLet toType = _GetResolvedObjType(aToTypeObj);
    return _LintTypeCanAssign(fromType, toType);
  }
};

static SQObjectPtr _MakeScriptErrorCode(ain<sLinter> aLinter, ain_nn_mut<iHString> aKind, const achar* aMsg) {
  return niNew sScriptTypeErrorCode(aLinter._ss, aKind, aMsg);
}

static SQObjectPtr _MakeLintCallError(ain<sLinter> aLinter, const achar* aMsg) {
  return _MakeScriptErrorCode(aLinter, _HC(error_code_lint_call_error), aMsg);
}

static QPtr<sScriptTypeErrorCode> _GetScriptErrorCode(ain<SQObjectPtr> aRet) {
  if (sqa_getscriptobjtype(aRet) == eScriptType_ErrorCode) {
    niLet unresolved = (sScriptTypeErrorCode*)_userdata(aRet);
    if (unresolved->_hspKind == _HC(error_code_lint_call_error)) {
      return unresolved;
    }
  }
  return nullptr;
}

struct sLintFuncCallCreateInstance : public ImplRC<iLintFuncCall> {
  NN<iHString> _name;

  sLintFuncCallCreateInstance(const iHString* aName)
      : _name(aName)
  {}

  virtual nn<iHString> __stdcall GetName() const {
    return _name;
  }

  virtual tI32 __stdcall GetArity() const {
    return -1;
  }

  virtual SQObjectPtr __stdcall LintCall(ain<sLinter> aLinter, ain<astl::vector<SQObjectPtr>> aCallArgs)
  {
    niLet numParams = aCallArgs.size() - 1;
    if (numParams < 1) {
      return _MakeLintCallError(aLinter,niFmt("not enough arguments '%d', expected at least 1.", numParams));
    }
    if (numParams > 3) {
      return _MakeLintCallError(aLinter,niFmt("too many arguments '%d', expected at most 3.", numParams));
    }

    niLet objTypeName = aCallArgs[1];
    // niDebugFmt(("... LintCall: objTypeName: %s", _ObjToString(objTypeName)));
    if (sqa_getscriptobjtype(objTypeName) == eScriptType_String) {
      niLet objTypeDef = aLinter.FindObjectTypeDef(_stringval(objTypeName));
      if (!objTypeDef.has_value()) {
        return _MakeLintCallError(aLinter,niFmt("Cant find object type '%s'.", _stringhval(objTypeName)));
      }
    }

    return niNew sScriptTypeInterfaceDef(
      aLinter._ss,
      ni::GetLang()->GetInterfaceDefFromUUID(niGetInterfaceUUID(ni::iUnknown)));
  }
};

struct sLintFuncCallLintAssertType : public ImplRC<iLintFuncCall> {
  NN<iHString> _name;

  sLintFuncCallLintAssertType(const iHString* aName)
      : _name(aName)
  {}

  virtual nn<iHString> __stdcall GetName() const {
    return _name;
  }

  virtual tI32 __stdcall GetArity() const {
    return 2;
  }

  virtual SQObjectPtr __stdcall LintCall(ain<sLinter> aLinter, ain<astl::vector<SQObjectPtr>> aCallArgs)
  {
    niLet& expectedTypeArg = aCallArgs[1];
    niLet& actualObject = aCallArgs[2];

    if (sqa_getscriptobjtype(expectedTypeArg) != eScriptType_String) {
      return _MakeLintCallError(
        aLinter,niFmt("First parameter should be the expected type name as a literal string but got '%s'.", _ObjToString(expectedTypeArg)));
    }

    niLet expectedType = _stringhval(expectedTypeArg);
    niLet actualType = _ObjTypeString(actualObject);
    if (!actualType.IEq(niHStr(expectedType))) {
      return _MakeLintCallError(
        aLinter,
        niFmt("Expected type '%s' but got '%s'.",
              expectedType, actualType));
    }

    return _one_;
  }
};

struct sLintFuncCallQueryInterface : public ImplRC<iLintFuncCall> {
  sLintFuncCallQueryInterface()
  {}

  virtual nn<iHString> __stdcall GetName() const {
    return _HC(QueryInterface);
  }

  virtual tI32 __stdcall GetArity() const {
    return 1;
  }

  virtual SQObjectPtr __stdcall LintCall(ain<sLinter> aLinter, ain<astl::vector<SQObjectPtr>> aCallArgs)
  {
    niLet& qiID = aCallArgs[1];
    // niDebugFmt(("... sLintFuncCallQueryInterface: qiID: %s", _ObjToString(qiID)));
    tUUID qiUUID = kuuidZero;
    switch(sqa_getscriptobjtype(qiID)) {
      case eScriptType_String: {
        qiUUID = ni::GetLang()->GetInterfaceUUID(_stringhval(qiID));
        break;
      }
      default:
        return _MakeLintCallError(
          aLinter,niFmt("Invalid interface id type '%s'.", _ObjToString(qiID)));
    }

    // niDebugFmt(("... sLintFuncCallQueryInterface: qiUUID: %s", qiUUID));
    if (qiUUID == kuuidZero) {
      return _MakeLintCallError(
        aLinter,niFmt("Cant find interface uuid '%s'.", _ObjToString(qiID)));
    }

    niLet qiDef = ni::GetLang()->GetInterfaceDefFromUUID(qiUUID);
    if (qiDef == nullptr) {
      return _MakeLintCallError(
        aLinter,niFmt("Cant find interface def '%s'.", _ObjToString(qiID)));
    }

    // niDebugFmt(("... sLintFuncCallQueryInterface: qiDef: %s", qiDef->maszName));
    return niNew sScriptTypeInterfaceDef(aLinter._ss, qiDef);
  }
};

void sLinter::RegisterBuiltinFuncs(SQTable* table) {
  // Those are hardcoded in ScriptVM.cpp, ideally it should be cleaned up
  RegisterFunc(table, "vmprint", niNew sScriptTypeMethodDef(_ss, nullptr, &kFuncDecl_vmprint));
  RegisterFunc(table, "vmprintln", niNew sScriptTypeMethodDef(_ss, nullptr, &kFuncDecl_vmprintln));
  RegisterFunc(table, "vmprintdebug", niNew sScriptTypeMethodDef(_ss, nullptr, &kFuncDecl_vmprintdebug));
  RegisterFunc(table, "vmprintdebugln", niNew sScriptTypeMethodDef(_ss, nullptr, &kFuncDecl_vmprintdebugln));
  RegisterSQRegFunctions(table, SQSharedState::_base_funcs);
  RegisterSQRegFunctions(table, SQSharedState::_concurrent_funcs);

  RegisterLintFunc(table, MakeNN<sLintFuncCallCreateInstance>(_H("CreateInstance")));
  RegisterLintFunc(table, MakeNN<sLintFuncCallCreateInstance>(_H("CreateGlobalInstance")));
  RegisterLintFunc(table, MakeNN<sLintFuncCallLintAssertType>(_H("LintAssertType")));

  _lintFuncCallQueryInterface = niNew sLintFuncCallQueryInterface();
}

struct sLintStackEntry {
  SQObjectPtr _name = _null_;
  SQObjectPtr _value = _null_;
  SQObjectPtr _provenance = _null_;
};

void SQFunctionProto::LintTrace(
  sLinter& aLinter,
  const LintClosure& aClosure) const
{
#define IARG0 inst._arg0
#define IARG1 inst._arg1
#define IARG2 inst._arg2
#define IARG3 inst._arg3
#define IEXT inst._ext

  const SQFunctionProto* thisfunc = this;
  const tBool shouldLintTrace = _ShouldKeepName(
    ni::GetProperty("niScript.LintTrace").c_str(),
    niHStr(thisfunc->GetName()), false);

  const int thisfunc_outerssize = (int)thisfunc->_outervalues.size();
  const int thisfunc_paramssize = (int)(thisfunc->_parameters.size() -
                                        thisfunc_outerssize);
  const int thisfunc_stacksize = thisfunc->_stacksize;
  niLet thisfunc_resolvedrettype = aLinter.ResolveType(thisfunc->_returntype);

  _LTRACE(("-------------------------------------------------------\n"));
  _LTRACE(("--- FUNCTION TRACE: %s\n", _FuncProtoToString(*thisfunc)));
  _LTRACE(("stacksize: %s\n", thisfunc_stacksize));
  _LTRACE(("outersize: %s\n", thisfunc_outerssize));
  _LTRACE(("paramssize: %s\n", thisfunc_paramssize));
  _LTRACE(("rettype: %s\n", _ObjToString(thisfunc_resolvedrettype)));

  auto getlinecol = [&](const SQInstruction& inst) -> sVec2i {
    return SQFunctionProto::_GetLineCol(_instructions,&inst,_lineinfos);
  };

  // Build the symbol tables & functions list
  int nlocals = 0;
  astl::vector<sLintStackEntry> stack;
  stack.resize(thisfunc_stacksize);

  // Set the names to the local variable names
  // _LTRACE(("--- LOCALS INIT ---------------------------------------\n"));
  {
    niLoop(i,_localvarinfos.size()) {
      const SQLocalVarInfo& vi = _localvarinfos[i];
      const int si = vi._pos;
      stack[si]._name = vi._name;
      nlocals = ni::Max(nlocals,si+1);
      // _LTRACE(("local[%d]: %s at s[%d] = %s",
      // i, _ObjToString(stack[si]._name),
      // si, _ObjToString(stack[si]._value)));
    }
  }

  // Set the parameters in the stack
  _LTRACE(("--- PARAMS --------------------------------------------\n"));
  {
    niLoop(pi,_parameters.size()) {
      const SQFunctionParameter& param = _parameters[pi];
      _LTRACE(("param[%d]: %s, type: %s",
               pi,
               _ObjToString(param._name),
               _ObjToString(param._type)));

      const int si = pi;
      if (si >= stack.size()) {
        _LINTERNAL_ERROR(niFmt("stack position overflow '%d' for parameter '%d', stack size: %d.",
                               si, pi, stack.size()));
        return;
      }

      if (param._name != stack[si]._name) {
        _LINTERNAL_ERROR(niFmt("stack[%d].name '%s' doesn't match the parameter[%d] name '%s'.",
                               si, _ObjToString(stack[si]._name),
                               pi, _ObjToString(param._name)));
        return;
      }

      if (stack[si]._provenance != _null_) {
        _LINTERNAL_ERROR(niFmt("stack position '%d' for parameter '%d' unexpectedly already has a provenance '%s'.",
                               si, pi, _ObjToString(stack[si]._provenance)));
        return;
      }

      stack[si]._provenance = _H(niFmt("__param%d__",pi));
      stack[si]._value = aLinter.ResolveType(param._type);
    }
  }

  // Push the outer variables on the stack
  // _LTRACE(("--- OUTERS --------------------------------------------\n"));
  {
    LintClosure* thisclosure = aLinter.GetClosure(thisfunc);
    if (thisclosure) {
      const tSize nouters = thisclosure->_outervalues.size();
      // _LTRACE(("%s outer variables", nouters>0 ? Var(nouters) : Var("No")));

      const int outerbase = thisfunc_paramssize;
      niLoop(oi,nouters) {
        const SQObjectPtr& outer = thisclosure->_outervalues[oi];
        const int si = oi + outerbase;
        if (si >= stack.size()) {
          _LINTERNAL_ERROR(niFmt("stack position overflow '%d' for outer '%d', stack size: %d.",
                                 si, oi, stack.size()));
          return;
        }

        SQObjectPtr expectedProvenance = _H(niFmt("__param%d__",thisfunc_paramssize+oi));
        if (stack[si]._provenance != expectedProvenance) {
          _LINTERNAL_ERROR(niFmt("stack position '%d' for outer '%d' unexpected provenance expected '%s' but got '%s'.",
                                 si, oi,
                                 _ObjToString(expectedProvenance),
                                 _ObjToString(stack[si]._provenance)));
          return;
        }

        stack[si]._provenance = _H(niFmt("__outer%d__",oi));
        stack[si]._value = outer;
      }
    }
    else if (thisfunc_outerssize > 0) {
      _LINTERNAL_ERROR(niFmt("outers: No closure found for function '%s' (%s).", thisfunc->GetName(), _PtrToString((tIntPtr)this)));
      return;
    }
  }

  auto islocal = [&](int stkpos) -> bool {
    if (stkpos>=nlocals)
      return false;
    else if(_sqtype(stack[stkpos]._name)!=OT_NULL)
      return true;
    return false;
  };
  auto localname = [&](unsigned int stkpos) -> SQObjectPtr {
    if (!islocal(stkpos))
      return _null_;
    return stack[stkpos]._name;
  };
  auto localindex = [&](const SQObjectPtr& aName) -> int {
    niLoopr(i,_localvarinfos.size()) {
      if (_stringeq(_localvarinfos[i]._name,aName)) {
        return _localvarinfos[i]._pos;
      }
    }
    return -1;
  };

  auto sset = [&](const int i, const SQObjectPtr& v) {
    if (i >= thisfunc_stacksize) {
      _LINTERNAL_ERROR(niFmt("sset: Invalid stack position '%d'",i));
      return;
    }
    stack[i]._value = v;
  };
  auto sget = [&](const int i) -> const SQObjectPtr& {
    if (i >= thisfunc_stacksize) {
      _LINTERNAL_ERROR(niFmt("sget: Invalid stack position '%d'",i));
      return _null_;
    }
    return stack[i]._value;
  };
  auto sstr = [&](const int arg) -> cString {
    return islocal(arg) ?
        niFmt("l[%d:%s]", arg, _ObjToString(localname(arg))) :
        niFmt("s[%d]", arg);
  };
  auto lget = [&](const int i) -> const SQObjectPtr& {
    if (i >= _literals.size()) {
      _LINTERNAL_ERROR(niFmt("lget: Invalid literal index '%d'",i));
      return _null_;
    }
    return _literals[i];
  };
  auto fnget = [&](const int i) -> const SQObjectPtr& {
    if (i >= _functions.size()) {
      _LINTERNAL_ERROR(niFmt("fnget: Invalid function index '%d'",i));
      return _null_;
    }
    return _functions[i];
  };

  auto op_newtable = [&](const SQInstruction& inst) {
    SQObjectPtr table = SQTable::Create();
    SQObjectPtr lname = localname(IARG0);
    if (!lname.IsNull()) {
      _table(table)->SetDebugName(_stringhval(lname));
    }
    _LTRACE(("op_newtable: %s, size: %d -> %s", sstr(IARG0), IARG1, _ObjToString(table)));
    sset(IARG0, table);
  };

  auto op_load = [&](const SQInstruction& inst) {
    SQObjectPtr v = lget(IARG1);
    _LTRACE(("op_load: %s -> %s", _ObjToString(v), sstr(IARG0)));
    sset(IARG0, v);
  };

  auto op_loadnull = [&](const SQInstruction& inst) {
    _LTRACE(("op_loadnull: %s", sstr(IARG0)));
    sset(IARG0, _null_);
  };

  auto op_loadroottable = [&](const SQInstruction& inst) {
    _LTRACE(("op_loadroottable: %s, %s", sstr(IARG0), _ObjToString(aClosure._root)));
    sset(IARG0, aClosure._root);
  };

  auto op_move = [&](const SQInstruction& inst) {
    SQObjectPtr v = sget(IARG1);
    _LTRACE(("op_move: %s -> %s", _ObjToString(v), sstr(IARG0)));
    sset(IARG0, v);
  };

  auto op_return = [&](const SQInstruction& inst) {
    if (IARG0 != 0xFF) {
      SQObjectPtr rval = sget(IARG1);
      _LTRACE(("op_return: %s", _ObjToString(rval)));
      if (sq_isnull(thisfunc->_returntype)) {
        // We don't do type checking if the return type is null as this is
        // essentially the "any" type.
      }
      else if (!aLinter.LintTypeObjCanAssign(rval, thisfunc_resolvedrettype)) {
        if (_LENABLED(ret_type_cant_assign)) {
          _LINT(ret_type_cant_assign, niFmt(
            "Cant assign type '%s' to return type '%s'. %s -> %s.",
            sqa_getscripttypename(aLinter._GetResolvedObjType(rval)),
            sqa_getscripttypename(aLinter._GetResolvedObjType(thisfunc_resolvedrettype)),
            _ObjToString(rval),
            _ObjToString(thisfunc_resolvedrettype)));
        }
      }
    }
    else {
      _LTRACE(("op_return: default"));
    }
  };

  auto op_closure = [&](const SQInstruction& inst) {
    const SQObjectPtr& func = fnget(IARG1);
    if (func.IsNull()) {
      _LINTERNAL_ERROR(niFmt("Cant get function '%d'.", IARG1));
      return;
    }

    // by default the closure's bound this is the local this
    const int localthisindex = localindex(SQObjectPtr(_HC(this)));
    const SQObjectPtr& localthis = sget(localthisindex);
    if (localthis.IsNull()) {
      _LINTERNAL_ERROR(niFmt("Cant get local this in function '%d'.", IARG1));
      return;
    }

    _LTRACE(("op_closure: %s = %s, this: [localindex: %d] %s, root: %s",
             sstr(IARG0),
             _ObjToString(func),
             localthisindex,
             _ObjToString(localthis),
             _ObjToString(aClosure._root)));

    SQFunctionProto* funcproto = _funcproto(func);
    Ptr<LintClosure> lintClosure = niNew LintClosure(
      funcproto,_table(aClosure._root),_table(localthis));
    {
      const tSize nouters = _funcproto(func)->_outervalues.size();
      if (nouters) {
        lintClosure->_outervalues.reserve(nouters);
        niLoop(i, nouters) {
          const SQOuterVar &v = funcproto->_outervalues[i];
          if (!v._blocal) { // environment object
            lintClosure->_outervalues.push_back(_null_);
            if (!aLinter.LintGet(
                  localthis,v._src,
                  lintClosure->_outervalues.back(),0))
            {
              if (_LENABLED(this_key_notfound_outer)) {
                _LINT(this_key_notfound_outer, niFmt(
                  "outer value %s not found in %s.",
                  _ObjToString(v._src), _ObjToString(localthis)));
              }
            }
          }
          else {//local
            lintClosure->_outervalues.push_back(sget(_int(v._src)));
          }
        }
      }
    }
    if (aLinter.AddClosure(lintClosure)) {
      _LTRACE(("Added closure %s to %s (%s)",
               _PtrToString((tIntPtr)lintClosure.ptr()),
               _ObjToString(funcproto),
               _PtrToString((tIntPtr)funcproto)));
    }
    else {
      _LTRACE(("Couldn't add closure %s to %s (%s)",
               _PtrToString((tIntPtr)lintClosure.ptr()),
               _ObjToString(funcproto),
               _PtrToString((tIntPtr)funcproto)));
    }

    sset(IARG0, func);
  };

  auto op_newslot = [&](const SQInstruction& inst) {
    SQObjectPtr table = sget(IARG1);
    SQObjectPtr k = sget(IARG2);
    SQObjectPtr v = sget(IARG3);
    if (_sqtype(table) == OT_TABLE) {
      if (aLinter.EnableFromSlot(thisfunc,table,k,v)) {
        _LTRACE(("op_newslot: EnableFromSlot: __lint: %s = %s",
                 _ObjToString(k), _ObjToString(v)));
      }
      _table(table)->NewSlot(k,v);
      // When we set a function as a new slot in a table, that table becomes
      // the default 'this' for the purpose of linting.
      if (sq_type(v) == OT_FUNCPROTO) {
        LintClosure* closure = aLinter.GetClosure(_funcproto(v));
        if (closure) {
          closure->_this = table;
        }
        else {
          _LINTERNAL_ERROR(niFmt("Cant find closure associated with '%s'",
                                 _ObjToString(v)));
        }
        _LTRACE(("op_newslot: %s has new this %s",
                 _ObjToString(v), _ObjToString(table)));
      }
    }
    _LTRACE(("op_newslot: %s = %s in %s -> %s",
             _ObjToString(k),
             _ObjToString(v),
             _ObjToString(table),
             sstr(IARG0)));
    if (IARG0 != IARG3) {
      SQObjectPtr r = sget(IARG3);
      sset(IARG0, r);
      _LTRACE(("op_newslot: IARG0 != IARG3, %d != %d. Set s[%d] to %s",
               IARG0, IARG3, IARG0, _ObjToString(r)));
    }
  };

  auto is_this_table = [&](const int aTableArg) -> tBool {
    return aTableArg == 0;
    // return _intptr(sget(aTableArg)) == _intptr(aLinter._roottable);
  };

  auto is_implicit_this = [&](const sLint& aLint, const SQInstruction& inst, const int aTableArg) -> tBool {
    return (aLinter.IsEnabled(aLint.key) &&
            niFlagIsNot(inst._ext,_OPEXT_EXPLICIT_THIS) &&
            is_this_table(aTableArg));
  };

  auto is_this_key_notfound = [&](const sLint& aLint,
                                  const SQInstruction& inst,
                                  const int aTableArg,
                                  const SQObjectPtr& t,
                                  const SQObjectPtr& k,
                                  SQObjectPtr& v)
      -> tBool
  {
    niLet didGet = aLinter.LintGet(t,k,v,inst._ext);
    return (aLinter.IsEnabled(aLint.key) &&
            is_this_table(aTableArg) && // is this call
            !didGet);
  };

  auto is_key_notfound = [&](const sLint& aLint,
                             const SQInstruction& inst,
                             const SQObjectPtr& t,
                             const SQObjectPtr& k,
                             SQObjectPtr& v)
      -> tBool
  {
    niLet didGet = aLinter.LintGet(t,k,v,inst._ext);
    return (aLinter.IsEnabled(aLint.key) && !didGet);
  };

  auto op_getk = [&](const SQInstruction& inst) {
    SQObjectPtr t = sget(IARG2);
    SQObjectPtr k = lget(IARG1);
    SQObjectPtr v;
    if (is_implicit_this(_LOBJ(implicit_this_getk),inst,inst._arg2)) {
      _LINT(implicit_this_getk, niFmt(
        "implicit this access to %s", _ObjToString(k)));
    }
    if (is_this_key_notfound(_LOBJ(this_key_notfound_getk),inst,inst._arg2,t,k,v)) {
      _LINT(this_key_notfound_getk, niFmt(
        "%s not found in %s.",
        _ObjToString(k), _ObjToString(t)));
    }
    else if (is_key_notfound(_LOBJ(key_notfound_getk),inst,t,k,v)) {
      _LINT(key_notfound_getk, niFmt(
        "%s not found in %s.",
        _ObjToString(k), _ObjToString(t)));
    }
    sset(IARG0, v);
    _LTRACE(("op_getk: %s[%s] in %s",
             _ObjToString(t), _ObjToString(k), sstr(IARG0)));
  };

  auto op_precallk = [&](const SQInstruction& inst) {
    SQObjectPtr t = sget(IARG2);
    SQObjectPtr k = lget(IARG1);
    SQObjectPtr v;
    if (is_implicit_this(_LOBJ(implicit_this_callk),inst,inst._arg2)) {
      _LINT(implicit_this_getk, niFmt(
        "implicit this access to %s", _ObjToString(k)));
    }
    if (sq_isnull(t)) {
      if (_LENABLED(getk_null)) {
        _LINT(this_key_notfound_callk, niFmt(
          "%s not found in Null.",
          _ObjToString(k)));
      }
    }
    else if (is_this_key_notfound(_LOBJ(this_key_notfound_callk),inst,inst._arg2,t,k,v)) {
      _LINT(this_key_notfound_callk, niFmt(
        "%s not found in %s.",
        _ObjToString(k), _ObjToString(t)));
    }
    else if (is_key_notfound(_LOBJ(key_notfound_callk),inst,t,k,v)) {
      _LINT(key_notfound_callk, niFmt(
        "%s not found in %s.",
        _ObjToString(k), _ObjToString(t)));
    }
    sset(IARG3, t);
    sset(IARG0, v);
    _LTRACE(("op_precallk: f = %s (%s), t = %s (%s), k = %s",
             _ObjToString(v), sstr(IARG0),
             _ObjToString(t), sstr(IARG3),
             _ObjToString(k)));
  };

  auto is_this_set_key_notfound = [&](const sLint& aLint,
                                      const SQInstruction& inst,
                                      const int aThisArg,
                                      const SQObjectPtr& t,
                                      const SQObjectPtr& k,
                                      const SQObjectPtr& v)
      -> tBool
  {
    return (aLinter.IsEnabled(aLint.key) &&
            is_this_table(aThisArg) &&
            !aLinter.LintSet(t,k,v,inst._ext));
  };

  auto is_set_key_notfound = [&](const sLint& aLint,
                                 const SQInstruction& inst,
                                 const SQObjectPtr& t,
                                 const SQObjectPtr& k,
                                 const SQObjectPtr& v)
      -> tBool
  {
    return (aLinter.IsEnabled(aLint.key) &&
            !aLinter.LintSet(t,k,v,inst._ext));
  };

  auto op_set = [&](const SQInstruction& inst) {
    SQObjectPtr t = sget(IARG1);
    SQObjectPtr k = sget(IARG2);
    SQObjectPtr v = sget(IARG3);

    if (is_this_set_key_notfound(_LOBJ(this_set_key_notfound),inst,inst._arg1,t,k,v))
    {
      _LINT(this_set_key_notfound, niFmt(
        "%s not found in %s.",
        _ObjToString(k), _ObjToString(t)));
    }
    else if (is_set_key_notfound(_LOBJ(set_key_notfound),inst,t,k,v)) {
      _LINT(set_key_notfound, niFmt(
        "%s not found in %s.",
        _ObjToString(k), _ObjToString(t)));
    }

    if (inst._arg0 != inst._arg3)
      sset(IARG0, v);
    _LTRACE(("op_setk: %s[%s] = %s in %s",
             _ObjToString(t), _ObjToString(k),
             _ObjToString(v), sstr(IARG0)));
  };

  auto op_call = [&](const SQInstruction& inst, const tBool abIsTailCall) {
    SQObjectPtr tocall = sget(IARG1);
    niLet tocalltype = sqa_getscriptobjtype(tocall);
    niLet nargs = IARG3;
    niLet stackbase = IARG2;
    _LTRACE(("op_call: '%s' (%s), nargs: %s, stackbase: %d, tailcall: %s",
             _ObjToString(tocall),
             ni::GetTypeString(tocalltype),
             nargs,
             stackbase,
             abIsTailCall?"yes":"no"));

    auto call_func = [&](ain<SQFunctionProto> func) -> SQObjectPtr {
      const int outerssize = (int)func._outervalues.size();
      const int paramssize = (int)(func._parameters.size() - outerssize);
      const int arity = paramssize-1; // number of paramssize-1 for the implicit this

      _LTRACE(("call_func: %s", _FuncProtoToString(func)));

      if (_LENABLED(call_num_args) && (paramssize != nargs)) {
        _LINT(call_num_args,
              niFmt("call_func: Incorrect number of arguments passed, expected %d but got %d. Calling %s.",
                    arity, nargs-1, _FuncProtoToString(func)));
      }

      niLet rettype = aLinter.ResolveType(func._returntype);
      return rettype;
    };

    auto call_nativeclosure = [&](ain<SQNativeClosure> func) -> SQObjectPtr {
      niLet nparamscheck = func._nparamscheck;
      _LTRACE(("call_func: %s, nparamscheck: %d", _NativeClosureToString(func), nparamscheck));

      if (_LENABLED(call_num_args)) {
        if ((nparamscheck < 0) && (nargs < (-nparamscheck))) {
          _LINT(call_num_args,
                niFmt("call_nativeclosure: Incorrect number of arguments, expected at least %d but got %d. Calling %s.",
                      (-func._nparamscheck)-1, nargs-1, _NativeClosureToString(func)));
        }
        else if ((nparamscheck > 0) && (nparamscheck != nargs)) {
          _LINT(call_num_args,
                niFmt("call_nativeclosure: Incorrect number of arguments, expected %d but got %d. Calling %s.",
                      func._nparamscheck-1, nargs-1, _NativeClosureToString(func)));
        }
      }

      niLet rettype = aLinter.ResolveType(func._returntype);
      return rettype;
    };

    auto call_method = [&](ain<sScriptTypeMethodDef> aMeth) -> SQObjectPtr {
      const int paramssize = (int)aMeth.pMethodDef->mnNumParameters + 1;
      // number of paramssize-1 for the implicit this
      const int arity = paramssize-1;

      _LTRACE(("call_method: %s", aMeth.GetTypeString()));

      if (_LENABLED(call_num_args) && (paramssize != nargs)) {
        _LINT(call_num_args,
              niFmt("call_method: Incorrect number of arguments passed, expected %d but got %d. Calling %s.",
                    arity, nargs-1, aMeth.GetTypeString()));
        return _null_;
      }

      SQObjectPtr retType;
      niLet scriptType = sqa_type2scripttype(aMeth.pMethodDef->mReturnType);
      switch (scriptType) {
        case eScriptType_Null:
        case eScriptType_Int:
        case eScriptType_Float:
        case eScriptType_Vec2:
        case eScriptType_Vec3:
        case eScriptType_Vec4:
        case eScriptType_Matrix:
        case eScriptType_String:
        case eScriptType_UUID: {
          retType = niNew sScriptTypeResolvedType(
            aLinter._ss,
            scriptType);
          break;
        }

        case eScriptType_IUnknown: {
          if (aMeth.pMethodDef->mReturnTypeUUID) {
            retType = aLinter.ResolveTypeUUID(
              aMeth.pMethodDef->mReturnTypeName,
              *aMeth.pMethodDef->mReturnTypeUUID);
          }
          else {
            retType = niNew sScriptTypeErrorCode(
              aLinter._ss,
              _HC(error_code_method_def_invalid_ret_type),
              niFmt("iunknown_not_type_uuid<%s::%s, %s(%s)>",
                    aMeth.pInterfaceDef->maszName,
                    aMeth.pMethodDef->maszName,
                    GetTypeString(aMeth.pMethodDef->mReturnType),
                    aMeth.pMethodDef->mReturnTypeName));
          }
          break;
        }

          // VM Internal types or invalid types that shouldn't be returned by the interop
        case eScriptType_EnumDef:
        case eScriptType_InterfaceDef:
        case eScriptType_MethodDef:
        case eScriptType_PropertyDef:
        case eScriptType_IndexedProperty:
        case eScriptType_Iterator:
        case eScriptType_ErrorCode:
        case eScriptType_ResolvedType:
        case eScriptType_Table:
        case eScriptType_Array:
        case eScriptType_UserData:
        case eScriptType_Closure:
        case eScriptType_NativeClosure:
        case eScriptType_FunctionProto:
        case eScriptType_Invalid: {
          // TODO: This is a bit shit, we probalby need some kind of
          // sScriptTypeError thing
          retType = niNew sScriptTypeErrorCode(
            aLinter._ss,
            _HC(error_code_method_def_invalid_ret_type),
            niFmt("method_def<%s::%s, %s(%s)>",
                  aMeth.pInterfaceDef->maszName,
                  aMeth.pMethodDef->maszName,
                  GetTypeString(aMeth.pMethodDef->mReturnType),
                  aMeth.pMethodDef->mReturnTypeName));
          break;
        };
      }

      return retType;
    };

    auto call_lint_func = [&](ain_nn_mut<iLintFuncCall> aLintFunc) -> SQObjectPtr {
      niLet arity = aLintFunc->GetArity();
      _LTRACE(("call_lint_func: %s/%d, nargs: %d, stackbase: %d",
               aLintFunc->GetName(), arity, nargs, stackbase));

      if (_LENABLED(call_num_args) && (arity >= 0) && (arity+1 != nargs)) {
        _LINT(call_num_args,
              niFmt("call_lint_func: Incorrect number of arguments passed, expected %d but got %d. Calling %s/%d.",
                    arity, nargs-1, aLintFunc->GetName(), arity));
        return _null_;
      }

      astl::vector<SQObjectPtr> vArgs;
      vArgs.resize(nargs);
      niLoop(pi, nargs) {
        vArgs[pi] = sget(stackbase+pi);
        // niDebugFmt(("... vArgs[%d]: %s", pi, _ObjToString(vArgs[pi])));
      }
      return aLintFunc->LintCall(aLinter, vArgs);
    };

    switch (tocalltype) {
      case eScriptType_Null: {
        if (_LENABLED(call_null)) {
          _LINT(call_null, "Attempting to call Null.");
        }
        sset(IARG0, _null_);
        break;
      }
      case eScriptType_Closure: {
        SQClosure* closure = _closure(tocall);
        SQFunctionProto* func = _funcproto(closure->_function);
        if (!func) {
          _LINT(internal_error, niFmt("Invalid closure object, no function prototype. Calling '%s'.", _ObjToString(tocall)));
        }
        else {
          niLet ret = call_func(*func);
          sset(IARG0, ret);
        }
        break;
      }
      case eScriptType_FunctionProto: {
        SQFunctionProto* func = _funcproto(tocall);
        niLet ret = call_func(*func);
        sset(IARG0, ret);
        break;
      }
      case eScriptType_NativeClosure: {
        SQNativeClosure* nativeclosure = _nativeclosure(tocall);
        niLet ret = call_nativeclosure(*nativeclosure);
        sset(IARG0, ret);
        break;
      }
      case eScriptType_MethodDef: {
        niLet mdef = (sScriptTypeMethodDef*)_userdata(tocall);
        niLet ret = call_method(*mdef);
        sset(IARG0, ret);
        break;
      }
      case eScriptType_IUnknown: {
        QPtr<iLintFuncCall> lintFuncCall = _iunknown(tocall);
        if (lintFuncCall.IsOK()) {
          niLet ret = call_lint_func(as_NN(lintFuncCall));
          // niDebugFmt(("... call_lint_func(%s): ret: %s", lintFuncCall->GetName(), _ObjToString(ret)));
          niLet lintCallError = _GetScriptErrorCode(ret);
          if (!lintCallError.IsOK()) {
            // no error
            sset(IARG0, ret);
          }
          else {
            if (_LENABLED(call_error)) {
              _LINT(call_error, niFmt("call_lint_func: %s/%s: %s", lintFuncCall->GetName(), lintFuncCall->GetArity(), lintCallError->_strDesc));
            }
            sset(IARG0, ret);
          }
        }
        else {
          if (_LENABLED(call_null)) {
            _LINT(call_null,
                  niFmt("Attempting to call iunknown '%s' (%s).",
                        _ObjToString(tocall),
                        aLinter._ss.GetTypeNameStr(_sqtype(tocall))));
          }
          sset(IARG0, _MakeLintCallError(aLinter, _ObjToString(tocall).c_str()));
        }
        break;
      };
      default: {
        if (_LENABLED(call_null)) {
          _LINT(call_null,
                niFmt("Attempting to call '%s' (%s).",
                      _ObjToString(tocall),
                      aLinter._ss.GetTypeNameStr(_sqtype(tocall))));
        }
        sset(IARG0, _MakeLintCallError(aLinter, _ObjToString(tocall).c_str()));
        break;
      }
    }
  };

  const int localThis = localindex(SQObjectPtr(_HC(this)));
  if (localThis < 0) {
    _LINTERNAL_ERROR("Function doesn't have a local 'this'.");
    return;
  }
  sset(localThis,aClosure._this);

  if (shouldLintTrace) {
    _LTRACE(("--- LOCALS BEGIN --------------------------------------\n"));
    niLoop(i,_localvarinfos.size()) {
      const int stackpos = _localvarinfos[i]._pos;
      _LTRACE(("local[%d]: s[%d]: %s = %s, provenance: %s",
               i, stackpos,
               _ObjToString(localname(stackpos)),
               _ObjToString(sget(stackpos)),
               _ObjToString(stack[stackpos]._provenance)));
    }
    _LTRACE(("root table: %s",
             _ObjToString(aClosure._root))
            // _ExpandedObjToString(aClosure._root))
    );
  }

  _LTRACE(("--- INST ----------------------------------------------\n"));
  // prologue checks
  {
    niLet retscripttype = sqa_getscriptobjtype(thisfunc_resolvedrettype);
    if (retscripttype == eScriptType_Null && _LENABLED(ret_type_is_null)) {
      niLet inst = 0; // placeholder
      niLet getlinecol = [&](niLet _) {
        return Vec2i(thisfunc->_sourceline, 0);
      };
      _LINT(ret_type_is_null, niFmt(
        "Return type is Null, specify an explicit return type.",
        _ObjToString(thisfunc->_returntype)));
    }
    else if (retscripttype == eScriptType_ErrorCode) {
      _LINTERNAL_ERROR(niFmt("Invalid function return type '%s'.",
                             _ObjToString(thisfunc_resolvedrettype)));
    }
  }

  // instructioncs check
  niLoop(i, _instructions.size()) {
    const SQInstruction &inst=_instructions[i];
    _LTRACE(("%s %d %d %d %d",
             _GetOpDesc(inst.op),
             inst._arg0, inst._arg1, inst._arg2, inst._arg3));
    switch (inst.op) {
      case _OP_NEWTABLE: op_newtable(inst); break;
      case _OP_LOAD: op_load(inst); break;
      case _OP_LOADNULL: op_loadnull(inst); break;
      case _OP_LOADROOTTABLE: op_loadroottable(inst); break;
      case _OP_MOVE: op_move(inst); break;
      case _OP_RETURN: op_return(inst); break;
      case _OP_CLOSURE: op_closure(inst); break;
      case _OP_NEWSLOT: op_newslot(inst); break;
      case _OP_GETK: op_getk(inst); break;
      case _OP_PREPCALLK: op_precallk(inst); break;
      case _OP_SET: op_set(inst); break;
      case _OP_CALL: op_call(inst,eFalse); break;
      case _OP_TAILCALL: op_call(inst,eTrue); break;
      default: {
        break;
      }
    }
  }

  _LTRACE(("--- LOCALS AFTER --------------------------------------\n"));
  niLoop(i,_localvarinfos.size()) {
    const int stackpos = _localvarinfos[i]._pos;
    _LTRACE(("local[%d]: s[%d]: %s = %s, provenance: %s",
             i, stackpos,
             _ObjToString(localname(stackpos)),
             _ObjToString(sget(stackpos)),
             _ObjToString(stack[stackpos]._provenance)));
  }
  _LTRACE(("root table: %s",
           _ObjToString(aClosure._root))
          // _ExpandedObjToString(aClosure._root))
  );

  _LTRACE(("--- FUNCS ---------------------------------------------\n"));
  if (_functions.empty()) {
    _LTRACE(("No functions."));
  } else
    niLoop(i,_functions.size()) {
      const SQObjectPtr& cf = _functions[i];
      const SQFunctionProto* cfproto = _funcproto(cf);
      const LintClosure* cfclosure = aLinter.GetClosure(cfproto);
      if (!cfclosure) {
        _LINTERNAL_ERROR(niFmt("Cant get closure for func[%d]: %s",
                               i, _ObjToString(cf)));
      }
      else {
        _LTRACE(("func[%d]: %s, this: %s",
                 i,
                 _ObjToString(cfproto->_name),
                 _ObjToString(cfclosure->_this)));
        sLinter::tLintKindMap wasLintEnabled = aLinter._lintEnabled;
        cfproto->LintTrace(aLinter,*cfclosure);
        aLinter._lintEnabled = wasLintEnabled;
      }
    }

  _LTRACE(("--- END FUNCTION TRACE: %s --------------------------------\n", thisfunc->GetName()));

#undef IARG0
#undef IARG1
#undef IARG2
#undef IARG3
#undef IEXT
}

void SQFunctionProto::LintTraceRoot() {
  sLinter linter;

  SQObjectPtr moduleRoot = SQTable::Create();
  _table(moduleRoot)->SetDebugName("__moduleroot__");
  _table(moduleRoot)->SetDelegate(_table(linter._vmroot));

  LintClosure rootClosure(this,_table(moduleRoot),_table(moduleRoot));
  this->LintTrace(linter,rootClosure);

#if !defined SQLINTER_LOG_INLINE
  niLoop(i, linter._logs.size()) {
    niPrintln(linter._logs[i]);
  }
#endif
}
