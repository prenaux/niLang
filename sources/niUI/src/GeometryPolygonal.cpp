// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "GeometryPolygonal.h"
#include <niLang/Utils/Hash.h>
#include <niLang/STL/set.h>
#include <niLang/STL/vector.h>
#include <niLang/Math/MathMatrix.h>
#include <niLang/Math/MathTriangle.h>
#include "API/niUI/Utils/Polygon.h"
#include "Graphics.h"
#include "GeometryPolygonal.h"
#include "GeometryQuadric.h"
#include <niLang/Utils/Trace.h>

niDeclareModuleTrace_(niUI,GeometryTrace);
#define TRACE_GEOMETRY(FMT) niModuleTrace_(niUI,GeometryTrace,FMT)

static const tU32 _knMaxGeomSubDiv = 256;


//  structures for the triangle order optimzation
struct tri_data
{
  bool added;
  float score;
  unsigned int verts[3];
};
struct vert_data
{
  float score;
  astl::set<unsigned int> remaining_tris;
};

/// return the number of cache_misses per triangle (lower is better)
float calculate_average_cache_miss_ratio(
    const astl::vector<unsigned int> &tri_indices,
    unsigned int cache_size = 14 );
/// reorder the triangles, trying to minimize number of cache_misses
/// per triangle (using Tom Forsyth's super simple method.)
bool optimize_vertex_cache_order(
    astl::vector<unsigned int> &tri_indices,
    unsigned int cache_size = 32 );

bool optimize_vertex_cache_order( astl::vector<unsigned int> &tri_indices, unsigned int cache_size )
{
  if( (tri_indices.size() < 3) || (tri_indices.size() % 3 != 0) || (cache_size < 4) )
    return false;
  unsigned int num_triangles = (tU32)tri_indices.size() / 3;
  unsigned int num_vertices = 0;
  for( unsigned int i = 0; i < num_triangles*3; ++i )
    if( tri_indices[i] > num_vertices )
      num_vertices = tri_indices[i];
  ++num_vertices;
  //  size of the optimization cache
  astl::vector<float> cache_score; cache_score.resize(cache_size + 3, 0.75);
  astl::vector<int> cache_idx; cache_idx.resize(cache_size + 3, -1);
  astl::vector<int> grow_cache_idx; grow_cache_idx.resize(cache_size + 3, -1);
  for( unsigned int i = 3; i < cache_size; ++i )
    cache_score[i] = ni::Pow( (cache_size - i) / (cache_size - 3.0f), 1.5f );
  for( unsigned int i = 0; i < 3; ++i )
    cache_score[cache_size+i] = 0.0;
  //  how many tris do we need to add?
  int tris_left = num_triangles;
  //  add all verts and tris to the lists
  astl::vector<tri_data> t; t.resize(num_triangles);
  astl::vector<vert_data> v; v.resize(num_vertices);
  for( unsigned int i = 0; i < num_vertices; ++i )
  {
    //  initialize this vert
    v[i].score = 0.0;
    v[i].remaining_tris.clear();
  }
  for( unsigned int i = 0; i < num_triangles; ++i )
  {
    //  set up this tri
    t[i].added = false;
    t[i].score = 0.0;
    t[i].verts[0] = tri_indices[i*3+0];
    t[i].verts[1] = tri_indices[i*3+1];
    t[i].verts[2] = tri_indices[i*3+2];
    //  and add this tri index to each of it's verts
    v[tri_indices[i*3+0]].remaining_tris.insert( i );
    v[tri_indices[i*3+1]].remaining_tris.insert( i );
    v[tri_indices[i*3+2]].remaining_tris.insert( i );
  }
  //  now initialize all the scores for the vertices
  for( unsigned int i = 0; i < num_vertices; ++i )
  {
    //  none of them are in the index yet, just use thier valence score
    v[i].score = (tF32)(ni::Pow((tF64)v[i].remaining_tris.size(), -0.5 ) * 2.0);
  }
  //  and the triangles' scores
  float best_score = 0.0;
  int best_idx = -1;
  for( unsigned int i = 0; i < num_triangles; ++i )
  {
    t[i].score = v[t[i].verts[0]].score + v[t[i].verts[1]].score + v[t[i].verts[2]].score;
    if( t[i].score > best_score )
    {
      best_score = t[i].score;
      best_idx = i;
    }
  }
  //  now keep adding triangles
  while( tris_left > 0 )
  {
    //  scan all tris if the best score is suspicious
    //if( best_score < 1.0 )
    if( best_score < 0.01 )
    {
      //astl::cerr << "low score looks suspicious...re-checking!" << astl::endl;
      best_score = 0.0;
      best_idx = -1;
      for( unsigned int i = 0; i < num_triangles; ++i )
        if( !t[i].added )
        {
          if( t[i].score > best_score )
          {
            best_score = t[i].score;
            best_idx = i;
          }
        }
    }
    if( best_idx < 0 )
    {
      tris_left = 0;
      //  need a warning...for now, an assert
      niAssertMsg(0,_A("Your stupid triangle re-organizing code isn't working, Jonathan!!"));
      return false;
    }
    else
    {
      //  add in this tri
      int a = t[best_idx].verts[0];
      int b = t[best_idx].verts[1];
      int c = t[best_idx].verts[2];
      //  put this tri back into circulation
      tri_indices[(num_triangles - tris_left)*3+0] = a;
      tri_indices[(num_triangles - tris_left)*3+1] = b;
      tri_indices[(num_triangles - tris_left)*3+2] = c;
      //  remove this tri from the association of each of the verts
      for( int i = 0; i < 3; ++i )
      {
        v[t[best_idx].verts[i]].remaining_tris.erase( best_idx );
      }
      t[best_idx].added = true;
      --tris_left;
      //astl::cerr << best_idx << "," << best_score << astl::endl;
      //  put these 3 verts at the top of the LRU list
      grow_cache_idx[0] = a;
      grow_cache_idx[1] = b;
      grow_cache_idx[2] = c;
      int idx = 3;
      for( unsigned int i = 0; i < cache_size; ++i )
      {
        //  clear out my growing cache
        grow_cache_idx[i+3] = -1;
        if( (cache_idx[i] != a) &&
            (cache_idx[i] != b) &&
            (cache_idx[i] != c) )
        {
          grow_cache_idx[idx++] = cache_idx[i];
        }
      }
      cache_idx = grow_cache_idx;

      //  update the weights
      //  (cache_size+3 because I want to update the triangles whose
      //  vertices fell out of the cache as well.)
      for( unsigned int i = 0; i < cache_size+3; ++i )
        if( cache_idx[i] >= 0 )
        {
          idx = cache_idx[i];
          //  store the old score
          float old_score = v[idx].score;
          //  get the new score
          float new_score = cache_score[i] + (tF32)(2.0 * ni::Pow((tF64)v[idx].remaining_tris.size(), -0.5 ));
          v[idx].score = new_score;
          //  now update all remaining linked triangles!
          for( astl::set<unsigned int>::iterator
                   it = v[idx].remaining_tris.begin();
               it != v[idx].remaining_tris.end();
               ++it )
          {
            t[*it].score += new_score - old_score;
          }
        }

      //  search for the next best tri
      best_score = 0.0;
      best_idx = -1;
      for( unsigned int i = 0; i < cache_size; ++i )
        if( cache_idx[i] >= 0 )
        {
          idx = cache_idx[i];
          //  is one of these triangles the best?
          for( astl::set<unsigned int>::iterator
                   it = v[idx].remaining_tris.begin();
               it != v[idx].remaining_tris.end();
               ++it )
          {
            if( t[*it].score > best_score )
            {
              best_score = t[*it].score;
              best_idx = *it;
            }
          }
        }
    }
  }
  return true;
}

float calculate_average_cache_miss_ratio(
    const astl::vector<unsigned int> &tri_indices,
    unsigned int cache_size )
{
  if( tri_indices.size() <= cache_size )
    return -1.0;
  astl::vector<int> cache_idx; cache_idx.resize(cache_size, -1);
  int cache_ptr = 0;
  int cache_misses = 0;
  for( unsigned int i = 0; i < tri_indices.size(); ++i )
  {
    //  is the newest vertex in the ring buffer (FIFO)?
    bool cache_hit = false;
    for( unsigned int j = 0; j < cache_size; ++j )
    {
      //  if this cache entry == the index, then cache_hit = true
      cache_hit |= ( cache_idx[j] == (int)tri_indices[i] );
    }
    if( !cache_hit )
    {
      //  store the newest vertex ID in the cache
      cache_idx[cache_ptr] = tri_indices[i];
      //  move to the next sopt (ring buffer FIFO)
      cache_ptr = (cache_ptr + 1) % cache_size;
      //  count'em
      ++cache_misses;
    }
  }
  //  cache misses per triangle
  return cache_misses / (tri_indices.size() / 3.0f);
}

/*
//  now, re-order the vertices to take advantage of this new triangle order
//  (this definitely helps!)
astl::map<GLuint,GLuint> old2new;
astl::vector<GLfloat> new_xyz; //  vertices
astl::vector<GLfloat> new_uv;  //  texture coordinates
astl::vector<GLfloat> new_ijk; //  normal vectors
GLuint counter = 0;
for( unsigned int i = 0; i < num_triangles*3; ++i )
{
//  has this point (vertex id) been found yet?
int vid = abc[i];
if( old2new.find( vid ) == old2new.end() )
{
//  not yet found!  add the original vertex info to the new vectors
new_xyz.push_back( xyz[vid*3+0] );
new_xyz.push_back( xyz[vid*3+1] );
new_xyz.push_back( xyz[vid*3+2] );
new_uv.push_back( uv[vid*2+0] );
new_uv.push_back( uv[vid*2+1] );
new_ijk.push_back( ijk[vid*3+0] );
new_ijk.push_back( ijk[vid*3+1] );
new_ijk.push_back( ijk[vid*3+2] );
//  add it to the map
old2new[vid] = counter;
//  and swap it!
abc[i] = counter;
//  increment my counter
++counter;
} else
{
//  yeah, have this one already
abc[i] = old2new[vid];
}
}
//  keep the updated data vectors
xyz.swap( new_xyz );
uv.swap( new_uv );
ijk.swap( new_ijk );

*/

