#ifndef _SQFUNCTION_H_
#define _SQFUNCTION_H_

#include "sqobject.h"
#include "sqopcodes.h"

struct sLinter;

struct SQOuterVar
{
  SQOuterVar(){}
  SQOuterVar(const SQObjectPtr &src,bool blocal)
  {
    _src=src;
    _blocal=blocal;
  }
  SQOuterVar(const SQOuterVar &ov)
  {
    _blocal=ov._blocal;
    _src=ov._src;
  }
  bool _blocal;
  SQObjectPtr _src;
};

struct SQLocalVarInfo
{
  SQLocalVarInfo():_start_op(0),_end_op(0){}
  SQLocalVarInfo(const SQLocalVarInfo &lvi)
  {
    _name=lvi._name;
    _start_op=lvi._start_op;
    _end_op=lvi._end_op;
    _pos=lvi._pos;
  }
  SQObjectPtr _name;
  unsigned int _start_op;
  unsigned int _end_op;
  unsigned int _pos;
};

struct SQLineInfo {
  int _line {};
  int _column {};
  int _op {};
};

SQ_VECTOR_TYPEDEF(SQOuterVar,SQOuterVarVec);
SQ_VECTOR_TYPEDEF(SQLocalVarInfo,SQLocalVarInfoVec);
SQ_VECTOR_TYPEDEF(SQLineInfo,SQLineInfoVec);

struct SQFunctionParameter {
  SQObjectPtr _name;
  SQObjectPtr _type;
};

struct SQFunctionProto : public SQCollectable, public SQ_ALLOCATOR(SQFunctionProto)
{
private:
  virtual ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) {
    return this->_DoQueryInterface(this,aIID);
  }
  virtual void __stdcall Mark(SQCollectable**) {}
  virtual void __stdcall Invalidate() {}

private:
  SQFunctionProto() {
    _stacksize = 0;
  }
public:
  static SQFunctionProto *Create() {
    return niNew SQFunctionProto();
  }
  const SQChar* GetLocal(SQVM *v,tU32 stackbase,tU32 nseq,tU32 nop);
  static sVec2i _GetLineCol(const SQInstructionVec& instructions,
                            const SQInstruction *curr,
                            const SQLineInfoVec& lineinfos);
  sVec2i GetLineCol(const SQInstruction *curr) const;

  void __stdcall SetName(iHString* ahspName) {
    this->_name = ni::HStringIsEmpty(ahspName) ? _null_ : ahspName;
  }
  iHString* __stdcall GetName() const {
    if (!sq_isstring(this->_name))
      return NULL;
    return _stringhval(this->_name);
  }

  iHString* __stdcall GetSourceName() const {
    if (!sq_isstring(this->_sourcename))
      return NULL;
    return _stringhval(this->_sourcename);
  }
  sVec2i __stdcall GetSourceLineCol() const {
    return Vec2i(this->_sourceline,this->_sourcecol);
  }

  void LintTrace(
    sLinter& aLinter,
    const struct LintClosure& aClosure) const;
  // Return the number of lint errors
  tU32 LintTraceRoot();

  SQObjectPtrVec _literals;
  SQObjectPtrVec _functions;
  astl::vector<SQFunctionParameter> _parameters;
  SQObjectPtr _returntype;
  SQOuterVarVec _outervalues;
  SQInstructionVec _instructions;
  SQObjectPtr _sourcename;
  int _sourceline;
  int _sourcecol;
  SQObjectPtr _name;
  SQLocalVarInfoVec _localvarinfos;
  SQLineInfoVec _lineinfos;
  int _stacksize;
};

#endif //_SQFUNCTION_H_
