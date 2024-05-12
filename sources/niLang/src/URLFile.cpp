// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "Lang.h"

#define TRACE_URLHANDLER_MANIFEST(X)
#define TRACE_FILEHANDLER(X) //niDebugFmt(X)
#define TRACE_FFP_TRYALLDIRS(FMT) //niDebugFmt(FMT)
#define TRACE_FFP_WARN(FMT) //niWarning(FMT)

#include "API/niLang/Utils/URLFileHandlerManifest.h"
#include "API/niLang/Utils/Path.h"

using namespace ni;

// Directly uses a file system
struct URLFileHandler_FileSystem : public ImplRC<iURLFileHandler> {
  Ptr<iFileSystem> _fileSystem;
  URLFileHandler_FileSystem(iFileSystem* apFileSystem) {
    niAssert(niIsOK(apFileSystem));
    _fileSystem = apFileSystem;
  }

  ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) niImpl {
    if (aIID == niGetInterfaceUUID(ni::iFileSystem))
      return _fileSystem;
    return BaseImpl::QueryInterface(aIID);
  }
  void __stdcall ListInterfaces(ni::iMutableCollection* apLst, ni::tU32 anFlags) const niImpl {
    apLst->Add(niGetInterfaceUUID(ni::iFileSystem));
    BaseImpl::ListInterfaces(apLst,anFlags);
  }

  virtual iFile* __stdcall URLOpen(const achar* aURL) {
    const cString path = StringURLGetPath(aURL);
    TRACE_FILEHANDLER(("... URLFileHandler_FileSystem.URLOpen: %s", path));
    return _fileSystem->FileOpen(path.Chars(),eFileOpenMode_Read);
  }

  virtual tBool __stdcall URLExists(const achar* aURL) {
    const cString path = StringURLGetPath(aURL);
    return _fileSystem->FileExists(path.Chars(),eFileAttrFlags_AllFiles);
  }
};

niExportFunc(iUnknown*) New_niLang_URLFileHandlerFileSystem(const Var& avarA, const Var& avarB) {
  QPtr<iFileSystem> fileSystem = avarA;
  niCheckIsOK(fileSystem,NULL);
  return niNew URLFileHandler_FileSystem(fileSystem);
}

// Optionally loads a manifest file to get a resource path and then delegate
// to a filesystem, look in a "data" subfolder first.
struct URLFileHandler_ManifestFileSystem : public URLFileHandler_Manifest {
  Ptr<iFileSystem> _fileSystem;
  URLFileHandler_ManifestFileSystem(iFileSystem* apFileSystem) {
    niAssert(niIsOK(apFileSystem));
    _fileSystem = apFileSystem;
  }

  ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) niImpl {
    if (aIID == niGetInterfaceUUID(ni::iFileSystem))
      return _fileSystem;
    return BaseImpl::QueryInterface(aIID);
  }
  void __stdcall ListInterfaces(ni::iMutableCollection* apLst, ni::tU32 anFlags) const niImpl {
    apLst->Add(niGetInterfaceUUID(ni::iFileSystem));
    BaseImpl::ListInterfaces(apLst,anFlags);
  }

  tBool _GetExistingPath(const achar* aURL, cString& aPath) {
    aPath = this->GetPathFromURL(aURL);

    // Test with the data dir first, this should be the most common since the
    // default URL handler indicates relative paths.
    const cString dataDir = ni::GetLang()->GetProperty("ni.dirs.data");
    if (!dataDir.empty()) {
      const cString testPath = dataDir + aPath;
      TRACE_FILEHANDLER(("... URLFileHandler_ManifestFileSystem.GetExisting: %s", testPath));
      if (_fileSystem->FileExists(testPath.Chars(), eFileAttrFlags_AllFiles)) {
        aPath = testPath;
        return eTrue;
      }
    }

    // Check the path as-is.
    TRACE_FILEHANDLER(("... URLFileHandler_ManifestFileSystem.GetExisting.asis: %s", aPath));
    if (_fileSystem->FileExists(aPath.Chars(), eFileAttrFlags_AllFiles)) {
      return eTrue;
    }

    return eFalse;
  }

  virtual iFile* __stdcall URLOpen(const achar* aURL) {
    cString path;
    if (!_GetExistingPath(aURL,path)) {
      TRACE_FILEHANDLER(("... URLFileHandler_ManifestFileSystem.URLOpen.noexisting: %s", aURL));
      return NULL;
    }
    TRACE_FILEHANDLER(("... URLFileHandler_ManifestFileSystem.URLOpen: %s", path));
    return _fileSystem->FileOpen(path.Chars(),eFileOpenMode_Read);
  }

  virtual tBool __stdcall URLExists(const achar* aURL) {
    cString path;
    return _GetExistingPath(aURL,path);
  }
};

