/*
  see copyright notice in squirrel.h
*/
#include "stdafx.h"

#include <niLang/Utils/Trace.h>
#include "sqconfig.h"
#include "sqcompiler.h"
#include "sqfuncproto.h"
#include "sqarray.h"
#include "sqtable.h"
#include "sqopcodes.h"
#include "sqfuncstate.h"
#include "sq_hstring.h"
#include "sqstate.h"

#define MAX_FUNC_STACKSIZE (1024)
#define MAX_LITERALS       (1024*1024)

// #define SQ_TRACE_OP_TRANSFORM

SQFuncState::SQFuncState(SQFunctionProto *func, SQFuncState *parent,
                         iHString *ahspSourceName, sVec2i aSourceLineCol)
    : _breaktargets(tI32CVec::Create()),
      _unresolvedbreaks(tI32CVec::Create()),
      _continuetargets(tI32CVec::Create()),
      _unresolvedcontinues(tI32CVec::Create())
{
  _nliterals = 0;
  _literals = SQTable::Create();
  _lastline = 0;
  _optimization = true;
  _func = func;
  _parent = parent;
  _traps = 0;
  _returnexp = 0;
  _funcproto(_func)->_sourcename =
      ni::HStringIsEmpty(ahspSourceName) ? _null_ : ahspSourceName;
  _funcproto(_func)->_sourceline = aSourceLineCol.x;
  _funcproto(_func)->_sourcecol = aSourceLineCol.y;
}

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

void SQFuncState::AddLineInfos(sVec2i aLineCol,bool lineop,bool force)
{
  SQLineInfo li;
  li._line = aLineCol.x;
  li._column = aLineCol.y;
  li._op = (GetCurrentPos()+1);
  if (_lastline!=aLineCol.x || force) {
    if (lineop) {
      AddInstruction(_OP_LINE,0,aLineCol.x);
    }
    _lastline=aLineCol.x;
  }
  if (!_lineinfos.empty() &&
      _lineinfos.back()._line == aLineCol.x &&
      _lineinfos.back()._column == aLineCol.y)
  {
    _lineinfos.back() = li;
  }
  else {
    _lineinfos.push_back(li);
  }
}

