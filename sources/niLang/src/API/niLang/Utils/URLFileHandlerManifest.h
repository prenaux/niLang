#pragma once
#ifndef __URLHANDLERMANIFEST_H_717267A8_D672_40A4_ADBB_CD27296AA55C__
#define __URLHANDLERMANIFEST_H_717267A8_D672_40A4_ADBB_CD27296AA55C__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "UnknownImpl.h"
#include "../IFile.h"

namespace ni {

#define TRACE_URLHANDLER_MANIFEST(X) //niDebugFmt(X)

struct URLFileHandler_Manifest : public ImplRC<iURLFileHandler> {
 private:
  tBool mbManifsetInitialized;
  typedef astl::map<cString,cString> tManifestMap;
  tManifestMap mmapFilesManifest;

 public:
  URLFileHandler_Manifest() : mbManifsetInitialized(eFalse) {
  }

  ~URLFileHandler_Manifest() {
  }

  void LoadManifest(const char* aManifestPath) {
    Ptr<iFile> fpManifest = this->URLOpen(aManifestPath);
    if (fpManifest.IsOK()) {
      TRACE_URLHANDLER_MANIFEST(("... URLHANDLER MANIFEST %p, FOUND: %d, %d",
                                 (tIntPtr)this, fpManifest->GetSize(), fpManifest->Tell()));

      do {
        cString fileName = fpManifest->ReadStringLine().After("Name: ");
        cString keyName = fpManifest->ReadStringLine().After("Key: ");
        if (fpManifest->GetPartialRead())
          break;

        StrMakeStdPath(fileName.data());
        if (StrFindProtocol(fileName.Chars())) {
          // Remove the protocol if its in the filename
          fileName = StringURLGetPath(fileName.Chars());
        }

        TRACE_URLHANDLER_MANIFEST(("... URLHANDLER MANIFEST %p, '%s' = %s", (tIntPtr)this, fileName, keyName));

        // add to the manifest
        mmapFilesManifest[fileName] = keyName;

        // skip the empty line
        fpManifest->ReadStringLine();
      } while (1);
    }
  }

  cString GetPathFromURL(const achar* aURL) const {
    if (!mbManifsetInitialized) {
      niThis(URLFileHandler_Manifest)->mbManifsetInitialized = eTrue;
      niThis(URLFileHandler_Manifest)->LoadManifest("MANIFEST/Files.mf");
    }

    cString path = StringURLGetPath(aURL);
    if (!mmapFilesManifest.empty()) {
      StrMakeStdPath(path.data());
      tManifestMap::const_iterator it = mmapFilesManifest.find(path);
      if (it != mmapFilesManifest.end()) {
        TRACE_URLHANDLER_MANIFEST(("... URLHANDLER MANIFEST %p, TRANSLATED '%s' -> '%s'", (tIntPtr)this, aURL, it->second));
        path = it->second;
      }
      else {
        TRACE_URLHANDLER_MANIFEST(("... URLHANDLER MANIFEST %p, COULDNT FIND '%s'", (tIntPtr)this, aURL));
      }
    }
    else {
      TRACE_URLHANDLER_MANIFEST(("... URLHANDLER MANIFEST %p, NOT TRANSLATED '%s'", (tIntPtr)this, aURL));
    }

    return path;
  }
};

}
#endif // __URLHANDLERMANIFEST_H_717267A8_D672_40A4_ADBB_CD27296AA55C__
