#include "stdafx.h"

#include <math.h>
#include <stdlib.h>
#include "sqconfig.h"
#include "sqopcodes.h"
#include "sqfuncproto.h"
#include "sqvm.h"
#include "sqclosure.h"
#include "sqtable.h"
#include "sqarray.h"
#include "ScriptVM.h"
#include "ScriptTypes.h"

#define TARGET _stack[_stackbase+arg0]
#define COND_LITERAL (arg3!=0?(*_ci->_literals)[arg1]:STK(arg1))

#define STK(a) _stack[_stackbase+(a)]

#define _GUARDV(v,exp) { if(!exp) { SQ_THROW(); } }
#define _GUARD(exp) { if(!exp) { SQ_THROW(); } }

#define VM_ERRORT(err)    { niRTError(this,err); SQ_THROW(); }

#ifdef _DEBUG
// this function is necessary to make the VM debuggable, usefull to put breakpoints, etc... It also
// make sure that compiler shows the right line
inline void my_sq_throw() {
  int x = 0;
  niUnused(x);
}
#define SQ_THROW()  { my_sq_throw(); goto exception_trap; }
#else
#define SQ_THROW()  { goto exception_trap; }
#endif

#define IS_ZERO(ARG)  ((_sqtype(ARG) == OT_NULL) || (sq_isnumeric(ARG) && _int(ARG) == 0))

#define ARITH_OP(op,trg,_a1_,_a2_) {                                    \
    const SQObjectPtr &o1=_a1_,&o2=_a2_;                                \
    if(sq_isnumeric(o1) && sq_isnumeric(o2)) {                          \
      if((_sqtype(o1)==OT_INTEGER) && (_sqtype(o2)==OT_INTEGER)) {      \
        trg=(SQInt)(_int(o1) op _int(o2));                  \
      }else{                                                            \
        trg=(SQFloat)(tofloat(o1) op tofloat(o2));                      \
      }                                                                 \
    } else {                                                            \
      const SQObjectPtr ro1=_a1_,ro2=_a2_;                              \
      if(#op[0] == '+' && (_sqtype(ro1) == OT_STRING || _sqtype(ro2) == OT_STRING))   { \
        _GUARD(StringCat(ro1, ro2, trg));                                 \
      }                                                                 \
      else if(!ArithMetaMethod(#op[0],ro1,ro2,trg))  {                    \
        VM_ERRORT(niFmt(_A("arith op '%c' on between '%s' and '%s'"),#op[0], \
                        _ss()->GetTypeNameStr(ro1),_ss()->GetTypeNameStr(ro2))); \
      }                                                                 \
    }                                                                   \
  }

#define ARITH_OP_(v,op,trg,_a1_,_a2_) {                                 \
    const SQObjectPtr &o1=_a1_,&o2=_a2_;                                \
    if(sq_isnumeric(o1) && sq_isnumeric(o2)) {                          \
      if((_sqtype(o1)==OT_INTEGER) && (_sqtype(o2)==OT_INTEGER)) {      \
        trg=(SQInt)(_int(o1) op _int(o2));                  \
      }else{                                                            \
        trg=(SQFloat)(tofloat(o1) op tofloat(o2));                      \
      }                                                                 \
    } else {                                                            \
      const SQObjectPtr ro1=_a1_,ro2=_a2_;                              \
      if(#op[0] == '+' && (_sqtype(ro1) == OT_STRING || _sqtype(ro2) == OT_STRING))   { \
        if (!v->StringCat(ro1, ro2, trg)) return false;                   \
      }                                                                 \
      else if(!v->ArithMetaMethod(#op[0],ro1,ro2,trg))  {                 \
        VM_ERRORB_(v,niFmt(_A("arith op '%c' on between '%s' and '%s'"),#op[0], \
                           _ss()->GetTypeNameStr(ro1),_ss()->GetTypeNameStr(ro2))); \
      }                                                                 \
    }                                                                   \
  }

#define BW_OP_EX(op,trg,_a1_,_a2_) {                                    \
    const SQObjectPtr &o1=_a1_,&o2=_a2_;                                \
    if((_sqtype(o1)==OT_INTEGER || _sqtype(o1)==OT_NULL) &&             \
       (_sqtype(o2)==OT_INTEGER || _sqtype(o2)==OT_NULL)) {             \
      trg=(SQInt)(_int(o1) op _int(o2));                    \
    } else { VM_ERRORT(niFmt(_A("bitwise op between '%s' and '%s'"),    \
                             _ss()->GetTypeNameStr(o1),_ss()->GetTypeNameStr(o2)));  } \
  }

#define BW_UOP_EX(op,trg,_a1_,_a2_) {                                   \
    const SQObjectPtr &o1=_a1_,&o2=_a2_;                                \
    if((_sqtype(o1)==OT_INTEGER || _sqtype(o1)==OT_NULL) &&             \
       (_sqtype(o2)==OT_INTEGER || _sqtype(o2)==OT_NULL)) {             \
      trg=(SQInt)(*((unsigned int*)&_int(o1)) op _int(o2)); \
    } else { VM_ERRORT(niFmt(_A("bitwise op between '%s' and '%s'"),    \
                             _ss()->GetTypeNameStr(o1),_ss()->GetTypeNameStr(o2)));   } \
  }

#define BW_OP(op) {                                                     \
    const SQObjectPtr &o1=STK(arg2),&o2=STK(arg1);                      \
    if((_sqtype(o1)==OT_INTEGER || _sqtype(o1)==OT_NULL) &&             \
       (_sqtype(o2)==OT_INTEGER || _sqtype(o2)==OT_NULL)) {             \
      TARGET=(SQInt)(_int(o1) op _int(o2));                 \
    } else { VM_ERRORT(niFmt(_A("bitwise op between '%s' and '%s'"),    \
                             _ss()->GetTypeNameStr(o1),_ss()->GetTypeNameStr(o2)));   } \
  }

#define LOCAL_INC_(v, target, a, incr)          \
  {                                             \
    ARITH_OP_(v, + , target, a, incr);          \
    a = target;                                 \
  }
#define LOCAL_INC(target, a, incr)              \
  {                                             \
    ARITH_OP( + , target, a, incr);             \
    a = target;                                 \
  }

#define PLOCAL_INC_(v, target, a, incr)         \
  {                                             \
    SQObjectPtr trg;                            \
    ARITH_OP_(v, + , trg, a, incr);             \
    target = a;                                 \
    a = trg;                                    \
  }
#define PLOCAL_INC(target, a, incr)             \
  {                                             \
    SQObjectPtr trg;                            \
    ARITH_OP(+ , trg, a, incr);                 \
    target = a;                                 \
    a = trg;                                    \
  }