float computeACMR(tIndex* inds, tU32 numIndices, const tU32 fifoSize) {
  astl::deque<tIndex>  fifo;
  tU32 numMiss = 0;

  // now 'render' the mesh
  niLoop(i,numIndices) {
    tU32 curIndex = inds[i];
    tBool found = eFalse;
    niLoopit(astl::deque<tIndex>::iterator,it,fifo) {
      if (*it == curIndex) {
        found = eTrue;
        break;
      }
    }
    if (!found) {
      // not found, trash the first vertex, push the current one and add one to the miss count
      while (fifo.size() > fifoSize) {
        fifo.pop_front();
      }
      fifo.push_back(curIndex);
      ++numMiss;
    }
  }

  return (float)((double)numMiss/(double)(numIndices/3));
}

// This algo seems to make caching worst than to start with ^_^ I suppose there's a bug, but
// might be aswell because input meshes dont have enougth shared vertices.
struct CacheOptimize
{
  struct VCacheData {
    tI32 cachePos;
    tF32 score;
    tU32 numActiveTris;
    tU32 numTris;
    astl::vector<tU32> tris;
    VCacheData() {
      numTris = 0;
      numActiveTris = 0;
      cachePos = -1;
      score = 0.0f;
    }
  };

  struct TCacheData {
    tBool added;
    tF32  score;
    tU32  verts[3];
    TCacheData() {
      added = eFalse;
      score = 0.0f;
      niLoop(i,3) verts[i] = 0;
    }
  };

  const tF32 _cacheDecayPower;
  const tF32 _lastTriScore;
  const tF32 _valenceBoostScale;
  const tF32 _valenceBoostPower;

  const tIndex* _src;
  const tU32    _srcCount;

  tU32    _baseVertexIndex;
  tU32    _numVertices;

  tF32    _averageTriPerVertex;

  astl::vector<tIndex>  _output;

  astl::vector<VCacheData>  _vcacheData;
  astl::vector<TCacheData>  _tcacheData;

  const tU32        _maxVertexCacheSize;
  astl::deque<tIndex>   _lru;

  ///////////////////////////////////////////////
  CacheOptimize(const tIndex* aSrc, const tU32 aSrcCount,
                tU32 aMaxVertexCacheSize = 32,
                tF32 aCacheDecayPower = 1.5f, tF32 aLastTriScore = 0.75f, tF32 aValBoostScale = 2.0f, tF32 aValBoostPower = 0.5f) :
      _cacheDecayPower(aCacheDecayPower),
      _lastTriScore(aLastTriScore),
      _valenceBoostScale(aValBoostScale),
      _valenceBoostPower(aValBoostPower),
      _src(aSrc),
      _srcCount(aSrcCount),
      _maxVertexCacheSize(aMaxVertexCacheSize+3)
  {
    _output.reserve(aSrcCount);
    _tcacheData.resize(aSrcCount/3);
  }

  ///////////////////////////////////////////////
  void Process() {
    InitFirstPass();
    // Compute the initial score of each vertex
    niLoop(i,_vcacheData.size()) {
      VCacheData& v = _vcacheData[i];
      v.score = ComputeVertexScore(&v);
    }
    // Compute the initial score of each triangle.
    niLoop(i,_tcacheData.size()) {
      TCacheData& t = _tcacheData[i];
      t.score = ComputeTriangleScore(&t);
    }
    // Find the initial best triangle
    tU32 numTriProcessed = 0;
    tU32 bestTri = FindAllBestScoreTriangle();
    niAssert(bestTri != eInvalidHandle);
    while (1) {
      ProcessTriangle(&_tcacheData[bestTri]);
      ++numTriProcessed;
      if (numTriProcessed >= _tcacheData.size())
        break;

      bestTri = FindLRUBestScoreTriangle();
      if (bestTri == eInvalidHandle) {
        bestTri = FindAllBestScoreTriangle();
        niAssert(bestTri != eInvalidHandle);
      }

      niAssert(bestTri != eInvalidHandle);
      if (bestTri == eInvalidHandle)
        break;  // this should never happen, but is kept to avoid crashes on corrupted data...
    }
  }

  ///////////////////////////////////////////////
  // Init first pass
  void InitFirstPass() {
    _baseVertexIndex = 0xFFFFFFFF;
    tU32 maxVertexIndex = 0;
    // Compute the base vertex index and the number of vertices used by the index array
    niLoop(i,_srcCount/3) {
      tU32 a = _src[i*3+0];
      tU32 b = _src[i*3+1];
      tU32 c = _src[i*3+2];
      if (a < _baseVertexIndex) { _baseVertexIndex = a; }
      if (a > maxVertexIndex)   { maxVertexIndex = a;   }
      if (b < _baseVertexIndex) { _baseVertexIndex = b; }
      if (b > maxVertexIndex)   { maxVertexIndex = b;   }
      if (c < _baseVertexIndex) { _baseVertexIndex = c; }
      if (c > maxVertexIndex)   { maxVertexIndex = c;   }
    }
    _numVertices = (maxVertexIndex-_baseVertexIndex)+1;
    _vcacheData.resize(_numVertices);

    // Allocate the vertex cache the triangle cache and increments counter of number of triangles that use each vertex
    niLoop(i,_srcCount/3) {
      tU32 a = _src[i*3+0]-_baseVertexIndex;
      tU32 b = _src[i*3+1]-_baseVertexIndex;
      tU32 c = _src[i*3+2]-_baseVertexIndex;
      // setup the triangle cache
      _tcacheData[i].verts[0] = a;
      _tcacheData[i].verts[1] = b;
      _tcacheData[i].verts[2] = c;
      // increment the triangle count for each vertices
      _vcacheData[a].numTris++;
      _vcacheData[b].numTris++;
      _vcacheData[c].numTris++;
    }
    // Compute the average number of triangles per vertex
    _averageTriPerVertex = tF32(_numVertices)/tF32(_srcCount/3);
    // Allocate the vertex cache triangle indices, init the active tris count
    niLoop(i,_vcacheData.size()) {
      VCacheData& v = _vcacheData[i];
      v.numActiveTris = v.numTris;
      v.tris.reserve(v.numTris);
    }
    // Fill the vertex cache triangle indices
    niLoop(i,_srcCount/3) {
      tU32 a = _src[i*3+0];
      tU32 b = _src[i*3+1];
      tU32 c = _src[i*3+2];
      _vcacheData[a].tris.push_back(i);
      _vcacheData[b].tris.push_back(i);
      _vcacheData[c].tris.push_back(i);
    }
  }

  ///////////////////////////////////////////////
  // Compute the score of the specified vertex
  tF32 ComputeVertexScore(const VCacheData* v) const {
    if (v->numActiveTris == 0) {
      // no triangle needs this vertex anymore !
      return -1.0f;
    }

    tF32 score = 0.0f;
    tI32 cachePos = v->cachePos;
    if (cachePos < 0) {
      // vertex is not in the fifo cache, no score
    }
    else {
      if (cachePos < 3) {
        // This vertex was used in the last triangle, so it has a fixed score,
        // whichever of the three it's in. Otherwise you can get very different
        // result depending on whether you add the triangle 1,2,3 or 3,2,1 -
        // which is not what we want.
        score = _lastTriScore;
      }
      else {
        niAssert(cachePos < (tI32)_maxVertexCacheSize);
        // Points for being high in the cache.
        const tF32 scaler = 1.0f / tF32(_maxVertexCacheSize-3);
        score = 1.0f - tF32(cachePos-3) * scaler;
        score = ni::Pow(score,_cacheDecayPower);
      }
    }

    // bonus points for having a low number of tris still
    // to use the vert, so we get rid of lone verts quickly
    tF32 valenceBoost = ni::Pow(tF32(v->numActiveTris), -_valenceBoostPower);
    score += _valenceBoostScale * valenceBoost;

    return score;
  }

  ///////////////////////////////////////////////
  // Compute the score of the specified triangle from it's vertices's scores
  tF32 ComputeTriangleScore(const TCacheData* t) const {
    const VCacheData& a = _vcacheData[t->verts[0]];
    const VCacheData& b = _vcacheData[t->verts[1]];
    const VCacheData& c = _vcacheData[t->verts[2]];
    return (a.score + b.score + c.score) / 3.0f;
  }

  ///////////////////////////////////////////////
  // Find the triangle that has the highest score
  tU32 FindAllBestScoreTriangle() const {
    tF32 bestScore = -1.0f;
    tU32 bestIndex = eInvalidHandle;
    niLoop(i,_tcacheData.size()) {
      const TCacheData& t = _tcacheData[i];
      if (t.added)
        continue;
      if (t.score > bestScore) {
        bestScore = t.score;
        bestIndex = i;
      }
    }
    return bestIndex;
  }

  ///////////////////////////////////////////////
  // Find the triangle that has the highest score in the LRU
  tU32 FindLRUBestScoreTriangle() const {
    tF32 bestScore = -10000.0f;
    tU32 bestIndex = eInvalidHandle;
    niLoop(i,_lru.size()) {
      niLoop(i,3) {
        const TCacheData& t = _tcacheData[i];
        if (t.added)
          continue;
        if (t.score > bestScore) {
          bestScore = t.score;
          bestIndex = i;
        }
      }
    }
    return bestIndex;
  }

