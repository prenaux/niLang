// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Line.h"
#include "EditorBuffer.h"

EditorBuffer::EditorBuffer(iWidgetSink* apMsgHandler)
{
  _msgHandler = apMsgHandler;
  _defaultnewline=eNewLine_LF;
  _logicalcursorcol = 0;
}

EditorBuffer::~EditorBuffer(void)
{
}

void EditorBuffer::PushLine(Line &line)
{
  _lines.push_back(line);
}

Position EditorBuffer::GetClosestValidPos(const Position &pos)
{
  Position ret;
  if(pos._line>=_lines.size())
    ret._line=(unsigned int)_lines.size()-1;
  else
    ret._line=pos._line;

  Line &line=_lines[ret._line];
  if (pos._col > line.GetLength())
    ret._col = (unsigned int)line.GetLength();
  else
    ret._col=pos._col;
  return ret;
}

Position &EditorBuffer::SetCursorPos(const Position &pos,bool forcelogicalcol)
{
  _cursor=GetClosestValidPos(pos);
  if(forcelogicalcol)
    _logicalcursorcol=_cursor._col;
  return _cursor;
}


bool EditorBuffer::AddChar(tU32 c)
{
  if(_cursor._line>=_lines.size())
    return false;
  Line &line=_lines[_cursor._line];
  if(line.InsertChar(_cursor._col,c)) {
    _cursor._col++;
  }
  _NotifyModified();
  return true;
}

bool EditorBuffer::AddText(const achar* aaszText)
{
  if (!niIsStringOK(aaszText))
    return false;

  StrCharIt itText(aaszText);
  while (!itText.is_end()) {
    const tU32 ch = itText.next();
    if (ch == '\r') {
      if (itText.peek_next() != '\n') {
        // '\r' alone, add a new line...
        if (!AddNewLine())
          return false;
      }
      else {
        // '\r\n', skip the \r, \n will add the new line...
      }
    }
    else if (ch == '\n') {
      if (!AddNewLine())
        return false;
    }
    else {
      if (!AddChar(ch))
        return false;
    }
  }

  return true;
}

bool EditorBuffer::DeleteChar()
{
  if(_cursor._line >= _lines.size())
    return false;
  Line &line = _lines[_cursor._line];
  if (_cursor._col >= line.GetLength()) {
    if(_lines.size()-1 > _cursor._line) {
      Line &other = _lines[_cursor._line+1];
      line.AppendData(other.GetStr());
      line.SetNewLine(other.GetNewLine());
      _lines.erase(_lines.begin() + (_cursor._line + 1));
      _NotifyModified();
      return true;
    }
  }
  else {
    line.EraseChar(_cursor._col);
  }
  _NotifyModified();
  return true;
}

bool EditorBuffer::BackSpace()
{
  if(_cursor._line>=_lines.size())
    return false;
  Line &line=_lines[_cursor._line];
  if(_cursor._col>0){
    line.EraseChar(_cursor._col-1);
    _cursor._col--;
    _logicalcursorcol=_cursor._col;
    _NotifyModified();
  }
  else{
    if(_cursor._line>0){
      Line &other = _lines[_cursor._line-1];
      _cursor._col = (int)other.GetLength();
      other.AppendData(line.GetStr());
      other.SetNewLine(line.GetNewLine());
      _lines.erase(_lines.begin()+_cursor._line);
      _cursor._line--;
      _logicalcursorcol=_cursor._col;
      _NotifyModified();
      return true;
    }
    return false;
  }
  return true;
}

///////////////////////////////////////////////
bool  EditorBuffer::AddNewLine()
{
  if(_cursor._line>=_lines.size())
    return false;
  Line &line=_lines[_cursor._line];
  Line newline(_defaultnewline);
  if(!line.AddNewLine(_cursor._col,newline))
    return false;
  _lines.insert(_lines.begin()+_cursor._line+1,newline);
  _cursor._line++;
  _logicalcursorcol=_cursor._col=0;
  _NotifyModified();
  return true;
}

