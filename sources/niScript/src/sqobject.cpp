#include "stdafx.h"

#include "sqvm.h"
#include "sqarray.h"
#include "sqtable.h"
#include "sqfuncproto.h"
#include "sqclosure.h"

#include "ScriptVM.h"
#include "ScriptTypes.h"
#include <niLang/IFile.h>

static const char _kTagPart[2] = { 'C', '1' };
static const char _kTagTail[2] = { 'E', 'D' };

#define SQ_CLOSURESTREAM_PART _kTagPart
#define SQ_CLOSURESTREAM_TAIL _kTagTail

tU32 TranslateIndex(const SQObjectPtr &idx)
{
  switch(_sqtype(idx)){
    case OT_NULL:
      return 0;
    case OT_INTEGER:
      return (tU32)_int(idx);
  }
  niAssertUnreachable("Invalid index type.");
  return 0;
}

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

#define _CHECK_IO(exp)  { if(!exp) { v->Raise_MsgError("io error"); return false; } }

bool SafeWrite(HSQUIRRELVM v,SQWRITEFUNC write, ni::tPtr up,ni::tPtr dest,tI32 size)
{
  if(write(up,dest,size) != size) {
    v->Raise_MsgError(_A("io error (write function failure)"));
    return false;
  }
  return true;
}

bool SafeRead(HSQUIRRELVM v,SQWRITEFUNC read, ni::tPtr up,ni::tPtr dest,tI32 size)
{
  if(size && read(up,dest,size) != size) {
    v->Raise_MsgError(_A("io error, read function failure, the origin stream could be corrupted/trucated"));
    return false;
  }
  return true;
}

bool WriteTag(HSQUIRRELVM v,SQWRITEFUNC write, ni::tPtr up, const char tag[2])
{
  return SafeWrite(v,write,up,(tPtr)tag,2);
}

bool CheckTag(HSQUIRRELVM v,SQWRITEFUNC read, ni::tPtr up, const char tag[2])
{
  char t[2];
  _CHECK_IO(SafeRead(v,read,up,(tPtr)t,2));
  if (t[0] != tag[0] || t[1] != tag[1]) {
    v->Raise_MsgError(_A("invalid or corrupted closure stream"));
    return false;
  }
  return true;
}

bool WriteObject(HSQUIRRELVM v,ni::tPtr up,SQWRITEFUNC write,const SQObjectPtr &o)
{
  {
    const SQObjectType t = _sqtype(o);
    _CHECK_IO(SafeWrite(v,write,up,(tPtr)&t,sizeof(SQObjectType)));
  }
  switch(_sqtype(o)){
    case OT_STRING: {
      reinterpret_cast<iFile*>(up)->WriteBitsString(_stringval(o));
      break;
    }
    case OT_INTEGER:
      _CHECK_IO(SafeWrite(v,write,up,(tPtr)&_int(o),sizeof(SQInt)));break;
    case OT_FLOAT:
      _CHECK_IO(SafeWrite(v,write,up,(tPtr)&_float(o),sizeof(SQFloat)));break;
    case OT_NULL:
      break;
    case OT_CLOSURE: {
      _CHECK_IO(WriteSQClosure(_closure(o), v, up, write));
      break;
    }
    case OT_TABLE: {
      SQTable* table = _table(o);
      const tSize nsize = table->GetHMap().size();
      if (!table->SerializeWriteLock()) {
        table->SerializeWriteUnlock();
        v->Raise_MsgError("Table already serialized, cyclic structures can't be serialized");
        return false;
      }
      _CHECK_IO(SafeWrite(v,write,up,(tPtr)&nsize,sizeof(nsize)));
      niLoopit(SQTable::tHMapCIt,it,table->GetHMap()) {
        _CHECK_IO(WriteObject(v,up,write,it->first));
        _CHECK_IO(WriteObject(v,up,write,it->second));
      }
      table->SerializeWriteUnlock();
      break;
    };
    case OT_ARRAY: {
      SQArray* array = _array(o);
      const SQObjectPtrVec& values = array->_values;
      const tSize nsize = values.size();
      if (!array->SerializeWriteLock()) {
        array->SerializeWriteUnlock();
        v->Raise_MsgError("Array already serialized, cyclic structures can't be serialized");
        return false;
      }
      _CHECK_IO(SafeWrite(v,write,up,(tPtr)&nsize,sizeof(nsize)));
      niLoop(i,nsize) {
        _CHECK_IO(WriteObject(v,up,write,values[i]));
      }
      array->SerializeWriteUnlock();
      break;
    };
    default: {
      v->Raise_MsgError(niFmt(
          "cannot serialize write a '%s'",
          v->_ss->GetTypeNameStr(o), _sqtype(o)));
      return false;
    }
  }
  return true;
}