  ///////////////////////////////////////////////
  void ProcessTriangle(TCacheData* apCacheData) {
    // get the indices
    tU32 a = apCacheData->verts[0];
    tU32 b = apCacheData->verts[1];
    tU32 c = apCacheData->verts[2];
    // remove from the LRU if already inside
    for (astl::deque<tIndex>::iterator it = _lru.begin(); it != _lru.end(); ) {
      if (*it == a || *it == b || *it == c) {
        it = _lru.erase(it);
      }
      else {
        ++it;
      }
    }
    // add to the head of the LRU
    _lru.push_front(a);
    _lru.push_front(b);
    _lru.push_front(c);
    // add triangle to the output
    apCacheData->added = eTrue;
    _output.push_back(a+_baseVertexIndex);
    _output.push_back(b+_baseVertexIndex);
    _output.push_back(c+_baseVertexIndex);
    // update vertices active triangles
    niAssert(_vcacheData[a].numActiveTris > 0);
    _vcacheData[a].numActiveTris--;
    niAssert(_vcacheData[b].numActiveTris > 0);
    _vcacheData[b].numActiveTris--;
    niAssert(_vcacheData[c].numActiveTris > 0);
    _vcacheData[c].numActiveTris--;
    // compute the cache pos and update the vertex scores
    tI32 cachePos = 0;
    for (astl::deque<tIndex>::iterator it = _lru.begin(); it != _lru.end(); ++it, ++cachePos) {
      VCacheData& v = _vcacheData[*it];
      v.cachePos = cachePos;
      v.score = ComputeVertexScore(&v);
    }
    // update the score of the triangles in the LRU
    niLoop(i,_lru.size()) {
      niLoop(i,3) {
        TCacheData& t = _tcacheData[i];
        if (t.added) continue;
        t.score = ComputeTriangleScore(&t);
      }
    }
    // remove the least recently used indices
    while (_lru.size() > (_maxVertexCacheSize-3)) {
      _lru.pop_back();
    }
  }
};

//////////////////////////////////////////////////////////////////////////////////////////////
// cGeometryPolygonal implementation.

///////////////////////////////////////////////
cGeometryPolygonal::cGeometryPolygonal(iGraphics* pGraphics, tU32 aulNumVertices, tU32 aulNumIndices, tGeometryCreateFlags aFlags, const cFVFDescription& aFVFDesc)
{
  niCheck(niIsOK(pGraphics),;);

  ZeroMembers();

  mptrGraphics = pGraphics;

  eArrayUsage VAUsage;
  if (!_GetVAUsage(aFlags, &VAUsage)) {
    niError(_A("Invalid vertex array usage creation flags."));
    return;
  }

  eArrayUsage IAUsage;
  if (!_GetIAUsage(aFlags, &IAUsage)) {
    niError(_A("Invalid index array usage creation flags."));
    return;
  }

  mptrVA = mptrGraphics->CreateVertexArray(aulNumVertices, aFVFDesc.GetFVF(), VAUsage);
  if (!mptrVA.IsOK()) {
    niError(_A("Can't create renderer vertex array."));
    return;
  }
  mptrIA = mptrGraphics->CreateIndexArray(eGraphicsPrimitiveType_TriangleList, aulNumIndices, aulNumVertices, IAUsage);
  if (!mptrIA.IsOK()) {
    niError(_A("Can't create renderer index array."));
    return;
  }

  mbOK = eTrue;
}

///////////////////////////////////////////////
cGeometryPolygonal::cGeometryPolygonal(iGraphics* pGraphics, iVertexArray* apVA, iIndexArray* apIA)
{
  ZeroMembers();

  if (!niIsOK(apVA)) {
    niError(_A("Invalid vertex array."));
    return;
  }
  if (!niIsOK(apIA)) {
    niError(_A("Invalid index array."));
    return;
  }

  mptrGraphics = pGraphics;

  mptrVA = apVA;
  mptrIA = apIA;
}

///////////////////////////////////////////////
cGeometryPolygonal::~cGeometryPolygonal()
{
  Invalidate();
}

///////////////////////////////////////////////
//! Zeros all the class members.
void cGeometryPolygonal::ZeroMembers()
{
}

///////////////////////////////////////////////
//! Sanity check.
tBool __stdcall cGeometryPolygonal::IsOK() const
{
  niClassIsOK(cGeometryPolygonal);
  return mptrIA.IsOK() && mptrVA.IsOK();
}

///////////////////////////////////////////////
void __stdcall cGeometryPolygonal::Invalidate()
{
  if (!mptrGraphics.IsOK()) return;
  mvFacesAdjacency.clear();
  mvFacesSubsetsIDs.clear();
  mulNumOriginalVertices = 0;
  mvDuplicationIndices.clear();
  mptrIA = NULL;
  mptrVA = NULL;
  mptrGraphics = NULL;
}

///////////////////////////////////////////////
//! Get the constant vertex array of this geometry.
iVertexArray* __stdcall cGeometryPolygonal::GetVertexArray() const
{
  return mptrVA;
}

///////////////////////////////////////////////
//! Get the constant index array of this geometry.
iIndexArray* __stdcall cGeometryPolygonal::GetIndexArray() const
{
  return mptrIA;
}

///////////////////////////////////////////////
//! Set duplication indices informations.
void cGeometryPolygonal::SetDuplicationIndices(tU32 ulOriginalVertices, tU32 ulIndices, const tU32* pIndices)
{
  if (!mptrGraphics.IsOK()) return;
  mulNumOriginalVertices = ulOriginalVertices;
  mvDuplicationIndices.resize(ulIndices);
  memcpy(&mvDuplicationIndices[0], pIndices, ulIndices*4);
}

///////////////////////////////////////////////
//! Get the number of duplication indices.
tU32 cGeometryPolygonal::GetNumDuplicationIndices() const
{
  return (tU32)mvDuplicationIndices.size();
}

///////////////////////////////////////////////
//! Get the number of indices before duplication.
tU32 cGeometryPolygonal::GetNumOriginalVertices() const
{
  return mulNumOriginalVertices;
}

///////////////////////////////////////////////
//! Get the duplication indices array.
const tU32* cGeometryPolygonal::GetDuplicationIndices() const
{
  return mvDuplicationIndices.empty()?NULL:&mvDuplicationIndices[0];
}

///////////////////////////////////////////////
tU32 __stdcall cGeometryPolygonal::GetNumFaces() const
{
  tU32 c = 0;
  if (mvSubsets.empty()) {
    return mptrIA.IsOK() ? mptrIA->GetNumIndices()/3 : 0;
  }
  else {
    niLoop(i,mvSubsets.size()) {
      const sGeometrySubset* sub = mvSubsets[i];
      if (sub) {
        c += sub->GetNumIndices()/3;
      }
    }
  }
  return c;
}

tBool cGeometryPolygonal::_BuildFaces(tFaceVec& avFaces) {
  if (!mptrVA.IsOK()) {
    niError(_A("Invalid vertex array."));
    return eFalse;
  }
  if (!mptrIA.IsOK()) {
    niError(_A("Invalid index array."));
    return eFalse;
  }

  const tU32 numVerts = mptrVA->GetNumVertices();

  avFaces.resize(GetNumFaces());
  if (avFaces.empty()) {
    niError(_A("No faces in the geometry."));
    return eFalse;
  }

  const tIndex* pIndices = (tIndex*)mptrIA->Lock(0,0,eLock_Normal);
  if (!pIndices) {
    niError(_A("Can't lock the index array."));
    return eFalse;
  }

  sFace* face = avFaces.data();
  if (mvSubsets.empty()) {
    const tIndex* inds = pIndices;
    niLoop(j,mptrIA->GetNumIndices()/3) {
      tIndex a = *inds++;
      tIndex b = *inds++;
      tIndex c = *inds++;
      if (a >= numVerts || b >= numVerts || c >= numVerts) {
        niWarning(niFmt(_A("Invalid index in face[%d](%d,%d,%d)."),
                        j,a,b,c));
        a = b = c = 0;
      }
      face->ulA = a;
      face->ulB = b;
      face->ulC = c;
      face->nSubset = 0;
      ++face;
    }
  }
  else {
    niLoop(i,mvSubsets.size()) {
      const sGeometrySubset* sub = mvSubsets[i];
      if (sub) {
        const tIndex* inds = pIndices+sub->GetFirstIndex();
        if (sub->GetFirstIndex() >= mptrIA->GetNumIndices()) {
          niWarning(niFmt(_A("Invalid subset[%d], first index (%d) out of range (%d)."),i,sub->GetFirstIndex(),mptrIA->GetNumIndices()));
          continue;
        }
        if (sub->GetFirstIndex()+sub->GetNumIndices() > mptrIA->GetNumIndices()) {
          niWarning(niFmt(_A("Invalid subset[%d], end index (%d) out of range (%d)."),i,sub->GetFirstIndex()+sub->GetNumIndices(),mptrIA->GetNumIndices()));
          continue;
        }
        niLoop(j,sub->GetNumIndices()/3) {
          tIndex a = *inds++;
          tIndex b = *inds++;
          tIndex c = *inds++;
          if (a >= numVerts || b >= numVerts || c >= numVerts) {
            niWarning(niFmt(_A("Invalid index in subset[%d], face[%d](%d,%d,%d)."),
                            i,j,a,b,c));
            a = b = c = 0;
          }
          face->ulA = a;
          face->ulB = b;
          face->ulC = c;
          face->nSubset = i;
          ++face;
        }
      }
    }
  }
  mptrIA->Unlock();
  return eTrue;
}

///////////////////////////////////////////////
//! Generate the specified things.
tBool __stdcall cGeometryPolygonal::Generate(eGeometryGenerate aGenerate, tF32 fEpsilon)
{
  if (!mptrGraphics.IsOK())
    return eFalse;

  // Generate the faces.
  tFaceVec vFaces;
  if (!_BuildFaces(vFaces)) {
    niError(_A("Can't build faces."));
    return eFalse;
  }

  //! Lock the vertex array and generate the specified things.
  tPtr pVerts = mptrVA->Lock(0,0,eLock_Normal);
  if (!pVerts) {
    niError(_A("Can't lock vertex array."));
    return eFalse;
  }
  cFVFDescription fvfDesc(mptrVA->GetFVF());
  cFVFStream stm(&fvfDesc, pVerts, mptrVA->GetNumVertices());
  if (fvfDesc.HasPosition()) {
    // Generate faces normals
    {
      cFVFVertex A(&stm.GetFVFDescription()), B(&stm.GetFVFDescription()), C(&stm.GetFVFDescription());
      for (tFaceVecIt itFace = vFaces.begin(); itFace != vFaces.end(); ++itFace)
      {
        A.Set(stm.GetVertex(itFace->ulA));
        B.Set(stm.GetVertex(itFace->ulB));
        C.Set(stm.GetVertex(itFace->ulC));
        PlaneNormalize(PlaneFromPoints(itFace->Plane, A.Position(), B.Position(), C.Position()));
      }
    }

    if (niFlagTest(aGenerate, eGeometryGenerate_Adjacency) //||
        //niFlagTest(aGenerate, eGeometryGenerate_TextureSpaces) ||
        //niFlagTest(aGenerate, eGeometryGenerate_Normals)
        )
    {
      _GenerateAdjacencyArrays(stm, vFaces, fEpsilon);
    }

    if (niFlagTest(aGenerate, eGeometryGenerate_FlatNormals) ||
        niFlagTest(aGenerate, eGeometryGenerate_Normals))
    {
      _GenerateNormals(stm, vFaces, niFlagTest(aGenerate, eGeometryGenerate_FlatNormals));
    }
  }
  mptrVA->Unlock();

  return eTrue;
}

