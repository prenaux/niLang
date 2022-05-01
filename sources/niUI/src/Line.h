#ifndef __LINE_10198240_H__
#define __LINE_10198240_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

enum eNewLine
{
  eNewLine_CR,
  eNewLine_LF,
  eNewLine_CRLF,
  eNewLine_Platform,
  eNewLine_Auto
};

inline const achar *_GetEOL(eNewLine aNewLine)
{
  switch(aNewLine) {
    case eNewLine_CR: return _A("\r");
    case eNewLine_LF: return _A("\n");
    case eNewLine_CRLF: return _A("\r\n");
    case eNewLine_Platform:
#ifdef niWindows
      return _A("\r\n");
#else
      return _A("\n");
#endif
  }
  niAssert(0 && "Unreachable.");
  return NULL;
}

class Line
{
 public:
  Line();
  Line(eNewLine nl);
  Line(const Line &o);
  Line(const achar *data,eNewLine nl);
  virtual ~Line(void);

  bool AppendChar(tU32 c);
  bool InsertChar(unsigned int col,tU32 c);
  bool AddNewLine(unsigned int col,Line &newline);
  const achar *GetEOL(eNewLine aNL = eNewLine_Auto) const;
  tU32 GetAt(tU32 c) const;
  void EraseChar(tU32 anCol);
  void EraseRange(tU32 anStartCol, tU32 anEndCol);
  void EraseCount(tU32 anStartCol, tU32 anCount);
  tU32 GetLength() const;
  tU32 GetSizeInBytes() const;
  void SetData(const cString& str);
  void SetData(const achar* aaszData);
  void AppendData(const cString& str);
  void AppendData(const achar* aaszData);
  tBool IsEmpty() const { return _data.empty(); }
  const achar* GetChars() const { return _data.Chars(); }
  const cString& GetStr() const { return _data; }
  void SetNewLine(eNewLine aNL) { _newline = aNL; }
  eNewLine GetNewLine() const { return _newline; }

 private:
  cString _data;
  eNewLine _newline;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __LINE_10198240_H__
