#include "stdafx.h"

#include <ctype.h>
#include <stdlib.h>
#include "sqconfig.h"
#include "sqtable.h"
#include "sqcompiler.h"
#include "sqlexer.h"

#define STATE_REGULAR          'r'
#define STATE_SEXP             's'
#define STATE_SEXP2            'S'
#define STATE_REGULAR_IN_SEXP  'R'

#define SEXP_ENTERED niBit(0)

#define TRACE_LEXER(X) //niDebugFmt(X)

#define CUR_CHAR (_currdata)
#define PREV_CHAR (_prevdata)

#define STATE_CURRENT() _stateStack.top()._id
#define STATE_FLAGS()   _stateStack.top()._flags
#define STATE_ISEMPTY() _stateStack.empty()

#define STATE_PUSH(STATE) {                             \
    _stateStack.push(sState(STATE,0));                  \
    TRACE_LEXER(("... STATE_PUSH: [%c:%d]",             \
                 STATE_CURRENT(), _stateStack.size())); \
  }

#define STATE_POP() {                                   \
    _stateStack.pop();                                  \
    TRACE_LEXER(("... STATE_POP: [%c:%d]",        \
                 STATE_CURRENT(), _stateStack.size())); \
  }

#define RETURN_TOKEN(t) {                               \
    TRACE_LEXER(("... [%c:%d] TOKEN: (%d:%c) %s", \
                 STATE_CURRENT(), _stateStack.size(),   \
                 t, t, t == 259 ? _longstr : ""));      \
    _prevtoken = _curtoken;                             \
    _curtoken = t;                                      \
    if (apTok) *apTok = t;                              \
    return eCompileResult_OK;                           \
  }

#define IS_EOB() (CUR_CHAR <= SQUIRREL_EOB)
#define NEXT() {Next();_currentcolumn++;}
#define INIT_TEMP_STRING() { _longstr.clear(); _longstr.reserve(256); }
#define APPEND_CHAR(c) { _longstr.appendChar(c);}
#define TERMINATE_BUFFER() {_longstr.appendChar(_A('\0'));}
#define ADD_KEYWORD(key,id) _keywords->NewSlot(_H(#key),SQInt(id))

#define LEXER_ERROR(ERR)                                      \
  niWarning(niFmt("Lexer Error: %s",ERR));                    \
  return aErrors.LexerError(GetLastTokenLine(),_currentcolumn,ERR)

#define INTERNAL_ERROR(ERR)                                     \
  niWarning(niFmt("InternalError Error: %s",ERR));              \
  niAssertUnreachable(ERR);                                     \
  return aErrors.InternalError(GetLastTokenLine(),_currentcolumn,ERR)

/*
  Rule for SExp operators:

  The whitespace around an operator is used to determine whether an operator
  is used as a prefix operator, a postfix operator, or a binary operator. This
  behavior is summarized in the following rules:

  If an operator has whitespace around both sides or around neither side, it
  is treated as a binary operator. As an example, the + operator in a+b and a
  + b is treated as a binary operator.

  If an operator has whitespace on the left side only, it is treated as a
  prefix unary operator. As an example, the ++ operator in a ++b is treated as
  a prefix unary operator.

  If an operator has whitespace on the right side only, it is treated as a
  postfix unary operator. As an example, the ++ operator in a++ b is treated
  as a postfix unary operator.

  For the purposes of these rules, the sexp punctuation characters are also
  considered whitespace.

 */
static inline tBool _IsSExpOperator(const tU32 c) {
  return
      (c == '/') ||
      (c == '=') ||
      (c == '-') ||
      (c == '+') ||
      (c == '!') ||
      (c == '*') ||
      (c == '%') ||
      (c == '<') ||
      (c == '>') ||
      (c == '&') ||
      (c == '|') ||
      (c == '^') ||
      (c == '~') ||
      (c == '.') ||
      (c == '@');
}

static inline tBool _IsSExpPunctuation(const tU32 c) {
  return
      c == ',' ||
      c == ';' ||
      c == '(' || c == ')' ||
      c == '{' || c == '}' ||
      c == '[' || c == ']';
}

static inline tBool _IsIdentifier(const tU32 c) {
  if (StrIsLetterDigit(c) || c == _A('_')) {
    return eTrue;
  }
  if (c <= 0xFF) {
    // Ascii char not [letter|digit|_] can't be part of identifiers
    return eFalse;
  }

  // Allow a large set of unicode characters as identifiers
  const tU32 category = StrGetUCPProps(c,NULL,NULL);
  switch (category) {
    case eUCPCategory_Letter:
    case eUCPCategory_Number:
    case eUCPCategory_Symbol:
      return eTrue;
    default:
      return eFalse;
  }
}

SQLexer::SQLexer() {
}
SQLexer::~SQLexer()
{
  _keywords->_CollectableRelease();
}

