#ifndef __SCOPEDRELEASE_H_1ED67A2D_4B29_464E_A519_F9D1A8272033__
#define __SCOPEDRELEASE_H_1ED67A2D_4B29_464E_A519_F9D1A8272033__

template <class T> class ScopedRelease
{
public:
  T *mVar;
  ScopedRelease(T * aVar)
  {
    mVar = aVar;
  }
  ~ScopedRelease()
  {
    if (mVar != 0)
      mVar->Release();
  }
};

#endif // __SCOPEDRELEASE_H_1ED67A2D_4B29_464E_A519_F9D1A8272033__
