::Import("lang.ni")

::namespace("serialize", {

  //========================================================================
  // Write
  //========================================================================
  function writeValue(fp,aValue,aRecursionGuard) {
    local valType = typeof(aValue);
    switch (valType) {
      case "null": {
        fp.WriteStringZ("nul")
        break;
      }
      case "string": {
        fp.WriteStringZ("str")
        fp.WriteStringZ(aValue)
        break;
      }
      case "int": {
        fp.WriteStringZ("i32")
        fp.WriteLE32(aValue)
        break;
      }
      case "float": {
        fp.WriteStringZ("f32")
        fp.WriteF32(aValue)
        break;
      }
      case "array": {
        return writeArray(fp,aValue,aRecursionGuard)
      }
      case "table": {
        return writeTable(fp,aValue,aRecursionGuard)
      }
      case "function": {
        return writeFunction(fp,aValue,aRecursionGuard)
      }
      default: {
        throw "writeValue: Can't serialize type '"+valType+"'."
      }
    }
  }

  function writeArray(fp,aArray,aRecursionGuard) {
    local guardKey = aArray.ToIntPtr()
    if (guardKey in aRecursionGuard)
      throw "writeArray: Array "+guardKey+" already serialized, recursive structures cannot be serialized."
    aRecursionGuard[guardKey] <- true

    fp.WriteStringZ("arr")
    fp.WriteLE32(aArray.len())
    foreach (idx,value in aArray) {
      writeValue(fp,value,aRecursionGuard)
    }

    delete aRecursionGuard[guardKey]
  }

  function writeTable(fp,aTable,aRecursionGuard) {
    local guardKey = aTable.ToIntPtr()
    if (guardKey in aRecursionGuard)
      throw "writeTable: Table "+guardKey+" already serialized, recursive structures cannot be serialized."
    aRecursionGuard[guardKey] <- true

    fp.WriteStringZ("tbl")
    foreach (key,value in aTable) {
      writeValue(fp,key,aRecursionGuard)
      writeValue(fp,value,aRecursionGuard)
    }
    fp.WriteStringZ("end")

    delete aRecursionGuard[guardKey]
  }

  function writeFunction(fp,aFunction,aRecursionGuard) {
    local guardKey = aFunction.ToIntPtr()
    if (guardKey in aRecursionGuard)
      throw "writeFunction: Function "+guardKey+" already serialized, recursive structures cannot be serialized."
    aRecursionGuard[guardKey] <- true

    fp.WriteStringZ("fun")
    local vm = ::GetThisVM()
    if (!vm.WriteClosure(fp,::ObjectToIScriptObject(aFunction)))
      throw "writeFunction: Can't write closure."

    delete aRecursionGuard[guardKey]
  }

  //========================================================================
  // Read
  //========================================================================
  function readValue(fp,typeHdr) {
    typeHdr = typeHdr || fp.ReadString();
    switch (typeHdr) {
      case "nul": {
        return null
      }
      case "str": {
        return fp.ReadString()
      }
      case "i32": {
        return fp.ReadLE32()
      }
      case "f32": {
        return fp.ReadF32()
      }
      case "arr": {
        return readArray(fp,typeHdr)
      }
      case "tbl": {
        return readTable(fp,typeHdr)
      }
      case "fun": {
        return readFunction(fp,typeHdr)
      }
      default: {
        throw "readValue: Can't serialize type '"+typeHdr+"'."
      }
    }
  }

  function readArray(fp,typeHdr) {
    typeHdr = typeHdr || fp.ReadString()
    ::assertEquals(typeHdr,"arr")
    local arrSize = fp.ReadLE32()
    local arr = []
    arr.resize(arrSize)
    foreach (idx in arrSize) {
      arr[idx] = readValue(fp)
    }
    return arr
  }

  function readTable(fp,typeHdr) {
    typeHdr = typeHdr || fp.ReadString()
    ::assertEquals("tbl",typeHdr)
    local table = {}
    for (;;) {
      local valType = fp.ReadString()
      if (valType == "end")
        break;
      local key = readValue(fp,valType)
      local val = readValue(fp)
      table[key] <- val
    }
    return table
  }

  function readFunction(fp,typeHdr) {
    typeHdr = typeHdr || fp.ReadString()
    ::assertEquals("fun",typeHdr)

    local vm = ::GetThisVM()
    local scriptObject = vm.ReadClosure(fp)
    if (!scriptObject)
      throw "readFunction: Can't read closure."

    return ::IScriptObjectToObject(scriptObject)
  }
})