void SQLexer::Init(const achar* aFileName, SQLEXREADFUNC rg, ni::tPtr up)
{
  _fileName = aFileName;

  _keywords = SQTable::Create();
  _keywords->_CollectableAddRef();

  memset(&_compilerCommands,0,sizeof(_compilerCommands));
  // debug is on by default, recall that the VM needs to be started with the debug flag
  // for this to be taken into account at all
  _compilerCommands.debug = true;

  ADD_KEYWORD(break, TK_BREAK);
  ADD_KEYWORD(case, TK_CASE);
  ADD_KEYWORD(catch, TK_CATCH);
  ADD_KEYWORD(continue, TK_CONTINUE);
  ADD_KEYWORD(default, TK_DEFAULT);
  ADD_KEYWORD(delete, TK_DELETE);
  ADD_KEYWORD(do, TK_DO);
  ADD_KEYWORD(else, TK_ELSE);
  ADD_KEYWORD(false, TK_FALSE);
  ADD_KEYWORD(for, TK_FOR);
  ADD_KEYWORD(foreach, TK_FOREACH);
  ADD_KEYWORD(function, TK_FUNCTION);
  ADD_KEYWORD(if, TK_IF);
  ADD_KEYWORD(in, TK_IN);
  ADD_KEYWORD(invalid, TK_INVALID);
  ADD_KEYWORD(local, TK_LOCAL);
  ADD_KEYWORD(null, TK_NULL);
  ADD_KEYWORD(return, TK_RETURN);
  ADD_KEYWORD(switch, TK_SWITCH);
  ADD_KEYWORD(this, TK_THIS);
  ADD_KEYWORD(throw, TK_THROW);
  ADD_KEYWORD(throw_silent, TK_THROW_SILENT);
  ADD_KEYWORD(true, TK_TRUE);
  ADD_KEYWORD(try, TK_TRY);
  ADD_KEYWORD(typeof, TK_TYPEOF);
  ADD_KEYWORD(while, TK_WHILE);

  _readf = rg;
  _up = up;
  _lasttokenline = _currentline = 1;
  _currentcolumn = 0;
  _prevtoken = -1;
  _svalue = NULL;
  _nvalue = 0;
  _fvalue = 0.0;
  _prevdata = _currdata = 0;
  _curtoken = 0;
  STATE_PUSH(STATE_REGULAR);

  Next();

  // Skip the first line if it starts with #, this is to handle shell #
  if (CUR_CHAR == '#') {
    // cString bangLine;
    // bangLine.appendChar(CUR_CHAR);
    do  {
      NEXT();
      // bangLine.appendChar(CUR_CHAR);
    } while (CUR_CHAR != '\n');
    NEXT();
    // niDebugFmt(("... bangLine: %s", bangLine));
  }
}

void SQLexer::Next()
{
  _prevdata = _currdata;
  SQInt t = _readf(_up);
  if (t != 0) {
    // put the current line inc here, since we can make sur easily
    // that we'll process it once only here (much better than
    // spreading the inc around...)
    if (t == _A('\n')) {
      _lasttokenline = _currentline;
      ++_currentline;
    }
    _currdata = t;
    return;
  }
  _currdata = SQUIRREL_EOB;
}

_HDecl(IDENTIFIER);
_HDecl(STRING_LITERAL);
_HDecl(INTEGER);
_HDecl(FLOAT);
_HDecl(NEWSLOT);

SQObjectPtr SQLexer::Tok2Str(int tok)
{
  if (tok <= 255) {
    return SQObjectPtr(_H(niFmt("%c",tok)));
  }

  switch (tok) {
    case TK_IDENTIFIER:
      return _HC(IDENTIFIER);
    case TK_STRING_LITERAL:
      return _HC(STRING_LITERAL);
    case TK_INTEGER:
      return _HC(INTEGER);
    case TK_FLOAT:
      return _HC(FLOAT);
    case TK_NEWSLOT:
      return _HC(NEWSLOT);
  }
  {
    SQObjectPtr itr, key, val, nitr;
    while (_keywords->Next(itr, key, val, nitr)) {
      itr = nitr;
      if (((int)_int(val)) == tok)
        return key;
    }
  }
  return SQObjectPtr(_H(niFmt("TOK{%d}",tok)));
}

eCompileResult SQLexer::LexBlockComment(sCompileErrors& aErrors)
{
  for (int nest = 1; nest > 0;) {
    switch(CUR_CHAR) {
      case _A('*'): {
        NEXT();
        if(CUR_CHAR == _A('/'))
        {
          nest--;
          NEXT();
        }
        continue;
      };
      case _A('/'): {
        NEXT();
        if(CUR_CHAR == _A('*'))
        {
          nest++;
          NEXT();
        }
        continue;
      };
      case _A('\n'):
        NEXT();
        continue;
      case SQUIRREL_EOB:
        LEXER_ERROR("missing \"*/\" in comment");
      default:
        NEXT();
    }
  }
  return eCompileResult_OK;
}

eCompileResult SQLexer::Lex(sCompileErrors& aErrors, int* apTok) {
  _lasttokenline = _currentline;
  switch (STATE_CURRENT()) {
    case STATE_REGULAR:
      return LexScript(aErrors,apTok,0);
    case STATE_SEXP:
    case STATE_SEXP2:
      return LexSExp(aErrors,apTok,STATE_CURRENT());
    case STATE_REGULAR_IN_SEXP:
      return LexScript(aErrors,apTok,'`');
  }
  INTERNAL_ERROR("Invalid state.");
}

