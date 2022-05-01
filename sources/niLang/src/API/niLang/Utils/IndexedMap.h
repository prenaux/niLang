#ifndef __INDEXEDMAP_H_241F989B_9F16_4D4E_8359_04487050DA40__
#define __INDEXEDMAP_H_241F989B_9F16_4D4E_8359_04487050DA40__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../STL/utils.h"
#include "../STL/map.h"
#include "../STL/vector.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

ASTL_RAW_ALLOCATOR_IMPL(indexed_map);

template <typename KEYT, typename ITEMT, ASTL_TMPL_PARAM(KEYCMP, eastl::less<KEYT>) >
class IndexedMap
{
  typedef KEYT   tKey;
  typedef ITEMT  tItem;

 public:
  typedef astl::map<KEYT, tU32, KEYCMP, ASTL_PAIR_ALLOCATOR(const KEYT,tU32,indexed_map)> tKeyIdxMap;
  typedef niTypename tKeyIdxMap::iterator        iterator;
  typedef niTypename tKeyIdxMap::const_iterator  const_iterator;

 private:
  struct sItem {
    iterator itMap;
    tItem    Item;
  };
  typedef astl::vector<sItem>                 tItemVec;

  void _UpdateIndices() {
    if (mnDirtyIndex == eInvalidHandle)
      return;
    for (tU32 i = mnDirtyIndex; i < mVec.size(); ++i) {
      mVec[i].itMap->second = i;
    }
    mnDirtyIndex = eInvalidHandle;
  }

 public:
  IndexedMap() {
    mnDirtyIndex = eInvalidHandle;
  }
  ~IndexedMap() {
    Clear();
  }

  //! Set value of an item. If the item don't exists it's added.
  //! \param  aKey is the name of the item to add.
  //! \param  aItem is the value of the item to add.
  //! \return The index of the added item, return eInvalidHandle if set failed.
  tU32 __stdcall Set(const tKey& aKey, const tItem& aItem) {
    iterator itMap = mMap.find(aKey);
    if (itMap != mMap.end()) {
      sItem* el = &mVec[itMap->second];
      el->Item = aItem;
      _UpdateIndices();
      return itMap->second;
    }
    sItem* el = &astl::push_back(mVec);
    tU32 idx = (tU32)(mVec.size()-1);
    el->Item = aItem;
    el->itMap = astl::upsert(mMap,aKey,idx);
    return idx;
  }

  //! Reserve memory for N items.
  void __stdcall Reserve(tSize anCount) {
    mVec.reserve(anCount);
  }
  //! Get whether the container is empty.
  tBool __stdcall IsEmpty() const {
    return mMap.empty();
  }

  //! Get the size of the container
  tSize __stdcall GetSize() const {
    return mVec.size();
  }

  //! Get whether an item with the specified key is in the container.
  tBool __stdcall HasKey(const tKey& aKey) const {
    return mMap.find(aKey) != mMap.end();
  }
  //! Get whether an item with the specified index is in the container.
  tBool __stdcall HasIndex(tU32 anIndex) const {
    if (anIndex >= mVec.size()) return eFalse;
    if (mVec[anIndex].itMap == mMap.end()) return eFalse;
    return eTrue;
  }

  //! Get the key of the item with the given index.
  //! \return NULL if the index is invalid.
  const tKey* __stdcall GetKeyPtr(tU32 anIndex) const {
    niCheckSilent(anIndex < mVec.size(),NULL);
    niCheckSilent(mVec[anIndex].itMap != mMap.end(),NULL);
    return &mVec[anIndex].itMap->first;
  }

  //! Get the key of the item with the given index.
  const tKey& __stdcall GetKey(tU32 anIndex) const {
    niAssert(anIndex < mVec.size());
    niAssert(mVec[anIndex].itMap != mMap.end());
    return mVec[anIndex].itMap->first;
  }

