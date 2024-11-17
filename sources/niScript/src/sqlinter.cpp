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
#include "sqcompiler.h"

struct sLinter;
struct LintClosure;

// TODO: Move to StringDef.h & StringLib.cpp
niExportFuncCPP(cString&) StringCatRepeat(cString& o, ain<tI32> aN, ain<tChars> aToRepeat) {
  if (aN <= 0 || !niStringIsOK(aToRepeat)) {
    return o;
  }
  tI32 bytesToRepeat = ni::StrSize(aToRepeat);
  if (bytesToRepeat == 0) {
    return o;
  }
  o.reserve(o.capacity() + (aN * bytesToRepeat));
  niLoop(i,aN) {
    o.appendEx(aToRepeat,bytesToRepeat);
  }
  return o;
}

// TODO: Move to StringDef.h & StringLib.cpp
niExportFuncCPP(cString) StringRepeat(ain<tI32> aN, ain<tChars> aToRepeat) {
  cString r;
  return StringCatRepeat(r, aN, aToRepeat);
}

struct iLintFuncCall : public iUnknown {
  niDeclareInterfaceUUID(iLintFuncCall,0xfeee9127,0x5c61,0xef11,0x8a,0x5c,0x97,0x69,0xc8,0x3a,0xa9,0xff);

  virtual nn_mut<iHString> __stdcall GetName() const = 0;
  // return -1 for varargs
  virtual tI32 __stdcall GetArity() const = 0;
  virtual SQObjectPtr __stdcall LintCall(sLinter& aLinter, const LintClosure& aClosure, ain<astl::vector<SQObjectPtr>> aCallArgs) = 0;
};

struct sLintTypeofInfo : public ImplRC<iUnknown> {
  niDeclareInterfaceUUID(sLintTypeofInfo,0x26ac90a9,0x8667,0xef11,0x9d,0x40,0x69,0xd2,0xf0,0xcb,0x27,0xfb);

  iUnknown* __stdcall QueryInterface(const tUUID& aIID) {
    if (aIID == niGetInterfaceUUID(sLintTypeofInfo))
      return (iUnknown*)this;
    return BaseImpl::QueryInterface(aIID);
  }

  SQObjectPtr _obj;
  cString _sstr;
  int _iarg1 = -1;
  astl::optional<SQObjectPtr> _wasValue;
};

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
  "_OP_LINT_HINT",
};
niCAssert(niCountOf(_InstrDesc) == __OP_LAST);
const achar* _GetOpDesc(int op) {
  return (op >= 0 && op < __OP_LAST) ? _InstrDesc[op] : "_OP_???";
}

const char* _GetLintHintDesc(eSQLintHint hint) {
  switch (hint) {
    case eSQLintHint_Unknown: return "Unknown";
    case eSQLintHint_SwitchBegin: return "SwitchBegin";
    case eSQLintHint_SwitchEnd: return "SwitchEnd";
    case eSQLintHint_SwitchDefault: return "SwitchDefault";
    case eSQLintHint_IfBegin: return "IfBegin";
    case eSQLintHint_IfEnd: return "IfEnd";
  }
  return "<InvalidLintHint>";
}

