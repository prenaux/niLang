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

function GenerateModule(aSource) {
  local dt = ::lang.loadDataTable("",aSource)
  try {
    ::println("- Generating IDLs for Module '"+dt.string.name+"'\n  from '"+aSource+"'.")
    for (local i = 0; i < dt.num_children; ++i) {
      local cdt = dt.child_from_index[i]
      if (cdt.name != "include")
        continue
      local d = cdt.metadata
      if (d) {
        local i = d.rfind(".idl")
        if (i) {
          local s = aSource.getdir() + d.slice(0,i)
          local d = aSource.getdir() + "_idl/" + d.slice(0,i).getfile() + ".idl.xml";
          ::idlGen.generate(s,d,d.setext("inl"))
        }
      }
    }
  }
  catch (e) {
    throw ("Error generating IDL for module '"+aSource+"':\n"+e)
  }
}

try {
  local args = ::?GetArgs();
  switch (args.GetSize()) {
    case 2: {
      GenerateModule(args[1]);
      break;
    }
    case 3: {
      Generate(args[1],args[2],args[2].setext("inl"));
      break;
    }
    case 4: {
      Generate(args[1],args[2],args[3]);
      break;
    }
    default: {
      throw {[
Invalid arguments, should be:
  - [MODULE] or
  - [SRC.h] [DEST.idl.xml] or
  - [SRC.h] [DEST.idl.xml] [DEST.idl.inl]
      ]}
      break;
    }
  }
}
catch (e) {
  ::println("E/" + e)
  ::gLang.Exit(1)
}
