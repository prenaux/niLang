// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "API/niLang/ILang.h"
#include "API/niLang/Utils/UnknownImpl.h"
#include "API/niLang/STL/utils.h"
#include "Lang.h"
#include "API/niLang/IDeviceResource.h"
#include "API/niLang/Utils/Trace.h"

using namespace ni;

niDeclareModuleTrace_(niLang,TraceDeviceResourceManager);
#define DRM_TRACE(FMT) niModuleTrace_(niLang,TraceDeviceResourceManager,FMT);

class cDeviceResourceManager : public cIUnknownImpl<iDeviceResourceManager>
{
  niBeginClass(cDeviceResourceManager);

 public:
  //! Constructor.
  cDeviceResourceManager(iHString* ahspType)
  {
    ZeroMembers();
    mhspType = ahspType;
  }

  //! Destructor.
  ~cDeviceResourceManager()
  {
    Invalidate();
    DRM_TRACE(("DRM[(%p)%s]: Destructed", (tIntPtr)this, mhspType));
  }

  //! Zeros all the class members.
  void ZeroMembers()
  {
  }

  //! Sanity check.
  tBool __stdcall IsOK() const
  {
    niClassIsOK(cDeviceResourceManager);
    return eTrue;
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate()
  {
    Clear();
  }

  ///////////////////////////////////////////////
  iHString* __stdcall GetType() const
  {
    __sync_lock();
    return mhspType;
  }

  ///////////////////////////////////////////////
  void __stdcall Clear()
  {
    __sync_lock();
    if (!mmapResources.empty())
    {
      tResHMap hmap = mmapResources;
      for (tResHMapIt itM = hmap.begin(); itM != hmap.end(); ++itM) {
        niWarning(niFmt(_A("Resource type '%s': '%s' not released, invalidated by the manager."), mhspType, itM->first));
        itM->second->Invalidate();
      }
      mmapResources.clear();
    }
    if (!mlstResources.empty())
    {
      niWarning(niFmt(_A("Resource type '%s': %d unnamed resources not released, invalidated by the manager."), mhspType, mlstResources.size()));
      tResLst lst = mlstResources;
      for (tResLstIt itL = lst.begin(); itL != lst.end(); ++itL)
        (*itL)->Invalidate();
      mlstResources.clear();
    }
    DRM_TRACE(("DRM[(%p)%s]: Cleared", (tIntPtr)this, mhspType));
  }

  ///////////////////////////////////////////////
  tU32 __stdcall GetSize() const
  {
    __sync_lock();
    return (tU32)(mmapResources.size()+mlstResources.size());
  }

  ///////////////////////////////////////////////
  iDeviceResource* __stdcall GetFromName(iHString* ahspName) const
  {
    __sync_lock();
    if (HStringIsEmpty(const_cast<iHString*>(ahspName)))
      return NULL;
    tResHMapCIt it = mmapResources.find(ahspName);
    if (it == mmapResources.end())
      return NULL;
    return it->second;
  }

  ///////////////////////////////////////////////
  iDeviceResource* __stdcall GetFromIndex(tU32 anIndex) const
  {
    __sync_lock();
    if (anIndex >= GetSize()) return NULL;
    tU32 nCount = 0;
    if (anIndex < mmapResources.size()) {
      for (tResHMapCIt mit = mmapResources.begin(); mit != mmapResources.end(); ++mit, ++nCount) {
        if (nCount == anIndex) {
          return mit->second;
        }
      }
      niAssertUnreachable("Unreachable code.");
    }
    else {
      nCount = (tU32)mmapResources.size();
    }
    for (tResLstCIt lit = mlstResources.begin(); lit != mlstResources.end(); ++lit, ++nCount) {
      if (nCount == anIndex) {
        return *lit;
      }
    }
    niAssertUnreachable("Unreachable code.");
    return NULL;
  }

  ///////////////////////////////////////////////
  tBool __stdcall Register(iDeviceResource* apRes)
  {
    __sync_lock();
    if (HStringIsNotEmpty(apRes->GetDeviceResourceName()))
    {
      if (!niIsOK(apRes)) {
        niError(niFmt(_A("Can't register invalid resource '%s', named '%s'."),
                      niHStr(mhspType),
                      niHStr(apRes->GetDeviceResourceName())));
        return eFalse;
      }

      DRM_TRACE(("DRM[(%p)%s]: REGISTER named resource (%p) '%s'",
                 (void*)this,mhspType,(void*)apRes,apRes->GetDeviceResourceName()));
      tResHMapIt it = mmapResources.find(apRes->GetDeviceResourceName());
      if (it == mmapResources.end()) {
        astl::upsert(mmapResources, apRes->GetDeviceResourceName(), apRes);
        return eTrue;
      }
      else if (it->second == apRes) {
        niWarning(niFmt(_A("Resource type '%s': '%s' overrided itself, skipped."), mhspType, it->first));
      }
      else {
        niWarning(niFmt(_A("Resource type '%s': '%s' overrided."), mhspType, it->first));
        it->second = apRes;
      }
    }
    else
    {
      if (!niIsOK(apRes)) {
        niError(niFmt(_A("Can't register invalid unnamed '%s' resource."),
                      niHStr(mhspType)));
        return eFalse;
      }
      DRM_TRACE(("DRM[(%p)%s]: REGISTER unnamed resource (%p)",
                 (void*)this,mhspType,(void*)apRes));
      mlstResources.push_back(apRes);
    }
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall Unregister(iDeviceResource* apRes)
  {
    __sync_lock();
    if (HStringIsNotEmpty(apRes->GetDeviceResourceName()))
    {
      DRM_TRACE(("DRM[(%p)%s]: Unregister named resource (%p) '%s'",
                 (void*)this,mhspType,(void*)apRes,apRes->GetDeviceResourceName()));
      tResHMapIt it = mmapResources.find(apRes->GetDeviceResourceName());
      if (it == mmapResources.end())
        return eFalse;
      if (it->second != apRes) {
        niWarning(niFmt(
            "Resource type '%s': '%s' not unregistered, expected '%p' but '%p' is registered there.",
            mhspType, it->first,
            (tIntPtr)apRes, (tIntPtr)it->second));
        return eFalse;
      }
      mmapResources.erase(it);
    }
    else
    {
      DRM_TRACE(("DRM[(%p)%s]: Unregister unnamed resource (%p)",
                 (void*)this,mhspType,(void*)apRes));
      if (!astl::find_erase(mlstResources,apRes))
        return eFalse;
    }
    return eTrue;
  }

 private:
  typedef astl::hstring_hash_map<iDeviceResource*> tResHMap;
  typedef tResHMap::iterator                       tResHMapIt;
  typedef tResHMap::const_iterator                 tResHMapCIt;
  typedef astl::list<iDeviceResource*>             tResLst;
  typedef tResLst::iterator                        tResLstIt;
  typedef tResLst::const_iterator                  tResLstCIt;

  tHStringPtr                                      mhspType;
  tResHMap                                         mmapResources;
  tResLst                                          mlstResources;

  __sync_mutex();

  niEndClass(cDeviceResourceManager);
};

///////////////////////////////////////////////
//! Create a new device resource manager.
iDeviceResourceManager* __stdcall cLang::CreateDeviceResourceManager(const achar* aszType)
{
  return niNew cDeviceResourceManager(_H(aszType));
}