///////////////////////////////////////////////
//! Set the faces subset's IDs.
void __stdcall cGeometryPolygonal::SetFacesSubsetsIDs(const tU32* apIDs)
{
  if (!mptrGraphics.IsOK()) return;
  niAssert(GetNumFaces());
  mvFacesSubsetsIDs.resize(GetNumFaces());
  memcpy(&mvFacesSubsetsIDs[0], apIDs, sizeof(tU32)*GetNumFaces());
}

///////////////////////////////////////////////
//! Get the faces subsets IDs.
const tU32* __stdcall cGeometryPolygonal::GetFacesSubsetsIDs() const
{
  return mvFacesSubsetsIDs.empty()?NULL:&mvFacesSubsetsIDs[0];
}

///////////////////////////////////////////////
//! Get the adjacency array.
const tU32* __stdcall cGeometryPolygonal::GetAdjacencyArray() const
{
  return mvFacesAdjacency.empty()?NULL:&mvFacesAdjacency[0];
}

///////////////////////////////////////////////
//! Get the number of subsets.
tU32 __stdcall cGeometryPolygonal::GetNumSubsets() const
{
  return (tU32)mvSubsets.size();
}

///////////////////////////////////////////////
//! Get the subset at the specified index.
iGeometrySubset* __stdcall cGeometryPolygonal::GetSubset(tU32 aulIdx) const
{
  if (!mptrGraphics.IsOK()) return NULL;
  if (aulIdx >= mvSubsets.size()) return NULL;
  return mvSubsets[aulIdx];
}

///////////////////////////////////////////////
//! Get the index of the subset with the specified ID.
tU32 __stdcall cGeometryPolygonal::GetSubsetIndex(tU32 aulID) const
{
  if (mptrGraphics.IsOK()) {
    tU32 ulCount = 0;
    for (tGeometrySubsetVec::const_iterator itSub = mvSubsets.begin(); itSub != mvSubsets.end(); ++itSub, ++ulCount)
    {
      if ((*itSub)->GetID() == aulID)
        return ulCount;
    }
  }
  return eInvalidHandle;
}

///////////////////////////////////////////////
//! Add a subset.
iGeometrySubset* __stdcall cGeometryPolygonal::AddSubset(tU32 anID, tU32 anFirstIndex, tU32 anNumIndices, tU32 anMaterial)
{
  if (!mptrGraphics.IsOK()) return NULL;
  tGeometrySubsetPtr ptrSubset = niNew sGeometrySubset();
  ptrSubset->SetID(anID);
  ptrSubset->SetFirstIndex(anFirstIndex);
  ptrSubset->SetNumIndices(anNumIndices);
  ptrSubset->SetMaterial(anMaterial);
  mvSubsets.push_back(ptrSubset);
  return ptrSubset;
}

///////////////////////////////////////////////
//! Remove the subset at the given index.
tBool __stdcall cGeometryPolygonal::RemoveSubset(tU32 aulIdx)
{
  if (!mptrGraphics.IsOK()) return eFalse;
  if (aulIdx >= mvSubsets.size()) return eFalse;
  mvSubsets.erase(mvSubsets.begin()+aulIdx);
  return eTrue;
}

///////////////////////////////////////////////
//! Create a copy of this geometry which use different vertex and/or index array format.
iGeometry* __stdcall cGeometryPolygonal::Clone(tGeometryCreateFlags aFlags, tFVF aFVF)
{
  if (!mptrGraphics.IsOK()) return NULL;

  if (!_GetVAUsage(aFlags,NULL)) {
    switch (mptrVA->GetUsage())
    {
      case eArrayUsage_Dynamic:     aFlags |= eGeometryCreateFlags_VADynamic; break;
      case eArrayUsage_DynamicReadWrite:  aFlags |= eGeometryCreateFlags_VADynamicReadWrite; break;
      case eArrayUsage_Static:      aFlags |= eGeometryCreateFlags_VAStatic; break;
      case eArrayUsage_SystemMemory:    aFlags |= eGeometryCreateFlags_VASystemMemory; break;
    }
  }

  if (!_GetIAUsage(aFlags,NULL)) {
    switch (mptrIA->GetUsage())
    {
      case eArrayUsage_Dynamic:     aFlags |= eGeometryCreateFlags_IADynamic; break;
      case eArrayUsage_DynamicReadWrite:  aFlags |= eGeometryCreateFlags_IADynamicReadWrite; break;
      case eArrayUsage_Static:      aFlags |= eGeometryCreateFlags_IAStatic; break;
      case eArrayUsage_SystemMemory:    aFlags |= eGeometryCreateFlags_IASystemMemory; break;
    }
  }

  Ptr<cGeometryPolygonal> ptrGeom = niNew cGeometryPolygonal(mptrGraphics,
                                                             mptrVA->GetNumVertices(),
                                                             mptrIA->GetNumIndices(),
                                                             aFlags,
                                                             aFVF?aFVF:mptrVA->GetFVF());
  if (!ptrGeom.IsOK()) {
    niError(_A("Can't create new geometry."));
    return NULL;
  }

  // Copy vertices
  cFVFDescription srcFVF(mptrVA->GetFVF());
  cFVFStream srcStream(&srcFVF);
  cFVFDescription dstFVF(ptrGeom->GetVertexArray()->GetFVF());
  cFVFStream dstStream(&dstFVF);

  srcStream.SetBase(mptrVA->Lock(0,0,eLock_Normal), mptrVA->GetNumVertices());
  if (!srcStream.IsOK()) {
    niError(_A("Can't lock source VA."));
    return NULL;
  }

  dstStream.SetBase(ptrGeom->GetVertexArray()->Lock(0,0,eLock_Normal), ptrGeom->GetVertexArray()->GetNumVertices());
  if (!dstStream.IsOK()) {
    niError(_A("Can't lock destination VA."));
    return NULL;
  }

  dstStream.Copy(srcStream);

  ptrGeom->GetVertexArray()->Unlock();
  mptrVA->Unlock();

  // Copy indices
  tIndex* pSrcIndices = (tIndex*)mptrIA->Lock(0,0,eLock_Normal);
  tIndex* pDstIndices = (tIndex*)ptrGeom->GetIndexArray()->Lock(0,0,eLock_Normal);
  memcpy(pDstIndices, pSrcIndices, mptrIA->GetNumIndices()*sizeof(tIndex));
  mptrIA->Unlock();
  ptrGeom->GetIndexArray()->Unlock();

  // Copy subsets
  for (tU32 i = 0; i < GetNumSubsets(); ++i) {
    iGeometrySubset* pSubset = GetSubset(i);
    ptrGeom->AddSubset(pSubset->GetID(),pSubset->GetFirstIndex(),pSubset->GetNumIndices(),pSubset->GetMaterial());
  }

  // Copy extra infos
  ptrGeom->mvFacesAdjacency = mvFacesAdjacency;
  ptrGeom->mvDuplicationIndices = mvDuplicationIndices;
  ptrGeom->mvFacesSubsetsIDs = mvFacesSubsetsIDs;
  return ptrGeom.GetRawAndSetNull();
}

///////////////////////////////////////////////
tBool __stdcall cGeometryPolygonal::SetDrawOp(iDrawOperation* apDrawOp, tU32 aulSubsetIdx) {
  niCheck(niIsOK(apDrawOp),eFalse);

  iGeometrySubset* pSubSet = GetSubset(aulSubsetIdx);
  if (!niIsOK(pSubSet))
    return eFalse;

  apDrawOp->SetIndexArray(mptrIA);
  apDrawOp->SetVertexArray(mptrVA);
  apDrawOp->SetFirstIndex(pSubSet->GetFirstIndex());
  apDrawOp->SetNumIndices(pSubSet->GetNumIndices());
  apDrawOp->SetBaseVertexIndex(0);
  return eTrue;
}

