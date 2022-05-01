#include "stdafx.h"
#include <niLang/STL/utils.h>
#include <niLang/STL/hash_map.h>
#include <niLang/STL/map.h>

// template class astl::map<ni::tInt,ni::tInt>;
// template class astl::hash_map<ni::tInt,ni::tInt>;

namespace ni {

ASTL_RAW_ALLOCATOR_IMPL(HashTable);

// In the end this is slightly slower than the STL's hash_map, I keep it here to avoid
// having some smart ass make a "faster" hash table... (like me :P)
// Also the current implementation if used for SQTable doesn't work very well, probably
// some issues with Erase, or something. Didn't go further tracking down the problem
// since its not faster than the hash_map...
template <typename tKey, typename tVal,
          typename tHashFun = astl::hash<tKey>,
          typename tEqFun = astl::equal_to<tKey> >
struct HashTable {
 private:
  HashTable& operator = (const HashTable&);

 public:
  static const tU32 knMinPower2 = 4;

  // typedef SQObjectPtr      tKey;
  // typedef SQObjectPtr      tVal;
  // typedef sObjectPtrHash   tHashFun;
  // typedef sObjectPtrEq     tEqFun;

  struct sHashNode : public astl::pair<tKey,tVal> {
    sHashNode* next;
    tBool used;
    sHashNode() {
      next = NULL;
      used = eFalse;
    }
  };
  typedef ASTL_ALLOCATOR(sHashNode,HashTable) tAlloc;

  sHashNode* mpFirstFree;
  sHashNode* mpNodes;
  tU32   mnNumNodes;
  tU32   mnUsedNodes;

  static inline const sHashNode& _NullNode() {
    static sHashNode _nullNode;
    return _nullNode;
  }

  static inline tBool _IsEqKey(const tKey& aLeft, const tKey& aRight) {
    return tEqFun()(aLeft,aRight);
  }
  inline tSize _HashKey(const tKey& aKey) const {
    return tHashFun()(aKey) & (mnNumNodes-1);
  }

  HashTable() {
    mnUsedNodes = 0;
    _AllocNodes(knMinPower2);
  }
  ~HashTable() {
    if (mpNodes) {
      _FreeNodes(mpNodes,mnNumNodes);
      mpNodes = NULL;
    }
  }

  static sHashNode* _AllocateNodes(tU32 anNumNodes) {
    sHashNode* nodes = (sHashNode*)tAlloc::raw_allocator::allocate(anNumNodes * sizeof(sHashNode));
    niLoop(i,anNumNodes) {
      new(&nodes[i]) sHashNode;
    }
    return nodes;
  }
  static void _FreeNodes(sHashNode* apNodes, tU32 anNumNodes) {
    niLoop(i,anNumNodes) {
      apNodes[i].~sHashNode();
    }
    tAlloc::raw_allocator::deallocate(apNodes,sizeof(sHashNode)*anNumNodes);
  }

  void _AllocNodes(tU32 anNumNodes) {
    mnNumNodes = anNumNodes;
    mpNodes = _AllocateNodes(anNumNodes);
    mpFirstFree = &mpNodes[mnNumNodes-1];
  }

  //! Resize the table
  void _Resize(tBool abForce) {
    sHashNode* const pPrevNodes = mpNodes;
    const tU32 nPrevSize = (mnNumNodes < knMinPower2) ? knMinPower2 : mnNumNodes;
    const tU32 nPrevUsedNodes = mnUsedNodes;
    // More that 3/4 ?
    if (nPrevUsedNodes >= (nPrevSize-(nPrevSize/4))) {
      _AllocNodes(nPrevSize*2);
    }
    // Less than 1/4 ?
    else if (nPrevUsedNodes < (nPrevSize/4) && (nPrevSize > knMinPower2)) {
      _AllocNodes(nPrevSize/2);
    }
    // Forced...
    else if (abForce) {
      _AllocNodes(nPrevSize);
    }
    // Don't change anything...
    else {
      return;
    }

    mnUsedNodes = 0;
    niLoop(i,nPrevSize) {
      sHashNode& n = pPrevNodes[i];
      if (n.used) {
        Insert(n.first,n.second);
      }
    }

    _FreeNodes(pPrevNodes,nPrevSize);
  }