  //! Get the index of the item with the given name.
  //! \return eInvalidHandle if the item is not in the MapIndex.
  tU32 __stdcall GetKeyIndex(const tKey& aKey) const {
    const_iterator itMap = mMap.find(aKey);
    niThis(IndexedMap)->_UpdateIndices();
    return (itMap == mMap.end()) ? eInvalidHandle : itMap->second;
  }

  //! Get a reference to the item with the given index.
  //! \remark Assert if the index is invalid.
  tItem& __stdcall GetItem(tU32 nIndex) {
    niAssert(nIndex < mVec.size());
    return const_cast<tItem&>(mVec[nIndex].Item);
  }
  //! Get a reference to the item from the given key.
  //! \remark Assert if the index is invalid.
  tItem& __stdcall GetItem(const tKey& aKey) {
    return GetItem(GetKeyIndex(aKey));
  }
  //! Get a pointer to the item with the given index.
  //! \remark return NULL if the index is invalid.
  tItem* __stdcall GetItemPtr(tU32 nIndex) {
    if (nIndex >= mVec.size()) return NULL;
    if (mVec[nIndex].itMap == mMap.end()) return NULL;
    return const_cast<tItem*>(&mVec[nIndex].Item);
  }
  //! Get a pointer to the item with the given key.
  //! \remark return NULL if the key is invalid.
  tItem* __stdcall GetItemPtr(const tKey& aKey) {
    return GetItemPtr(GetKeyIndex(aKey));
  }

  __forceinline const tItem& __stdcall GetItem(tU32 anIndex) const {
    return niThis(IndexedMap)->GetItem(anIndex);
  }
  __forceinline const tItem& __stdcall GetItem(const tKey& aKey) const {
    return niThis(IndexedMap)->GetItem(aKey);
  }
  __forceinline const tItem* __stdcall GetItemPtr(tU32 anIndex) const {
    return niThis(IndexedMap)->GetItemPtr(anIndex);
  }
  __forceinline const tItem* __stdcall GetItemPtr(const tKey& aKey) const {
    return niThis(IndexedMap)->GetItemPtr(aKey);
  }

  //! Remove the item with the given index.
  //! \return eFalse if the index is invalid, else eTrue.
  tBool __stdcall EraseIndex(tU32 nIndex) {
    if (nIndex >= mVec.size())
      return eFalse;
    iterator it = mVec[nIndex].itMap;
    if (it != mMap.end()) {
      mMap.erase(it);
    }
    mVec.erase(mVec.begin()+nIndex);
    mnDirtyIndex = ni::Min(mnDirtyIndex,nIndex);
    return eTrue;
  }

  //! Remove the item with the given name.
  //! \return eFalse if the item don't exists, else eTrue.
  tBool __stdcall EraseKey(const tKey& aKey) {
    return EraseIndex(GetKeyIndex(aKey));
  }

  //! Remove all items in the MapIndex.
  void __stdcall Clear() {
    mMap.clear();
    mVec.clear();
    mnDirtyIndex = eInvalidHandle;
  }

  //! Return a reference of the item with the given index.
  tItem& __stdcall operator [] (tU32 nIndex) { return GetItem(nIndex); }
  //! Return a constant reference of the item with the given index.
  const tItem& __stdcall operator [] (tU32 nIndex) const { return GetItem(nIndex); }
  //! Return a reference of the item with the given name.
  tItem& __stdcall operator [] (const tKey& aKey) { return GetItem(GetKeyIndex(aKey)); }
  //! Return a reference of the item with the given name.
  const tItem& __stdcall operator [] (const tKey& aKey) const { return GetItem(GetKeyIndex(aKey)); }

 private:
  tKeyIdxMap mMap;
  tItemVec   mVec;
  tU32       mnDirtyIndex;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}; // End of ni
#endif // __INDEXEDMAP_H_241F989B_9F16_4D4E_8359_04487050DA40__