eCompileResult SQLexer::LexScript(sCompileErrors& aErrors, int* apTok, tU32 backToSExpChar)
{
  while (CUR_CHAR>SQUIRREL_EOB) {
    if (CUR_CHAR == backToSExpChar) {
      niAssert(STATE_CURRENT() == STATE_REGULAR_IN_SEXP);
      NEXT();
      STATE_POP();
      if (STATE_ISEMPTY()) {
        LEXER_ERROR("error regular parsing in sexp, restored empty state");
      }
      if (STATE_CURRENT() != STATE_SEXP && STATE_CURRENT() != STATE_SEXP2) {
        LEXER_ERROR("error regular parsing in sexp, didnt restore sexp");
      }
      // RETURN_TOKEN(',');
      return Lex(aErrors,apTok);
    }

    switch(CUR_CHAR){
      case _A('\t'):
      case _A('\r'):
      case _A(' '):
        NEXT();
      continue;

      case _A('\n'):
        _prevtoken=_curtoken;
        _curtoken=_A('\n');
        NEXT();
        _currentcolumn=1;
        continue;

      case _A('/'):
        NEXT();
        switch(CUR_CHAR){
          case _A('*'):
            NEXT();
            COMPILE_CHECK(LexBlockComment(aErrors));
            continue;
          case _A('/'): {
            NEXT(); // skip the '/'
            switch (CUR_CHAR) {
              case _A('#'): {
                NEXT(); // skip the '#'
                // compiler command comment
                ReadCompilerCommand();
                break;
              }
              default: {
                // regular comment
                while (CUR_CHAR != _A('\n') && (!IS_EOB())) {
                  NEXT();
                }
                break;
              }
            }
            continue;
          }
          case _A('='):
            NEXT();
            RETURN_TOKEN(TK_DIVEQ);
          default:
            RETURN_TOKEN('/');
        }
      case _A('#'): {
        NEXT();
        RETURN_TOKEN(TK_FUNCTION);
      }
      case _A('='):
        NEXT();
        if (CUR_CHAR == _A('>')) {
          // =>, return shorthand
          NEXT();
          RETURN_TOKEN(TK_RETURN);
        }
        else if (CUR_CHAR == _A('=')) {
          // ==, equal comparison operator
          NEXT();
          RETURN_TOKEN(TK_EQ);
        }
        else {
          // =, assignement
          RETURN_TOKEN('=');
        }
      case _A('<'):
        NEXT();
        if (CUR_CHAR == _A('{')) {
          NEXT();
          COMPILE_CHECK(ReadRawString(aErrors,0,eReadRawStringMode_LtCurly));
          RETURN_TOKEN(TK_STRING_LITERAL);
        }
        else if (CUR_CHAR == _A('=')) {
          NEXT();
          if (CUR_CHAR == _A('>')) {
            NEXT();
            RETURN_TOKEN(TK_SPACESHIP);
          }
          else {
            RETURN_TOKEN(TK_LE);
          }
        }
        else if (CUR_CHAR == _A('-')) {
          NEXT();
          RETURN_TOKEN(TK_NEWSLOT);
        }
        else if (CUR_CHAR == _A('<')) {
          NEXT();
          if(CUR_CHAR == _A('=')){
            NEXT();
            RETURN_TOKEN(TK_SHIFTLEQ);
          }
          RETURN_TOKEN(TK_SHIFTL);
        }
        else {
          RETURN_TOKEN('<');
        }
      case _A('>'):
        NEXT();
        if (CUR_CHAR == _A('=')){ NEXT(); RETURN_TOKEN(TK_GE);}
        else if(CUR_CHAR == _A('>')){
          NEXT();
          if(CUR_CHAR == _A('>')){
            NEXT();
            if(CUR_CHAR == _A('=')){
              NEXT();
              RETURN_TOKEN(TK_USHIFTREQ);
            }
            else
              RETURN_TOKEN(TK_USHIFTR);
          }
          else if(CUR_CHAR == _A('=')){
            NEXT();
            RETURN_TOKEN(TK_SHIFTREQ);
          }
          RETURN_TOKEN(TK_SHIFTR);
        }
        else { RETURN_TOKEN('>') }
      case _A('!'):
        NEXT();
        if (CUR_CHAR != _A('=')){ RETURN_TOKEN('!')}
        else { NEXT(); RETURN_TOKEN(TK_NE); }
      case _A('"'):
      case _A('\''):
        {
          int stype = -1;
          COMPILE_CHECK(ReadString(aErrors,&stype,CUR_CHAR,eTrue));
          if (stype != -1) {
            RETURN_TOKEN(stype);
          }
          LEXER_ERROR("error parsing the string");
        }
      case '`': {
        int stype = -1;
        COMPILE_CHECK(ReadString(aErrors,&stype,'`',eFalse));
        if (stype != -1) {
          RETURN_TOKEN(TK_IDENTIFIER);
        }
        LEXER_ERROR("error parsing backtick identifier");
      }
      case _A('{'):
        NEXT();
        if (CUR_CHAR == _A('[')) {
          NEXT();
          COMPILE_CHECK(ReadRawString(aErrors,0,eReadRawStringMode_CurlySquareBrackets));
          RETURN_TOKEN(TK_STRING_LITERAL);
        }
        else {
          RETURN_TOKEN('{');
        }
      case _A('}'):
      case _A('('): case _A(')'):
      case _A('['): case _A(']'):
      case _A(';'): case _A(','):
      case _A('?'): case _A('~'):
      case _A('.'): {
        int ret = CUR_CHAR;
        NEXT();
        RETURN_TOKEN(ret);
      }
      case _A('^'):
        NEXT();
        if (CUR_CHAR == _A('=')){ NEXT(); RETURN_TOKEN(TK_BWXOREQ); }
        else RETURN_TOKEN('^');
      case _A('&'):
        NEXT();
        if (CUR_CHAR == _A('=')) { NEXT(); RETURN_TOKEN(TK_BWANDEQ); }
        else if (CUR_CHAR != _A('&')){ RETURN_TOKEN('&') }
        else { NEXT(); RETURN_TOKEN(TK_AND); }
      case _A('|'):
        NEXT();
        if (CUR_CHAR == _A('=')) { NEXT(); RETURN_TOKEN(TK_BWOREQ); }
        else if (CUR_CHAR != _A('|')){ RETURN_TOKEN('|') }
        else { NEXT(); RETURN_TOKEN(TK_OR); }
      case _A(':'):
        NEXT();
        switch (CUR_CHAR) {
          case ':':
            // ::
            NEXT();
            RETURN_TOKEN(TK_DOUBLE_COLON);
          default:
            RETURN_TOKEN(':');
        }
      case _A('-'):
        NEXT();
        if (CUR_CHAR == _A('=')){ NEXT(); RETURN_TOKEN(TK_MINUSEQ);}
        else if  (CUR_CHAR == _A('-')){ NEXT(); RETURN_TOKEN(TK_MINUSMINUS);}
        else RETURN_TOKEN('-');
      case _A('+'):
        NEXT();
        if (CUR_CHAR == _A('=')){ NEXT(); RETURN_TOKEN(TK_PLUSEQ);}
        else if (CUR_CHAR == _A('+')){ NEXT(); RETURN_TOKEN(TK_PLUSPLUS);}
        else RETURN_TOKEN('+');
      case _A('*'):
        NEXT();
        if (CUR_CHAR == _A('=')){ NEXT(); RETURN_TOKEN(TK_MULEQ); }
        else RETURN_TOKEN('*');
      case _A('%'):
        NEXT();
        if (CUR_CHAR == _A('=')){ NEXT(); RETURN_TOKEN(TK_MODULOEQ); }
        else RETURN_TOKEN('%');
      case _A('$'):
        NEXT();
        if (CUR_CHAR == _A('(')) {
          NEXT();
          STATE_PUSH(STATE_SEXP2);
          RETURN_TOKEN(TK_SEXP_START_COMMA);
        }
        else {
          RETURN_TOKEN('$');
        }
      case _A('@'):
        NEXT();
        if (CUR_CHAR == _A('(')) {
          NEXT();
          STATE_PUSH(STATE_SEXP);
          RETURN_TOKEN(TK_SEXP_START_COMMA);
        }
        LEXER_ERROR("@identifier (atcall) has been removed, replace it with a regular global call");
      case SQUIRREL_EOB: {
        RETURN_TOKEN(SQUIRREL_EOB);
      }
      default:{
        if (StrIsDigit(CUR_CHAR)) {
          int numTok = -1;
          COMPILE_CHECK(ReadNumber(aErrors,&numTok));
          niAssert(numTok != -1);
          RETURN_TOKEN(numTok);
        }
        else if (_IsIdentifier(CUR_CHAR)) {
          int t = ReadScriptID();
          RETURN_TOKEN(t);
        }
        else {
          int c = CUR_CHAR;
          if (StrIsControl(c)) {
            LEXER_ERROR("unexpected control character");
          }
          NEXT();
          RETURN_TOKEN(c);
        }
      }
    }
  }

  RETURN_TOKEN(SQUIRREL_EOB);
}