void SQFuncState::AddInstruction(const SQInstruction& i)
{
  niLet size = _instructions.size();

  if (size > 0 && (_optimization || i.op == _OP_RETURN)) {
    niLet piIdx = size-1;
    niLet prevInst = _instructions[piIdx];

#ifdef SQ_TRACE_OP_TRANSFORM
  niLet debugOpMutation = [&](ain<SQInstruction> pi) {
    niDebugFmt(("... SQFuncState::AddInstruction(%s): Modified prevInst:\n  From: %s\n    To: %s",
                _GetOpDesc(i.op), SQInstructionToString(prevInst), SQInstructionToString(pi)));
  };
  #define LOG_OP_TRANSFORM() debugOpMutation(pi)
#else
  #define LOG_OP_TRANSFORM()
#endif

    switch (i.op) {
      case _OP_RETURN: {
        if (_parent && i._arg0 != 0xFF && prevInst.op == _OP_CALL && _returnexp < size-1) {
          niLetMut& pi = _instructions[piIdx];
          pi.op = _OP_TAILCALL;
          pi._ext = i._ext;
          LOG_OP_TRANSFORM();
        }
        break;
      }

      case _OP_GET: {
        if (prevInst.op == _OP_LOAD &&
            prevInst._arg0 == i._arg2 &&
            !IsLocal(prevInst._arg0))
        {
          niLetMut& pi = _instructions[piIdx];
          pi.op = _OP_GETK;
          pi._arg2 = (unsigned char)i._arg1;
          pi._arg0 = i._arg0;
          if (i._ext != 0)
            pi._ext = i._ext;
          LOG_OP_TRANSFORM();
          return;
        }
        break;
      }

      case _OP_PREPCALL: {
        if (prevInst.op == _OP_LOAD &&
            prevInst._arg0 == i._arg1 &&
            !IsLocal(prevInst._arg0))
        {
          niLetMut& pi = _instructions[piIdx];
          pi.op = _OP_PREPCALLK;
          pi._arg0 = i._arg0;
          pi._arg2 = i._arg2;
          pi._arg3 = i._arg3;
          if (i._ext != 0)
            pi._ext = i._ext;
          LOG_OP_TRANSFORM();
          return;
        }
        break;
      }

      case _OP_APPENDARRAY: {
        if (prevInst.op == _OP_LOAD &&
            prevInst._arg0 == i._arg1 &&
            !IsLocal(prevInst._arg0))
        {
          niLetMut& pi = _instructions[piIdx];
          pi.op = _OP_APPENDARRAY;
          pi._arg0 = i._arg0;
          pi._arg1 = pi._arg1;
          pi._arg2 = 0xFF;
          pi._ext = i._ext;
          LOG_OP_TRANSFORM();
          return;
        }
        break;
      }

      case _OP_MOVE: {
        if ((prevInst.op == _OP_GET ||
             prevInst.op == _OP_ADD || prevInst.op == _OP_SUB || prevInst.op == _OP_MUL || prevInst.op == _OP_DIV ||
             prevInst.op == _OP_SHIFTL || prevInst.op == _OP_SHIFTR ||
             prevInst.op == _OP_BWOR || prevInst.op == _OP_BWXOR || prevInst.op == _OP_BWAND) &&
            (prevInst._arg0 == i._arg1))
        {
          niLetMut& pi = _instructions[piIdx];
          pi._arg0 = i._arg0;
          pi._ext |= i._ext; // we're keeping the previous opcode, we must preserve its _ext aswell
          _optimization = false;
          LOG_OP_TRANSFORM();
          return;
        }

        if (prevInst.op == _OP_MOVE) {
          niLetMut& pi = _instructions[piIdx];
          pi.op = _OP_DMOVE;
          pi._arg2 = i._arg0;
          pi._arg3 = (unsigned char)i._arg1;
          pi._ext = i._ext;
          LOG_OP_TRANSFORM();
          return;
        }
        break;
      }

      case _OP_ADD: case _OP_SUB: case _OP_MUL: case _OP_DIV:
      case _OP_EQ: case _OP_NE: case _OP_G: case _OP_GE: case _OP_L: case _OP_LE:
      case _OP_SPACESHIP: {
        if (prevInst.op == _OP_LOAD && prevInst._arg0 == i._arg1 && !IsLocal(prevInst._arg0)) {
          niLetMut& pi = _instructions[piIdx];
          pi.op = i.op;
          pi._arg0 = i._arg0;
          pi._arg2 = i._arg2;
          pi._arg3 = 0xFF;
          pi._ext = i._ext;
          LOG_OP_TRANSFORM();
          return;
        }
        break;
      }

      case _OP_LOADNULLS:
      case _OP_LOADNULL: {
        if ((prevInst.op == _OP_LOADNULL && prevInst._arg0 == i._arg0-1) ||
            (prevInst.op == _OP_LOADNULLS && prevInst._arg0+prevInst._arg1 == i._arg0))
        {
          niLetMut& pi = _instructions[piIdx];
          pi.op = _OP_LOADNULLS;
          pi._arg1 = (prevInst.op == _OP_LOADNULL ? 2 : prevInst._arg1+1);
          pi._ext = i._ext;
          LOG_OP_TRANSFORM();
          return;
        }
        break;
      }

      case _OP_LINE: {
        if (prevInst.op == _OP_LINE) {
          _instructions.pop_back();
          _lineinfos.pop_back();
        }
        break;
      }
    }
  }

  _optimization = true;
  _instructions.push_back(i);
}

void SQFuncState::FinalizeFuncProto()
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
      i.op == _OP_PREPCALLK ||
      i.op == _OP_LOAD);
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
