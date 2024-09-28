// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("lang.ni")

module <- {
  ///////////////////////////////////////////////
  // Create a new file pattern based regular expression
  function filePattern(aStr) iRegex {
    return ::CreateInstance("niLang.FilePatternRegex",aStr,"|,").QueryInterface("iRegex");
  }

  ///////////////////////////////////////////////
  // Create a new pcre regular expression
  function pcre(aStr,_aOptions) iPCRE {
    return ::CreateInstance("niLang.PCRE",aStr,_aOptions).QueryInterface("iPCRE");
  }
}

::regex <- module
