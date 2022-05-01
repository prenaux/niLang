#pragma once
#ifndef __GDRV_STATECACHE_H_B9D9E953_C301_4A7D_B476_F46DAEE2ECDF__
#define __GDRV_STATECACHE_H_B9D9E953_C301_4A7D_B476_F46DAEE2ECDF__

struct sStateCache {
  sStateCache(tU32 anNumStates, tU32 anAlwaysOn)
      : mnNumStates(anNumStates)
      , mnAlwaysOn(anAlwaysOn)
  {
    mCacheValues = niTMalloc(tIntPtr,mnNumStates);
    Reset();
  }
  ~sStateCache() {
    niFree(mCacheValues);
  }
  void Reset() {
    ni::MemSet((tPtr)mCacheValues,0,sizeof(tIntPtr)*mnNumStates);
  }

  // Usage:
  //
  // {
  //    tBool bShouldUpdate = eFalse;
  //    cache.ShouldUpdate(&bShouldUpdate,eCache_Foo,FooValue);
  //    cache.ShouldUpdate(&bShouldUpdate,eCache_Bar,BarValue);
  //    if (bShouldUpdate) { ... do update ... }
  // }
  //
  // The API forces a sequential check so that multiple checks per section are
  // always handled correctly.
  //
  // Allowing "if (ShouldUpdate(Foo) || ShouldUpdate(Bar))" would result in
  // Bar's value to not be updated correctly in the cache and then cause hard
  // to track bugs.
  //
  inline void ShouldUpdate(tBool* aShouldUpdate, tU32 aCache, tIntPtr v) {
    niAssert(aShouldUpdate != NULL);
    niAssert(aCache < mnNumStates);

    if (aCache < mnAlwaysOn) {
      if (v && mCacheValues[aCache] == v) {
        return;
      }
    }

    mCacheValues[aCache] = v;

    *aShouldUpdate = eTrue;
  }

 private:
  const tU32 mnNumStates;
  const tU32 mnAlwaysOn;
  tIntPtr*   mCacheValues;

  niClassStrictLocal(sStateCache);
};

#endif // __GDRV_STATECACHE_H_B9D9E953_C301_4A7D_B476_F46DAEE2ECDF__