niExportFunc(iUnknown*) New_niLang_URLFileHandlerManifestFileSystem(const Var& avarA, const Var& avarB) {
  QPtr<iFileSystem> fileSystem = avarA;
  niCheckIsOK(fileSystem,NULL);
  return niNew URLFileHandler_ManifestFileSystem(fileSystem);
}

// Decode a URL and then delegate the decoded result to the regular URLOpen
// TODO: Prevent infinite loop if the decode url decodes the url:// protocol...
struct URLFileHandler_URL : public ImplRC<iURLFileHandler> {
  virtual iFile* __stdcall URLOpen(const achar* aURL) niImpl {
    cString urlDecoded;
    StringDecodeUrl(urlDecoded, StringURLGetPath(aURL));
    return ni::GetLang()->URLOpen(urlDecoded.Chars());
  }
  virtual tBool __stdcall URLExists(const achar* aURL) niImpl {
    cString urlDecoded;
    StringDecodeUrl(urlDecoded, StringURLGetPath(aURL));
    return ni::GetLang()->URLExists(urlDecoded.Chars());
  }
};

niExportFunc(iUnknown*) New_niLang_URLFileHandlerURL(const Var& avarA, const Var& avarB) {
  return niNew URLFileHandler_URL();
}


struct URLFileHandler_Prefixed : public ImplRC<iURLFileHandler> {
  Ptr<iURLFileHandler> _handler;
  cString _prefix;

  URLFileHandler_Prefixed(iURLFileHandler* apURLFileHandler, const achar* aPrefix) {
    _handler = apURLFileHandler;
    _prefix = aPrefix;
  }

  iFile* __stdcall URLOpen(const achar* aURL) niImpl {
    const cString path = _prefix + StringURLGetPath(aURL);
    return _handler->URLOpen(path.Chars());
  }

  tBool __stdcall URLExists(const achar* aURL) niImpl {
    const cString path = _prefix + StringURLGetPath(aURL);
    return _handler->URLExists(path.Chars());
  }
};
niExportFunc(iUnknown*) New_niLang_URLFileHandlerPrefixed(const Var& avarA, const Var& avarB) {
  QPtr<iURLFileHandler> handler = avarA;
  niCheckIsOK(handler,NULL);
  cString prefix = VarGetString(avarB);
  niCheck(prefix.IsNotEmpty(),NULL);
  return niNew URLFileHandler_Prefixed(handler,prefix.Chars());
}

static void _RegisterDefaultURLFileHandler() {
  static tBool _bInitialized = eFalse;
  if (_bInitialized)
    return;
  _bInitialized = eTrue;

  tGlobalInstanceCMap* imap = ni::GetLang()->GetGlobalInstanceMap();
  const tBool hasDefaultHandler = (imap->find("URLFileHandler.default") != imap->end());
  if (!hasDefaultHandler) {
    Ptr<iURLFileHandler> defaultHandler = niNew URLFileHandler_ManifestFileSystem(ni::GetRootFS());
    astl::upsert(*imap, _ASTR("URLFileHandler.default"), defaultHandler);
  }
  const tBool hasFileHandler = (imap->find("URLFileHandler.file") != imap->end());
  if (!hasFileHandler) {
    Ptr<iURLFileHandler> fileHandler = niNew URLFileHandler_FileSystem(ni::GetRootFS());
    astl::upsert(*imap, _ASTR("URLFileHandler.file"), fileHandler);
  }
  const tBool hasURLHandler = (imap->find("URLFileHandler.url") != imap->end());
  if (!hasURLHandler) {
    Ptr<iURLFileHandler> URLHandler = niNew URLFileHandler_URL();
    astl::upsert(*imap, _ASTR("URLFileHandler.url"), URLHandler);
  }
}