///////////////////////////////////////////////
void cGeometryPolygonal::_GenerateAdjacencyArrays(cFVFStream& astmVerts, tFaceVec& avFaces, tF32 afEpsilon)
{
  const cFVFDescription& fvfDesc = astmVerts.GetFVFDescription();
  if (!fvfDesc.HasPosition()) {
    return;
  }

  cFVFVertex A[3];
  cFVFVertex vtxB0(&astmVerts.GetFVFDescription()),
      vtxB1(&astmVerts.GetFVFDescription()),
      vtxB2(&astmVerts.GetFVFDescription());

  mvFacesAdjacency.resize(avFaces.size()*3);
  astl::fill(mvFacesAdjacency, eInvalidHandle);

  tU32* pFacesAdjacencyA = &mvFacesAdjacency[0];

  tU32 i;
  for (i = 0; i < 3; ++i)
  {
    A[i].SetEx(&astmVerts.GetFVFDescription());
  }

  for (tFaceVecIt itFaceA = avFaces.begin(); itFaceA != avFaces.end(); ++itFaceA)
  {
    A[0].Set(astmVerts.GetVertex(itFaceA->ulA));
    A[1].Set(astmVerts.GetVertex(itFaceA->ulB));
    A[2].Set(astmVerts.GetVertex(itFaceA->ulC));

    for (i = 0; i < 3; ++i)
    {
      const sVec3f& A0 = A[i].Position();
      const sVec3f& A1 = A[(i+1)%3].Position();

      for (tFaceVecIt itFaceB = avFaces.begin(); itFaceB != avFaces.end(); ++itFaceB)
      {
        if (itFaceA == itFaceB)
          continue;

        vtxB0.Set(astmVerts.GetVertex(itFaceB->ulA));
        vtxB1.Set(astmVerts.GetVertex(itFaceB->ulB));
        vtxB2.Set(astmVerts.GetVertex(itFaceB->ulC));

        const sVec3f& B0 = vtxB0.Position();
        const sVec3f& B1 = vtxB1.Position();
        const sVec3f& B2 = vtxB2.Position();


        // Check against first edge 0-1
        if ((VecEqual(A0,B0,afEpsilon) && VecEqual(A1,B1,afEpsilon)) ||
            (VecEqual(A0,B1,afEpsilon) && VecEqual(A1,B0,afEpsilon)))
        {
          pFacesAdjacencyA[i] = astl::iterator_index(avFaces,itFaceB);
          //          TRACE_GEOMETRY((_A("]] Face %d, Adj %d: %d\n"),astl::iterator_index(avFaces,itFaceA),i,pFacesAdjacencyA[i]));
          break;
        }

        // Check against second edge 1-2
        if ((VecEqual(A0,B1,afEpsilon) && VecEqual(A1,B2,afEpsilon)) ||
            (VecEqual(A0,B2,afEpsilon) && VecEqual(A1,B1,afEpsilon)))
        {
          pFacesAdjacencyA[i] = astl::iterator_index(avFaces,itFaceB);
          //          TRACE_GEOMETRY((_A("]] Face %d, Adj %d: %d\n"),astl::iterator_index(avFaces,itFaceA),i,pFacesAdjacencyA[i]));
          break;
        }

        // Check against third edge 2-0
        if ((VecEqual(A0,B2,afEpsilon) && VecEqual(A1,B0,afEpsilon)) ||
            (VecEqual(A0,B0,afEpsilon) && VecEqual(A1,B2,afEpsilon)))
        {
          pFacesAdjacencyA[i] = astl::iterator_index(avFaces,itFaceB);
          //          TRACE_GEOMETRY((_A("]] Face %d, Adj %d: %d\n"),astl::iterator_index(avFaces,itFaceA),i,pFacesAdjacencyA[i]));
          break;
        }
      }
    }

    pFacesAdjacencyA += 3;
  }
}

///////////////////////////////////////////////
void cGeometryPolygonal::_GenerateNormals(cFVFStream& astmVerts, tFaceVec& avFaces, tBool bFlat)
{
  const cFVFDescription& fvfDesc = astmVerts.GetFVFDescription();
  if (!fvfDesc.HasNormal()) {
    return;
  }

  cFVFVertex A(&astmVerts.GetFVFDescription()),
      B(&astmVerts.GetFVFDescription()),
      C(&astmVerts.GetFVFDescription());
  tFaceVecIt itFace;

  // Use faces normals
  if (bFlat)
  {
    for (itFace = avFaces.begin(); itFace != avFaces.end(); ++itFace)
    {
      A.Set(astmVerts.GetVertex(itFace->ulA));
      B.Set(astmVerts.GetVertex(itFace->ulB));
      C.Set(astmVerts.GetVertex(itFace->ulC));
      A.Normal() = B.Normal() = C.Normal() = itFace->Plane.GetNormal();
    }
  }
  // Apply normals on vertices
  else
  {
    for (itFace = avFaces.begin(); itFace != avFaces.end(); ++itFace) {
      A.Set(astmVerts.GetVertex(itFace->ulA));
      A.Normal().Set(0.0f);
      B.Set(astmVerts.GetVertex(itFace->ulB));
      B.Normal().Set(0.0f);
      C.Set(astmVerts.GetVertex(itFace->ulC));
      C.Normal().Set(0.0f);
    }

    for (itFace = avFaces.begin(); itFace != avFaces.end(); ++itFace)
    {
      A.Set(astmVerts.GetVertex(itFace->ulA));
      B.Set(astmVerts.GetVertex(itFace->ulB));
      C.Set(astmVerts.GetVertex(itFace->ulC));
      const sVec3f planeNormal = itFace->Plane.GetNormal();
      A.Normal() += planeNormal;
      B.Normal() += planeNormal;
      C.Normal() += planeNormal;
    }

    if (!mvDuplicationIndices.empty()) {
      for (itFace = avFaces.begin(); itFace != avFaces.end(); ++itFace) {
        if (mvDuplicationIndices[itFace->ulA] != itFace->ulA) {
          A.Set(astmVerts.GetVertex(itFace->ulA));
          A.Normal() += itFace->Plane.GetNormal();
        }

        if (mvDuplicationIndices[itFace->ulB] != itFace->ulB) {
          B.Set(astmVerts.GetVertex(itFace->ulB));
          B.Normal() += itFace->Plane.GetNormal();
        }
        if (mvDuplicationIndices[itFace->ulC] != itFace->ulC) {
          C.Set(astmVerts.GetVertex(itFace->ulC));
          C.Normal() += itFace->Plane.GetNormal();
        }
      }
    }

    for (itFace = avFaces.begin(); itFace != avFaces.end(); ++itFace) {
      A.Set(astmVerts.GetVertex(itFace->ulA));
      VecNormalize(A.Normal());
      B.Set(astmVerts.GetVertex(itFace->ulB));
      VecNormalize(B.Normal());
      C.Set(astmVerts.GetVertex(itFace->ulC));
      VecNormalize(C.Normal());
    }
  }
}

///////////////////////////////////////////////
tBool cGeometryPolygonal::_GetVAUsage(tGeometryCreateFlags aFlags, eArrayUsage* aVAUsage)
{
  if (niFlagTest(aFlags,eGeometryCreateFlags_VADynamic))
  {
    if (aVAUsage) *aVAUsage = eArrayUsage_Dynamic;
  }
  else if (niFlagTest(aFlags,eGeometryCreateFlags_VADynamicReadWrite))
  {
    if (aVAUsage) *aVAUsage = eArrayUsage_DynamicReadWrite;
  }
  else if (niFlagTest(aFlags,eGeometryCreateFlags_VAStatic))
  {
    if (aVAUsage) *aVAUsage = eArrayUsage_Static;
  }
  else if (niFlagTest(aFlags,eGeometryCreateFlags_VASystemMemory))
  {
    if (aVAUsage) *aVAUsage = eArrayUsage_SystemMemory;
  }
  else
    return eFalse;

  return eTrue;
}

///////////////////////////////////////////////
tBool cGeometryPolygonal::_GetIAUsage(tGeometryCreateFlags aFlags, eArrayUsage* aIAUsage)
{
  if (niFlagTest(aFlags,eGeometryCreateFlags_IADynamic))
  {
    if (aIAUsage) *aIAUsage = eArrayUsage_Dynamic;
  }
  else if (niFlagTest(aFlags,eGeometryCreateFlags_IADynamicReadWrite))
  {
    if (aIAUsage) *aIAUsage = eArrayUsage_DynamicReadWrite;
  }
  else if (niFlagTest(aFlags,eGeometryCreateFlags_IAStatic))
  {
    if (aIAUsage) *aIAUsage = eArrayUsage_Static;
  }
  else if (niFlagTest(aFlags,eGeometryCreateFlags_IASystemMemory))
  {
    if (aIAUsage) *aIAUsage = eArrayUsage_SystemMemory;
  }
  else
    return eFalse;

  return eTrue;
}

///////////////////////////////////////////////
//! Optimize the mesh.
tBool __stdcall cGeometryPolygonal::Optimize(tGeometryOptimizeFlags aFlags)
{
  tFaceVec vFaces;
  if (!_BuildFaces(vFaces)) {
    niError(_A("Can't build faces."));
    return eFalse;
  }

  TRACE_GEOMETRY((_A("*** Begining of geometry optimization (%d verts, %d inds, %d subsets) ***"),mptrVA->GetNumVertices(),mptrIA->GetNumIndices(),mvSubsets.size()));

  // Remove degenerate triangles
  if (aFlags&eGeometryOptimizeFlags_RemoveDegenerates) {
    tU32 nRet = _OptimizeRemoveDegenerates(aFlags,vFaces);
    if (nRet == eInvalidHandle)
    {
      niError(niFmt(_A("Can't remove degenerate faces.")));
      return eFalse;
    }
    TRACE_GEOMETRY((_A("# Remove degnerates: %d."),nRet));
  }
  if (aFlags&eGeometryOptimizeFlags_RemoveDuplicateVertices) {
    tU32 nRet = _OptimizeDuplicateVertices(aFlags,vFaces);
    if (nRet == eInvalidHandle)
    {
      niError(niFmt(_A("Can't remove duplicate vertices.")));
      return eFalse;
    }
    TRACE_GEOMETRY((_A("# Remove duplicates: %d."),nRet));
  }
  if (aFlags&eGeometryOptimizeFlags_RemoveUnusedVertices) {
    tU32 nRet = _OptimizeUnusedVertices(aFlags,vFaces);
    if (nRet == eInvalidHandle)
    {
      niError(niFmt(_A("Can't remove unused vertices.")));
      return eFalse;
    }
    TRACE_GEOMETRY((_A("# Remove unused: %d."),nRet));
  }

  // Update the subsets
  if (!mvSubsets.empty()) {
    tGeometrySubsetVec vNewSubsets;
    for (tGeometrySubsetVec::iterator itSubset = mvSubsets.begin(); itSubset != mvSubsets.end(); ++itSubset)
    {
      tU32 nFirstIdx = ~0;
      tU32 nNumIndices = 0;
      const tU32 nSubsetIndex = astl::iterator_index(mvSubsets,itSubset);
      for (tFaceVecIt itFace = vFaces.begin(); itFace != vFaces.end(); ++itFace)
      {
        //        if (itFace->nSubset == nSubsetIndex)
        //          break;
        //        nFirstIndex += 3;
        if (itFace->nSubset != nSubsetIndex)
          continue;
        if (nFirstIdx == ~0) {
          nFirstIdx = astl::iterator_index(vFaces,itFace)*3;
        }
        nNumIndices += 3;
      }
      //      for ( ; itFace != vFaces.end(); ++itFace)
      //      {
      //        if (itFace->nSubset != nSubsetIndex)
      //          break;
      //        nNumIndices += 3;
      //      }
      {
        sGeometrySubset* pSubset = *itSubset;
        pSubset->SetFirstIndex(nNumIndices==0?0:nFirstIdx);
        pSubset->SetNumIndices(nNumIndices);
        vNewSubsets.push_back(pSubset);
      }
    }
    mvSubsets = vNewSubsets;
  }

  // ACMR computation
  float acmr = -1.0f;
  float gacmr = -1.0f;
  /*
    {
    tU32 numInds = mptrIA->GetNumIndices();
    tIndex* inds = (tIndex*)mptrIA->Lock(0,numInds,eLock_ReadOnly);
    gacmr = computeACMR(inds,numInds,16);
    acmr = 0.0f;
    niLoop(i,mvSubsets.size()) {
    acmr += computeACMR(inds+mvSubsets[i]->GetFirstIndex(),mvSubsets[i]->GetNumIndices(),32);
    }
    acmr /= float(mvSubsets.size());

    CacheOptimize cacheOpt(inds,numInds);
    cacheOpt.Process();
    niAssert(cacheOpt._output.size() == numInds);
    tF32 cacmr = computeACMR(&cacheOpt._output[0],cacheOpt._output.size(),16);

    mptrIA->Unlock();

    }
  */
  TRACE_GEOMETRY((_A("*** End of geometry optimization (%d verts, %d tris, %d subsets, %.5f acmr, %.5f gacmr) ***"),mptrVA->GetNumVertices(),mptrIA->GetNumIndices()/3,mvSubsets.size(),acmr,gacmr));
  return eTrue;
}

