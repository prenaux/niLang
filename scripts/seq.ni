// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("lang.ni")

module_seq <- {
  // Create a sequence table between aBegin/aLast, if aFun isn't it will be
  // called with each index and the return value of the function will be the
  // value returned by the iterator.
  function range(aBegin,aLast,_aFun) {
    if (typeof aLast == "function") {
      // new(0-num,aFun(idx))
      _aFun = aLast
      aLast = aBegin
      aBegin = 0
    }
    if (aBegin > aLast)
      throw "Invalid range, begin > last"
    local r = {
      _start = aBegin
      _last = aLast
      function len() {
        return _last-_start
      }
      function empty() {
        return _start != _last
      }
      function _nexti(itr) : (_aFun) {
        local r
        if (itr == null) {
          r = _start;
          itr = [r,r]
        }
        else {
          r = ++itr[0]
        }
        if (r >= _last)
          return null
        itr[0] = r
        itr[1] = _aFun ? _aFun(r) : r
        return itr
      }
    }
    r.__SetCanCallMetaMethod(true)
    return r
  }

  // Create a sequence which iterates over the lines of the specified file.
  function fileLines(aFilePath) {
    local r = {
      _count = 0
      _resume = false
      _fileStart = null
      _fp = ::lang.urlOpen(aFilePath)
      function reset(_aSeekTo) {
        _count = 0;
        if (_fp) {
          if (_aSeekTo != null) {
            _fileStart = _aSeekTo;
          }
          else if (_fileStart == null) {
            // first time reset is called we initialize _fileStart to the current
            // position in the file
            _fileStart = _fp.Tell();
          }
          _fp.SeekSet(_fileStart);
          // BeginTextFileRead resets the partial_read flag
          _fp.BeginTextFileRead(false);
        }
      }
      function empty() {
        return !_fp || (_fp.GetSize() <= 0)
      }
      function resume() {
        _resume = true;
        return this;
      }
      function _nexti(itr) {
        if (itr == null) {
          // initialize _fileStart if not already done
          if (_resume) {
            // ::println("... SEQ RESUME ITERATOR:" _fp.source_path _count _fileStart)
            _resume = false;
          }
          else {
            // ::println("... SEQ RESET ITERATOR:" _fp.source_path)
            reset();
          }
          // don't reset the counter here...
          itr = [_count,_count]
        }
        if (!_fp || _fp.partial_read)
          return null
        itr[0] = _count++;
        itr[1] = _fp.ReadStringLine();
        return itr;
      }
    }
    r.__SetCanCallMetaMethod(true)
    return r
  }

  // foreach (key,value in ::seq.directory(...))  => key = itr[0], value = itr[1]
  function directory(aDirectory) {
    local r = {
      _e = ::?gRootFS.CreateEnumerator()
      _count = 0
      _directory = "".setdir(aDirectory).setfile("*")
      function _nexti(itr) {

        if (itr == null) {
          if (!_e.FindFirst(_directory))
            return null;
          itr = [null,null];
          _count = 0;
        }
        else {
          if (!_e.FindNext())
            return null;
        }
        itr[0] = _count++;
        itr[1] = {
          file_name = _e.file_name
          file_size = _e.file_size
          file_attributes = _e.file_attributes
          file_system = _e.file_system
          file_time = _e.file_time.Clone()
        }
        return itr;
      }
    }
    r.__SetCanCallMetaMethod(true)
    return r
  }

  // Creates a new sequenced which is filtered with the specified
  // aIteratorFilter.
  //
  // The filter should preserve the iterator since _nexti could rely on the
  // iterator to remain with a certain structure. The filter should only
  // augment the iterator, this of course works well only will iterator values
  // than can be reasonable augmented, such as strings or tables.
  function filtered(aSeq,aIteratorFilter) {
    if (!aIteratorFilter)
      throw "Invalid filter"

    local r = {
      _filter = aIteratorFilter
      _seq = aSeq
      function empty() {
        return _seq.empty()
      }
      function _nexti(itr) {
        return _filter(_seq._nexti(itr))
      }
    }
    r.__SetCanCallMetaMethod(true)
    return r
  }
}

::seq <- module_seq
