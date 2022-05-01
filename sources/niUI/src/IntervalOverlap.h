#ifndef __INTERVALOVERLAP_21960290_H__
#define __INTERVALOVERLAP_21960290_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include <niLang/STL/set.h>
#include <niLang/STL/sort.h>
namespace ni {

template <typename T>
struct sEdgeKey : public sVec2<T>
{
  sEdgeKey(T a = -1, T b = -1) {
    Set(a,b);
  }
  sEdgeKey(const sEdgeKey& aKey) {
    this->x = aKey.x;
    this->y = aKey.y;
  }
  void Set(T a, T b) {
    if (a <= b) {
      // a is minimum
      this->x = a;
      this->y = b;
    }
    else {
      // b is minimum
      this->x = b;
      this->y = a;
    }
  }

  bool operator < (const sEdgeKey& aKey) const {
    if (this->y < aKey.y)
      return true;
    if (this->y > aKey.y)
      return false;
    return this->x < aKey.x;
  }
};

typedef sEdgeKey<tI32>  sEdgeKeyl;

template <class T>
class IntersectingIntervals
{
 public:
  typedef sVec2<T>   tInterval;
  typedef IntersectingIntervals<T>  tSelf;
  typedef astl::set<sEdgeKeyl>    tOverlapSet;

  // Intervals are of the form [b,e] where b is the beginning value and e
  // is the ending value with b < e.  The end points are stored as a
  // Vec2.

  // construction and destruction
  IntersectingIntervals () : mDirty(0) {
  }
  ~IntersectingIntervals () {
  }

  // Reserve memory for the specified number of intervals.
  void __stdcall ReserveIntervals(tU32 anCount) {
    mvIntervals.reserve(anCount);
  }

  // Get the number of intervals.
  tU32 __stdcall GetNumIntervals() const {
    return mvIntervals.size();
  }

  // Insert an interval before the specified index.
  void __stdcall InsertInterval(tU32 i, const tInterval& aI) {
    mvIntervals.insert(mvIntervals.begin()+i,aI);
    mDirty |= DIRTY_INITIALIZE;
  }

  // Add an interval.
  void __stdcall AddInterval(const tInterval& aI) {
    mvIntervals.push_back(aI);
    mDirty |= DIRTY_INITIALIZE;
  }

  // Remove an interval.
  void __stdcall RemoveInterval(tU32 i) {
    mvIntervals.erase(mvIntervals.begin()+i);
    mDirty |= DIRTY_INITIALIZE;
  }

  // After the system is initialized, you can move the intervals using this
  // function.  It is not enough to modify the input array of intervals
  // since the end point values stored internally by this class must also
  // change.  You can also retrieve the current interval information.
  void SetInterval(tU32 i, const tInterval& aI) {
    mvIntervals[i] = aI;
    DirtyUpdateInitialize();
    mvEndPoints[mvLookup[2*i]].Value = aI.x;
    mvEndPoints[mvLookup[2*i+1]].Value = aI.y;
    mDirty |= DIRTY_OVERLAP;
  }

  const tInterval& GetInterval(tU32 i) const {
    return mvIntervals[i];
  }

  // If (i,j) is in the overlap set, then interval i and interval j are
  // overlapping.  The indices are those for the the input array.  The
  // set elements (i,j) are stored so that i < j.
  const tOverlapSet& GetOverlap () const {
    niThis(IntersectingIntervals)->DirtyUpdateOverlap();
    return mOverlaps;
  }

 private:
  void DirtyUpdateInitialize() {
    if (niFlagIs(mDirty,DIRTY_INITIALIZE)) {
      _Initialize();
      niFlagOff(mDirty,DIRTY_INITIALIZE);
    }
  }
  void DirtyUpdateOverlap() {
    DirtyUpdateInitialize();
    if (niFlagIs(mDirty,DIRTY_OVERLAP)) {
      _Update();
      niFlagOff(mDirty,DIRTY_OVERLAP);
    }
  }

  // This function is called by the constructor and does the sort-and-sweep
  // to initialize the update system.  However, if you add or remove items
  // from the array of intervals after the constructor call, you will need
  // to call this function once before you start the multiple calls of the
  // update function.
  void _Initialize () {
    //niPrintln(_A("--- II OVERLAP INIT ---\n"));
    // get the interval end points
    tI32 iISize = (tI32)mvIntervals.size();
    tI32 iESize = 2*iISize;
    mvEndPoints.resize(iESize);
    tI32 i, j;
    for (i = 0, j = 0; i < iISize; i++)
    {
      sEndPoint& rkEMin = mvEndPoints[j++];
      rkEMin.Type = 0;
      rkEMin.Value = mvIntervals[i][0];
      rkEMin.Index = i;

      sEndPoint& rkEMax = mvEndPoints[j++];
      rkEMax.Type = 1;
      rkEMax.Value = mvIntervals[i][1];
      rkEMax.Index = i;
    }

    // sort the interval end points
    astl::sort(mvEndPoints.begin(),mvEndPoints.end());

    // create the interval-to-endpoint lookup table
    mvLookup.resize(iESize);
    for (j = 0; j < iESize; j++)
    {
      sEndPoint& rkE = mvEndPoints[j];
      mvLookup[2*rkE.Index + rkE.Type] = j;
    }

    // active set of intervals (stored by index in array)
    astl::set<tI32> kActive;

    // set of overlapping intervals (stored by pairs of indices in array)
    mOverlaps.clear();

    // sweep through the end points to determine overlapping intervals
    for (i = 0; i < iESize; i++)
    {
      sEndPoint& rkEnd = mvEndPoints[i];
      tU32 iIndex = rkEnd.Index;
      if ( rkEnd.Type == 0 )  // an interval 'begin' value
      {
        astl::set<tI32>::iterator pkIter = kActive.begin();
        for (/**/; pkIter != kActive.end(); pkIter++)
        {
          tU32 iAIndex = *pkIter;
          mOverlaps.insert(sEdgeKeyl(iAIndex,iIndex));
        }
        kActive.insert(iIndex);
      }
      else  // an interval 'end' value
      {
        kActive.erase(iIndex);
      }
    }
  }