///////////////////////////////////////////////
tU32 cGeometryPolygonal::_OptimizeRemoveDegenerates(tGeometryOptimizeFlags aFlags, tFaceVec& avFaces)
{
  tU32 nNumDegenerate = 0;

  tPtr pVerts = mptrVA->Lock(0,0,eLock_ReadOnly);
  if (!pVerts)
  {
    niError(_A("Can't lock vertex array."));
    return eInvalidHandle;
  }
  cFVFDescription fvfDesc(mptrVA->GetFVF());
  cFVFStream stm(&fvfDesc, pVerts, mptrVA->GetNumVertices());

  tFaceVec vNewFaces;
  vNewFaces.reserve(avFaces.size());
  cFVFVertex A(&stm.GetFVFDescription()), B(&stm.GetFVFDescription()), C(&stm.GetFVFDescription());
  sVec3f triNormal;
  for (tFaceVecIt itFace = avFaces.begin(); itFace != avFaces.end(); ++itFace)
  {
    A.Set(stm.GetVertex(itFace->ulA));
    B.Set(stm.GetVertex(itFace->ulB));
    C.Set(stm.GetVertex(itFace->ulC));
    if (TriangleIsDegenerate(A.Position(),B.Position(),C.Position()) ||
        TriangleAreaAndNormal(triNormal,A.Position(),B.Position(),C.Position()) <
        niEpsilon9)
    {
      ++nNumDegenerate;
    }
    else
    {
      vNewFaces.push_back(*itFace);
    }
  }

  avFaces = vNewFaces;
  mptrVA->Unlock();

  if (!nNumDegenerate) return 0;

  // Reconstruct the index array
  if (!_OptimizeRebuildIndexArray(aFlags,avFaces)) {
    niError(_A("Can't rebuild the index array."));
    return eInvalidHandle;
  }

  return nNumDegenerate;
}

///////////////////////////////////////////////
tU32 cGeometryPolygonal::_OptimizeUnusedVertices(tGeometryOptimizeFlags aFlags, tFaceVec& avFaces)
{
  tU32 nNumUnused = 0;
  tU32Vec vVAStat;
  tU32VecIt itV;
  vVAStat.resize(mptrVA->GetNumVertices());
  astl::fill(vVAStat,eInvalidHandle);

  // first mark all the used vertices
  for (tFaceVecIt itFace = avFaces.begin(); itFace != avFaces.end(); ++itFace)
  {
    vVAStat[itFace->ulA] = 0;
    vVAStat[itFace->ulB] = 0;
    vVAStat[itFace->ulC] = 0;
  }

  // count the unused vertices
  for (itV = vVAStat.begin(); itV != vVAStat.end(); ++itV)
  {
    if (*itV == 0) continue;
    ++nNumUnused;
  }

  if (!nNumUnused)  return 0;

  // Allocate the new vertex array
  Ptr<iVertexArray> ptrNewVA =
      mptrGraphics->CreateVertexArray(mptrVA->GetNumVertices()-nNumUnused, mptrVA->GetFVF(), mptrVA->GetUsage());
  if (!niIsOK(ptrNewVA)) {
    niError(_A("Can't create vertex array."));
    return eInvalidHandle;
  }

  // copy the vertices in the new vertex array
  tPtr pVertsSrc = mptrVA->Lock(0,0,eLock_ReadOnly);
  if (!pVertsSrc)
  {
    niError(_A("Can't lock src vertex array."));
    return eInvalidHandle;
  }
  cFVFDescription srcFvf(mptrVA->GetFVF());
  cFVFStream stmSrc(&srcFvf, pVertsSrc, mptrVA->GetNumVertices());

  tPtr pVertsDst = ptrNewVA->Lock(0,0,eLock_Discard);
  if (!pVertsDst)
  {
    niError(_A("Can't lock dest vertex array."));
    return eInvalidHandle;
  }
  cFVFDescription dstFvf(ptrNewVA->GetFVF());
  cFVFStream stmDst(&dstFvf, pVertsDst, ptrNewVA->GetNumVertices());

  cFVFVertex s(&stmSrc.GetFVFDescription());
  cFVFVertex d(&stmDst.GetFVFDescription());
  itV = vVAStat.begin();
  tU32 nCount = 0;
  for (stmSrc.Reset(), stmDst.Reset(); !stmSrc.End(); stmSrc.Next(), ++itV)
  {
    if (*itV != 0) continue;
    s.Set(stmSrc.Current());
    d.Set(stmDst.Current());
    d.Copy(s);
    stmDst.Next();
    *itV = nCount;  // Set the new index
    ++nCount;
  }

  ptrNewVA->Unlock();
  mptrVA->Unlock();

  mptrVA = ptrNewVA;

  // reassign the indices
  for (tFaceVecIt itFace = avFaces.begin(); itFace != avFaces.end(); ++itFace)
  {
    itFace->ulA = vVAStat[itFace->ulA];
    niAssert(itFace->ulA != eInvalidHandle);
    itFace->ulB = vVAStat[itFace->ulB];
    niAssert(itFace->ulB != eInvalidHandle);
    itFace->ulC = vVAStat[itFace->ulC];
    niAssert(itFace->ulC != eInvalidHandle);
  }

  // Reconstruct the index array
  if (!_OptimizeRebuildIndexArray(aFlags,avFaces)) {
    niError(_A("Can't rebuild the index array."));
    return eInvalidHandle;
  }

  return nNumUnused;
}

///////////////////////////////////////////////
tU32 cGeometryPolygonal::_OptimizeDuplicateVertices(tGeometryOptimizeFlags aFlags, tFaceVec& avFaces)
{
  tU32 nNumDuplicates = 0;

  tPtr pVertsSrc = mptrVA->Lock(0,0,eLock_ReadOnly);
  if (!pVertsSrc) {
    niError(_A("Can't lock vertex array."));
    return eInvalidHandle;
  }
  cFVFDescription fvfDesc(mptrVA->GetFVF());
  cFVFStream stmA(&fvfDesc, pVertsSrc, mptrVA->GetNumVertices());
  cFVFStream stmB(&fvfDesc, pVertsSrc, mptrVA->GetNumVertices());
  const cFVFDescription& fvf = stmA.GetFVFDescription();

  astl::vector<tU32> vDuplicate;
  vDuplicate.resize(mptrVA->GetNumVertices());
  astl::fill(vDuplicate,eInvalidHandle);

#define HASH(VEC) ((tU16)(HashVec3(VEC.ptr())&0xFFF))

  tU32Vec vHash[0x1000];

  tU32 nCountA = 0;
  for (stmA.Reset(); !stmA.End(); stmA.Next(), ++nCountA)
  {
    cFVFVertex& vertA = stmA.Vertex();
    vHash[HASH(vertA.Position())].push_back(nCountA);
  }

  for (tU32 i = 0; i < 0x1000; ++i)
  {
    if (vHash[i].empty()) continue;

    for (tU32VecIt itPA = vHash[i].begin(); itPA != vHash[i].end(); ++itPA)
    {
      nCountA = *itPA;
      if (vDuplicate[nCountA] != eInvalidHandle) continue;
      stmA.SetCurrentIndex(nCountA);
      cFVFVertex& vertA = stmA.Vertex();
      for (tU32VecIt itPB = itPA+1; itPB != vHash[i].end(); ++itPB)
      {
        tU32 nCountB = *itPB;
        if (vDuplicate[nCountB] != eInvalidHandle) continue;
        stmB.SetCurrentIndex(nCountB);
        cFVFVertex& vertB = stmB.Vertex();
        if (fvf.HasPosition() && (vertA.Position() != vertB.Position())) continue;
        if (fvf.HasNormal()   && (vertA.Normal() != vertB.Normal())) continue;
        if (fvf.HasColorA()   && (vertA.ColorA() != vertB.ColorA())) continue;

        tU32 ulNumTexCoos = fvf.GetNumTexCoos();
        if (ulNumTexCoos)
        {
          tBool bSameTexCoo = eTrue;
          for (tU32 i = 0; i < ulNumTexCoos; ++i)
          {
            tU32 ulTexCooDim = fvf.GetTexCooDim(i);
            switch (ulTexCooDim)
            {
              case 4: if (vertA.TexCoo4(i) != vertB.TexCoo4(i)) bSameTexCoo = eFalse; break;
              case 3: if (vertA.TexCoo3(i) != vertB.TexCoo3(i)) bSameTexCoo = eFalse; break;
              case 2: if (vertA.TexCoo2(i) != vertB.TexCoo2(i)) bSameTexCoo = eFalse; break;
            }
            if (!bSameTexCoo)
              break;
          }
          if (!bSameTexCoo)
            continue;
        }

        if (fvf.HasWeights4() && (vertA.Weights4() != vertB.Weights4()))
          continue;
        if (fvf.HasWeights3() && (vertA.Weights3() != vertB.Weights3()))
          continue;
        if (fvf.HasWeights2() && (vertA.Weights2() != vertB.Weights2()))
          continue;
        if (fvf.HasWeights1() && (vertA.Weights1() != vertB.Weights1()))
          continue;
        if (fvf.HasIndices() && (vertA.Indices() != vertB.Indices()))
          continue;
        if (fvf.HasPointSize() && (vertA.PointSize() != vertB.PointSize()))
          continue;

        vDuplicate[nCountB] = nCountA;
        ++nNumDuplicates;
      }
    }
  }
  mptrVA->Unlock();

  // reassign the indices
  for (tFaceVecIt itFace = avFaces.begin(); itFace != avFaces.end(); ++itFace)
  {
    if (vDuplicate[itFace->ulA] != eInvalidHandle)  itFace->ulA = vDuplicate[itFace->ulA];
    if (vDuplicate[itFace->ulB] != eInvalidHandle)  itFace->ulB = vDuplicate[itFace->ulB];
    if (vDuplicate[itFace->ulC] != eInvalidHandle)  itFace->ulC = vDuplicate[itFace->ulC];
  }

  // Reconstruct the index array
  if (!_OptimizeRebuildIndexArray(aFlags,avFaces)) {
    niError(_A("Can't rebuild the index array."));
    return eInvalidHandle;
  }

  return nNumDuplicates;
}

