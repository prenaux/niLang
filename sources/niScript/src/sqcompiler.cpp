#include "stdafx.h"

#include <stdarg.h>
#include "sqconfig.h"
#include "sqopcodes.h"
#include "sqfuncproto.h"
#include "sqcompiler.h"
#include "sqfuncstate.h"
#include "sqlexer.h"
#include "sqvm.h"
#include "sq_hstring.h"

#define DEREF_NO_DEREF  -1
#define DEREF_FIELD   -2

struct ExpState {
  ExpState()
  {
    _deref = DEREF_NO_DEREF;
    _delete = false;
    _opExt = 0;
  }
  bool _delete;
  int _deref;
  int _opExt;
};

SQ_VECTOR_TYPEDEF(ExpState,ExpStateVec);

#define _exst (_expstates.back())

#define BEGIN_BREAKBLE_BLOCK()                                \
  int __nbreaks__ = _fs->GetUnresolvedBreaks()->size();       \
  int __ncontinues__ = _fs->GetUnresolvedContinues()->size(); \
  _fs->GetBreakTargets()->push_back(0);                       \
  _fs->GetContinueTargets()->push_back(0);

#define END_BREAKBLE_BLOCK(continue_target) {                           \
    __nbreaks__ = _fs->GetUnresolvedBreaks()->size()-__nbreaks__;       \
    __ncontinues__ = _fs->GetUnresolvedContinues()->size()-__ncontinues__; \
    if (__ncontinues__ > 0)                                             \
      _fs->ResolveContinues(__ncontinues__,continue_target);            \
    if (__nbreaks__ > 0)                                                \
      _fs->ResolveBreaks(__nbreaks__);                                  \
    _fs->GetBreakTargets()->RemoveLast();                               \
    _fs->GetContinueTargets()->RemoveLast();                            \
  }

#define COMPILER_ERROR(ERRMSG)                                          \
  return aErrors.CompilerError(_lex.GetLastTokenLineCol(),ERRMSG)

#define IS_TK_COMMA(TK) ((TK) == ',' || (TK) == TK_SEXP_START_COMMA)

struct SQCompiler {
  SQCompiler(
    SQLEXREADFUNC rg,
    ni::tPtr up,
    iHString* sourcename)
  {
    _sourcename = sourcename;
    _lex.Init(niHStr(_sourcename), rg, up);
  }

  eCompileResult Lex(sCompileErrors& aErrors) {
    _token = -1;
    COMPILE_CHECK(_lex.Lex(aErrors,&_token));
    niAssert(_token != -1);
    return eCompileResult_OK;
  }

  void PushExpState() {
    _expstates.push_back(ExpState());
  }

  void PopExpState(ExpState* apExpState) {
    ExpState ret = _expstates.back();
    _expstates.pop_back();
    if (apExpState) *apExpState = ret;
  }

  eCompileResult _Expect(sCompileErrors& aErrors, int* toks, int num, int* ptheTok, SQObjectPtr* pRet)
  {
    SQObjectPtr ret;
    int thetok = -1;
    int expectedI;
    for (expectedI = 0; expectedI < num; ++expectedI)
    {
      if (_token == toks[expectedI])
      {
        thetok = toks[expectedI];
        break;
      }
    }
    if (expectedI == num) {
      cString str;
      for (int i = 0; i < num; ++i) {
        str += _stringval(_lex.Tok2Str(toks[i]));
        if (i+1 != num)
          str += _A(" or ");
      }
      COMPILER_ERROR(niFmt("Expected: %s got %d (%s)", str.Chars(), _token, _stringval(_lex.Tok2Str(_token))));
    }
    if (ptheTok) *ptheTok = thetok;
    switch(thetok)
    {
      case TK_IDENTIFIER:
        ret = _H(_lex._svalue);
        break;
      case TK_STRING_LITERAL:
        ret = _H(cString(_lex._svalue, _lex._longstr.size()-1));
        break;
      case TK_INTEGER:
        ret = _lex._nvalue;
        break;
      case TK_FLOAT:
        ret = _lex._fvalue;
        break;
    }
    if (pRet) *pRet = ret;
    return eCompileResult_OK;
  }

  eCompileResult Expect(sCompileErrors& aErrors, int tok, SQObjectPtr* pRet) {
    COMPILE_CHECK(_Expect(aErrors,&tok,1,NULL,pRet));
    return Lex(aErrors);
  }

  eCompileResult ExpectAssignment(sCompileErrors& aErrors) {
    int thetok;
    int toks[] = { '=', TK_NEWSLOT };
    COMPILE_CHECK(_Expect(aErrors,toks,niCountOf(toks),&thetok,NULL));
    return Lex(aErrors);
  }

  eCompileResult ExpectFuncArgList(sCompileErrors& aErrors, bool& hasArgList) {
    int thetok;
    int toks[] = { '(', TK_RETURN, '{' };
    COMPILE_CHECK(_Expect(aErrors,toks,niCountOf(toks),&thetok,NULL));
    hasArgList = (thetok == '(');
    if (hasArgList) {
      return Lex(aErrors);
    }
    else {
      return eCompileResult_OK;
    }
  }

  eCompileResult OpExt(sCompileErrors& aErrors, int& f) {
    f = 0;
    niLoop(i,2) {
      if (_token == '?') {
        f |= _OPEXT_GET_SAFE;
        COMPILE_CHECK(Lex(aErrors));
        continue;
      }
      break;
    }
    return eCompileResult_OK;
  }

  bool IsEndOfStatement()
  {
    return (_lex._prevtoken == '\n' ||
            (_token == SQUIRREL_EOB) ||
            (_token == '}') ||
            (_token == ';'));
  }

  eCompileResult OptionalSemicolon(sCompileErrors& aErrors)
  {
    if (_token == ';') {
      COMPILE_CHECK(Lex(aErrors));
      return eCompileResult_OK;
    }
    if (!IsEndOfStatement()) {
      COMPILER_ERROR("end of statement expected (; or lf)");
    }
    return eCompileResult_OK;
  }

  bool getGenerateLineInfo() {
    // TODO: This used to be a vm parameter but the compiler is not
    // separated. Generating line info is mainly useful to build an
    // interactive debugger.
    return _lex._compilerCommands.debug;
  }

  tBool Compile(sCompileErrors& aErrors, SQObjectPtr &o, int* apPos)
  {
    if (COMPILE_FAILED(Lex(aErrors))) {
      return eFalse;
    }

    SQFuncState funcstate(
      SQFunctionProto::Create(), NULL,
      _sourcename,
      _lex.GetLastTokenLineCol());
    _fs = &funcstate;
    _fs->proto().SetName(_HC(compilecontext));
    _fs->AddParameter(_HC(this),_null_);
    int stacksize = _fs->GetStackSize();
    while (_token > 0) {
      if (COMPILE_FAILED(Statement(aErrors,apPos))) {
        return eFalse;
      }
      if (_lex._prevtoken != '}') {
        if (COMPILE_FAILED(OptionalSemicolon(aErrors)))
          return eFalse;
      }
    }
    _fs->CleanStack(stacksize);
    _fs->AddLineInfos(
      _lex.GetLastTokenLineCol(),
      getGenerateLineInfo(), true);
    _fs->AddInstruction(_OP_RETURN, 0xFF);
    _fs->SetStackSize(0);
    _fs->FinalizeFuncProto();
    o = _fs->_func;
    return eTrue;
  }

  eCompileResult Statements(sCompileErrors& aErrors, int* apPos)
  {
    while (_token != '}' && _token != TK_DEFAULT && _token != TK_CASE) {
      COMPILE_CHECK(Statement(aErrors,apPos));
      if (_lex._prevtoken != '}' && _lex._prevtoken != ';') {
        COMPILE_CHECK(OptionalSemicolon(aErrors));
      }
    }
    return eCompileResult_OK;
  }

