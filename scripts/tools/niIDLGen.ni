// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("lang.ni")
::Import("tools/IDLGen.ni")

function Generate(aSource,aDest,aDestInl)
{
  try {
    ::idlGen.generate(aSource,aDest,aDestInl)
  }
  catch (e) {
    throw ("Error generating IDL for '"+aSource+"':\n"+e)
  }
}

try {
  local args = ::?GetArgs();
  switch (args.GetSize()) {
    case 3: {
      Generate(args[1],args[2],args[2].setext("inl"));
      break;
    }
    case 4: {
      Generate(args[1],args[2],args[3]);
      break;
    }
    default: {
      throw """
Invalid arguments, should be:
- [SRC.h] [DEST.idl.xml] or
- [SRC.h] [DEST.idl.xml] [DEST.idl.inl]
"""
      break;
    }
  }
}
catch (e) {
  ::gLang.Exit(1)
}