eCompileResult SQLexer::LexSExp(sCompileErrors& aErrors, int* apTok, const int aSExpType) {
  if (!niFlagIs(STATE_FLAGS(),SEXP_ENTERED)) {
    niFlagOn(STATE_FLAGS(),SEXP_ENTERED);
    RETURN_TOKEN('[');
  }

  while(CUR_CHAR>SQUIRREL_EOB) {
    if (aSExpType == STATE_SEXP2) {
      // for $()
      if (StrIsSpace(CUR_CHAR) && CUR_CHAR != '\n') {
        NEXT();
        continue;
      }
    }
    else {
      // for @()
      if (StrIsSpace(CUR_CHAR) || CUR_CHAR == ',') {
        if (CUR_CHAR == '\n') {
          _prevtoken = _curtoken;
          NEXT();
          _currentcolumn = 1;
        }
        else {
          NEXT();
        }
        continue;
      }
    }

    const int switchChar = CUR_CHAR;
    switch (switchChar) {
      case _A('('):
        NEXT();
        STATE_PUSH(aSExpType);
        _prevSExpKind = "(";
        RETURN_TOKEN(TK_SEXP_START_COMMA);
      case _A(')'):
        NEXT();
        STATE_POP();
        if (STATE_ISEMPTY()) {
          LEXER_ERROR("S-exp, unexpected ')'");
        }
        _prevSExpKind = ")";
        RETURN_TOKEN(']');
      case _A('/'): {
        const tU32 prevChar = PREV_CHAR;
        NEXT();
        switch(CUR_CHAR){
          case _A('*'):
            NEXT();
            COMPILE_CHECK(LexBlockComment(aErrors));
            continue;
          case _A('/'): {
            NEXT(); // skip the '/'
            switch (CUR_CHAR) {
              case _A('#'): {
                NEXT(); // skip the '#'
                // compiler command comment
                ReadCompilerCommand();
                break;
              }
              default: {
                // regular comment
                while (CUR_CHAR != _A('\n') && (!IS_EOB())) {
                  NEXT();
                }
                break;
              }
            }
            continue;
          }
          default: {
            int t = ReadSExpID('/',aSExpType,prevChar);
            RETURN_TOKEN(t);
          }
        }
      }
      case _A('<'): {
        const tU32 prevChar = PREV_CHAR;
        NEXT();
        switch(CUR_CHAR){
          case _A('{'): {
            NEXT();
            COMPILE_CHECK(ReadRawString(aErrors,'`',eReadRawStringMode_LtCurly));
            _prevSExpKind = "string";
            RETURN_TOKEN(TK_STRING_LITERAL);
          }
          default: {
            int t = ReadSExpID('<',aSExpType,prevChar);
            RETURN_TOKEN(t);
          }
        }
      }
      case _A('"'):
      case _A('\''):
        {
          int stype = -1;
          COMPILE_CHECK(ReadString(aErrors,&stype,CUR_CHAR,eTrue,'`'));
          if (stype != -1) {
            _prevSExpKind = "string";
            RETURN_TOKEN(stype);
          }
          LEXER_ERROR("S-exp, error parsing string");
        }
      case _A('{'):
        NEXT();
        if (CUR_CHAR == _A('[')) {
          NEXT();
          COMPILE_CHECK(ReadRawString(aErrors,'`',eReadRawStringMode_CurlySquareBrackets));
          _prevSExpKind = "string";
          RETURN_TOKEN(TK_STRING_LITERAL);
        }
        // fallthrought...
      case _A('}'):
      case _A('['):
      case _A(']'):
      case _A(';'):
      case _A(','):
        {
          if (switchChar != '{') {
            NEXT();
          }
          _longstr = AZEROSTR;
          _longstr.appendChar(switchChar);
          const int t = FinalizeSExpStringLiteral(aSExpType,'$',"punc");
          RETURN_TOKEN(t);
        }
      case _A('\n'): {
        _prevtoken = _curtoken;
        NEXT();
        tBool insertSemicolon = eFalse;
        // niDebugFmt(("... sqlexer: Insert ; ? kind: %s (%s)", _prevSExpKind, _longstr));
        if (_prevSExpKind.Eq("punc")) {
          if (_longstr.StartsWith("$][") ||
              _longstr.StartsWith("$}[") ||
              _longstr.StartsWith("$)["))
          {
            insertSemicolon = eTrue;
          }
        }
        else if (_prevSExpKind.StartsWith("op_") && _longstr.StartsWith("$![")) {
          // macro expansion !
          insertSemicolon = eTrue;
        }
        else if (_prevSExpKind.Eq("id") || _prevSExpKind.Eq("number") || _prevSExpKind.Eq("string") || _prevSExpKind.Eq(")")) {
          insertSemicolon = eTrue;
        }
        _prevSExpKind = "newline";
        if (insertSemicolon) {
          // niDebugFmt(("... sqlexer: inserting ; %s (%s)", _prevSExpKind, _longstr));
          _longstr = AZEROSTR;
          _longstr.appendChar(';');
          --_currentline; // -1 line for the expression
          const int t = FinalizeSExpStringLiteral(aSExpType,'$',"newline");
          ++_currentline; // restore the current line
          _currentcolumn = 1;
          RETURN_TOKEN(t);
        }
        else {
          _currentcolumn = 1;
          continue;
        }
      }
      case '`': {
        const tBool retComma =
            (_curtoken == ')' || _curtoken == ']');
        // TRACE_LEXER(("... Enter STATE_REGULAR_IN_SEXP: %c",_curtoken));
        NEXT();
        STATE_PUSH(STATE_REGULAR_IN_SEXP);
        if (retComma) {
          RETURN_TOKEN(',');
        }
        return Lex(aErrors,apTok);
      }
      case SQUIRREL_EOB: {
        LEXER_ERROR("S-exp, unexpected end of file");
      }
      case '-':
      case '+': {
        const tU32 prevChar = PREV_CHAR;
        const int preChar = CUR_CHAR;
        NEXT();
        if (StrIsDigit(CUR_CHAR)) {
          if (aSExpType == STATE_SEXP2) {
            const int t = ReadSExpID(preChar,aSExpType,prevChar);
            RETURN_TOKEN(t);
          }
          else {
            int numTok = -1;
            COMPILE_CHECK(ReadNumber(aErrors,&numTok,preChar));
            niAssert(numTok != -1);
            RETURN_TOKEN(numTok);
          }
        }
        else {
          int t = ReadSExpID(preChar,aSExpType,prevChar);
          RETURN_TOKEN(t);
        }
      }
      default: {
        const tU32 prevChar = PREV_CHAR;
        if (StrIsDigit(CUR_CHAR)) {
          if (aSExpType == STATE_SEXP2) {
            int t = ReadSExpID(0,aSExpType,prevChar);
            RETURN_TOKEN(t);
          }
          else {
            int numTok = -1;
            COMPILE_CHECK(ReadNumber(aErrors,&numTok));
            niAssert(numTok != -1);
            RETURN_TOKEN(numTok);
          }
        }
        else {
          int t = ReadSExpID(0,aSExpType,prevChar);
          RETURN_TOKEN(t);
        }
      }
    }
  }

  RETURN_TOKEN(SQUIRREL_EOB);
}

