/*
  see copyright notice in squirrel.h
*/
#include "stdafx.h"

#include "sqconfig.h"
#include "sqcompiler.h"
#include "sqfuncproto.h"
#include "sqarray.h"
#include "sqtable.h"
#include "sqopcodes.h"
#include "sqfuncstate.h"

#define MAX_FUNC_STACKSIZE (1024)
#define MAX_LITERALS       (1024*1024)

#ifdef _DEBUG_DUMP
static const achar* const _InstrDesc[]={
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
  return op < niCountOf(_InstrDesc) ? _InstrDesc[op] : "_OP_???";
}
#endif

SQFuncState::SQFuncState(SQFunctionProto *func,SQFuncState *parent)
    : _breaktargets(tI32CVec::Create())
    , _unresolvedbreaks(tI32CVec::Create())
    , _continuetargets(tI32CVec::Create())
    , _unresolvedcontinues(tI32CVec::Create())
{
  _nliterals = 0;
  _literals = SQTable::Create();
  _lastline = 0;
  _optimization = true;
  _func = func;
  _parent = parent;
  _traps = 0;
  _returnexp = 0;
}

#ifdef _DEBUG_DUMP
static cString _LiteralToString(const SQObjectPtr& obj) {
  switch (_sqtype(obj)) {
    case OT_STRING:
      return niFmt("\"%s\"",_stringval(obj));
    case OT_IUNKNOWN:
      return niFmt("{IUnknown:%p}",(tIntPtr)_iunknown(obj));
    default:
      return niFmt("{%s:%s}",sqa_gettypestring(obj._var.mType),(Var&)obj._var);
  }
}

void SQFuncState::Dump()
{
  unsigned int n=0,i;
  SQFunctionProto *func=_funcproto(_func);
  cString o;
  o << niFmt(_A("SQInstruction sizeof %d\n"),sizeof(SQInstruction));
  o << niFmt(_A("SQObject sizeof %d\n"),sizeof(SQObject));
  o << niFmt(_A("--------------------------------------------------------------------\n"));
  o << niFmt(_A("*****FUNCTION [%s]\n"),_sqtype(func->_name)==OT_STRING?_stringval(func->_name):_A("unknown"));
  o << niFmt(_A("-----LITERALS\n"));
  {
    SQObjectPtr refidx,key,val,itr;
    SQObjectPtrVec templiterals;
    templiterals.resize(_nliterals);
    while (_table(_literals)->Next(refidx,key,val,itr)) {
      templiterals[_int(val)] = key;
      refidx = itr;
    }
    for(i=0;i<templiterals.size();i++){
      o << niFmt(_A("[%d] "),n);
      o << _LiteralToString(templiterals[i]);
      o << niFmt(_A("\n"));
      n++;
    }
  }
  o << niFmt(_A("-----PARAMS\n"));
  n=0;
  for(i=0;i<_parameters.size();i++){
    o << niFmt(_A("[%d] "),n);
    o << _LiteralToString(_parameters[i]._type);
    o << _LiteralToString(_parameters[i]._name);
    o << niFmt(_A("\n"));
    n++;
  }
  o << "returntype: ";
  o << _LiteralToString(_returntype);
  o << niFmt(_A("\n"));
  o << niFmt(_A("-----LOCALS\n"));
  for(i=0;i<func->_localvarinfos.size();i++){
    SQLocalVarInfo lvi=func->_localvarinfos[i];
    o << niFmt(_A("[%d] %s \t%d %d\n"),lvi._pos,_stringval(lvi._name),lvi._start_op,lvi._end_op);
    n++;
  }
  o << niFmt(_A("-----LINE INFO\n"));
  for(i=0;i<_lineinfos.size();i++){
    SQLineInfo li=_lineinfos[i];
    o << niFmt(_A("op [%d] line [%d] \n"),li._op,li._line);
    n++;
  }
  o << niFmt(_A("-----dump\n"));
  n=0;
  for(i=0;i<_instructions.size();i++){
    SQInstruction &inst=_instructions[i];
    if (inst.op==_OP_LOAD || inst.op==_OP_PREPCALLK || inst.op==_OP_GETK
        || ((inst.op==_OP_ADD || inst.op==_OP_SUB || inst.op==_OP_MUL || inst.op==_OP_DIV)
            && inst._arg3==0xFF))
    {
      o << niFmt(_A("[%03d] %20s %d "),n,_GetOpDesc(inst.op),inst._arg0);
      if (inst._arg1==0xFFFF) {
        o << niFmt(_A("null"));
      }
      else {
        SQObjectPtr val,key,refo,ito;
        while (_table(_literals)->Next(refo,key,val,ito)
               && (_int(val) != inst._arg1))
        {
          refo = ito;
        }
        o << _LiteralToString(key);
      }
      o << niFmt(_A(" %d %d \n"),inst._arg2,inst._arg3);
    }
    else {
      o << niFmt(_A("[%03d] %20s %d %d %d %d\n"),n,_GetOpDesc(inst.op),inst._arg0,inst._arg1,inst._arg2,inst._arg3);
    }
    n++;
  }
  o << niFmt(_A("-----\n"));
  o << niFmt(_A("stack size[%d]\n"),func->_stacksize);
  o << niFmt("--------------------------------------------------------------------\n\n");
  niDebugFmt((o.Chars()));
}
#endif

