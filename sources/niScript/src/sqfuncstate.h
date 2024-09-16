#ifndef _SQFUNCSTATE_H_
#define _SQFUNCSTATE_H_
///////////////////////////////////
#include "sqvector.h"
#include "sqfuncproto.h"
#include "sqobject.h"
#include "sqopcodes.h"

struct SQFuncState
{
  SQFuncState(SQFunctionProto *func,SQFuncState *parent,iHString* ahspSourceName,sVec2i aSourceLineCol);
  void AddInstruction(SQOpcode _op,int arg0=0,int arg1=0,int arg2=0,int arg3=0,int ext=0){SQInstruction i(_op,arg0,arg1,arg2,arg3,ext);AddInstruction(i);}
  void AddInstruction(const SQInstruction &i);
  void SetIntructionParams(int pos,int arg0,int arg1,int arg2=0,int arg3=0);
  void SetIntructionParam(int pos,int arg,int val);
  SQInstruction& GetInstruction(int pos){return _instructions[pos];}
  void PopInstructions(int size){for(int i=0;i<size;i++)_instructions.pop_back();}
  void SetStackSize(int n);
  void SnoozeOpt(){_optimization=false;}
  int GetCurrentPos(){return (int)(_instructions.size()-1);}
  int GetStringConstant(const SQObjectPtr& cons);
  int GetNumericConstant(const SQInt cons);
  int GetNumericConstant(const SQFloat cons);
  int PushLocalVariable(const SQObjectPtr &name);
  void AddParameter(const SQObjectPtr &name, const SQObjectPtr &typeName);
  void AddOuterValue(const SQObjectPtr &name);
  int GetLocalVariable(const SQObjectPtr &name);
  int GenerateCode();
  int GetStackSize();
  int CalcStackFrameSize();
  void AddLineInfos(sVec2i aSourceLineCol, bool lineop, bool force);
  int AllocStackPos();
  int PushTarget(int n=-1);
  int PopTarget();
  int TopTarget();
  bool IsLocal(unsigned int stkpos);
  void SetTopInstructionOpExt(int opExt);

  void FinalizeFuncProto();

  int GetConstant(SQObjectPtr cons);

  void CleanStack(int stacksize);

  void ResolveBreaks(int ntoresolve);
  void ResolveContinues(int ntoresolve, int targetpos);
  tI32CVec* __stdcall GetBreakTargets() const { return _breaktargets; }
  tI32CVec* __stdcall GetUnresolvedBreaks() const { return _unresolvedbreaks; }
  tI32CVec* __stdcall GetContinueTargets() const { return _continuetargets; }
  tI32CVec* __stdcall GetUnresolvedContinues() const { return _unresolvedcontinues; }

  void SetReturnExp(int aReturnExp);
  int GetReturnExp() const;

  void SetTraps(int aTraps);
  int GetTraps() const;

  int GetNumFunctions() const;
  void AddFunction(SQFuncState* apFuncState);

  SQFunctionProto& proto() const {
    return *_funcproto(_func);
  }

 public:
  SQObjectPtr _func;
  int _returnexp;
  SQLocalVarInfoVec _vlocals;
  SQIntVec _targetstack;
  SQObjectPtrVec _functions;
  astl::vector<SQFunctionParameter> _parameters;
  SQObjectPtr _returntype;
  SQOuterVarVec _outervalues;
  SQInstructionVec _instructions;
  SQLocalVarInfoVec _localvarinfos;
  SQObjectPtr _literals;
  SQInt _nliterals;
  SQLineInfoVec _lineinfos;
  SQFuncState *_parent;
  int _lastline;
  int _traps;
  bool _optimization;

  // contains number of nested exception traps
  Nonnull<tI32CVec> _breaktargets;
  Nonnull<tI32CVec> _unresolvedbreaks;
  Nonnull<tI32CVec> _continuetargets;
  Nonnull<tI32CVec> _unresolvedcontinues;
};

#endif //_SQFUNCSTATE_H_