int SQLexer::GetIDType(const SQChar *s)
{
  SQObjectPtr t;
  SQObjectPtr str = _H(s);
  if(_keywords->Get(str,t)) {
    return int(_int(t));
  }
  return TK_IDENTIFIER;
}

eCompileResult SQLexer::ReadRawString(
    sCompileErrors& aErrors, const int preChar, const eReadRawStringMode aMode)
{
  INIT_TEMP_STRING();
  if (preChar != 0) {
    APPEND_CHAR(preChar);
  }

  const tU32 startLine = _currentline;

  tBool skippedInitialNewLine = eFalse;

  tBool stripLeadingSpaces = (aMode != eReadRawStringMode_TripleDoubleQuote);
  tBool countLeadingSpaces = eTrue;
  tU32 numLeadingSpaces = 0;
  tU32 remLeadingSpaces = 0;

  for (int nest = 1; nest > 0;) {
    switch(CUR_CHAR) {
      case _A(']'): {
        NEXT();
        if((aMode == eReadRawStringMode_CurlySquareBrackets) && CUR_CHAR == _A('}')) {
          nest--; NEXT();
          if(nest > 0) {
            _longstr.appendChar(_A(']'));
            _longstr.appendChar(_A('}'));
          }
        }
        else {
          _longstr.appendChar(_A(']'));
        }
        continue;
      }

      case _A('{'): {
        _longstr.appendChar(CUR_CHAR);
        NEXT();
        if ((aMode == eReadRawStringMode_CurlySquareBrackets) && CUR_CHAR == _A('[')) {
          _longstr.appendChar(CUR_CHAR);
          nest++; NEXT();
        }
        continue;
      }

      case _A('}'): {
        NEXT();
        if((aMode == eReadRawStringMode_LtCurly) && CUR_CHAR == _A('>')) {
          nest--; NEXT();
          if(nest > 0) {
            _longstr.appendChar(_A('}'));
            _longstr.appendChar(_A('>'));
          }
        }
        else {
          _longstr.appendChar(_A('}'));
        }
        continue;
      }
      case _A('<'): {
        _longstr.appendChar(CUR_CHAR);
        NEXT();
        if ((aMode == eReadRawStringMode_LtCurly) && CUR_CHAR == _A('{')) {
          _longstr.appendChar(CUR_CHAR);
          nest++; NEXT();
        }
        continue;
      }

      case _A('\\'): {
        if (aMode == eReadRawStringMode_TripleDoubleQuote) {
          NEXT();
          switch(CUR_CHAR) {
            case '"': {
              _longstr.appendChar(CUR_CHAR);
              NEXT();
              continue;
            }
            default: {
              _longstr.appendChar(_A('\\'));
              _longstr.appendChar(CUR_CHAR);
              NEXT();
              continue;
            }
          }
        }
        else {
          NEXT();
          switch(CUR_CHAR) {
            case '{':
            case '}': {
              _longstr.appendChar(CUR_CHAR);
              NEXT();
              continue;
            }
            default: {
              _longstr.appendChar(_A('\\'));
              _longstr.appendChar(CUR_CHAR);
              NEXT();
              continue;
            }
          }
        }
      }

      case '\"': {
        NEXT();
        if (aMode == eReadRawStringMode_TripleDoubleQuote) {
          if (CUR_CHAR == '\"') { // two double quotes
            NEXT();
            if (CUR_CHAR == '\"') { // three double quotes
              --nest;
              NEXT();
            }
            else { // *not* three double quotes
              _longstr.appendChar(_A('\"'));
              _longstr.appendChar(_A('\"'));
            }
            continue;
          }
          else { // *not* three double quotes
            _longstr.appendChar(_A('\"'));
          }
        }
        else { // *not* two double quotes
          _longstr.appendChar(_A('\"'));
        }
        break;
      }

      case SQUIRREL_EOB: {
        LEXER_ERROR(niFmt("Unexpected end of file in raw string started at line %d.",
                          startLine));
      }

      case '\n': {
        if (!skippedInitialNewLine && _longstr.empty()) {
          // skip initial empty new line
          skippedInitialNewLine = eTrue;
          countLeadingSpaces = eTrue;
          numLeadingSpaces = 0;
          remLeadingSpaces = 0;
        }
        else {
          _longstr.appendChar(CUR_CHAR);
          remLeadingSpaces = numLeadingSpaces;
        }
        NEXT();
        break;
      }

      default: {
        if (stripLeadingSpaces) {
          if (countLeadingSpaces) {
            if ((CUR_CHAR != '\n') && StrIsSpace(CUR_CHAR)) {
              ++numLeadingSpaces;
              NEXT();
              continue; // skip this space
            }
            else {
              countLeadingSpaces = eFalse;
            }
          }
          else {
            if (remLeadingSpaces > 0 && (CUR_CHAR != '\n') && StrIsSpace(CUR_CHAR)) {
              --remLeadingSpaces;
              NEXT();
              continue; // skip this leading space
            }
            else {
              // not a leading space, clear "remLeadingSpaces"
              remLeadingSpaces = 0;
            }
          }
        }
        _longstr.appendChar(CUR_CHAR);
        NEXT();
        break;
      }
    }
  }

  // skip last empty new line
  if (_longstr.size() >= 1 &&
      _longstr[_longstr.size()-1] == '\n')
  {
    _longstr.resize(_longstr.size()-1);
  }

  _longstr.appendChar(_A('\0'));
  _svalue = _longstr.Chars();
  return eCompileResult_OK;
}