int SQFuncState::GetStringConstant(const SQObjectPtr& cons) {
  return GetConstant(cons);
}
int SQFuncState::GetNumericConstant(const SQInt cons) {
  return GetConstant(cons);
}
int SQFuncState::GetNumericConstant(const SQFloat cons) {
  return GetConstant(cons);
}
int SQFuncState::GetConstant(SQObjectPtr cons) {
  SQObjectPtr val;
  if (!_table(_literals)->Get(cons,val)) {
    val=_nliterals;
    _table(_literals)->NewSlot(cons,val);
    _nliterals++;
    niAssert(_nliterals < MAX_LITERALS);
  }
  return _int(val);
}

void SQFuncState::SetIntructionParams(int pos,int arg0,int arg1,int arg2,int arg3)
{
  _instructions[pos]._arg0=*((unsigned int *)&arg0);
  _instructions[pos]._arg1=*((unsigned int *)&arg1);
  _instructions[pos]._arg2=*((unsigned int *)&arg2);
  _instructions[pos]._arg3=*((unsigned int *)&arg3);
}

void SQFuncState::SetIntructionParam(int pos,int arg,int val)
{
  switch(arg){
    case 0:_instructions[pos]._arg0=*((unsigned int *)&val);break;
    case 1:_instructions[pos]._arg1=*((unsigned int *)&val);break;
    case 2:_instructions[pos]._arg2=*((unsigned int *)&val);break;
    case 3:_instructions[pos]._arg3=*((unsigned int *)&val);break;
  };
}

int SQFuncState::AllocStackPos()
{
  int npos=_vlocals.size();
  _vlocals.push_back(SQLocalVarInfo());
  if (_vlocals.size() > (tSize)_funcproto(_func)->_stacksize) {
    niAssert(_vlocals.size() < MAX_FUNC_STACKSIZE);
    _funcproto(_func)->_stacksize = _vlocals.size();
  }
  return npos;
}

int SQFuncState::PushTarget(int n)
{
  if(n!=-1){
    niAssert(n < (int)_vlocals.size());
    _targetstack.push_back(n);
    return n;
  }
  n=AllocStackPos();
  niAssert(n < (int)_vlocals.size());
  _targetstack.push_back(n);
  return n;
}