  eCompileResult Statement(sCompileErrors& aErrors, int* apPos)
  {
    _fs->AddLineInfos(_lex.GetLastTokenLineCol(), getGenerateLineInfo(), false);
    switch(_token){
      case ';': {
        COMPILE_CHECK(Lex(aErrors));
        break;
      }
      case TK_IF: {
        COMPILE_CHECK(IfStatement(aErrors,apPos));
        break;
      }
      case TK_WHILE: {
        COMPILE_CHECK(WhileStatement(aErrors,apPos));
        break;
      }
      case TK_DO: {
        COMPILE_CHECK(DoWhileStatement(aErrors,apPos));
        break;
      }
      case TK_FOR: {
        COMPILE_CHECK(ForStatement(aErrors,apPos));
        break;
      }
      case TK_FOREACH: {
        COMPILE_CHECK(ForEachStatement(aErrors,apPos));
        break;
      }
      case TK_SWITCH: {
        COMPILE_CHECK(SwitchStatement(aErrors,apPos));
        break;
      }
      case TK_LOCAL: {
        COMPILE_CHECK(LocalDeclStatement(aErrors,apPos));
        break;
      }
      case TK_RETURN: {
        COMPILE_CHECK(Lex(aErrors));
        if (!IsEndOfStatement()) {
          int retexp = _fs->GetCurrentPos()+1;
          COMPILE_CHECK(CommaExpr(aErrors,apPos));
          if (_fs->GetTraps() > 0)
            _fs->AddInstruction(_OP_POPTRAP, _fs->GetTraps(), 0);
          _fs->SetReturnExp(retexp);
          _fs->AddInstruction(_OP_RETURN, 1, _fs->PopTarget());
        }
        else {
          if (_fs->GetTraps() > 0)
            _fs->AddInstruction(_OP_POPTRAP, _fs->GetTraps() ,0);
          _fs->SetReturnExp(-1);
          _fs->AddInstruction(_OP_RETURN, 0xFF);
        }
        break;
      }
      case TK_BREAK:
        if (_fs->GetBreakTargets()->size() <= 0) {
          COMPILER_ERROR("'break' has to be in a loop block");
        }
        if (_fs->GetBreakTargets()->back() > 0){
          _fs->AddInstruction(_OP_POPTRAP, _fs->GetBreakTargets()->back(), 0);
        }
        _fs->AddInstruction(_OP_JMP, 0, -1234);
        _fs->GetUnresolvedBreaks()->push_back(_fs->GetCurrentPos());
        COMPILE_CHECK(Lex(aErrors));
        break;
      case TK_CONTINUE:
        if (_fs->GetContinueTargets()->size() <= 0) {
          COMPILER_ERROR("'continue' has to be in a loop block");
        }
        if(_fs->GetContinueTargets()->back() > 0) {
          _fs->AddInstruction(_OP_POPTRAP, _fs->GetContinueTargets()->back(), 0);
        }
        _fs->AddInstruction(_OP_JMP, 0, -1234);
        _fs->GetUnresolvedContinues()->push_back(_fs->GetCurrentPos());
        COMPILE_CHECK(Lex(aErrors));
        break;
      case TK_FUNCTION:
        COMPILE_CHECK(FunctionStatement(aErrors,apPos));
        break;
      case '{':{
        int stacksize = _fs->GetStackSize();
        COMPILE_CHECK(Lex(aErrors));
        COMPILE_CHECK(Statements(aErrors,apPos));
        COMPILE_CHECK(Expect(aErrors,'}',NULL));
        _fs->SetStackSize(stacksize);
      }
        break;
      case TK_TRY:
        COMPILE_CHECK(TryCatchStatement(aErrors,apPos));
        break;
      case TK_THROW:
        COMPILE_CHECK(Lex(aErrors));
        COMPILE_CHECK(CommaExpr(aErrors,apPos));
        _fs->AddInstruction(_OP_THROW, _fs->PopTarget());
        break;
      case TK_THROW_SILENT:
        COMPILE_CHECK(Lex(aErrors));
        COMPILE_CHECK(CommaExpr(aErrors,apPos));
        _fs->AddInstruction(_OP_THROW_SILENT, _fs->PopTarget());
        break;
      default:
        COMPILE_CHECK(CommaExpr(aErrors,apPos));
        _fs->PopTarget();
        break;
    }
    _fs->SnoozeOpt();
    return eCompileResult_OK;
  }

  void EmitDerefOp(SQOpcode op)
  {
    int val = _fs->PopTarget();
    int key = _fs->PopTarget();
    int src = _fs->PopTarget();
    _fs->AddInstruction(op,_fs->PushTarget(),src,key,val);
  }

  void Emit2ArgsOP(SQOpcode op, int p3 = 0)
  {
    int p2 = _fs->PopTarget(); //src in OP_GET
    int p1 = _fs->PopTarget(); //key in OP_GET
    _fs->AddInstruction(op,_fs->PushTarget(), p1, p2, p3);
  }

  eCompileResult CommaExpr(sCompileErrors& aErrors, int* apPos)
  {
    COMPILE_CHECK(Expression(aErrors,NULL,apPos));
    while (IS_TK_COMMA(_token)) {
      _fs->PopTarget();
      COMPILE_CHECK(Lex(aErrors));
      COMPILE_CHECK(CommaExpr(aErrors,apPos));
    }
    return eCompileResult_OK;
  }

  eCompileResult Expression(sCompileErrors& aErrors, ExpState* apExpState, int* apPos)
  {
    PushExpState();
    COMPILE_CHECK(LogicalOrExp(aErrors, apPos));
    switch(_token)  {
      case '=':
      case TK_NEWSLOT:
      case TK_MINUSEQ:
      case TK_PLUSEQ:
      case TK_DIVEQ:
      case TK_MULEQ:
      case TK_MODULOEQ:
      case TK_SHIFTLEQ:
      case TK_SHIFTREQ:
      case TK_USHIFTREQ:
      case TK_BWANDEQ:
      case TK_BWOREQ:
      case TK_BWXOREQ:
        {
          int op = _token;
          int ds = _exst._deref;
          if (ds == DEREF_NO_DEREF) {
            COMPILER_ERROR("can't assign expression");
          }
          COMPILE_CHECK(Lex(aErrors));
          COMPILE_CHECK(Expression(aErrors,NULL,apPos));

          switch(op){
            case TK_NEWSLOT:
              if (ds == DEREF_FIELD) {
                EmitDerefOp(_OP_NEWSLOT);
              }
              else {
                //if _derefstate != DEREF_NO_DEREF && DEREF_FIELD so is the index of a local
                COMPILER_ERROR("can't 'create' a local slot");
              }
              break;
            case '=': //ASSIGN
              if (ds == DEREF_FIELD) {
                EmitDerefOp(_OP_SET);
              }
              else {//if _derefstate != DEREF_NO_DEREF && DEREF_FIELD so is the index of a local
                int p2 = _fs->PopTarget(); //src in OP_GET
                int p1 = _fs->TopTarget(); //key in OP_GET
                _fs->AddInstruction(_OP_MOVE, p1, p2);
              }
              break;

              // if (ds == DEREF_FILED) {...} else if _derefstate != DEREF_NO_DEREF && DEREF_FIELD so is the index of a local {...}
            case TK_MINUSEQ:
              if(ds == DEREF_FIELD) { EmitDerefOp(_OP_MINUSEQ); }
              else          { Emit2ArgsOP(_OP_MINUSEQ, -1); }
              break;
            case TK_PLUSEQ:
              if(ds == DEREF_FIELD) { EmitDerefOp(_OP_PLUSEQ);  }
              else          { Emit2ArgsOP(_OP_PLUSEQ, -1);  }
              break;
            case TK_MULEQ:
              if(ds == DEREF_FIELD) { EmitDerefOp(_OP_MULEQ); }
              else          { Emit2ArgsOP(_OP_MULEQ, -1); }
              break;
            case TK_MODULOEQ:
              if(ds == DEREF_FIELD) { EmitDerefOp(_OP_MODULOEQ);  }
              else          { Emit2ArgsOP(_OP_MODULOEQ, -1);  }
              break;
            case TK_DIVEQ:
              if(ds == DEREF_FIELD) { EmitDerefOp(_OP_DIVEQ); }
              else          { Emit2ArgsOP(_OP_DIVEQ, -1); }
              break;
            case TK_SHIFTLEQ:
              if(ds == DEREF_FIELD) { EmitDerefOp(_OP_SHIFTLEQ);  }
              else          { Emit2ArgsOP(_OP_SHIFTLEQ, -1);  }
              break;
            case TK_SHIFTREQ:
              if(ds == DEREF_FIELD) { EmitDerefOp(_OP_SHIFTREQ);  }
              else          { Emit2ArgsOP(_OP_SHIFTREQ, -1);  }
              break;
            case TK_USHIFTREQ:
              if(ds == DEREF_FIELD) { EmitDerefOp(_OP_USHIFTREQ); }
              else          { Emit2ArgsOP(_OP_USHIFTREQ, -1); }
              break;
            case TK_BWANDEQ:
              if(ds == DEREF_FIELD) { EmitDerefOp(_OP_BWANDEQ); }
              else          { Emit2ArgsOP(_OP_BWANDEQ, -1); }
              break;
            case TK_BWOREQ:
              if(ds == DEREF_FIELD) { EmitDerefOp(_OP_BWOREQ);  }
              else          { Emit2ArgsOP(_OP_BWOREQ, -1);  }
              break;
            case TK_BWXOREQ:
              if(ds == DEREF_FIELD) { EmitDerefOp(_OP_BWXOREQ); }
              else          { Emit2ArgsOP(_OP_BWXOREQ, -1); }
              break;
          }

          if (_exst._opExt) {
            _fs->SetTopInstructionOpExt(_exst._opExt);
            _exst._opExt = 0;
          }
        }
        break;
      case '?': {
        COMPILE_CHECK(Lex(aErrors));
        _fs->AddInstruction(_OP_JZ, _fs->PopTarget());
        int jzpos = _fs->GetCurrentPos();
        int trg = _fs->PushTarget();
        COMPILE_CHECK(Expression(aErrors,NULL,apPos));
        int first_exp = _fs->PopTarget();
        if(trg != first_exp) _fs->AddInstruction(_OP_MOVE, trg, first_exp);
        int endfirstexp = _fs->GetCurrentPos();
        _fs->AddInstruction(_OP_JMP, 0, 0);
        COMPILE_CHECK(Expect(aErrors,':',NULL));
        int jmppos = _fs->GetCurrentPos();
        COMPILE_CHECK(Expression(aErrors,NULL,apPos));
        int second_exp = _fs->PopTarget();
        if(trg != second_exp) _fs->AddInstruction(_OP_MOVE, trg, second_exp);
        _fs->SetIntructionParam(jmppos, 1, _fs->GetCurrentPos() - jmppos);
        _fs->SetIntructionParam(jzpos, 1, endfirstexp - jzpos + 1);
        _fs->SnoozeOpt();
      }
        break;
    }
    PopExpState(apExpState);
    return eCompileResult_OK;
  }