/////////////////////////////////////////////////////////////////
cString __stdcall cLang::URLGetProtocol(const achar* aURL) {
  cString r = StringURLGetProtocol(aURL);
  if (r.empty()) {
    if (aURL && *aURL && (
            // root directory ?
            (aURL[0] == '\\' || aURL[0] == '/') ||
            // relative file path
            (aURL[0] == '.' && (aURL[1] == '/' || aURL[1] == '\\'))  ||
            (aURL[0] == '.' && aURL[1] == '.' && (aURL[2] == '/' || aURL[2] == '\\')) ||
            // windows drive ?
            (((aURL[0] >= 'a' && aURL[0] <= 'z') ||
              (aURL[0] >= 'A' && aURL[0] <= 'Z'))
              && aURL[1] == ':')
       ))
    {
      r = "file";
    }
    else {
      r = "default";
    }
  }
  return r;
}

/////////////////////////////////////////////////////////////////
iURLFileHandler* __stdcall cLang::URLGetHandler(const achar* aURL) {
  _RegisterDefaultURLFileHandler();

  const cString protocol = URLGetProtocol(aURL);
  cString strHandler = "URLFileHandler.";
  strHandler += protocol;

  tGlobalInstanceCMap* imap = ni::GetLang()->GetGlobalInstanceMap();
  tGlobalInstanceCMap::const_iterator it = imap->find(strHandler);
  if (it == imap->end())
    return NULL;

  QPtr<iURLFileHandler> handler = it->second.ptr();
  return handler;
}

/////////////////////////////////////////////////////////////////
iFile* __stdcall cLang::URLOpen(const achar* aURL) {
  _RegisterDefaultURLFileHandler();

  Ptr<iURLFileHandler> handler = URLGetHandler(aURL);
  if (!handler.IsOK()) {
    niError(niFmt("Can't get the URLFileHandler for '%s'.", aURL));
    return NULL;
  }
  return handler->URLOpen(aURL);
}

/////////////////////////////////////////////////////////////////
tBool __stdcall cLang::URLExists(const achar* aURL) {
  _RegisterDefaultURLFileHandler();

  Ptr<iURLFileHandler> handler = URLGetHandler(aURL);
  if (!handler.IsOK()) {
    return eFalse;
  }
  return handler->URLExists(aURL);
}

///////////////////////////////////////////////
static inline cString _TrySinglePath(const achar* aszRes) {
  if (ni::GetLang()->URLExists(aszRes)) {
    return ni::URLExistsComputeShortestRelativeFilePath(aszRes);
  }
  return "";
}

static cString _TryPathAndBasePath(const cPath& aResPath, const cPath& aBasePath) {
  cString ret;

  // Try the path itself
  if (ret.empty()) {
    ret = _TrySinglePath(aResPath.GetPath().Chars());
  }

  // Try with the base path before the path provided, base path relative
  if (ret.empty() && aBasePath.IsNotEmpty() && !aResPath.IsAbsolute()) {
    cPath testPath;
    testPath.SetDirectory(aBasePath.GetDirectory().Chars());
    testPath.AddDirectoryBack(aResPath.GetDirectory().Chars());
    testPath.SetFile(aResPath.GetFile().Chars());
    cString r = _TrySinglePath(testPath.GetPath().Chars());
    if (!r.empty())
      return r;
  }

  // Try with the base path using the filename provided
  if (ret.empty() && aBasePath.IsNotEmpty()) {
    cPath testPath;
    testPath.SetDirectory(aBasePath.GetDirectory().Chars());
    testPath.SetFile(aResPath.GetFile().Chars());
    cString r = _TrySinglePath(testPath.GetPath().Chars());
    if (!r.empty())
      return r;
  }

  return ret;
}

