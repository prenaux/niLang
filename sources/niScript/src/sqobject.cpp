#include "stdafx.h"

#include "sqvm.h"
#include "sqarray.h"
#include "sqtable.h"
#include "sqfuncproto.h"
#include "sqclosure.h"

#include "ScriptVM.h"
#include "ScriptTypes.h"
#include <niLang/IFile.h>

constexpr tU8 _kTagTypeNull = 'Z';
constexpr tU8 _kTagTypeInteger = 'I';
constexpr tU8 _kTagTypeFloat = 'F';
constexpr tU8 _kTagTypeDouble = 'D';
constexpr tU8 _kTagTypeTable = 'T';
constexpr tU8 _kTagTypeArray = 'A';
constexpr tU8 _kTagTypeClosure = 'C';
constexpr tU8 _kTagTypeProto = 'P';
constexpr tU8 _kTagTypeStringShort = 'S'; // < _kLongStringLen chars
constexpr tU32 _knLongStringLen = 255;
constexpr tU8 _kTagTypeStringLong = 'X';

constexpr tU8 _kTagFuncProtoHeader = 'h';
constexpr tU8 _kTagFuncProtoRetAndParams = 'p';
constexpr tU8 _kTagFuncProtoLiterals = 'r';
constexpr tU8 _kTagFuncProtoOuters = 'o';
constexpr tU8 _kTagFuncProtoLocals = 'l';
constexpr tU8 _kTagFuncProtoLines = 'd';
constexpr tU8 _kTagFuncProtoInstructions = 'y';
constexpr tU8 _kTagFuncProtoFunctions = 'x';

const SQChar* SQFunctionProto::GetLocal(SQVM *vm,tU32 stackbase,tU32 nseq,tU32 nop)
{
  const tSize nvars=_localvarinfos.size();
  const SQChar *res=NULL;
  if(nvars>=nseq){
    for(tU32 i=0;i<nvars;i++){
      if(_localvarinfos[i]._start_op<=nop && _localvarinfos[i]._end_op>=nop)
      {
        if(nseq==0){
          vm->Push(vm->_stack[stackbase+_localvarinfos[i]._pos]);
          res=_stringval(_localvarinfos[i]._name);
          break;
        }
        nseq--;
      }
    }
  }
  return res;
}

sVec2i SQFunctionProto::_GetLineCol(const SQInstructionVec &instructions,
                                    const SQInstruction *curr,
                                    const SQLineInfoVec &lineinfos) {
  tI32 op = (curr - (&instructions[0]));
  tI32 line = lineinfos[0]._line;
  tI32 column = lineinfos[0]._column;
  for (tU32 i = 1; i < lineinfos.size(); i++) {
    if (lineinfos[i]._op > op) {
      return Vec2i(line, column);
    }
    line = lineinfos[i]._line;
    column = lineinfos[i]._column;
  }
  return Vec2i(line, column);
}

sVec2i SQFunctionProto::GetLineCol(const SQInstruction *curr) const
{
  return _GetLineCol(_instructions, curr, _lineinfos);
}

#define _CHECK_IO(exp,reason)  { if(!(exp)) { v->Raise_MsgError("io error: " reason); return false; } }

static inline bool SafeWrite(SQVM* v, ain<nn<ni::iFile>> fp, ni::tPtr dest, tI32 size)
{
  if (fp->WriteRaw(dest,size) != size) {
    v->Raise_MsgError("io error (write function failure)");
    return false;
  }
  return true;
}

static inline bool SafeRead(SQVM* v, ain<nn<ni::iFile>> fp, ni::tPtr dest, tI32 size)
{
  if (size && fp->ReadRaw(dest,size) != size) {
    v->Raise_MsgError("io error, read function failure, the origin stream could be corrupted/trucated");
    return false;
  }
  return true;
}

static inline bool ReadAndCheckTag(SQVM* v, ain<nn<ni::iFile>> fp, ain<tU8> aExpectedTag)
{
  niLet readTag = fp->Read8();
  if (readTag != aExpectedTag) {
    v->Raise_MsgError(niFmt(
      "invalid or corrupted closure stream, expected %c (%d) but got %c (%d).",
      aExpectedTag, aExpectedTag, readTag, readTag
    ));
    return false;
  }
  return true;
}

