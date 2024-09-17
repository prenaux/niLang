// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("lang.ni")
::Import("seq.ni")
::Import("regex.ni")
::Import("fs.ni")

::gConcurrent <- ::LINT_AS_TYPE("iConcurrent",::Concurrent_Get())

::concurrent <- {

  function isConcurrent() {
    return true
  }

  function newRunnable(aFunOrTable,_aThis) {
    switch (typeof aFunOrTable) {
      case "closure":
      case "function": {
        local _this = _aThis || this
        return ::Concurrent_RunnableFromFunction(function() : (aFunOrTable,_this) {
          try {
            return aFunOrTable.call(_this);
          }
          catch (e) {
            // ::println("E/Runnable function throwed: " + e);
            return e;
          }
        })
      }
      case "table": {
        local _this = aFunOrTable
        if (!("Run" in _this)) {
          throw "The specified table doesn't have a Run method."
        }
        local theMethod = _this.?Run
        return ::Concurrent_RunnableFromFunction(#():(_this,theMethod) {
          try {
            return theMethod.call(_this);
          }
          catch (e) {
            // ::println("E/Runnable table throwed: " + e);
            return e;
          }
        })
      }
      case "iunknown": {
        // assume its already a iRunnable
        return aFunOrTable;
      }
    }
    throw "Invalid function type: " + typeof(aFunOrTable)
  }

  function newCallback(aFunOrTable,aThis) {
    switch (typeof aFunOrTable) {
      case "closure":
      case "function": {
        local _this = aThis || this
        return ::Concurrent_CallbackFromFunction(function(aA,aB) : (aFunOrTable,_this) {
          try {
            return aFunOrTable.call(_this,aA,aB);
          }
          catch (e) {
            // ::println("E/Callback function throwed: " + e);
            return e;
          }
        })
      }
      case "table": {
        local _this = aFunOrTable
        local runMethodName = "RunCallback"
        if (!(runMethodName in _this)) {
          runMethodName = "Run"
          if (!(runMethodName in _this)) {
            throw "The specified table doesn't have a RunCallback nor a Run method."
          }
        }
        local theMethod = _this[?runMethodName]
        return ::Concurrent_CallbackFromFunction(#(aA,aB):(_this,theMethod) {
          try {
            return theMethod.call(_this,aA,aB);
          }
          catch (e) {
            // ::println("E/Callback table throwed: " + e);
            return e;
          }
        })
      }
      case "iunknown": {
        // assume its already a iCallback
        return aFunOrTable;
      }
    }
    throw "Invalid function type: " + typeof(aFunOrTable)
  }

  function ioRun(aRunnable,_aThis) iFuture {
    local r = ::concurrent.newRunnable(aRunnable,_aThis)
    return ::gConcurrent.executor_io.Submit(r)
  }

  function cpuRun(aRunnable,_aThis) iFuture {
    local r = ::concurrent.newRunnable(aRunnable,_aThis)
    return ::gConcurrent.executor_cpu.Submit(r)
  }

  function mainRun(aRunnable,_aThis) iFuture {
    local r = ::concurrent.newRunnable(aRunnable,_aThis)
    return ::gConcurrent.executor_main.Submit(r)
  }

  function update(aTimeSlice) {
    aTimeSlice = aTimeSlice || 5;
    return ::gConcurrent.executor_main.Update(aTimeSlice)
  }

  function getOrCreateMessageQueue(anThreadID, aMaxItems) {
    anThreadID = (anThreadID == null ? ::gConcurrent.current_thread_id : anThreadID)
    local mq = ::gConcurrent.message_queue[anThreadID]
    if (mq)
      return mq
    aMaxItems = aMaxItems || ~0
    return ::gConcurrent.CreateMessageQueue(anThreadID,aMaxItems)
  }
}

::ioRun <- concurrent.ioRun
::cpuRun <- concurrent.cpuRun
::mainRun <- concurrent.mainRun

// This is set as the concurrent child vm root table's delegate.
// In the future it should be made immutable.
::__concurrent_child_vm_root <- {
  lang = {
    toString = ::lang.toString
    toPrintString = ::lang.toPrintString
  }
  eTextEncodingFormat = ::eTextEncodingFormat

  eLogFlags = ::eLogFlags
  log <- ::lang.log
  logWarning <- ::lang.logWarning
  logError <- ::lang.logError
  trace <- ::lang.dbg
  dbg <- ::lang.dbg
  dbgs <- ::lang.dbgs
  print <- ::lang.print
  println <- ::lang.println
  isValid <- ::lang.isValid
  cat <- ::lang.toPrintString

  format <- ::format
  getbasestackinfos <- ::getbasestackinfos

  eSerializeMode = ::eSerializeMode
  eSerializeFlags = ::eSerializeFlags

  FourCC = ::FourCC

  lang = ::lang
  gLang = ::gLang
  concurrent = ::concurrent
  Concurrent_RunnableFromFunction = ::Concurrent_RunnableFromFunction
  Concurrent_CallbackFromFunction = ::Concurrent_CallbackFromFunction
  ioRun = ::ioRun
  cpuRun = ::cpuRun
  mainRun = ::mainRun
  gConcurrent = ::gConcurrent

  fs = ::fs
  gRootFS = ::gRootFS
  eFileFlags = ::eFileFlags
  eFileAttrFlags = ::eFileAttrFlags
  eFileOpenMode = ::eFileOpenMode

  regex = ::regex
  seq = ::seq

  SetRaiseErrorMode = ::SetRaiseErrorMode
  setraiseerrormode = ::setraiseerrormode
  toString = ::lang.toString
  toPrintString = ::lang.toPrintString
  clock = ::clock
  Vector = ::Vector
  Map = ::Map
  Vec2 = ::Vec2
  Vec3 = ::Vec3
  Vec4 = ::Vec4
  RGB = ::RGB
  RGBA = ::RGBA
  Rect = ::Rect
  Matrix = ::Matrix
  function print(_args_) {
    ::vmprint(toPrintString(_args_))
  }
  function println(_args_) {
    ::vmprintln(toPrintString(_args_))
  }
}
