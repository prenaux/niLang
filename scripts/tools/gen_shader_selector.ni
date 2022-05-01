//
// ni _gen_shader_selector.ni
//
// This is meant to be called after you added & compiled new light shader types
//
::Import("fs.ni");
::Import("zip.ni");

local tGenSelector = {
  _out = ""
  _indentation = 0
  _indentString = ""

  function addLine(msg) {
    _out.WriteString(_indentString + msg + "\n");
  }

  function addEmptyLine() {
    _out.WriteString("\n");
  }

  function indent() {
    ++_indentation;
    updateIndentation()
  }
  function unindent() {
    --_indentation;
    updateIndentation()
  }
  function updateIndentation() {
    _indentString = ""
    foreach (i in _indentation) {
      _indentString += "  "
    }
  }

  //
  // The macros are declared in PainterShaders.h
  //
  // The order reflects the depth of the if() test, it should reflect where
  // the name appears, lowest order should be the flags to the left of the
  // name.
  //
  _selectors = {
    // ["2side"] = {
      // order = 100
      // test = "SHADER_MATERIAL_HAS_FLAG(DoubleSided)"
    // },
    ["tilemap"] = {
      order = 101
      test = "SHADER_MATERIAL_HAS_FLAG(TileMap)"
    },
    ["decal"] = {
      order = 102
      test = "SHADER_MATERIAL_HAS_FLAG(Decal)"
    },
    ["refract"] = {
      order = 102
      test = "SHADER_MATERIAL_HAS_FLAG(Refraction)"
    },

    ["env"] = {
      order = 1000
      test = "SHADER_MATERIAL_HAS_CHANNEL(Environment)"
    },
    ["hmap"] = {
      order = 1001
      test = "SHADER_MATERIAL_HAS_CHANNEL(Bump) && SHADER_MATERIAL_HAS_FLAG(HeightMap)"
    },
    ["nmap"] = {
      order = 1002
      test = "SHADER_MATERIAL_HAS_CHANNEL(Bump) && SHADER_MATERIAL_HAS_FLAG(NormalMap)"
    },
    ["bump"] = {
      order = 1003
      test = "SHADER_MATERIAL_HAS_CHANNEL(Bump)"
    },
    ["spec"] = {
      order = 1004
      test = "SHADER_MATERIAL_HAS_CHANNEL(Specular)"
    },
    ["ambient"] = {
      order = 1005
      test = "SHADER_MATERIAL_HAS_CHANNEL(Ambient)"
    },
    ["base"] = {
      order = 1006
      test = "SHADER_MATERIAL_HAS_CHANNEL(Base)"
    },
    ["opacity"] = {
      order = 1007
      test = "SHADER_MATERIAL_HAS_CHANNEL(Opacity)"
    },
    // distancefield overrides translucent & alphatest
    ["distancefield"] = {
      order = 1008
      test = "SHADER_MATERIAL_HAS_FLAG(DistanceField)"
    },
    // translucent overrides alphatest
    ["translucent"] = {
      order = 1009
      test = "SHADER_MATERIAL_HAS_FLAG(Translucent)"
    },
    ["alphatest"] = {
      order = 1010
      test = "SHADER_MATERIAL_HAS_FLAG(Transparent)"
    },

    // emmissive_ps
    ["emissive"] = {
      order = 0
      test = "SHADER_MATERIAL_HAS_CHANNEL(Emissive)"
    },

    // shadowmap_ps
    ["smpacked"] = {
      order = 0
      test = "SHADER_MATERIAL_IS_SMPACKED()"
    },

    // lit
    ["shadows"] = {
      order = 10
      test = "SHADER_LIGHT_HAS_FLAG(CastShadows)"
    },
    ["cookie"] = {
      order = 11
      test = "SHADER_LIGHT_HAS_TEX(Cookie)"
    },
    ["ibl"] = {
      order = 11
      test = "SHADER_LIGHT_HAS_TEX(IBLDiffuse)"
    },
  }

  function outputSelectorChildren(dt) {
    local children = []
    foreach (i in dt.num_children) {
      local cdt = dt.child_from_index[i]
      children.append({ order = _selectors[cdt.name].order, dt = cdt })
    }
    children.sort(function(a,b) { return a.order <=> b.order })
    foreach (child in children) {
      outputSelectorBranch(child.dt)
    }
    return children.len()
  }

  function outputSelectorBranch(dt) {
    local selector = _selectors[dt.name]
    if (selector.test) {
      addLine("if (" + selector.test + ") {")
      indent()
    }

    local numChildren = outputSelectorChildren(dt);

    local shaderName = dt.string.name
    if (shaderName.?len()) {
      addLine("return " + dt.string.name + ";")
    }

    if (selector.test) {
      unindent()
      addLine("}")
      addEmptyLine()
    }
  }

  function doit(aShadersDir, aName,aBuildLibrary) {
    local shadersDir = aShadersDir
    local shadersLibraryPath = shadersDir.setfile(aName).setext("cgl");
    if (!::fs.dirExists(shadersDir)) {
      ::log("Can't find shaders directory: " + shadersDir)
      return
    }
    if (aBuildLibrary && ::fs.fileExists(shadersLibraryPath)) {
      ::log("Shaders library already built: " + shadersLibraryPath)
      return
    }

    _out = ::fs.fileOpenWrite("./shaders/" + aName.setext(".h"))
    local shaderEnumName = "e_" + aName
    local shaderNames = "fn_" + aName

    local files = ::fs.listFiles(shadersDir, aName + "*.cgo")
    local shaders = []
    foreach (f in files) {
      local abspath = shadersDir.setfile(f.path)
      local filename = f.path.getfile().getfilenoext()
      local name = f.path.getfilenoext().after(aName).after("_")
      local ename = shaderEnumName + "_" + (name.?len() ? name : "_default")
      local flags = name.split("_")
      local dtpath = flags.join("/")
      local shader = { abspath = abspath, filename = filename, name = name, ename = ename, flags = flags, dtpath = dtpath }
      // ::println(::format("I/ename: %s, name: %s, dtpath: %s, filename: %s", ename, name, dtpath, filename));
      shaders.append(shader)
    }

    ::log(::format("%s has %d shaders.", aName, shaders.len()));

    addLine("// Auto generated, do not edit.");
    addLine(::format("#ifndef __%s_shaders_h_", aName));
    addLine(::format("#define __%s_shaders_h_", aName));
    addEmptyLine();

    addLine(::format("#ifndef SHADER_SELECTOR_PARAMS_%s", aName));
    addLine(::format("#define SHADER_SELECTOR_PARAMS_%s", aName));
    addLine("#endif");
    addLine(::format("#ifndef SHADER_SELECTOR_INIT_%s", aName));
    addLine(::format("#define SHADER_SELECTOR_INIT_%s", aName));
    addLine("#endif");
    addEmptyLine();

    addLine("// Shader enum");
    {
      addLine("enum " + shaderEnumName + " {");
      indent();
      foreach (i,shader in shaders) {
        addLine(shader.ename + " = " + i + ",");
      }
      addLine(shaderEnumName + "_last = " + shaders.len());
      unindent();
      addLine("};");
    }
    addEmptyLine();

    addLine("// Shader names");
    {
      addLine("static const char* " + shaderNames + "[" + shaderEnumName + "_last] = {");
      indent();
      foreach (i,shader in shaders) {
        addLine("\"" + shader.filename + "\",");
      }
      unindent();
      addLine("};");
    }
    addEmptyLine();

    addLine("// Shader Selector");
    local dt = ::DT("ShaderTree");
    foreach (i,shader in shaders) {
      dt.NewVarFromPath(shader.dtpath + "/@name", shader.ename);
    }

    if (false) {
      addEmptyLine();
      addLine("/*");
      addLine(::lang.dataTableToString(dt,"xml"));
      addLine("*/");
      addEmptyLine();
    }

    addLine("static tU32 select_shader_" + aName + "(const sMaterialDesc& matDesc SHADER_SELECTOR_PARAMS_" + aName + ") {");
    indent();
    addLine("const tMaterialFlags matFlags = matDesc.mFlags;");
    addLine("SHADER_SELECTOR_INIT_" + aName + ";");
    outputSelectorChildren(dt);
    addLine("return " + dt.string.name + ";");
    unindent();
    addLine("}");

    addLine(::format("#endif // __%s_shaders_h_", aName));

    _out.Flush()

    if (aBuildLibrary) {
      local fp = ::fs.fileOpenWrite(shadersLibraryPath)
      ::log("Building shader library: " + fp.source_path)
      local arch = ::gZip.CreateZipArchive(fp,null,null)
      foreach (i,shader in shaders) {
        if (!arch.AddFileBlock(shader.filename,::fs.fileOpenRead(shader.abspath),0))
          throw ::format("Can't add %s to shader library %s", shader.filename, fp.source_path)
      }
      ::log("Deleting packed shaders...")
      foreach (i,shader in shaders) {
        ::gRootFS.FileDelete(shader.abspath)
      }
      ::log("DONE Building shader library: " + fp.source_path)
    }
  }
}

module <- {
  function genShaderSelector(aShadersDir,aName,aBuildLibrary) : (tGenSelector) {
    return tGenSelector.Clone().doit(aShadersDir,aName,aBuildLibrary);
  }
}