bool WriteSQObject(SQVM* v, ain<nn<ni::iFile>> fp, ain<SQObjectPtr> o)
{
  switch (_sqtype(o)) {
    case OT_NULL: {
      fp->Write8(_kTagTypeNull);
      break;
    }
    case OT_INTEGER: {
      fp->Write8(_kTagTypeInteger);
      fp->WriteLE32((tU32)_int(o));
      break;
    }
    case OT_FLOAT: {
      fp->Write8(_kTagTypeDouble);
      fp->WriteF64(_float(o));
      break;
    }
    case OT_STRING: {
      niLet hstr = _stringhval(o);
      niLet len = hstr->GetLength();
      const char* text = hstr->GetChars();
      if (len < _knLongStringLen) {
        fp->Write8(_kTagTypeStringShort);
        fp->Write8((tU8)len);
        _CHECK_IO(SafeWrite(v,fp,(tPtr)text,len),"write short string");
      }
      else {
        fp->Write8(_kTagTypeStringLong);
        fp->WriteLE32(len);
        _CHECK_IO(SafeWrite(v,fp,(tPtr)text,len),"write long string");
      }
      break;
    }
    case OT_CLOSURE: {
      fp->Write8(_kTagTypeClosure);
      _CHECK_IO(WriteSQFunctionProto(v, fp, _funcproto(_closure(o)->_function)), "write closure funcproto");
      break;
    }
    case OT_TABLE: {
      fp->Write8(_kTagTypeTable);
      SQTable* table = _table(o);
      tU32 nsize = (tU32)table->GetHMap().size();
      niLet locked = table->SerializeWriteLock();
      niDefer { table->SerializeWriteUnlock(); };
      if (!locked) {
        v->Raise_MsgError("Table already serialized, cyclic structures can't be serialized");
        return false;
      }
      fp->WriteLE32(nsize);
      tU32 nwritten = 0;
      niLoopit(SQTable::tHMapCIt,it,table->GetHMap()) {
        _CHECK_IO(WriteSQObject(v,fp,it->first), "write table key");
        _CHECK_IO(WriteSQObject(v,fp,it->second), "write table value");
        ++nwritten;
      }
      if (nsize != nwritten) {
        v->Raise_MsgError(niFmt(
          "Table size mismatch, nsize is %d but serialized %d fields.",
          nsize, nwritten));
        return eFalse;
      }
      break;
    };
    case OT_ARRAY: {
      fp->Write8(_kTagTypeArray);
      SQArray* array = _array(o);
      const SQObjectPtrVec& values = array->_values;
      tU32 nsize = (tU32)values.size();
      niLet locked = array->SerializeWriteLock();
      niDefer { array->SerializeWriteUnlock(); };
      if (!locked) {
        v->Raise_MsgError("Array already serialized, cyclic structures can't be serialized");
        return false;
      }
      fp->WriteLE32(nsize);
      niLoop(i,nsize) {
        _CHECK_IO(WriteSQObject(v,fp,values[i]), "write array value");
      }
      break;
    };
    default: {
      v->Raise_MsgError(niFmt(
          "cannot serialize write a '%s' (%d)",
          v->_ss->GetTypeNameStr(o), _sqtype(o)));
      return false;
    }
  }
  return true;
}

bool ReadSQObject(SQVM* v, ain<nn<ni::iFile>> fp, aout<SQObjectPtr> o)
{
  niLet readTag = fp->Read8();
  switch (readTag) {
    case _kTagTypeNull: {
      o = _null_;
      break;
    }
    case _kTagTypeInteger: {
      static_assert(sizeof(SQInt) == 4);
      o = (SQInt)fp->ReadLE32();
      break;
    }
    case _kTagTypeFloat: {
      o = (SQFloat)fp->ReadF32();
      break;
    }
    case _kTagTypeDouble: {
      static_assert(sizeof(SQFloat) == 8);
      o = (SQFloat)fp->ReadF64();
      break;
    }
    case _kTagTypeStringShort: {
      niLet len = fp->Read8();
      if (len) {
        char buffer[_knLongStringLen+1];
        buffer[len] = 0;
        _CHECK_IO(SafeRead(v,fp,(tPtr)buffer,len),"read short string");
        _CHECK_IO(
          buffer[len-1] != 0 && buffer[len] == 0,
          "read short string, invalid end of string");
        o = _H(buffer);
      }
      else {
        o = _H("");
      }
      break;
    }
    case _kTagTypeStringLong: {
      niLet len = fp->ReadLE32();
      if (len) {
        astl::vector<char> buffer;
        buffer.resize(len+1);
        buffer[len] = 0;
        _CHECK_IO(SafeRead(v,fp,(tPtr)buffer.data(),len),"read long string");
        _CHECK_IO(
          buffer[len-1] != 0 && buffer[len] == 0,
          "read long string, invalid end of string");
        o = _H(buffer.data());
      }
      else {
        o = _H("");
      }
      break;
    }
    case _kTagTypeClosure: {
      SQObjectPtr func = SQFunctionProto::Create();
      o = SQClosure::Create(_funcproto(func),_table(v->_roottable));
      _CHECK_IO(ReadSQFunctionProto(v,fp,_funcproto(_closure(o)->_function)),"read closure");
      break;
    }
    case _kTagTypeTable: {
      niLet nsize = fp->ReadLE32();
      o = SQTable::Create();
      _table(o)->Reserve(nsize);
      SQTable::tHMap& hmap = _table(o)->GetHMap();
      niLoop(i,nsize) {
        SQObjectPtr key, value;
        _CHECK_IO(ReadSQObject(v,fp,key),"read table key");
        _CHECK_IO(ReadSQObject(v,fp,value),"read table value");
        astl::upsert(hmap,key,value);
      }
      break;
    }
    case _kTagTypeArray: {
      niLet nsize = fp->ReadLE32();
      o = SQArray::Create(nsize);
      SQObjectPtrVec& values = _array(o)->_values;
      niLoop(i,nsize) {
        _CHECK_IO(ReadSQObject(v,fp,values[i]),"read array value");
      }
      break;
    }
    default: {
      v->Raise_MsgError(niFmt(
        "cannot serialize read tag '%c' (%d)",
        readTag, readTag));
      return false;
    }
  }
  return true;
}