  eCompileResult BIN_EXP(sCompileErrors& aErrors, SQOpcode op, int* apPos,
                         eCompileResult (SQCompiler::*f)(sCompileErrors&, int* apPos))
  {
    COMPILE_CHECK(Lex(aErrors));
    COMPILE_CHECK((this->*f)(aErrors,apPos));
    int op1 = _fs->PopTarget();
    int op2 = _fs->PopTarget();
    _fs->AddInstruction(op, _fs->PushTarget(), op1, op2);
    return eCompileResult_OK;
  }

  eCompileResult LogicalOrExp(sCompileErrors& aErrors, int* apPos)
  {
    COMPILE_CHECK(LogicalAndExp(aErrors, apPos));
    for (;;) {
      if (_token == TK_OR) {
        int first_exp = _fs->PopTarget();
        int trg = _fs->PushTarget();
        _fs->AddInstruction(_OP_OR, trg, 0, first_exp, 0);
        int jpos = _fs->GetCurrentPos();
        if (trg != first_exp)
          _fs->AddInstruction(_OP_MOVE, trg, first_exp);
        COMPILE_CHECK(Lex(aErrors));
        COMPILE_CHECK(LogicalOrExp(aErrors,apPos));
        int second_exp = _fs->PopTarget();
        if (trg != second_exp)
          _fs->AddInstruction(_OP_MOVE, trg, second_exp);
        _fs->SnoozeOpt();
        _fs->SetIntructionParam(jpos, 1, (_fs->GetCurrentPos() - jpos));
        break;
      }
      else {
        break;
      }
    }
    return eCompileResult_OK;
  }

  eCompileResult LogicalAndExp(sCompileErrors& aErrors, int* apPos)
  {
    COMPILE_CHECK(BitwiseOrExp(aErrors,apPos));
    for (;;)  {
      switch(_token) {
        case TK_AND: {
          int first_exp = _fs->PopTarget();
          int trg = _fs->PushTarget();
          _fs->AddInstruction(_OP_AND, trg, 0, first_exp, 0);
          int jpos = _fs->GetCurrentPos();
          if (trg != first_exp)
            _fs->AddInstruction(_OP_MOVE, trg, first_exp);
          COMPILE_CHECK(Lex(aErrors));
          COMPILE_CHECK(LogicalOrExp(aErrors, apPos));
          int second_exp = _fs->PopTarget();
          if(trg != second_exp) _fs->AddInstruction(_OP_MOVE, trg, second_exp);
          _fs->SnoozeOpt();
          _fs->SetIntructionParam(jpos, 1, (_fs->GetCurrentPos() - jpos));
          break;
        }
        case TK_IN:
          COMPILE_CHECK(BIN_EXP(aErrors, _OP_EXISTS, apPos, &SQCompiler::BitwiseOrExp));
          break;
        default:
          return eCompileResult_OK;
      }
    }
  }

  eCompileResult BitwiseOrExp(sCompileErrors& aErrors, int* apPos)
  {
    COMPILE_CHECK(BitwiseXorExp(aErrors,apPos));
    for (;;) {
      if (_token == '|') {
        COMPILE_CHECK(BIN_EXP(aErrors,_OP_BWOR, apPos, &SQCompiler::BitwiseXorExp));
      }
      else return eCompileResult_OK;
    }
  }

  eCompileResult BitwiseXorExp(sCompileErrors& aErrors, int* apPos)
  {
    COMPILE_CHECK(BitwiseAndExp(aErrors,apPos));
    for (;;) {
      if (_token == '^') {
        COMPILE_CHECK(BIN_EXP(aErrors,_OP_BWXOR, apPos, &SQCompiler::BitwiseAndExp));
      }
      else return eCompileResult_OK;
    }
  }

  eCompileResult BitwiseAndExp(sCompileErrors& aErrors, int* apPos)
  {
    COMPILE_CHECK(CompExp(aErrors,apPos));
    for(;;) {
      if(_token == '&') {
        COMPILE_CHECK(BIN_EXP(aErrors, _OP_BWAND, apPos, &SQCompiler::CompExp));
      }
      else return eCompileResult_OK;
    }
  }

  eCompileResult CompExp(sCompileErrors& aErrors, int* apPos)
  {
    COMPILE_CHECK(ShiftExp(aErrors,apPos));
    for (;;) {
      switch(_token) {
        case TK_SPACESHIP:
          COMPILE_CHECK(BIN_EXP(aErrors,_OP_SPACESHIP, apPos, &SQCompiler::ShiftExp));
          break;
        case TK_EQ:
          COMPILE_CHECK(BIN_EXP(aErrors,_OP_EQ, apPos, &SQCompiler::ShiftExp));
          break;
        case '>':
          COMPILE_CHECK(BIN_EXP(aErrors,_OP_G, apPos, &SQCompiler::ShiftExp));
          break;
        case '<':
          COMPILE_CHECK(BIN_EXP(aErrors,_OP_L, apPos, &SQCompiler::ShiftExp));
          break;
        case TK_GE:
          COMPILE_CHECK(BIN_EXP(aErrors,_OP_GE, apPos, &SQCompiler::ShiftExp));
          break;
        case TK_LE:
          COMPILE_CHECK(BIN_EXP(aErrors,_OP_LE, apPos, &SQCompiler::ShiftExp));
          break;
        case TK_NE:
          COMPILE_CHECK(BIN_EXP(aErrors,_OP_NE, apPos, &SQCompiler::ShiftExp));
          break;
        default: return eCompileResult_OK;
      }
    }
  }

  eCompileResult ShiftExp(sCompileErrors& aErrors, int* apPos)
  {
    COMPILE_CHECK(PlusExp(aErrors,apPos));
    for (;;) {
      switch(_token) {
        case TK_USHIFTR:
          COMPILE_CHECK(BIN_EXP(aErrors,_OP_USHIFTR, apPos, &SQCompiler::PlusExp));
          break;
        case TK_SHIFTL:
          COMPILE_CHECK(BIN_EXP(aErrors,_OP_SHIFTL, apPos, &SQCompiler::PlusExp));
          break;
        case TK_SHIFTR:
          COMPILE_CHECK(BIN_EXP(aErrors,_OP_SHIFTR, apPos, &SQCompiler::PlusExp));
          break;
        default: return eCompileResult_OK;
      }
    }
  }