eCompileResult SQLexer::ReadString(sCompileErrors& aErrors, int* apTok, tU32 ndelim, ni::tBool abFormat, int preChar)
{
  INIT_TEMP_STRING();
  NEXT();
  if (preChar != 0) {
    APPEND_CHAR(preChar);
  }

  // handle triple double quote rawstring
  if (ndelim == '\"') {
    if (CUR_CHAR == '\"') { // two double quotes
      NEXT();
      if (CUR_CHAR == '\"') { // three double quotes
        NEXT();
        COMPILE_CHECK(ReadRawString(aErrors,preChar,eReadRawStringMode_TripleDoubleQuote));
      }
      else {
        // "" -> return an empty string
        _svalue = _longstr.Chars();
      }
      if (apTok) {
        *apTok = TK_STRING_LITERAL;
      }
      return eCompileResult_OK;
    }
  }

  if (IS_EOB()) {
    LEXER_ERROR("unexpected end of file");
  }

  for (;;)
  {
    while (CUR_CHAR != ndelim)
    {
      switch(CUR_CHAR)
      {
        case SQUIRREL_EOB:
          LEXER_ERROR("unexpected end of file");
        case _A('\n'):
          if (abFormat) {
            LEXER_ERROR("newline in a regular string constant file");
            break;
          }
        case _A('\\'):
          if (abFormat) {
            NEXT();
            switch(CUR_CHAR)
            {
              case _A('t'): APPEND_CHAR(_A('\t')); NEXT(); break;
              case _A('a'): APPEND_CHAR(_A('\a')); NEXT(); break;
              case _A('b'): APPEND_CHAR(_A('\b')); NEXT(); break;
              case _A('n'): APPEND_CHAR(_A('\n')); NEXT(); break;
              case _A('r'): APPEND_CHAR(_A('\r')); NEXT(); break;
              case _A('v'): APPEND_CHAR(_A('\v')); NEXT(); break;
              case _A('f'): APPEND_CHAR(_A('\f')); NEXT(); break;
              case _A('0'): APPEND_CHAR(_A('\0')); NEXT(); break;
              case _A('\\'): APPEND_CHAR(_A('\\')); NEXT(); break;
              case _A('"'): APPEND_CHAR(_A('"')); NEXT(); break;
              case _A('\''): APPEND_CHAR(_A('\'')); NEXT(); break;
              default:
                LEXER_ERROR(niFmt("unrecognized escape character '%c' (%d)",
                                  CUR_CHAR,CUR_CHAR));
            }
            break;
          }
        default:
          APPEND_CHAR(CUR_CHAR);
          NEXT();
          break;
      }
    }

    NEXT();
    break;
  }

  TERMINATE_BUFFER();
  if (ndelim == _A('\'')) {
    int len = _longstr.length();
    if (len == 0) {
      LEXER_ERROR("empty constant");
    }
    if (len > 2048) {
      niFmt(_A("constant '%s' (%d) too long"),_longstr,len);
    }
    _nvalue = _longstr[0];
    if (apTok) {
      *apTok = TK_INTEGER;
    }
  }
  else {
    _svalue = _longstr.Chars();
    if (apTok) {
      *apTok = TK_STRING_LITERAL;
    }
  }

  return eCompileResult_OK;
}

