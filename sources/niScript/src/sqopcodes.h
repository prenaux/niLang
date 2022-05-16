#ifndef _SQOPCODES_H_
#define _SQOPCODES_H_

enum SQOpcode
{
  _OP_LINE          = 0,  //LINE <<linenum>>
  _OP_LOAD          = 1,  //<<target>> <<literal>>
  _OP_LOADNULL      = 2,  //<<target>>
  _OP_NEWSLOT       = 3,  //<<target>> <<src>> <<key>> <<val>>
  _OP_SET           = 4,  //<<target>> <<src>> <<key>> <<val>>
  _OP_GET           = 5,  //<<target>> <<src>> <<key>>
  _OP_LOADROOTTABLE = 6,  //<<target>>
  _OP_PREPCALL      = 7,  //<<closuretarget>> <<key>> <<src>> <<ttarget>>
  _OP_CALL          = 8,  //<<target>> <<closure>> <<firstarg>> <<nargs>>
  _OP_MOVE          = 9,  //<<target>> <<src>>
  _OP_ADD           = 10, //<<terget>> <<op1>> <<op2>>
  _OP_SUB           = 11, //like ADD
  _OP_MUL           = 12, //like ADD
  _OP_DIV           = 13, //like ADD
  _OP_MODULO        = 14, //like ADD
  _OP_EQ            = 15, //like ADD
  _OP_NE            = 16, //like ADD
  _OP_G             = 17, //like ADD
  _OP_GE            = 18, //like ADD
  _OP_L             = 19, //like ADD
  _OP_LE            = 20, //like ADD
  _OP_EXISTS        = 21, //like ADD
  _OP_NEWTABLE      = 22, //<<target>> <<size>>
  _OP_JMP           = 23, //<<pos>>
  _OP_JNZ           = 24, //<<src>> <<pos>>
  _OP_JZ            = 25, //<<src>> <<pos>>
  _OP_RETURN        = 26, //1(return) or -1(pushnull) <<retval>>
  _OP_CLOSURE       = 27, //<<target>> <<index>> <<if 1 is a generator>>
  _OP_FOREACH       = 28, //<<container>> <<first arg>> <<jmppos>>
  _OP_TYPEOF        = 29, //<<target>> <<obj>>
  _OP_PUSHTRAP      = 30, //<<pos>>
  _OP_POPTRAP       = 31, //none
  _OP_THROW         = 32, //<<target>>
  _OP_NEWARRAY      = 33, //<<target>> <<size>>
  _OP_APPENDARRAY   = 34, //<<array>><<val>>
  _OP_AND           = 35, //<<target>> <<op1>> <<jmp>>
  _OP_OR            = 36, //<<target>> <<op1>> <<jmp>>
  _OP_NEG           = 37, //<<target>> <<src>>
  _OP_NOT           = 38, //<<target>> <<src>>
  _OP_DELETE        = 39, //<<target>> <<table>> <<key>>
  _OP_BWNOT         = 40, //like UNM
  _OP_BWAND         = 41, //like ADD
  _OP_BWOR          = 42, //like ADD
  _OP_BWXOR         = 43, //like ADD
  _OP_MINUSEQ       = 44,
  _OP_PLUSEQ        = 45,
  _OP_MULEQ         = 46,
  _OP_DIVEQ         = 47,
  _OP_TAILCALL      = 48,
  _OP_SHIFTL        = 49,
  _OP_SHIFTR        = 50,
  _OP_INC           = 51, //<<target>> <<src>> ++i
  _OP_DEC           = 52, //<<target>> <<src>> --i
  _OP_PINC          = 53, //<<target>> <<src>> i++
  _OP_PDEC          = 54, //<<target>> <<src>> i--
  _OP_GETK          = 55, //<<target>> <<key-literal>> <<src>>
  _OP_PREPCALLK     = 56, //<<target>> <<key-literal>> <<src>> <<target where src is copied>>
  _OP_DMOVE         = 57,
  _OP_LOADNULLS     = 58, //<<target>> <<count>>
  _OP_USHIFTR       = 59,
  _OP_SHIFTLEQ      = 60,
  _OP_SHIFTREQ      = 61,
  _OP_USHIFTREQ     = 62,
  _OP_BWANDEQ       = 63,
  _OP_BWOREQ        = 64,
  _OP_BWXOREQ       = 65,
  _OP_MODULOEQ      = 66,
  _OP_SPACESHIP     = 67,
  _OP_THROW_SILENT  = 68, //<<target>>
  __OP_LAST
};

#define _OPEXT_GET_RAW         niBit(0)
#define _OPEXT_GET_SAFE        niBit(1)
// explicit this access, shouldn't produce any lint warning
#define _OPEXT_EXPLICIT_THIS   niBit(2)

struct SQInstruction
{
  SQInstruction(){};
  SQInstruction(const SQInstruction &i)
  {
    op=i.op;
    _arg0=i._arg0;_arg1=i._arg1;
    _arg2=i._arg2;_arg3=i._arg3;
    _ext=i._ext;
  }
  SQInstruction(SQOpcode _op,int a0=0,int a1=0,int a2=0,int a3=0,int ext=0)
  { op=_op;
    _arg0=a0;_arg1=a1;
    _arg2=a2;_arg3=a3;
    _ext=ext;
  }
  unsigned char op;
  unsigned char _arg0;
  unsigned short _arg1;
  unsigned char _arg2;
  unsigned char _arg3;
  unsigned short _ext;
};
niCAssert(sizeof(SQInstruction) == 8);

extern "C" const achar* _GetOpDesc(int op);

#include "sqvector.h"
SQ_VECTOR_TYPEDEF(SQInstruction,SQInstructionVec);

#endif // _SQOPCODES_H_