  eCompileResult PlusExp(sCompileErrors& aErrors, int* apPos)
  {
    COMPILE_CHECK(MultExp(aErrors,apPos));
    for(;;) {
      switch(_token) {
        case '+':
          COMPILE_CHECK(BIN_EXP(aErrors,_OP_ADD, apPos, &SQCompiler::MultExp));
          break;
        case '-':
          COMPILE_CHECK(BIN_EXP(aErrors,_OP_SUB, apPos, &SQCompiler::MultExp));
          break;
        default: return eCompileResult_OK;
      }
    }
  }

  eCompileResult MultExp(sCompileErrors& aErrors, int* apPos)
  {
    COMPILE_CHECK(PrefixedExpr(aErrors,apPos));
    for(;;) {
      switch(_token) {
        case '*':
          COMPILE_CHECK(BIN_EXP(aErrors,_OP_MUL, apPos, &SQCompiler::PrefixedExpr));
          break;
        case '/':
          COMPILE_CHECK(BIN_EXP(aErrors,_OP_DIV, apPos, &SQCompiler::PrefixedExpr));
          break;
        case '%':
          COMPILE_CHECK(BIN_EXP(aErrors,_OP_MODULO, apPos, &SQCompiler::PrefixedExpr));
          break;
        default:
          return eCompileResult_OK;
      }
    }
  }

  eCompileResult GetExpr(sCompileErrors& aErrors, int& pos, bool needGetIfNotDot = false) {
    SQObjectPtr idx;
    COMPILE_CHECK(Lex(aErrors));
    {
      int opExt;
      COMPILE_CHECK(OpExt(aErrors,opExt));
      COMPILE_CHECK(Expect(aErrors,TK_IDENTIFIER,&idx));
      _fs->AddLineInfos(_lex.GetLastTokenLineCol(), getGenerateLineInfo(), false);
      _fs->AddInstruction(_OP_LOAD, _fs->PushTarget(), _fs->GetStringConstant(idx));
      if (_exst._opExt & _OPEXT_EXPLICIT_THIS) {
        _fs->SetTopInstructionOpExt(_OPEXT_EXPLICIT_THIS);
        _exst._opExt = 0;
        niFlagOff(_exst._opExt, _OPEXT_EXPLICIT_THIS);
      }
      if (NeedGet(false)) {
        Emit2ArgsOP(_OP_GET,0);
        if (opExt) {
          _fs->SetTopInstructionOpExt(opExt);
        }
      }
      else {
        _exst._opExt = opExt;
      }
    }
    _exst._deref = DEREF_FIELD;
    return eCompileResult_OK;
  }

  eCompileResult FuncCallExpr(sCompileErrors& aErrors, const int pos, const bool closeWithParen, int* apPos) {
    if(_exst._deref != DEREF_NO_DEREF) {
      if (pos < 0) {
        int key = _fs->PopTarget(); //key
        int table = _fs->PopTarget(); //table etc...
        int closure = _fs->PushTarget();
        int ttarget = _fs->PushTarget();
        _fs->AddInstruction(_OP_PREPCALL, closure, key, table, ttarget);
        if (_exst._opExt) {
          _fs->SetTopInstructionOpExt(_exst._opExt);
          _exst._opExt = 0;
        }
      }
      else{
        _fs->AddInstruction(_OP_MOVE, _fs->PushTarget(), 0);
      }
    }
    else {
      _fs->AddInstruction(_OP_MOVE, _fs->PushTarget(), 0);
    }
    _exst._deref = DEREF_NO_DEREF;
    if (closeWithParen) {
      COMPILE_CHECK(Lex(aErrors));
    }
    COMPILE_CHECK(FunctionCallArgs(aErrors,closeWithParen,apPos));
    return eCompileResult_OK;
  }

  //if 'pos' != -1 the previous variable is a local variable
  eCompileResult PrefixedExpr(sCompileErrors& aErrors, int* apPos)
  {
    int pos = eInvalidHandle;
    COMPILE_CHECK(Factor(aErrors,&pos));
    niAssert(pos != eInvalidHandle);
    for(;;) {
      switch(_token) {
        case '.': {
          pos = -1;
          COMPILE_CHECK(GetExpr(aErrors,pos));
          break;
        }
        case '[': {
          if (_lex._prevtoken == '\n') {
            COMPILER_ERROR(
                "cannot brake deref/or comma needed before [exp] = exp slot declaration");
          }
          COMPILE_CHECK(Lex(aErrors));
          int opExt;
          COMPILE_CHECK(OpExt(aErrors,opExt));
          COMPILE_CHECK(Expression(aErrors,NULL,apPos));
          COMPILE_CHECK(Expect(aErrors,']',NULL));
          pos = -1;
          if (NeedGet(false)) {
            Emit2ArgsOP(_OP_GET);
            if (opExt) {
              _fs->SetTopInstructionOpExt(opExt);
            }
          }
          else {
            _exst._opExt = opExt;
          }
          _exst._deref = DEREF_FIELD;
          break;
        }
        case '(': {
          COMPILE_CHECK(FuncCallExpr(aErrors,pos,true,apPos));
          break;
        }
        case TK_MINUSMINUS:
        case TK_PLUSPLUS: {
          if (_exst._deref != DEREF_NO_DEREF && !IsEndOfStatement()) {
            int tok = _token;
            COMPILE_CHECK(Lex(aErrors));
            if (pos < 0) {
              Emit2ArgsOP(tok == TK_MINUSMINUS?_OP_PDEC:_OP_PINC);
            }
            else {
              // if _derefstate != DEREF_NO_DEREF && DEREF_FIELD so is the index of a local
              int src = _fs->PopTarget();
              _fs->AddInstruction(tok == TK_MINUSMINUS?_OP_PDEC:_OP_PINC, _fs->PushTarget(), src, 0, 0xFF);
            }
            if (_exst._opExt) {
              _fs->SetTopInstructionOpExt(_exst._opExt);
              _exst._opExt = 0;
            }
          }
          goto _done_PrefixedExprEx;
        }
        default:
          goto _done_PrefixedExprEx;
      }
    }
 _done_PrefixedExprEx:
    if (apPos) *apPos = pos;
    return eCompileResult_OK;
  }