void EditorBuffer::DeleteSelectionContent(const Selection &sel)
{
  if(sel._start._line==sel._end._line) {
    Line &line=_lines[sel._start._line];
    line.EraseRange(sel._start._col,sel._end._col);
  } else {
    int toremove=0;
    int startremove=sel._start._line;
    tU32 curline=sel._start._line;
    curline++;
    while(curline!=sel._end._line) {
      if(toremove==0) {
        startremove=curline;
      }
      toremove++;
      curline++;
    }
    if(toremove>0) {
      _lines.erase(_lines.begin()+startremove,_lines.begin()+(startremove+toremove));
    }
    //erase the end start
    Line &end=_lines[sel._start._line+1];
    Line &start=_lines[sel._start._line];
    if (sel._end._col>0) {
      end.EraseCount(0,end.GetLength()-(end.GetLength()-sel._end._col));
    }
    start.EraseCount(sel._start._col,start.GetLength()-sel._start._col);
    start.AppendData(end.GetStr());
    start.SetNewLine(end.GetNewLine());
    _lines.erase(_lines.begin()+(sel._start._line+1));
  }
  _NotifyModified();
}

bool EditorBuffer::GetSelectionData(const Selection &sel,cString &dest) const
{
  dest.clear();
  if(sel._start._line==sel._end._line) {
    const Line &line=_lines[sel._start._line];
    const tU32 selStartOffset = StrOffset(line.GetChars(),sel._start._col);
    const tU32 selEndOffset = StrOffset(line.GetChars(),sel._end._col);
    dest.appendEx(&line.GetChars()[selStartOffset],selEndOffset-selStartOffset);
  }else {
    const Line &end=_lines[sel._end._line];
    const Line &start=_lines[sel._start._line];
    const tU32 selStartOffset = StrOffset(start.GetChars(),sel._start._col);
    const tU32 selEndOffset = StrOffset(end.GetChars(),sel._end._col);
    dest.appendEx(&start.GetChars()[selStartOffset],start.GetSizeInBytes()-selStartOffset);
    dest.append(start.GetEOL(eNewLine_Platform));
    tU32 curline = sel._start._line+1;
    while (curline!=sel._end._line) {
      dest.append(_lines[curline].GetChars());
      dest.append(_lines[curline].GetEOL(eNewLine_Platform));
      curline++;
    }
    dest.appendEx(end.GetChars(),selEndOffset);
  }
  return true;
}

int EditorBuffer::GetLineLen(unsigned int line) const {
  if(line>_lines.size())
    return -1;
  return _lines[line].GetLength();
}

int EditorBuffer::GetLinesCount() const {
  return (int)_lines.size();
}

const achar* EditorBuffer::GetLineData(int line) const {
  return _lines[line].GetChars();
}
void EditorBuffer::SetLineData(unsigned int line,const achar *data) {
  if (line < _lines.size()) {
    _lines[line].SetData(data);
  }
}
int EditorBuffer::GetLineCount() const {
  return (int)_lines.size();
}
const Position &EditorBuffer::GetCursor() const {
  return _cursor;
}
int EditorBuffer::GetLogicalCursorCol() const {
  return _logicalcursorcol;
}
const LineVec &EditorBuffer::GetLines() const {
  return _lines;
}
const Line &EditorBuffer::GetLine(unsigned int line) const {
  return line>=_lines.size()?_lines[_lines.size()-1]:_lines[line];
}
Position EditorBuffer::GetStartPosition() const {
  return Position(0,0);
}
Position EditorBuffer::GetEndPosition() const {
  return GetLineCount()?Position(GetLineCount()-1,GetLineLen(GetLineCount()-1)):Position(0,0);
}

void EditorBuffer::_NotifyModified() {
  QPtr<iWidgetSink> handler = _msgHandler;
  if (handler.IsOK()) {
    handler->OnWidgetSink(NULL,eEditorBufferMessage_Modified,niVarNull,niVarNull);
  }
}