cString _GetOpExt(int opExt) {
  int added = 0;
  cString o;
  o << opExt;
  if (added++ > 0) { o << "|"; }
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
    case _RT_TABLE: {
      return niFmt("table:%s", _table(obj)->GetDebugName());
    }
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
    case OT_NULL: {
      return _ObjTypeString(obj);
    }
    case OT_STRING: {
      return niFmt("\"%s\"",_stringval(obj));
    }
    case OT_IUNKNOWN: {
      QPtr<iLintFuncCall> lintFuncCall = _iunknown(obj);
      if (lintFuncCall.IsOK()) {
        return niFmt("%s/%s",
                     lintFuncCall->GetName(),
                     lintFuncCall->GetArity());
      }
      else {
        return niFmt("IUnknown{%s}",_PtrToString((tIntPtr)_iunknown(obj)));
      }
    }
    case OT_FUNCPROTO: {
      return niFmt("funcproto{name:%s}",_ObjToString(_funcproto(obj)->_name));
    }
    case OT_TABLE: {
      return niFmt("TABLE{%s,%s}",
                   _table(obj)->GetDebugName(),
                   _PtrToString((tIntPtr)_table(obj)));
    }
    case OT_ARRAY: {
      return niFmt("ARRAY{size:%s,%s}",
                   _array(obj)->Size(),
                   _PtrToString((tIntPtr)_array(obj)));
    }
    case OT_USERDATA: {
      return niFmt("%s", _userdata(obj)->GetTypeString());
    }
    default:
      return niFmt("%s{%s}",_ObjTypeString(obj),(Var&)obj._var);
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

static SQObjectPtr _VarToObj(const SQSharedState& aSS, const Var& aVar)
{
  switch (niType(aVar.GetType())) {
    case eType_Null: {  return _null_; }
    case eType_I8:  { return (SQInt)aVar.GetI8(); }
    case eType_U8:  { return (SQInt)aVar.GetU8(); }
    case eType_I16: { return (SQInt)aVar.GetI16(); }
    case eType_U16: { return (SQInt)aVar.GetU16(); }
    case eType_I32: { return (SQInt)aVar.GetI32(); }
    case eType_U32: { return (SQInt)aVar.GetU32(); }
    case eType_I64: { return (SQInt)(aVar.GetI64()&0xFFFFFFFF); }
    case eType_U64: { return (SQInt)(aVar.GetU64()&0xFFFFFFFF); }
    case eType_F32: { return (SQFloat)aVar.GetF32(); }
    case eType_F64: { return (SQFloat)aVar.GetF64(); }
    case eType_UUID: {
      return niNew sScriptTypeUUID(aSS, aVar.GetUUID());
    }
    case eType_Vec2f:  {
      return niNew sScriptTypeVec2f(aSS, aVar.GetVec2f());
    }
    case eType_Vec2i:  {
      return niNew sScriptTypeVec2f(aSS, Vec2f(aVar.GetVec2i()));
    }
    case eType_Vec3f:  {
      return niNew sScriptTypeVec3f(aSS, aVar.GetVec3f());
    }
    case eType_Vec3i:  {
      return niNew sScriptTypeVec3f(aSS, Vec3f(aVar.GetVec3i()));
    }
    case eType_Vec4f:  {
      return niNew sScriptTypeVec4f(aSS, aVar.GetVec4f());
    }
    case eType_Vec4i:  {
      return niNew sScriptTypeVec4f(aSS, Vec4f(aVar.GetVec4i()));
    }
    case eType_Matrixf:   {
      return niNew sScriptTypeMatrixf(aSS, aVar.GetMatrixf());
    }
    case eType_IUnknown:  {
      if (!aVar.IsIUnknownPointer()) {
        return niNew sScriptTypeErrorCode(aSS, _HC(error_code), "var2obj: IUnknown variant not a pointer.");
      }
      return aVar.GetIUnknownPointer();
    }
    case eType_AChar:   {
      if (!aVar.IsACharConstPointer()) {
        return niNew sScriptTypeErrorCode(aSS, _HC(error_code), "var2obj: AChar not a constant pointer.");
      }
      return _H(aVar.GetACharConstPointer()?aVar.GetACharConstPointer():cString::ConstSharpNull());
    }
    case eType_String:  {
      return _H(aVar.GetString());
    }
    default: {
      return niNew sScriptTypeErrorCode(aSS, _HC(error_code), niFmt("var2obj: type '%d' is not handled.",niType(aVar.GetType())));
    }
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

static Ptr<sScriptTypeResolvedType> _ObjToResolvedTyped(const SQObjectPtr& obj) {
  if (sqa_getscriptobjtype(obj) == eScriptType_ResolvedType) {
    return (sScriptTypeResolvedType*)_userdata(obj);
  }
  return nullptr;
}

static eScriptType _GetResolvedObjType(const SQObjectPtr& aObj) {
  niLet type = sqa_getscriptobjtype(aObj);
  if (type == eScriptType_ResolvedType) {
    return ((sScriptTypeResolvedType*)_userdata(aObj))->_scriptType;
  }
  return type;
}

// for iInterfaceName naming convention
static tBool _IsInterfaceTypeNameByConvention(ain<tChars> aName) {
  return aName[0] == 'i' && StrIsUpper(aName[1]);
}

static cString _FmtKeyNotFoundMsg(ain<SQObjectPtr> aObj, ain<SQObjectPtr> aKey, ain<SQObjectPtr> aVal)
{
  if (sqa_getscriptobjtype(aVal) == eScriptType_ErrorCode) {
    niLet errorCode = (sScriptTypeErrorCode*)_userdata(aVal);
    return niFmt(
      "%s not found in %s: %s.",
      _ObjToString(aKey), _ObjToString(aObj),
      errorCode->_strErrorDesc);
  }
  else {
    return niFmt(
      "%s not found in %s.",
      _ObjToString(aKey), _ObjToString(aObj));
  }
}

static void _TableSetDebugNameFromSourceName(ain_nn_mut<SQTable> aTable, const achar* aBaseName, iHString* ahspSourceName) {
  if (HStringIsNotEmpty(ahspSourceName)) {
    cPath path(niHStr(ahspSourceName));
    niLet fileName = path.GetFileNoExt();
    if (fileName.EndsWith("_aflymake")) { // TODO: HACK: Not super elegant but really useful for me...
      aTable->SetDebugName(niFmt("__%s_%s__",aBaseName,fileName.RBefore("_aflymake")));
    }
    else {
      aTable->SetDebugName(niFmt("__%s_%s__",aBaseName,fileName));
    }
  }
  else {
    aTable->SetDebugName(niFmt("__%s_%p__",aBaseName,_PtrToString((tIntPtr)aTable.raw_ptr())));
  }
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

struct sLintStackEntry {
  SQObjectPtr _name = _null_;
  SQObjectPtr _value = _null_;
  SQObjectPtr _provenance = _null_;
};

struct LintClosure : public ImplRC<iUnknown>
{
  LintClosure(const SQFunctionProto *func,
              ain_nn_mut<SQTable> rootTable,
              ain_nn_mut<SQTable> thisModuleTable,
              ain_nn_mut<SQTable> thisTable) {
    _func = func;
    _root = rootTable.raw_ptr();
    _thisModule = thisModuleTable.raw_ptr();
    _thisWhenAssigned = thisTable.raw_ptr();
    _stack.resize(_func->_stacksize);
  }

  Ptr<SQFunctionProto> _func;
  SQObjectPtrVec _outervalues;
  // The vmroot table when the closure has been assigned. TODO: This might actually not be necessary?
  SQObjectPtr _root;
  // The this table of the module import.
  SQObjectPtr _thisModule;
  // The this table when the closure was set in a table, this is the this
  // table that is used when linting the closure.
  SQObjectPtr _thisWhenAssigned;
  astl::vector<sLintStackEntry> _stack;
};

typedef astl::hash_map<const SQFunctionProto*,Ptr<LintClosure> > tFuncClosureMap;

#define _LOBJ(KIND) (kLint_##KIND)
#define _LKEY(KIND) (kLint_##KIND.key)
#define _LNAME(KIND) (kLint_##KIND.name.ptr())
#define _LTRACE(MSG) if (shouldLintTrace) { niDebugFmt(MSG); }
#define _LINTERNAL_ERROR(MSG) aLinter.Log(_LKEY(internal_error), _LNAME(internal_error), *thisfunc, thisfunc->GetSourceLineCol(), MSG)
#define _LINTERNAL_WARNING(MSG) aLinter.Log(_LKEY(internal_warning), _LNAME(internal_warning), *thisfunc, thisfunc->GetSourceLineCol(), MSG)
#define _LENABLED(KIND) aLinter.IsEnabled(_LKEY(KIND))
#define _LINT_(KIND,LINECOL,MSG) aLinter.Log(_LKEY(KIND), _LNAME(KIND), *thisfunc, LINECOL, MSG)
#define _LINT(KIND,MSG) _LINT_(KIND, getlinecol(inst), MSG)

static tU32 _lintKeyGen = 0;
#define _DEF_LINT(NAME,CAT1,CAT2)                                       \
  static const sLint kLint_##NAME = { ++_lintKeyGen | eLintFlags_##CAT1 | eLintFlags_##CAT2, _H(#NAME) };

// Internal lints
_DEF_LINT(internal_error,IsError,IsInternal);
_DEF_LINT(internal_warning,IsWarning,IsInternal);

// Default lints
_DEF_LINT(key_notfound_outer,IsError,None);
_DEF_LINT(key_notfound_getk,IsError,None);
_DEF_LINT(key_notfound_get,IsError,None);
_DEF_LINT(key_notfound_callk,IsError,None);
_DEF_LINT(key_cant_set,IsError,None);
_DEF_LINT(key_cant_newslot,IsError,None);
_DEF_LINT(call_error,IsError,None);
_DEF_LINT(call_num_args,IsError,None);
_DEF_LINT(ret_type_cant_assign,IsError,None);
_DEF_LINT(typeof_usage,IsWarning,None);
_DEF_LINT(typeof_invalid,IsError,None);
_DEF_LINT(param_decl,IsError,None);
_DEF_LINT(foreach_usage,IsError,None);

// Pedantic lints
_DEF_LINT(implicit_this_getk,IsWarning,IsPedantic);
_DEF_LINT(implicit_this_get,IsWarning,IsPedantic);
_DEF_LINT(implicit_this_callk,IsWarning,IsPedantic);
_DEF_LINT(prepcall_dynamic,IsWarning,IsPedantic);

// Explicit lints
_DEF_LINT(call_null,IsWarning,IsExplicit);
_DEF_LINT(getk_in_null,IsWarning,IsExplicit);
_DEF_LINT(set_in_null,IsWarning,IsExplicit);
_DEF_LINT(ret_type_is_null,IsWarning,IsExplicit);
// "Null not found in X" is almost always the result of some computation, a
// message for that is generally not helpful.
_DEF_LINT(null_notfound,IsWarning,IsExplicit);

#undef _DEF_LINT

SQObjectPtr DoImportNative(aout<SQSharedState> aSS, ain<SQObjectPtr> aDestTable, ain<SQObjectPtr> aImports, ain<SQObjectPtr> aModuleName) {
  niPanicAssert(sq_istable(aDestTable));
  niPanicAssert(sq_isstring(aModuleName));

  if (!sq_isnull(aImports)) {
    niPanicAssert(sq_istable(aImports));
    niLet importTable = _table(aImports);
    SQObjectPtr foundValue;
    if (importTable->Get(aModuleName, foundValue)) {
      return foundValue;
    }
  }

  niLet hspModuleName = _stringhval(aModuleName);

  // Native module import
  niLet ptrModuleDef = niCheckNNSilent(
    ptrModuleDef,
    ni::GetLang()->LoadModuleDef(niHStr(hspModuleName)),
    niNew sScriptTypeErrorCode(
      aSS, _HC(error_code_import_native),
      niFmt("Can't load module '%s'.",hspModuleName)));
  SQObjectPtr objModuleDef = (iUnknown*)ptrModuleDef.raw_ptr();

  // insert in the map here to make sure we don't re-import unnecessarly
  // if there's any indirect circular dependency
  if (!sq_isnull(aImports)) {
    niLet importTable = _table(aImports);
    importTable->NewSlot(aModuleName,objModuleDef);
  }

  niLoop(i, ptrModuleDef->GetNumEnums()) {
    niLet pEnumDef = ptrModuleDef->GetEnum(i);
    niLet enumName = niFun(&) -> tHStringNN {
      if (ni::StrCmp(pEnumDef->maszName,_A("Unnamed")) == 0) {
        return _HC(e);
      }
      else {
        return _H(pEnumDef->maszName);
      }
    }();
    _table(aDestTable)->NewSlot(enumName, niNew sScriptTypeEnumDef(aSS,pEnumDef));
  }

  niLoop(i,ptrModuleDef->GetNumDependencies()) {
    // check for invalid dependency
    if (ni::StrCmp(ptrModuleDef->GetDependency(i),ptrModuleDef->GetName()) == 0) {
      cString moduleLoadingWarning = niFmt(_A("Module '%s' loading, self-dependency."),ptrModuleDef->GetName());
      niWarning(moduleLoadingWarning);
      continue;
    }

    // import the dependency
    niLet ret = DoImportNative(aSS, aDestTable, aImports, _H(ptrModuleDef->GetDependency(i)));
    if (sqa_getscriptobjtype(ret) == eScriptType_ErrorCode) {
      cString moduleLoadingWarning = niFmt(
        "Module '%s' loading, cant import the dependency '%s': %s.",
        ptrModuleDef->GetName(),ptrModuleDef->GetDependency(i),
        _ObjToString(ret));
      niWarning(moduleLoadingWarning);
    }
  }

  niLoop(i, ptrModuleDef->GetNumConstants()) {
    niLet constDef = ptrModuleDef->GetConstant(i);
    niLet constName = _H(constDef->maszName);
    niLet& constVal = constDef->mvarValue;
    SQObjectPtr value = _VarToObj(aSS, constDef->mvarValue);
    if (sqa_getscriptobjtype(value) == eScriptType_ErrorCode) {
      cString moduleLoadingWarning = niFmt(
        "Module '%s' loading, cant load constant '%s' = '%s'.",
        ptrModuleDef->GetName(),
        constName,
        _ObjToString(value));
      niWarning(moduleLoadingWarning);
    }
    _table(aDestTable)->NewSlot(constName, value);
  }

  return objModuleDef;
}

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

  typedef astl::hash_map<cString,SQObjectPtr> tImportMap;
  typedef tImportMap::iterator tImportMapIt;
  typedef tImportMap::const_iterator tImportMapCIt;
  tImportMap _scriptImports;
  astl::vector<tHStringNN> _scriptImportStack;

  tBool _printLogs = eTrue;
  astl::vector<cString> _logs;
  tU32 _numLintErrors = 0;
  tU32 _numLintWarnings = 0;

  astl::vector<NN_mut<SQTable>> _tables;
  astl::vector<NN_mut<SQArray>> _arrays;

#define _REG_LINT(KIND) astl::upsert(_lintEnabled,_LKEY(KIND),          \
                                     niFlagIsNot(_LKEY(KIND),eLintFlags_IsPedantic) && \
                                     niFlagIsNot(_LKEY(KIND),eLintFlags_IsExperimental) && \
                                     niFlagIsNot(_LKEY(KIND),eLintFlags_IsExplicit))

  sLinter(iHString* ahspSourceName) {
    _vmroot = this->CreateTable().raw_ptr();
    _TableSetDebugNameFromSourceName(as_nn(_table(_vmroot)), "vmroot", ahspSourceName);

    _typedefs = this->CreateTable().raw_ptr();
    _table(_typedefs)->SetDebugName("__typedefs__");

    this->RegisterBuiltinTypesAndFuncs(_table(_vmroot));
    SetDefaultLintEnabled();
  }

  ~sLinter() {
    static tBool _printStats = ni::GetProperty("niScript.LintPrintStats").Bool(eFalse);
    if (_printStats) {
      niLog(Info, niFmt(
        "Linter cleanup stats: vmroot: %s, errors: %d, warnings: %d, tables: %d, arrays: %d, typedefs: %d, nativeimports: %d",
        _ObjToString(_vmroot),
        _numLintErrors, _numLintWarnings,
        _tables.size(),
        _arrays.size(),
        _table(_typedefs)->GetHMap().size()));
    }

    // Clear all the tables & arrays we created
    niLoop(i,_arrays.size()) {
      _arrays[i]->Clear();
    }
    _arrays.clear();
    niLoop(i,_tables.size()) {
      _tables[i]->Clear();
    }
    _tables.clear();
    _table(_vmroot)->Clear();
    _vmroot = _null_;
    _table(_typedefs)->Clear();
    _typedefs = _null_;
  }

  NN_mut<SQTable> CreateTable() {
    return _tables.emplace_back(SQTable::Create());
  }

  NN_mut<SQArray> CreateArray(int nsize) {
    return _arrays.emplace_back(SQArray::Create(nsize));
  }

  void SetDefaultLintEnabled() {
    _REG_LINT(internal_error);
    _REG_LINT(internal_warning);
    _REG_LINT(implicit_this_getk);
    _REG_LINT(implicit_this_get);
    _REG_LINT(implicit_this_callk);
    _REG_LINT(prepcall_dynamic);
    _REG_LINT(key_notfound_getk);
    _REG_LINT(key_notfound_get);
    _REG_LINT(key_notfound_callk);
    _REG_LINT(key_cant_set);
    _REG_LINT(key_cant_newslot);
    _REG_LINT(call_null);
    _REG_LINT(getk_in_null);
    _REG_LINT(set_in_null);
    _REG_LINT(call_error);
    _REG_LINT(call_num_args);
    _REG_LINT(ret_type_is_null);
    _REG_LINT(ret_type_cant_assign);
    _REG_LINT(null_notfound);
    _REG_LINT(typeof_usage);
    _REG_LINT(typeof_invalid);
    _REG_LINT(param_decl);
    _REG_LINT(foreach_usage);
  }
#undef _REG_LINT

  tBool MaybeEnableLintFromSlot(const SQFunctionProto* thisfunc, ain<SQTable> table, ain<SQObjectPtr> k, ain<SQObjectPtr> v) {
    sLinter& aLinter = *this;
    tHStringPtr tableName = table.GetDebugHName();
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
    _E(implicit_this_get)
    _E(implicit_this_callk)
    _E(prepcall_dynamic)
    _E(key_notfound_callk)
    _E(key_notfound_getk)
    _E(key_notfound_get)
    _E(key_cant_set)
    _E(key_cant_newslot)
    _E(call_null)
    _E(getk_in_null)
    _E(set_in_null)
    _E(call_error)
    _E(call_num_args)
    _E(ret_type_is_null)
    _E(ret_type_cant_assign)
    _E(null_notfound)
    _E(typeof_usage)
    _E(typeof_invalid)
    _E(param_decl)
    _E(foreach_usage)
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
    cString& o = _logs.push_back();
    o << "Lint: ";
    if (niFlagIs(aLint,eLintFlags_IsError)) {
      o << "Error: ";
      ++_numLintErrors;
    }
    else if (niFlagIs(aLint,eLintFlags_IsWarning)) {
      o << "Warning: ";
      ++_numLintWarnings;
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

    if (_printLogs) {
      // Raw goes to stderr when eLogFlags_Stdout isnt specified
      ni_log(eLogFlags_Raw, o, niSourceLoc);
    }
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

  void RegisterFunc(SQTable* table, const achar* name, ain<SQObjectPtr> aLintFunc) {
    table->NewSlot(_H(name), aLintFunc);
  }

  void RegisterLintFunc(SQTable* table, ain_nn_mut<iLintFuncCall> aLintFunc) {
    table->NewSlot(aLintFunc->GetName(), aLintFunc.raw_ptr());
  }

  tBool OverrideDelegateFunc(SQTable* del, ain_nn_mut<iLintFuncCall> aLintFunc) {
    return del->Set(aLintFunc->GetName(), aLintFunc.raw_ptr());
  }

  void RegisterBuiltinTypesAndFuncs(SQTable* table);

  SQObjectPtr ResolveTypeUUID(const achar* aTypeName, const tUUID& aTypeUUID) {
    if (aTypeUUID == niGetInterfaceUUID(iHString)) {
      return GetRegisteredTypeDef(_ss._typeStr_string);
    }
    else {
      niLet idef = ni::GetLang()->GetInterfaceDefFromUUID(aTypeUUID);
      if (idef) {
        return niNew sScriptTypeInterfaceDef(this->_ss, idef);
      }
    }

    return niNew sScriptTypeErrorCode(
      this->_ss,
      _HC(error_code_cant_find_type_uuid),
      niFmt("Cant find type uuid '%s' (%s).",aTypeName,aTypeUUID));
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

  int GetLocalIndex(ain<SQFunctionProto> func, iHString* ahspName) {
    niLoopr(i,func._localvarinfos.size()) {
      if (func._localvarinfos[i]._name == ahspName) {
        return func._localvarinfos[i]._pos;
      }
    }
    return -1;
  }

  //
  // Type can be fetched from the root table or relative to the module's this
  // table.
  //
  // Examples:
  //   foo.bar -> __this_module__.foo.bar
  //   tFoo.bar -> __this_module__.tFoo.bar
  //   ::tInRoot.qoo.nix -> __root_module__.tInRoot.qoo.nix
  //   __root_module__
  //   __this_module__
  //   __this__
  //
  SQObjectPtr _ResolveTableTypePath(
    iHString* ahspPath,
    ain<LintClosure> aClosure,
    const SQObjectPtr& aThis)
  {
    niLetMut pathCursor = niHStr(ahspPath);
    // Start at this module
    SQObjectPtr curr = aClosure._thisModule;
    // or start at root?
    if (StrStartsWith(pathCursor, "::")) {
      curr = aClosure._root;
      pathCursor += 2;
    }

    niPanicAssert(sq_istable(curr));
    niLet hasMultipleElements = niFun(&) -> tBool {
      const achar* p = pathCursor;
      while (*p) {
        if (*p == '.')
          return eTrue;
        ++p;
      }
      return eFalse;
    }();

    niLet doGetElement = [&](iHString* aEl) -> SQObjectPtr {
      SQObjectPtr key = aEl;
      SQObjectPtr dest;
      if (!DoLintGet(curr, key, dest, _OPEXT_GET_RAW))
        return _null_;
      return dest;
    };

    if (hasMultipleElements) {
      astl::vector<cString> pathElements;
      pathElements.reserve(8);
      cString toSplit(pathCursor);
      // niDebugFmt(("... ResolveTableTypePath hasMultipleElements: %s", pathCursor));
      StringSplit(toSplit,".",&pathElements);
      niPanicAssert(pathElements.size() >= 2);
      niLoop(i, pathElements.size()) {
        niLet& pathElement = pathElements[i];
        tHStringNN hspEl = _H(pathElement.c_str());
        if (hspEl == _HC(__this__) || hspEl == _HC(__this_module__) || hspEl == _HC(__root_table__)) {
          return niNew sScriptTypeErrorCode(
            this->_ss, _HC(error_code_cant_resolve_table_type_path),
            niFmt("Cant find table type '%s'. Path element[%d] '%s' is a reserved type name that cant be used in a path.",
                  ahspPath, i, hspEl));
        }

        // niDebugFmt(("... ResolveTableTypePath el[%d]: %s", i, pathElement));
        SQObjectPtr found = doGetElement(hspEl);
        if (!sq_istable(found)) {
          return niNew sScriptTypeErrorCode(
            this->_ss, _HC(error_code_cant_resolve_table_type_path),
            niFmt("Cant find table type '%s'. Path element[%d] '%s' not found in %s, got %s.",
                  ahspPath, i, pathElement, _ObjToString(curr), _ObjToString(found), ahspPath));
        }
        curr = found;
      }
    }
    else {
      // niDebugFmt(("... ResolveTableTypePath single elemente: %s", pathCursor));
      tHStringNN hspEl = _H(pathCursor);
      SQObjectPtr found = doGetElement(hspEl);
      if (!sq_istable(found)) {
        return niNew sScriptTypeErrorCode(
          this->_ss, _HC(error_code_cant_resolve_table_type_path),
          niFmt("Cant find table type '%s'. Single path element '%s' not found in %s, got %s.",
                ahspPath, pathCursor, _ObjToString(curr), _ObjToString(found)));
      }
      curr = found;
    }

    _table(curr)->SetDebugName(niFmt("%s", ahspPath));
    return curr;
  }

  SQObjectPtr GetRegisteredTypeDef(const SQObjectPtr& aType)
  {
    if (sq_isnull(aType))
      return _null_;

    SQObjectPtr typeDef;
    if (LintGet(_typedefs,aType,typeDef,0)) {
      return typeDef;
    }

    return niNew sScriptTypeErrorCode(
      this->_ss, _HC(error_code_cant_find_registered_type),
      niFmt("Cant find registered typedef: %s.", _ObjToString(aType)));
  }

  SQObjectPtr ResolveType(const SQObjectPtr& aType, ain<LintClosure> aClosure, const SQObjectPtr& aThis)
  {
    if (sq_isnull(aType))
      return _null_;
    if (!sq_isstring(aType)) {
      return _null_;
    }

    niLet hspType = as_NN(_stringhval(aType));

    // Reserved type names
    if (hspType == _HC(__this__)) {
      // TODO: Maybe this should be a clone of this? Same issue as for
      // ShallowClone/DeepClone we return the this table which might be
      // modified afterward. Again though for the purpose of linting this
      // should be good enough.
      return aThis;
    }
    else if (hspType == _HC(__this_module__)) {
      return aClosure._thisModule;
    }
    else if (hspType == _HC(__root_table__)) {
      return aClosure._root;
    }

    // Already registered types
    {
      SQObjectPtr foundType = _null_;
      if (LintGet(_typedefs,aType,foundType,0)) {
        return foundType;
      }
    }

    // Other types
    niLet typeStrChars = hspType->GetChars();
    SQObjectPtr resolvedType = _null_;
    // Interface types
    if (_IsInterfaceTypeNameByConvention(typeStrChars)) {
      niLetMut foundInterfaceDef = this->FindInterfaceDef(hspType);
      if (!foundInterfaceDef.has_value()) {
        resolvedType = niNew sScriptTypeErrorCode(
          this->_ss, _HC(error_code_cant_find_interface_def),
          niFmt("Cant find interface definition '%s'.", hspType));
      }
      else {
        niLet idef = foundInterfaceDef.value();
        resolvedType = this->ResolveTypeUUID(idef->maszName, *idef->mUUID);
      }
    }
    // Table types
    else {
      resolvedType = _ResolveTableTypePath(hspType, aClosure, aThis);
      if (sqa_getscriptobjtype(resolvedType) == eScriptType_ErrorCode) {
        // we return without caching if there's an error because the type
        // might become available later on or in another context
        return resolvedType;
      }
    }

    niPanicAssert(resolvedType != _null_);
    niPanicAssert(this->LintNewSlot(_typedefs,aType,resolvedType));
    return resolvedType;
  }

  SQObjectPtr ResolveMethodRetType(ain<sInterfaceDef> aInterfaceDef, ain<sMethodDef> aMethodDef) {
    SQObjectPtr retType;
    niLet scriptType = sqa_type2scripttype(aMethodDef.mReturnType);
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
        retType = niNew sScriptTypeResolvedType(_ss, scriptType);
        break;
      }

      case eScriptType_IUnknown: {
        if (aMethodDef.mReturnTypeUUID) {
          retType = ResolveTypeUUID(
            aMethodDef.mReturnTypeName,
            *aMethodDef.mReturnTypeUUID);
        }
        else {
          retType = niNew sScriptTypeErrorCode(
            _ss,
            _HC(error_code_method_def_invalid_ret_type),
            niFmt("Invalid method '%s::%s' return type '%s' (%s), expected a return type UUID.",
                  aInterfaceDef.maszName,
                  aMethodDef.maszName,
                  GetTypeString(aMethodDef.mReturnType),
                  aMethodDef.mReturnTypeName));
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
        retType = niNew sScriptTypeErrorCode(
          _ss,
          _HC(error_code_method_def_invalid_ret_type),
          niFmt("Unsupported method '%s::%s' return type '%s' (%s).",
                aInterfaceDef.maszName,
                aMethodDef.maszName,
                GetTypeString(aMethodDef.mReturnType),
                aMethodDef.mReturnTypeName));
        break;
      };
    }

    return retType;
  }

  bool _VecGet(ain<tU32> anVecSize, ain<SQObjectPtr> aKey, aout<SQObjectPtr> aDest, int opExt)
  {
    niPanicAssert(anVecSize <= 4);
    niLet isSet = niFlagIs(opExt, _OPEXT_LINT_SETVALUE);
    switch (sqa_getscriptobjtype(aKey)) {
      case eScriptType_Int: {
        niLet idx = _int(aKey);
        if (idx < 0 || idx >= anVecSize) {
          aDest = niNew sScriptTypeErrorCode(
            _ss, _HC(error_code_out_of_range),
            niFmt("index out of range '%d'", idx));
          return false;
        }
        aDest = niNew sScriptTypeResolvedType(
          _ss, eScriptType_Float);
        return true;
      }
      case eScriptType_String: {
        niLet hspSwz = as_NN(_stringhval(aKey));
        niLet swzChars = hspSwz->GetChars();
        niLet swzLen = hspSwz->GetLength();

        if (anVecSize == 4) {
          if (ni::StrEq(swzChars,"width") ||
              ni::StrEq(swzChars,"height") ||
              ni::StrEq(swzChars,"left") ||
              ni::StrEq(swzChars,"top") ||
              ni::StrEq(swzChars,"right") ||
              ni::StrEq(swzChars,"bottom"))
          {
            aDest = niNew sScriptTypeResolvedType(
              _ss, eScriptType_Float);
            return true;
          }
          else if (
            ni::StrEq(swzChars,"size") ||
            ni::StrEq(swzChars,"center") ||
            ni::StrEq(swzChars,"top_left") ||
            ni::StrEq(swzChars,"top_right") ||
            ni::StrEq(swzChars,"bottom_left") ||
            ni::StrEq(swzChars,"bottom_right"))
          {
            aDest = niNew sScriptTypeResolvedType(
              _ss, eScriptType_Vec2);
            return true;
          }
        }

        if ((swzLen > anVecSize) ||
            (isSet && swzLen != 1))
        {
          return false;
        }
        else {
          static const tF32 _dummy[4] = {0,1,2,3};
          switch (swzLen) {
            case 1: {
              tF32 r;
              if (sqa_getVecElementFromChar(swzChars[0],_dummy,anVecSize,r)) {
                aDest = niNew sScriptTypeResolvedType(_ss, eScriptType_Float);
                return true;
              }
              break;
            }
            case 2: {
              sVec2f r;
              if (sqa_getVecElementFromChar(swzChars[0],_dummy,anVecSize,r[0])) {
                if (sqa_getVecElementFromChar(swzChars[1],_dummy,anVecSize,r[1])) {
                  aDest = niNew sScriptTypeResolvedType(_ss, eScriptType_Vec2);
                  return true;
                }
              }
              break;
            }
            case 3: {
              sVec3f r;
              if (sqa_getVecElementFromChar(swzChars[0],_dummy,anVecSize,r[0])) {
                if (sqa_getVecElementFromChar(swzChars[1],_dummy,anVecSize,r[1])) {
                  if (sqa_getVecElementFromChar(swzChars[2],_dummy,anVecSize,r[2])) {
                    aDest = niNew sScriptTypeResolvedType(_ss, eScriptType_Vec3);
                    return true;
                  }
                }
              }
              break;
            }
            case 4: {
              sVec4f r;
              if (sqa_getVecElementFromChar(swzChars[0],_dummy,anVecSize,r[0])) {
                if (sqa_getVecElementFromChar(swzChars[1],_dummy,anVecSize,r[1])) {
                  if (sqa_getVecElementFromChar(swzChars[2],_dummy,anVecSize,r[2])) {
                    if (sqa_getVecElementFromChar(swzChars[3],_dummy,anVecSize,r[3])) {
                      aDest = niNew sScriptTypeResolvedType(_ss, eScriptType_Vec4);
                      return true;
                    }
                  }
                }
              }
              break;
            }
          }

        }
        break;
      }
      case eScriptType_ResolvedType: {
        if (((sScriptTypeResolvedType*)_userdata(aKey))->_scriptType == eScriptType_Int) {
          aDest = niNew sScriptTypeResolvedType(
            _ss, eScriptType_Float);
          return true;
        }
        break;
      }
      default: {
        break;
      }
    }
    return false;
  }

  bool _MatrixGet(ain<SQObjectPtr> aKey, aout<SQObjectPtr> aDest)
  {
    switch (sqa_getscriptobjtype(aKey)) {
      case eScriptType_Int: {
        niLet idx = _int(aKey);
        if (idx < 0 || idx >= 16) {
          aDest = niNew sScriptTypeErrorCode(
            _ss, _HC(error_code_out_of_range),
            niFmt("index out of range '%d'", idx));
          return false;
        }
        aDest = niNew sScriptTypeResolvedType(
          _ss, eScriptType_Float);
        return true;
      }
      case eScriptType_String: {
        niLet hspSwz = as_NN(_stringhval(aKey));
        const achar* p = hspSwz->GetChars();
        if (*p != '_') {
          if (ni::StrEq(p,"right") ||
              ni::StrEq(p,"up") ||
              ni::StrEq(p,"forward") ||
              ni::StrEq(p,"translation"))
          {
            aDest = niNew sScriptTypeResolvedType(
              _ss, eScriptType_Vec3);
            return true;
          }
        }
        else {
          int cidx = -1;
          int ridx = -1;
          niUnused(cidx);
          niUnused(ridx);
          ++p;
          switch (*p) {
            case '1': ridx = 0; break;
            case '2': ridx = 1; break;
            case '3': ridx = 2; break;
            case '4': ridx = 3; break;
            default: {
              aDest = niNew sScriptTypeErrorCode(
                _ss, _HC(error_code_out_of_range),
                niFmt("matrix invalid row in member '%s'", hspSwz));
              return false;
            }
          }
          ++p;
          switch (*p) {
            case '1': cidx = 0; break;
            case '2': cidx = 1; break;
            case '3': cidx = 2; break;
            case '4': cidx = 3; break;
            default: {
              aDest = niNew sScriptTypeErrorCode(
                _ss, _HC(error_code_out_of_range),
                niFmt("matrix invalid column in member '%s'", hspSwz));
              return false;
            }
          }
          ++p;
          if (*p != 0) {
            aDest = niNew sScriptTypeErrorCode(
              _ss, _HC(error_code_out_of_range),
              niFmt("matrix invalid swizzle member '%s'", hspSwz));
            return false;
          }
          aDest = niNew sScriptTypeResolvedType(
            _ss, eScriptType_Float);
          return true;
        }
        break;
      }
      case eScriptType_ResolvedType: {
        if (((sScriptTypeResolvedType*)_userdata(aKey))->_scriptType == eScriptType_Int) {
          aDest = niNew sScriptTypeResolvedType(
            _ss, eScriptType_Float);
          return true;
        }
        break;
      }
      default: {
        break;
      }
    }
    return false;
  }

  bool _InterfaceGetInSingleDef(ain_nn<sInterfaceDef> apIDef, const SQObjectPtr &key, SQObjectPtr &dest, int opExt) {
    niLet intfDel = _ss.GetInterfaceDelegate(*apIDef->mUUID);
    if (intfDel == _null_) {
      dest = niNew sScriptTypeErrorCode(
        _ss,
        _HC(error_code_cant_find_method_def),
        niFmt("Cant find delegate for interface_def '%s'.", apIDef->maszName));
      return false;
    }

    if (_table(intfDel)->Get(key,dest)) {
      niLet destType = sqa_getscriptobjtype(dest);
      if ((destType == eScriptType_PropertyDef) && niFlagIsNot(opExt,_OPEXT_LINT_DONT_DEREF_PROPERTY)) {
        // "Dereference" the property
        niLet destUD = (sScriptTypePropertyDef*)_userdata(dest);
        niLet pdefGet = destUD->pGetMethodDef;
        if (pdefGet && pdefGet->mnNumParameters == 0) {
          dest = ResolveMethodRetType(
            *destUD->pInterfaceDef,
            *pdefGet);
        }
      }
      return true;
    }

    return false;
  }

  bool _InterfaceGetInParents(ain_nn<sInterfaceDef> apIDef, const SQObjectPtr &key, SQObjectPtr &dest, int opExt) {
    niLoop(i, apIDef->mnNumBases) {
      niLet uuid = apIDef->mpBases[i];
      niPanicAssert(uuid != nullptr);
      if (*uuid == niGetInterfaceUUID(iUnknown)) {
        // we skip iUnknown here since its the base of everything
        return false;
      }

      niLet baseIDef = ni::GetLang()->GetInterfaceDefFromUUID(*uuid);
      if (!baseIDef) {
        dest = niNew sScriptTypeErrorCode(
          _ss,
          _HC(error_code_cant_find_method_def),
          niFmt("Cant find interface def %s for base %d of %s.", *uuid, i, apIDef->maszName));
        return false;
      }

      // look in the current base
      if (_InterfaceGetInSingleDef(as_nn(baseIDef), key, dest, opExt)) {
        return true;
      }
      if (sqa_getscriptobjtype(dest) == eScriptType_ErrorCode) {
        return false;
      }

      // go in the bases of the base...
      if (_InterfaceGetInParents(as_nn(baseIDef), key, dest, opExt)) {
        return true;
      }
      if (sqa_getscriptobjtype(dest) == eScriptType_ErrorCode) {
        return false;
      }
    }

    return false;
  }

  bool _InterfaceDefGet(ain_nn<sInterfaceDef> apIDef, const SQObjectPtr &key, SQObjectPtr &dest, int opExt) {
    if (_stringhval(key) == _HC(QueryInterface)) {
      dest = _lintFuncCallQueryInterface;
      return true;
    }
    dest = _null_;

    // look in the interface
    if (_InterfaceGetInSingleDef(apIDef, key, dest, opExt)) {
      return true;
    }
    if (sqa_getscriptobjtype(dest) == eScriptType_ErrorCode) {
      return false;
    }

    // go in the base interfaces...
    if (_InterfaceGetInParents(apIDef, key, dest, opExt)) {
      return true;
    }
    if (sqa_getscriptobjtype(dest) == eScriptType_ErrorCode) {
      return false;
    }

    // look in iUnknown
    return _ScriptTypeDelegateGet(eScriptType_IUnknown, key, dest, opExt);
  }

  bool _ScriptTypeDelegateGet(eScriptType aScriptType, const SQObjectPtr &key, SQObjectPtr &dest, int opExt) {
    const SQSharedState& ss = this->_ss;
    switch (aScriptType) {
      case eScriptType_String:
        return _ddel(ss,string)->Get(key,dest);
      case eScriptType_Int:
      case eScriptType_Float:
        return _ddel(ss,number)->Get(key,dest);
      case eScriptType_Vec2: {
        if (_ddel(ss,vec2f)->Get(key,dest)) {
          return true;
        }
        return _VecGet(2,key,dest,opExt);
      }
      case eScriptType_Vec3: {
        if (_ddel(ss,vec3f)->Get(key,dest)) {
          return true;
        }
        return _VecGet(3,key,dest,opExt);
      }
      case eScriptType_Vec4: {
        if (_ddel(ss,vec4f)->Get(key,dest)) {
          return true;
        }
        return _VecGet(4,key,dest,opExt);
      }
      case eScriptType_Matrix: {
        if (_ddel(ss,matrixf)->Get(key,dest)) {
          return true;
        }
        return _MatrixGet(key,dest);
      }
      case eScriptType_Array:
        return _ddel(ss,array)->Get(key,dest);
      case eScriptType_Table:
        return _ddel(ss,table)->Get(key,dest);
      case eScriptType_Closure:
      case eScriptType_NativeClosure:
      case eScriptType_FunctionProto:
        return _ddel(ss,closure)->Get(key,dest);
      case eScriptType_UUID:
        return _ddel(ss,uuid)->Get(key,dest);
      case eScriptType_IUnknown: {
        if (_stringhval(key) == _HC(QueryInterface)) {
          dest = _lintFuncCallQueryInterface;
          return true;
        }
        else {
          niLet intfDel = _ss.GetInterfaceDelegate(niGetInterfaceUUID(iUnknown));
          if (intfDel != _null_) {
            niPanicAssert(sq_istable(intfDel));
            return _table(intfDel)->Get(key,dest);
          }
        }
        return false;
      }
      case eScriptType_EnumDef:
        return _ddel(ss,enum)->Get(key,dest);
      default:
        return false;
    }
  }

  bool DoLintGet(const SQObjectPtr &self, const SQObjectPtr &key, SQObjectPtr &dest, int opExt)
  {
    const SQSharedState& ss = this->_ss;
    switch(_sqtype(self)){
      case OT_TABLE:
        {
          if (_table(self)->Get(key,dest))
            return true;
          // delegation
          if (_table(self)->GetDelegate()) {
            return DoLintGet(SQObjectPtr(_table(self)->GetDelegate()),key,dest,opExt);
          }
          if (opExt & _OPEXT_GET_RAW) {
            return false;
          }
          // niDebugFmt(("... _table_ddel: %s", _TableToString(_table_ddel,"")));
          return _ddel(ss,table)->Get(key,dest);
        }
      case OT_IUNKNOWN:
        // TODO: This should not happen, it should be a sInterfaceDef. Should we niPanicAssert here?
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
              niLet selfIDef = as_nn(((sScriptTypeInterfaceDef*)ud)->pInterfaceDef);
              niLet r = _InterfaceDefGet(selfIDef,key,dest,opExt);
              if (r || sqa_getscriptobjtype(dest) == eScriptType_ErrorCode)
                return r;
              dest = niNew sScriptTypeErrorCode(
                _ss,
                _HC(error_code_cant_find_method_def),
                niFmt("Cant find method definition '%s::%s'",
                      selfIDef->maszName, _stringhval(key)));
              return false;
            }
            case eScriptType_EnumDef: {
              SQObjectPtr enumTable = ((sScriptTypeEnumDef*)ud)->_GetTable(const_cast<SQSharedState&>(ss));
              niLet r = DoLintGet(enumTable, key, dest, opExt);
              if (r || sqa_getscriptobjtype(dest) == eScriptType_ErrorCode)
                return r;
              break;
            }
            case eScriptType_PropertyDef: {
              niLet pdefGet = ((sScriptTypePropertyDef*)ud)->pGetMethodDef;
              if (pdefGet) {
                dest = ResolveMethodRetType(
                  *((sScriptTypePropertyDef*)ud)->pInterfaceDef,
                  *pdefGet);
                if (!sq_isnull(dest)) {
                  return true;
                }
              }
              return false;
            }
            case eScriptType_ResolvedType: {
              return _ScriptTypeDelegateGet(((sScriptTypeResolvedType*)ud)->_scriptType, key, dest, opExt);
            }
          }

          bool getRetVal = false;
          if (ud->GetDelegate()) {
            getRetVal = DoLintGet(
              SQObjectPtr(ud->GetDelegate()),
              key,dest,opExt|_OPEXT_GET_RAW);
            if (!getRetVal) {
              if (opExt & _OPEXT_GET_RAW)
                return false;
              // TODO: Lint call _get metamethod? I don't think its necessary
              // for practical linting as it's very uncommon.
              return false;
            }
          }
          return getRetVal;
        }
        break;

      case OT_INTEGER: case OT_FLOAT:
        return _ddel(ss,number)->Get(key,dest);
      case OT_CLOSURE: case OT_NATIVECLOSURE: case OT_FUNCPROTO:
        return _ddel(ss,closure)->Get(key,dest);
      default:
        return false;
    }
  }

  bool LintGet(const SQObjectPtr &self, const SQObjectPtr &key, SQObjectPtr &dest, int opExt)
  {
    bool r = DoLintGet(self,key,dest,opExt);
    if (!r) {
      if (opExt & _OPEXT_GET_SAFE) {
        return true;
      }
      return false;
    }
    return true;
  }

  bool DoLintSet(cString& errDesc, const SQObjectPtr &self,const SQObjectPtr &key,const SQObjectPtr &val, int opExt)
  {
    // niDebugFmt(("... DoLintSet: '%s' in %s", _ObjToString(key), _ObjToString(self)));
    switch(_sqtype(self)) {
      case OT_TABLE: {
        // Disabled with #if 0 because the linter, which doesn't fully execute
        // the code, may legitimately pass the same table without creating new
        // tables or clones when unnecessary for linting purposes. As a
        // result, the table might end up being set in itself here
        // directly. In the runtime version, however, it would usually operate
        // on a clone of the table.
#if 0
        if (sq_istable(val)) {
          if (_table(self) == _table(val)) {
            errDesc.Format(niFmt("setting table '%s' as the value for key '%s' in itself",
                                 _ObjToString(self), _ObjToString(key)));
            return false;
          }
        }
#endif

        if (!_table(self)->Set(key,val)) {
          if (_table(self)->GetDelegate()) {
            SQObjectPtr v;
            if (!DoLintGet(self,key,v,_OPEXT_LINT_SETVALUE)) {
              return false;
            }
            return _table(self)->NewSlot(key,val);
          }
          else {
            return false;
          }
        }
        else {
          return true;
        }
      }
      case OT_ARRAY:
        if (!sq_isnumeric(key)) {
          return false;
        }
        return _array(self)->Set(toint(key),val);
      case OT_IUNKNOWN:
        // TODO: This should not happen, it should be a sInterfaceDef. Should we niPanicAssert here?
        return false;
      default: {
        switch (sqa_getscriptobjtype(self)) {
          case eScriptType_PropertyDef: {
            niLet selfUD = (sScriptTypePropertyDef*)_userdata(self);
            niLet pdefSet = selfUD->pSetMethodDef;
            if (!pdefSet) {
              errDesc.Format("no setter for indexed property");
              return false;
            }
            else if (pdefSet->mnNumParameters == 2) {
              return true;
            }
            else {
              errDesc.Format(niFmt("invalid number of parameters '%d' for indexed property", pdefSet->mnNumParameters));
              return false;
            }
            break;
          }
          default: {
            // niDebugFmt(("... DoLintSet: Try get %s[%s].", _ObjToString(self), _ObjToString(key)));
            SQObjectPtr dest;
            if (!DoLintGet(self,key,dest,_OPEXT_LINT_SETVALUE|_OPEXT_LINT_DONT_DEREF_PROPERTY)) {
              return false;
            }

            // niDebugFmt(("... DoLintSet: didGet: %s[%s] = %s.", _ObjTypeString(self), _ObjToString(key), _ObjToString(dest)));

            if (sqa_getscriptobjtype(dest) == eScriptType_PropertyDef) {
              niLet destUD = (sScriptTypePropertyDef*)_userdata(dest);
              niLet pdefSet = destUD->pSetMethodDef;
              if (!pdefSet) {
                errDesc.Format("no setter");
                return false;
              }
              else if (pdefSet->mnNumParameters != 1) {
                errDesc.Format(niFmt("invalid number of parameters '%d' for property", pdefSet->mnNumParameters));
                return false;
              }
              else {
                return true;
              }
            }
            else {
              return true;
            }

            break;
          }
        }
        return false;
      }
    }
  }

  bool LintSet(cString& errDesc, const SQObjectPtr &self, const SQObjectPtr &key, const SQObjectPtr &val, int opExt) {
    bool r = DoLintSet(errDesc,self,key,val,opExt);
    if (!r && !(opExt & _OPEXT_GET_SAFE)) {
      return false;
    }
    return true;
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

  static bool LintTypeObjCanAssign(const SQObjectPtr& aFromTypeObj, const SQObjectPtr& aToTypeObj) {
    niLetMut fromType = _GetResolvedObjType(aFromTypeObj);
    niLet toType = _GetResolvedObjType(aToTypeObj);
    if (_LintTypeCanAssign(fromType, toType)) {
      if (fromType == eScriptType_InterfaceDef && toType == eScriptType_InterfaceDef) {
        niLet fromIntf = ((sScriptTypeInterfaceDef*)_userdata(aFromTypeObj))->pInterfaceDef;
        niLet toIntf = ((sScriptTypeInterfaceDef*)_userdata(aToTypeObj))->pInterfaceDef;
        if (*fromIntf->mUUID == *toIntf->mUUID)
          return eTrue;
        if (*toIntf->mUUID == niGetInterfaceUUID(iUnknown))
          return eTrue;
        niLoop(i,fromIntf->mnNumBases) {
          if (*fromIntf->mpBases[i] == *toIntf->mUUID)
            return eTrue;
        }
        return eFalse;
      }
      else {
        return eTrue;
      }
    }
    return eFalse;
  }

  // TODO: Dedupe with implementation in cScriptVM::ImportFileOpen
  Opt_mut<iFile> __stdcall ImportFileOpen(const achar* aaszFile) {
    return ni::GetLang()->URLOpen(
      (StrFindProtocol(aaszFile) < 0)
      ? (niFmt("script://%s",aaszFile)) // use default 'script' protocol
      : (aaszFile) // use the protocol specified in the URL
    );
  }

  SQObjectPtr ImportScript(ain<LintClosure> aClosure, ain<tChars> aModuleName) {
    static tBool _traceImportScript = ni::GetProperty("niScript.LintTraceImportScript").Bool(eFalse);
    static tBool _traceImportScriptAlreadyImported = ni::GetProperty("niScript.LintTraceImportScript").Bool(eFalse);
    static tBool _lintPrintImportLint = ni::GetProperty("niScript.LintPrintImportLint").Bool(eTrue);

    SQObjectPtr roottable = aClosure._root;
    niPanicAssert(sq_istable(roottable));

    // Script code import
    niLetMut fp = niCheckNNSilent(
      fp,
      ImportFileOpen(aModuleName),
      niNew sScriptTypeErrorCode(
        _ss, _HC(error_code_lint_call_error),
        niFmt("ImportScript: Cant open script module file '%s'.",aModuleName)));

    tHStringNN hspSourceName = _H(fp->GetSourcePath());
    if (astl::contains(_scriptImportStack, hspSourceName)) {
      if (_traceImportScript) {
        niDebugFmt(("%sImportScript: '%s'", StringRepeat(_scriptImportStack.size(), "  "), aModuleName));
      }
      return niNew sScriptTypeErrorCode(
        _ss, _HC(error_code_lint_call_error),
        niFmt("ImportScript: Import cycle while importing '%s'. Stack: %s.",
              aModuleName,StringJoin(_scriptImportStack," > ")));
    }

    tImportMapIt it = _scriptImports.find(aModuleName);
    if (it != _scriptImports.end()) {
      if (_traceImportScriptAlreadyImported) {
        niDebugFmt(("%sImportScript: '%s', already imported.", StringRepeat(_scriptImportStack.size(), "  "), aModuleName));
      }
      return it->second;
    }
    else {
      if (_traceImportScript) {
        niDebugFmt(("%sImportScript: '%s'", StringRepeat(_scriptImportStack.size(), "  "), aModuleName));
      }
    }

    cString sourceCode = fp->ReadString();
    SQObjectPtr o;
    sCompileErrors errors;
    if (!CompileString(hspSourceName,sourceCode.Chars(),errors,o)) {
      o = niNew sScriptTypeErrorCode(
        _ss, _HC(error_code_lint_call_error),
        niFmt("ImportScript: Linting compiler error: %s:%d:%d: %s",
              aModuleName,
              errors.GetLastError().line,
              errors.GetLastError().col,
              errors.GetLastError().msg));
      astl::upsert(_scriptImports, aModuleName, o);
      return o;
    }

    niPanicAssert(sq_isfuncproto(o));

    _scriptImportStack.push_back(hspSourceName);
    SQObjectPtr moduleThis = this->CreateTable().raw_ptr();
    astl::upsert(_scriptImports, aModuleName, moduleThis);
    _table(moduleThis)->SetDebugName(niFmt("__modulethis_%s__",hspSourceName));
    // TODO: Skipping the logs of the imported scripts for now, a bit jank
    // way to do it but it'll do for now.  Should this be its own linter but
    // sharing the maps?
    {
      niLet wasPrintLogs = _printLogs;
      niLet wasLintEnabled = _lintEnabled;
      SetDefaultLintEnabled();
      _printLogs = _lintPrintImportLint;
      _funcproto(o)->LintTrace(*this,_table(roottable),_table(moduleThis),_table(moduleThis));
      _lintEnabled = wasLintEnabled;
      _printLogs = wasPrintLogs;
    }
    _scriptImportStack.pop_back();

    return moduleThis;
  }

  SQObjectPtr ImportNative(ain<LintClosure> aClosure, ain<tChars> aModuleName) {
    SQObjectPtr roottable = aClosure._root;
    niPanicAssert(sq_istable(roottable));
    niLet ret = DoImportNative(_ss, roottable, _ss._nativeimports_table, _H(aModuleName));
    if (sqa_getscriptobjtype(ret) != eScriptType_IUnknown) {
      return ret;
    }

    return niNew sScriptTypeInterfaceDef(_ss, ni::GetLang()->GetInterfaceDefFromUUID(niGetInterfaceUUID(ni::iModuleDef)));
  }

  SQObjectPtr Import(ain<LintClosure> aClosure, ain<tChars> aModuleName) {
    niLet isScriptFile =
        StrEndsWithI(aModuleName,".ni") ||
        StrEndsWithI(aModuleName,".nim") ||
        StrEndsWithI(aModuleName,".nio") ||
        StrEndsWithI(aModuleName,".nip") ||
        StrEndsWithI(aModuleName,".niw");
    if (isScriptFile) {
      return ImportScript(aClosure, aModuleName);
    }
    else {
      return ImportNative(aClosure, aModuleName);
    }
  }
};

static SQObjectPtr _MakeLintCallError(ain<sLinter> aLinter, const achar* aMsg) {
  return niNew sScriptTypeErrorCode(aLinter._ss, _HC(error_code_lint_call_error), aMsg);
}

struct sLintFuncCallCreateInstance : public ImplRC<iLintFuncCall> {
  NN_mut<iHString> _name;

  sLintFuncCallCreateInstance(iHString* aName)
      : _name(aName)
  {}

  virtual nn_mut<iHString> __stdcall GetName() const {
    return _name;
  }

  virtual tI32 __stdcall GetArity() const {
    return -1;
  }

  virtual SQObjectPtr __stdcall LintCall(sLinter& aLinter, const LintClosure& aClosure, ain<astl::vector<SQObjectPtr>> aCallArgs)
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

struct sLintFuncCallImport : public ImplRC<iLintFuncCall> {
  NN_mut<iHString> _name;

  sLintFuncCallImport(iHString* aName)
      : _name(aName)
  {}

  virtual nn_mut<iHString> __stdcall GetName() const {
    return _name;
  }

  virtual tI32 __stdcall GetArity() const {
    return 1;
  }

  virtual SQObjectPtr __stdcall LintCall(sLinter& aLinter, const LintClosure& aClosure, ain<astl::vector<SQObjectPtr>> aCallArgs)
  {
    niLet objModuleName = aCallArgs[1];
    // niDebugFmt(("... LintCall: objModuleName: %s", _ObjToString(objModuleName)));

    if (sqa_getscriptobjtype(objModuleName) != eScriptType_String) {
      if (_GetResolvedObjType(objModuleName) == eScriptType_String) {
        return _null_;
      }
      else {
        return _MakeLintCallError(
          aLinter,niFmt("First parameter should be the name of the module to load as a literal string but got '%s'.", _ObjToString(objModuleName)));
      }
    }

    return aLinter.Import(aClosure, _stringval(objModuleName));
  }
};

struct sLintFuncCallGetLangDelegate : public ImplRC<iLintFuncCall> {
  NN_mut<iHString> _name;

  sLintFuncCallGetLangDelegate(iHString* aName)
      : _name(aName)
  {}

  virtual nn_mut<iHString> __stdcall GetName() const {
    return _name;
  }

  virtual tI32 __stdcall GetArity() const {
    return 1;
  }

  virtual SQObjectPtr __stdcall LintCall(sLinter& aLinter, const LintClosure& aClosure, ain<astl::vector<SQObjectPtr>> aCallArgs)
  {
    niLet objDelegateName = aCallArgs[1];
    // niDebugFmt(("... LintCall: objDelegateName: %s", _ObjToString(objDelegateName)));
    if (sqa_getscriptobjtype(objDelegateName) != eScriptType_String) {
      return _MakeLintCallError(
        aLinter,niFmt("First parameter should be the name of the builtin delegate as a literal string but got '%s'.", _ObjToString(objDelegateName)));
    }

    SQObjectPtr objDelegate = aLinter._ss.GetLangDelegate(_stringval(objDelegateName));
    if (objDelegate == _null_) {
      return _MakeLintCallError(
        aLinter,niFmt("Cant get the builtin delegate '%s'.", _ObjToString(objDelegateName)));
    }

    return objDelegate;
  }
};

struct sLintFuncCall_lint_check_type : public ImplRC<iLintFuncCall> {
  NN_mut<iHString> _name;

  sLintFuncCall_lint_check_type(iHString* aName)
      : _name(aName)
  {}

  virtual nn_mut<iHString> __stdcall GetName() const {
    return _name;
  }

  virtual tI32 __stdcall GetArity() const {
    return 2;
  }

  virtual SQObjectPtr __stdcall LintCall(sLinter& aLinter, const LintClosure& aClosure, ain<astl::vector<SQObjectPtr>> aCallArgs)
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
        niFmt("Expected type '%s' but got '%s' for %s.",
              expectedType, actualType, _ObjToString(actualObject)));
    }

    return _one_;
  }
};