  eCompileResult Factor(sCompileErrors& aErrors, int* apPos)
  {
    switch(_token) {
      case TK_STRING_LITERAL: {
        SQObjectPtr id(_H(_lex._svalue));
        _fs->AddLineInfos(_lex.GetLastTokenLineCol(), getGenerateLineInfo(), false);
        _fs->AddInstruction(_OP_LOAD, _fs->PushTarget(), _fs->GetStringConstant(id));
        COMPILE_CHECK(Lex(aErrors));
        break;
      }
      case TK_IDENTIFIER:
      case TK_THIS:
        {
          tBool isThis;
          SQObjectPtr id;
          if (_token == TK_IDENTIFIER) {
            isThis = eFalse;
            id = _H(_lex._svalue);
          }
          else {
            isThis = eTrue;
            id = _HC(this);
          }

          COMPILE_CHECK(Lex(aErrors));
          _fs->AddLineInfos(_lex.GetLastTokenLineCol(), getGenerateLineInfo(), false);
          const int pos = _fs->GetLocalVariable(id);
          if (isThis) {
            if (pos == -1) {
              COMPILER_ERROR("'this' used outside of a function.");
            }
            else if (pos != 0) {
              COMPILER_ERROR("'this' local at an unexpected position.");
            }
            // 'this' is always the 0th local
            _fs->PushTarget(0);
            _exst._deref = pos;
            niFlagOn(_exst._opExt, _OPEXT_EXPLICIT_THIS);
          }
          else {
            if (pos == -1) {
              _fs->PushTarget(0);
              _fs->AddInstruction(_OP_LOAD, _fs->PushTarget(), _fs->GetStringConstant(id));
              if (NeedGet(false))
                Emit2ArgsOP(_OP_GET);
              _exst._deref = DEREF_FIELD;
            }
            else {
              _fs->PushTarget(pos);
              _exst._deref = pos;
            }
            niFlagOff(_exst._opExt, _OPEXT_EXPLICIT_THIS);
          }
          if (apPos) {
            *apPos = _exst._deref;
          }
          return eCompileResult_OK;
        }
        break;
      case TK_DOUBLE_COLON:  // "::"
        _fs->AddInstruction(_OP_LOADROOTTABLE, _fs->PushTarget());
        _exst._deref = DEREF_FIELD;
        _token = '.'; // hack to have a 'get' generated afterward
        if (apPos) {
          *apPos = -1;
        }
        return eCompileResult_OK;
      case TK_NULL:
        _fs->AddInstruction(_OP_LOADNULL, _fs->PushTarget());
        COMPILE_CHECK(Lex(aErrors));
        break;
      case TK_INTEGER:
        _fs->AddInstruction(_OP_LOAD, _fs->PushTarget(), _fs->GetNumericConstant(_lex._nvalue));
        COMPILE_CHECK(Lex(aErrors));
        break;
      case TK_TRUE:
        _fs->AddInstruction(_OP_LOAD, _fs->PushTarget(), _fs->GetNumericConstant(1));
        COMPILE_CHECK(Lex(aErrors));
        break;
      case TK_FALSE:
        _fs->AddInstruction(_OP_LOAD, _fs->PushTarget(), _fs->GetNumericConstant(0));
        COMPILE_CHECK(Lex(aErrors));
        break;
      case TK_INVALID:
        _fs->AddInstruction(_OP_LOAD, _fs->PushTarget(), _fs->GetNumericConstant((SQInt)eInvalidHandle));
        COMPILE_CHECK(Lex(aErrors));
        break;
      case TK_FLOAT:
        _fs->AddInstruction(_OP_LOAD, _fs->PushTarget(), _fs->GetNumericConstant(_lex._fvalue));
        COMPILE_CHECK(Lex(aErrors));
        break;
      case '[': {
        _fs->AddInstruction(_OP_NEWARRAY, _fs->PushTarget());
        int apos = _fs->GetCurrentPos(),key = 0;
        COMPILE_CHECK(Lex(aErrors));
        while(_token != ']') {
          COMPILE_CHECK(Expression(aErrors,NULL,apPos));
          if (IS_TK_COMMA(_token)) {
            COMPILE_CHECK(Lex(aErrors));
          }
          int val = _fs->PopTarget();
          int array = _fs->TopTarget();
          _fs->AddInstruction(_OP_APPENDARRAY, array, val);
          key++;
        }
        _fs->SetIntructionParam(apos, 1, key);
        COMPILE_CHECK(Lex(aErrors));
      }
        break;
      case '{':{
        _fs->AddInstruction(_OP_NEWTABLE, _fs->PushTarget());
        int tpos = _fs->GetCurrentPos(),nkeys = 0;
        COMPILE_CHECK(Lex(aErrors));
        while(_token != '}') {
          switch(_token) {
            case TK_FUNCTION:
              {
                COMPILE_CHECK(Lex(aErrors));
                SQObjectPtr id;
                COMPILE_CHECK(Expect(aErrors,TK_IDENTIFIER,&id));
                _fs->AddLineInfos(_lex.GetLastTokenLineCol(), getGenerateLineInfo(), false);
                bool hasArgList;
                COMPILE_CHECK(ExpectFuncArgList(aErrors,hasArgList));
                _fs->AddInstruction(_OP_LOAD, _fs->PushTarget(), _fs->GetStringConstant(id));
                COMPILE_CHECK(CreateFunction(aErrors,id,hasArgList,apPos));
                _fs->AddInstruction(_OP_CLOSURE, _fs->PushTarget(), _fs->GetNumFunctions() - 1, 0);
                break;
              }
            case '[': {
              COMPILE_CHECK(Lex(aErrors));
              COMPILE_CHECK(CommaExpr(aErrors,apPos));
              COMPILE_CHECK(Expect(aErrors,']',NULL));
              COMPILE_CHECK(ExpectAssignment(aErrors));
              COMPILE_CHECK(Expression(aErrors,NULL,apPos));
              break;
            }
            default: {
              SQObjectPtr id;
              COMPILE_CHECK(Expect(aErrors,TK_IDENTIFIER,&id));
              _fs->AddLineInfos(_lex.GetLastTokenLineCol(), getGenerateLineInfo(), false);
              _fs->AddInstruction(
                  _OP_LOAD, _fs->PushTarget(),
                  _fs->GetStringConstant(id));
              COMPILE_CHECK(ExpectAssignment(aErrors));
              COMPILE_CHECK(Expression(aErrors,NULL,apPos));
            }
          }

          if (IS_TK_COMMA(_token)) {
            // optional comma
            COMPILE_CHECK(Lex(aErrors));
          }
          nkeys++;
          int val = _fs->PopTarget();
          int key = _fs->PopTarget();
          int table = _fs->TopTarget(); //<<BECAUSE OF THIS NO COMMON EMIT FUNC IS POSSIBLE
          _fs->AddInstruction(_OP_NEWSLOT, _fs->PushTarget(), table, key, val);
          _fs->PopTarget();
        }
        _fs->SetIntructionParam(tpos, 1, nkeys);
        COMPILE_CHECK(Lex(aErrors));
        break;
      }
      case TK_FUNCTION: {
        COMPILE_CHECK(FunctionExp(aErrors,_token,apPos));
        break;
      }
      case '-': {
        COMPILE_CHECK(UnaryOP(aErrors,_OP_NEG, apPos));
        break;
      }
      case '!': {
        COMPILE_CHECK(UnaryOP(aErrors,_OP_NOT, apPos));
        break;
      }
      case '~': {
        COMPILE_CHECK(UnaryOP(aErrors,_OP_BWNOT, apPos));
        break;
      }
      case TK_TYPEOF: {
        COMPILE_CHECK(UnaryOP(aErrors,_OP_TYPEOF, apPos));
        break;
      }
      case TK_MINUSMINUS:
      case TK_PLUSPLUS: {
        COMPILE_CHECK(PrefixIncDec(aErrors,_token, apPos));
        break;
      }
      case TK_DELETE: {
        COMPILE_CHECK(DeleteExpr(aErrors,apPos));
        break;
      }
      case '(': {
        COMPILE_CHECK(Lex(aErrors));
        COMPILE_CHECK(CommaExpr(aErrors,apPos));
        COMPILE_CHECK(Expect(aErrors,')',NULL));
        break;
      }
      default: {
        if (_token == TK_SEXP_START_COMMA) {
          // Skip the S-Exp start comma
          COMPILE_CHECK(Lex(aErrors));
          return Expression(aErrors,NULL,apPos);
        }
        else {
          COMPILER_ERROR(niFmt("expression expected, got (%d:%c)",_token,_token));
        }
      }
    }

    if (apPos) *apPos = -1;
    return eCompileResult_OK;
  }

  eCompileResult UnaryOP(sCompileErrors& aErrors, SQOpcode op, int* apPos)
  {
    COMPILE_CHECK(Lex(aErrors));
    COMPILE_CHECK(PrefixedExpr(aErrors,apPos));
    int src = _fs->PopTarget();
    _fs->AddInstruction(op, _fs->PushTarget(), src);
    return eCompileResult_OK;
  }

  bool NeedGet(bool bIsDot)
  {
    if (bIsDot && _token == '=')
      return true;

    if (_token == TK_PLUSPLUS || _token == TK_MINUSMINUS) {
      // Fix for issue 2412
      if (_lex._prevtoken == '\n' || _lex._prevtoken == ';') {
        return true;
      }
    }

    return _token != '='
        && _token != TK_PLUSPLUS && _token != TK_MINUSMINUS
        && _token != TK_PLUSEQ && _token != TK_MINUSEQ && _token != TK_MULEQ && _token != TK_DIVEQ && _token != TK_MODULOEQ
        && _token != TK_SHIFTLEQ && _token != TK_SHIFTREQ && _token != TK_USHIFTREQ
        && _token != TK_BWANDEQ && _token != TK_BWOREQ && _token != TK_BWXOREQ
        && _token != '(' && _token != TK_NEWSLOT
        && ((!_exst._delete) ||
            (_exst._delete && (_token == '.' || _token == '[')));
  }