eCompileResult SQLexer::ReadNumber(sCompileErrors& aErrors, int* apTok, int preChar)
{
#define TINT 1
#define TFLOAT 2
#define THEX 3
#define TSCIENTIFIC 4
  INIT_TEMP_STRING();
  bool negate = (preChar == '-');
  int type = TINT, firstchar = CUR_CHAR;
  const SQChar *sTemp;
  NEXT();
  if (firstchar == _A('0') && toupper(CUR_CHAR) == _A('X'))
  {
    NEXT();
    type = THEX;
    while (isxdigit(CUR_CHAR)) {
      APPEND_CHAR(CUR_CHAR);
      NEXT();
    }
    if (_longstr.size() > 8) {
      LEXER_ERROR("Hexadecimal number over 8 digits");
    }
  }
  else
  {
    APPEND_CHAR(firstchar);
    if (firstchar == _A('.'))
      type = TFLOAT;

    while (CUR_CHAR == _A('.') || StrIsDigit(CUR_CHAR) ||
           CUR_CHAR == _A('e') || CUR_CHAR == _A('E'))
    {
      if (CUR_CHAR == _A('.'))
      {
        if (type == TSCIENTIFIC) {
          LEXER_ERROR("Invalid scientific number format, no '.' allowed in the exponent");
        }
        type = TFLOAT;
      }

      if (CUR_CHAR == _A('e') || CUR_CHAR == _A('E'))
      {
        if (type != TFLOAT && type != TINT) {
          LEXER_ERROR("Invalid scientific number format");
        }
        type = TSCIENTIFIC;
        APPEND_CHAR('e');
        NEXT();
        if (CUR_CHAR == '+' || CUR_CHAR == '-')
        {
          APPEND_CHAR(CUR_CHAR);
          NEXT();
        }
        if (!StrIsDigit(CUR_CHAR)) {
          LEXER_ERROR("Exponent expected in scientific number");
        }
      }
      APPEND_CHAR(CUR_CHAR);
      NEXT();
    }
  }
  TERMINATE_BUFFER();
  switch(type) {
    case TSCIENTIFIC:
    case TFLOAT:
      _fvalue = (SQFloat)ni::StrToD(_longstr.Chars(),&sTemp);
      if (negate)
        _fvalue = -_fvalue;
      if (apTok)
        *apTok = TK_FLOAT;
      break;
    case TINT:
      _nvalue = (SQInt)ni::StrAToL(_longstr.Chars());
      if (negate)
        _nvalue = -_nvalue;
      if (apTok)
        *apTok = TK_INTEGER;
      break;
    case THEX:
      *((tU32*)&_nvalue) = (tU32)ni::StrToUL(_longstr.Chars(),&sTemp,16);
      if (negate)
        _nvalue = -_nvalue;
      if (apTok)
        *apTok = TK_INTEGER;
      break;
    default:
      INTERNAL_ERROR("Unknown number format.");
  }
  return eCompileResult_OK;
}

