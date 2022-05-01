#ifndef __RAWLIST_H_5EB6E79D_399B_409E_9A28_A7E1C1F176F0__
#define __RAWLIST_H_5EB6E79D_399B_409E_9A28_A7E1C1F176F0__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"

#ifdef __cplusplus
namespace ni {
#endif // __cplusplus

/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

typedef struct tListNode_tag
{
  struct tListNode_tag* next;
  struct tListNode_tag* previous;
} tListNode;
typedef tListNode* tListIt;

typedef struct tList_tag
{
  tType     type;
  tListNode sentinel;
} tList;

#define ni_list_begin(list) ((list) -> sentinel.next)
#define ni_list_end(list)   (&(list) -> sentinel)
#define ni_list_empty(list) (ni_list_begin (list) == ni_list_end (list))
#define ni_list_next(iterator) ((iterator) -> next)
#define ni_list_previous(iterator) ((iterator) -> previous)
#define ni_list_front(list) ((list) -> sentinel.next)
#define ni_list_back(list) ((list) -> sentinel.previous)
#define ni_list_push_back(list,data) ni_list_insert(ni_list_end(list),data)
#define ni_list_foreach(list,itname)            \
  tListIt itname;                               \
  for (itname = ni_list_begin(list);            \
       itname != ni_list_end(list);             \
       itname = ni_list_next(itname))
#define ni_list_foreachi(list,itname,iname)     \
  tListIt itname; size_t iname;                 \
  for (iname = 0, itname = ni_list_begin(list); \
       itname != ni_list_end(list);             \
       itname = ni_list_next(itname), ++iname)

static __forceinline tListIt ni_list_at(tList* list, size_t index) {
  ni_list_foreachi(list,it,i) {
    if (i == index) return it;
  }
  return ni_list_end(list);
}

static __forceinline void ni_list_init(tList* list, tType aType) {
  list->type = aType;
  list->sentinel.next = &list->sentinel;
  list->sentinel.previous = &list->sentinel;
}

static __forceinline void ni_list_clear(tList* list) {
  list->sentinel.next = &list->sentinel;
  list->sentinel.previous = &list->sentinel;
}

static __forceinline tListIt ni_list_insert(tListIt position, void* data) {
  tListIt result = (tListIt)data;
  result->previous = position->previous;
  result->next = position;
  result->previous->next = result;
  position->previous = result;
  return result;
}
static __forceinline void* ni_list_remove(tListIt position) {
  position->previous->next = position->next;
  position->next->previous = position->previous;
  return position;
}
static __forceinline size_t ni_list_size(tList* list) {
  size_t size = 0;
  tListIt position;
  for (position = ni_list_begin(list);
       position != ni_list_end(list);
       position = ni_list_next(position))
    ++size;
  return size;
}

#ifndef ni_tlist_malloc
#  define ni_tlist_malloc(TYPE) (TYPE*)niMalloc(sizeof(TYPE))
#  define ni_tlist_free(P) niFree(P)
#else
#  pragma message("## Using custom tList allocator.")
#endif

#define ni_tlist_insert(ctype,iterator,data) {  \
    ctype* v = ni_tlist_malloc(ctype);          \
    v->_data = data;                            \
    ni_list_insert(iterator,v);                 \
  }

#define ni_tlist_push_back(ctype,list,data) {   \
    ctype* v = ni_tlist_malloc(ctype);          \
    v->_data = data;                            \
    ni_list_push_back(list,v);                  \
  }

#define ni_tlist_remove(iterator) {             \
    void* data = ni_list_remove(iterator);      \
    ni_tlist_free(data);                        \
  }

#define ni_tlist_clear(list) {                  \
    while (ni_list_empty(list) == 0) {          \
      ni_tlist_remove(ni_list_begin(list)) ;    \
    }                                           \
  }

#define ni_tlist_data(ctype,iterator) ((ctype*)(iterator))->_data

#define ni_tlist_findr(rit,ctype,begin,end,data) {  \
    for (rit = begin;                               \
         rit != end;                                \
         rit = ni_list_next(rit))                   \
    {                                               \
      if (ni_tlist_data(ctype,rit) == data)         \
        break;                                      \
    }                                               \
  }

#define ni_tlist_find(rit,ctype,list,data) ni_tlist_findr(rit,ctype,ni_list_begin(list),ni_list_end(list),data)

/**@}*/
/**@}*/
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __RAWLIST_H_5EB6E79D_399B_409E_9A28_A7E1C1F176F0__