int SQFuncState::TopTarget(){
  niAssert(!_targetstack.empty());
  return _targetstack.back();
}
int SQFuncState::PopTarget()
{
  niAssert(!_targetstack.empty());
  const int npos = _targetstack.back();
  if (npos < (int)_vlocals.size()) {
    SQLocalVarInfo t = _vlocals[_targetstack.back()];
    if (_sqtype(t._name) == OT_NULL) {
      _vlocals.pop_back();
    }
  }
  _targetstack.pop_back();
  return npos;
}

int SQFuncState::GetStackSize()
{
  return _vlocals.size();
}

void SQFuncState::SetStackSize(int n)
{
  int size=_vlocals.size();
  while(size>n){
    size--;
    SQLocalVarInfo lvi=_vlocals.back();
    if(_sqtype(lvi._name)!=OT_NULL){
      lvi._end_op=GetCurrentPos();
      _localvarinfos.push_back(lvi);
    }
    _vlocals.pop_back();
  }
}

bool SQFuncState::IsLocal(unsigned int stkpos)
{
  if(stkpos>=_vlocals.size())return false;
  else if(_sqtype(_vlocals[stkpos]._name)!=OT_NULL)return true;
  return false;
}

int SQFuncState::PushLocalVariable(const SQObjectPtr &name)
{
  int pos=_vlocals.size();
  SQLocalVarInfo lvi;
  lvi._name=name;
  lvi._start_op=GetCurrentPos()+1;
  lvi._pos=_vlocals.size();
  _vlocals.push_back(lvi);
  if (_vlocals.size() > (tSize)_funcproto(_func)->_stacksize)
    _funcproto(_func)->_stacksize = _vlocals.size();

  return pos;
}

int SQFuncState::GetLocalVariable(const SQObjectPtr &name)
{
  int locals=_vlocals.size();
  while(locals>=1){
    if (_sqtype(_vlocals[locals-1]._name)==OT_STRING &&
        _stringeq(_vlocals[locals-1]._name,name))
    {
      return locals-1;
    }
    locals--;
  }
  return -1;
}

void SQFuncState::AddOuterValue(const SQObjectPtr &name)
{
  //     niDebugFmt((_A("ADDING OUTER: %s to %s"),
  //                  _stringval(name),
  //                  _stringval(_funcproto(this->_func)->_name)));

  if (GetLocalVariable(name) != -1) {
    //         niDebugFmt((_A("ALREADY EXISTS: %s"),_stringval(name)));
    return; // already added
  }

  AddParameter(name,_null_);
  int pos=-1;
  if (_parent) {
    pos = _parent->GetLocalVariable(name);
  }
  if (pos != -1) {
    _outervalues.push_back(SQOuterVar(SQObjectPtr(SQInt(pos)),true)); //local
  }
  else {
    _outervalues.push_back(SQOuterVar(name,false)); //global
  }
}

void SQFuncState::AddParameter(const SQObjectPtr &name, const SQObjectPtr &typeName)
{
  PushLocalVariable(name);
  SQFunctionParameter param;
  param._name = name;
  param._type = typeName;
  _parameters.push_back(param);
}

void SQFuncState::AddLineInfos(int line,bool lineop,bool force)
{
  if (_lastline!=line || force) {
    SQLineInfo li;
    li._line = line;
    li._op = (GetCurrentPos()+1);
    if (lineop)
      AddInstruction(_OP_LINE,0,line);
    _lineinfos.push_back(li);
    _lastline=line;
  }
}

