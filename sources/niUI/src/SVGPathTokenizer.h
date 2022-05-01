#ifndef __SVGPATHTOKENIZER_59D26DF6_D99D_449D_A78C_D8C0D3625E22_H__
#define __SVGPATHTOKENIZER_59D26DF6_D99D_449D_A78C_D8C0D3625E22_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#if niMinFeatures(20)

#include "AGG.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// SVG path tokenizer.
// Example:
//
// agg::svg::cSVGPathTokenizer tok;
//
// tok.set_str("M-122.304 84.285L-122.304 84.285 122.203 86.179 ");
// while(tok.next())
// {
//     printf("command='%c' number=%f\n",
//             tok.last_command(),
//             tok.last_number());
// }
//
// The tokenizer does all the routine job of parsing the SVG paths.
// It doesn't recognize any graphical primitives, it even doesn't know
// anything about pairs of coordinates (X,Y). The purpose of this class
// is to tokenize the numeric values and commands. SVG paths can
// have single numeric values for Horizontal or Vertical line_to commands
// as well as more than two coordinates (4 or 6) for Bezier curves
// depending on the semantics of the command.
// The behaviour is as follows:
//
// Each call to next() returns true if there's new command or new numeric
// value or false when the path ends. How to interpret the result
// depends on the sematics of the command. For example, command "C"
// (cubic Bezier curve) implies 6 floating point numbers preceded by this
// command. If the command assumes no arguments (like z or Z) the
// the last_number() values won't change, that is, last_number() always
// returns the last recognized numeric value, so does last_command().
//===============================================================
class cSVGPathTokenizer
{
 public:
  cSVGPathTokenizer();

  void SetPathString(const char* str);
  tBool Next();

  tF32 Next(char cmd);

  char LastCommand() const { return m_last_command; }
  tF32 LastNumber() const { return (tF32)m_last_number; }

  tBool HasError() const { return mbError; }

 private:
  static void InitCharMask(char* mask, const char* char_set);

  bool Contains(const char* mask, unsigned c) const
  {
    return (mask[(c >> 3) & (256/8-1)] & (1 << (c & 7))) != 0;
  }

  bool IsCommand(unsigned c) const
  {
    return Contains(m_commands_mask, c);
  }

  bool IsNumeric(unsigned c) const
  {
    return Contains(m_numeric_mask, c);
  }

  bool IsSeparator(unsigned c) const
  {
    return Contains(m_separators_mask, c);
  }

  tBool ParseNumber();

  char m_separators_mask[256/8];
  char m_commands_mask[256/8];
  char m_numeric_mask[256/8];

  const char* m_path;
  agg_real m_last_number;
  char   m_last_command;
  tBool mbError;

  static const char s_commands[];
  static const char s_numeric[];
  static const char s_separators[];
};

#endif // niMinFeatures

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __VG_317650_H__