  //! Return first used node at specified index
  sHashNode* NodeAt(tU32 anIndex) {
    while (anIndex < mnNumNodes) {
      if (mpNodes[anIndex].used) {
        return (sHashNode*)&mpNodes[anIndex];
      }
      ++anIndex;
    }
    return NULL;
  }

  //! Find node matching the specified key using the key's hash as starting point
  sHashNode* _FindNode(const tKey& aKey, tSize aHash) {
    niAssert(aHash < mnNumNodes);
    sHashNode* n = &mpNodes[aHash];
    do {
      if (_IsEqKey(n->first,aKey))
        return n;
      n = n->next;
    } while (n);
    return NULL;
  }

  //! Find node pointing to the specified key
  sHashNode* FindNode(const tKey& aKey) {
    return _FindNode(aKey,_HashKey(aKey));
  }

  //! Find node pointing to the specified key
  const sHashNode* FindNode(const tKey& aKey) const {
    return ((HashTable*)this)->_FindNode(aKey,_HashKey(aKey));
  }

  //! [] operator
  const tVal& operator [] (const tKey& aKey) const {
    return FindNode(aKey)->second;
  }
  tVal& operator [] (const tKey& aKey) {
    sHashNode* n = FindNode(aKey);
    if (!n) {
      n = Insert(aKey,tVal());
    }
    return n->second;
  }

  //! Erase the specifed key.
  tBool Erase(const tKey& aKey) {
    sHashNode* n = _FindNode(aKey,_HashKey(aKey));
    if (n) {
      niAssert(mnUsedNodes > 0);
      sHashNode* pNext = n->next;
      *n = _NullNode();
      n->next = pNext;
      --mnUsedNodes;
      _Resize(eFalse);
      return eTrue;
    }
    return eFalse;
  }

  //! Set existing value
  tBool Set(const tKey& aKey, const tVal& aVal) {
    tSize hash = _HashKey(aKey);
    sHashNode* n = _FindNode(aKey,hash);
    if (n) {
      n->first = aVal;
      return eTrue;
    }
    return eFalse;
  }

  //! Set existing value, or add it if it doesn't already exist
  sHashNode* Insert(const tKey& aKey, const tVal& aVal) {
    const tSize hash = _HashKey(aKey);
    sHashNode* n = _FindNode(aKey,hash);
    if (n) {
      n->second = aVal;
      return n;
    }

    sHashNode* pMainNode = &mpNodes[hash];
    n = pMainNode;

    // Key not found we'll add if the main position isn't free
    if (pMainNode->used) {
      n = mpFirstFree; // get a free node
      tSize mainNodeHash = _HashKey(pMainNode->first);
      sHashNode* pColNode = &mpNodes[mainNodeHash];
      if (pMainNode > n && pColNode != pMainNode) {
        // Move colliding node to a free node
        while (pColNode->next != pMainNode) {
          niAssert(pColNode->next != NULL);
          pColNode = pColNode->next;
        }
        // redo the chain with 'n' replacing 'pMainNode'
        pColNode->next = n;
        // copy the colliding node into the free position
        *n = *pMainNode;
        // now 'pMainNode' is free
        *pMainNode = _NullNode();
      }
      else {
        // new node goes in a free position
        n->next = pMainNode->next;
        pMainNode->next = n;
        pMainNode = n;
      }
    }
    pMainNode->first = aKey;
    pMainNode->used = eTrue;

    // Fix up 'first free'
    for (;;) {
      if (!mpFirstFree->used && mpFirstFree->next == NULL) {
        // Still has free space
        pMainNode->second = aVal;
        ++mnUsedNodes;
        return pMainNode;
      }
      else if (mpFirstFree == mpNodes) {
        // No more free node...
        break;
      }
      else {
        --mpFirstFree;
      }
    }

    // Cannot decrement anymore, resize and retry...
    _Resize(eTrue);
    return Insert(aKey,aVal);
  }

