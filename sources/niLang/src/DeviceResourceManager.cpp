// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "API/niLang/ILang.h"
#include "API/niLang/Utils/UnknownImpl.h"
#include "API/niLang/STL/utils.h"
#include "API/niLang/STL/queue.h"
#include "Lang.h"
#include "API/niLang/IDeviceResource.h"
#include "API/niLang/Utils/Trace.h"

using namespace ni;

niDeclareModuleTrace_(niLang,TraceDeviceResourceManager);
#define DRM_TRACE(FMT) niModuleTrace_(niLang,TraceDeviceResourceManager,FMT);

class cDeviceResourceManager : public ImplRC<iDeviceResourceManager>
{
  niBeginClass(cDeviceResourceManager);

 public:
  niConstValue tU32 knFreeListSizeBeforeReuse = 64;

  ///////////////////////////////////////////////
  cDeviceResourceManager(iHString* ahspType) {
    mhspType = ahspType;
    mvResources.reserve(64);
  }

  ///////////////////////////////////////////////
  ~cDeviceResourceManager() {
    Invalidate();
    DRM_TRACE(("DRM[(%p)%s]: Destructed", (tIntPtr)this, mhspType));
  }

  ///////////////////////////////////////////////
  tBool __stdcall IsOK() const {
    niClassIsOK(cDeviceResourceManager);
    return eTrue;
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
    Clear();
  }

  ///////////////////////////////////////////////
  iHString* __stdcall GetType() const {
    return mhspType;
  }

  ///////////////////////////////////////////////
  void __stdcall Clear()
  {
    __sync_lock();
    tU32 numInvalidated = 0;
    astl::vector<iDeviceResource*> resourcesToInvalidate = mvResources;
    for (iDeviceResource* toInvalidate : resourcesToInvalidate) {
      if (toInvalidate) {
        toInvalidate->Invalidate();
        ++numInvalidated;
      }
    }
    if (numInvalidated) {
      niWarning(niFmt(_A("Resource type '%s': %d resources not released and invalidated by the manager."),
                      mhspType, numInvalidated));
    }
    mvResources.clear();
    DRM_TRACE(("DRM[(%p)%s]: Cleared", (tIntPtr)this, mhspType));
  }

  ///////////////////////////////////////////////
  tU32 __stdcall GetSize() const {
    __sync_lock();
    return (tU32)mvResources.size();
  }

  ///////////////////////////////////////////////
  iDeviceResource* __stdcall GetFromName(iHString* ahspName) const {
    __sync_lock();
    niLoop(i,mvResources.size()) {
      iDeviceResource* r = mvResources[i];
      if (r != nullptr  && r->GetDeviceResourceName() == ahspName)
        return r;
    }
    return nullptr;
  }

  ///////////////////////////////////////////////
  iDeviceResource* __stdcall GetFromIndex(tU32 anIndex) const {
    __sync_lock();
    if (anIndex >= mvResources.size())
      return nullptr;
    return mvResources[anIndex];
  }

  ///////////////////////////////////////////////
  tU32 __stdcall GetIndexFromName(iHString* ahspName) const {
    __sync_lock();
    niLoop(i,mvResources.size()) {
      iDeviceResource* r = mvResources[i];
      if (r != nullptr && r->GetDeviceResourceName() == ahspName)
        return i;
    }
    return eInvalidHandle;
  }

  ///////////////////////////////////////////////
  tU32 __stdcall GetIndexFromResource(iDeviceResource* apResource) const {
    __sync_lock();
    if (apResource) {
      niLoop(i,mvResources.size()) {
        iDeviceResource* r = mvResources[i];
        if (r == apResource)
          return i;
      }
    }
    return eInvalidHandle;
  }

  ///////////////////////////////////////////////
  tU32 __stdcall Register(iDeviceResource* apRes)
  {
    __sync_lock();

    tHStringPtr resName = apRes->GetDeviceResourceName();
    if (HStringIsNotEmpty(resName)) {
      if (!niIsOK(apRes)) {
        niError(niFmt(
          "Can't register invalid resource '%s' (%p), named '%s'.",
          mhspType,
          (tIntPtr)apRes,
          apRes->GetDeviceResourceName()));
        return eFalse;
      }
      DRM_TRACE(("DRM[(%p)%s]: REGISTER named resource (%p) '%s'",
                 (void*)this,mhspType,(void*)apRes,apRes->GetDeviceResourceName()));

      const tU32 foundWithSameName = GetIndexFromName(resName);
      if (foundWithSameName != eInvalidHandle) {
        niWarning(niFmt(_A("Resource type '%s': name '%s' already found at index '%d'."), mhspType, resName, foundWithSameName));
      }
    }
    else
    {
      if (!niIsOK(apRes)) {
        niError(niFmt(
          "Can't register invalid unnamed resource '%s' (%p).",
          mhspType, (tIntPtr)apRes));
        return eFalse;
      }
      DRM_TRACE(("DRM[(%p)%s]: REGISTER unnamed resource (%p)",
                 (void*)this,mhspType,(tIntPtr)apRes));
    }

    tU32 newIndex = eInvalidHandle;
    if (mFreeList.size() >= knFreeListSizeBeforeReuse) {
      newIndex = mFreeList.front();
      mFreeList.pop();
      niPanicAssert(mvResources[newIndex] == nullptr);
      mvResources[newIndex] = apRes;
    }
    else {
      newIndex = (tU32)mvResources.size();
      mvResources.emplace_back(apRes);
    }

    return newIndex;
  }

  ///////////////////////////////////////////////
  tBool __stdcall Unregister(iDeviceResource* apRes) {
    __sync_lock();

    tHStringPtr resName = apRes->GetDeviceResourceName();
    const tU32 foundIndex = GetIndexFromResource(apRes);
    if (foundIndex == eInvalidHandle) {
      niWarning(niFmt(
        "Resource type '%s': not unregistered, couldnt find resource (%p) '%s'.",
        mhspType, (tIntPtr)apRes, resName));
      return eFalse;
    }

    if (HStringIsNotEmpty(resName)) {
      DRM_TRACE(("DRM[(%p)%s]: Unregistered named resource (%p) '%s' at index '%d'.",
                 (void*)this,mhspType,(void*)apRes,resName,foundIndex));
    }
    else {
      DRM_TRACE(("DRM[(%p)%s]: Unregistered unnamed resource (%p) at index '%d'.",
                 (void*)this,mhspType,(void*)apRes,foundIndex));
    }

    mFreeList.emplace(foundIndex);
    mvResources[foundIndex] = nullptr;
    return eTrue;
  }

 private:
  __sync_mutex();
  tHStringPtr mhspType;
  astl::vector<iDeviceResource*> mvResources;
  astl::queue<tU32> mFreeList;

  niEndClass(cDeviceResourceManager);
};

///////////////////////////////////////////////
//! Create a new device resource manager.
iDeviceResourceManager* __stdcall cLang::CreateDeviceResourceManager(const achar* aszType)
{
  return niNew cDeviceResourceManager(_H(aszType));
}