bool ReadObject(HSQUIRRELVM v,ni::tPtr up,SQREADFUNC read,SQObjectPtr &o)
{
  SQObjectType t;
  _CHECK_IO(SafeRead(v,read,up,(tPtr)&t,sizeof(SQObjectType)));
  switch(t) {
    case OT_STRING: {
      cString strOut = reinterpret_cast<iFile*>(up)->ReadBitsString();
      o = _H(strOut);
      break;
    }
    case OT_INTEGER: {
      SQInt i;
      _CHECK_IO(SafeRead(v,read,up,(tPtr)&i,sizeof(SQInt)));
      o = i;
      break;
    }
    case OT_FLOAT: {
      SQFloat f;
      _CHECK_IO(SafeRead(v,read,up,(tPtr)&f,sizeof(SQFloat)));
      o = f;
      break;
    }
    case OT_NULL: {
      o=_null_;
      break;
    }
    case OT_CLOSURE: {
      SQObjectPtr func=SQFunctionProto::Create();
      o = SQClosure::Create(_funcproto(func),_table(v->_roottable));
      if(!ReadSQClosure(_closure(o),v,up,read)) {
        return false;
      }
      break;
    }
    case OT_TABLE: {
      tI32 nsize;
      _CHECK_IO(SafeRead(v,read,up,(tPtr)&nsize,sizeof(nsize)));
      o = SQTable::Create();
      SQTable::tHMap& hmap = _table(o)->GetHMap();
      niLoop(i,nsize) {
        SQObjectPtr key, value;
        _CHECK_IO(ReadObject(v,up,read,key));
        _CHECK_IO(ReadObject(v,up,read,value));
        astl::upsert(hmap,key,value);
      }
      break;
    }
    case OT_ARRAY: {
      tI32 nsize;
      _CHECK_IO(SafeRead(v,read,up,(tPtr)&nsize,sizeof(nsize)));
      o = SQArray::Create(nsize);
      SQObjectPtrVec& values = _array(o)->_values;
      niLoop(i,nsize) {
        _CHECK_IO(ReadObject(v,up,read,values[i]));
      }
      break;
    }
    default: {
      v->Raise_MsgError(niFmt(
          "cannot serialize read a '%s' (%d)",
          v->_ss->GetTypeNameStr(t), (tInt)t));
      return false;
    }
  }
  return true;
}

bool WriteSQClosure(SQClosure* _this, SQVM *v,ni::tPtr up,SQWRITEFUNC write)
{
  //_CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_HEAD));
  //_CHECK_IO(WriteTag(v,write,up,sizeof(SQChar)));
  _CHECK_IO(WriteSQFunctionProto(_funcproto(_this->_function),v,up,write));
  _CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_TAIL));
  return true;
}

bool ReadSQClosure(SQClosure* _this, SQVM *v,ni::tPtr up,SQREADFUNC read)
{
  //_CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_HEAD));
  //_CHECK_IO(CheckTag(v,read,up,sizeof(SQChar)));
  _CHECK_IO(ReadSQFunctionProto(_funcproto(_this->_function), v,up,read));
  _CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_TAIL));
  return true;
}

bool WriteSQFunctionProto(SQFunctionProto* _this, SQVM *v,ni::tPtr up,SQWRITEFUNC write)
{
  tI32 i,nsize=(tI32)_this->_literals.size();
  _CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART));
  _CHECK_IO(WriteObject(v,up,write,_this->_sourcename));
  _CHECK_IO(WriteObject(v,up,write,_this->_name));
  _CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART));
  _CHECK_IO(SafeWrite(v,write,up,(tPtr)&nsize,sizeof(nsize)));
  for(i=0;i<nsize;i++){
    _CHECK_IO(WriteObject(v,up,write,_this->_literals[i]));
  }
  _CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART));
  nsize=(tI32)_this->_parameters.size();
  _CHECK_IO(SafeWrite(v,write,up,(tPtr)&nsize,sizeof(nsize)));
  for(i=0;i<nsize;i++){
    _CHECK_IO(WriteObject(v,up,write,_this->_parameters[i]._name));
    _CHECK_IO(WriteObject(v,up,write,_this->_parameters[i]._type));
  }
  _CHECK_IO(WriteObject(v,up,write,_this->_returntype));
  _CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART));
  nsize=(tI32)_this->_outervalues.size();
  _CHECK_IO(SafeWrite(v,write,up,(tPtr)&nsize,sizeof(nsize)));
  for(i=0;i<nsize;i++){
    _CHECK_IO(SafeWrite(v,write,up,(tPtr)&_this->_outervalues[i]._blocal,sizeof(bool)));
    _CHECK_IO(WriteObject(v,up,write,_this->_outervalues[i]._src));
  }
  _CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART));
  nsize=(tI32)_this->_localvarinfos.size();
  _CHECK_IO(SafeWrite(v,write,up,(tPtr)&nsize,sizeof(nsize)));
  for(i=0;i<nsize;i++){
    SQLocalVarInfo &lvi=_this->_localvarinfos[i];
    _CHECK_IO(WriteObject(v,up,write,lvi._name));
    _CHECK_IO(SafeWrite(v,write,up,(tPtr)&lvi._pos,sizeof(tU32)));
    _CHECK_IO(SafeWrite(v,write,up,(tPtr)&lvi._start_op,sizeof(tU32)));
    _CHECK_IO(SafeWrite(v,write,up,(tPtr)&lvi._end_op,sizeof(tU32)));
  }
  _CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART));
  nsize=(tI32)_this->_lineinfos.size();
  _CHECK_IO(SafeWrite(v,write,up,(tPtr)&nsize,sizeof(nsize)));
  _CHECK_IO(SafeWrite(v,write,up,(tPtr)&_this->_lineinfos[0],sizeof(SQLineInfo)*nsize));
  _CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART));
  nsize=(tI32)_this->_instructions.size();
  _CHECK_IO(SafeWrite(v,write,up,(tPtr)&nsize,sizeof(nsize)));
  _CHECK_IO(SafeWrite(v,write,up,(tPtr)&_this->_instructions[0],sizeof(SQInstruction)*nsize));
  _CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART));
  nsize=(tI32)_this->_functions.size();
  _CHECK_IO(SafeWrite(v,write,up,(tPtr)&nsize,sizeof(nsize)));
  for(i=0;i<nsize;i++){
    _CHECK_IO(WriteSQFunctionProto(_funcproto(_this->_functions[i]),v,up,write));
  }
  _CHECK_IO(SafeWrite(v,write,up,(tPtr)&_this->_stacksize,sizeof(_this->_stacksize)));
  return true;
}

