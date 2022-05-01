#ifndef __PATH_39500984_H__
#define __PATH_39500984_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../StringDef.h"
#include "../STL/stack.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

//! Path utility class.
class cPath
{
 public:
  //! Empty constructor.
  inline cPath() {
  }

  //! cString constructor.
  inline cPath(const cString& aPath) {
    SetPath(aPath);
  }

  //! UTF8 string constructor.
  inline cPath(const achar* aPath) {
    SetPath(aPath);
  }

  //! Unicode string constructor.
  inline cPath(const uchar* auszStr) {
    SetPath(cString(auszStr).Chars());
  }

  //! Directory + File constructor.
  inline cPath(const achar* aszDirectory, const achar* aszFile) {
    SetDirectory(aszDirectory).SetFile(aszFile);
  }

  //! Directory + File constructor.
  inline cPath(const cString& aDirectory, const cString& aFile) {
    SetDirectory(aDirectory).SetFile(aFile);
  }

  //! Directory + File constructor.
  inline cPath(const cString& aDirectory, const achar* aszFile) {
    SetDirectory(aDirectory).SetFile(aszFile);
  }

  //! Return eTrue if the path is empty.
  inline tBool IsEmpty() const {
    return GetPath().IsEmpty();
  }

  //! Return eTrue if the path is not empty.
  inline tBool IsNotEmpty() const {
    return GetPath().IsNotEmpty();
  }

  //! Return eTrue if this path is a file.
  inline tBool IsFile() const {
    return mstrFile.IsNotEmpty();
  }

  //! Return eTrue if this path is a directory.
  inline tBool IsDirectory() const {
    return !IsFile();
  }

  //! Return eTrue if this path has a directory.
  inline tBool HasDirectory() const {
    return mstrDirectory.IsNotEmpty();
  }

  //! Return eTrue if the path is absolute.
  inline tBool IsAbsolute() const {
    return StrIsAbsolutePath(GetPath().Chars());
  }

  //! Return eTrue if the path is relative to the app's directory.
  inline tBool IsRelative() const {
    return !IsAbsolute();
  }

  //! Set the file of the path.
  inline cPath& SetFile(const achar* aszFile) {
    if (!niIsStringOK(aszFile)) {
      mstrFile.Clear();
    }
    else {
      FixPath(mstrFile,aszFile);
      tI32 pos = mstrFile.rfind('/');
      if (pos != cString::npos) {
        ++pos;
        mstrFile = mstrFile.substr(pos);
      }
    }

    _UpdatePath();
    return *this;
  }

  //! Set the file of the path.
  inline cPath& SetFile(const cString& v) {
    return SetFile(v.Chars());
  }

  //! Get the file of the path.
  inline const cString& GetFile() const {
    return mstrFile;
  }

  //! Set the file of the path, while preserving the previous file extension.
  inline cPath& SetFileNoExt(const achar* aFileName) {
    cString curExt = this->GetExtension();
    this->SetFile(aFileName);
    this->SetExtension(curExt.Chars());
    return *this;
  }

  //! Set the file of the path, while preserving the previous file extension.
  inline cPath& SetFileNoExt(const cString& v) {
    return SetFileNoExt(v.Chars());
  }

  //! Get the name of the file without the extension.
  inline cString GetFileNoExt() const {
    cString fileName = GetFile();
    tI32 pos = fileName.rfind(_A("."));
    if (pos != cString::npos)
      fileName.resize(pos);
    return fileName;
  }

  //! Set the file extension of the path.
  inline cPath& SetExtension(const achar* aszExtension) {
    tI32 pos = mstrFile.rfind(_A("."));
    if (pos != cString::npos)
      mstrFile.resize(pos);

    if (niIsStringOK(aszExtension)) {
      if (*aszExtension != '.') {
        mstrFile += _A(".");
      }
      mstrFile += aszExtension;
    }

    _UpdatePath();
    return *this;
  }

