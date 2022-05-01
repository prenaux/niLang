// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("lang.ni")

local module = {
  ///////////////////////////////////////////////
  // Create a new file pattern based regular expression
  function filePattern(aStr) {
    return ::CreateInstance("niLang.FilePatternRegex",aStr,"|,")
  }

  ///////////////////////////////////////////////
  // Create a new pcre regular expression
  function pcre(aStr,aOptions) {
    return ::CreateInstance("niLang.PCRE",aStr,aOptions);
  }
}
::namespaceOrModule(this, "regex", module);