  eCompileResult FunctionCallArgs(sCompileErrors& aErrors, bool closeWithParen, int* apPos)
  {
    int nargs = 1; // this
    while (1) {
      if (closeWithParen) {
        if (_token == ')') {
          break;
        }
      }
      else {
        // niDebugFmt(("TOKEN: %d (%c - %s)",_token,_token,_lex._svalue))
        if (_lex._prevtoken == '\n' ||
            (_token == SQUIRREL_EOB) ||
            (_token == ';') ||
            (_token == '}') ||
            (_token == ')') ||
            (_token == ']'))
        {
          // niDebugFmt(("...Break"))
          break;
        }
      }
      COMPILE_CHECK(Expression(aErrors,NULL,apPos));
      int pos = _fs->PopTarget();
      int trg = _fs->PushTarget();
      if (trg != pos)
        _fs->AddInstruction(_OP_MOVE, trg, pos);
      nargs++;
      if (IS_TK_COMMA(_token)) {
        COMPILE_CHECK(Lex(aErrors));
      }
    }
    if (closeWithParen) {
      COMPILE_CHECK(Lex(aErrors));
    }
    for(int i = 0; i < (nargs - 1); i++)
      _fs->PopTarget();
    int stackbase = _fs->PopTarget();
    int closure = _fs->PopTarget();
    _fs->AddInstruction(_OP_CALL, _fs->PushTarget(), closure, stackbase, nargs);
    return eCompileResult_OK;
  }

  eCompileResult LocalDeclStatement(sCompileErrors& aErrors, int* apPos)
  {
    SQObjectPtr varname;
    COMPILE_CHECK(Lex(aErrors));
    if (_token == TK_FUNCTION) {
      COMPILE_CHECK(Lex(aErrors));
      COMPILE_CHECK(Expect(aErrors,TK_IDENTIFIER,&varname));
      bool hasArgList;
      COMPILE_CHECK(ExpectFuncArgList(aErrors,hasArgList));
      COMPILE_CHECK(CreateFunction(aErrors,varname,hasArgList,apPos));
      _fs->AddInstruction(_OP_CLOSURE, _fs->PushTarget(), _fs->GetNumFunctions() - 1, 0);
      _fs->PopTarget();
      _fs->PushLocalVariable(varname);
      return eCompileResult_OK;
    }

    do {
      COMPILE_CHECK(Expect(aErrors,TK_IDENTIFIER,&varname));
      if (_token == '=' || _token == TK_NEWSLOT) {
        COMPILE_CHECK(Lex(aErrors));
        COMPILE_CHECK(Expression(aErrors,NULL,apPos));
        int src = _fs->PopTarget();
        int dest = _fs->PushTarget();
        if(dest != src) _fs->AddInstruction(_OP_MOVE, dest, src);
      }
      else{
        _fs->AddInstruction(_OP_LOADNULL, _fs->PushTarget());
      }
      _fs->PopTarget();
      _fs->PushLocalVariable(varname);
      if (IS_TK_COMMA(_token)) {
        COMPILE_CHECK(Lex(aErrors));
      }
      else {
        break;
      }
    } while (1);
    return eCompileResult_OK;
  }

  eCompileResult IfStatement(sCompileErrors& aErrors, int* apPos)
  {
    int jmppos;
    bool haselse = false;
    COMPILE_CHECK(Lex(aErrors));
    _fs->AddInstruction(_OP_LINT_HINT, eSQLintHint_IfBegin);

    COMPILE_CHECK(Expect(aErrors,'(',NULL));
    int startCondInst = _fs->_instructions.size();
    COMPILE_CHECK(CommaExpr(aErrors,apPos));
    SetTypeofArg2(startCondInst, 2);
    COMPILE_CHECK(Expect(aErrors,')',NULL));
    _fs->AddInstruction(_OP_JZ, _fs->PopTarget());
    int jnepos = _fs->GetCurrentPos();
    int stacksize = _fs->GetStackSize();

    COMPILE_CHECK(Statement(aErrors,apPos));
    if (_token != '}' && _token != TK_ELSE) {
      COMPILE_CHECK(OptionalSemicolon(aErrors));
    }

    _fs->AddInstruction(_OP_LINT_HINT, eSQLintHint_IfEnd);

    _fs->CleanStack(stacksize);
    int endifblock = _fs->GetCurrentPos();
    if(_token == TK_ELSE){
      haselse = true;
      stacksize = _fs->GetStackSize();
      _fs->AddInstruction(_OP_JMP);
      jmppos = _fs->GetCurrentPos();
      COMPILE_CHECK(Lex(aErrors));
      COMPILE_CHECK(Statement(aErrors,apPos));
      COMPILE_CHECK(OptionalSemicolon(aErrors));
      _fs->CleanStack(stacksize);
      _fs->SetIntructionParam(jmppos, 1, _fs->GetCurrentPos() - jmppos);
    }
    _fs->SetIntructionParam(jnepos, 1, endifblock - jnepos + (haselse?1:0));
    return eCompileResult_OK;
  }

  eCompileResult WhileStatement(sCompileErrors& aErrors, int* apPos)
  {
    int jzpos, jmppos;
    int stacksize = _fs->GetStackSize();
    jmppos = _fs->GetCurrentPos();
    COMPILE_CHECK(Lex(aErrors));
    COMPILE_CHECK(Expect(aErrors,'(',NULL));
    COMPILE_CHECK(CommaExpr(aErrors,apPos));
    COMPILE_CHECK(Expect(aErrors,')',NULL));

    BEGIN_BREAKBLE_BLOCK();
    _fs->AddInstruction(_OP_JZ, _fs->PopTarget());
    jzpos = _fs->GetCurrentPos();
    stacksize = _fs->GetStackSize();

    COMPILE_CHECK(Statement(aErrors,apPos));

    _fs->CleanStack(stacksize);
    _fs->AddInstruction(_OP_JMP, 0, jmppos - _fs->GetCurrentPos() - 1);
    _fs->SetIntructionParam(jzpos, 1, _fs->GetCurrentPos() - jzpos);

    END_BREAKBLE_BLOCK(jmppos);
    return eCompileResult_OK;
  }

  eCompileResult DoWhileStatement(sCompileErrors& aErrors, int* apPos)
  {
    COMPILE_CHECK(Lex(aErrors));
    int jzpos = _fs->GetCurrentPos();
    int stacksize = _fs->GetStackSize();
    BEGIN_BREAKBLE_BLOCK();
    COMPILE_CHECK(Statement(aErrors,apPos));
    _fs->CleanStack(stacksize);
    COMPILE_CHECK(Expect(aErrors,TK_WHILE,NULL));
    int continuetrg = _fs->GetCurrentPos();
    COMPILE_CHECK(Expect(aErrors,'(',NULL));
    COMPILE_CHECK(CommaExpr(aErrors,apPos));
    COMPILE_CHECK(Expect(aErrors,')',NULL));
    _fs->AddInstruction(_OP_JNZ, _fs->PopTarget(), jzpos - _fs->GetCurrentPos() - 1);
    END_BREAKBLE_BLOCK(continuetrg);
    return eCompileResult_OK;
  }

  eCompileResult ForStatement(sCompileErrors& aErrors, int* apPos)
  {
    COMPILE_CHECK(Lex(aErrors));
    int stacksize = _fs->GetStackSize();
    COMPILE_CHECK(Expect(aErrors,'(',NULL));
    if (_token == TK_LOCAL) {
      COMPILE_CHECK(LocalDeclStatement(aErrors,apPos));
    }
    else if(_token != ';') {
      COMPILE_CHECK(CommaExpr(aErrors,apPos));
      _fs->PopTarget();
    }
    COMPILE_CHECK(Expect(aErrors,';',NULL));
    _fs->SnoozeOpt();
    int jmppos = _fs->GetCurrentPos();
    int jzpos = -1;
    if (_token != ';') {
      COMPILE_CHECK(CommaExpr(aErrors,apPos));
      _fs->AddInstruction(_OP_JZ, _fs->PopTarget());
      jzpos = _fs->GetCurrentPos();
    }
    COMPILE_CHECK(Expect(aErrors,';',NULL));
    _fs->SnoozeOpt();
    int expstart = _fs->GetCurrentPos() + 1;
    if(_token != ')') {
      COMPILE_CHECK(CommaExpr(aErrors,apPos));
      _fs->PopTarget();
    }
    COMPILE_CHECK(Expect(aErrors,')',NULL));
    _fs->SnoozeOpt();
    int expend = _fs->GetCurrentPos();
    int expsize = (expend - expstart) + 1;
    SQInstructionVec exp;
    if(expsize > 0) {
      for(int i = 0; i < expsize; i++) {
        exp.push_back(_fs->GetInstruction(expstart + i));
      }
      _fs->PopInstructions(expsize);
    }

    BEGIN_BREAKBLE_BLOCK();
    COMPILE_CHECK(Statement(aErrors,apPos));
    int continuetrg = _fs->GetCurrentPos();
    if(expsize > 0) {
      for(int i = 0; i < expsize; i++)
        _fs->AddInstruction(exp[i]);
    }
    _fs->AddInstruction(_OP_JMP, 0, jmppos - _fs->GetCurrentPos() - 1, 0);
    if(jzpos>  0) _fs->SetIntructionParam(jzpos, 1, _fs->GetCurrentPos() - jzpos);
    _fs->CleanStack(stacksize);
    END_BREAKBLE_BLOCK(continuetrg);

    return eCompileResult_OK;
  }

