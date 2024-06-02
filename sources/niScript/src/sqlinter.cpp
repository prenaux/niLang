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

#define SQLINTER_LOG_INLINE

niDeclareModuleTrace_(niScript,LintDump);
niDeclareModuleTrace_(niScript,LintTrace);

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
    default:
      return niFmt("%s::%s",(Var&)obj._var,sqa_gettypestring(obj._var.mType));
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
  const int arity = paramssize-1; // number of paramerter - 1 for the implicit 'this' argument
  return niFmt("%s/%d(%d)[%s:%d]",
               func.GetName(),
               arity,
               outerssize,
               func.GetSourceName(),
               func._sourceline);
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
#define _DEF_LINT(NAME,CAT1,CAT2) \
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
_DEF_LINT(call_warning,IsWarning,IsExplicit);
_DEF_LINT(call_num_args,IsError,None);

#undef _DEF_LINT

struct sLinter {
 private:
  sLinter(const sLinter& aLinter);

 public:
  typedef astl::hash_map<tU32,tI32> tLintKindMap;
  tLintKindMap _lintEnabled;
  SQSharedState _ss;
  SQObjectPtr _vmroot;

#if !defined SQLINTER_LOG_INLINE
  astl::vector<cString> _logs;
#endif

#define _REG_LINT(KIND) astl::upsert(_lintEnabled,_LKEY(KIND),  \
    niFlagIsNot(_LKEY(KIND),eLintFlags_IsPedantic) &&           \
    niFlagIsNot(_LKEY(KIND),eLintFlags_IsExperimental) &&       \
    niFlagIsNot(_LKEY(KIND),eLintFlags_IsExplicit))

  sLinter() {
    _vmroot = SQTable::Create();
    _table(_vmroot)->SetDebugName("__vmroot__");
    this->RegisterBuiltinFuncs(_table(_vmroot));

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
    _REG_LINT(call_warning);
    _REG_LINT(call_num_args);
  }
#undef _REG_LINT

  tBool EnableFromSlot(const SQFunctionProto* thisfunc, SQObjectPtr table, SQObjectPtr k, SQObjectPtr v) {
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

    // applies to all the lints
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
    _E(implicit_this_getk)
    _E(implicit_this_callk)
    _E(this_key_notfound_getk)
    _E(this_key_notfound_callk)
    _E(this_key_notfound_outer)
    _E(key_notfound_callk)
    _E(key_notfound_getk)
    _E(this_set_key_notfound)
    _E(set_key_notfound)
    _E(call_warning)
    _E(call_num_args)
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

  void RegisterFunc(SQTable* table, const achar* name) {
    table->NewSlot(_H(name), _null_);
  }

  int RegisterFuncs(SQTable* table, const SQRegFunction* regs) {
    int i = 0;
    while (regs[i].name != NULL) {
      RegisterFunc(table, regs[i].name);
      ++i;
    }
    return i;
  }

  void RegisterBuiltinFuncs(SQTable* table) {
    // Those are hardcode in ScriptVM.cpp, ideally it should be cleaned up
    RegisterFunc(table, "print");
    RegisterFunc(table, "println");
    RegisterFunc(table, "printdebug");
    RegisterFunc(table, "printdebugln");
    RegisterFuncs(table, SQSharedState::_base_funcs);
    RegisterFuncs(table, SQSharedState::_concurrent_funcs);
  }
};