  //! Clear the content of the table
  void Clear() {
    niLoop(i,mnNumNodes) {
      mpNodes[i] = _NullNode();
    }
    mnUsedNodes = 0;
    _Resize(eTrue);
  }

  //! Returns the number of nodes used
  tU32 GetSize() const {
    return mnUsedNodes;
  }

  class iterator {
    HashTable* mpTable;
    tU32       mnIndex;
    friend struct HashTable;

   public:
    iterator(HashTable* apTable = NULL, tU32 anIndex = 0) : mpTable(apTable), mnIndex(anIndex) {}

    bool operator == (const iterator& rhs) const { return (mnIndex == rhs.mnIndex); }
    bool operator != (const iterator& rhs) const { return (mnIndex != rhs.mnIndex); }

    iterator& operator ++ () {
      ++mnIndex;
      return *this;
    }
    iterator operator ++ (int) {
      iterator temp = *this;
      ++mnIndex;
      return temp;
    }
    iterator& operator -- () {
      --mnIndex;
      return *this;
    }
    iterator operator -- (int) {
      iterator temp = *this;
      --mnIndex;
      return temp;
    }

    sHashNode* getHashNode() const {
      sHashNode* n = ((HashTable*)mpTable)->NodeAt(mnIndex);
      niAssert(n != NULL);
      return n;
    }
    sHashNode* operator -> () const {
      return getHashNode();
    }
  };

  typedef iterator const_iterator;

  iterator begin() const {
    return iterator((HashTable*)this,0);
  }
  iterator end() const {
    return iterator((HashTable*)this,mnUsedNodes);
  }

  size_t size() const {
    return GetSize();
  }
  bool empty() const {
    return mnUsedNodes == 0;
  }
  bool erase(iterator& it) {
    niAssert(it.mpTable == this);
    niAssert(it != end());
    return !!Erase(it.getHashNode()->first);
  }
  void clear() {
    this->Clear();
  }
};

}

//--------------------------------------------------------------------------------------------
//
//  Tests
//
//--------------------------------------------------------------------------------------------

const ni::tU32 kNumTests = 100;

typedef astl::map<ni::cString,ni::tU32> tMap;
typedef astl::hash_map<ni::cString,ni::tU32> tHMap;

template<typename MAP>
bool test_str_insert(MAP& map, const ni::tU32 numTest = kNumTests) {
  niLoop(i,numTest) {
    map[niFmt(_A("%d"),i)] = i;
  }
  return true;
}
template<typename MAP>
bool test_str_lookup(MAP& map) {
  ni::RandSeed(123);
  niLoop(i,kNumTests) {
    ni::tU32 k = ni::RandIntRange(0,(ni::tI32)kNumTests-1);
    ni::cString str = niFmt(_A("%d"),k);
    if (map[str] != k) {
      niDebugFmt((_A("[%s] Expected %d, got %d\n"),
                  str.Chars(),
                  k,map[str]));
      return false;
    }
  }
  return true;
}

struct ASTL_map {};

TEST_FIXTURE(ASTL_map,hashmap_str_time) {
  tHMap map;
  TEST_TIMING_BEGIN(Insert)
      CHECK_EQUAL(true,test_str_insert(map));
  TEST_TIMING_END()
      TEST_TIMING_BEGIN(Lookup)
      CHECK_EQUAL(true,test_str_lookup(map));
  TEST_TIMING_END()
      CHECK_EQUAL(10,map["10"]);
  CHECK_EQUAL(20,map["20"]);
  CHECK_EQUAL(30,map["30"]);
}