static cString _TryPathAllDirs(const cPath& aResPath, const cPath& aBasePath) {
  TRACE_FFP_TRYALLDIRS(("... _TryPathAllDirs: %s, %s", aResPath.GetPath(), aBasePath.GetPath()));

  astl::vector<cString> parts;
  ni::StringSplit(aResPath.GetPath().Chars(),"/",&parts);
  ni::tU32 addDirs = 0;
  niLoopr(i,(tI32)parts.size()) {
    cPath path;
    if (addDirs > 0) {
      niLoop(j,addDirs) {
        path.AddDirectoryBack(parts[parts.size()-addDirs-1+j].Chars());
      }
    }
    ++addDirs;
    path.SetFile(parts.back().Chars());

    TRACE_FFP_TRYALLDIRS(("... Trying: %s", path.GetPath()));
    cString r = _TryPathAndBasePath(path,aBasePath);
    if (!r.empty()) {
      TRACE_FFP_TRYALLDIRS(("... Found: %s", r));
      return r;
    }
  }

  TRACE_FFP_TRYALLDIRS(("... Couldn't find the file."));
  return "";
}

cString __stdcall _URLFindFilePath(const achar* aszRes, const achar* aszBasePath, const cString* aLoaderPrefix)
{
  const cPath resPath(aszRes);
  const cPath basePath(aszBasePath);

  cString ret;
  const cString ext = _ASTR(aszRes).RAfter(".").ToLower();
  if (aLoaderPrefix && ext.empty()) {
    Ptr<tGlobalInstanceCMap> mapGI = ni::GetLang()->GetGlobalInstanceMap();

    // try to find the file with the base path
    if (ret.empty()) {
      for (tGlobalInstanceCMap::const_iterator it = mapGI->lower_bound(*aLoaderPrefix);
            it != mapGI->end() && it->first.StartsWith(aLoaderPrefix->Chars()); ++it)
      {
        cPath path(aszRes);
        path.SetExtension(it->first.After(*aLoaderPrefix).Chars());

        ret = _TryPathAndBasePath(path.GetPath().Chars(), basePath);
        if (!ret.empty())
          break;
      }
    }

    // try to find the file with all the subdir components
    if (ret.empty()) {
      for (tGlobalInstanceCMap::const_iterator it = mapGI->lower_bound(*aLoaderPrefix);
            it != mapGI->end() && it->first.StartsWith(aLoaderPrefix->Chars()); ++it)
      {
        cPath path(aszRes);
        path.SetExtension(it->first.After(*aLoaderPrefix).Chars());

        ret = _TryPathAllDirs(path.GetPath().Chars(), aszBasePath);
        if (!ret.empty())
          break;
      }
    }
  }
  else {
    ret = _TryPathAndBasePath(resPath, basePath);
    if (ret.empty()) {
      TRACE_FFP_WARN(niFmt("Couldn't find directly '%s' (%s), trying harder...", aszRes, aszBasePath));
      ret = _TryPathAllDirs(resPath, basePath);
    }
  }

  if (!ret.empty() && ret.size() >= 2 && (ret[0] == '/' || ret[1] == ':')) {
    const cString origAbsolutPath = ret;
    // this is an absolute directory...
    TRACE_FFP_WARN(niFmt("Found in an absolute path '%s' (%s), trying to find a relative path...", aszRes, aszBasePath));
    ret = _TryPathAllDirs(ret.Chars(),basePath);
    if (ret.empty() || (ret.size() >= 2 && (ret[0] == '/' || ret[1] == ':'))) {
      TRACE_FFP_WARN(niFmt("Couldn't find a relative path for '%s' (%s), you're going to have a bad time, move the file asap.", aszRes, aszBasePath));
      ret = origAbsolutPath;
    }
  }

  return ret;
}

cString __stdcall cLang::URLFindFilePath(const achar* aszRes, const achar* aszBasePath, const achar* aLoaderPrefix)
{
  if (niStringIsOK(aLoaderPrefix)) {
    cString loaderPrefix = aLoaderPrefix;
    return _URLFindFilePath(aszRes, aszBasePath, &loaderPrefix);
  }
  else {
    return _URLFindFilePath(aszRes, aszBasePath, NULL);
  }
}