bool WriteSQFunctionProto(SQVM *v, ain<nn<ni::iFile>> fp, SQFunctionProto* aProto) {
  _CHECK_IO(fp->Write8(_kTagFuncProtoHeader), "write tag closure header");
  _CHECK_IO(WriteSQObject(v,fp,aProto->_sourcename), "write sourcename");
  _CHECK_IO(WriteSQObject(v,fp,aProto->_name), "write name");
  _CHECK_IO(fp->WriteLE32(aProto->_stacksize), "write stacksize");

  {
    _CHECK_IO(fp->Write8(_kTagFuncProtoRetAndParams), "write tag closure ret params");
    _CHECK_IO(WriteSQObject(v,fp,aProto->_returntype), "write return type");
    const tU32 nsize = (tU32)aProto->_parameters.size();
    _CHECK_IO(fp->WriteLE32(nsize), "write parameters size");
    niLoop(i,nsize) {
      _CHECK_IO(WriteSQObject(v,fp,aProto->_parameters[i]._name), "write param name");
      _CHECK_IO(WriteSQObject(v,fp,aProto->_parameters[i]._type), "write param type");
    }
  }

  {
    _CHECK_IO(fp->Write8(_kTagFuncProtoLiterals), "write tag closure literals");
    const tU32 nsize = (tU32)aProto->_literals.size();
    _CHECK_IO(fp->WriteLE32(nsize), "write literals size");
    niLoop(i,nsize) {
      _CHECK_IO(WriteSQObject(v,fp,aProto->_literals[i]), "write literal");
    }
  }

  {
    _CHECK_IO(fp->Write8(_kTagFuncProtoOuters), "write tag closure outers");
    const tU32 nsize = (tU32)aProto->_outervalues.size();
    _CHECK_IO(fp->WriteLE32(nsize), "write outervals size");
    niLoop(i,nsize) {
      _CHECK_IO(fp->Write8(aProto->_outervalues[i]._blocal), "write outerval blocal");
      _CHECK_IO(WriteSQObject(v,fp,aProto->_outervalues[i]._src), "write outerval src");
    }
  }

  {
    _CHECK_IO(fp->Write8(_kTagFuncProtoLocals), "write tag closure locals");
    const tU32 nsize = (tU32)aProto->_localvarinfos.size();
    _CHECK_IO(fp->WriteLE32(nsize), "write localvars size");
    niLoop(i,nsize) {
      SQLocalVarInfo &lvi=aProto->_localvarinfos[i];
      _CHECK_IO(WriteSQObject(v,fp,lvi._name), "write localvar name");
      _CHECK_IO(fp->WriteLE32(lvi._pos), "write localvar pos");
      _CHECK_IO(fp->WriteLE32(lvi._start_op), "write localvar start_op");
      _CHECK_IO(fp->WriteLE32(lvi._end_op), "write localvar end_op");
    }
  }

  {
    static_assert(sizeof(SQLineInfo) == 12);
    _CHECK_IO(fp->Write8(_kTagFuncProtoLines), "write tag closure lines");
    const tU32 nsize = (tU32)aProto->_lineinfos.size();
    _CHECK_IO(fp->WriteLE32(nsize), "write lineinfos size");
    _CHECK_IO(fp->WriteRaw(aProto->_lineinfos.data(),sizeof(SQLineInfo)*nsize) == sizeof(SQLineInfo)*nsize, "write lineinfos");
  }

  {
    static_assert(sizeof(SQInstruction) == 8);
    _CHECK_IO(fp->Write8(_kTagFuncProtoInstructions), "write tag closure instructions");
    const tU32 nsize = (tU32)aProto->_instructions.size();
    _CHECK_IO(fp->WriteLE32(nsize), "write instructions size");
    _CHECK_IO(fp->WriteRaw(aProto->_instructions.data(),sizeof(SQInstruction)*nsize) == sizeof(SQInstruction)*nsize, "write instructions");
  }

  {
    _CHECK_IO(fp->Write8(_kTagFuncProtoFunctions), "write tag closure functions");
    const tU32 nsize = (tU32)aProto->_functions.size();
    _CHECK_IO(fp->WriteLE32(nsize), "write functions size");
    niLoop(i,nsize) {
      _CHECK_IO(WriteSQFunctionProto(v,fp,_funcproto(aProto->_functions[i])), "write function");
    }
  }

  return true;
}

