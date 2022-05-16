#ifndef _SQLEXER_H_
#define _SQLEXER_H_
#include <niLang/STL/stack.h>

#define MAX_STRING 2024

SQ_VECTOR_TYPEDEF(SQChar,SQCharVec);

enum eReadRawStringMode {
  eReadRawStringMode_CurlySquareBrackets = 0,
  eReadRawStringMode_LtCurly = 1,
  eReadRawStringMode_TripleDoubleQuote = 2,
};

struct SQLexer
{
  SQLexer();
  ~SQLexer();
  void Init(const achar* aFileName, SQLEXREADFUNC rg, ni::tPtr up);
  SQObjectPtr Tok2Str(int tok);
  eCompileResult Lex(sCompileErrors& errors, int* apTok);
  int GetLastTokenLine() const;

 private:
  eCompileResult LexScript(sCompileErrors& errors, int* apTok, tU32 backToSExpChar);
  eCompileResult LexSExp(sCompileErrors& errors, int* apTok, const int aSExpType);
  int GetIDType(const SQChar *s);
  eCompileResult ReadString(sCompileErrors& errors, int* apTok, tU32 ndelim, ni::tBool abFormat, int preChar = 0);
  eCompileResult ReadRawString(sCompileErrors& errors, const int preChar, const eReadRawStringMode aRawStringMode);
  eCompileResult ReadNumber(sCompileErrors& errors, int* apTok, int preChar = 0);
  eCompileResult LexBlockComment(sCompileErrors& errors);
  int FinalizeSExpStringLiteral(const int aSExpType, const tU32 aSymbolPrefix, const achar* kind);
  int ReadSExpID(int preChar, const int aSExpType, const tU32 anPrevChar);
  int ReadScriptID();
  void ReadCompilerCommand();
  void Next();

 public:
  int _curtoken;
  SQTable *_keywords;
  struct sState {
    int _id;
    int _flags;
    sState() {}
    sState(int id, int flags) : _id(id), _flags(flags) {}
  };
  astl::stack<sState> _stateStack;
  int _prevtoken;
  int _currentline;
  int _lasttokenline;
  int _currentcolumn;
  const SQChar *_svalue;
  SQInt _nvalue;
  SQFloat _fvalue;
  cString _fileName;
  SQLEXREADFUNC _readf;
  ni::tPtr _up;
  tU32 _prevdata;
  tU32 _currdata;
  ni::cString _longstr;
  ni::cString _prevSExpKind;
  struct {
    int debug;
  } _compilerCommands;
};

#endif