  //! Set the file extension of the path.
  inline cPath& SetExtension(const cString& v) {
    return SetExtension(v.Chars());
  }

  //! Get the extension of this file.
  inline const cString GetExtension() const {
    tI32 pos = mstrFile.rfind(_A("."));
    if (pos != cString::npos) {
      ++pos;
      return mstrFile.substr(pos);
    }
    return cString();
  }

  //! Set the directory of the path.
  inline cPath& SetDirectory(const achar* aszDirectory) {
    const cString protocol = GetProtocol();
    if (!niIsStringOK(aszDirectory)) {
      mstrDirectory.Clear();
    }
    else {
      FixPath(mstrDirectory,aszDirectory);
      if (!StrIsPathSep(mstrDirectory[mstrDirectory.Len()-1]))
        mstrDirectory.appendChar('/');
    }
    if (!protocol.IsEmpty() && StrFindProtocol(aszDirectory) < 0) {
      SetProtocol(protocol.Chars());
    }
    _UpdatePath();
    return *this;
  }

  //! Set the directory of the path.
  inline cPath& SetDirectory(const cString& v) {
    return SetDirectory(v.Chars());
  }

  //! Get the directory of the path.
  inline const cString& GetDirectory() const {
    return mstrDirectory;
  }

  //! Get the directory of the path without the protocol.
  inline cString GetDirectoryNoProtocol() const {
    const tI32 ppos = StrFindProtocol(mstrDirectory.Chars());
    if (ppos >= 0) {
      return mstrDirectory.substr(ppos + 3);
    }
    else {
      return GetDirectory();
    }
  }

  //! Check whether the path has a protocol defined.
  inline tBool HasProtocol() const {
    const tI32 ppos = StrFindProtocol(mstrDirectory.Chars());
    return ppos >= 0;
  }

  //! Set the file protocol of the path.
  inline cPath& SetProtocol(const achar* aszProtocol) {
    const tI32 ppos = StrFindProtocol(mstrDirectory.Chars());
    if (ppos < 0) {
      if (niStringIsOK(aszProtocol)) {
        mstrDirectory = _ASTR(aszProtocol) + "://" + mstrDirectory;
      }
      else {
        // nothing changed
        return *this;
      }
    }
    else {
      if (niStringIsOK(aszProtocol)) {
        mstrDirectory = _ASTR(aszProtocol) + mstrDirectory.substr(ppos);
      }
      else {
        mstrDirectory = mstrDirectory.substr(ppos+3);
      }
    }

    _UpdatePath();
    return *this;
  }

  //! Set the file protocol of the path.
  inline cPath& SetProtocol(const cString& v) {
    return SetProtocol(v.Chars());
  }

  //! Get the protocol of this file.
  inline const cString GetProtocol() const {
    return StringURLGetProtocol(mstrDirectory.Chars());
  }

  //! Add a directory to the front of the path.
  //! \remark Not meant be used with absolute directories. It'll lead to invalid paths, ex: 'N:/foo' -> 'front_dir/N:/foo'
  inline cPath& AddDirectoryFront(const achar* aszDirectory) {
    niAssert(!IsAbsolute()); // shouldn't be used with an absolute directory
    cPath path(aszDirectory, NULL);
    path.AddDirectoryBack(GetDirectoryNoProtocol().Chars());
    return SetDirectory(path.GetDirectory().Chars());
  }

  inline cPath& AddDirectoryFront(const cString& v) {
    return AddDirectoryFront(v.Chars());
  }

  //! Add a directory to the bakc of the path.
  inline cPath& AddDirectoryBack(const achar* aszDirectory) {
    cString strDirectory = mstrDirectory;
    strDirectory += aszDirectory;
    return SetDirectory(strDirectory.Chars());
  }

  inline cPath& AddDirectoryBack(const cString& v) {
    return AddDirectoryBack(v.Chars());
  }