///////////////////////////////////////////////
tBool cGeometryPolygonal::_OptimizeRebuildIndexArray(tGeometryOptimizeFlags aFlags, tFaceVec& avFaces)
{
  eArrayUsage usage = mptrIA->GetUsage();
  mptrIA = NULL;
  mptrIA = mptrGraphics->CreateIndexArray(eGraphicsPrimitiveType_TriangleList, (tU32)avFaces.size()*3,
                                          mptrVA->GetNumVertices(), usage);
  if (!mptrIA.IsOK()) {
    niError(_A("Can't create renderer index array."));
    return eFalse;
  }

  tIndex* pIndices = (tIndex*)mptrIA->Lock(0,0,eLock_Discard);
  if (!pIndices)
  {
    niError(_A("Can't lock the index array."));
    return eFalse;
  }

#ifdef _DEBUG
  const tU32 numVerts = mptrVA->GetNumVertices();
#endif
  for (tFaceVecIt itFace = avFaces.begin(); itFace != avFaces.end(); ++itFace)
  {
    *pIndices = itFace->ulA;
    niAssert(*pIndices < numVerts);
    ++pIndices;
    *pIndices = itFace->ulB;
    niAssert(*pIndices < numVerts);
    ++pIndices;
    *pIndices = itFace->ulC;
    niAssert(*pIndices < numVerts);
    ++pIndices;
  }

  mptrIA->Unlock();
  return eTrue;
}


///////////////////////////////////////////////
iGeometry* __stdcall cGraphics::CreateGeometryPolygonal(tU32 aulNumVertices, tU32 aulNumIndices, tGeometryCreateFlags aFlags, tFVF aFVF)
{
  return niNew cGeometryPolygonal(this, aulNumVertices, aulNumIndices, aFlags, cFVFDescription(aFVF));
}

///////////////////////////////////////////////
iGeometry* __stdcall cGraphics::CreateGeometryPolygonalEx(iVertexArray* apVA, iIndexArray* apIA)
{
  return niNew cGeometryPolygonal(this, apVA, apIA);
}

///////////////////////////////////////////////
//! Create a cube polygonal geometry.
iGeometry* cGraphics::CreateGeometryPolygonalCube(tGeometryCreateFlags aFlags, tFVF aFVF,
                                                  const sVec3f& avCenter, tF32 afWidth,
                                                  tBool abCW, tU32 aulColor, const sMatrixf& amtxUV)
{
  iGeometry* pGeom = CreateGeometryPolygonal(24, 36, aFlags, aFVF);
  if (!niIsOK(pGeom)) {
    niError(_A("Can't create geometry."));
    return NULL;
  }

  cFVFDescription fvf(pGeom->GetVertexArray()->GetFVF());
  cFVFStream verts(&fvf, pGeom->GetVertexArray()->Lock(0,0,eLock_Normal), 24);

  if (fvf.HasPosition())
  {
    tF32 size = afWidth/2.0f;

    verts.Reset();
#define SET(vx,vy,vz)                             \
    verts.Vertex().Position().x = vx+avCenter.x;  \
    verts.Vertex().Position().y = vy+avCenter.y;  \
    verts.Vertex().Position().z = vz+avCenter.z;  \
    verts.Next();

    // right plane
    SET( size,  size, -size);
    SET( size,  size,  size);
    SET( size, -size,  size);
    SET( size, -size, -size);

    // left plane
    SET( -size,  size,  size);
    SET( -size,  size, -size);
    SET( -size, -size, -size);
    SET( -size, -size,  size);

    // top plane
    SET( -size,  size,  size);
    SET(  size,  size,  size);
    SET(  size,  size, -size);
    SET( -size,  size, -size);

    // bottom plane
    SET( -size, -size, -size);
    SET(  size, -size, -size);
    SET(  size, -size,  size);
    SET( -size, -size,  size);

    // front plane
    SET(  size,  size,  size);
    SET( -size,  size,  size);
    SET( -size, -size,  size);
    SET(  size, -size,  size);

    // back plane
    SET( -size,  size, -size);
    SET(  size,  size, -size);
    SET(  size, -size, -size);
    SET( -size, -size, -size);
#undef SET
  }

  if (fvf.HasNormal())
  {
    verts.Reset();

#define SET(vx,vy,vz)                                                   \
    if (abCW)                                                           \
    { verts.Vertex().Normal().x = vx; verts.Vertex().Normal().y = vy; verts.Vertex().Normal().z = vz; } \
    else                                                                \
    { verts.Vertex().Normal().x = -(vx); verts.Vertex().Normal().y = -(vy); verts.Vertex().Normal().z = -(vz); } \
    verts.Next();

    // right plane
    SET(  1.0f,  0.0f,  0.0f);
    SET(  1.0f,  0.0f,  0.0f);
    SET(  1.0f,  0.0f,  0.0f);
    SET(  1.0f,  0.0f,  0.0f);

    // left plane (x = -1)
    SET( -1.0f,  0.0f,  0.0f);
    SET( -1.0f,  0.0f,  0.0f);
    SET( -1.0f,  0.0f,  0.0f);
    SET( -1.0f,  0.0f,  0.0f);

    // top plane (y = +1)
    SET(  0.0f,  1.0f,  0.0f);
    SET(  0.0f,  1.0f,  0.0f);
    SET(  0.0f,  1.0f,  0.0f);
    SET(  0.0f,  1.0f,  0.0f);

    // bottom plane
    SET(  0.0f, -1.0f,  0.0f);
    SET(  0.0f, -1.0f,  0.0f);
    SET(  0.0f, -1.0f,  0.0f);
    SET(  0.0f, -1.0f,  0.0f);

    // front plane
    SET(  0.0f,  0.0f,  1.0f);
    SET(  0.0f,  0.0f,  1.0f);
    SET(  0.0f,  0.0f,  1.0f);
    SET(  0.0f,  0.0f,  1.0f);

    // back plane Z = -1 (ok)
    SET(  0.0f,  0.0f, -1.0f);
    SET(  0.0f,  0.0f, -1.0f);
    SET(  0.0f,  0.0f, -1.0f);
    SET(  0.0f,  0.0f, -1.0f);
#undef SET
  }

  if (fvf.HasColorA())
  {
    for (verts.Reset(); !verts.End(); verts.Next())
      verts.Vertex().ColorA() = aulColor;
  }

  if (fvf.HasTexCoo(0))
  {
    verts.Reset();
#define SET(vx,vy) {                                                    \
      niLoop(k,fvf.GetNumTexCoos()) {                                    \
        if (fvf.GetTexCooDim(k) >= 2) {                                 \
          VecTransformCoord(verts.Vertex().TexCoo2(k), Vec2f(vx,vy), amtxUV); \
        }                                                               \
      }                                                                 \
      verts.Next();                                                     \
    }
    SET( 0.0f,  0.0f); SET( 1.0f,  0.0f); SET( 1.0f,  1.0f); SET( 0.0f,  1.0f); // right plane UV's
    SET( 0.0f,  0.0f); SET( 1.0f,  0.0f); SET( 1.0f,  1.0f); SET( 0.0f,  1.0f); // left plane UV's
    SET( 1.0f,  0.0f); SET( 1.0f,  1.0f); SET( 0.0f,  1.0f); SET( 0.0f,  0.0f); // top plane UV's
    SET( 0.0f,  1.0f); SET( 0.0f,  0.0f); SET( 1.0f,  0.0f); SET( 1.0f,  1.0f); // bottom plane UV's
    SET( 0.0f,  0.0f); SET( 1.0f,  0.0f); SET( 1.0f,  1.0f); SET( 0.0f,  1.0f); // front plane UV's
    SET( 0.0f,  0.0f); SET( 1.0f,  0.0f); SET( 1.0f,  1.0f); SET( 0.0f,  1.0f); // back plane UV's
#undef SET
  }

  pGeom->GetVertexArray()->Unlock();

  tIndex va, vb, vc;
  if (abCW)
  {
    va = 0;
    vb = 1;
    vc = 2;
  }
  else
  {
    vc = 0;
    vb = 1;
    va = 2;
  }

  tIndex* pIndices = (tIndex*)pGeom->GetIndexArray()->Lock(0,0,eLock_Normal);
  for (tU32 i = 0; i < 6; ++i)
  {
    tIndex j = i*4;

    pIndices[va] = j;
    pIndices[vb] = j+1;
    pIndices[vc] = j+3;

    pIndices[va+3] = j+1;
    pIndices[vb+3] = j+2;
    pIndices[vc+3] = j+3;

    pIndices += 6;
  }
  pGeom->GetIndexArray()->Unlock();
  pGeom->AddSubset(0,0,pGeom->GetIndexArray()->GetNumIndices(),0);
  return pGeom;
}