#ifdef niGCC
//
// This will use computed goto instead of a big switch/case,
// when writting a new OPCODE you have to make sure that any variable
// with a destructor gets destructed properly. The 'goto' statement will not
// automatically call the destructor in current scope, however you can work
// around the problem by adding an explicit scope around the variables that
// need to be destructed. The EX_BEGIN/EX_END macros basically enforce that.
// In general make sure new OPCODE are written with the form :
//    EX_BEGIN(NAME) { CODE; } EX_END_*();
// And DOT NOT put any explicit 'goto' inside the 'CODE' section.
//
/*

  Timing of niScript_Units Test Fixture (30 tests) :
  on a TeleChips TC89xx 599Mhz ARMv6 (with VFP) 256MB RAM based 7" MID by HSG :

  --- BigSwitch:
  (FAILURE: 1 out of 30 tests failed.)
  Test time: 4.376 seconds.
  Test time: 4.38 seconds.
  Test time: 4.505 seconds.
  Test time: 4.489 seconds.
  Test time: 4.362 seconds.

  --- GCC addressed labels:
  (FAILURE: 1 out of 30 tests failed.)
  Test time: 4.532 seconds.
  Test time: 4.566 seconds.
  Test time: 4.649 seconds.
  Test time: 4.49 seconds.
  Test time: 4.56 seconds.

*/
// Not enabled, timing above shows that the BigSwitch is still faster...
// #define USE_ADDRLABEL
#endif

#define EQ_OP(v,TEST) {                                         \
    int r = 0; _GUARDV(v,v->ObjEq(STK(arg2), COND_LITERAL, r)); \
    TARGET = ((TEST)?_notnull_:_null_);                         \
  }
#define CMP_OP(v,TEST) {                                          \
    int r = 0; _GUARDV(v,v->ObjCmp(STK(arg2), COND_LITERAL, r));  \
    TARGET = ((TEST)?_notnull_:_null_);                           \
  }

#define arg0 (_i_->_arg0)
#define arg1 (_i_->_arg1)
#define sarg1 (*((short *)&arg1))
#define arg2 (_i_->_arg2)
#define arg3 (_i_->_arg3)

#if !defined USE_ADDRLABEL

#ifdef NO_GARBAGE_COLLECTOR
#pragma message("VMExec: Using BigSwitch (No GC)")
#else
#pragma message("VMExec: Using BigSwitch (GC Enabled)")
#endif

#define EX_LOOP()                               \
  const SQInstruction* _i_;                  \
  for (;;)

#define EX_LOOP_INIT()                          \
  _i_ = _ci->_ip++;

#define EX_SWITCH()    switch(_i_->op)
#define EX_BEGIN(NAME) case NAME:
#define EX_END_CONTINUE()  continue;
#define EX_END_FALLTHROUGH(NEXT)  // this replace a fallthrough case statement (a case statement without break or continue at the end)
#else

#ifdef NO_GARBAGE_COLLECTOR
#pragma message("VMExec: Using GCC addressed labels (No GC)")
#else
#pragma message("VMExec: Using GCC addressed labels (GC Enabled)")
#endif

#define EX_LOOP()                               \
  const SQInstruction* _i_;
#define EX_LOOP_INIT()                          \
  EX_END_CONTINUE()

#define EX_SWITCH()
#define EX_BEGIN(NAME) EXOP_LABEL(NAME):
#define EX_END_CONTINUE()  _i_ = _ci->_ip++; goto *__op_labels[_i_->op];
#define EX_END_FALLTHROUGH(NEXT) goto EXOP_LABEL(NEXT); // this replace a fallthrough case statement (a case statement without break or continue

#define EXOP_LABEL(NAME) _label_##NAME
#define EXOP_LABELADDR(NAME)  &&EXOP_LABEL(NAME)

#endif

