#ifndef ___BIMAP_H_6171C8AC_CB99_42B7_A6AA_8D4B5726798D__
#define ___BIMAP_H_6171C8AC_CB99_42B7_A6AA_8D4B5726798D__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "../Types.h"
#include "stl_alloc.h"
#include "map.h"

namespace astl {

ASTL_RAW_ALLOCATOR_IMPL(bimap);

template <typename FROM, typename TO,
          ASTL_TMPL_PARAM(FROMCMP, eastl::less<FROM>),
          ASTL_TMPL_PARAM(TOCMP, eastl::less<TO>) >
struct bimap
{
 private:
  typedef ASTL_PAIR_ALLOCATOR(const FROM,TO,bimap) _AllocFrom;
  typedef map<FROM,TO,FROMCMP,_AllocFrom> from;
  typedef ASTL_PAIR_ALLOCATOR(const TO,FROM,bimap) _AllocTo;
  typedef map<TO,FROM,TOCMP,_AllocTo> to;

 public:
  typedef niTypename from::iterator iterator_from;
  typedef niTypename to::iterator iterator_to;
  typedef niTypename from::const_iterator const_iterator_from;
  typedef niTypename to::const_iterator const_iterator_to;

  bimap() {
  }

  bimap(const from& aFromMap, const to& aToMap) {
    _from = aFromMap;
    _to = aToMap;
  }

	bimap& operator=(const bimap& r) {
    this->_from = r._from;
    this->_to = r._to;
		return *this;
	}

  void clear() {
    _from.clear();
    _to.clear();
  }

  size_t size() const {
    niAssert(_from.size() == _to.size());
    return _from.size();
  }

  void insert(const FROM& k, const TO& v) {
    niTypename from::iterator itFrom = _from.find(k);
    if (itFrom != _from.end()) {
      eraseFrom(itFrom);
    }
    else {
      niTypename to::iterator itTo = _to.find(v);
      if (itTo != _to.end()) {
        eraseTo(itTo);
      }
    }

    _from.insert(eastl::make_pair(k,v));
    _to.insert(eastl::make_pair(v,k));
  }

  void eraseFrom(const_iterator_from it) {
    niTypename to::iterator itTo = _to.find(it->second);
    niAssert(itTo != _to.end());
    _to.erase(itTo);
    _from.erase(*(niTypename from::iterator*)&it);
  }
  niTypename from::const_iterator findFrom(const FROM& k) const {
    return _from.find(k);
  }
  niTypename from::const_iterator beginFrom() const {
    return _from.begin();
  }
  niTypename from::const_iterator endFrom() const {
    return _from.end();
  }
  const TO& getFrom(const FROM& k) const {
    niTypename from::const_iterator it = findFrom(k);
    niAssert(it != _from.end());
    return it->second;
  }
  bool hasFrom(const FROM& k) const {
    niTypename from::const_iterator it = findFrom(k);
    return (it != _from.end());
  }

  void eraseTo(niTypename to::const_iterator it) {
    niTypename from::iterator itFrom = _from.find(it->second);
    niAssert(itFrom != _from.end());
    _from.erase(itFrom);
    _to.erase(*(niTypename to::iterator*)&it);
  }
  niTypename to::const_iterator findTo(const TO& v) const {
    return _to.find(v);
  }
  niTypename to::const_iterator beginTo() const {
    return _to.begin();
  }
  niTypename to::const_iterator endTo() const {
    return _to.end();
  }
  const FROM& getTo(const TO& v) const {
    niTypename to::const_iterator it = findTo(v);
    niAssert(it != _to.end());
    return it->second;
  }
  bool hasTo(const TO& v) const {
    niTypename to::const_iterator it = findTo(v);
    return (it != _to.end());
  }

  static const FROM& firstFrom(const_iterator_from it) {
    return it->first;
  }
  static const TO& secondFrom(const_iterator_from it) {
    return it->second;
  }
  static const FROM& firstTo(const_iterator_to it) {
    return it->second;
  }
  static const TO& secondTo(const_iterator_to it) {
    return it->first;
  }

 private:
  from _from;
  to _to;
};

}
#endif // ___BIMAP_H_6171C8AC_CB99_42B7_A6AA_8D4B5726798D__