static int lint_lint_check_type(HSQUIRRELVM v)
{
  // SQObjectPtr& expectedType = stack_get(v,2);
  // niUnused(expectedType);
  // SQObjectPtr& actualObj = stack_get(v,3);
  // niUnused(actualObj);
  v->Push(_one_);
  return 1;
}

struct sLintFuncCall_lint_as_type : public ImplRC<iLintFuncCall> {
  NN_mut<iHString> _name;

  sLintFuncCall_lint_as_type(iHString* aName)
      : _name(aName)
  {}

  virtual nn_mut<iHString> __stdcall GetName() const {
    return _name;
  }

  virtual tI32 __stdcall GetArity() const {
    return 2;
  }

  virtual SQObjectPtr __stdcall LintCall(sLinter& aLinter, const LintClosure& aClosure, ain<astl::vector<SQObjectPtr>> aCallArgs)
  {
    niLet& expectedTypeArg = aCallArgs[1];
    niLet& actualObject = aCallArgs[2];

    if (sqa_getscriptobjtype(expectedTypeArg) != eScriptType_String) {
      return _MakeLintCallError(
        aLinter,niFmt("First parameter should be the expected type name as a literal string but got '%s'.", _ObjToString(expectedTypeArg)));
    }

    return aLinter.ResolveType(expectedTypeArg, aClosure, aCallArgs[0]);
  }
};