///////////////////////////////////////////////
bool SQVM::Execute(const SQObjectPtr &aClosure, int target, int nargs, int stackbase,SQObjectPtr &outres)
{
#ifdef USE_ADDRLABEL
  static void* __op_labels[] = {
    EXOP_LABELADDR(_OP_LINE),
    EXOP_LABELADDR(_OP_LOAD),
    EXOP_LABELADDR(_OP_LOADNULL),
    EXOP_LABELADDR(_OP_NEWSLOT),
    EXOP_LABELADDR(_OP_SET),
    EXOP_LABELADDR(_OP_GET),
    EXOP_LABELADDR(_OP_LOADROOTTABLE),
    EXOP_LABELADDR(_OP_PREPCALL),
    EXOP_LABELADDR(_OP_CALL),
    EXOP_LABELADDR(_OP_MOVE),
    EXOP_LABELADDR(_OP_ADD),
    EXOP_LABELADDR(_OP_SUB),
    EXOP_LABELADDR(_OP_MUL),
    EXOP_LABELADDR(_OP_DIV),
    EXOP_LABELADDR(_OP_MODULO),
    EXOP_LABELADDR(_OP_EQ),
    EXOP_LABELADDR(_OP_NE),
    EXOP_LABELADDR(_OP_G),
    EXOP_LABELADDR(_OP_GE),
    EXOP_LABELADDR(_OP_L),
    EXOP_LABELADDR(_OP_LE),
    EXOP_LABELADDR(_OP_EXISTS),
    EXOP_LABELADDR(_OP_NEWTABLE),
    EXOP_LABELADDR(_OP_JMP),
    EXOP_LABELADDR(_OP_JNZ),
    EXOP_LABELADDR(_OP_JZ),
    EXOP_LABELADDR(_OP_RETURN),
    EXOP_LABELADDR(_OP_CLOSURE),
    EXOP_LABELADDR(_OP_FOREACH),
    EXOP_LABELADDR(_OP_TYPEOF),
    EXOP_LABELADDR(_OP_PUSHTRAP),
    EXOP_LABELADDR(_OP_POPTRAP),
    EXOP_LABELADDR(_OP_THROW),
    EXOP_LABELADDR(_OP_NEWARRAY),
    EXOP_LABELADDR(_OP_APPENDARRAY),
    EXOP_LABELADDR(_OP_AND),
    EXOP_LABELADDR(_OP_OR),
    EXOP_LABELADDR(_OP_NEG),
    EXOP_LABELADDR(_OP_NOT),
    EXOP_LABELADDR(_OP_DELETE),
    EXOP_LABELADDR(_OP_BWNOT),
    EXOP_LABELADDR(_OP_BWAND),
    EXOP_LABELADDR(_OP_BWOR),
    EXOP_LABELADDR(_OP_BWXOR),
    EXOP_LABELADDR(_OP_MINUSEQ),
    EXOP_LABELADDR(_OP_PLUSEQ),
    EXOP_LABELADDR(_OP_MULEQ),
    EXOP_LABELADDR(_OP_DIVEQ),
    EXOP_LABELADDR(_OP_TAILCALL),
    EXOP_LABELADDR(_OP_SHIFTL),
    EXOP_LABELADDR(_OP_SHIFTR),
    EXOP_LABELADDR(_OP_INC),
    EXOP_LABELADDR(_OP_DEC),
    EXOP_LABELADDR(_OP_PINC),
    EXOP_LABELADDR(_OP_PDEC),
    EXOP_LABELADDR(_OP_GETK),
    EXOP_LABELADDR(_OP_PREPCALLK),
    EXOP_LABELADDR(_OP_DMOVE),
    EXOP_LABELADDR(_OP_LOADNULLS),
    EXOP_LABELADDR(_OP_USHIFTR),
    EXOP_LABELADDR(_OP_SHIFTLEQ),
    EXOP_LABELADDR(_OP_SHIFTREQ),
    EXOP_LABELADDR(_OP_USHIFTREQ),
    EXOP_LABELADDR(_OP_BWANDEQ),
    EXOP_LABELADDR(_OP_BWOREQ),
    EXOP_LABELADDR(_OP_BWXOREQ),
    EXOP_LABELADDR(_OP_MODULOEQ),
    EXOP_LABELADDR(_OP_SPACESHIP),
    EXOP_LABELADDR(_OP_THROW_SILENT),
  };
#endif
#ifdef EXCEPTION_DEBUG
  try {
#endif
    if ((_nnativecalls + 1) > MAX_NATIVE_CALLS) {
      VM_ERRORB(_A("Native stack overflow"));
    }
    _nnativecalls++;
    AutoDec ad(&_nnativecalls);
    int traps = 0;
    //_temp_reg vars for OP_CALL
    SQObjectPtr temp_reg = _null_;
    int ct_target;
    bool ct_tailcall;
    int ct_stackbase;

    {
      if(!StartCall(_closure(aClosure), _top - nargs, nargs, stackbase, false)) {
        // call the handler if there are no calls in the stack,
        // if not relies on the previous node
        if (_ci == NULL) {
          CallErrorHandler(_lasterror);
        }
        return false;
      }
      _ci->_root = true;
      if (_sqtype(_debughook) != OT_NULL && _rawval(_debughook) != _rawval(_ci->_closurePtr))
        CallDebugHook(_A('c'));
    }

 exception_restore:
    //SQ_TRY
    {
      EX_LOOP()
      {
        EX_LOOP_INIT();

#ifdef _DEBUG_DUMP
        if (_execDumpStack) {
          dumpstack(_stackbase);
          niDebugFmt(("\n[%d] %s %d %d %d %d\n",_ci->_ip-_ci->_iv->size(),
                      _GetOpDesc(_i_->op),arg0,arg1,arg2,arg3));
        }
#endif

        EX_SWITCH() {
          //----------------------------------- _OP_LOAD
          EX_BEGIN(_OP_LOAD) {
            TARGET = (*_ci->_literals)[arg1];
          } EX_END_CONTINUE();

          //----------------------------------- _OP_LOADNULL
          EX_BEGIN(_OP_LOADNULL) {
            TARGET = _null_;
          } EX_END_CONTINUE();

          //----------------------------------- _OP_LOADNULLS
          EX_BEGIN(_OP_LOADNULLS) {
            niLoop(i,arg1) {
              STK(arg0+i) = _null_;
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_LOADROOTTABLE
          EX_BEGIN(_OP_LOADROOTTABLE) {
            SQClosure* closure = _closure(_ci->_closurePtr);
            Ptr<SQTable> t = closure->_root;
            if (!t.IsOK()) {
              this->Raise_MsgError("Null exec closure root table.");
              SQ_THROW();
              TARGET = _null_;
            }
            else {
              TARGET = t.ptr();
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_MOVE
          EX_BEGIN(_OP_MOVE) {
            TARGET = STK(arg1);
          } EX_END_CONTINUE();

          //----------------------------------- _OP_DMOVE
          EX_BEGIN(_OP_DMOVE) {
            STK(arg0) = STK(arg1);
            STK(arg2) = STK(arg3);
          } EX_END_CONTINUE();

          //----------------------------------- _OP_NEWTABLE
          EX_BEGIN(_OP_NEWTABLE) {
            TARGET = SQTable::Create();
          } EX_END_CONTINUE();

          //----------------------------------- _OP_NEWARRAY
          EX_BEGIN(_OP_NEWARRAY) {
            TARGET = SQArray::Create(0);
            _array(TARGET)->Reserve(arg1);
          } EX_END_CONTINUE();

          //----------------------------------- _OP_APPENDARRAY
          EX_BEGIN(_OP_APPENDARRAY) {
            if(arg2!=0xFF)
              _array(STK(arg0))->Append(STK(arg1));
            else
              _array(STK(arg0))->Append((*_ci->_literals)[arg1]);
          } EX_END_CONTINUE();

          //----------------------------------- _OP_NEWSLOT
          EX_BEGIN(_OP_NEWSLOT) {
            _GUARD(NewSlot(STK(arg1), STK(arg2), STK(arg3), _i_->_ext));
            if(arg0 != arg3)
              TARGET = STK(arg3);
          } EX_END_CONTINUE();

          //----------------------------------- _OP_DELETE
          EX_BEGIN(_OP_DELETE) {
            _GUARD(DeleteSlot(STK(arg1), STK(arg2), TARGET, _i_->_ext));
          } EX_END_CONTINUE();

          //----------------------------------- _OP_SET
          EX_BEGIN(_OP_SET) {
            if (!Set(STK(arg1), STK(arg2), STK(arg3), _i_->_ext)) {
              Raise_IdxError(STK(arg2));
              SQ_THROW();
            }
            if (arg0 != arg3)
              TARGET = STK(arg3);
          } EX_END_CONTINUE();

          //----------------------------------- _OP_GET
          EX_BEGIN(_OP_GET) {
            if (!Get(STK(arg1), STK(arg2), temp_reg, NULL, _i_->_ext)) {
              Raise_IdxError(STK(arg2));
              SQ_THROW();
            }
            TARGET = temp_reg;
          } EX_END_CONTINUE();

          //----------------------------------- _OP_GETK
          EX_BEGIN(_OP_GETK) {
            if (!Get(STK(arg2), (*_ci->_literals)[arg1], temp_reg, NULL, _i_->_ext)) {
              Raise_IdxError((*_ci->_literals)[arg1]);
              SQ_THROW();
            }
            TARGET = temp_reg;
          } EX_END_CONTINUE();

          //----------------------------------- _OP_MINUSEQ
          EX_BEGIN(_OP_MINUSEQ) {
            if (arg3 == 0xFF) {
              ARITH_OP( - ,temp_reg, STK(arg1), STK(arg2)); TARGET = temp_reg;
              if(arg1 != arg0) STK(arg1) = TARGET;
            }
            else {
              SQObjectPtr tmp, tself = STK(arg1), tkey = STK(arg2);
              if(!Get(tself, tkey, tmp, NULL, _i_->_ext)) {
                Raise_IdxError(tkey);
                SQ_THROW();
              }
              if ((_i_->_ext & _OPEXT_GET_SAFE) && tmp.IsNull()) {
                // Do nothing...
              }
              else {
                ARITH_OP( - , temp_reg, tmp, STK(arg3));  TARGET = temp_reg;
                if(!Set(tself, tkey, TARGET, _i_->_ext)) {
                  Raise_IdxError(tkey);
                  SQ_THROW();
                }
              }
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_PLUSEQ
          EX_BEGIN(_OP_PLUSEQ) {
            if (arg3 == 0xFF) {
              LOCAL_INC(TARGET, STK(arg1), STK(arg2));
            }
            else {
              _GUARD(DerefInc(TARGET, STK(arg1), STK(arg2), STK(arg3), false, _i_->_ext));
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_MULEQ
          EX_BEGIN(_OP_MULEQ) {
            if (arg3 == 0xFF) {
              ARITH_OP( * ,temp_reg, STK(arg1), STK(arg2)); TARGET = temp_reg;
              if(arg1 != arg0) STK(arg1) = TARGET;
            }
            else {
              SQObjectPtr tmp, tself = STK(arg1), tkey = STK(arg2);
              if(!Get(tself, tkey, tmp, NULL, _i_->_ext)) {
                Raise_IdxError(tkey);
                SQ_THROW();
              }
              if ((_i_->_ext & _OPEXT_GET_SAFE) && tmp.IsNull()) {
                // Do nothing...
              }
              else {
                ARITH_OP( * , temp_reg, tmp, STK(arg3));  TARGET = temp_reg;
                if(!Set(tself, tkey, TARGET, _i_->_ext)) {
                  Raise_IdxError(tkey);
                  SQ_THROW();
                }
              }
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_DIVEQ
          EX_BEGIN(_OP_DIVEQ) {
            if (arg3 == 0xFF) {
              _GUARD(Div(STK(arg1), STK(arg2), temp_reg));
              TARGET = temp_reg;
              if(arg1 != arg0) STK(arg1) = TARGET;
            }
            else {
              SQObjectPtr tmp, tself = STK(arg1), tkey = STK(arg2);
              if(!Get(tself, tkey, tmp, NULL, _i_->_ext)) {
                Raise_IdxError(tkey);
                SQ_THROW();
              }
              if ((_i_->_ext & _OPEXT_GET_SAFE) && tmp.IsNull()) {
                // Do nothing...
              }
              else {
                _GUARD(Div(tmp, STK(arg3), temp_reg));
                TARGET = temp_reg;
                if(!Set(tself, tkey, TARGET, _i_->_ext)) {
                  Raise_IdxError(tkey);
                  SQ_THROW();
                }
              }
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_MODULOEQ
          EX_BEGIN(_OP_MODULOEQ) {
            if (arg3 == 0xFF) {
              _GUARD(Modulo(STK(arg1), STK(arg2), temp_reg));
              TARGET = temp_reg;
              if(arg1 != arg0) STK(arg1) = TARGET;
            }
            else {
              SQObjectPtr tmp, tself = STK(arg1), tkey = STK(arg2);
              if(!Get(tself, tkey, tmp, NULL, _i_->_ext)) {
                Raise_IdxError(tkey);
                SQ_THROW();
              }
              if ((_i_->_ext & _OPEXT_GET_SAFE) && tmp.IsNull()) {
                // Do nothing...
              }
              else {
                _GUARD(Modulo(tmp, STK(arg3), temp_reg));
                TARGET = temp_reg;
                if(!Set(tself, tkey, TARGET, _i_->_ext)) {
                  Raise_IdxError(tkey);
                  SQ_THROW();
                }
              }
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_SHIFTLEQ
          EX_BEGIN(_OP_SHIFTLEQ) {
            if (arg3 == 0xFF) {
              BW_OP_EX( << ,temp_reg, STK(arg1), STK(arg2)); TARGET = temp_reg;
              if(arg1 != arg0) STK(arg1) = TARGET;
            }
            else {
              SQObjectPtr tmp, tself = STK(arg1), tkey = STK(arg2);
              if(!Get(tself, tkey, tmp, NULL, _i_->_ext)) {
                Raise_IdxError(tkey);
                SQ_THROW();
              }
              if ((_i_->_ext & _OPEXT_GET_SAFE) && tmp.IsNull()) {
                // Do nothing...
              }
              else {
                BW_OP_EX( << , temp_reg, tmp, STK(arg3));  TARGET = temp_reg;
                if(!Set(tself, tkey, TARGET, _i_->_ext)) {
                  Raise_IdxError(tkey);
                  SQ_THROW();
                }
              }
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_SHIFTREQ
          EX_BEGIN(_OP_SHIFTREQ) {
            if (arg3 == 0xFF) {
              BW_OP_EX( >> ,temp_reg, STK(arg1), STK(arg2)); TARGET = temp_reg;
              if(arg1 != arg0) STK(arg1) = TARGET;
            }
            else {
              SQObjectPtr tmp, tself = STK(arg1), tkey = STK(arg2);
              if (!Get(tself, tkey, tmp, NULL, _i_->_ext)) {
                Raise_IdxError(tkey);
                SQ_THROW();
              }
              if ((_i_->_ext & _OPEXT_GET_SAFE) && tmp.IsNull()) {
                // Do nothing...
              }
              else {
                BW_OP_EX( >> , temp_reg, tmp, STK(arg3));  TARGET = temp_reg;
                if(!Set(tself, tkey, TARGET, _i_->_ext)) {
                  Raise_IdxError(tkey);
                  SQ_THROW();
                }
              }
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_USHIFTREQ
          EX_BEGIN(_OP_USHIFTREQ) {
            if (arg3 == 0xFF) {
              BW_UOP_EX( >> ,temp_reg, STK(arg1), STK(arg2));
              TARGET = temp_reg;
              if(arg1 != arg0) STK(arg1) = TARGET;
            }
            else {
              SQObjectPtr tmp, tself = STK(arg1), tkey = STK(arg2);
              if(!Get(tself, tkey, tmp, NULL, _i_->_ext)) {
                Raise_IdxError(tkey);
                SQ_THROW();
              }
              if ((_i_->_ext & _OPEXT_GET_SAFE) && tmp.IsNull()) {
                // Do nothing...
              }
              else {
                BW_UOP_EX( >> , temp_reg, tmp, STK(arg3));
                TARGET = temp_reg;
                if(!Set(tself, tkey, TARGET, _i_->_ext)) {
                  Raise_IdxError(tkey);
                  SQ_THROW();
                }
              }
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_BWANDEQ
          EX_BEGIN(_OP_BWANDEQ) {
            if (arg3 == 0xFF) {
              BW_OP_EX( & ,temp_reg, STK(arg1), STK(arg2)); TARGET = temp_reg;
              if(arg1 != arg0) STK(arg1) = TARGET;
            }
            else {
              SQObjectPtr tmp, tself = STK(arg1), tkey = STK(arg2);
              if(!Get(tself, tkey, tmp, NULL, _i_->_ext)) {
                Raise_IdxError(tkey);
                SQ_THROW();
              }
              if ((_i_->_ext & _OPEXT_GET_SAFE) && tmp.IsNull()) {
                // Do nothing...
              }
              else {
                BW_OP_EX( & , temp_reg, tmp, STK(arg3));  TARGET = temp_reg;
                if(!Set(tself, tkey, TARGET, _i_->_ext)) {
                  Raise_IdxError(tkey);
                  SQ_THROW();
                }
              }
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_BWOREQ
          EX_BEGIN(_OP_BWOREQ) {
            if (arg3 == 0xFF) {
              BW_OP_EX( | , temp_reg, STK(arg1), STK(arg2)); TARGET = temp_reg;
              if(arg1 != arg0) STK(arg1) = TARGET;
            }
            else {
              SQObjectPtr tmp, tself = STK(arg1), tkey = STK(arg2);
              if(!Get(tself, tkey, tmp, NULL, _i_->_ext)) {
                Raise_IdxError(tkey);
                SQ_THROW();
              }
              if ((_i_->_ext & _OPEXT_GET_SAFE) && tmp.IsNull()) {
                // Do nothing...
              }
              else {
                BW_OP_EX( | , temp_reg, tmp, STK(arg3));  TARGET = temp_reg;
                if(!Set(tself, tkey, TARGET, _i_->_ext)) {
                  Raise_IdxError(tkey);
                  SQ_THROW();
                }
              }
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_BWXOREQ
          EX_BEGIN(_OP_BWXOREQ) {
            if (arg3 == 0xFF) {
              BW_OP_EX( ^ , temp_reg, STK(arg1), STK(arg2)); TARGET = temp_reg;
              if(arg1 != arg0) STK(arg1) = TARGET;
            }
            else {
              SQObjectPtr tmp, tself = STK(arg1), tkey = STK(arg2);
              if(!Get(tself, tkey, tmp, NULL, _i_->_ext)) {
                Raise_IdxError(tkey);
                SQ_THROW();
              }
              if ((_i_->_ext & _OPEXT_GET_SAFE) && tmp.IsNull()) {
                // Do nothing...
              }
              else {
                BW_OP_EX( ^ , temp_reg, tmp, STK(arg3));  TARGET = temp_reg;
                if(!Set(tself, tkey, TARGET, _i_->_ext)) {
                  Raise_IdxError(tkey);
                  SQ_THROW();
                }
              }
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_INC
          EX_BEGIN(_OP_INC) {
            if (arg3 == 0xFF) {
              LOCAL_INC(TARGET, STK(arg1), _one_);
            }
            else {
              _GUARD(DerefInc(TARGET, STK(arg1), STK(arg2), _one_, false, _i_->_ext));
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_PINC
          EX_BEGIN(_OP_PINC) {
            if (arg3 == 0xFF) {
              PLOCAL_INC(TARGET, STK(arg1), _one_);
            }
            else {
              _GUARD(DerefInc(TARGET, STK(arg1), STK(arg2), _one_, true, _i_->_ext));
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_DEC
          EX_BEGIN(_OP_DEC) {
            if (arg3 == 0xFF) {
              LOCAL_INC(TARGET, STK(arg1), _minusone_);
            }
            else {
              _GUARD(DerefInc(TARGET, STK(arg1), STK(arg2), _minusone_, false, _i_->_ext));
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_PDEC
          EX_BEGIN(_OP_PDEC) {
            if (arg3 == 0xFF) {
              PLOCAL_INC(TARGET, STK(arg1), _minusone_);
            }
            else {
              _GUARD(DerefInc(TARGET, STK(arg1), STK(arg2), _minusone_, true, _i_->_ext));
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_PREPCALL
          EX_BEGIN(_OP_PREPCALL) {
            SQObjectPtr tmp = STK(arg2);
            if (!Get(STK(arg2), STK(arg1), temp_reg, NULL, _i_->_ext)) {
              Raise_IdxError(STK(arg1));
              SQ_THROW();
            }
            STK(arg3) = tmp;
            TARGET = temp_reg;
          } EX_END_CONTINUE();

          //----------------------------------- _OP_PREPCALLK
          EX_BEGIN(_OP_PREPCALLK) {
            SQObjectPtr tmp = STK(arg2);
            if (!Get(STK(arg2), (*_ci->_literals)[arg1], temp_reg, NULL, _i_->_ext)) {
              Raise_IdxError((*_ci->_literals)[arg1]);
              SQ_THROW();
            }
            STK(arg3) = tmp;
            TARGET = temp_reg;
          } EX_END_CONTINUE();

          //----------------------------------- _OP_JMP
          EX_BEGIN(_OP_JMP) {
            _ci->_ip += (sarg1);
          } EX_END_CONTINUE();

          //----------------------------------- _OP_TAILCALL
          EX_BEGIN(_OP_TAILCALL) {
            temp_reg = STK(arg1);
            if (_sqtype(temp_reg) == OT_CLOSURE){
              ct_tailcall = true;
              for (int i = 0; i < arg3; i++) STK(i) = STK(arg2 + i);
              ct_target = _ci->_target;
              goto common_call;
            }
          } EX_END_FALLTHROUGH(_OP_CALL);

          //----------------------------------- _OP_CALL
          EX_BEGIN(_OP_CALL) {
            ct_tailcall = false;
            ct_target = arg0;
            temp_reg = STK(arg1);
            ct_stackbase = _stackbase+arg2;
         common_call:
            switch (_sqtype(temp_reg)) {
              case OT_NULL: {
                STK(ct_target) = _null_;
                break;
              }
              case OT_CLOSURE: {
                {
                  _GUARD(StartCall(_closure(temp_reg), ct_target, arg3, ct_tailcall?_stackbase:_stackbase+arg2, ct_tailcall));
                }
                if (_sqtype(_debughook) != OT_NULL && _rawval(_debughook) != _rawval(_ci->_closurePtr))
                  CallDebugHook(_A('c'));
                break;
              }
              case OT_NATIVECLOSURE: {
                _GUARD(CallNative(_nativeclosure(temp_reg), arg3, _stackbase+arg2, temp_reg))
                    STK(ct_target) = temp_reg;
                break;
              }
              case OT_TABLE:
                {
                  if (_table(temp_reg)->CanCallMetaMethod()) {
                    Push(temp_reg);
                    for (int i = 0; i < arg3; i++)
                      Push(STK(arg2 + i));
                    if (CallMetaMethod(_table(temp_reg), MT_STD_CALL, arg3+1, temp_reg)) {
                      STK(ct_target) = temp_reg;
                      break;
                    }
                  }
                  VM_ERRORT(niFmt(_A("attempt to call '%s'"), _ss()->GetTypeNameStr(temp_reg)));
                }
              case OT_USERDATA:
                {
                  if (_userdata(temp_reg)->GetType() == eScriptType_MethodDef) {
                    _GUARD(CallMethodDef(
                        (sScriptTypeMethodDef*)_userdata(temp_reg),
                        arg3,_stackbase+arg2, temp_reg));
                    STK(ct_target) = temp_reg;
                    break;
                  }
                  else {
                    Push(temp_reg);
                    for (int i = 0; i < arg3; i++) Push(STK(arg2 + i));

                    if (_userdata(temp_reg)->GetDelegate() &&
                        CallMetaMethod(_userdata(temp_reg)->GetDelegate(),
                                       MT_STD_CALL, arg3+1, temp_reg))
                    {
                      STK(ct_target) = temp_reg;
                      break;
                    }

                    VM_ERRORT(niFmt(_A("attempt to call '%s'"), _ss()->GetTypeNameStr(temp_reg)));
                  }
                }
              case OT_IUNKNOWN:
                {
                  Push(temp_reg);
                  // skip the first parameter its an invalid 'this'
                  // pushed by the compiler to conform to the standar _call(this,...)
                  // but for iUnknown we just want _call(...) (since this
                  // is the object to 'call')
                  for (int i = 1; i < arg3; i++)
                    Push(STK(arg2 + i));

                  if (CallIUnknownMetaMethod(
                          _iunknown(temp_reg), MT_STD_CALL, arg3, temp_reg))
                  {
                    STK(ct_target) = temp_reg;
                    break;
                  }
                  VM_ERRORT(niFmt(_A("attempt to call '%s'"), _ss()->GetTypeNameStr(temp_reg)));
                }
              default:
                VM_ERRORT(niFmt(_A("attempt to call '%s'"), _ss()->GetTypeNameStr(temp_reg)));
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_JNZ
          EX_BEGIN(_OP_JNZ) {
            SQObjectPtr &o1 = STK(arg0);
            if (!IS_ZERO(o1))
              _ci->_ip+=(sarg1);
          } EX_END_CONTINUE();

          //----------------------------------- _OP_JZ
          EX_BEGIN(_OP_JZ) {
            SQObjectPtr &o1 = STK(arg0);
            if (IS_ZERO(o1))
              _ci->_ip+=(sarg1);
          } EX_END_CONTINUE();

          //----------------------------------- _OP_NOT
          EX_BEGIN(_OP_NOT) {
            SQObjectPtr &o1 = STK(arg1);
            TARGET = IS_ZERO(o1)?_notnull_:_null_;
          } EX_END_CONTINUE();

          //----------------------------------- _OP_ADD
          EX_BEGIN(_OP_ADD) {
            ARITH_OP( + , temp_reg, STK(arg2), COND_LITERAL);
            TARGET = temp_reg;
          } EX_END_CONTINUE();

          //----------------------------------- _OP_SUB
          EX_BEGIN(_OP_SUB) {
            ARITH_OP( - , temp_reg, STK(arg2), COND_LITERAL);
            TARGET = temp_reg;
          } EX_END_CONTINUE();

          //----------------------------------- _OP_MUL
          EX_BEGIN(_OP_MUL) {
            ARITH_OP( * , temp_reg, STK(arg2), COND_LITERAL);
            TARGET = temp_reg;
          } EX_END_CONTINUE();

          //----------------------------------- _OP_DIV
          EX_BEGIN(_OP_DIV) {
            _GUARD(Div(STK(arg2), COND_LITERAL, temp_reg));
            TARGET = temp_reg;
          } EX_END_CONTINUE();

          //----------------------------------- _OP_MODULO
          EX_BEGIN(_OP_MODULO) {
            _GUARD(Modulo(STK(arg2), STK(arg1), temp_reg));
            TARGET = temp_reg;
          } EX_END_CONTINUE();

          //----------------------------------- _OP_BWAND
          EX_BEGIN(_OP_BWAND) {
            BW_OP( & );
          } EX_END_CONTINUE();

          //----------------------------------- _OP_BWOR
          EX_BEGIN(_OP_BWOR) {
            BW_OP( | );
          } EX_END_CONTINUE();

          //----------------------------------- _OP_BWXOR
          EX_BEGIN(_OP_BWXOR) {
            BW_OP( ^ );
          } EX_END_CONTINUE();

          //----------------------------------- _OP_SHIFTL
          EX_BEGIN(_OP_SHIFTL) {
            BW_OP( << );
          } EX_END_CONTINUE();

          //----------------------------------- _OP_SHIFTR
          EX_BEGIN(_OP_SHIFTR) {
            BW_OP( >> );
          } EX_END_CONTINUE();

          //----------------------------------- _OP_USHIFTR
          EX_BEGIN(_OP_USHIFTR) {
            BW_UOP_EX( >> , TARGET,STK(arg2),STK(arg1));
          } EX_END_CONTINUE();

          //----------------------------------- _OP_SPACESHIP
          EX_BEGIN(_OP_SPACESHIP) {
            int r = 0;
            _GUARD(this->ObjCmp(STK(arg2), COND_LITERAL, r));
            TARGET = r;
          } EX_END_CONTINUE();

          //----------------------------------- _OP_EQ
          EX_BEGIN(_OP_EQ) {
            EQ_OP(this,r==0);
          } EX_END_CONTINUE();

          //----------------------------------- _OP_NE
          EX_BEGIN(_OP_NE) {
            EQ_OP(this,r!=0);
          } EX_END_CONTINUE();

          //----------------------------------- _OP_G
          EX_BEGIN(_OP_G) {
            CMP_OP(this,r>0);
          } EX_END_CONTINUE();

          //----------------------------------- _OP_GE
          EX_BEGIN(_OP_GE) {
            CMP_OP(this,r>=0);
          } EX_END_CONTINUE();

          //----------------------------------- _OP_L
          EX_BEGIN(_OP_L) {
            CMP_OP(this,r<0);
          } EX_END_CONTINUE();

          //----------------------------------- _OP_LE
          EX_BEGIN(_OP_LE) {
            CMP_OP(this,r<=0);
          } EX_END_CONTINUE();

          //----------------------------------- _OP_EXISTS
          EX_BEGIN(_OP_EXISTS) {
            TARGET = DoGet(STK(arg1), STK(arg2), temp_reg, NULL, _OPEXT_GET_RAW|_OPEXT_GET_SAFE)
                ? _notnull_ : _null_;
          } EX_END_CONTINUE();

          //----------------------------------- _OP_AND
          EX_BEGIN(_OP_AND) {
            SQObjectPtr &o1 = STK(arg2);
            if (IS_ZERO(o1)) {
              TARGET = STK(arg2);
              _ci->_ip += (sarg1);
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_OR
          EX_BEGIN(_OP_OR) {
            if (!IS_ZERO(STK(arg2))) {
              TARGET = STK(arg2);
              _ci->_ip += (sarg1);
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_NEG
          EX_BEGIN(_OP_NEG) {
            SQObjectPtr &o = STK(arg1);
            switch(_sqtype(o)) {
              case OT_INTEGER:
                TARGET = SQInt(-_int(o));
                break;
              case OT_FLOAT:
                TARGET = SQFloat(-_float(o));
                break;
              case OT_TABLE: {
                if (_table(o)->CanCallMetaMethod()) {
                  Push(o);
                  if (CallMetaMethod(_table(o), MT_ARITH_UNM, 1, temp_reg)) {
                    TARGET = temp_reg;
                    break;
                  }
                }
                VM_ERRORT("attempt to negate a table");
              }
              case OT_USERDATA:
                if(_userdata(o)->GetDelegate()) {
                  Push(o);
                  if (CallMetaMethod(_userdata(o)->GetDelegate(),
                                     MT_ARITH_UNM, 1, temp_reg))
                  {
                    TARGET = temp_reg;
                    break;
                  }
                  else {
                    VM_ERRORT("attempt to negate a userdata");
                  }
                }
                break;
              case OT_IUNKNOWN:
                Push(o);
                if (CallIUnknownMetaMethod(_iunknown(o), MT_ARITH_UNM, 1, temp_reg)) {
                  TARGET = temp_reg;
                  break;
                }
                else {
                  VM_ERRORT("attempt to negate a iunknown");
                }
                break;
              default:
                VM_ERRORT(niFmt(_A("attempt to negate a %s"), _ss()->GetTypeNameStr(STK(arg1))));
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_BWNOT
          EX_BEGIN(_OP_BWNOT) {
            if (_sqtype(STK(arg1)) == OT_INTEGER) {
              TARGET = SQInt(~_int(STK(arg1)));
            }
            else if (_sqtype(STK(arg1)) == OT_NULL) {
              TARGET = SQInt(~0);
            }
            else {
              VM_ERRORT(niFmt("attempt to perform a bitwise op on a %s",
                              _ss()->GetTypeNameStr(STK(arg1))));
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_CLOSURE
          EX_BEGIN(_OP_CLOSURE) {
            SQFunctionProto *func = _funcproto(_funcproto(_closure(_ci->_closurePtr)->_function)->_functions[arg1]);
            Ptr<SQTable> rootTable = _closure(_ci->_closurePtr)->_root;
            if (!rootTable.IsOK()) rootTable = _table(this->_roottable);
            if (!rootTable.IsOK()) {
              this->Raise_MsgError("Null create closure root table.");
              SQ_THROW();
              TARGET = _null_;
            }
            else
            {
              SQClosure *closure = SQClosure::Create(func,rootTable);
              SQObjectPtr closureObj = closure;
              tSize nouters = func->_outervalues.size();
              if (nouters) {
                closure->_outervalues.reserve(nouters);
                for(int i = 0; i<nouters; i++) {
                  SQOuterVar &v = func->_outervalues[i];
                  if(!v._blocal) {//environment object
                    closure->_outervalues.push_back(_null_);
                    if (!Get(STK(0), v._src, closure->_outervalues.back(), NULL, NULL)) {
                      Raise_IdxError(v._src);
                      SQ_THROW();
                    }
                  }
                  else {//local
                    closure->_outervalues.push_back(STK(_int(v._src)));
                  }
                }
              }
              TARGET = closureObj;
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_RETURN
          EX_BEGIN(_OP_RETURN) {
            if (Return(arg0, arg1, temp_reg)){
              niAssert(traps==0);
              outres = temp_reg;
              return true;
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_FOREACH
          EX_BEGIN(_OP_FOREACH) {
#define oThis STK(arg0)
#define oRefPos STK(arg2+2)
#define oKey STK(arg2)
#define oVal STK(arg2+1)

            switch (_sqtype(oThis)) {
              case OT_NULL: {
                // iterating null is noop
                break;
              }
              case OT_TABLE: {
                if (_table(oThis)->CanCallMetaMethod()) {
                  SQObjectPtr closure;
                  if (Table_GetMetaMethod(closure,_table(oThis),MT_STD_NEXTI)) {
                    SQObjectPtr itr;
                    Push(oThis);
                    Push(oRefPos);
                    if (CallMetaMethod(closure,2,itr)) {
                      if (_sqtype(itr) == OT_NULL)
                        break;
                      if (_sqtype(itr) != OT_ARRAY || _array(itr)->Size() < 2) {
                        VM_ERRORT(niFmt(_A("_nexti returned an invalid iterator, should be an array with at least two elements [key,value]")));
                      }
                      oRefPos = itr;
                      _array(itr)->Get(0,oKey);
                      _array(itr)->Get(1,oVal);
                      EX_END_CONTINUE();
                    }
                    else {
                      VM_ERRORT("_nexti failed");
                    }
                  }
                }
                SQObjectPtr itr;
                if (!_table(oThis)->Next(oRefPos, oKey, oVal, itr)) {
                  if (_sqtype(itr) == OT_STRING) {
                    this->Raise_ObjError(itr,true);
                    SQ_THROW();
                  }
                  break;
                }
                oRefPos = itr;
              } EX_END_CONTINUE();
              case OT_ARRAY: {
                int nrefidx = _array(oThis)->Next(oRefPos, oKey, oVal);
                if (nrefidx == -1)
                  break;
                oRefPos = (SQInt)nrefidx;
              } EX_END_CONTINUE();
              case OT_STRING: {
                SQObjectPtr itr;
                if (!vm_string_nexti(_stringobj(oThis), oRefPos, oKey, oVal, itr))
                  break;
                oRefPos = itr;
              } EX_END_CONTINUE();
              case OT_IUNKNOWN: {
                SQObjectPtr itr;
                if (!iunknown_nexti(this,_iunknown(oThis), oRefPos, oKey, oVal, itr)) {
                  break;
                }
                oRefPos = itr;
              } EX_END_CONTINUE();
              case OT_INTEGER: {
                const SQInt maxV = _int(oThis);
                int v;
                if (maxV == 0) {
                  break;
                }
                else if (_sqtype(oRefPos) == OT_NULL) {
                  v = 0;
                }
                else {
                  v = _int(oRefPos);
                  if (maxV < 0) {
                    --v;
                    if (v <= maxV)
                      break;
                  }
                  else if (maxV > 0) {
                    ++v;
                    if (v >= maxV)
                      break;
                  }
                }
                oRefPos = oVal = v;
                oKey = maxV - v - 1;
              } EX_END_CONTINUE();
              case OT_USERDATA:
                if (_userdata(oThis)->GetDelegate()) {
                  SQObjectPtr itr;
                  Push(oThis);
                  Push(oRefPos);
                  if (CallMetaMethod(_userdata(oThis)->GetDelegate(), MT_STD_NEXTI, 2, itr))
                  {
                    oRefPos = oKey = itr;
                    if (_sqtype(itr) == OT_NULL)
                      break;
                    if (!Get(oThis, itr, oVal, NULL, NULL)) {
                      VM_ERRORT(niFmt(_A("_nexti returned an invalid idx")));
                    }
                    EX_END_CONTINUE();
                  }
                }
              default:
                VM_ERRORT(niFmt(_A("cannot iterate %s"), _ss()->GetTypeNameStr(STK(arg0))));
            }
            _ci->_ip += sarg1;
          } EX_END_CONTINUE();

          //----------------------------------- _OP_TYPEOF
          EX_BEGIN(_OP_TYPEOF) {
            TypeOf(STK(arg1), TARGET);
          } EX_END_CONTINUE();

          //----------------------------------- _OP_LINE
          EX_BEGIN(_OP_LINE) {
            if(_sqtype(_debughook) != OT_NULL && _rawval(_debughook) != _rawval(_ci->_closurePtr))
              CallDebugHook(_A('l'),arg1);
          } EX_END_CONTINUE();

          //----------------------------------- _OP_PUSHTRAP
          EX_BEGIN(_OP_PUSHTRAP) {
            _etraps.push_back(SQExceptionTrap(_top,_stackbase, &(*_ci->_iv)[(_ci->_ip-(&(*_ci->_iv)[0]))+arg1], arg0)); traps++;
            _ci->_etraps++;
          } EX_END_CONTINUE();

          //----------------------------------- _OP_POPTRAP
          EX_BEGIN(_OP_POPTRAP) {
            for(int i=0; i<arg0; i++) {
              _etraps.pop_back(); traps--;
              _ci->_etraps--;
            }
          } EX_END_CONTINUE();

          //----------------------------------- _OP_THROW
          EX_BEGIN(_OP_THROW) {
            Raise_ObjError(TARGET,true);
            SQ_THROW();
          } EX_END_CONTINUE();

          //----------------------------------- _OP_THROW
          EX_BEGIN(_OP_THROW_SILENT) {
            Raise_ObjError(TARGET,false);
            SQ_THROW();
          } EX_END_CONTINUE();

        } // EX_SWITCH
      } // EX_LOOP
    } // SQ_TRY

 exception_trap:
    {
      SQObjectPtr currerror = _lasterror;
#ifdef _DEBUG_DUMP
      if (_execDumpStack) {
        dumpstack(_stackbase);
      }
#endif
      int n = 0;
      int last_top = _top;
      if (_ci) {
        if(traps) {
          do {
            if(_ci->_etraps > 0) {
              SQExceptionTrap &et = _etraps.back();
              _ci->_ip = et._ip;
              _top = et._stacksize;
              _stackbase = et._stackbase;
              _stack[_stackbase+et._extarget] = currerror;
              _etraps.pop_back(); traps--; _ci->_etraps--;
              while(last_top >= _top) _stack[last_top--] = _null_;
              goto exception_restore;
            }
            //if is a native closure
            if(_sqtype(_ci->_closurePtr) != OT_CLOSURE && n)
              break;
            pop_callinfo(this);
            n++;
          }while(_callsstack.size());
        }
        // call the hook
        {
          CallErrorHandler(currerror);
        }
        //remove call stack until a C function is found or the cstack is empty
        if (_ci) {
          do {
            bool exitafterthisone = _ci->_root;
            _stackbase -= _ci->_prevstkbase;
            _top = _stackbase + _ci->_prevtop;
            pop_callinfo(this);
            if ((_ci && _sqtype(_ci->_closurePtr) != OT_CLOSURE) || exitafterthisone)
              break;
          } while(_callsstack.size());
        }

        while(last_top >= _top)
          _stack[last_top--] = _null_;
        // throw the exception to terminate the execution of the function
      }
      _lasterror = currerror;
      return false;
    }
#ifdef EXCEPTION_DEBUG
  }
  catch (...) {
    cString strErr;
    sqFormatError(this,_A("UNHANDLED NATIVE EXCEPTION."),strErr,0);
    niError(strErr);
  }
#endif
}

bool SQVM::Return(int _arg0, int _arg1, SQObjectPtr& retval)
{
  if (_sqtype(_debughook) != OT_NULL && _rawval(_debughook) != _rawval(_ci->_closurePtr))
    for (int i = 0; i<_ci->_ncalls; i++)
      CallDebugHook(_A('r'));
  bool broot = _ci->_root;
  int last_top = _top;
  int target = _ci->_target;
  int oldstackbase = _stackbase;
  _stackbase -= _ci->_prevstkbase;
  _top = _stackbase + _ci->_prevtop;
  pop_callinfo(this);
  if (broot)
  {
    if (_arg0 != 0xFF)
      retval = _stack[oldstackbase + _arg1];
    else
      retval = _null_;
  }
  else
  {
    if (_arg0 != 0xFF)
      STK(target) = _stack[oldstackbase + _arg1];
    else
      STK(target) = _null_;
  }
  while (last_top>=_top)
    _stack[last_top--] = _null_;
  niAssert(oldstackbase>=_stackbase);
  return broot;
}

bool SQVM::DerefInc(SQObjectPtr& target, SQObjectPtr& self, SQObjectPtr& key, SQObjectPtr& incr, bool postfix, int opExt)
{
  SQObjectPtr tmp, tself = self, tkey = key;
  if (!Get(tself, tkey, tmp, NULL, opExt)) {
    Raise_IdxError(tkey);
    return false;
  }
  if ((opExt & _OPEXT_GET_SAFE) && tmp.IsNull()) {
    // do nothing...
    return true;
  }
  ARITH_OP_(this, + , target, tmp, incr);
  Set(tself, tkey, target, opExt);
  if (postfix)
    target = tmp;
  return true;
}

bool SQVM::Call(SQObjectPtr &closure,int nparams,int stackbase,SQObjectPtr &outres)
{
#ifdef LAST_CALLSTACK
  _strLastCallStack.clear();
  _raiseErrorMode = eScriptRaiseErrorMode_Locals;
  sqFormatError(this, NULL, _strLastCallStack,0);
  niDebugFmt((_A("LAST CALLSTACK : \n%s"),_strLastCallStack));
#endif
  switch(_sqtype(closure))
  {
    case OT_CLOSURE:
      return Execute(closure, _top - nparams, nparams, stackbase, outres);
    case OT_NATIVECLOSURE: {
      return CallNative(_nativeclosure(closure), nparams, stackbase, outres);
    }
    case OT_USERDATA: {
      SQUserData* ud = _userdata(closure);
      if (ud->GetType() != eScriptType_MethodDef) return false;
      sScriptTypeMethodDef* pMethodDef = (sScriptTypeMethodDef*)ud;
      return CallMethodDef(pMethodDef,nparams,stackbase,outres);
    }
    default: {
      niError(niFmt("InternalError: unexpected closure type: %s",
                    sqa_gettypestring(_sqtype(closure))));
      return false;
    }
  }
}