int SQLexer::FinalizeSExpStringLiteral(const int aSExpType, const tU32 aSymbolPrefix, const achar* kind)
{
  _prevSExpKind = kind;
  if (aSExpType == STATE_SEXP2) {
    const int len = _longstr.size();
    _longstr = niFmt("%c%s[src=%s,line=%d,col=%d,kind=%s]",
                     aSymbolPrefix,
                     _longstr,
                     _fileName,
                     GetLastTokenLine(),
                     _currentcolumn-len,
                     kind);
    _svalue = _longstr.Chars();
  }
  else {
    _svalue = _longstr.Chars();
  }
  return TK_STRING_LITERAL;
}

int SQLexer::ReadSExpID(int preChar, const int aSExpType, const tU32 anPrevChar)
{
  INIT_TEMP_STRING();
  if (preChar) {
    APPEND_CHAR(preChar);
  }

  tU32 symbolPrefix = '$';
  const char* kind;

  const tBool parseOperator = _IsSExpOperator(preChar) || _IsSExpOperator(CUR_CHAR);
  if (parseOperator) {
    while (_IsSExpOperator(CUR_CHAR)) {
      APPEND_CHAR(CUR_CHAR);
      NEXT();
    }

    const tBool bPrevIsSpace = StrIsSpace(anPrevChar) || _IsSExpPunctuation(anPrevChar);
    const tBool bNextIsSpace = StrIsSpace(CUR_CHAR) || _IsSExpPunctuation(CUR_CHAR);
    if (bPrevIsSpace && !bNextIsSpace) {
      // special case for numbers declared with prefix +/-
      if (_longstr.size() == 1 && (_longstr[0] == '+' || _longstr[0] == '-') &&
          StrIsDigit(CUR_CHAR))
      {
        kind = "number";
        symbolPrefix = '#';
        for (;;) {
          if (CUR_CHAR != '.' &&
              (StrIsSpace(CUR_CHAR) ||
               _IsSExpOperator(CUR_CHAR) ||
               _IsSExpPunctuation(CUR_CHAR)))
          {
            break;
          }
          APPEND_CHAR(CUR_CHAR);
          NEXT();
        }
      }
      else {
        // postfix operator
        kind = "op_pre";
      }
    }
    else if (!bPrevIsSpace && bNextIsSpace) {
      // postfix operator
      kind = "op_post";
    }
    else /*if (bPrevIsSpace == bNextIsSpace)*/ {
      kind = "op_bin";
    }
  }
  else
  {
    if (StrIsDigit(CUR_CHAR)) {
      kind = "number";
      symbolPrefix = '#';
      for (;;) {
        if (CUR_CHAR != '.' &&
            (StrIsSpace(CUR_CHAR) ||
             _IsSExpOperator(CUR_CHAR) ||
             _IsSExpPunctuation(CUR_CHAR)))
        {
          break;
        }
        APPEND_CHAR(CUR_CHAR);
        NEXT();
      }
    }
    else {
      kind = "id";
      for (;;) {
        if (StrIsSpace(CUR_CHAR) ||
            _IsSExpOperator(CUR_CHAR) ||
            _IsSExpPunctuation(CUR_CHAR))
        {
          break;
        }
        APPEND_CHAR(CUR_CHAR);
        NEXT();
      }
    }
  }

  TERMINATE_BUFFER();
  return FinalizeSExpStringLiteral(aSExpType,symbolPrefix,kind);
}

int SQLexer::ReadScriptID()
{
  INIT_TEMP_STRING();
  do {
    APPEND_CHAR(CUR_CHAR);
    NEXT();
  } while (_IsIdentifier(CUR_CHAR));
  TERMINATE_BUFFER();
  int res = GetIDType(_longstr.Chars());
  if (res == TK_IDENTIFIER ||
      res == TK_STRING_LITERAL) {
    _svalue = _longstr.Chars();
  }
  return res;
}

void SQLexer::ReadCompilerCommand() {
  INIT_TEMP_STRING();
  do {
    APPEND_CHAR(CUR_CHAR);
    NEXT();
  } while (CUR_CHAR != _A('\n') && (!IS_EOB()));
  TERMINATE_BUFFER();
  _longstr.Normalize();
  if (_longstr.IEq(_A("debug:on"))) {
    _compilerCommands.debug = true;
    //         TRACE_LEXER((_A("-- DEBUG: ON")));
  }
  else if (_longstr.IEq(_A("debug:off"))) {
    //         TRACE_LEXER((_A("-- DEBUG: OFF")));
  }
}

int SQLexer::GetLastTokenLine() const {
  // if CUR_CHAR == \n _currentline already has been incremented but the actual 'current line' to display errors, etc... is still the previous line
  return (CUR_CHAR == '\n') ? _lasttokenline : _currentline;
}
