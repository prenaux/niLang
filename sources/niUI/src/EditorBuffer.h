#ifndef __EDITORBUFFER_52204276_H__
#define __EDITORBUFFER_52204276_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "Line.h"

class EditorBuffer;

typedef astl::vector<Line> LineVec;
typedef LineVec::iterator LineVecItor;

enum eEditorBufferMessage {
  eEditorBufferMessage_Modified = niMessageID('_','E','B','M','m')
};

enum eSelectionType{
  stNormal,
  stRange
};

struct Position
{
  Position() { _line = 0; _col = 0; }
  Position(unsigned int line,unsigned int col) { _line = line; _col = col; }
  bool operator <(const Position &other)
  {
    if(_line<other._line)return true;
    if(_line==other._line){
      return _col<other._col;
    }
    return false;
  }
  bool operator ==(const Position &other) const
  {
    return (_line==other._line && _col==other._col);
  }
  bool operator !=(const Position &other) const
  {
    return (_line!=other._line || _col!=other._col);
  }
  unsigned int _line;
  unsigned int _col;
};

struct Selection {
  Selection() {
    _type=stNormal;
  }
  Selection(Position a,Position b,eSelectionType type) {
    _type=type;
    if(!(a<b)) {
      _start=b;
      _end=a;
    }
    else {
      _start=a;
      _end=b;
    }
  }
  bool operator ==(const Selection &other) const
  {
    return (_start==other._start && _end==other._end && _type==other._type);
  }
  bool IsEmpty() {
    return (_start._col==_end._col) && (_start._line==_end._line);
  }
  bool IsIn(unsigned int line)
  {
    return (line >= _start._line && line <= _end._line);
  }
  bool IsIn(unsigned int line,unsigned int col)
  {
    if(IsIn(line)){
      if(_start._line!=_end._line) {
        if(line == _start._line) {
          return col>=_start._col;
        }
        else if(line == _end._line) {
          return col<_end._col;
        }
        else return true;
      } else {
        return (col >= _start._col && col < _end._col);
      }
    }
    return false;
  }
  void Merge(Position &pivot, Position &pos)
  {

    if(pivot==_start)
      *this=Selection(pos,_start,_type);
    else
      *this=Selection(pos,_end,_type);
  }
  Position _start;
  Position _end;
  eSelectionType _type;
};

class EditorBuffer
{
 public:
  EditorBuffer(iWidgetSink* apMsgHandler);
  ~EditorBuffer(void);
  void Clear() {_lines.resize(0);_cursor=Position(); }
  void PushLine(Line &line);
  void PushLine() {
    Line l(_defaultnewline);
    PushLine(l);
  }
  //data
  bool AddChar(tU32 c);
  bool AddText(const achar* aaszText);
  bool AddNewLine();
  bool DeleteChar();
  bool BackSpace();

  //
  Position &SetCursorPos(const Position &pos,bool forcelogicalcol);
  bool GetSelectionData(const Selection &sel,cString &dest) const;
  //pos
  Position GetClosestValidPos(const Position &pos);

  int GetLineLen(unsigned int line) const;
  int GetLinesCount() const;
  void DeleteSelectionContent(const Selection &sel);
  const achar *GetLineData(int line) const;
  void SetLineData(unsigned int line,const achar *data);
  int GetLineCount() const;
  const Position &GetCursor() const;
  int GetLogicalCursorCol() const;
  const LineVec &GetLines() const;
  const Line &GetLine(unsigned int line) const;
  Position GetStartPosition() const;
  Position GetEndPosition() const;
  void _NotifyModified();

 private:
  LineVec _lines;
  Position _cursor;
  int _logicalcursorcol;
  eNewLine _defaultnewline;
  WeakPtr<iWidgetSink> _msgHandler;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __EDITORBUFFER_52204276_H__