static int lint_lint_as_type(HSQUIRRELVM v)
{
  //SQObjectPtr& type = stack_get(v,2);
  SQObjectPtr& obj = stack_get(v,3);
  v->Push(obj);
  return 1;
}

struct sLintFuncCall_root_QueryInterface : public ImplRC<iLintFuncCall> {
  sLintFuncCall_root_QueryInterface()
  {}

  virtual nn_mut<iHString> __stdcall GetName() const {
    return _HC(QueryInterface);
  }

  virtual tI32 __stdcall GetArity() const {
    return 2;
  }

  static SQObjectPtr __stdcall _LintCallQueryInterface(sLinter& aLinter, ain<SQObjectPtr> qiID) {
    tHStringPtr qiName;
    tUUID qiUUID = kuuidZero;
    switch(sqa_getscriptobjtype(qiID)) {
      case eScriptType_String: {
        qiName = _stringhval(qiID);
        qiUUID = ni::GetLang()->GetInterfaceUUID(qiName);
        break;
      }
      default: {
        switch (_GetResolvedObjType(qiID)) {
          case eScriptType_Null:
          case eScriptType_String:
          case eScriptType_UUID: {
            // we cant determine the interface type from a non-literal value so we return iUnknown
            qiUUID = niGetInterfaceUUID(iUnknown);
            break;
          }
          default: {
            return _MakeLintCallError(
              aLinter,niFmt("Invalid interface id type '%s'.", _ObjToString(qiID)));
          }
        }
      }
    }

    // niDebugFmt(("... sLintFuncCall_this_QueryInterface: qiUUID: %s", qiUUID));
    if (qiUUID == kuuidZero) {
      return _MakeLintCallError(
        aLinter,niFmt("Cant find interface uuid '%s'.", _ObjToString(qiID)));
    }

    niLet qiDef = ni::GetLang()->GetInterfaceDefFromUUID(qiUUID);
    if (qiDef == nullptr) {
      return _MakeLintCallError(
        aLinter,niFmt("Cant find interface def '%s'.", _ObjToString(qiID)));
    }

    // niDebugFmt(("... sLintFuncCall_this_QueryInterface: qiDef: %s", qiDef->maszName));
    return aLinter.ResolveTypeUUID(niHStr(qiName), qiUUID);
  }

