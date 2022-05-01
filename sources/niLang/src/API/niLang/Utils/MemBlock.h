#ifndef __MEMBLOCK_H_7F246608_8D8A_4F2A_96F1_6BB6C98BC677__
#define __MEMBLOCK_H_7F246608_8D8A_4F2A_96F1_6BB6C98BC677__
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */


#ifndef __JSCC__
#include "RawList.h"

namespace ni {

niExportFunc(tList*) ni_getMemBlockList();

static const tU32 kfccMemTag = niFourCC('M','T','A','G');

struct sMemBlock : public tListNode {
  tU32        header;
  tU32        size;
  tPtr        p;
};

static inline sMemBlock* MemBlockListBegin(tList* apLst) {
  return (sMemBlock*)ni_list_begin(apLst);
}
static inline sMemBlock* MemBlockListEnd(tList* apLst) {
  return (sMemBlock*)ni_list_end(apLst);
}
static inline sMemBlock* MemBlockListNext(sMemBlock* apIt) {
  return (sMemBlock*)ni_list_next(apIt);
}
static inline tU32 MemBlockListSize(tList* apLst) {
  tU32 s = 0;
  for (sMemBlock* n = MemBlockListBegin(apLst); n != MemBlockListEnd(apLst); n = MemBlockListNext(n))
    ++s;
  return s;
}
static inline tBool MemBlockListEmpty(tList* apLst) {
  return apLst == NULL || ni_list_empty(apLst);
}

}
#endif

/**@}*/
/**@}*/
#endif // __MEMBLOCK_H_7F246608_8D8A_4F2A_96F1_6BB6C98BC677__