  // When you are finished moving intervals, call this function to determine
  // the overlapping intervals.  An incremental update is applied to
  // determine the new set of overlapping intervals.
  void _Update () {
    //niPrintln(_A("--- II OVERLAP UPDATE ---\n"));
    // Apply an insertion sort.  Under the assumption that the intervals
    // have not changed much since the last call, the end points are nearly
    // sorted.  The insertion sort should be very fast in this case.
    tI32 iESize = (tI32)mvEndPoints.size();
    for (tI32 j = 1; j < iESize; j++)
    {
      sEndPoint kKey = mvEndPoints[j];
      tI32 i = j - 1;
      while ( i >= 0 && kKey < mvEndPoints[i] )
      {
        sEndPoint kE0 = mvEndPoints[i];
        sEndPoint kE1 = mvEndPoints[i+1];

        // update the overlap status
        if ( kE0.Type == 0 )
        {
          if ( kE1.Type == 1 )
          {
            // The 'b' of interval E0.Index was smaller than the 'e'
            // of interval E1.Index, and the intervals *might have
            // been* overlapping.  Now 'b' and 'e' are swapped, and
            // the intervals cannot overlap.  Remove the pair from
            // the overlap set.  The removal operation needs to find
            // the pair and erase it if it exists.  Finding the pair
            // is the expensive part of the operation, so there is no
            // real time savings in testing for existence first, then
            // deleting if it does.
            mOverlaps.erase(sEdgeKeyl(kE0.Index,kE1.Index));
          }
        }
        else
        {
          if ( kE1.Type == 0 )
          {
            // The 'b' of interval E0.index was larger than the 'e'
            // of interval E1.index, and the intervals were not
            // overlapping.  Now 'b' and 'e' are swapped, and the
            // intervals *might be* overlapping.  We need to determine
            // if this is so and insert only if they do overlap.
            const tInterval& rkI0 = mvIntervals[kE0.Index];
            const tInterval& rkI1 = mvIntervals[kE1.Index];
            if ( rkI0[1] >= rkI1[0] ) {
              mOverlaps.insert(sEdgeKeyl(kE0.Index,kE1.Index));
            }
          }
        }

        // reorder the items to maintain the sorted list
        mvEndPoints[i] = kE1;
        mvEndPoints[i+1] = kE0;
        mvLookup[(2*kE1.Index) + kE1.Type] = i;
        mvLookup[(2*kE0.Index) + kE0.Type] = i+1;
        --i;
      }
      mvEndPoints[i+1] = kKey;
      mvLookup[(2*kKey.Index) + kKey.Type] = i+1;
    }
  }

  struct sEndPoint
  {
    T Value;
    tU32 Type;  // '0' if interval min, '1' if interval max
    tU32 Index;  // index of interval containing this end point

    // support for sorting of end points
    bool operator< (const sEndPoint& rkEP) const
    {
      if ( Value < rkEP.Value )
        return true;
      if ( Value > rkEP.Value )
        return false;
      return Type < rkEP.Type;
    }
  };

  enum {
    DIRTY_INITIALIZE = niBit(0),
    DIRTY_OVERLAP = niBit(1)
  };

  tU32 mDirty;
  astl::vector<tInterval> mvIntervals;
  astl::vector<sEndPoint> mvEndPoints;
  astl::set<sEdgeKeyl> mOverlaps;

  // The intervals are indexed 0 <= i < n.  The end point array has 2*n
  // entries.  The original 2*n interval values are ordered as b[0], e[0],
  // b[1], e[1], ..., b[n-1], e[n-1].  When the end point array is sorted,
  // the mapping between interval values and end points is lost.  In order
  // to modify interval values that are stored in the end point array, we
  // need to maintain the mapping.  This is done by the following lookup
  // table of 2*n entries.  The value mvLookup[2*i] is the index of b[i]
  // in the end point array.  The value mvLookup[2*i+1] is the index of
  // e[i] in the end point array.
  astl::vector<tU32> mvLookup;
};

typedef IntersectingIntervals<float> IntersectingIntervalsf;
typedef IntersectingIntervals<double> IntersectingIntervalsd;

}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __INTERVALOVERLAP_21960290_H__