void SQFuncState::AddInstruction(SQInstruction &i)
{
  int size = _instructions.size();
  if(size > 0 && (_optimization || i.op == _OP_RETURN))
  { //simple optimizer
    SQInstruction &pi = _instructions[size-1];//previous intruction
    switch(i.op) {
      case _OP_RETURN:
        if ( _parent && i._arg0 != 0xFF && pi.op == _OP_CALL && _returnexp < size-1) {
          pi.op = _OP_TAILCALL;
          pi._ext = i._ext;
        }
        break;
      case _OP_GET:
        if( pi.op == _OP_LOAD && pi._arg0 == i._arg2 && (!IsLocal(pi._arg0))){
          pi._arg2 = (unsigned char)i._arg1;
          pi.op = _OP_GETK;
          pi._arg0 = i._arg0;
          if (i._ext != 0)
            pi._ext = i._ext;
          return;
        }
        break;
      case _OP_PREPCALL:
        if( pi.op == _OP_LOAD && pi._arg0 == i._arg1 && (!IsLocal(pi._arg0))){
          pi.op = _OP_PREPCALLK;
          pi._arg0 = i._arg0;
          pi._arg2 = i._arg2;
          pi._arg3 = i._arg3;
          if (i._ext != 0)
            pi._ext = i._ext;
          return;
        }
        break;
      case _OP_APPENDARRAY:
        if(pi.op == _OP_LOAD && pi._arg0 == i._arg1 && (!IsLocal(pi._arg0))){
          pi.op = _OP_APPENDARRAY;
          pi._arg0 = i._arg0;
          pi._arg1 = pi._arg1;
          pi._arg2 = 0xFF;
          pi._ext = i._ext;
          return;
        }
        break;
      case _OP_MOVE:
        if((pi.op == _OP_GET || pi.op == _OP_ADD || pi.op == _OP_SUB
            || pi.op == _OP_MUL || pi.op == _OP_DIV || pi.op == _OP_SHIFTL
            || pi.op == _OP_SHIFTR || pi.op == _OP_BWOR || pi.op == _OP_BWXOR
            || pi.op == _OP_BWAND) && (pi._arg0 == i._arg1))
        {
          pi._arg0 = i._arg0;
          pi._ext = i._ext;
          _optimization = false;
          return;
        }

        if(pi.op == _OP_MOVE)
        {
          pi.op = _OP_DMOVE;
          pi._arg2 = i._arg0;
          pi._arg3 = (unsigned char)i._arg1;
          pi._ext = i._ext;
          return;
        }
        break;

      case _OP_ADD:case _OP_SUB:case _OP_MUL:case _OP_DIV:
      case _OP_EQ:case _OP_NE:case _OP_G:case _OP_GE:case _OP_L:case _OP_LE:
      case _OP_SPACESHIP:
        if(pi.op == _OP_LOAD && pi._arg0 == i._arg1 && (!IsLocal(pi._arg0) ))
        {
          pi.op = i.op;
          pi._arg0 = i._arg0;
          pi._arg2 = i._arg2;
          pi._arg3 = 0xFF;
          pi._ext = i._ext;
          return;
        }
        break;
      case _OP_LOADNULLS:
      case _OP_LOADNULL:
        if((pi.op == _OP_LOADNULL && pi._arg0 == i._arg0-1) ||
           (pi.op == _OP_LOADNULLS && pi._arg0+pi._arg1 == i._arg0)
           ) {

          pi._arg1 = pi.op == _OP_LOADNULL?2:pi._arg1+1;
          pi.op = _OP_LOADNULLS;
          pi._ext = i._ext;
          return;
        }
        break;
      case _OP_LINE:
        if(pi.op == _OP_LINE) {
          _instructions.pop_back();
          _lineinfos.pop_back();
        }
        break;
    }
  }
  _optimization = true;
  _instructions.push_back(i);
}

void SQFuncState::Invalidate()
{
  SQFunctionProto *f=_funcproto(_func);
  f->_literals.resize(_nliterals);
  SQObjectPtr refidx,key,val,nitr;
  while (_table(_literals)->Next(refidx,key,val,nitr)) {
    f->_literals[_int(val)] = key;
    refidx = nitr;
  }
  f->_functions.resize(_functions.size());
  f->_functions = _functions;
  f->_parameters.resize(_parameters.size());
  f->_parameters = _parameters;
  f->_returntype = _returntype;
  f->_outervalues.resize(_outervalues.size());
  f->_outervalues = _outervalues;
  f->_instructions.resize(_instructions.size());
  f->_instructions = _instructions;
  f->_localvarinfos.resize(_localvarinfos.size());
  f->_localvarinfos = _localvarinfos;
  f->_lineinfos.resize(_lineinfos.size());
  f->_lineinfos = _lineinfos;
}