  //! Remove the directory on the right of the path.
  inline cPath& RemoveDirectoryBack() {
    cString newDir = GetDirectory();
    if (newDir == cString(_A("//")))
      return *this;
    newDir.TrimRightEx(_A("/"));
    tI32 pos = newDir.rfind(_A("/"));
    if (pos != cString::npos) {
      newDir = newDir.substr(0,pos);
      SetDirectory(newDir.Chars());
    }
    else {
      SetDirectory(_A("//"));
    }
    return *this;
  }

  //! Push the current directory.
  inline cPath& PushDirectory() {
    mstkDirectory.push(mstrDirectory);
    return *this;
  }

  //! Pop the latest pushed directory.
  inline cPath& PopDirectory() {
    niAssert(!mstkDirectory.empty());
    mstrDirectory = mstkDirectory.top();
    mstkDirectory.pop();
    _UpdatePath();
    return *this;
  }

  //! Set the full path.
  inline cPath& SetPath(const achar* aszPath) {
    if (!niIsStringOK(aszPath)) {
      mstrDirectory.Clear();
      mstrFile.Clear();
      mstrPathPrivate.Clear();
      mbPathDirty = eFalse;
    }
    else {
      FixPath(mstrPathPrivate,aszPath);
      mbPathDirty = eFalse;
      tI32 pos = GetPath().rfind('/');
      if (pos != cString::npos) {
        ++pos;
        mstrDirectory = GetPath().substr(0,pos);
        mstrFile = GetPath().substr(pos);
      }
      else {
        mstrDirectory.Clear();
        mstrFile = mstrPathPrivate;
      }
    }
    return *this;
  }

  //! Set the full path.
  inline cPath& SetPath(const cString& aPath) {
    return SetPath(aPath.Chars());
  }

  //! Get the full path.
  inline const cString& GetPath() const {
    if (mbPathDirty) {
      // Not using clear to avoid a memory allocation here.
      niThis(cPath)->mstrPathPrivate.resize(0);
      if (mstrDirectory.IsNotEmpty()) {
        niThis(cPath)->mstrPathPrivate += mstrDirectory;
      }
      if (mstrFile.IsNotEmpty()) {
        niThis(cPath)->mstrPathPrivate += mstrFile;
      }
      niThis(cPath)->mbPathDirty = eFalse;
    }
    return mstrPathPrivate;
  }

  //! Get the full path's chars.
  inline const achar* Chars() const {
    return GetPath().Chars();
  }
  //! Get the full path's chars.
  inline const achar* c_str() const {
    return GetPath().c_str();
  }

  //! Set the full path.
  inline cPath& operator = (const cPath& aPath) {
    SetPath(aPath.GetPath().Chars());
    return *this;
  }

  //! Set the full path.
  inline cPath& operator = (const achar* aszPath) {
    SetPath(aszPath);
    return *this;
  }

  //! Set the full path.
  inline cPath& operator = (const cString& aPath) {
    SetPath(aPath.Chars());
    return *this;
  }

  //! Convert a string to a standard path
  static inline cString& FixPath(cString& aOut, const achar* aaszPath) {
    const achar* p = aaszPath;
    aOut.resize(0);
    if (StrStartsWith(p,_A("\\\\"))) {
      // Windows network path...
      aOut.appendChar('\\');
      aOut.appendChar('\\');
      p += StrOffset(p,2);
    }
    while (*p) {
      tU32 c = StrGetNextX(&p);
      if (!c) {
        aOut.appendChar('?');
        ++p;
      }
      else if (StrIsPathSep(c)) {
        aOut.appendChar('/');
      }
      else if (c != '"') {
        aOut.appendChar(c);
      }
    }
    aOut = aOut.GetWithoutBEQuote();
    return aOut;
  }

 private:
  cString mstrDirectory;
  astl::stack<cString>  mstkDirectory;

  cString mstrFile;

  inline void _UpdatePath() {
    mbPathDirty = eTrue;
  }
  cString mstrPathPrivate;
  tBool   mbPathDirty;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __PATH_39500984_H__