bool ReadSQFunctionProto(SQFunctionProto* _this, SQVM *v,ni::tPtr up,SQREADFUNC read)
{
  tI32 i, nsize = (tI32)_this->_literals.size();
  SQObjectPtr o;
  _CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));
  _CHECK_IO(ReadObject(v, up, read, _this->_sourcename));
  _CHECK_IO(ReadObject(v, up, read, _this->_name));
  _CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));
  _CHECK_IO(SafeRead(v,read,up, (tPtr)&nsize, sizeof(nsize)));
  for(i = 0;i < nsize; i++){
    _CHECK_IO(ReadObject(v, up, read, o));
    _this->_literals.push_back(o);
  }
  _CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));
  _CHECK_IO(SafeRead(v,read,up, (tPtr)&nsize, sizeof(nsize)));
  for(i = 0; i < nsize; i++){
    SQFunctionParameter param;
    _CHECK_IO(ReadObject(v, up, read, param._name));
    _CHECK_IO(ReadObject(v, up, read, param._type));
    _this->_parameters.push_back(param);
  }
  _CHECK_IO(ReadObject(v, up, read, _this->_returntype));
  _CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));
  _CHECK_IO(SafeRead(v,read,up,(tPtr)&nsize,sizeof(nsize)));
  for(i = 0; i < nsize; i++){
    bool bl;
    _CHECK_IO(SafeRead(v,read,up, (tPtr)&bl, sizeof(bool)));
    _CHECK_IO(ReadObject(v, up, read, o));
    _this->_outervalues.push_back(SQOuterVar(o, bl));
  }
  _CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));
  _CHECK_IO(SafeRead(v,read,up,(tPtr)&nsize, sizeof(nsize)));
  for(i = 0; i < nsize; i++){
    SQLocalVarInfo lvi;
    _CHECK_IO(ReadObject(v, up, read, lvi._name));
    _CHECK_IO(SafeRead(v,read,up, (tPtr)&lvi._pos, sizeof(tU32)));
    _CHECK_IO(SafeRead(v,read,up, (tPtr)&lvi._start_op, sizeof(tU32)));
    _CHECK_IO(SafeRead(v,read,up, (tPtr)&lvi._end_op, sizeof(tU32)));
    _this->_localvarinfos.push_back(lvi);
  }
  _CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));
  _CHECK_IO(SafeRead(v,read,up, (tPtr)&nsize,sizeof(nsize)));
  _this->_lineinfos.resize(nsize);
  _CHECK_IO(SafeRead(v,read,up, (tPtr)&_this->_lineinfos[0], sizeof(SQLineInfo)*nsize));
  _CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));
  _CHECK_IO(SafeRead(v,read,up, (tPtr)&nsize, sizeof(nsize)));
  _this->_instructions.resize(nsize);
  _CHECK_IO(SafeRead(v,read,up, (tPtr)&_this->_instructions[0], sizeof(SQInstruction)*nsize));
  _CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));
  _CHECK_IO(SafeRead(v,read,up, (tPtr)&nsize, sizeof(nsize)));
  for(i = 0; i < nsize; i++){
    o = SQFunctionProto::Create();
    _CHECK_IO(ReadSQFunctionProto(_funcproto(o), v, up, read));
    _this->_functions.push_back(o);
  }
  _CHECK_IO(SafeRead(v,read,up, (tPtr)&_this->_stacksize, sizeof(_this->_stacksize)));
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