  virtual SQObjectPtr __stdcall LintCall(sLinter& aLinter, const LintClosure& aClosure, ain<astl::vector<SQObjectPtr>> aCallArgs) {
    return _LintCallQueryInterface(aLinter, aCallArgs[2]);
  }
};

struct sLintFuncCall_this_QueryInterface : public ImplRC<iLintFuncCall> {
  sLintFuncCall_this_QueryInterface()
  {}

  virtual nn_mut<iHString> __stdcall GetName() const {
    return _HC(QueryInterface);
  }

  virtual tI32 __stdcall GetArity() const {
    return 1;
  }

  virtual SQObjectPtr __stdcall LintCall(sLinter& aLinter, const LintClosure& aClosure, ain<astl::vector<SQObjectPtr>> aCallArgs) {
    return sLintFuncCall_root_QueryInterface::_LintCallQueryInterface(aLinter, aCallArgs[1]);
  }
};

struct sLintFuncCall_table_or_array_clone : public ImplRC<iLintFuncCall> {
  NN_mut<iHString> _name;

  sLintFuncCall_table_or_array_clone(iHString* aName)
      : _name(aName)
  {}

  virtual nn_mut<iHString> __stdcall GetName() const {
    return _name;
  }

  virtual tI32 __stdcall GetArity() const {
    return 0;
  }

  virtual SQObjectPtr __stdcall LintCall(sLinter& aLinter, const LintClosure& aClosure, ain<astl::vector<SQObjectPtr>> aCallArgs)
  {
    niLet objThis = aCallArgs[0];
    // niDebugFmt(("... sLintFuncCall_table_clone: objThis: %s", _ObjToString(objThis)));
    niLet objThisType = sqa_getscriptobjtype(objThis);
    if (objThisType == eScriptType_Table) {
      // TODO: Do deep clone? Technically its more correct, if we deep clone the
      // sub tables that get modified might get incorrectly linted. So for 100%
      // correctness it'd be better, however its somewhat of an edge case so
      // we'll leave it for now.
      NN_mut<SQTable> clonedTable { _table(objThis)->Clone(nullptr) };
      return clonedTable.raw_ptr();
    }
    else if (objThisType == eScriptType_Array) {
      // TODO: Do deep clone? Technically its more correct, if we deep clone the
      // sub tables that get modified might get incorrectly linted. So for 100%
      // correctness it'd be better, however its somewhat of an edge case so
      // we'll leave it for now.
      NN_mut<SQArray> clonedArray { _array(objThis)->Clone(nullptr) };
      return clonedArray.raw_ptr();
    }
    else if (objThisType == eScriptType_ResolvedType) {
      niLet resolvedType = _ObjToResolvedTyped(objThis);
      if (resolvedType->_scriptType == eScriptType_Table ||
          resolvedType->_scriptType == eScriptType_Array) {
        return objThis;
      }
    }

    return _MakeLintCallError(
      aLinter,niFmt("This should be a table but got '%s'.", _ObjToString(objThis)));
  }
};

struct sLintFuncCall_table_setdelegate : public ImplRC<iLintFuncCall> {
  NN_mut<iHString> _name;

  sLintFuncCall_table_setdelegate(iHString* aName)
      : _name(aName)
  {}

  virtual nn_mut<iHString> __stdcall GetName() const {
    return _name;
  }

  virtual tI32 __stdcall GetArity() const {
    return 1;
  }

  virtual SQObjectPtr __stdcall LintCall(sLinter& aLinter, const LintClosure& aClosure, ain<astl::vector<SQObjectPtr>> aCallArgs)
  {
    niLet& objTable = aCallArgs[0];
    niLet& delTable = aCallArgs[1];
    // niDebugFmt(("... sLintFuncCall_table_setdelegate: objTable: %s, delTable", _ObjToString(objTable), _ObjToString(delTable)));

    niLet resolvedDelType = _GetResolvedObjType(delTable);
    if (resolvedDelType != eScriptType_Null &&
        resolvedDelType != eScriptType_Table)
    {
      return _MakeLintCallError(
        aLinter,niFmt("Delegate expected a table but got '%s'.", _ObjToString(delTable)));
    }

    niLet objTableType = sqa_getscriptobjtype(objTable);
    if (objTableType == eScriptType_Table) {
      if (sq_istable(delTable)) {
        _table(objTable)->SetDelegate(_table(delTable));
      }
      return objTable;
    }
    else if (objTableType == eScriptType_ResolvedType) {
      niLet resolvedType = _ObjToResolvedTyped(objTable);
      if (resolvedType->_scriptType == eScriptType_Table) {
        return objTable;
      }
    }

    return _MakeLintCallError(
      aLinter,niFmt("This should be a table but got '%s'.", _ObjToString(objTable)));
  }
};

struct sLintFuncCall_table_getdelegate : public ImplRC<iLintFuncCall> {
  NN_mut<iHString> _name;

  sLintFuncCall_table_getdelegate(iHString* aName)
      : _name(aName)
  {}

  virtual nn_mut<iHString> __stdcall GetName() const {
    return _name;
  }

  virtual tI32 __stdcall GetArity() const {
    return 0;
  }

  virtual SQObjectPtr __stdcall LintCall(sLinter& aLinter, const LintClosure& aClosure, ain<astl::vector<SQObjectPtr>> aCallArgs)
  {
    niLet& objTable = aCallArgs[0];

    niLet objTableType = sqa_getscriptobjtype(objTable);
    if (objTableType == eScriptType_Table) {
      SQTable* del = _table(objTable)->GetDelegate();
      if (del) {
        return del;
      }
      else {
        return _null_;
      }
    }
    else if (objTableType == eScriptType_ResolvedType) {
      niLet resolvedType = _ObjToResolvedTyped(objTable);
      if (resolvedType->_scriptType == eScriptType_Table) {
        return niNew sScriptTypeResolvedType(aLinter._ss, eScriptType_Table);
      }
    }

    return _MakeLintCallError(
      aLinter,niFmt("This should be a table but got '%s'.", _ObjToString(objTable)));
  }
};

// Registered in sqvm.cpp
SQRegFunction SQSharedState::_lint_funcs[] = {
  {"LINT_AS_TYPE", lint_lint_as_type, 3, "ts."},
  {"LINT_CHECK_TYPE", lint_lint_check_type, 3, "ts.", _HC(typestr_bool)},
  {0,0}
};