bool ReadSQFunctionProto(SQVM *v, ain<nn<ni::iFile>> fp, SQFunctionProto* aProto) {
  SQObjectPtr o;
  {
    _CHECK_IO(ReadAndCheckTag(v,fp,_kTagFuncProtoHeader), "read tag");
    _CHECK_IO(ReadSQObject(v,fp,aProto->_sourcename), "read sourcename");
    _CHECK_IO(ReadSQObject(v,fp,aProto->_name), "read name");
    aProto->_stacksize = fp->ReadLE32();
  }

  {
    _CHECK_IO(ReadAndCheckTag(v,fp,_kTagFuncProtoRetAndParams), "read tag closure ret params");
    _CHECK_IO(ReadSQObject(v,fp,aProto->_returntype), "read return type");
    niLet paramsSize = fp->ReadLE32();
    niLoop(i,paramsSize) {
      SQFunctionParameter param;
      _CHECK_IO(ReadSQObject(v,fp,param._name), "read param name");
      _CHECK_IO(ReadSQObject(v,fp,param._type), "read param type");
      aProto->_parameters.push_back(param);
    }
  }

  {
    _CHECK_IO(ReadAndCheckTag(v,fp,_kTagFuncProtoLiterals), "read tag closure literals");
    niLet nsize = fp->ReadLE32();
    niLoop(i,nsize) {
      _CHECK_IO(ReadSQObject(v,fp,o), "read literal");
      aProto->_literals.push_back(o);
    }
  }

  {
    _CHECK_IO(ReadAndCheckTag(v,fp,_kTagFuncProtoOuters), "read tag closure outers");
    niLet outerSize = fp->ReadLE32();
    aProto->_outervalues.resize(outerSize);
    niLoop(i,outerSize) {
      niLet bl = fp->Read8();
      _CHECK_IO(ReadSQObject(v,fp,o), "read outerval");
      aProto->_outervalues[i] = SQOuterVar(o,bl);
    }
  }

  {
    _CHECK_IO(ReadAndCheckTag(v,fp,_kTagFuncProtoLocals), "read tag closure locals");
    niLet localVarsSize = fp->ReadLE32();
    aProto->_localvarinfos.resize(localVarsSize);
    niLoop(i,localVarsSize) {
      SQLocalVarInfo& lvi = aProto->_localvarinfos[i];
      _CHECK_IO(ReadSQObject(v,fp,lvi._name), "read localvar name");
      lvi._pos = fp->ReadLE32();
      lvi._start_op = fp->ReadLE32();
      lvi._end_op = fp->ReadLE32();
    }
  }

  {
    static_assert(sizeof(SQLineInfo) == 12);
    _CHECK_IO(ReadAndCheckTag(v,fp,_kTagFuncProtoLines), "read tag closure lines");
    niLet lineInfosSize = fp->ReadLE32();
    aProto->_lineinfos.resize(lineInfosSize);
    _CHECK_IO(fp->ReadRaw(aProto->_lineinfos.data(),sizeof(SQLineInfo)*lineInfosSize) == sizeof(SQLineInfo)*lineInfosSize, "read lineinfos");
  }

  {
    static_assert(sizeof(SQInstruction) == 8);
    _CHECK_IO(ReadAndCheckTag(v,fp,_kTagFuncProtoInstructions), "read tag instructions");
    niLet instructionsSize = fp->ReadLE32();
    aProto->_instructions.resize(instructionsSize);
    _CHECK_IO(fp->ReadRaw(aProto->_instructions.data(),sizeof(SQInstruction)*instructionsSize) == sizeof(SQInstruction)*instructionsSize, "read instructions");
  }

  {
    _CHECK_IO(ReadAndCheckTag(v,fp,_kTagFuncProtoFunctions), "read tag closure functions");
    niLet functionsSize = fp->ReadLE32();
    aProto->_functions.resize(functionsSize);
    niLoop(i,functionsSize) {
      aProto->_functions[i] = SQFunctionProto::Create();
      _CHECK_IO(ReadSQFunctionProto(v,fp,_funcproto(aProto->_functions[i])), "read function");
    }
  }

  return true;
}

