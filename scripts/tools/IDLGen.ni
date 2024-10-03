// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("fs.ni")

// Interface Attributes :
//
//  NoAutomation
//

// Method Attributes :
//
//  PropertySet/Get - {Property} - property='set|get'
//  SerializeCategoryN - {SerializeN}, N not specified assumes 1 - serialize='N'
//  NoAutomation - {NoAutomation} = noautomation='yes'
//  Const - automatic - const='yes'
//

::Import("lang.ni")
::Import("xml.ni")

::idlGen <- {

  mSrc = null
  mDest = null
  mDestInl = null
  mLine = ""
  mRawLine = ""
  mComments = ""
  mPos = null
  mTokens = []
  mLineCount = 0
  mDT = null
  mOutputDebug = false
  mOutputComments = true
  mConstants = {}
  mNamespaceStack = []
  mInterfaceStack = []
  mMinFeatures = []
  mInMinFeaturesCount = 0

  tInteropParam = {
    name = ""
    type = ""
    typeval = ""
    typei = ""
    typec = ""
  }

  ///////////////////////////////////////////////
  function reset()
  {
    mSrc = null
    mDest = null
    mDestInl = null
    mLine = ""
    mComments = ""
    mPos = null
    mLineCount = 0
    mDT = ::xml.tNode.new("niIDL")
  }

  ///////////////////////////////////////////////
  function message(aMessage)
  {
    ::lang.log(aMessage)
    ::println(aMessage)
  }

  ///////////////////////////////////////////////
  function resetComments()
  {
    mComments = ""
  }

  ///////////////////////////////////////////////
  function addCommentLine(aLine)
  {
    mComments += aLine + "\n"
  }

  ///////////////////////////////////////////////
  function outRaw(aText)
  {
    if (!mOutputDebug) return
    mDest.WriteString(aText)
  }

  ///////////////////////////////////////////////
  function outLine(aText)
  {
    if (!mOutputDebug) return
    mDest.WriteString(aText)
    if (!aText.endswith("\n")) {
      mDest.WriteString("\n")
    }
  }

  ///////////////////////////////////////////////
  function outInl(aText)
  {
    if (!mDestInl) return
    mDestInl.WriteString(aText)
    if (!aText.endswith("\n")) {
      mDestInl.WriteString("\n")
    }
  }

  ///////////////////////////////////////////////
  function getValidTypeNameEx(aType) {
    local type = "";
    local tokens = tokenize(aType,null)
    for (local i = 0; i < tokens.len(); ++i) {
      type += tokens[i];
      if (i+1 >= tokens.len()) {
      }
      else if (tokens[i] == ":" || tokens[i+1] == ":") {
      }
      else {
        type += " "
      }
    }
    return [type,tokens]
  }

  ///////////////////////////////////////////////
  function getValidTypeName(aType) {
    return getValidTypeNameEx(aType)[0]
  }

  ///////////////////////////////////////////////
  function getTypeClass(aType) {
    return aType
  }

  ///////////////////////////////////////////////
  function getInteropTypeEx(aType) {
    local typeVal = 0
    local otype = ""
    local ctype = ""
    local typeEx = getValidTypeNameEx(aType)
    local type = typeEx[0]
    local toks = typeEx[1]
    function tokForEach(aFun) : (toks) {
      for (local i = 0; i < toks.len(); ++i)
      if (aFun(toks[i])) return true
      return false
    }
    function tokEq(aStr) : (toks) {
      for (local i = 0; i < toks.len(); ++i)
      if (toks[i] == aStr) return true
      return false
    }
    function tokEndsWith(aStr) : (toks) {
      for (local i = 0; i < toks.len(); ++i)
      if (toks[i].endswith(aStr)) return true
      return false
    }
    function tokStartsWith(aStr) : (toks) {
      for (local i = 0; i < toks.len(); ++i)
      if (toks[i].endswith(aStr)) return true
      return false
    }
    function isConst() : (type) {
      return tokEq("const")
    }
    function isPtr() : (type) {
      return type.contains("Ptr<") || tokEq("*") || tokEq("&")
    }
    function isInterface() : (type) {
      return type.startswith("i")
        || type.startswith("const i")
        || (type.contains("::i") && !type.startswith("int"))
        || type.contains("Ptr<")
    }
    function isEnum() : (type) {
      return type.startswith("e")
        || type.startswith("const e")
        || type.contains("::e")
    }
    function isFlags() : (type) {
      return (type.startswith("t")
              || type.startswith("const t")
              || type.contains("::t"))
        && type.endswith("Flags")
    }
    function isVec() : (type) {
      return type.contains("CVec")
    }
    function isLst() : (type) {
      return type.contains("CLst")
        || type.contains("SinkLst")
    }
    function isMap() : (type) {
      return type.contains("CMap")
    }
    function isCollection() : (type) {
      return isVec() || isLst() || isMap()
    }
    function isVar() : (type) {
      return type.startswith("Var") ||
        type.startswith("ni::Var") ||
        type.startswith("const Var") ||
        type.startswith("const ni::Var") ||
        type.startswith("niConstValue Var")
        type.startswith("niConstValue ni::Var")
    }
    function isType(aType) {
      return tokEq(aType)
    }

    if (isConst()) {
      otype += "ni::eTypeFlags_Constant|"
      typeVal |= ::eTypeFlags.Constant
    }
    otype += "ni::eType_"

    if (type.contains("tHStringPtr")) {
      throw "Can't use tHStringPtr, use ni::Ptr<ni::iHString> instead."
    }
    else if (type.contains("**")) {
      throw "Pointer of pointer not allowed."
      otype += "Ptr"
      typeVal |= ::eType.Ptr
    }
    else if (isCollection()) {
      if (!isPtr()) throw "Collections can't be passed by value."
      otype += "IUnknown|ni::eTypeFlags_Pointer"
      // get_type_class
      typeVal |= ::eType.IUnknown|::eTypeFlags.Pointer
    }
    else if (isInterface()) {
      if (!isPtr()) throw "Interfaces can't be passed by value."
      otype += "IUnknown|ni::eTypeFlags_Pointer"
      // get_type_class
      typeVal |= ::eType.IUnknown|::eTypeFlags.Pointer
    }
    else if (isEnum() || isFlags()) {
      otype += "Enum"
      // get_type_class
      typeVal |= ::eType.Enum
    }
    else if (isType("void")) {
      if (isPtr()) {
        otype += "Ptr"
        typeVal |= ::eType.Ptr
      }
      else {
        otype += "Null"
        typeVal |= ::eType.Null
      }
    }
    else {
      if (isType("sRecti")) {
        otype += "Vec4i"
        typeVal |= ::eType.Vec4i
      }
      else if (isType("sRectf")) {
        otype += "Vec4f"
        typeVal |= ::eType.Vec4f
      }
      else if (isVar()) {
        otype += "Variant"
        typeVal |= ::eType.Variant
      }
      else if (isType("tFVF")) {
        otype += "U32"
        typeVal |= ::eType.U32
      }
      else if (isType("tType")) {
        otype += "U32"
        typeVal |= ::eType.U32
      }
      else if (isType("tTime")) {
        otype += "I64"
        typeVal |= ::eType.I64
      }
      else if (isType("tVersion")) {
        otype += "U32"
        typeVal |= ::eType.U32
      }
      else if (isType("tEnum")) {
        otype += "U32"
        typeVal |= ::eType.U32
      }
      else if (isType("tHandle")) {
        otype += "UIntPtr"
        typeVal |= ::eType.UIntPtr
      }
      else if (isType("tIntPtr")) {
        otype += "IntPtr"
        typeVal |= ::eType.IntPtr
      }
      else if (isType("tUIntPtr")) {
        otype += "UIntPtr"
        typeVal |= ::eType.UIntPtr
      }
      else if (isType("tPtr")) {
        otype += "Ptr"
        typeVal |= ::eType.Ptr
      }
      else if (isType("tOffset")) {
        otype += "Offset"
        typeVal |= ::eType.Offset
      }
      else if (isType("cString")) {
        otype += "String"
        typeVal |= ::eType.String
      }
      else if (isType("tBool")) {
        otype += "I8"
        typeVal |= ::eType.I8
      }
      else if (isType("tSize")) {
        otype += "Size"
        typeVal |= ::eType.Size
      }
      else if (isType("tI8")) {
        otype += "I8"
        typeVal |= ::eType.I8
      }
      else if (isType("tU8")) {
        otype += "U8"
        typeVal |= ::eType.U8
      }
      else if (isType("tI16")) {
        otype += "I16"
        typeVal |= ::eType.I16
      }
      else if (isType("tU16")) {
        otype += "U16"
        typeVal |= ::eType.U16
      }
      else if (isType("tI32")) {
        otype += "I32"
        typeVal |= ::eType.I32
      }
      else if (isType("tU32")) {
        otype += "U32"
        typeVal |= ::eType.U32
      }
      else if (isType("tI64")) {
        otype += "I64"
        typeVal |= ::eType.I64
      }
      else if (isType("tU64")) {
        otype += "U64"
        typeVal |= ::eType.U64
      }
      else if (isType("tUUID")) {
        otype += "UUID"
        typeVal |= ::eType.UUID
      }
      else if (isType("sUUID")) {
        otype += "UUID"
        typeVal |= ::eType.UUID
      }
      else if (isType("tInt")) {
        otype += "Int"
        typeVal |= ::eType.Int
      }
      else if (isType("tUInt")) {
        otype += "UInt"
        typeVal |= ::eType.UInt
      }
      else if (isType("tF32")) {
        otype += "F32"
        typeVal |= ::eType.F32
      }
      else if (isType("tFloat")) {
        otype += "F32"
        typeVal |= ::eType.F32
      }
      else if (isType("tF64")) {
        otype += "F64"
        typeVal |= ::eType.F64
      }
      else if (isType("sVec2i")) {
        otype += "Vec2i"
        typeVal |= ::eType.Vec2i
      }
      else if (isType("sVec3i")) {
        otype += "Vec3i"
        typeVal |= ::eType.Vec3i
      }
      else if (isType("sVec4i")) {
        otype += "Vec4i"
        typeVal |= ::eType.Vec4i
      }
      else if (isType("sVec2f")) {
        otype += "Vec2f"
        typeVal |= ::eType.Vec2f
      }
      else if (isType("sVec3f")) {
        otype += "Vec3f"
        typeVal |= ::eType.Vec3f
      }
      else if (isType("sVec4f")) {
        otype += "Vec4f"
        typeVal |= ::eType.Vec4f
      }
      else if (isType("sPlanef")) {
        otype += "Vec4f"
        typeVal |= ::eType.Vec4f
      }
      else if (isType("sColor3ub")) {
        otype += "U32"
        typeVal |= ::eType.U32
      }
      else if (isType("sColor4ub")) {
        otype += "U32"
        typeVal |= ::eType.U32
      }
      else if (isType("sColor3f")) {
        otype += "Vec3f"
        typeVal |= ::eType.Vec3f
      }
      else if (isType("sColor4f")) {
        otype += "Vec4f"
        typeVal |= ::eType.Vec4f
      }
      else if (isType("sQuatf")) {
        otype += "Vec4f"
        typeVal |= ::eType.Vec4f
      }
      else if (isType("sMatrixf")) {
        otype += "Matrixf"
        typeVal |= ::eType.Matrixf
      }
      else if (isType("achar") ||
               isType("cchar") ||
               isType("gchar") ||
               isType("xchar") ||
               isType("char") || // space is necessary to avoid ambiguities
               isType("uchar"))
      {
        if (!(isConst() && isPtr())) {
          throw "Char types can only be constant pointers. ("+type+")"
        }
        if (isType("achar")) {
          otype += "AChar"
          typeVal |= ::eType.AChar
        }
        else {
          throw "Char type has been deprecated, only achar* should be used : " + type
        }
      }
      else if (isType("sInterfaceDef") ||
               isType("sEnumDef") ||
               isType("sMethodDef") ||
               isType("sConstantDef") ||
               isType("sParameterDef"))
      {
        otype += "Ptr"
        typeVal |= ::eType.Ptr
      }
      else {
        parserWarning(mLineCount,"Type '"+type+"' can't be identified as an interop type.")
        otype += "Null"
        typeVal |= ::eType.Null
      }
      if (isPtr()) {
        otype += "|ni::eTypeFlags_Pointer";
        typeVal |= ::eTypeFlags.Pointer
      }
    }
    return [otype,typeVal]
  }
  function getInteropType(aType) {
    return getInteropTypeEx(aType)[1]
  }
  function getInteropTypeName(aType) {
    return getInteropTypeEx(aType)[0]
  }

  ///////////////////////////////////////////////
  function tokenize(aText,aStop)
  {
    local splitChars = [
      { c='{',  t=1, }
      { c='}',  t=1, }
      { c='(',  t=1, }
      { c=')',  t=1, }
      { c=' ',  t=0, }
      { c='\t', t=0, }
      { c='\n', t=0, }
      { c='\r', t=0, }
      { c=';',  t=1, }
      { c=':',  t=1, }
      { c='=',  t=1, }
      { c=',',  t=1, }
      { c='*',  t=1, }
      { c='&',  t=1, }
    ]
    local tokens = []
    local i = 0
    local currentToken = ""

    while (i < aText.len()) {
      local split = false
      local splitistoken = false
      local c = aText[i]
      foreach (s in splitChars) {
        if (s.c == c) {
          split = true
          splitistoken = s.t
          break
        }
      }
      if (split) {
        if (currentToken.notempty()) {
          tokens.append(currentToken)
          if (aStop) foreach (stop in aStop) {
            if (stop == currentToken)
              return tokens;
          }
        }
        if (splitistoken) {
          currentToken = c.tochar()
          tokens.append(currentToken)
          if (aStop) foreach (stop in aStop) {
            if (stop == currentToken)
              return tokens;
          }
        }
        currentToken = ""
      }
      else {
        currentToken += c.tochar()
      }
      ++i
    }
    if (currentToken.notempty()) {
      tokens.append(currentToken)
      currentToken = ""
    }

    if (aStop != null) {
      while (1) {
        local split = false
        local splitistoken = false
        local c = mSrc.ReadChar()
        if (c == '\n')
          ++mLineCount;
        foreach (s in splitChars) {
          if (s.c == c) {
            split = true
            splitistoken = s.t
            break
          }
        }
        if (split) {
          if (currentToken.notempty()) {
            tokens.append(currentToken)
            if (aStop) foreach (stop in aStop) {
              if (stop == currentToken)
                return tokens;
            }
          }
          if (splitistoken) {
            currentToken = c.tochar()
            tokens.append(currentToken)
            if (aStop) foreach (stop in aStop) {
              if (stop == currentToken)
                return tokens;
            }
          }
          currentToken = ""
        }
        else {
          currentToken += c.tochar()
        }
        if (mSrc.partial_read)
          return null
      }
      if (currentToken.notempty()) {
        tokens.append(currentToken)
        currentToken = ""
      }
    }

    return tokens
  }

  ///////////////////////////////////////////////
  function hasAttribute(attrs, toFind) {
    if (attrs) {
      foreach (a in attrs) {
        if (a.name == toFind)
          return a.value
      }
    }
    return false
  }

  ///////////////////////////////////////////////
  function getCommentsAttributes()
  {
    local attrs = []
    if (!mMinFeatures.empty()) {
      attrs.append({
        name = "min_features",
        value = ""+mMinFeatures.top().value
      })
    }

    if (mComments.empty())
      return attrs

    outLine("---COMMENTBLOCK-BEGIN---")

    local tokens = tokenize(mComments,null)
    for (local i = 0; i < tokens.len(); ++i) {
      if (tokens[i] == "{") {
        if (i+2 >= tokens.len()) {
          parserError(mLineCount,"Unexpected end of comments when parsing attributes.");
        }

        local a = { name="", value="" };
        a.name = tokens[i+1]

        if (a.name == "!") {
          // Most likely a comment that embbeds a Script or TScript definition:
          //! XYZ {
          //!   abc = 123
          //! }
          // So just skip over it
          continue;
        }

        // {Attr=Value}
        if (tokens[i+2] == "=") {
          if (i+4 >= tokens.len()) {
            parserWarning(mLineCount,"Unexpected end of comments when parsing assigned attribute '"+a.name+"'.");
            continue;
          }
          if (tokens[i+4] != "}") {
            parserWarning(mLineCount,"Expected } parsing assigned attribute '"+a.name+"'.");
            continue;
          }
          a.value = tokens[i+3]
        }
        else {
          // {Attr}
          if (tokens[i+2] != "}") {
            parserWarning(mLineCount,"Expected } parsing attribute '"+a.name+"'.");
            continue;
          }
        }

        a.name = a.name.tolower()
        attrs.append(a)
        outLine("ATTR:"+a.name+"="+a.value)

      }
    }

    outLine("---COMMENTBLOCK-END---")
    return attrs
  }

  ///////////////////////////////////////////////
  function dtPushNew(aType) {
    mDT.pushNew(aType)
  }

  ///////////////////////////////////////////////
  function dtPushNewName(aType,aName) {
    mDT.pushNew(aType)
    mDT.setAttribute("name",aName)
  }

  ///////////////////////////////////////////////
  function dtPushName(aType,aName,aAttrs) {
    foreach (i,c in mDT.top()._children) {
      mDT.pushChild(i)
      if (mDT.getAttribute("name") == aName) {
        return true
      }
      mDT.pop()
    }
    mDT.pushNew(aType)
    mDT.setAttribute("name",aName)
    if (aAttrs)
      setAttributes(aAttrs)
    return false
  }

  ///////////////////////////////////////////////
  function dtPushNewNameAttrs(aType,aName,aAttrs) {
    mDT.pushNew(aType)
    mDT.setAttribute("name",aName)
    setAttributes(aAttrs)
  }

  ///////////////////////////////////////////////
  function dtPop() {
    mDT.pop()
  }

  ///////////////////////////////////////////////
  function dtSet(aName,aValue) {
    mDT.setAttribute(aName,aValue)
  }

  ///////////////////////////////////////////////
  function dtGet(aName) {
    return mDT.getAttribute(aName)
  }

  ///////////////////////////////////////////////
  function getPropertyName(aMeth) {
    function isUpper(c) {
      return c == c.toupper();
    }
    function isNumber(c) {
      return  c == "0" || c == "1" || c == "2" || c == "3" || c == "4" ||
        c == "5" || c == "6" || c == "7" || c == "8" || c == "9";
    }

    local i = 0;
    local ret = "";

    // ni property names : property_name
    local name = aMeth.slice(3);
    local c = "", n = "";
    while (1) {
      c = name[i].tochar();
      ret = ret + c.tolower();
      i = i+1;
      if (i >= name.len())
        break;
      n = name[i].tochar();
      if (isUpper(n) && !isUpper(c) && !isNumber(n))
        ret = ret + "_";
    }

    return ret;
  }
  function getPropertyName2(aMeth) {
    function isUpper(c) {
      return c == c.toupper();
    }
    local name = aMeth.slice(3)
    local i = 0
    local ret = "";
    while (i < name.len()) {
      local c = name[i].tochar()
      if (!isUpper(c)) break;
      ret = ret + c.tolower()
      ++i;
    }
    if (i < name.len())
      ret += name.slice(i)
    return ret;
  }
  function getJavaName(aMeth) {
    function isUpper(c) {
      return c == c.toupper();
    }
    local name = aMeth
    local i = 0
    local ret = "";
    while (i < name.len()) {
      local c = name[i].tochar()
      if (!isUpper(c)) break;
      ret = ret + c.tolower()
      ++i;
    }
    if (i < name.len())
      ret += name.slice(i)
    switch (ret) {
      // reserved keywords
      case "enum":
      return "doEnum";
      case "import":
      return "doImport";
      case "package":
      return "doPackage";
      case "final":
      return "doFinal";
      case "break":
      return "doBreak";
      case "continue":
      return "doContinue";
      // java.lang.Object final methods
      case "getClass":
      return "getClassName";
      case "notify":
      return "doNotify";
      case "notifyAll":
      return "doNotifyAll";
      case "wait":
      return "doWait";
      default:
      return ret;
    }
    return ret;
  }

  ///////////////////////////////////////////////
  function setAttributes(aAttrs) {
    if (!aAttrs || aAttrs.len() == 0) return
    local top = mDT.top()
    foreach (a in aAttrs) {
      if (a.name.icompare("Property") == 0) {
        local methName = top._attributes["name"]
        if (methName.startswith("Set")) {
          dtSet(a.name,"set")
          if (!("property_name" in top._attributes)) {
            dtSet("property_name",getPropertyName(methName))
            dtSet("property_name2",getPropertyName2(methName))
          }
        }
        else if (methName.startswith("Get")) {
          dtSet(a.name,"get")
          if (!("property_name" in top._attributes)) {
            dtSet("property_name",getPropertyName(methName))
            dtSet("property_name2",getPropertyName2(methName))
          }
        }
        else {
          parserError(mLineCount,"Property attribute is set on an invalid item '"+methName+"'.");
        }
      }
      else if (a.name.startswithi("Serialize")) {
        local cat = "1"
        if (a.name.len() == 9) {
          // category one
        }
        else {
          cat = a.name[a.name.len()-1].tochar().tostring()
        }
        dtSet("serialize",cat)
      }
      else {
        if (a.value.len()) {
          dtSet(a.name,a.value)
        }
        else {
          dtSet(a.name,"yes")
        }
      }
    }
  }

  ///////////////////////////////////////////////
  function processCommentsAndEmptyLines()
  {
    if (mLine.len() == 0) {
      resetComments();
      outLine("---EMPTYLINE---")
      return true
    }

    if (mLine.startswith("#")) {  // # preprocessor command considered as empty value
      skipPreprocessor(mLine)
      resetComments();
      outLine("---EMPTYLINE-PREPROCESSOR---")
      return true
    }

    mLine = mRawLine

    local pos
    // Is it a block comment ?
    pos = mLine.find("/*")
    if (pos != null) {
      local commentStart = mLineCount
      if (mLine.rfind("*/") != null) {
        addCommentLine(mLine.slice(2,-2))

      }
      else {
        addCommentLine(mLine.slice(2))
        while (1) {
          mLine = readLine()
          if (mLine.find("/*")) parserError(commentStart,"/* comment block opening found inside comment block.");
          local pos = mLine.find("*/")
          if (pos != null) {
            addCommentLine(mLine.slice(0,pos))
            break;
          }
          else {
            addCommentLine(mLine)
          }
          if (mSrc.partial_read) parserError(commentStart,"Unexpected end of file while parsing comment block.");
        }
      }
      return true
    }
    // Comments
    pos = mLine.find("//")
    if (pos != null) {
      if (pos == 0) {
        // Comment begining the line can be used to store attributes, add it to the comments buffer
        addCommentLine(mLine.slice(pos+2))
        return true
      }
      mLine = mLine.slice(0,pos)  // remove the comment and pass it through normal processing
    }
    return false
  }

  ///////////////////////////////////////////////
  function processNamespace()
  {
    local startLine = mLineCount
    local tokens = tokenize(mLine,["{",";"])
    if (tokens == null) parserError(startLine,"Unexpected end of file while parsing namespace.")
    if (tokens.top() == ";")
      return;

    local name = tokens[1];
    outLine("---NAMESPACE-BEGIN:"+name)
    dtPushNewName("namespace",name)
    mNamespaceStack.push(name)

    outInl("/** NAMESPACE : " + name + " **/");

    while (!mSrc.partial_read) {
      // Read the next line, remove the begining and end spaces, tabs, and newlines
      mLine = readLine();
      if (mLine.len()) {
        // Preprocessor
        if (mLine[0] == '#') {
          skipPreprocessor(mLine)
        }
        // Inline functions, static inline functions, and templates
        else if (mLine.startswith("inline") ||
                 mLine.startswith("__forceinline") ||
                 mLine.startswith("static") ||
                 mLine.startswith("template") ||
                 mLine.startswith("niExportFunc") ||
                 mLine.startswith("niExportFuncCPP") ||
                 mLine.startswith("niFn") ||
                 mLine.startswith("niFnS") ||
                 mLine.startswith("niFnV"))
        {
          local start = mLine;
          if (mLine.endswith(";")) {
            // line ends with ';' so it's a prototype or forward declaration
            // just skip this line
          }
          else
          {
            // skip the block
            local shouldSkipBlock = false;
            while (1) {
              if (mLine.find("{") != null) {
                shouldSkipBlock = true;
                break;
              }
              if (mLine.endswith(";")) {
                // this is a forward declaration
                shouldSkipBlock = false;
                break;
              }
              if (mSrc.partial_read)
                parserError(mLineCount,"Unexpected end of file while looking for block start for: " + start);
              mLine = readLine();
            }
            if (shouldSkipBlock) {
              skipBlock();
            }
          }
        }
        // Comments
        else if (processCommentsAndEmptyLines()) {
          continue; // skip reset comments
        }
        // Namespace
        else if (mLine.startswith("namespace")) {
          processNamespace()
        }
        // Enumeration
        else if (mLine.startswith("enum") || mLine.startswith("typedef enum")) {
          processEnum()
        }
        // Interface
        else if (mLine.startswith("niInterface") || mLine.startswith("struct i") || mLine.startswith("interface")) {
          processInterface()
        }
        // Constant variables
        else if (mLine.startswith("const") || mLine.startswith("niConstValue")) {
          processConstVariable()
        }
        // Unknown block
        else if (mLine.find("{") != null) {
          skipBlock()
        }
        // End of this namespace
        else if (mLine.find("}") != null) {
          mNamespaceStack.pop();
          break
        }
        else {
          outLine("%%" + mLine)
        }
      }
      resetComments();
    }
    dtPop();
    outLine("---NAMESPACE-END:"+name)
  }

  ///////////////////////////////////////////////
  function getScriptValue(val) {
  }

  ///////////////////////////////////////////////
  function processEnum()
  {
    local startLine = mLineCount
    local attributes = getCommentsAttributes()
    // Parse the name and parents, all must be defined on one line
    local tokens = tokenize(mLine,["{",";"])
    if (!tokens)
      parserError(startLine,"Unexpected end of file while parsing enum header.")

    if (tokens.top() == ";") {
      // Forward declaration
      outLine("EFWD:"+mLine)
    }
    else {
      // enum declaration
      foreach (i,t in tokens) {
        outLine("ED:"+i+":"+t)
      }

      local name;
      if (mLine.startswith("typedef enum")) {
        if (tokens.len() < 3)
          parserError(startLine,"Invalid typedef enum declaration.")
        name = tokens[2]
      }
      else {
        if (tokens.len() < 2)
          parserError(startLine,"Invalid enum declaration.")
        name = tokens[1]
      }

      if (name == "{") {  // Unammed enum
        name = ""
      }

      local numValues = 0

      dtPushNewNameAttrs("enum",name,attributes)
      dtPushComments(mComments);
      resetComments()

      local vals = {}

      // Parse the enum values
      while (1) {
        mLine = readLine()
        if (processCommentsAndEmptyLines())
          continue;

        if (mLine.startswith(name) || (name=="" && !mLine.startswith("};"))) {
          // process the enum value
          tokens = tokenize(mLine,null)
          local n = "", v = ""
          // read the name
          n = tokens[0];

          local eqIdx = null, isValid = false
          if (tokens.len() > 1) {
            if (tokens[1] == "niMaybeUnused") {
              if (tokens.len() >= 3 && (tokens[2] == "=" || tokens[2] == ",")) {
                eqIdx = 2;
                isValid = true;
              }
            } else {
              if (tokens[1] == "=" || tokens[1] == ",") {
                eqIdx = 1;
                isValid = true;
              }
            }
          }

          if (!isValid)
            parserError(mLineCount,"Invalid enum value declaration inside enum '"+name+"', no value or missing colon.")

          if (eqIdx) {
            if ((tokens.len()-eqIdx) < 2) {
              parserError(mLineCount,"Invalid assigned enum value declaration inside enum '"+name+"'.")
            }
            for (local i = eqIdx+1; i < tokens.len(); ++i) {
              v += tokens[i];
              if (i+1 != tokens.len())
                v += " ";
            }
            if (v.endswith(",")) {
              v = v.slice(0,-1)
            }
          }

          if (!n.endswith("ForceDWORD")) {
            local attributesVal = getCommentsAttributes()
            if (!(n in vals)) {
              local exists = dtPushNewNameAttrs("value",n,attributesVal)
              dtPushComments(mComments);
              resetComments()
              if (!exists) {  // set only if it's the first one
                dtSet("value",v)
              }
              dtPop()
              vals[n] <- true
              ++numValues
            }
          }

          outLine("EV: "+n+" = "+v)
          continue
        }

        if (mLine.find("}") != null && mLine.find(";") != null) {
          break;
        }

        if (mSrc.partial_read)
          parserError(startLine,"Unexpected end of file while parsing enum values.")

        parserError(mLineCount,"Invalid enum declaration inside enum '"+mLine+"'.")
      }

      dtPop()

      if (numValues == 0) {
        parserError(startLine,"Empty enum, the values cant be on the same same line as the enum header.")
      }
    }
  }

  ///////////////////////////////////////////////
  function skipBlock()
  {
    local startLine = mLineCount
    local attributes = getCommentsAttributes()
    local tokens
    if (mLine.rfind("}")) return
    // Parse the block header
    tokens = tokenize(mLine,["{",";"])
    if (!tokens)  parserError(startLine,"Unexpected end of file while parsing block header.")
    if (tokens.top() == ";") {
      // Forward declaration
      outLine("BFWD:"+mLine)
    }
    else {
      outLine("BD0:"+mLine)
      local blockCount = 1
      // Parse the methods
      while (1) {
        mLine = readLine()
        if (mLine.find("{") != null)  {
          ++blockCount
        }
        if (mLine.find("}") != null)  {
          --blockCount
        }

        outLine("BD("+blockCount+"):"+mLine)

        if (blockCount == 0)
          break
        if (blockCount < 0) {
          parserError(startLine,"Unbalanced blocks.")
        }
        if (mSrc.partial_read)
          parserError(startLine,"Unexpected end of file while parsing block.")
      }
    }
  }

  ///////////////////////////////////////////////
  function UUIDFromTokens(toks) {
    local numComma = 0;
    local uuid1 = "", uuid2 = "";
    for (local i = 4; i < toks.len(); ++i) {
      local t = toks[i]
      if (t == ",") {
        if (numComma > 4 || numComma == 3) {
        }
        else {
          uuid1 += "-";
        }
        uuid2 += ",";
        ++numComma;
        continue;
      }
      else if (t == ")") {
        break;
      }
      uuid1 += t.after("0x").toupper();
      uuid2 += t;
    }
    return [uuid1,uuid2]
  }

  ///////////////////////////////////////////////
  function processInterface()
  {
    local startLine = mLineCount
    local attributes = getCommentsAttributes()
    local tokens = null
    local hasNoAutomation = hasAttribute(attributes,"noautomation")
    local hasMinFeatures = hasAttribute(attributes,"min_features")

    // Parse the name and parents, all must be defined on one line
    tokens = tokenize(mLine,["{",";",";"])
    if (!tokens)  parserError(startLine,"Unexpected end of file while parsing interface header.")
    if (tokens.top() == ";") {
      // Forward declaration
      outLine("IFWD:"+mLine)
    }
    else {
      foreach (i,t in tokens) {
        outLine("IT["+i+"]:"+t)
      }
      local name = tokens[1]

      // Interface declaration
      if (tokens.len() <= 2)
        parserError(startLine,"Invalid interface declaration.")

      local parents = []
      if (tokens.len() > 3) {
        if (tokens[2] != ":")
          parserError(startLine,"End of line or ':' expected after interface name.")
        local i = 3;
        local t
        while (1) {
          local parentName = ""

          if (tokens[i++] != "public")
            parserError(startLine,"'public' expected after interface name.")
          if (i >= tokens.len())
            parserError(startLine,"Unexpected end of interface declaration.")

          parentName = tokens[i++]
          if (i >= tokens.len())
            parserError(startLine,"Unexpected end of interface declaration.")

          t = tokens[i++]

          if (t == "{" || t == ";") {
            parents.append(parentName)
            break
          }

          if (t == ":") {
            t = tokens[i++]
            if (t != ":")
              parserError(startLine,"'::' expected to list interface's parent namespace declaration.")
            parentName += "::"+tokens[i++]

            t = tokens[i++]
            if (t == "{" || t == ";") {
              parents.append(parentName)
              break
            }
          }

          parents.append(parentName)
          if (t != ",")
            parserError(startLine,"comma ',' expected to list interface's parents ('"+t+"') found.")
        }
      }

      {
        local str = name
        foreach (p in parents) {
          str += ":"+p
        }
        outLine("IDEF:"+str)
      }

      dtPushNewNameAttrs("interface",name,attributes)
      dtPushComments(mComments);
      if (parents.len()) {
        dtPushNew("parents")
        foreach (p in parents) {
          dtPushNewName("parent",p)
          dtPop()
        }
        dtPop()
      }

      mInterfaceStack.push(name)
      outInl("/** interface : " + name + " **/");

      if (hasNoAutomation) {
        outInl("/** " + name + " -> NO AUTOMATION **/")
      }
      else {
        if (hasMinFeatures) {
          outInl("/** " + name + " -> MIN FEATURES '"+hasMinFeatures+"' **/")
          outInl("#if niMinFeatures("+hasMinFeatures+")")
        }
        outInl("IDLC_BEGIN_INTF(" + mNamespaceStack.top() + "," + name + ")")
      }

      // Parse the methods
      local hasDeclareInterface = false
      while (1) {
        mLine = readLine()
        if (processCommentsAndEmptyLines())
          continue;

        if (mLine.contains("DeclareInterfaceUUID")) {
          hasDeclareInterface = true
          local toks = tokenize(mLine,[")"])
          if (toks[2] != name)
            throw "Invalid declare interface, declared '"+toks[2]+"' but the interface name is '"+name+"'."
          local uuid = UUIDFromTokens(toks)
          dtSet("uuid1",uuid[0])
          dtSet("uuid2",uuid[1])
          continue
        }
        else if (mLine.contains("DeclareBaseInterface")) {
          hasDeclareInterface = true
          local toks = tokenize(mLine,[")"])
          if (toks[2] != name)
            throw "Invalid declare interface, declared '"+toks[2]+"' but the interface name is '"+name+"'."
          local uuid = UUIDFromTokens(toks)
          dtSet("uuid1",uuid[0])
          dtSet("uuid2",uuid[1])
          continue
        }
        else if (mLine.startswith("virtual") || mLine.startswith("inline")) {
          processMethod(hasNoAutomation)
          resetComments()
          continue
        }

        if (mLine.endswith("};"))
          break;

        if (mSrc.partial_read)
          parserError(startLine,"Unexpected end of file while parsing interface methods.")
      }

      if (!hasDeclareInterface)
        throw "No niDeclareInterfaceUUID for interface '"+name+"'."

      if (!hasNoAutomation) {
        outInl("IDLC_END_INTF(" + mNamespaceStack.top() + "," + mInterfaceStack.top() + ")\n\n")
        if (hasMinFeatures) {
          outInl("#endif // if niMinFeatures("+hasMinFeatures+")")
        }
      }

      mInterfaceStack.pop();
      dtPop()
    }
  }

  ///////////////////////////////////////////////
  function dtPushComments(aComments) {
    if (!mOutputComments)
      return
    dtPushNew("comments");
    local state = "main"; // main|remark|param
    local main = "";
    local ret = "";
    local unknown = "";
    local remarks = []
    local params = []
    local lines = aComments.split("\n");
    local inPreTag = 0
    foreach (l in lines) {
      if (!l.startswith("!"))
        continue

      local rawLine = l.after("!")
      l = rawLine.normalize()
      if (l.startswith("<pre>")) {
        // change rawLine, that's what's used when in a pre tag
        rawLine = "\n<pre>"+rawLine.after("<pre>")
        ++inPreTag
      }
      else if (l.startswith("</pre>")) {
        --inPreTag
      }
      else if (l.startswith("\\remark")) {
        state = "remark"
        l = l.after("\\remark").normalize()
        remarks.append("")
      }
      else if (l.startswith("\\param")) {
        state = "param"
        l = l.after("\\param").normalize()
        params.append("")
      }
      else if (l.startswith("\\return")) {
        state = "return"
        l = l.after("\\return").normalize()
      }
      else if (l.startswith("{") && l.contains("}")) {
        // skip line starting with attributes...
        continue;
      }

      local eol
      if (inPreTag) {
        l = rawLine
        eol = "\n"
      }
      else {
        eol = " "
      }

      switch (state) {
        case "main": {
          main += l+eol;
          break;
        }
        case "return": {
          ret += l+eol;
          break;
        }
        case "remark": {
          remarks[remarks.len()-1] += l+eol;
          break;
        }
        case "param": {
          params[params.len()-1] += l+eol;
          break;
        }
        default: {
          unknown += l+eol;
          break;
        }
      }
    }
    dtSet("desc",main)
    if (!unknown.empty())
      dtSet("unknown",unknown)
    if (!ret.empty())
      dtSet("return",ret)
    foreach (r in remarks) {
      dtPushNew("remark")
      dtSet("text",r)
      dtPop()
    }
    foreach (p in params) {
      dtPushNew("params")
      dtSet("text",p)
      dtPop()
    }
    dtPop()
  }

  ///////////////////////////////////////////////
  function isValidReturnTypeForInl(aTypeTable) {
    local type = aTypeTable.typeval
    local baseType = type & ::knTypeMask
    if (baseType == ::eType.Null && (type&::eTypeFlags.Pointer))
      return false
    if (type&::eTypeFlags.Pointer) {
      if (baseType == ::eType.IUnknown) {
        // iUnknown*, allowed
        return true;
      }
      if (type&::eTypeFlags.Constant)  {
        // const &, allowed
        if (aTypeTable.typec.contains("&"))
          return true;
        // string types (const achar*), allowed
        if (baseType == ::eType.AChar)
          return true;
        // C/G/XString*, allowed
        if (baseType == ::eType.String)
          return true;
      }
      // other pointer types won't work
      return false;
    }
    return true
  }
  function isValidParamTypeForInl(aTypeTable) {
    local type = aTypeTable.typeval
    local baseType = type & ::knTypeMask
    if (baseType == ::eType.Null && type&::eTypeFlags.Pointer)
      return false
    if ((type&::eTypeFlags.Pointer) &&
        !(type&::eTypeFlags.Constant) &&
        baseType != ::eType.IUnknown &&
        !aTypeTable.typec.contains("*"))
    {
      return false
    }
    return true
  }

  ///////////////////////////////////////////////
  function processMethod(aHasNoAutomation)
  {
    local startLine = mLineCount
    local attributes = getCommentsAttributes()
    local tokens = tokenize(mLine,[";"])
    if (!tokens)
      parserError(startLine,"Unexpected end of file while parsing method.")

    if (tokens[1].startswith("~"))
      return  // destructor, skip it
    if (mLine.find("operator"))
      return  // operator, skip it

    if (!mLine.find("__stdcall") &&
        !mLine.find("__nicall") &&
        !mLine.find("niCall") &&
        !mLine.find("niDelegateCall"))
      parserError(startLine,"Invalid method declaration, has to be __stdcall, __nicall, niCall or niDelegateCall : "+mLine)

    if (tokens.len() < 7)
      parserError(startLine,"Invalid method declaration.")

    local retType = ""
    local i = 1
    for ( ; i < tokens.len(); ++i) {
      local v = tokens[i]
      if (v == "__stdcall" ||
          v == "__nicall" ||
          v == "niCall" ||
          v == "niDelegateCall")
        break;
      retType += v
      if (v != ":" && i+1 < tokens.len() &&
          tokens[i+1] != ":" &&
          tokens[i+1] != "*" &&
          tokens[i+1] != "&")
      {
        retType += " "
      }
    }
    if (retType.empty())
      parserError(startLine,"Invalid method declaration, empty return type declaration.")
    if (i+1 >= tokens.len())
      parserError(startLine,"Invalid method return type declaration.")
    ++i;   // skip calling convention (__stdcall, __nicall, niCall or niDelegateCall)

    local name = tokens[i++];
    if (i >= tokens.len())
      parserError(startLine,"Invalid method '"+name+"' declaration, can't parse after the name.")

    local params = []
    if (tokens[i++] != "(")
      parserError(startLine,"Invalid method '"+name+"' declaration, '(' expected.")

    local currentParam = []
    local currentDefValue = ""
    local parseDefValue = false
    local parCount = 1
    for ( ; i < tokens.len(); ++i) {
      if (tokens[i] == "(") {
        ++parCount
      }
      else if (tokens[i] == "=") {
        parseDefValue = true
        continue
      }
      else if (tokens[i] == "," || (parCount == 1 && tokens[i] == ")")) {
        if (parseDefValue && parCount > 1) {
        }
        else {
          if (currentParam.len() < 2) {
            if (tokens[i] == ")")
              break
            else
              parserError(startLine,"Invalid method '"+name+"' declaration, empty or unnamed parameter found.")
          }
          params.append({ _param = currentParam.Clone(), _default = currentDefValue })
          currentParam.clear()
          currentDefValue = ""
          parseDefValue = false
          if (tokens[i] == ")")
            break
          ++i;  // skip the ','
        }
      }
      else {
        if (tokens[i] == ")") {
          if (--parCount == 0)
            break;
        }
      }
      if (parseDefValue) {
        currentDefValue += tokens[i]+" "
      }
      else {
        currentParam.append(tokens[i])
      }
    }

    if (tokens[i++] != ")")
      parserError(startLine,"Invalid method '"+name+"' declaration, ')' expected.")

    local isConst = false
    if (tokens[i] == "const")
      isConst = true;
    else if (tokens[i] != "=" && tokens[i] != ";")
      parserError(startLine,"Invalid method '"+name+"' declaration, 'const', '=', or ';' expected, ["+tokens[i]+"] found.")

    /*    outLine("METHOD:"+name)
    outLine("    CONST:"+(isConst?"yes":"no"))
    outLine("    RETTYPE:"+retType)
    foreach (i,pi in params) {
      local p = pi.Clone()
      local name = p._param.top()
      p._param.pop()
      local type = ""
      foreach (v in p._param) {
        type += v;
        type += " ";
      }
      outLine("    P"+i+": "+type+"; "+name+" = "+p._default)
    }
*/

    //    if (retType[0] != 'e' && retType.find("Flags") == null) {
    //      if (retType.find("Rect") != null ||
    //                 retType.find("Vector") != null ||
    //                 retType.find("Quat") != null ||
    //                 retType.find("Matrix") != null ||
    //                 retType.find("Var") != null ||
    //                 retType.find("Plane") != null ||
    //                 retType.find("Color") != null)
    //             {
    //        if (retType.find("&") == null && retType.find("*") == null) {
    //          parserWarning(startLine,"Method "+name+"'s return type '"+retType+"' is a return by value that is not complatible with most interop layers.")
    //        }
    //      }
    //    }

    local hasNoAutomation = aHasNoAutomation || hasAttribute(attributes,"noautomation")
    local hasMinFeatures = hasAttribute(attributes,"min_features")

    local getTypeTable = function(aType) {
      local interopType = getInteropTypeEx(aType)
      return {
        type = getValidTypeName(aType).trim()
        typeval = interopType[1]
        typei = interopType[0].trim()
        typec = getTypeClass(aType).trim()
      }
    }

    local interopRetType = getTypeTable(retType)
    local interopParams = []
    foreach (pi in params) {
      local p = pi.Clone()
      local name = p._param.top()
      p._param.pop()
      local type = "";
      foreach (i,v in p._param) {
        type += v;
        if (v != ":" && i+1 < p._param.len() &&
            p._param[i+1] != ":" &&
            p._param[i+1] != "*" &&
            p._param[i+1] != "&")
        {
          type += " ";
        }
      }
      interopParams.push(getTypeTable(type));
      interopParams.top().name <- name
      if (p._default.len()) {
        interopParams.top()["default"] <- p._default
      }
    }

    local inlFmtMethodCmt = mNamespaceStack.top() + " -> " + mInterfaceStack.top() + "::" + name + "/" + params.len();
    if (hasNoAutomation) {
      outInl("/** " + inlFmtMethodCmt + " -> NO AUTOMATION **/")
    }
    else {
      if (hasMinFeatures) {
        outInl("/** " + inlFmtMethodCmt + " -> MIN FEATURES '"+hasMinFeatures+"' **/")
        outInl("#if niMinFeatures("+hasMinFeatures+")")
      }

      outInl("/** " + inlFmtMethodCmt + " **/")
      local isValid = isValidReturnTypeForInl(interopRetType)
      if (isValid) {
        foreach (pi in interopParams) {
          isValid = isValidParamTypeForInl(pi)
          if (!isValid)
            break;
        }
      }

      local inlFmtMethodDecl =
        mNamespaceStack.top()+","+
        mInterfaceStack.top()+","+
        name+","+
        params.len();

      if (!isValid) {
        parserWarning(mLineCount,"Invalid method for automation: "+inlFmtMethodCmt+".")
        outInl("IDLC_METH_INVALID_BEGIN("+inlFmtMethodDecl+")")
        outInl("IDLC_METH_INVALID_END("+inlFmtMethodDecl+")")
      }
      else {
        outInl("IDLC_METH_BEGIN("+inlFmtMethodDecl+")")
        foreach (pi in interopParams) {
          pi = ::LINT_AS_TYPE("::idlGen.tInteropParam", pi)
          local type = pi.typeval
          local baseType = type&::knTypeMask;
          local typeName = pi.typec
          if (typeName.startswith("const "))
            typeName = typeName.after("const ");
          if (baseType == ::eType.IUnknown) {
            outInl("\tIDLC_DECL_VAR("+typeName+","+pi.name+")")
            outInl("\tIDLC_BUF_TO_INTF("+typeName.rbefore("*").trim()+","+pi.name+")")
          }
          else if (pi.typei.contains("_Enum")) {
            outInl("\tIDLC_DECL_VAR("+typeName+","+pi.name+")")
            outInl("\tIDLC_BUF_TO_ENUM("+typeName+","+pi.name+")")
          }
          else if (typeName.contains("&")) {
            typeName = typeName.rbefore("&")
            outInl("\tIDLC_DECL_VAR("+typeName+","+pi.name+")")
            outInl("\tIDLC_BUF_TO_BASE("+pi.typei+","+pi.name+")")
          }
          else if (pi.typei.contains("_Ptr")) {
            outInl("\tIDLC_DECL_VAR("+typeName+","+pi.name+")")
            outInl("\tIDLC_BUF_TO_PTR("+pi.typei+","+pi.name+")")
          }
          else if ((type&::eTypeFlags.Pointer) &&
                   baseType != ::eType.AChar)
          {
            outInl("\tIDLC_DECL_VAR("+typeName+","+pi.name+")")
            outInl("\tIDLC_BUF_TO_NULL("+pi.typei+","+pi.name+")")
          }
          else if (baseType == ::eType.Null) {
            outInl("\tIDLC_DECL_VAR("+typeName+","+pi.name+")")
            outInl("\tIDLC_BUF_TO_NULL("+pi.typei+","+pi.name+")")
          }
          else {
            outInl("\tIDLC_DECL_VAR("+typeName+","+pi.name+")")
            outInl("\tIDLC_BUF_TO_BASE("+pi.typei+","+pi.name+")")
          }
        }
        {
          local pi = interopRetType
          local type = pi.typeval
          local baseType = type&::knTypeMask;
          local typeName = pi.typec
          if (!pi.typec.contains("*") && typeName.startswith("const "))
            typeName = typeName.after("const ");
          local inlFmtMethodParams = "(";
          foreach (i,pi in interopParams) {
            pi = ::LINT_AS_TYPE("::idlGen.tInteropParam", pi)
            inlFmtMethodParams += pi.name
            if (i+1 < interopParams.len())
              inlFmtMethodParams += ","
          }
          inlFmtMethodParams += ")"
          if (type == ::eType.Null) {
            outInl("\tIDLC_METH_CALL_VOID("+inlFmtMethodDecl+","+inlFmtMethodParams+")")
          }
          else {
            if (pi.typec.contains("&")) {
              local retTypeName = pi.typec
              local refType = pi.typec.rbefore("&")
              {
                if (retTypeName.startswith("const "))
                  retTypeName = retTypeName.after("const ");
                retTypeName = retTypeName.trim()
                if (refType.startswith("const "))
                  refType = refType.after("const ");
                refType = refType.trim()
              }
              outInl("\tIDLC_DECL_RETREFVAR("+retTypeName+",_Ret,"+refType+")")
            }
            else {
              outInl("\tIDLC_DECL_RETVAR("+typeName+",_Ret)")
            }
            outInl("\tIDLC_METH_CALL(_Ret,"+inlFmtMethodDecl+","+inlFmtMethodParams+")")
            if (baseType == ::eType.IUnknown) {
              if (!(pi.type.contains("Ptr<")) &&
                  (name.contains("Create") || name.contains("Grab") ||
                   name.contains("Clone") || name.contains("Copy")))
              {
                parserWarning(startLine,
                              inlFmtMethodCmt + ": Create|Grab|Clone|Copy methods shouldn't return a raw pointer, use Ptr<INTERFACE> instead.");
              }
              outInl("\tIDLC_RET_FROM_INTF("+typeName.rbefore("*").trim()+",_Ret)")
            }
            else if (pi.typei.contains("_Enum")) {
              outInl("\tIDLC_RET_FROM_ENUM("+typeName+",_Ret)")
            }
            else if (pi.typec.contains("&")) {
              outInl("\tIDLC_RET_FROM_BASE("+pi.typei+",_Ret)")
            }
            else if (((baseType == ::eType.Ptr) || (type&::eTypeFlags.Pointer)) &&
                     !pi.typei.contains("_U32") &&
                     !pi.typei.contains("_U64") &&
                     baseType != ::eType.AChar)
            {
              outInl("\tIDLC_RET_FROM_PTR("+pi.typei+",_Ret)")
            }
            else {
              outInl("\tIDLC_RET_FROM_BASE("+pi.typei+",_Ret)")
            }
          }
        }
        outInl("IDLC_METH_END("+inlFmtMethodDecl+")\n")
      }
      if (hasMinFeatures) {
        outInl("#endif // niMinFeatures("+hasMinFeatures+")\n")
      }
      outInl("\n")
    }

    dtPushNewNameAttrs("method",name,attributes)
    dtSet("java_name", getJavaName(name))
    dtPushComments(mComments);
    dtSet("num_parameters",params.len().tostring())
    if (isConst)
      dtSet("const","yes")
    dtPushNew("return")
    dtSet("type",interopRetType.type)
    if (!hasNoAutomation)
      dtSet("typei",interopRetType.typei)
    dtSet("typec",interopRetType.typec)
    dtPop()
    foreach (i,pi in interopParams) {
      pi = ::LINT_AS_TYPE("::idlGen.tInteropParam", pi)
      dtPushNewName("parameter",pi.name)
      dtSet("type",pi.type)
      if (!hasNoAutomation)
        dtSet("typei",pi.typei)
      dtSet("typec",pi.typec)
      if ("default" in pi)
        dtSet("default",pi[?"default"])
      if (i == interopParams.len()-1)
        dtSet("last","1")
      dtPop()
    }
    dtPop()
  }

  ///////////////////////////////////////////////
  function processConstVariable()
  {
    local startLine = mLineCount
    local attributes = getCommentsAttributes()
    local tokens
    // Parse the name and parents, all must be defined on one line
    tokens = tokenize(mLine,[";"])
    if (!tokens)
      parserError(startLine,"Unexpected end of file while parsing constant variable.")
    if (tokens.len() < 3)
      parserError(startLine,"Invalid constant variable declaration.")

    local typeAndName = [];
    local hasValue = false;
    local i = 0;
    for ( ; i < tokens.len(); ++i) {
      if (tokens[i] == "=") {
        hasValue = true;
        break;
      }
      typeAndName.append(tokens[i])
    }

    local value = ""
    if (hasValue) {
      if (i+2 >= tokens.len())  {
        parserError(startLine,"Invalid constant variable with value declaration.")
      }
      for (i += 1; tokens[i] != ";"; ++i) {
        value += tokens[i];
      }
    }

    local type = ""
    local name = typeAndName.top()
    typeAndName.pop()
    foreach (t in typeAndName) {
      type += t + " "
    }
    type = getValidTypeName(type)
    outLine("VAR: "+type+"; "+name+" = "+value)

    dtPushName("variable",name,attributes)
    dtSet("type",type)
    dtSet("typei",getInteropTypeName(type))
    dtSet("typec",getTypeClass(type))
    dtSet("value",value)
    dtPop()
  }

  ///////////////////////////////////////////////
  function skipPreprocessor(aStr)
  {
    outLine("###:"+aStr)
    if (aStr.endswith("\\")) {
      local startLine = mLineCount
      local str = readLine()
      while (!mSrc.partial_read && str.endswith("\\")) {
        str = readLine()
        outLine("###:"+str)
      }
      if (mSrc.partial_read) {
        parserError(startLine,"Unexpected end of file while skipping preprocessor directive.")
      }
    }
    else if (aStr.startswith("#endif")) {
      if (!mMinFeatures.empty()) {
        local f = mMinFeatures.top()
        if (--f.count == 0) {
          mMinFeatures.pop()
        }
      }
    }
    else if (aStr.startswith("#if")) {
      local toks = aStr.split(" \t()")
      if (toks.len() == 3 &&
          toks[0] == "#if" &&
          toks[1] == "niMinFeatures")
      {
        local v = toks[2].toint()
        mMinFeatures.push({
          count = 1,
          value = v
        })
      }
      else {
        if (!mMinFeatures.empty()) {
          local f = mMinFeatures.top();
          ++f.count;
        }
      }
    }
  }

  ///////////////////////////////////////////////
  function readLine()
  {
    mLineCount++;
    mRawLine = mSrc.ReadStringLine().trimex(" \t\n\r")
    return mRawLine.normalize();
  }

  ///////////////////////////////////////////////
  function parserError(aLine,aText)
  {
    throw mSrc.source_path+":"+aLine+": "+aText
  }

  ///////////////////////////////////////////////
  function parserWarning(aLine,aText)
  {
    ::println(mSrc.source_path+":"+aLine+": "+aText)
  }

  ///////////////////////////////////////////////
  function generateInPlace(aSource)
  {
    local fp = ::fs.fileOpenRead(aSource);
    if (!fp) throw "Can't open the source file :"+aSource
    local d = fp.source_path + ".idl.xml"
    fp = null
    return generate(aSource,d,d.setext("inl"))
  }

  ///////////////////////////////////////////////
  function generate(aSource, aDest, aDestInl)
  {
    try {
      reset()

      message("IDL: '"+aDest+"'\nfrom '"+aSource+"'");
      mSrc = ::fs.fileOpenRead(aSource);
      if (!mSrc) throw "Can't open the source file :"+aSource;

      //local pp = ::CreateInstance("niPreprocessor.Preprocessor")
      //if (!pp)  throw "Can't create the preprocessor.";
      //mSrc = pp.PreprocessFile(mSrc,0)
      //if (!mSrc)  throw "Can't create preprocess the source file :"+aSource;

      mDest = ::fs.fileOpenWrite(aDest);
      if (!::lang.isValid(mDest))
        throw "Can't open the destination file:"+aDest;

      if (aDestInl) {
        mDestInl = ::fs.fileOpenWrite(aDestInl);
        if (!::lang.isValid(mDestInl))
          throw "Can't open the destination vmcall file:"+aDestInl;

        outInl("// clang-format off")
        outInl("//")
        if (aSource.contains("/API/")) {
          outInl("// Autogenerated IDL definition for 'API/"+aSource.after("/API/")+"'.")
        }
        else {
          outInl("// Autogenerated IDL definition for '"+aSource.getfile()+"'.")
        }
        outInl("//")

        local incName = aSource.after("API/");
        if (incName && incName.len()) {
          outInl("#include \"../API/"+incName+"\"")
        }

        outInl("")
        outInl("#ifndef IDLC_BEGIN_NAMESPACE\n")
        outInl("#error \"IDLC_BEGIN_NAMESPACE not defined !\"\n")
        outInl("#endif\n")
        outInl("#ifndef IDLC_END_NAMESPACE\n")
        outInl("#error \"IDLC_END_NAMESPACE not defined !\"\n")
        outInl("#endif\n")
        outInl("")

        outInl("IDLC_BEGIN_NAMESPACE()\n")
        outInl("")
      }

      // Disabled, this is isn't used by any other scripts and
      // creates too frequent diffs
      // dtSet("source",::gLang.root_fs.GetAbsolutePath(aSource))
      // dtSet("destination",::gLang.root_fs.GetAbsolutePath(aDest))

      while (!mSrc.partial_read) {
        // Read the next line, remove the begining and end spaces, tabs, and newlines
        mLine = readLine();

        if (mLine.len()) {
          // Preprocessor
          if (mLine[0] == '#') {
            skipPreprocessor(mLine)
          }
          // Comments
          else if (processCommentsAndEmptyLines()) {
            continue; // skip reset comments
          }
          // Namespace
          else if (mLine.startswith("namespace")) {
            processNamespace()
          }
          // Blocks
          else if (mLine.find("{") != null) {
            skipBlock()
          }
          else {
            outLine("%%" + mLine)
          }
        }
        resetComments();
      }

      getCommentsAttributes()

      outLine("--- IDL DataTable ---")
      ::xml.toFile(mDT,mDest,0)

      outInl("IDLC_END_NAMESPACE()\n")
      outInl("// EOF //\n")
    }
    catch (e) {
      if (mDest) mDest.Invalidate()
      if (mDestInl) mDestInl.Invalidate()
      if (mDest) ::gRootFS.FileDelete(aDest)
      throw aSource+"("+mLineCount+"): Generation error : " + e
    }

    return mDT
  }
}
