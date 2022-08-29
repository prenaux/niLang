#pragma once
#ifndef __UTILS_H_D308314C_59E1_E345_B070_E0B4895E4773__
#define __UTILS_H_D308314C_59E1_E345_B070_E0B4895E4773__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "vector.h"
#include "pair.h"
#include "memory.h"
#include "EASTL/utility.h"
#include "EASTL/algorithm.h"
#include "EASTL/functional.h"
#include "EASTL/numeric.h"

namespace eastl {

ASTL_TEMPLATE_NULL
struct hash<const ni::iUnknown*> {
  size_t operator()(const ni::iUnknown* v) const {
    return size_t(v);
  }
};

}

namespace astl {
/** \addtogroup niSTL
 * @{
 */

using eastl::accumulate;
using eastl::adjacent_find;
using eastl::advance;
using eastl::binary_function;
using eastl::bind2nd;
using eastl::copy;
using eastl::equal_to;
using eastl::find;
using eastl::find_if;
using eastl::for_each;
using eastl::forward;
using eastl::function;
using eastl::hash;
using eastl::less;
using eastl::max;
using eastl::max_element;
using eastl::min;
using eastl::min_element;
using eastl::move;
using eastl::reverse;
using eastl::set_difference;
using eastl::set_intersection;
using eastl::set_union;
using eastl::swap;
using eastl::transform;
using eastl::unique;
using eastl::remove_if;
using eastl::equal;
using eastl::identical;
using eastl::search;

template <typename T>
struct hash_cast {
  size_t operator()(T __x) const { return (size_t)__x; }
};

struct dangling_iterator {};

template <typename T>
typename T::const_iterator find(const T& container, const typename T::value_type& v) {
  return eastl::find(container.begin(), container.end(), v);
}

template <typename T>
typename T::iterator find(T& container, const typename T::value_type& v) {
  return eastl::find(container.begin(), container.end(), v);
}

template <typename T>
dangling_iterator find(T&& container, const typename T::value_type& v) {
  return dangling_iterator();
}

template <typename T>
void fill(T& container, const typename T::value_type& v) {
  eastl::fill(container.begin(), container.end(), v);
}

template <typename T, class... ARGS>
typename T::value_type& emplace_back(T& container, ARGS&&... args) {
  container.emplace_back(eastl::forward<ARGS>(args)...);
  return container.back();
}

template <typename T, typename U>
typename T::value_type& push_back(T& container, const U& v) {
  container.push_back(v);
  return container.back();
}

template <typename T>
typename T::value_type& push_back(T& container) {
  container.emplace_back();
  return container.back();
}

//! Invalidate all pointers contained in a container
template <typename T>
void clear_invalidate(T& t)
{
  if (t.empty()) return;
  for (typename T::iterator it = t.begin(); it != t.end(); ++it)
  {
    if (niIsOK(*it)) {
      (*it)->Invalidate();
    }
  }
  t.clear();
}

//! Erase the specified iterator in map-type container.
template <typename T, typename S>
S map_erase(T& map, S it) {
  S nextIt = it; ++nextIt;
  map.erase(it);
  return nextIt;
}

//! Erase a found object.
template <typename T, typename U>
bool map_find_erase(T& container, U v)
{
  if (container.empty()) return false;
  typename T::iterator it = container.find(v);
  if (it == container.end()) return false;
  container.erase(it);
  return true;
}

//! Erase a found object.
template <typename T, typename U>
bool find_erase(T& container, U v)
{
  if (container.empty()) return false;
  for (typename T::iterator it = container.begin(); it != container.end(); ++it)
  {
    if (*it == v) {
      container.erase(it);
      return true;
    }
  }
  return false;
}

//! Erase a found object and invalidate.
template <typename T, typename U>
bool find_erase_invalidate(T& container, U v)
{
  if (container.empty()) return false;
  for (typename T::iterator it = container.begin(); it != container.end(); ++it)
  {
    if (*it == v) {
      if (niIsOK(*it)) {
        (*it)->Invalidate();
      }
      container.erase(it);
      return true;
    }
  }
  return false;
}

//! Insert/add or update/set map like container.
template <typename MapT, typename KeyArgT, typename ValueArgT>
typename MapT::iterator upsert(MapT& m, const KeyArgT& k, const ValueArgT& v) {
  return m.insert_or_assign(k,v).first;
}

//! Push back an object in a container only if it's not already in the container.
//! \return false if the object was already in the container, true if not and that
//!     the object has been added.
template <typename T, typename U>
bool push_back_once(T& container, U& v)
{
  for (typename T::iterator it = container.begin(); it != container.end(); ++it)
  {
    if (*it == v)
      return false;
  }
  container.push_back(v);
  return true;
}

//! Push front an object in a container only if it's not already in the container.
//! \return false if the object was already in the container, true if not and that
//!     the object has been added.
template <typename T, typename U>
bool push_front_once(T& container, U& v)
{
  for (typename T::iterator it = container.begin(); it != container.end(); ++it)
  {
    if (*it == v)
      return false;
  }
  container.push_front(v);
  return true;
}

//! Get the index of the given iterator.
//! \remark This works only for vectors.
template <typename T>
ni::tSize iterator_index(T& v, niTypename T::iterator it)
{
  return (ni::tSize)(it - v.begin());
}

//! Get the index of the given iterator.
//! \remark This works only for vectors.
template <class T>
ni::tSize const_iterator_index(const T& v, niTypename T::const_iterator it)
{
  return (ni::tSize)(it - v.begin());
}

//! Get the index of the given reverse_iterator.
//! \remark This works only for vectors.
template <class T>
ni::tSize reverse_iterator_index(T& v, niTypename T::reverse_iterator it)
{
  return (ni::tSize)(v.size()-(it-v.rbegin()));
}

//! Get the index of the given const_reverse_iterator.
//! \remark This works only for vectors.
template <class T>
ni::tSize const_reverse_iterator_index(const T& v, niTypename T::const_reverse_iterator it)
{
  return (ni::tSize)(v.size()-(it-v.rbegin()));
}

//! Check whether the specified container contains the specified value. (based on astl::find)
template <typename T, typename U>
bool contains(const T& container, const U& v) {
  return astl::find(container,v) != container.end();
}

template <typename T>
niTypename T::iterator cadvance(niTypename T::iterator it, size_t n) {
  eastl::advance(it, n);
  return it;
}

template <typename T>
void insert_at(T& container, unsigned int idx, const typename T::value_type& val) {
  container.insert(container.begin()+idx,val);
}

template <typename T>
void remove_at(T& container, unsigned int idx) {
  container.erase(container.begin()+idx);
}

//! Get a value in a map like container, return aDefault if it can't be found.
template <typename T>
typename T::mapped_type get_default(const T& aContainer, const typename T::key_type& aKey, const typename T::mapped_type& aDefaultValue) {
  typename T::const_iterator it = aContainer.find(aKey);
  if (it == aContainer.end())
    return aDefaultValue;
  return it->second;
}

//! Get a value in at the specified index in an int addressable container, return aDefault if the index isn't whitin range.
template <typename T>
typename T::value_type at_default(const T& aContainer, const size_t aIndex, const typename T::value_type& aDefaultValue) {
  if (aIndex >= aContainer.size())
    return aDefaultValue;
  return aContainer[aIndex];
}

template <typename T, typename FUN>
ni::tSize erase_if(T& container, const FUN& pred) {
  const ni::tSize wasSize = container.size();
  typename T::iterator remit = eastl::remove_if(
    container.begin(), container.end(), pred);
  if (remit != container.end()) {
    container.erase(remit, container.end());
  }
  return wasSize - container.size();
}

/**@}*/
}; // End of astl
#endif // __UTILS_H_D308314C_59E1_E345_B070_E0B4895E4773__