  eCompileResult ForEachStatement(sCompileErrors& aErrors, int* apPos)
  {
    SQObjectPtr idxname, valname;
    COMPILE_CHECK(Lex(aErrors));
    COMPILE_CHECK(Expect(aErrors,'(',NULL));
    COMPILE_CHECK(Expect(aErrors,TK_IDENTIFIER,&valname));
    if (IS_TK_COMMA(_token)) {
      idxname = valname;
      COMPILE_CHECK(Lex(aErrors));
      COMPILE_CHECK(Expect(aErrors,TK_IDENTIFIER,&valname));
    }
    else{
      idxname = _HC(AT_INDEX_AT);
    }
    COMPILE_CHECK(Expect(aErrors,TK_IN,NULL));

    // save the stack size
    int stacksize = _fs->GetStackSize();
    // put the table in the stack(evaluate the table expression)
    COMPILE_CHECK(Expression(aErrors,NULL,apPos));
    COMPILE_CHECK(Expect(aErrors,')',NULL));
    int container = _fs->TopTarget();
    // push the index local var
    int indexpos = _fs->PushLocalVariable(idxname);
    _fs->AddInstruction(_OP_LOADNULL, indexpos);
    // push the value local var
    int valuepos = _fs->PushLocalVariable(valname);
    _fs->AddInstruction(_OP_LOADNULL, valuepos);
    // push reference index
    // use an invalid id to make it inaccessible
    int itrpos = _fs->PushLocalVariable(_HC(AT_ITERATOR_AT));
    _fs->AddInstruction(_OP_LOADNULL, itrpos);
    int jmppos = _fs->GetCurrentPos();
    _fs->AddInstruction(_OP_FOREACH, container, 0, indexpos);
    int foreachpos = _fs->GetCurrentPos();

    // generate the statement code
    BEGIN_BREAKBLE_BLOCK();
    COMPILE_CHECK(Statement(aErrors,apPos));
    _fs->AddInstruction(_OP_JMP, 0, jmppos - _fs->GetCurrentPos() - 1);
    _fs->SetIntructionParam(foreachpos, 1, _fs->GetCurrentPos() - foreachpos);
    // restore the local variable stack(remove index,val and ref idx)
    _fs->CleanStack(stacksize);
    END_BREAKBLE_BLOCK(foreachpos - 1);

    return eCompileResult_OK;
  }

  void SetTypeofArg2(int aStartCondInst, int aArg2) {
    const int numInsts = (int)_fs->_instructions.size();
    for (int i = aStartCondInst; i < numInsts; ++i) {
      niLetMut& inst = _fs->_instructions[i];
      if (inst.op == _OP_TYPEOF) {
        inst._arg2 = aArg2;
      }
    }
  }

  eCompileResult SwitchStatement(sCompileErrors& aErrors, int* apPos)
  {
    COMPILE_CHECK(Lex(aErrors));
    _fs->AddInstruction(_OP_LINT_HINT, eSQLintHint_SwitchBegin);

    COMPILE_CHECK(Expect(aErrors,'(',NULL));
    const int startCondInst = _fs->_instructions.size();
    COMPILE_CHECK(CommaExpr(aErrors,apPos));
    SetTypeofArg2(startCondInst, 1);
    COMPILE_CHECK(Expect(aErrors,')',NULL));
    COMPILE_CHECK(Expect(aErrors,'{',NULL));
    int expr = _fs->TopTarget();
    bool bfirst = true;
    int tonextcondjmp = -1;
    int skipcondjmp = -1;
    int __nbreaks__ = _fs->GetUnresolvedBreaks()->size();
    _fs->GetBreakTargets()->push_back(0);
    while (_token == TK_CASE) {
      if(!bfirst) {
        _fs->AddInstruction(_OP_JMP, 0, 0);
        skipcondjmp = _fs->GetCurrentPos();
        _fs->SetIntructionParam(tonextcondjmp, 1, _fs->GetCurrentPos() - tonextcondjmp);
      }
      // condition
      COMPILE_CHECK(Lex(aErrors));
      COMPILE_CHECK(Expression(aErrors,NULL,apPos));
      COMPILE_CHECK(Expect(aErrors,':',NULL));
      int trg = _fs->PopTarget();
      _fs->AddInstruction(_OP_EQ, trg, trg, expr);
      _fs->AddInstruction(_OP_JZ, trg, 0);
      // end condition
      if(skipcondjmp != -1) {
        _fs->SetIntructionParam(skipcondjmp, 1, (_fs->GetCurrentPos() - skipcondjmp));
      }
      tonextcondjmp = _fs->GetCurrentPos();
      int stacksize = _fs->GetStackSize();
      COMPILE_CHECK(Statements(aErrors,apPos));
      _fs->SetStackSize(stacksize);
      bfirst = false;
    }
    if(tonextcondjmp != -1)
      _fs->SetIntructionParam(tonextcondjmp, 1, _fs->GetCurrentPos() - tonextcondjmp);
    if(_token == TK_DEFAULT) {
      COMPILE_CHECK(Lex(aErrors));
      COMPILE_CHECK(Expect(aErrors,':',NULL));
      _fs->AddInstruction(_OP_LINT_HINT, eSQLintHint_SwitchDefault, expr);
      int stacksize = _fs->GetStackSize();
      COMPILE_CHECK(Statements(aErrors,apPos));
      _fs->SetStackSize(stacksize);
    }
    COMPILE_CHECK(Expect(aErrors,'}',NULL));

    _fs->AddInstruction(_OP_LINT_HINT, eSQLintHint_SwitchEnd, expr);

    _fs->PopTarget();
    __nbreaks__ = _fs->GetUnresolvedBreaks()->size() - __nbreaks__;
    if (__nbreaks__ > 0)
      _fs->ResolveBreaks(__nbreaks__);
    _fs->GetBreakTargets()->RemoveLast();

    return eCompileResult_OK;
  }

  eCompileResult FunctionStatement(sCompileErrors& aErrors, int* apPos)
  {
    SQObjectPtr id;
    tBool bPushTarget0 = eTrue;
    COMPILE_CHECK(Lex(aErrors));

    if (_token == TK_DOUBLE_COLON) {
      bPushTarget0 = eFalse;
      _fs->AddInstruction(_OP_LOADROOTTABLE, _fs->PushTarget(0));
      COMPILE_CHECK(Lex(aErrors));
    }

    COMPILE_CHECK(Expect(aErrors,TK_IDENTIFIER,&id));
    _fs->AddLineInfos(_lex.GetLastTokenLineCol(), getGenerateLineInfo(), false);
    if (bPushTarget0) _fs->PushTarget(0);
    _fs->AddInstruction(_OP_LOAD, _fs->PushTarget(), _fs->GetStringConstant(id));

    if (_token == TK_DOUBLE_COLON)
      Emit2ArgsOP(_OP_GET);
    while(_token == TK_DOUBLE_COLON) {
      COMPILE_CHECK(Lex(aErrors));
      COMPILE_CHECK(Expect(aErrors,TK_IDENTIFIER,&id));
      _fs->AddLineInfos(
        _lex.GetLastTokenLineCol(),
        getGenerateLineInfo(),
        false);
      _fs->AddInstruction(_OP_LOAD, _fs->PushTarget(), _fs->GetStringConstant(id));
      if (_token == TK_DOUBLE_COLON)
        Emit2ArgsOP(_OP_GET);
    }
    bool hasArgList;
    COMPILE_CHECK(ExpectFuncArgList(aErrors,hasArgList));
    COMPILE_CHECK(CreateFunction(aErrors,id,hasArgList,apPos));
    _fs->AddInstruction(_OP_CLOSURE, _fs->PushTarget(), _fs->GetNumFunctions() - 1, 0);
    EmitDerefOp(_OP_NEWSLOT);
    _fs->PopTarget();

    return eCompileResult_OK;
  }