TEST_FIXTURE(ASTL_map,map_str_time) {
  tMap map;
  //  map.reserve(kNumTests);
  TEST_TIMING_BEGIN(Insert)
      CHECK_EQUAL(true,test_str_insert(map));
  TEST_TIMING_END()
      TEST_TIMING_BEGIN(Lookup)
      CHECK_EQUAL(true,test_str_lookup(map));
  TEST_TIMING_END()
      CHECK_EQUAL(10,map["10"]);
  CHECK_EQUAL(20,map["20"]);
  CHECK_EQUAL(30,map["30"]);
}

TEST_FIXTURE(ASTL_map,HashTable_str_time) {
  ni::HashTable<ni::cString,ni::tU32> map;
  TEST_TIMING_BEGIN(Insert)
      CHECK_EQUAL(true,test_str_insert(map));
  TEST_TIMING_END();
  TEST_TIMING_BEGIN(Lookup)
      CHECK_EQUAL(true,test_str_lookup(map));
  TEST_TIMING_END();
  CHECK_EQUAL(10,map["10"]);
  CHECK_EQUAL(20,map["20"]);
  CHECK_EQUAL(30,map["30"]);
}

TEST_FIXTURE(ASTL_map,map_insert) {
  astl::map<ni::cString,ni::tInt> map;
  astl::upsert(map, _ASTR("a"), 1);
  CHECK_EQUAL(1, map["a"]);
  astl::upsert(map, _ASTR("a"), 2);
  CHECK_EQUAL(2, map["a"]);
}


TEST_FIXTURE(ASTL_map,map_construct) {
  astl::map<ni::cString,ni::tInt> map = { {"a",1}, {"b",2} };
  CHECK_EQUAL(1, map["a"]);
  CHECK_EQUAL(2, map["b"]);
}

TEST_FIXTURE(ASTL_map,hmap_construct) {
  astl::hash_map<ni::cString,ni::tInt> hmap = { {"a",1}, {"b",2} };
  CHECK_EQUAL(1, hmap["a"]);
  CHECK_EQUAL(2, hmap["b"]);
}

TEST_FIXTURE(ASTL_map,get_default) {
  {
    astl::map<ni::cString,ni::tInt> map = { {"a",1}, {"b",2} };
    CHECK_EQUAL(1, map["a"]);
    CHECK_EQUAL(2, map["b"]);

    CHECK_EQUAL(1,astl::get_default(map,"a",ni::eInvalidHandle));
    CHECK_EQUAL(ni::eInvalidHandle,astl::get_default(map,"weee",ni::eInvalidHandle));
  }

  {
    astl::hash_map<ni::cString,ni::tInt> hmap = { {"a",1}, {"b",2} };
    CHECK_EQUAL(1, hmap["a"]);
    CHECK_EQUAL(2, hmap["b"]);

    CHECK_EQUAL(1,astl::get_default(hmap,"a",ni::eInvalidHandle));
    CHECK_EQUAL(ni::eInvalidHandle,astl::get_default(hmap,"weee",ni::eInvalidHandle));
  }

  {
    astl::map<ni::cString,ni::cString> map = { {"a","A"}, {"b","B"} };
    CHECK_EQUAL(_ASTR("A"), map["a"]);
    CHECK_EQUAL(_ASTR("B"), map["b"]);

    CHECK_EQUAL(_ASTR("A"),astl::get_default(map,"a",""));
    CHECK_EQUAL(_ASTR(""),astl::get_default(map,"weee",""));
  }

  {
    astl::hash_map<ni::cString,ni::tInt> hmap = { {"a",1}, {"b",2} };
    CHECK_EQUAL(1, hmap["a"]);
    CHECK_EQUAL(2, hmap["b"]);

    CHECK_EQUAL(1,astl::get_default(hmap,"a",ni::eInvalidHandle));
    CHECK_EQUAL(ni::eInvalidHandle,astl::get_default(hmap,"weee",ni::eInvalidHandle));
  }
}