void SQFuncState::SetTopInstructionOpExt(int opExt) {
  int size = _instructions.size();
  niAssert(size > 0);
  SQInstruction& i = _instructions[size-1];
  niAssert(
      i.op == _OP_DELETE ||
      i.op == _OP_NEWSLOT ||
      i.op == _OP_INC ||
      i.op == _OP_PINC ||
      i.op == _OP_DEC ||
      i.op == _OP_PDEC ||
      i.op == _OP_MINUSEQ ||
      i.op == _OP_PLUSEQ ||
      i.op == _OP_MULEQ ||
      i.op == _OP_DIVEQ ||
      i.op == _OP_SHIFTLEQ ||
      i.op == _OP_SHIFTREQ ||
      i.op == _OP_USHIFTREQ ||
      i.op == _OP_BWANDEQ ||
      i.op == _OP_BWOREQ ||
      i.op == _OP_BWXOREQ ||
      i.op == _OP_MODULOEQ ||
      i.op == _OP_PLUSEQ ||
      i.op == _OP_SET ||
      i.op == _OP_GET ||
      i.op == _OP_GETK ||
      i.op == _OP_PREPCALL ||
      i.op == _OP_PREPCALLK);
  i._ext = opExt;
}

void SQFuncState::CleanStack(int stacksize)
{
  if (this->GetStackSize() != stacksize)
    this->SetStackSize(stacksize);
}

void SQFuncState::ResolveBreaks(int ntoresolve)
{
  while (ntoresolve > 0) {
    int pos = this->_unresolvedbreaks->back();
    this->_unresolvedbreaks->RemoveLast();
    //set the jmp instruction
    this->SetIntructionParams(pos, 0, this->GetCurrentPos() - pos, 0);
    ntoresolve--;
  }
}

void SQFuncState::ResolveContinues(int ntoresolve, int targetpos)
{
  while (ntoresolve > 0) {
    int pos = this->_unresolvedcontinues->back();
    this->_unresolvedcontinues->RemoveLast();
    //set the jmp instruction
    this->SetIntructionParams(pos, 0, targetpos - pos, 0);
    ntoresolve--;
  }
}

void SQFuncState::SetReturnExp(int aReturnExp) {
  _returnexp = aReturnExp;
}
int SQFuncState::GetReturnExp() const {
  return _returnexp;
}

void SQFuncState::SetTraps(int aTraps) {
  _traps = aTraps;
}
int SQFuncState::GetTraps() const {
  return _traps;
}

int SQFuncState::GetNumFunctions() const {
  return _functions.size();
}
void SQFuncState::AddFunction(SQFuncState* apFuncState) {
  _functions.push_back(apFuncState->_func);
}

void __stdcall SQFuncState::SetName(iHString* ahspName) {
  _funcproto(_func)->_name = ni::HStringIsEmpty(ahspName) ? _null_ : ahspName;
}
iHString* __stdcall SQFuncState::GetName() const {
  if (!sq_isstring(_funcproto(_func)->_name))
    return NULL;
  return _stringhval(_funcproto(_func)->_name);
}

void __stdcall SQFuncState::SetSourceName(iHString* ahspSourceName) {
  _funcproto(_func)->_sourcename = ni::HStringIsEmpty(ahspSourceName) ? _null_ : ahspSourceName;
}
iHString* __stdcall SQFuncState::GetSourceName() const {
  if (!sq_isstring(_funcproto(_func)->_sourcename))
    return NULL;
  return _stringhval(_funcproto(_func)->_sourcename);
}

const SQObjectPtr& SQFuncState::GetFunctionObject() const {
  return _func;
}