  eCompileResult TryCatchStatement(sCompileErrors& aErrors, int* apPos)
  {
    SQObjectPtr exid;
    COMPILE_CHECK(Lex(aErrors));
    _fs->AddInstruction(_OP_PUSHTRAP,0,0);
    _fs->SetTraps(_fs->GetTraps()+1);
    if(_fs->GetBreakTargets()->size()) _fs->GetBreakTargets()->back()++;
    if(_fs->GetContinueTargets()->size()) _fs->GetContinueTargets()->back()++;
    int trappos = _fs->GetCurrentPos();
    COMPILE_CHECK(Statement(aErrors,apPos));
    _fs->SetTraps(_fs->GetTraps()-1);
    _fs->AddInstruction(_OP_POPTRAP, 1, 0);
    if(_fs->GetBreakTargets()->size()) _fs->GetBreakTargets()->back()--;
    if(_fs->GetContinueTargets()->size()) _fs->GetContinueTargets()->back()--;
    _fs->AddInstruction(_OP_JMP, 0, 0);
    int jmppos = _fs->GetCurrentPos();
    _fs->SetIntructionParam(trappos, 1, (_fs->GetCurrentPos() - trappos));
    COMPILE_CHECK(Expect(aErrors,TK_CATCH,NULL));
    COMPILE_CHECK(Expect(aErrors,'(',NULL));
    COMPILE_CHECK(Expect(aErrors,TK_IDENTIFIER,&exid));
    COMPILE_CHECK(Expect(aErrors,')',NULL));
    int stacksize = _fs->GetStackSize();
    int ex_target = _fs->PushLocalVariable(exid);
    _fs->SetIntructionParam(trappos, 0, ex_target);
    COMPILE_CHECK(Statement(aErrors,apPos));
    _fs->SetIntructionParams(jmppos, 0, (_fs->GetCurrentPos() - jmppos), 0);
    _fs->CleanStack(stacksize);
    return eCompileResult_OK;
  }

  eCompileResult FunctionExp(sCompileErrors& aErrors, int ftype, int* apPos)
  {
    COMPILE_CHECK(Lex(aErrors));
    bool hasArgList;
    COMPILE_CHECK(ExpectFuncArgList(aErrors,hasArgList));
    COMPILE_CHECK(CreateFunction(aErrors,_null_,hasArgList,apPos));
    _fs->AddInstruction(_OP_CLOSURE, _fs->PushTarget(), _fs->GetNumFunctions() - 1, ftype == TK_FUNCTION?0:1);
    return eCompileResult_OK;
  }

  eCompileResult DeleteExpr(sCompileErrors& aErrors, int* apPos)
  {
    ExpState es;
    COMPILE_CHECK(Lex(aErrors));
    PushExpState();
    _exst._delete = true;
    COMPILE_CHECK(PrefixedExpr(aErrors,apPos));
    int opExt = _exst._opExt;
    _exst._opExt = 0;
    PopExpState(&es);
    if (es._deref == DEREF_NO_DEREF) {
      COMPILER_ERROR("can't delete an expression");
    }
    if (es._deref == DEREF_FIELD) {
      Emit2ArgsOP(_OP_DELETE);
    }
    else {
      COMPILER_ERROR("cannot delete a local");
    }
    if (opExt) {
      _fs->SetTopInstructionOpExt(opExt);
    }
    return eCompileResult_OK;
  }

  eCompileResult PrefixIncDec(sCompileErrors& aErrors, int token, int* apPos)
  {
    ExpState es;
    COMPILE_CHECK(Lex(aErrors));
    PushExpState();
    _exst._delete = true;
    COMPILE_CHECK(PrefixedExpr(aErrors,apPos));
    int opExt = _exst._opExt;
    _exst._opExt = 0;
    PopExpState(&es);
    if (es._deref == DEREF_FIELD) {
      Emit2ArgsOP(token == TK_PLUSPLUS?_OP_INC:_OP_DEC);
    }
    else {
      int src = _fs->PopTarget();
      _fs->AddInstruction(token == TK_PLUSPLUS?_OP_INC:_OP_DEC, _fs->PushTarget(), src, 0, 0xFF);
    }
    if (opExt) {
      _fs->SetTopInstructionOpExt(opExt);
    }
    return eCompileResult_OK;
  }

  eCompileResult ParseType(sCompileErrors& aErrors, SQObjectPtr& aTypeDef, tBool* abCanOnlyBeAType) {
    if (abCanOnlyBeAType) {
      *abCanOnlyBeAType = eFalse;
    }

    cString strType;
    SQObjectPtr o;

    COMPILE_CHECK(Expect(aErrors,TK_IDENTIFIER,&o));
    strType += _stringval(o);

    if (_token == ':' || _token == '|') {
      if (abCanOnlyBeAType) {
        *abCanOnlyBeAType = eTrue;
      }
      do {
        const tU32 wasToken = _token;
        COMPILE_CHECK(Lex(aErrors));
        strType.appendChar(wasToken);
        COMPILE_CHECK(Expect(aErrors,TK_IDENTIFIER,&o));
        strType += _stringval(o);
      } while (_token == ':' || _token == '|');
    }

    aTypeDef = _H(strType);
    return eCompileResult_OK;
  }

  eCompileResult CreateFunction(sCompileErrors& aErrors, SQObjectPtr name, bool hasArgList, int* apPos)
  {
    SQFuncState funcstate(
      SQFunctionProto::Create(), _fs,
      _sourcename,
      _lex.GetLastTokenLineCol());
    funcstate.proto().SetName(sq_isstring(name) ? _stringhval(name) : NULL);
    funcstate.AddParameter(_HC(this),_null_);

    if (hasArgList) {
      while (_token!=')') {
        tBool canOnlyBeAType = eFalse;
        SQObjectPtr typeOrParamName;
        COMPILE_CHECK(ParseType(aErrors,typeOrParamName,&canOnlyBeAType));

        if (canOnlyBeAType || _token == TK_IDENTIFIER) {
          SQObjectPtr paramName;
          COMPILE_CHECK(Expect(aErrors,TK_IDENTIFIER,&paramName));
          funcstate.AddParameter(paramName,typeOrParamName);
        }
        else {
          funcstate.AddParameter(typeOrParamName,_null_);
        }

        if (IS_TK_COMMA(_token)) {
          COMPILE_CHECK(Lex(aErrors));
        }
        else if (_token != ')') {
          COMPILER_ERROR("expected ')', identifier or ','");
        }
      }
      COMPILE_CHECK(Expect(aErrors,')',NULL));

      // outer values
      if (_token == ':') {
        COMPILE_CHECK(Lex(aErrors));
        COMPILE_CHECK(Expect(aErrors,'(',NULL));
        while (_token != ')') {
          SQObjectPtr paramname;
          COMPILE_CHECK(Expect(aErrors,TK_IDENTIFIER,&paramname));
          // outers are treated as implicit local variables
          funcstate.AddOuterValue(paramname);
          if (IS_TK_COMMA(_token)) {
            COMPILE_CHECK(Lex(aErrors));
          }
          else if(_token != ')') {
            COMPILER_ERROR(_A("expected ')', identifier or ','"));
          }
        }
        COMPILE_CHECK(Lex(aErrors));
      }

      // return type ?
      if (_token == TK_IDENTIFIER) {
        SQObjectPtr typeOrParamName;
        COMPILE_CHECK(ParseType(aErrors,funcstate._returntype,NULL));
      }
    }

    SQFuncState *currchunk = _fs;
    _fs = &funcstate;
    COMPILE_CHECK(Statement(aErrors,apPos));
    funcstate.AddLineInfos(
        _lex.GetLastTokenLineCol(),
        getGenerateLineInfo(), true);
    funcstate.AddInstruction(_OP_RETURN, -1);
    funcstate.SetStackSize(0);
    funcstate.FinalizeFuncProto();
    _fs = currchunk;
    _fs->AddFunction(&funcstate);

    return eCompileResult_OK;
  }

  int _token;
  SQFuncState* _fs;
  tHStringPtr _sourcename;
  SQLexer _lex;
  ExpStateVec _expstates;
};

tBool CompileScript(
  aout<sCompileErrors> aErrors,
  ain<SQLEXREADFUNC> aReadFn,
  ain<ni::tPtr> aReadUserPtr,
  ain_nn_mut<iHString> ahspSourceName,
  aout<SQObjectPtr> aOut)
{
  niCheck(HStringIsNotEmpty(ahspSourceName), false);
  SQCompiler compiler(aReadFn, aReadUserPtr, ahspSourceName);
  return compiler.Compile(aErrors,aOut,nullptr);
}