///////////////////////////////////////////////
//! Create a plane polygonal geometry.
iGeometry* cGraphics::CreateGeometryPolygonalPlane(tGeometryCreateFlags aFlags, tFVF aFVF,
                                                            ePlaneType aPlaneType, const sVec3f& avCenter,
                                                            tF32 afWidth, tF32 afHeight, tU32 aulNumDiv,
                                                            tBool abCW, tU32 aulColor, const sMatrixf& amtxUV)
{
  aulNumDiv = ni::Clamp(aulNumDiv,1,_knMaxGeomSubDiv);

  tU32 ulNumVertices = (aulNumDiv+1)*(aulNumDiv+1);
  tU32 ulNumIndices = (aulNumDiv*aulNumDiv)*6;
  tU32 i, j;

  iGeometry* pGeom = CreateGeometryPolygonal(ulNumVertices, ulNumIndices, aFlags, aFVF);
  if (!niIsOK(pGeom))
  {
    niError(_A("Can't create geometry."));
    return NULL;
  }

  cFVFDescription fvf(pGeom->GetVertexArray()->GetFVF());
  cFVFStream verts(&fvf, pGeom->GetVertexArray()->Lock(0,0,eLock_Normal), ulNumVertices);

  tF32 sizeX = afWidth/2.0f;
  tF32 sizeY = afHeight/2.0f;
  float fDelta = 2.0f/float(aulNumDiv);

  if (fvf.HasPosition())
  {
    verts.Reset();
#define SET(vx,vy,vz)                             \
    verts.Vertex().Position().x = vx+avCenter.x;  \
    verts.Vertex().Position().y = vy+avCenter.y;  \
    verts.Vertex().Position().z = vz+avCenter.z;  \
    verts.Next();

    if (aPlaneType == ePlaneType_XZ)
    {
      for (i = 0; i <= aulNumDiv; ++i)
      {
        for (j = 0; j <= aulNumDiv; ++j)
        {
          SET((-1.0f+(float(j)*fDelta))*sizeX, 0.0f, (-1.0f+(float(i)*fDelta))*sizeY);  // EDMOND
        }
      }
    }
    else if (aPlaneType == ePlaneType_YZ)
    {
      for (i = 0; i <= aulNumDiv; ++i)
      {
        for (j = 0; j <= aulNumDiv; ++j)
        {
          SET(0.0f, (-1.0f+(float(i)*fDelta))*sizeY, (-1.0f+(float(j)*fDelta))*sizeX);  // EDMOND
        }
      }
    }
    else //if (aPlaneType == ePlaneType_XY)
    {
      for (i = 0; i <= aulNumDiv; ++i)
      {
        for (j = 0; j <= aulNumDiv; ++j)
        {
          SET((-1.0f+(float(j)*fDelta))*sizeX, (-1.0f+(float(i)*fDelta))*sizeY, 0.0f);  // EDMOND
        }
      }
    }

#undef SET
  }

  if (fvf.HasNormal())
  {
    verts.Reset();

    tF32 h = abCW?-1.0f:1.0f;

    if (aPlaneType == ePlaneType_XZ)
    {
      for (verts.Reset(); !verts.End(); verts.Next())
        verts.Vertex().Normal().Set(0.0f,-h,0.0f);
    }
    else if (aPlaneType == ePlaneType_YZ)
    {
      for (verts.Reset(); !verts.End(); verts.Next())
        verts.Vertex().Normal().Set(-h,0.0f,0.0f);
    }
    else //if (aPlaneType == ePlaneType_XY)
    {
      for (verts.Reset(); !verts.End(); verts.Next())
        verts.Vertex().Normal().Set(0.0f,0.0f,h);
    }

#undef SET
  }

  if (fvf.HasColorA())
  {
    for (verts.Reset(); !verts.End(); verts.Next())
      verts.Vertex().ColorA() = aulColor;
  }

  if (fvf.HasTexCoo(0))
  {
    verts.Reset();
#define SET(vx,vy) {                                                    \
      niLoop(k,fvf.GetNumTexCoos()) {                                    \
        if (fvf.GetTexCooDim(k) >= 2) {                                 \
          VecTransformCoord(verts.Vertex().TexCoo2(k), Vec2f(vx,vy), amtxUV); \
        }                                                               \
      }                                                                 \
      verts.Next();                                                     \
    }
    for (i = 0; i <= aulNumDiv; ++i) {
      for (j = 0; j <= aulNumDiv; ++j) {
        SET(float(j)*fDelta*0.5f, 1.0f-float(i)*fDelta*0.5f);
      }
    }
#undef SET
  }

  pGeom->GetVertexArray()->Unlock();

  tIndex va, vb, vc;
  if (abCW)
  {
    va = 0;
    vb = 1;
    vc = 2;
  }
  else
  {
    vc = 0;
    vb = 1;
    va = 2;
  }

  tIndex* pIndices = (tIndex*)pGeom->GetIndexArray()->Lock(0,0,eLock_Normal);
  for (i = 0; i < aulNumDiv; ++i)
  {
    for (j = 0; j < aulNumDiv; ++j)
    {
      tU32 startvert = i*(aulNumDiv+1) + j;
      pIndices[va] = startvert;
      pIndices[vb] = startvert+aulNumDiv+1;
      pIndices[vc] = startvert+1;
      pIndices[va+3] = startvert+1;
      pIndices[vb+3] = startvert+aulNumDiv+1;
      pIndices[vc+3] = startvert+aulNumDiv+2;
      pIndices += 6;
    }
  }
  pGeom->GetIndexArray()->Unlock();
  pGeom->AddSubset(0,0,pGeom->GetIndexArray()->GetNumIndices(),0);
  return pGeom;
}

///////////////////////////////////////////////
iGeometry* __stdcall cGraphics::CreateGeometryPolygonalCylinder(
    tGeometryCreateFlags aFlags, tFVF aFVF,
    tF32 baseRadius, tF32 topRadius,
    tF32 height, tI32 slices, tI32 stacks,
    tBool abCW, tU32 aulColor, const sMatrixf& amtxUV,
    tBool abCap, tBool abCentered)
{
  slices = ni::Clamp(slices,3,_knMaxGeomSubDiv);
  stacks = ni::Clamp(stacks,1,_knMaxGeomSubDiv);
  return QuadricCylinder(this,aFlags,aFVF,baseRadius,topRadius,height,slices,stacks,abCW,aulColor,amtxUV,
                         abCap,abCentered);
}

///////////////////////////////////////////////
iGeometry* __stdcall cGraphics::CreateGeometryPolygonalSphere(tGeometryCreateFlags aFlags, tFVF aFVF,
                                                                       tF32 radius, tI32 slices, tI32 stacks,
                                                                       tBool abCW, tU32 aulColor, const sMatrixf& amtxUV)
{
  slices = ni::Clamp(slices,3,_knMaxGeomSubDiv);
  stacks = ni::Clamp(stacks,3,_knMaxGeomSubDiv);
  return QuadricSphere(this,aFlags,aFVF,radius,slices,stacks,abCW,aulColor,amtxUV);
}

///////////////////////////////////////////////
iGeometry* __stdcall cGraphics::CreateGeometryPolygonalGeosphere(tGeometryCreateFlags aFlags, tFVF aFVF,
                                                                          tF32 radius, tI32 slices,
                                                                          tBool abCW, tU32 aulColor, const sMatrixf& amtxUV)
{
  slices = ni::Clamp(slices,2,_knMaxGeomSubDiv);
  return GeoSphere(this,aFlags,aFVF,0,eFalse,radius,slices,abCW,aulColor,amtxUV);
}

///////////////////////////////////////////////
iGeometry* __stdcall cGraphics::CreateGeometryPolygonalDiskSweep(tGeometryCreateFlags aFlags, tFVF aFVF,
                                                                          tF32 innerRadius, tF32 outerRadius, tI32 slices, tI32 stacks, tF32 startAngle, tF32 sweepAngle,
                                                                          tBool abCW, tU32 aulColor, const sMatrixf& amtxUV)
{
  slices = ni::Clamp(slices,2,_knMaxGeomSubDiv);
  stacks = ni::Clamp(stacks,1,_knMaxGeomSubDiv);
  return QuadricDiskSweep(this,aFlags,aFVF,innerRadius,outerRadius,slices,stacks,startAngle,sweepAngle,abCW,aulColor,amtxUV);
}

///////////////////////////////////////////////
tBool __stdcall cGraphics::UpdateGeometryPolygonalDiskSweep(iGeometry* apGeom,
                                                            tF32 innerRadius, tF32 outerRadius, tI32 slices, tI32 stacks, tF32 startAngle, tF32 sweepAngle,
                                                            tBool abCW, tU32 aulColor, const sMatrixf& amtxUV)
{
  slices = ni::Clamp(slices,2,_knMaxGeomSubDiv);
  stacks = ni::Clamp(stacks,1,_knMaxGeomSubDiv);
  return QuadricDiskSweepEx(apGeom,innerRadius,outerRadius,slices,stacks,startAngle,sweepAngle,abCW,aulColor,amtxUV);
}

///////////////////////////////////////////////
iGeometry* __stdcall cGraphics::CreateGeometryPolygonalDisk(tGeometryCreateFlags aFlags, tFVF aFVF,
                                                                     tF32 innerRadius, tF32 outerRadius, tI32 slices, tI32 stacks,
                                                                     tBool abCW, tU32 aulColor, const sMatrixf& amtxUV)
{
  slices = ni::Clamp(slices,2,_knMaxGeomSubDiv);
  stacks = ni::Clamp(stacks,1,_knMaxGeomSubDiv);
  return QuadricDisk(this,aFlags,aFVF,innerRadius,outerRadius,slices,stacks,abCW,aulColor,amtxUV);
}
