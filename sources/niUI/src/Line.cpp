// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Line.h"

Line::Line()
{
  _newline = eNewLine_CRLF;
}

Line::Line(eNewLine nl)
{
  _newline=nl;
}

Line::Line(const Line &o)
{
  _data=o._data;
  _newline=o._newline;
}

Line::Line(const achar *data,eNewLine nl)
{
  _data=data;
  _newline=nl;
}

Line::~Line(void)
{
}

bool Line::AppendChar(tU32 c)
{
  _data.appendChar(c);
  return true;
}

bool Line::InsertChar(unsigned int col,tU32 c)
{
  if(col>_data.length()) return false;
  _data.insert(StrOffset(_data.Chars(),col),c);
  return true;
}

bool Line::AddNewLine(unsigned int col,Line &newline)
{
  if (col > (tU32)StrLen(_data.Chars()))
    return false;
  newline._data = _data.charIt(col).current();
  _data = _data.charIt(0,col);
  return true;
}

const achar *Line::GetEOL(eNewLine aNewLine) const
{
  return _GetEOL((aNewLine == eNewLine_Auto)?_newline:aNewLine);
}

tU32 Line::GetAt(tU32 c) const {
  return StrGetAt(_data.Chars(),c);
}

void Line::EraseChar(tU32 anCol) {
  _data.eraseCharAt(anCol);
}

void Line::EraseRange(tU32 anStartCol, tU32 anEndCol) {
  // erase range
  const tU32 startOffset = StrOffset(_data.Chars(),anStartCol);
  const tU32 endOffset = StrOffset(_data.Chars(),anEndCol);
  _data.erase(startOffset,endOffset-startOffset);
}

void Line::EraseCount(tU32 anStartCol, tU32 anCount) {
  // erase count
  const tU32 startOffset = StrOffset(_data.Chars(),anStartCol);
  const tU32 endOffset = StrOffset(_data.Chars()+startOffset,anCount)+startOffset;
  _data.erase(startOffset,endOffset-startOffset);
}

tU32 Line::GetLength() const {
  return StrLen(_data.Chars());
}

tU32 Line::GetSizeInBytes() const {
  return _data.size();
}

void Line::SetData(const cString& str) {
  _data = str;
}

void Line::SetData(const achar* aaszData) {
  _data = aaszData;
}

void Line::AppendData(const cString& str) {
  _data.append(str);
}

void Line::AppendData(const achar* aaszData) {
  _data.append(aaszData);
}