void sLinter::RegisterBuiltinTypesAndFuncs(SQTable* table) {

#define NEW_BASE_RESOLVED_TYPE(SCRIPTTYPE,DEL)              \
  SQObjectPtr resolvedType = niNew sScriptTypeResolvedType( \
    this->_ss, SCRIPTTYPE);                                 \
  _userdata(resolvedType)->SetDelegate(_ddel(_ss,DEL));

#define REGISTER_BASE_TYPE(NAME)                        \
  this->LintNewSlot(_typedefs,_HC(NAME),resolvedType);

  // Builtin types
  {
    NEW_BASE_RESOLVED_TYPE(eScriptType_String, string);
    REGISTER_BASE_TYPE(typestr_string);
  }
  {
    NEW_BASE_RESOLVED_TYPE(eScriptType_Int, number);
    REGISTER_BASE_TYPE(typestr_int);
    REGISTER_BASE_TYPE(typestr_bool);
  }
  {
    NEW_BASE_RESOLVED_TYPE(eScriptType_Float, number);
    REGISTER_BASE_TYPE(typestr_float);
  }
  {
    NEW_BASE_RESOLVED_TYPE(eScriptType_Array, array);
    REGISTER_BASE_TYPE(typestr_array);
  }
  {
    NEW_BASE_RESOLVED_TYPE(eScriptType_Table, table);
    REGISTER_BASE_TYPE(typestr_table);
  }
  {
    NEW_BASE_RESOLVED_TYPE(eScriptType_Closure, closure);
    REGISTER_BASE_TYPE(typestr_closure);
    REGISTER_BASE_TYPE(typestr_function);
  }
  {
    SQObjectPtr resolvedType = niNew sScriptTypeResolvedType(this->_ss, eScriptType_Null);
    REGISTER_BASE_TYPE(typestr_null);
    REGISTER_BASE_TYPE(typestr_void);
  }
  {
    SQObjectPtr resolvedType = niNew sScriptTypeResolvedType(this->_ss, eScriptType_IUnknown);
    REGISTER_BASE_TYPE(typestr_iunknown);
  }
  {
    NEW_BASE_RESOLVED_TYPE(eScriptType_Vec2, vec2f);
    REGISTER_BASE_TYPE(Vec2);
  }
  {
    NEW_BASE_RESOLVED_TYPE(eScriptType_Vec3, vec3f);
    REGISTER_BASE_TYPE(Vec3);
  }
  {
    NEW_BASE_RESOLVED_TYPE(eScriptType_Vec4, vec4f);
    REGISTER_BASE_TYPE(Vec4);
  }
  {
    NEW_BASE_RESOLVED_TYPE(eScriptType_Matrix, matrixf);
    REGISTER_BASE_TYPE(Matrix);
  }
  {
    NEW_BASE_RESOLVED_TYPE(eScriptType_UUID, uuid);
    REGISTER_BASE_TYPE(UUID);
  }

  // Those are hardcoded in ScriptVM.cpp, ideally it should be cleaned up
  RegisterFunc(table, "vmprint", niNew sScriptTypeMethodDef(_ss, nullptr, &kFuncDecl_vmprint));
  RegisterFunc(table, "vmprintln", niNew sScriptTypeMethodDef(_ss, nullptr, &kFuncDecl_vmprintln));
  RegisterFunc(table, "vmprintdebug", niNew sScriptTypeMethodDef(_ss, nullptr, &kFuncDecl_vmprintdebug));
  RegisterFunc(table, "vmprintdebugln", niNew sScriptTypeMethodDef(_ss, nullptr, &kFuncDecl_vmprintdebugln));

  // Register these first, the iLintFuncCall implementations will override some of them
  RegisterSQRegFunctions(as_nn(table), SQSharedState::_base_funcs);
  RegisterSQRegFunctions(as_nn(table), SQSharedState::_concurrent_funcs);
  RegisterSQRegFunctions(as_nn(table), SQSharedState::_automation_funcs);

  // Do this last so that we override the builtin functions
  RegisterLintFunc(table, MakeNN<sLintFuncCallCreateInstance>(_H("CreateInstance")));
  RegisterLintFunc(table, MakeNN<sLintFuncCallCreateInstance>(_H("CreateGlobalInstance")));
  RegisterLintFunc(table, MakeNN<sLintFuncCallImport>(_H("Import")));
  RegisterLintFunc(table, MakeNN<sLintFuncCallImport>(_H("NewImport"))); // for now NewImport is considered the same as Import by the linter
  RegisterLintFunc(table, MakeNN<sLintFuncCall_root_QueryInterface>());
  RegisterLintFunc(table, MakeNN<sLintFuncCallGetLangDelegate>(_H("GetLangDelegate")));
  RegisterLintFunc(table, MakeNN<sLintFuncCall_lint_check_type>(_H("LINT_CHECK_TYPE")));
  RegisterLintFunc(table, MakeNN<sLintFuncCall_lint_as_type>(_H("LINT_AS_TYPE")));

  // Some delegate methods to override
  {
    SQTable* del = _ddel(_ss,table);
    niPanicAssert(
      OverrideDelegateFunc(del, MakeNN<sLintFuncCall_table_or_array_clone>(_H("DeepClone"))));
    niPanicAssert(
      OverrideDelegateFunc(del, MakeNN<sLintFuncCall_table_or_array_clone>(_H("ShallowClone"))));
    niPanicAssert(
      OverrideDelegateFunc(del, MakeNN<sLintFuncCall_table_setdelegate>(_H("SetDelegate"))));
    niPanicAssert(
      OverrideDelegateFunc(del, MakeNN<sLintFuncCall_table_setdelegate>(_H("setdelegate"))));
    niPanicAssert(
      OverrideDelegateFunc(del, MakeNN<sLintFuncCall_table_getdelegate>(_H("GetDelegate"))));
    niPanicAssert(
      OverrideDelegateFunc(del, MakeNN<sLintFuncCall_table_getdelegate>(_H("getdelegate"))));
  }

  {
    SQTable* del = _ddel(_ss,array);
    niPanicAssert(
      OverrideDelegateFunc(del, MakeNN<sLintFuncCall_table_or_array_clone>(_H("DeepClone"))));
    niPanicAssert(
      OverrideDelegateFunc(del, MakeNN<sLintFuncCall_table_or_array_clone>(_H("ShallowClone"))));
  }

  _lintFuncCallQueryInterface = niNew sLintFuncCall_this_QueryInterface();
}