void SQFuncState::LintDump()
{
  // unsigned int n=0,i;
  SQFunctionProto *func=_funcproto(_func);
  cString o;
  o << niFmt(_A("--------------------------------------------------------------------\n"));
  o << niFmt(_A("-----FUNCTION: %s\n"), _FuncProtoToString(*func));
  {
    o << niFmt(_A("-----INFO\n"));
    o << niFmt(_A("stack size = %d\n"),func->_stacksize);
    o << niFmt(_A("SQInstruction sizeof %d\n"),sizeof(SQInstruction));
    o << niFmt(_A("SQObject sizeof %d\n"),sizeof(SQObject));
  }
  {
    o << niFmt(_A("-----PARAMS\n"));
    for(int i=0;i<_parameters.size();i++){
      o << niFmt(_A("[%d] "),i);
      o << _ObjToString(_parameters[i]._name);
      o << ", type: ";
      o << _ObjToString(_parameters[i]._type);
      o << niFmt(_A("\n"));
    }
    o << "returntype: ";
    o << _ObjToString(_returntype);
    o << niFmt(_A("\n"));
  }
  SQObjectPtrVec vLiterals;
  {
    o << niFmt(_A("-----LITERALS\n"));
    vLiterals.resize(_nliterals);
    {
      SQObjectPtr refidx,key,val,itr;
      while (_table(_literals)->Next(refidx,key,val,itr)) {
        vLiterals[_int(val)] = key;
        refidx = itr;
      }
      for(int i=0;i<vLiterals.size();i++){
        o << niFmt(_A("[%d] "),i);
        o << _ObjToString(vLiterals[i]);
        o << niFmt(_A("\n"));
      }
    }
  }
  {
    o << niFmt(_A("-----LOCALS\n"));
    for(int i=0;i<func->_localvarinfos.size();i++){
      SQLocalVarInfo lvi=func->_localvarinfos[i];
      o << niFmt(_A("[%d] %s \t%d %d\n"),lvi._pos,_stringval(lvi._name),lvi._start_op,lvi._end_op);
    }
  }
  {
    o << niFmt(_A("-----LINE INFO\n"));
    for(int i=0;i<_lineinfos.size();i++){
      SQLineInfo li=_lineinfos[i];
      o << niFmt(_A("op [%d] line [%d]\n"),li._op,li._line);
    }
  }
  {
    o << niFmt(_A("-----Instructions\n"));
    for(int i=0;i<_instructions.size();i++){
      SQInstruction &inst=_instructions[i];
      const sVec2i linecol = SQFunctionProto::_GetLineCol(_instructions,&inst,_lineinfos);
      if (inst.op==_OP_LOAD ||
          inst.op==_OP_PREPCALLK ||
          inst.op==_OP_GETK ||
          ((inst.op==_OP_ADD ||
            inst.op==_OP_SUB ||
            inst.op==_OP_MUL ||
            inst.op==_OP_DIV) &&
           inst._arg3==0xFF))
      {
        o << niFmt(_A("[%03d:L%03d:C%03d] %20s %d "), i, linecol.x, linecol.y,
                   _GetOpDesc(inst.op), inst._arg0);
        if (inst._arg1 == 0xFFFF) {
          o << niFmt(_A("null"));
        } else {
          cString literalStr;
          if (inst._arg1 < vLiterals.size()) {
            literalStr = _ObjToString(vLiterals[inst._arg1]);
          }
          cString keyStr = _ObjToString(_GetKeyFromArg(_literals, inst._arg1));
          o << inst._arg1
            << "["
            << ((!literalStr.Eq(keyStr)) ? "INVALID_ARG1, " : "")
            << keyStr
            << "]";
        }
        if (inst.op == _OP_GETK) {
          if (inst._arg2 == 0) {
            o << niFmt(_A(" this %d "),inst._arg3);
          }
          else {
            o << niFmt(_A(" %d %d "),inst._arg2,inst._arg3);
          }
        }
        else {
          o << niFmt(_A(" %d %d "),inst._arg2,inst._arg3);
        }
      }
      else {
        o << niFmt(_A("[%03d:L%03d:C%03d] %20s %d %d %d %d "), i, linecol.x,
                   linecol.y, _GetOpDesc(inst.op), inst._arg0, inst._arg1,
                   inst._arg2, inst._arg3);
      }
      _AddOpExt(o,inst._ext);
      o << "\n";
    }
  }
  o << niFmt("--------------------------------------------------------------------\n\n");
  niDebugFmt((o.Chars()));
}

