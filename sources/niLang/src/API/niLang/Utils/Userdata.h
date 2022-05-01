#ifndef __USERDATA_51546720_H__
#define __USERDATA_51546720_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../IHString.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

// Utility userdata class.
template <typename BASE>
class cUserdata : public BASE
{
  typedef astl::hstring_hash_map<Ptr<iUnknown> > tHashMap;

 public:
  cUserdata () {}
  ~cUserdata  () {}

  //! Set a userdata.
  virtual tBool __stdcall SetUserdata(iHString* ahspID, iUnknown* apUserdata) {
    tHashMap::iterator it = mUserdata.find(ahspID);
    if (!niIsOK(apUserdata)) {
      if (it != mUserdata.end()) {
        mUserdata.erase(it);
      }
      return eFalse;
    }
    else {
      if (it != mUserdata.end()) {
        it->second = apUserdata;
      }
      else {
        astl::upsert(mUserdata,ahspID,apUserdata);
      }
      return eTrue;
    }
  }

  //! Get a userdata.
  virtual iUnknown* __stdcall GetUserdata(iHString* ahspID) const {
    tHashMap::const_iterator it = mUserdata.find(ahspID);
    if (it == mUserdata.end()) return NULL;
    return it->second;
  }

  //! Get the number of userdata.
  virtual tSize __stdcall GetNumUserdata() const {
    return mUserdata.size();
  }

  //! Get the name of the user data at the specified index.
  virtual iHString* __stdcall GetUserdataName(tU32 anIndex) const {
    tU32 nCount = 0;
    for (tHashMap::const_iterator it = mUserdata.begin(); it != mUserdata.end(); ++it, ++nCount) {
      if (nCount == anIndex) {
        return it->first;
      }
    }
    return NULL;
  }

  //! Get the userdata at the specified index.
  virtual iUnknown* __stdcall GetUserdataFromIndex(tU32 anIndex) const {
    tU32 nCount = 0;
    for (tHashMap::const_iterator it = mUserdata.begin(); it != mUserdata.end(); ++it, ++nCount) {
      if (nCount == anIndex) {
        return it->second;
      }
    }
    return NULL;
  }

 private:
  tHashMap  mUserdata;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __USERDATA_51546720_H__