void SQFunctionProto::LintTrace(
  sLinter& aLinter,
  SQTable* rootTable,
  SQTable* thisModuleTable,
  SQTable* thisTable) const
{
#define IARG0 inst._arg0
#define IARG1 inst._arg1
#define IARG2 inst._arg2
#define IARG3 inst._arg3
#define IEXT inst._ext

  const SQFunctionProto* thisfunc = this;
  LintClosure thisClosure(thisfunc, as_nn(rootTable), as_nn(thisModuleTable), as_nn(thisTable));

  const tBool shouldLintTrace = _ShouldKeepName(
    ni::GetProperty("niScript.LintTrace").c_str(),
    niHStr(thisfunc->GetName()), false);

  const int thisfunc_outerssize = (int)thisfunc->_outervalues.size();
  const int thisfunc_paramssize = (int)(thisfunc->_parameters.size() -
                                        thisfunc_outerssize);
  astl::vector<sLintStackEntry>& stack = thisClosure._stack;
  niLet thisfunc_stacksize = stack.size();

  niLet thisfunc_resolvedrettype = aLinter.ResolveType(thisfunc->_returntype, thisClosure, thisClosure._thisWhenAssigned);

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
    int firstOptional = -1;
    SQObjectPtr firstOptionalName;
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

      // check optional parameter declaration sanity
      if (pi < thisfunc_paramssize) {
        if (_stringval(param._name)[0] == '_') {
          if (firstOptional < 0) {
            firstOptional = pi;
            firstOptionalName = param._name;
          }
        }
        else if (firstOptional >= 0) {
          _LINT_(param_decl, thisfunc->GetSourceLineCol(), niFmt(
            "Non-optional parameter[%d] %s after optional parameter[%d] %s.",
            pi-1, _ObjToString(param._name),
            firstOptional, _ObjToString(firstOptionalName)));
        }
      }

      stack[si]._provenance = _H(niFmt("__param%d__",pi));

      SQObjectPtr resolvedParamType = aLinter.ResolveType(param._type, thisClosure, thisClosure._thisWhenAssigned);
      if (sqa_getscriptobjtype(resolvedParamType) == eScriptType_ErrorCode) {
        if (_LENABLED(param_decl)) {
          _LINT_(param_decl, thisfunc->GetSourceLineCol(), niFmt(
            "Cant resolve type %s of parameter[%d] %s: %s",
            _ObjToString(param._type),
            pi-1, _ObjToString(param._name),
            ((sScriptTypeErrorCode*)_userdata(resolvedParamType))->_strErrorDesc));
        }
      }
      stack[si]._value = resolvedParamType;
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
    SQObjectPtr table = aLinter.CreateTable().raw_ptr();
    SQObjectPtr lname = localname(IARG0);
    if (!lname.IsNull()) {
      _table(table)->SetDebugName(_stringhval(lname));
    }
    _table(table)->Reserve(IARG1);
    _LTRACE(("op_newtable: %s, reserve: %s, lname: %s (%s)",
             _ObjToString(table), IARG1, _ObjToString(lname), sstr(IARG0)));
    sset(IARG0, table);
  };

  auto op_newarray = [&](const SQInstruction& inst) {
    SQObjectPtr array = aLinter.CreateArray(0).raw_ptr();
    SQObjectPtr lname = localname(IARG0);
    _array(array)->Reserve(IARG1);
    _LTRACE(("op_newarray: %s, reserve: %s, lname: %s (%s)",
             _ObjToString(array), IARG1, _ObjToString(lname), sstr(IARG0)));
    sset(IARG0, array);
  };

  auto op_appendarray = [&](const SQInstruction& inst) {
    SQObjectPtr target = sget(IARG0);
    if (!sq_isarray(target)) {
      _LINTERNAL_ERROR(niFmt("op_appendarray: target isn't an array '%s' (%s).",
                             _ObjToString(target), sstr(IARG0)));
    }
    SQObjectPtr val = (IARG2 != 0xFF) ? sget(IARG1) : lget(IARG1);
    _LTRACE(("op_appendarray: %s, val: %s (%s)",
             _ObjToString(target), _ObjToString(val), sstr(IARG1)));
    _array(target)->Append(val);
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
    _LTRACE(("op_loadroottable: %s, %s", sstr(IARG0), _ObjToString(thisClosure._root)));
    sset(IARG0, thisClosure._root);
  };

  auto op_move = [&](const SQInstruction& inst) {
    SQObjectPtr v = sget(IARG1);
    _LTRACE(("op_move: %s -> %s", _ObjToString(v), sstr(IARG0)));
    sset(IARG0, v);
  };

  auto op_dmove = [&](const SQInstruction& inst) {
    SQObjectPtr a1 = sget(IARG1);
    SQObjectPtr a3 = sget(IARG3);
    _LTRACE(("op_dmove: %s -> %s, %s -> %s",
             _ObjToString(a1), sstr(IARG0),
             _ObjToString(a3), sstr(IARG2)));
    sset(IARG0, a1);
    sset(IARG2, a3);
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
            "Cant assign type '%s' to return type '%s'.",
            _ObjTypeString(rval),
            _ObjTypeString(thisfunc_resolvedrettype)));
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
    const int localthisindex = aLinter.GetLocalIndex(*this,_HC(this));
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
             _ObjToString(thisClosure._root)));

    SQFunctionProto* funcproto = _funcproto(func);
    Ptr<LintClosure> lintClosure = niNew LintClosure(
      funcproto,as_nn(_table(thisClosure._root)),as_nn(thisModuleTable),as_nn(_table(localthis)));
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
              if (_LENABLED(key_notfound_outer)) {
                _LINT(key_notfound_outer, _FmtKeyNotFoundMsg(
                  localthis, v._src, _null_));
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

  auto update_slot = [&](ain<SQObjectPtr> t, ain<SQObjectPtr> k, ain<SQObjectPtr> v) {
    if (sq_istable(t)) {
      niLet table = as_nn(_table(t));

      if (aLinter.MaybeEnableLintFromSlot(thisfunc,*table,k,v)) {
        _LTRACE(("update_slot: EnableFromSlot: __lint: %s = %s",
                 _ObjToString(k), _ObjToString(v)));
      }

      // When we set a function as a new slot in a table, that table becomes
      // the default 'this' for the purpose of linting.
      if (sq_type(v) == OT_FUNCPROTO) {
        LintClosure* closure = aLinter.GetClosure(_funcproto(v));
        if (closure) {
          closure->_thisWhenAssigned = table.raw_ptr();
        }
        else {
          _LINTERNAL_ERROR(niFmt("Cant find closure associated with '%s'",
                                 _ObjToString(v)));
        }
        _LTRACE(("update_slot: %s has new this %s",
                 _ObjToString(v), _ObjToString(table.raw_ptr())));
      }
    }

    // assign a function or debug name if there isn't one this takes care of
    // the `someslot <- function() {}` pattern
    if (sq_isstring(k)) {
      if (sq_isfuncproto(v) && _funcproto(v)->_name == _null_) {
        _funcproto(v)->_name = k;
      }
      else if (sq_istable(v) && _table(v)->GetDebugHName() == nullptr) {
        _table(v)->SetDebugName(_stringhval(k));
      }
      // TODO: Should we have a pendantic warning if we're assigning an
      // already named object?
    }
  };

  auto op_set = [&](const SQInstruction& inst) {
    SQObjectPtr t = sget(IARG1);
    SQObjectPtr k = sget(IARG2);
    SQObjectPtr v = sget(IARG3);

    if (sq_isnull(t)) {
      if (_LENABLED(set_in_null)) {
        _LINT(key_cant_set, _FmtKeyNotFoundMsg(_null_, k, _null_));
      }
    }
    else {
      cString errDesc;
      niLet didSet = aLinter.LintSet(errDesc,t,k,v,inst._ext);
      if (!didSet) {
        if (_LENABLED(key_cant_set) &&
            (!sq_isnull(k) || _LENABLED(null_notfound)))
        {
          _LINT(key_cant_set, niFmt(
            "%s key cant be set in %s: %s.",
            _ObjToString(k), _ObjToString(t),
            errDesc.empty() ? "not found" : errDesc.Chars()));
        }
      }
    }

    update_slot(t, k, v);
    if (inst._arg0 != inst._arg3)
      sset(IARG0, v);

    _LTRACE(("op_set: %s[%s] = %s in %s",
             _ObjToString(t), _ObjToString(k),
             _ObjToString(v), sstr(IARG0)));
  };

  auto op_newslot = [&](const SQInstruction& inst) {
    SQObjectPtr t = sget(IARG1);
    SQObjectPtr k = sget(IARG2);
    SQObjectPtr v = sget(IARG3);
    _LTRACE(("op_newslot: %s = %s in %s -> %s",
             _ObjToString(k), _ObjToString(v), _ObjToString(t), sstr(IARG0)));

    if (_sqtype(t) == OT_TABLE) {
      _table(t)->NewSlot(k,v);
    }
    else if ((_GetResolvedObjType(t) != eScriptType_Table)
             && _LENABLED(key_cant_newslot)
             && (!sq_isnull(k) || _LENABLED(null_notfound))
             && (!sq_isnull(t) || _LENABLED(set_in_null)))
    {
      _LINT(key_cant_newslot, niFmt(
        "newslot %s cant be created in %s.",
        _ObjToString(k), _ObjToString(t)));
    }

    update_slot(t, k, v);

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

  auto op_getk = [&](const SQInstruction& inst) {
    SQObjectPtr t = sget(IARG2);
    SQObjectPtr k = lget(IARG1);
    SQObjectPtr v;
    if (is_implicit_this(_LOBJ(implicit_this_getk),inst,inst._arg2)) {
      _LINT(implicit_this_getk, niFmt(
        "implicit this access to %s", _ObjToString(k)));
    }

    if (sq_isnull(t)) {
      if (_LENABLED(getk_in_null)) {
        _LINT(key_notfound_getk, _FmtKeyNotFoundMsg(_null_,k,_null_));
      }
    }
    else {
      niLet didGet = aLinter.LintGet(t,k,v,inst._ext);
      if (!didGet) {
        if (_LENABLED(key_notfound_getk) &&
            (!sq_isnull(k) || _LENABLED(null_notfound)))
        {
          _LINT(key_notfound_getk, _FmtKeyNotFoundMsg(t,k,v));
        }
      }
    }

    sset(IARG0, v);
    _LTRACE(("op_getk: %s[%s] -> %s (%s)",
             _ObjToString(t), _ObjToString(k),
             _ObjToString(v), sstr(IARG0)));
  };

  auto op_get = [&](const SQInstruction& inst) {
    SQObjectPtr t = sget(IARG1);
    SQObjectPtr k = sget(IARG2);
    SQObjectPtr v;
    if (is_implicit_this(_LOBJ(implicit_this_getk),inst,inst._arg1)) {
      _LINT(implicit_this_get, niFmt(
        "implicit this access to %s", _ObjToString(k)));
    }

    if (sq_isnull(t)) {
      if (_LENABLED(getk_in_null)) {
        _LINT(key_notfound_get, _FmtKeyNotFoundMsg(_null_, k, _null_));
      }
    }
    else {
      niLet didGet = aLinter.LintGet(t,k,v,inst._ext);
      if (!didGet) {
        if (_LENABLED(key_notfound_get) &&
            (!sq_isnull(k) || _LENABLED(null_notfound)))
        {
          _LINT(key_notfound_get, _FmtKeyNotFoundMsg(t,k,v));
        }
      }
    }

    sset(IARG0, v);
    _LTRACE(("op_get: %s[%s] in %s",
             _ObjToString(t), _ObjToString(k), sstr(IARG0)));
  };

  auto op_prepcall = [&](const SQInstruction& inst) {
    SQObjectPtr t = sget(IARG2);
    SQObjectPtr k = sget(IARG1);
    if (_LENABLED(prepcall_dynamic) &&
        (!sq_isnull(k) || _LENABLED(null_notfound)))
    {
      _LINT(prepcall_dynamic, niFmt(
        "dynamic function call prevents linting, trying to call %s from %s",
        _ObjToString(k), _ObjToString(t)));
    }

    sset(IARG3, t);
    sset(IARG0, _null_); // TODO: should this return resolved_type<null> instead?
    _LTRACE(("op_prepcall: t = %s (tgt: %s, from: %s), k = %s",
             _ObjToString(t), sstr(IARG3), sstr(IARG2),
             _ObjToString(k)));
  };

  auto op_prepcallk = [&](const SQInstruction& inst) {
    SQObjectPtr t = sget(IARG2);
    SQObjectPtr k = lget(IARG1);
    SQObjectPtr v;
    if (is_implicit_this(_LOBJ(implicit_this_callk),inst,inst._arg2)) {
      _LINT(implicit_this_callk, niFmt(
        "implicit this access to %s", _ObjToString(k)));
    }

    if (sq_isnull(t)) {
      if (_LENABLED(getk_in_null)) {
        _LINT(key_notfound_callk, _FmtKeyNotFoundMsg(_null_, k, _null_));
      }
    }
    else {
      niLet didGet = aLinter.LintGet(t,k,v,inst._ext);
      if (!didGet) {
        if (_LENABLED(key_notfound_callk) &&
            (!sq_isnull(k) || _LENABLED(null_notfound)))
        {
          _LINT(key_notfound_callk, _FmtKeyNotFoundMsg(t,k,v));
        }
      }
    }

    sset(IARG3, t);
    sset(IARG0, v);
    _LTRACE(("op_prepcallk: f = %s (tgt: %s), t = %s (tgt: %s, from: %s), k = %s",
             _ObjToString(v), sstr(IARG0),
             _ObjToString(t), sstr(IARG3), sstr(IARG2),
             _ObjToString(k)));
  };

  auto op_call = [&](const SQInstruction& inst, const tBool abIsTailCall) {
    SQObjectPtr tocall = sget(IARG1);
    niLet tocalltype = sqa_getscriptobjtype(tocall);
    niLet nargs = IARG3;
    niLet stackbase = IARG2;
    niLet thisobj = sget(IARG2);
    _LTRACE(("op_call: '%s' (%s), nargs: %s, stackbase: %d, tailcall: %s, thisobj: %s",
             _ObjToString(tocall),
             ni::GetTypeString(tocalltype),
             nargs,
             stackbase,
             abIsTailCall?"yes":"no",
             _ObjToString(thisobj)));

    auto call_func = [&](ain<SQFunctionProto> func) -> SQObjectPtr {
      const int outerssize = (int)func._outervalues.size();
      const int paramssize = (int)(func._parameters.size() - outerssize);
      const int arity = paramssize-1; // number of paramssize-1 for the implicit this

      _LTRACE(("call_func: %s", _FuncProtoToString(func)));

      if (paramssize != nargs) {
        // is varargs?
        if (arity == 1 && func._parameters[1]._name == _HC(_args_)) {
          // any number of parameters is fine
          niUnused(0);
        }
        // too many arguments is always wrong
        else if (nargs > paramssize) {
          if (_LENABLED(call_num_args)) {
            _LINT(call_num_args,
                  niFmt("call_func: Too many arguments passed, expected at most %d but got %d. Calling %s.",
                        arity, nargs-1, _FuncProtoToString(func)));
          }
        }
        else {
          niLetMut optionalParams = 0;
          niLoop(i, paramssize) {
            niLet paramName = _stringval(func._parameters[i]._name);
            if (paramName[0] == '_') {
              // It's an optional parameter, everything afterward is
              // effectively optional aswell. Validation of the parameter
              // names is done when the closure is created.
              optionalParams = paramssize-i;
              break;
            }
          }

          if (nargs < (paramssize-optionalParams)) {
            if (_LENABLED(call_num_args)) {
              _LINT(call_num_args,
                    niFmt("call_func: Not enough arguments passed, expected at least %d but got %d. Calling %s.",
                          arity-optionalParams, nargs-1, _FuncProtoToString(func)));
            }
          }
        }
      }

      niLet rettype = aLinter.ResolveType(func._returntype, thisClosure, thisobj);
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

      niLet rettype = aLinter.ResolveType(func._returntype, thisClosure, thisobj);
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

      return aLinter.ResolveMethodRetType(*aMeth.pInterfaceDef, *aMeth.pMethodDef);
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
      return aLintFunc->LintCall(aLinter, thisClosure, vArgs);
    };

    auto set_call_ret = [&](ain<tChars> aKind, ain<SQObjectPtr> aRet) -> void {
      if (sqa_getscriptobjtype(aRet) == eScriptType_ErrorCode) {
        niLet errorCode = (sScriptTypeErrorCode*)_userdata(aRet);
        if (_LENABLED(call_error)) {
          _LINT(call_error, niFmt(
            "%s: %s: %s",
            aKind, _ObjToString(tocall), errorCode->_strErrorDesc));
        }
      }
      sset(IARG0, aRet);
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
          set_call_ret("call_closure", ret);
        }
        break;
      }
      case eScriptType_FunctionProto: {
        SQFunctionProto* func = _funcproto(tocall);
        niLet ret = call_func(*func);
        set_call_ret("call_funcproto", ret);
        break;
      }
      case eScriptType_NativeClosure: {
        SQNativeClosure* nativeclosure = _nativeclosure(tocall);
        niLet ret = call_nativeclosure(*nativeclosure);
        set_call_ret("call_nativeclosure", ret);
        break;
      }
      case eScriptType_MethodDef: {
        niLet mdef = (sScriptTypeMethodDef*)_userdata(tocall);
        niLet ret = call_method(*mdef);
        set_call_ret("call_method", ret);
        break;
      }
      case eScriptType_IUnknown: {
        QPtr<iLintFuncCall> lintFuncCall = _iunknown(tocall);
        if (lintFuncCall.IsOK()) {
          niLet ret = call_lint_func(as_NN(lintFuncCall));
          set_call_ret("call_lint_func", ret);
        }
        else {
          if (_LENABLED(call_null)) {
            _LINT(call_null,
                  niFmt("Attempting to call iunknown '%s' (%s).",
                        _ObjToString(tocall),
                        aLinter._ss.GetTypeNameStr(_sqtype(tocall))));
          }
          sset(IARG0, _MakeLintCallError(aLinter, niFmt("Cant call iunknown %s", _ObjToString(tocall))));
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
        sset(IARG0, _MakeLintCallError(aLinter, niFmt("Cant call type %s", _ObjToString(tocall))));
        break;
      }
    }
  };

  struct sLintScope {
    astl::vector<NN<sLintTypeofInfo>> _typeofs;
  };
  astl::stack<sLintScope> scopes;

  auto lint_typeof_eq = [&](ain_nn_mut<sLintTypeofInfo> typeofInfo, const SQObjectPtr& eqLiteral, ain<sVec2i> lineCol) {
    niLet& ss = aLinter._ss;
    niLet typeofObj = typeofInfo->_obj;
    niLet resolvedType = aLinter.ResolveType(eqLiteral, thisClosure, _null_);

    _LTRACE(("lint_typeof_eq: typeofObj: %s (%s), eqLiteral: %s, resolvedType: %s",
             _ObjToString(typeofInfo->_obj),
             typeofInfo->_sstr,
             _ObjToString(eqLiteral),
             _ObjToString(resolvedType)));

    if (!sq_isstring(eqLiteral)) {
      if (_LENABLED(typeof_usage)) {
        _LINT_(typeof_usage, lineCol, niFmt(
          "typeof_eq: Typedef should be a literal string but got '%s', when checking type of '%s'.",
          _ObjToString(eqLiteral),
          typeofInfo->_sstr));
      }
    }
    else {
      if (sqa_getscriptobjtype(resolvedType) == eScriptType_ErrorCode) {
        if (_LENABLED(typeof_usage)) {
          _LINT_(
            typeof_usage, lineCol,
            niFmt("typeof_eq: Invalid typeof test type: %s.", _ObjToString(resolvedType)));
        }
      }

      niLet typeofStr = _stringhval(eqLiteral);
      if ((typeofStr != _HC(typestr_null))
          && (typeofStr != _HC(typestr_int))
          && (typeofStr != _HC(typestr_float))
          && (typeofStr != _HC(typestr_string))
          && (typeofStr != _HC(typestr_table))
          && (typeofStr != _HC(typestr_array))
          && (typeofStr != _HC(typestr_closure))
          && (typeofStr != _HC(typestr_function))
          && (typeofStr != _HC(typestr_userdata))
          && (typeofStr != _HC(typestr_function))
          && (typeofStr != _HC(typestr_iunknown))
          && (typeofStr != _HC(UUID))
          && (typeofStr != _HC(Vec2))
          && (typeofStr != _HC(Vec3))
          && (typeofStr != _HC(Vec4))
          && (typeofStr != _HC(Matrix))
      )
      {
        if (ni::StrStartsWith(niHStr(typeofStr), "enum[")) {
          // TODO: Can we check this?
        }
        else if (_LENABLED(typeof_invalid)) {
          _LINT_(
            typeof_invalid, lineCol,
            niFmt("typeof_eq: Unknown typeof typename: '%s'.", typeofStr));
        }
      }

      if (typeofInfo->_iarg1 >= 0) {
        if (scopes.empty()) {
          _LINTERNAL_ERROR("typeof_eq: Outside of a if/switch scope.");
        }
        else {
          // Update the type of the value what typeof checked.
          // TODO: This is correct for typeof(localvariable or paramname). It
          // doesnt work correctly for typeof(someexpr) since the stack position
          // is temporary then...
          if (!typeofInfo->_wasValue.has_value()) {
            typeofInfo->_wasValue = sget(typeofInfo->_iarg1);
          }
          sset(typeofInfo->_iarg1, resolvedType);
          scopes.top()._typeofs.emplace_back(typeofInfo);
        }
      }
      else {
        if (_LENABLED(typeof_usage)) {
          _LINT_(typeof_usage, lineCol, "typeof_eq: Typeof == used outside of a switch condition prevents linting.");
        }
      }
    }
  };

  auto lint_typeof_restore = [&](ain<tChars> aReason, tBool abClear) {
    if (scopes.empty()) {
      _LINTERNAL_ERROR(niFmt("lint_typeof_restore: Outside of a if/switch scope, '%s'.",aReason));
    }
    else {
      niLetMut& typeofs = scopes.top()._typeofs;
      niLoop(i,typeofs.size()) {
        niLet typeof = typeofs[i];
        niLet typeofObj = typeof->_obj;
        if (typeof->_iarg1 >= 0) {
          if (typeof->_wasValue.has_value() &&
              sqa_getscriptobjtype(typeof->_wasValue.value()) != eScriptType_Null &&
              sqa_getscriptobjtype(typeof->_wasValue.value()) != eScriptType_ErrorCode)
          {
            sset(typeof->_iarg1, typeof->_wasValue.value());
          }
          else {
            sset(typeof->_iarg1, niNew sScriptTypeErrorCode(
              aLinter._ss, _HC(error_code_dangling_type),
              aReason));
          }
        }
      }
      if (abClear) {
        typeofs.clear();
      }
    }
  };

  auto op_typeof = [&](const SQInstruction& inst) {
    SQObjectPtr typeofObj = sget(IARG1);
    _LTRACE(("op_typeof: typeofObj: %s (%s), target: %s",
             _ObjToString(typeofObj), sstr(IARG1), sstr(IARG0)));

    NN_mut<sLintTypeofInfo> typeofInfo = MakeNN<sLintTypeofInfo>();
    typeofInfo->_obj = typeofObj;
    typeofInfo->_sstr = sstr(IARG1);

    if (_LENABLED(typeof_usage)) {
      niLet typeofObjType = _GetResolvedObjType(typeofObj);
      if (typeofObjType != eScriptType_Null && typeofObjType != eScriptType_ErrorCode) {
        _LINT(typeof_usage, niFmt(
          "Redundant typeof with known value type '%s' (%s).",
          _ObjToString(typeofObj), sstr(IARG1)));
      }
    }

    if (IARG2 == 0) {
      typeofInfo->_iarg1 = -1;
    }
    else {
      typeofInfo->_iarg1 = IARG1;
    }

    sset(IARG0, niNew sScriptTypeResolvedType(aLinter._ss, eScriptType_String, _OP_TYPEOF, typeofInfo));
  };

  auto op_eq = [&](const SQInstruction& inst) {
    SQObjectPtr eqLeft = sget(IARG2);
    SQObjectPtr eqRight = (IARG3 != 0) ? lget(IARG1) : sget(IARG1);

    _LTRACE(("op_eq: eqLeft: %s (%s), eqRight: %s (%s)",
             _ObjToString(eqLeft), sstr(IARG2),
             _ObjToString(eqRight), sstr(IARG1)));

    {
      niLet resolvedTypeLeft = _ObjToResolvedTyped(eqLeft);
      if (resolvedTypeLeft.IsOK() && resolvedTypeLeft->_opcode == _OP_TYPEOF) {
        NN_mut<sLintTypeofInfo> typeofInfo { ni::QueryInterface<sLintTypeofInfo>(resolvedTypeLeft->_opcodeInfo) };
        lint_typeof_eq(typeofInfo, eqRight, getlinecol(inst));
      }
    }

    {
      niLet resolvedTypeRight = _ObjToResolvedTyped(eqRight);
      if (resolvedTypeRight.IsOK() && resolvedTypeRight->_opcode == _OP_TYPEOF) {
        NN_mut<sLintTypeofInfo> typeofInfo { ni::QueryInterface<sLintTypeofInfo>(resolvedTypeRight->_opcodeInfo) };
        lint_typeof_eq(typeofInfo, eqLeft, getlinecol(inst));
      }
    }

    sset(IARG0, niNew sScriptTypeResolvedType(aLinter._ss, eScriptType_Int, _OP_EQ, nullptr));
  };

  auto op_cond = [&](const SQInstruction& inst, SQOpcode aOpcode) {
    // TODO: We can probably do some static analysis like "I know for a fact
    // this condition is always true", etc... But that's really low priority.
    sset(IARG0, niNew sScriptTypeResolvedType(aLinter._ss, eScriptType_Int, aOpcode, nullptr));
  };

  auto op_arith = [&](const SQInstruction& inst, SQOpcode aOpcode) {
    SQObjectPtr opLeft = sget(IARG2);
    SQObjectPtr opRight = (IARG3 != 0) ? lget(IARG1) : sget(IARG1);

    _LTRACE(("op_arith: opLeft: %s (%s), opRight: %s (%s)",
             _ObjToString(opLeft), sstr(IARG2),
             _ObjToString(opRight), sstr(IARG1)));

    // by default with return the type of left operand since that's how
    // metamethods would be selected
    sset(IARG0, opLeft);
  };

  auto op_aritheq = [&](const SQInstruction& inst, SQOpcode aOpcode) {
    SQObjectPtr opSelf, opVal;

    _LTRACE(("op_aritheq: iarg1: %s, iarg2: %s, iarg3: %s",
             IARG1, IARG2, IARG3));

    if (IARG3 == 0xFF) {
      opSelf = sget(IARG1);
      opVal = sget(IARG2);
      _LTRACE(("op_aritheq: set: self: %s (%s), val: %s (%s)",
               _ObjToString(opSelf), sstr(IARG1),
               _ObjToString(opVal), sstr(IARG2)));

      if (IARG1 != IARG0)
        sset(IARG1, opSelf);
    }
    else {
      opSelf = sget(IARG1);
      SQObjectPtr opKey = sget(IARG2);
      opVal = sget(IARG3);
      _LTRACE(("op_aritheq: set key: self: %s (%s), key: %s (%s), val: %s (%s)",
               _ObjToString(opSelf), sstr(IARG1),
               _ObjToString(opKey), sstr(IARG2),
               _ObjToString(opVal), sstr(IARG3)));

      if (sq_isnull(opSelf)) {
        if (_LENABLED(getk_in_null)) {
          _LINT(key_notfound_getk, _FmtKeyNotFoundMsg(_null_,opKey,_null_));
        }
      }
      else {
        SQObjectPtr currVal;
        niLet didGet = aLinter.LintGet(opSelf,opKey,currVal,inst._ext);
        if (!didGet) {
          if (_LENABLED(key_notfound_getk) &&
              (!sq_isnull(opKey) || _LENABLED(null_notfound)))
          {
            _LINT(key_notfound_getk, _FmtKeyNotFoundMsg(opSelf,opKey,currVal));
          }
        }
        // NOTE: For typechecking I dont see any reason to check the set portion
        // of the opcode atm since if it can get it it should be able to set it.
#if 0
        else {
          cString errDesc;
          // NOTE: We just set the value, skipping the math since this is not a VM this should do for type checking...
          niLet didSet = aLinter.LintSet(errDesc,opSelf,opKey,opVal,inst._ext);
          if (!didSet) {
            if (_LENABLED(key_cant_set) &&
                (!sq_isnull(opKey) || _LENABLED(null_notfound)))
            {
              _LINT(key_cant_set, niFmt(
                "%s key cant be set in %s: %s.",
                _ObjToString(opKey), _ObjToString(opSelf),
                errDesc.empty() ? "not found" : errDesc.Chars()));
            }
          }
        }
#endif
      }
    }
  };

  auto op_lint_hint = [&](const SQInstruction& inst) {
    niLet hint = (eSQLintHint)inst._arg0;
    SQObjectPtr arg1 = sget(IARG1);

    _LTRACE(("op_lint_hint: hint: %s, arg1: %s (%s)",
             _GetLintHintDesc(hint),
             _ObjToString(arg1), sstr(IARG1)));

    switch (hint) {
      case eSQLintHint_Unknown: {
        break;
      }
      case eSQLintHint_IfBegin:
      case eSQLintHint_SwitchBegin: {
        scopes.push(sLintScope{});
        break;
      }
      case eSQLintHint_IfEnd:
      case eSQLintHint_SwitchEnd: {
        if (scopes.empty()) {
          _LINTERNAL_ERROR(niFmt("op_lint_hint: Outside of a if/switch scope, '%s'.", _GetLintHintDesc(hint)));
        }
        else {
          lint_typeof_restore(niFmt("typeof_end %s", _GetLintHintDesc(hint)), eTrue);
          scopes.pop();
        }
        break;
      }
      case eSQLintHint_SwitchDefault: {
        lint_typeof_restore(niFmt("typeof_end %s", _GetLintHintDesc(hint)), eFalse);
        break;
      }
    }
  };

  auto op_foreach = [&](const SQInstruction& inst) {
    SQObjectPtr othis = sget(IARG0);
    _LTRACE(("op_foreach: othis: %s, okey: %s, oval: %s, orefpos: %s",
             _ObjToString(othis), sstr(IARG2), sstr(IARG2+1), sstr(IARG2+2)));

    niLet othisType = _GetResolvedObjType(othis);
    SQObjectPtr okey = _null_;
    SQObjectPtr oval = _null_;
    SQObjectPtr orefpos = _null_;
    switch (othisType) {
      case eScriptType_Null: {
        // iterating null is a noop
        break;
      }
      case eScriptType_Table: {
        // the key is most of the time a string
        okey = niNew sScriptTypeResolvedType(aLinter._ss, eScriptType_String);
        // iterator is generally going to be a table itself...
        orefpos = niNew sScriptTypeResolvedType(aLinter._ss, eScriptType_Table);
        // value type might be anything
        oval = _null_;
        break;
      }
      case eScriptType_Array: {
        // the key is the iteration index
        okey = niNew sScriptTypeResolvedType(aLinter._ss, eScriptType_Int);
        // the iterator is the index aswell
        orefpos = okey;
        break;
      }
      case eScriptType_String: {
        // key is the byte offset of the current codepoint
        okey = niNew sScriptTypeResolvedType(aLinter._ss, eScriptType_Int);
        // value is the code point
        oval = niNew sScriptTypeResolvedType(aLinter._ss, eScriptType_Int);
        // iterator is iHStringCharIt
        orefpos = aLinter.ResolveTypeUUID(niGetInterfaceID(iHStringCharIt), niGetInterfaceUUID(iHStringCharIt));
        break;
      }
      case eScriptType_Int: {
        okey = oval = orefpos = niNew sScriptTypeResolvedType(aLinter._ss, eScriptType_Int);
        break;
      }
      case eScriptType_InterfaceDef: {
        sScriptTypeInterfaceDef* pV = (sScriptTypeInterfaceDef*)_userdata(othis);
        if ((*pV->pInterfaceDef->mUUID == niGetInterfaceUUID(iIterator)) ||
            (*pV->pInterfaceDef->mUUID == niGetInterfaceUUID(iCollection)))
        {
          // the iterator type
          orefpos = aLinter.ResolveTypeUUID(niGetInterfaceID(iIterator), niGetInterfaceUUID(iIterator));
          // for now we dont retrieve the key/val type but we should be able
          // to derive it from the full typename in sMethodDef if we can carry
          // this here somehow
          okey = oval = _null_;
        }
        else if (_LENABLED(foreach_usage)) {
          _LINT(foreach_usage, niFmt("Cannot iterate '%s'.", _ObjToString(othis)));
        }
        break;
      }
      default: {
        if (_LENABLED(foreach_usage)) {
          _LINT(foreach_usage, niFmt("Cannot iterate '%s'.", _ObjToString(othis)));
        }
        break;
      }
    }

    // output key
    sset(IARG2, okey);
    // output val
    sset(IARG2+1, oval);
    // output iterator
    sset(IARG2+2, orefpos);
  };

  const int localThis = aLinter.GetLocalIndex(*this,_HC(this));
  if (localThis < 0) {
    _LINTERNAL_ERROR("Function doesn't have a local 'this'.");
    return;
  }
  sset(localThis,thisClosure._thisWhenAssigned);

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
             _ObjToString(thisClosure._root))
            // _ExpandedObjToString(thisClosure._root))
    );
  }

  _LTRACE(("--- INST ----------------------------------------------\n"));
  // prologue checks
  {
    niLet inst = 0; // placeholder
    niLet getlinecol = [&](niLet _) {
      return Vec2i(thisfunc->_sourceline, 0);
    };
    niLet retscripttype = sqa_getscriptobjtype(thisfunc_resolvedrettype);
    if (retscripttype == eScriptType_Null && _LENABLED(ret_type_is_null)) {
      if (_LENABLED(ret_type_is_null)) {
        _LINT(ret_type_is_null, niFmt(
          "Return type is Null, specify an explicit return type.",
          _ObjToString(thisfunc->_returntype)));
      }
    }
    else if (retscripttype == eScriptType_ErrorCode) {
      if (_LENABLED(param_decl)) {
        _LINT(param_decl, niFmt(
          "Invalid function return type '%s'.",
          _ObjToString(thisfunc_resolvedrettype)));
      }
    }
  }

  // instructioncs check
  niLoop(i, _instructions.size()) {
    niLet& inst = _instructions[i];
    _LTRACE(("%s %d %d %d %d (%s)",
             _GetOpDesc(inst.op),
             inst._arg0, inst._arg1, inst._arg2, inst._arg3,
             _GetOpExt(inst._ext)));
    switch (inst.op) {
      case _OP_NEWTABLE: op_newtable(inst); break;
      case _OP_NEWARRAY: op_newarray(inst); break;
      case _OP_APPENDARRAY: op_appendarray(inst); break;
      case _OP_LOAD: op_load(inst); break;
      case _OP_LOADNULL: op_loadnull(inst); break;
      case _OP_LOADROOTTABLE: op_loadroottable(inst); break;
      case _OP_MOVE: op_move(inst); break;
      case _OP_DMOVE: op_dmove(inst); break;
      case _OP_RETURN: op_return(inst); break;
      case _OP_CLOSURE: op_closure(inst); break;
      case _OP_NEWSLOT: op_newslot(inst); break;
      case _OP_GETK: op_getk(inst); break;
      case _OP_GET: op_get(inst); break;
      case _OP_PREPCALL: op_prepcall(inst); break;
      case _OP_PREPCALLK: op_prepcallk(inst); break;
      case _OP_SET: op_set(inst); break;
      case _OP_CALL: op_call(inst,eFalse); break;
      case _OP_TAILCALL: op_call(inst,eTrue); break;
      case _OP_SPACESHIP: op_cond(inst, _OP_SPACESHIP); break;
      case _OP_EQ: op_eq(inst); break;
      case _OP_NE: op_cond(inst, _OP_NE); break;
      case _OP_G: op_cond(inst, _OP_G); break;
      case _OP_GE: op_cond(inst, _OP_GE); break;
      case _OP_L: op_cond(inst, _OP_L); break;
      case _OP_LE: op_cond(inst, _OP_LE); break;
      case _OP_EXISTS: op_cond(inst, _OP_EXISTS); break;
      case _OP_TYPEOF: op_typeof(inst); break;
      case _OP_LINT_HINT: op_lint_hint(inst); break;
      case _OP_FOREACH: op_foreach(inst); break;
      case _OP_ADD: op_arith(inst, _OP_ADD); break;
      case _OP_SUB: op_arith(inst, _OP_SUB); break;
      case _OP_MUL: op_arith(inst, _OP_MUL); break;
      case _OP_DIV: op_arith(inst, _OP_DIV); break;
      case _OP_MODULO: op_arith(inst, _OP_MODULO); break;
      case _OP_PLUSEQ: op_aritheq(inst, _OP_PLUSEQ); break;
      case _OP_MINUSEQ: op_aritheq(inst, _OP_MINUSEQ); break;
      case _OP_MULEQ: op_aritheq(inst, _OP_MUL); break;
      case _OP_DIVEQ: op_aritheq(inst, _OP_DIV); break;
      case _OP_MODULOEQ: op_aritheq(inst, _OP_MODULO); break;
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
           _ObjToString(thisClosure._root))
          // _ExpandedObjToString(thisClosure._root))
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
                 _ObjToString(cfclosure->_thisWhenAssigned)));
        sLinter::tLintKindMap wasLintEnabled = aLinter._lintEnabled;
        cfproto->LintTrace(aLinter,_table(cfclosure->_root),thisModuleTable,_table(cfclosure->_thisWhenAssigned));
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

cString SQInstructionToString(ain<SQInstruction> inst) {
  return niFmt("%s %d %d %d %d (%s)",
               _GetOpDesc(inst.op),
               inst._arg0, inst._arg1, inst._arg2, inst._arg3,
               _GetOpExt(inst._ext));
}

tU32 SQFunctionProto::LintTraceRoot() const {
  sLinter linter(this->GetSourceName());
  SQObjectPtr moduleThis = linter.CreateTable().raw_ptr();
  _TableSetDebugNameFromSourceName(as_nn(_table(moduleThis)), "modulethis", this->GetSourceName());
  this->LintTrace(linter,_table(linter._vmroot),_table(moduleThis),_table(moduleThis));
  return linter._numLintErrors;
}