void SQFuncState::LintCompileTime()
{
  if (niModuleShouldTrace_(niScript,LintDump)) {
    this->LintDump();
  }
}

struct sLintStackEntry {
  SQObjectPtr _name;
  SQObjectPtr _value;
};

bool _LintGet(const sLinter& aLinter, const SQObjectPtr &self, const SQObjectPtr &key, SQObjectPtr &dest, int opExt)
{
  const SQSharedState& ss = aLinter._ss;
  switch(_sqtype(self)){
    case OT_TABLE:
      {
        if (_table(self)->Get(key,dest))
          return true;
        // delegation
        if (_table(self)->GetDelegate()) {
          return _LintGet(
            aLinter,SQObjectPtr(_table(self)->GetDelegate()),key,dest,opExt);
        }
        if (opExt & _OPEXT_GET_RAW) {
          return false;
        }
        // niDebugFmt(("... _table_ddel: %s", _TableToString(_table_ddel,"")));
        return _ddel(ss,table)->Get(key,dest);
      }
      // TODO: Ideally we'd lookup the methods in the interface, etc...
    case OT_IUNKNOWN:
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
        bool getRetVal = false;
        if (_userdata(self)->GetDelegate()) {
          getRetVal = _LintGet(
            aLinter,
            SQObjectPtr(_userdata(self)->GetDelegate()),
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

bool _LintSet(const sLinter& aLinter, const SQObjectPtr &self,const SQObjectPtr &key,const SQObjectPtr &val, int opExt)
{
  switch(_sqtype(self)) {
    case OT_TABLE: {
      SQObjectPtr v;
      // niDebugFmt(("... _LintSet: '%s' in %s", _ObjToString(key), _ExpandedObjToString(self)));
      if (!_LintGet(aLinter,self,key,v,0)) {
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

void SQFunctionProto::LintTrace(
  sLinter& aLinter,
  const LintClosure& aClosure) const
{
#define IARG0 inst._arg0
#define IARG1 inst._arg1
#define IARG2 inst._arg2
#define IARG3 inst._arg3
#define IEXT inst._ext

  const tBool shouldLintTrace = niModuleTraceObject_(niScript,LintTrace).get(eTrue);
  const SQFunctionProto* thisfunc = this;
  _LTRACE(("--- TRACE FUNCTION ------------------------------------\n"));
  _LTRACE(("... func: %s\n", _FuncProtoToString(*thisfunc)));

  auto getlinecol = [&](const SQInstruction& inst) -> sVec2i {
    return SQFunctionProto::_GetLineCol(_instructions,&inst,_lineinfos);
  };

  // Build the symbol tables & functions list
  int nlocals = 0;
  const int stacksize = thisfunc->_stacksize;
  astl::vector<sLintStackEntry> stack;
  stack.resize(stacksize);

  // Set the names to the local variable names
  _LTRACE(("--- LOCALS --------------------------------------------\n"));
  {
    niLoop(i,_localvarinfos.size()) {
      const SQLocalVarInfo& vi = _localvarinfos[i];
      const int si = vi._pos;
      stack[si]._name = vi._name;
      nlocals = ni::Max(nlocals,si+1);
      _LTRACE(("... local[%d]: %s at s[%d]", i, _ObjToString(stack[si]._name),si));
    }
  }

  // Push the outer variables on the stack
  _LTRACE(("--- OUTERS --------------------------------------------\n"));
  LintClosure* thisclosure = aLinter.GetClosure(thisfunc);
  if (thisclosure) {
    _LTRACE(("... Has closure with %d outer variables",
             thisclosure->_outervalues.size()));
    niLoop(i,thisclosure->_outervalues.size()) {
      const int si = i + 1; // +1 because `this` is at position 0
      const SQObjectPtr& v = thisclosure->_outervalues[i];
      _LTRACE(("... outer[%d]: s[%s] = %s", i, si, _ObjToString(v)));
      stack[si]._name = _H(niFmt("__outer%d__",i));
      stack[si]._value = v;
    }
  }
  else {
    _LTRACE(("... No closure found for %s",
             _PtrToString((tIntPtr)this)));
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
    if (i >= stacksize) {
      _LINTERNAL_ERROR(niFmt("Lint: sset: Invalid stack position '%d'",i));
      return;
    }
    stack[i]._value = v;
  };
  auto sget = [&](const int i) -> const SQObjectPtr& {
    if (i >= stacksize) {
      _LINTERNAL_ERROR(niFmt("Lint: sset: Invalid stack position '%d'",i));
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
      _LINTERNAL_ERROR(niFmt("Lint: sset: Invalid literal index '%d'",i));
      return _null_;
    }
    return _literals[i];
  };
  auto fnget = [&](const int i) -> const SQObjectPtr& {
    if (i >= _functions.size()) {
      _LINTERNAL_ERROR(niFmt("Lint: fnget: Invalid function index '%d'",i));
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
    _LTRACE(("op_loadroottable: %s", sstr(IARG0)));
    sset(IARG0, aClosure._root);
  };

  auto op_closure = [&](const SQInstruction& inst) {
    const SQObjectPtr& func = fnget(IARG1);
    if (func.IsNull()) {
      _LINTERNAL_ERROR(niFmt("Can't get function '%d'.", IARG1));
      return;
    }

    // by default the closure's bound this is the local this
    const int localthisindex = localindex(SQObjectPtr(_HC(this)));
    const SQObjectPtr& localthis = sget(localthisindex);
    if (localthis.IsNull()) {
      _LINTERNAL_ERROR(niFmt("Can't get local this in function '%d'.", IARG1));
      return;
    }

    _LTRACE(("op_closure: %s = %s, this: %s, root: %s",
             sstr(IARG0),
             _ObjToString(func),
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
            if (!_LintGet(
                  aLinter,
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
      _LTRACE(("... Added closure %s to %s (%s)",
               _PtrToString((tIntPtr)lintClosure.ptr()),
               _ObjToString(funcproto),
               _PtrToString((tIntPtr)funcproto)));
    }
    else {
      _LTRACE(("... Couldn't add closure %s to %s (%s)",
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
          _LINTERNAL_ERROR(niFmt("Can't find closure associated with '%s'",
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
      _LTRACE(("... op_newslot: IARG0 != IARG3, %d != %d. Set s[%d] to %s",
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
    return (aLinter.IsEnabled(aLint.key) &&
            is_this_table(aTableArg) && // is this call
            !_LintGet(aLinter,t,k,v,inst._ext));
  };

  auto is_key_notfound = [&](const sLint& aLint,
                             const SQInstruction& inst,
                             const SQObjectPtr& t,
                             const SQObjectPtr& k,
                             SQObjectPtr& v)
      -> tBool
  {
    return (aLinter.IsEnabled(aLint.key) &&
            !_LintGet(aLinter,t,k,v,inst._ext));
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
    if (is_this_key_notfound(_LOBJ(this_key_notfound_callk),inst,inst._arg2,t,k,v)) {
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
    _LTRACE(("op_precallk: %s[%s] in %s & table in %s",
             _ObjToString(t), _ObjToString(k), sstr(IARG0), sstr(IARG3)));
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
            !_LintSet(aLinter,t,k,v,inst._ext));
  };

  auto is_set_key_notfound = [&](const sLint& aLint,
                                 const SQInstruction& inst,
                                 const SQObjectPtr& t,
                                 const SQObjectPtr& k,
                                 const SQObjectPtr& v)
      -> tBool
  {
    return (aLinter.IsEnabled(aLint.key) &&
            !_LintSet(aLinter,t,k,v,inst._ext));
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

  auto op_call = [&](const SQInstruction& inst) {
    SQObjectPtr tocall = sget(IARG1);
    const int nargs = IARG3;
    const int stackbase = IARG2;
    _LTRACE(("op_call: '%s', nargs: %s, stackbase: %d", _ObjToString(tocall), nargs, stackbase));

    auto call_func = [&](const SQFunctionProto& func) {
      const int outerssize = (int)func._outervalues.size();
      const int paramssize = (int)(func._parameters.size() - outerssize);
      const int arity = paramssize-1; // number of paramerter - 1 for the implicit 'this' argument

      _LTRACE(("call_func: %s",
               _ObjToString(tocall),
               func.GetName(),
               paramssize,
               outerssize,
               func._sourceline
              ));

      if (_LENABLED(call_num_args) && (paramssize != nargs)) {
        _LINT(call_num_args,
              niFmt("Incorrect number of arguments passed, expected %d but got %d. Calling %s.",
                    arity, nargs-1, _FuncProtoToString(func)));
      }
    };

    switch (_sqtype(tocall)) {
      case OT_NULL: {
        if (_LENABLED(call_warning)) {
          _LINT(call_warning, "Attempting to call Null.");
        }
        sset(IARG0, _null_);
        break;
      }
      case OT_CLOSURE: {
        SQClosure* closure = _closure(tocall);
        SQFunctionProto* func = _funcproto(closure->_function);
        if (!func) {
          _LINT(internal_error, niFmt("Invalid closure object, no function prototype. Calling '%s'.", _ObjToString(tocall)));
        }
        else {
          call_func(*func);
        }
        break;
      }
      case OT_FUNCPROTO: {
        SQFunctionProto* func = _funcproto(tocall);
        call_func(*func);
        break;
      }
      default: {
        if (_LENABLED(call_warning)) {
          _LINT(call_warning, niFmt("Attempting to call '%s' (%s).",
                                    _ObjToString(tocall), aLinter._ss.GetTypeNameStr(_sqtype(tocall))));
        }
        sset(IARG0, _null_);
        break;
      }
    }
  };

  const int localThis = localindex(SQObjectPtr(_HC(this)));
  if (localThis < 0) {
    _LINTERNAL_ERROR("Function doesn't have a local 'this'.");
    return;
  }
  _LTRACE(("... localthis: %d", localThis));
  sset(localThis,aClosure._this);

  _LTRACE(("--- INST ----------------------------------------------\n"));
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
      case _OP_CLOSURE: op_closure(inst); break;
      case _OP_NEWSLOT: op_newslot(inst); break;
      case _OP_GETK: op_getk(inst); break;
      case _OP_PREPCALLK: op_precallk(inst); break;
      case _OP_SET: op_set(inst); break;
      case _OP_CALL: op_call(inst); break;
      default: {
        break;
      }
    }
  }

  _LTRACE(("--- LOCALS AFTER --------------------------------------\n"));
  niLoop(i,_localvarinfos.size()) {
    const int stackpos = _localvarinfos[i]._pos;
    _LTRACE(("... local[%d]: s[%d]: %s = %s",
             i, stackpos,
             _ObjToString(localname(stackpos)),
             _RootObjToRecString(sget(stackpos))));
  }
  _LTRACE(("... root table: %s",
           _ObjToString(aClosure._root))
           // _ExpandedObjToString(aClosure._root))
  );

  _LTRACE(("--- FUNCS ---------------------------------------------\n"));
  niLoop(i,_functions.size()) {
    const SQObjectPtr& cf = _functions[i];
    const SQFunctionProto* cfproto = _funcproto(cf);
    const LintClosure* cfclosure = aLinter.GetClosure(cfproto);
    if (!cfclosure) {
      _LINTERNAL_ERROR(niFmt("Can't get closure for func[%d]: %s",
                             i, _ObjToString(cf)));
    }
    else {
      _LTRACE(("... func[%d]: %s, this: %s",
               i,
               _ObjToString(cfproto->_name),
               _ObjToString(cfclosure->_this)));
      sLinter::tLintKindMap wasLintEnabled = aLinter._lintEnabled;
      cfproto->LintTrace(aLinter,*cfclosure);
      aLinter._lintEnabled = wasLintEnabled;
    }
  }

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