#ifndef NO_GARBAGE_COLLECTOR
void SQSharedState::Mark(SQCollectable **chain) {
  SQGarbageCollector::MarkObject(_refs_table,chain);
  SQGarbageCollector::MarkObject(_scriptimports_table,chain);
  SQGarbageCollector::MarkObject(_nativeimports_table,chain);
  SQGarbageCollector::MarkObject(_table_default_delegate,chain);
  SQGarbageCollector::MarkObject(_array_default_delegate,chain);
  SQGarbageCollector::MarkObject(_string_default_delegate,chain);
  SQGarbageCollector::MarkObject(_number_default_delegate,chain);
  SQGarbageCollector::MarkObject(_closure_default_delegate,chain);
  SQGarbageCollector::MarkObject(_idxprop_default_delegate,chain);
  SQGarbageCollector::MarkObject(_vec2f_default_delegate,chain);
  SQGarbageCollector::MarkObject(_vec3f_default_delegate,chain);
  SQGarbageCollector::MarkObject(_vec4f_default_delegate,chain);
  SQGarbageCollector::MarkObject(_matrixf_default_delegate,chain);
  SQGarbageCollector::MarkObject(_uuid_default_delegate,chain);
  SQGarbageCollector::MarkObject(_enum_default_delegate,chain);
  SQGarbageCollector::MarkObject(_method_default_delegate,chain);
  for (SQSharedState::tDelegateMap::iterator it = mmapDelegates.begin();
       it != mmapDelegates.end(); ++it)
  {
    SQGarbageCollector::MarkObject(it->second,chain);
  }
  for (SQSharedState::tEnumDefMap::iterator it = mmapEnumDefs.begin();
       it != mmapEnumDefs.end(); ++it)
  {
    SQGarbageCollector::MarkObject(it->second,chain);
  }
}

void SQVM::Mark(SQCollectable **chain)
{
  START_MARK();
  SQGarbageCollector::MarkObject(_lasterror,chain);
  SQGarbageCollector::MarkObject(_errorhandler,chain);
  SQGarbageCollector::MarkObject(_debughook,chain);
  SQGarbageCollector::MarkObject(_roottable, chain);
  for(tU32 i = 0; i < _stack.size(); i++) {
    SQGarbageCollector::MarkObject(_stack[i], chain);
  }
  for(tU32 i = 0; i < _callsstack.size(); i++) {
    SQGarbageCollector::MarkObject(_callsstack[i]._closurePtr, chain);
  }
  END_MARK(chain);
}

void SQClosure::Mark(SQCollectable **chain)
{
  START_MARK();
  for(tU32 i = 0; i < _outervalues.size(); i++) {
    SQGarbageCollector::MarkObject(_outervalues[i], chain);
  }
  END_MARK(chain);
}

void SQNativeClosure::Mark(SQCollectable **chain)
{
  START_MARK();
  END_MARK(chain);
}

void SQCollectable::UnMark() { UNMARK(); }
#endif

tBool vm_string_nexti(iHString* hsp, const SQObjectPtr &refpos, SQObjectPtr &outkey, SQObjectPtr &outval, SQObjectPtr& itr)
{
  Ptr<iHStringCharIt> it;
  if (_sqtype(refpos) == OT_NULL) {
    if (HStringIsEmpty(hsp))
      return eFalse;
    it = hsp->CreateCharIt(0);
    niAssert(!it->GetIsEnd());
  }
  else {
    it = down_cast<iHStringCharIt*>(_iunknown(refpos));
    niAssert(!it->GetIsEnd());
    it->Next();
    if (it->GetIsEnd()) {
      return eFalse;
    }
  }
  outkey = (SQInt)it->GetPosition();
  outval = (SQInt)it->PeekNext();
  itr = it.ptr();
  return eTrue;
}
