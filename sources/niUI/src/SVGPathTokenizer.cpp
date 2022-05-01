// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"

#if niMinFeatures(20)

#include "SVGPathTokenizer.h"

//------------------------------------------------------------------------
const char cSVGPathTokenizer::s_commands[]   = "+-MmZzLlHhVvCcSsQqTtAaFfPp";
const char cSVGPathTokenizer::s_numeric[]    = ".Ee0123456789";
const char cSVGPathTokenizer::s_separators[] = " ,\t\n\r";

//------------------------------------------------------------------------
cSVGPathTokenizer::cSVGPathTokenizer()
    : m_path(0), m_last_command(0), m_last_number(0.0), mbError(eFalse)
{
  InitCharMask(m_commands_mask,   s_commands);
  InitCharMask(m_numeric_mask,    s_numeric);
  InitCharMask(m_separators_mask, s_separators);
}


//------------------------------------------------------------------------
void cSVGPathTokenizer::SetPathString(const char* str)
{
  m_path = str;
  m_last_command = 0;
  m_last_number = 0.0;
}


//------------------------------------------------------------------------
void cSVGPathTokenizer::InitCharMask(char* mask, const char* char_set)
{
  memset(mask, 0, 256/8);
  while(*char_set)
  {
    unsigned c = unsigned(*char_set++) & 0xFF;
    mask[c >> 3] |= 1 << (c & 7);
  }
}


//------------------------------------------------------------------------
tBool cSVGPathTokenizer::Next()
{
  if (mbError || m_path == 0)
    return eFalse;

  // Skip all white spaces and other garbage
  while(*m_path && !IsCommand(*m_path) && !IsNumeric(*m_path))
  {
    if (!IsSeparator(*m_path)) {
      mbError = eTrue;
      niError(niFmt(_A("Invalid character '%c' !"),*m_path));
      return eFalse;
    }
    m_path++;
  }

  if (*m_path == 0)
    return eFalse;

  if (IsCommand(*m_path))
  {
    // Check if the command is a numeric sign character
    if (*m_path == '-' || *m_path == '+') {
      return ParseNumber();
    }
    m_last_command = *m_path++;
    while (*m_path && IsSeparator(*m_path))
      m_path++;
    if (*m_path == 0)
      return eTrue;
  }
  return ParseNumber();
}



//------------------------------------------------------------------------
tF32 cSVGPathTokenizer::Next(char cmd)
{
  if (mbError)
    return -1;

  if (!Next()) {
    mbError = eTrue;
    niError(_A("Unexpected end of path !"));
    return -1;
  }
  if (LastCommand() != cmd)
  {
    mbError = eTrue;
    niError(niFmt(_A("Command '%c': bad or missing parameters"), cmd));
    return -1;
  }
  return LastNumber();
}


//------------------------------------------------------------------------
tBool cSVGPathTokenizer::ParseNumber()
{
  char buf[256]; // Should be enough for any number
  char* buf_ptr = buf;

  // Copy all sign characters
  while (buf_ptr < buf+255 && (*m_path == '-' || *m_path == '+')) {
    *buf_ptr++ = *m_path++;
  }

  // Copy all numeric characters
  while(buf_ptr < buf+255 && IsNumeric(*m_path)) {
    *buf_ptr++ = *m_path++;
  }

  *buf_ptr = 0;
  m_last_number = agg_real(atof(buf));
  return eTrue;
}

#endif // niMinFeatures
