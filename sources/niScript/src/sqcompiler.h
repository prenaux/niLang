#ifndef _SQCOMPILER_H_
#define _SQCOMPILER_H_

#include "sqobject.h"

struct SQVM;

#define TK_IDENTIFIER 258
#define TK_STRING_LITERAL 259
#define TK_INTEGER  260
#define TK_FLOAT  261
#define TK_DELETE 262
#define TK_EQ 263
#define TK_NE 264
#define TK_LE 265
#define TK_GE 266
#define TK_SWITCH 267
#define TK_ARROW  268
#define TK_AND  269
#define TK_OR 270
#define TK_IF 271
#define TK_ELSE 272
#define TK_WHILE  273
#define TK_BREAK  274
#define TK_FOR  275
#define TK_DO 276
#define TK_NULL 277
#define TK_FOREACH  278
#define TK_IN 279
#define TK_NEWSLOT  280
#define TK_MODULO 281
#define TK_LOCAL  282
#define TK_FUNCTION 283
#define TK_RETURN 284
#define TK_TYPEOF 285
#define TK_UMINUS 286
#define TK_PLUSEQ 287
#define TK_MINUSEQ  288
#define TK_CONTINUE 289
#define TK_TRY 290
#define TK_CATCH 291
#define TK_THROW 292
#define TK_SHIFTL 293
#define TK_SHIFTR 294
#define TK_DOUBLE_COLON 295
#define TK_CASE 296
#define TK_DEFAULT 297
#define TK_THIS 298
#define TK_PLUSPLUS 299
#define TK_MINUSMINUS 300
#define TK_USHIFTR 301
#define TK_MULEQ  302
#define TK_DIVEQ  303
#define TK_SHIFTLEQ 304
#define TK_SHIFTREQ 305
#define TK_USHIFTREQ 306
#define TK_BWANDEQ  307
#define TK_BWOREQ 308
#define TK_BWXOREQ  309
#define TK_MODULOEQ 310
#define TK_TRUE   311
#define TK_FALSE  312
#define TK_INVALID  313
#define TK_SPACESHIP  314
#define TK_SEXP_START_COMMA 315
#define TK_THROW_SILENT 316 // that's what you want to use to rethrow exceptions

enum eCompileResult {
  eCompileResult_InternalError = -100,
  eCompileResult_CompilerError = -2,
  eCompileResult_LexerError = -1,
  eCompileResult_Error = -1,
  eCompileResult_OK = 0
};

#define COMPILE_FAILED(r) ((r) <= eCompileResult_Error)
#define COMPILE_SUCCEEDED(r) ((r) >= eCompileResult_OK)
#define COMPILE_CHECK(r) { eCompileResult r__ = r; if (COMPILE_FAILED(r__)) return r__; }

struct sCompileErrors {
  struct sError {
    eCompileResult type;
    int line;
    int col;
    cString msg;
  };
  astl::vector<sError> _errors;

  sCompileErrors() {
    _AddError(eCompileResult_OK,0,0,"");
  }

  tBool HasError() const {
    return _errors.size() > 1;
  }

  tU32 GetNumErrors() const {
    return (tU32)_errors.size()-1;
  }

  sError& GetLastError() {
    return _errors.back();
  }

  sError& _AddError(eCompileResult aType, int aLine, int aCol, const char* aMsg) {
    sError& e = astl::push_back(_errors);
    e.type = aType;
    e.line = aLine;
    e.col = aCol;
    if (aMsg) {
      e.msg = aMsg;
    }
    return e;
  }
  eCompileResult CompilerError(int aLine, int aCol, const char* aMsg) {
    return _AddError(
        eCompileResult_CompilerError,
        aLine,aCol,aMsg).type;
  }
  eCompileResult LexerError(int aLine, int aCol, const char* aMsg) {
    return _AddError(
        eCompileResult_LexerError,
        aLine,aCol,aMsg).type;
  }
  eCompileResult InternalError(int aLine, int aCol, const char* aMsg) {
    return _AddError(
        eCompileResult_InternalError,
        aLine,aCol,aMsg).type;
  }
};

bool CompileScript(SQVM *vm, SQLEXREADFUNC rg, ni::tPtr up, const SQChar *sourcename, SQObjectPtr &out, bool raiseerror, bool lineinfo);

#endif //_SQCOMPILER_H_
