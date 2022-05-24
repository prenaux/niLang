#pragma once
#ifndef __MATHPROB_H_3CBCF22B_7990_6C46_BE94_8F9A00A061A9__
#define __MATHPROB_H_3CBCF22B_7990_6C46_BE94_8F9A00A061A9__

#include "../Types.h"
#include "../Utils/CollectionImpl.h"
#include "../Utils/Random.h"

namespace ni {

inline tF64 ProbSum(const tF64* apProbs, const tSize aNumProbs) {
  if (!apProbs || aNumProbs < 1)
    return 0.0;
  tF64 sum = 0.0;
  niLoop(i,aNumProbs) {
    sum += apProbs[i];
  }
  return sum;
}

inline tBool ProbNormalize(tF64* apProbs, const tSize aNumProbs) {
  const tF64 sum = ProbSum(apProbs, aNumProbs);
  if (sum <= 0.0)
    return eFalse;
  const tF64 m = 1.0 / sum;
  niLoop(i,aNumProbs) {
    apProbs[i] *= m;
  }
  return eTrue;
}

// p[n], q[n], a[n*2]
inline void ProbSampleBuildAliasMethodArrays(const tF64* p, const tSize n, tF64* q, tU32* a)
{
  niAssert(p && n >= 1);

  // start of H and L vector
  tU32* LL = a+n, *HH = a+2*n-1;
  // end of H and L vector
  tU32* L = LL, *H = HH;

  // set up alias table, initialize q with n*p0,...n*p_n-1
  niLoop(i,n) {
    q[i] = p[i]*n;
  }

  // initialize a with indices
  niLoop(i,n) {
    a[i] = i;
  }

  // set up H and L
  niLoop(i,n) {
    if( q[i] >= 1.)
      *H-- = i;
    else
      *L++ = i;
  }

  while (L != LL && H != HH) {
    tI32 j = *(L-1);
    tI32 k = *(H+1);
    a[j] = k;
    q[k] += q[j] - 1;
    L--; // remove j from L
    if (q[k] < 1.) {
      *L++ = k; // add k to L
      ++H; // remove k
    }
  }
}

// r[nres], q[n], a[n*2]
template <typename TRNG>
inline tBool ProbSampleAliasMethod(
  tU32* r, const tSize nres,
  const tF64* q, const tU32* a, const tSize n,
  TRNG&& rngGetNormalizedFloat)
{
  niAssert(r && nres >= 1);
  niAssert(q && a && n >= 1);

  const tF64 ndbl = (tF64)n;
  niLoop(i,nres) {
    tF64 rU = rngGetNormalizedFloat() * ndbl;
    const tU32 k = (tU32)(rU);
    rU -= k; // rU becomes rU-[rU]
    if (rU < q[k])
      r[i] = k;
    else
      r[i] = a[k];
  }

  return eTrue;
}

template <typename TRNG>
Ptr<tU32CVec> ProbSampleAliasMethod(
  const tSize nres,
  const ni::tF64CVec* apProbs,
  TRNG&& rngGetNormalizedFloat)
{
  niCheck(apProbs && !apProbs->empty(), NULL);

  // Create the alias arrays used for the prng generation afterward.
  ni::Ptr<ni::tF64CVec> amq = ni::tF64CVec::Create();
  amq->resize(apProbs->size());
  ni::Ptr<ni::tU32CVec> ama = ni::tU32CVec::Create();
  ama->resize(apProbs->size()*2);
  ProbSampleBuildAliasMethodArrays(
    apProbs->data(), apProbs->size(), amq->data(), ama->data());

  ni::Ptr<ni::tU32CVec> r = ni::tU32CVec::Create();
  r->resize(nres);
  ProbSampleAliasMethod(
    r->data(), nres,
    amq->data(), ama->data(), apProbs->size(),
    rngGetNormalizedFloat);
  return r;
}

Ptr<tU32CVec> ProbSampleAliasMethod(
  const tSize nres,
  const ni::tF64CVec* apProbs,
  int4* aPRNG = ni_prng_global())
{
  niCheck(apProbs && !apProbs->empty(), NULL);
  return ProbSampleAliasMethod(
    nres, apProbs,
    [&]() -> ni::tF64 { return ni_prng_next_f64(aPRNG); });
}

template <typename... Ts>
ni::Ptr<ni::tF64CVec> ProbArray(Ts... values) {
  ni::Ptr<ni::tF64CVec> probs = ni::tF64CVec::Create();
  probs->insert(probs->end(), {values...});
  ProbNormalize(probs->data(), probs->size());
  return probs;
}

}
#endif // __MATHPROB_H_3CBCF22B_7990_6C46_BE94_8F9A00A061A9__
