#ifndef __META_75732352_H__
#define __META_75732352_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

// Some fucking library author are retarded (found in GCC obviously...)
#undef T0
#undef T1
#undef B0
#undef B1

#define niMetaAssert(condition) typedef bool MetaAssert[2 * !!(condition) - 1]
#define niMetaAssert_(n,condition)  typedef bool MetaAssert##n[2 * !!(condition) - 1]

#define niMetaDeclareIsOfType(TYPE)                                     \
  template<class T> struct MetaIOT_##TYPE   { enum {res = false}; };    \
  template<>      struct MetaIOT_##TYPE<TYPE> { enum {res = true};  };

#define niMetaIsOfType(T,TYPE) MetaIOT_##TYPE<T >::res

template<class T>
struct MetaIsVoid
{
  enum {res = false};
};

template<>
struct MetaIsVoid<void>
{
  enum {res = true};
};

#define niMetaIsVoid(T) ni::MetaIsVoid<T >::res

template<bool>
struct MetaSelect
{
  template<typename T0, typename T1>
  struct Type
  {
    typedef T1 Res;
  };
};

template<>
struct MetaSelect<true>
{
  template <typename T0, typename T1>
  struct Type
  {
    typedef T0 Res;
  };
};

#define niMetaSelect(i, T0, T1) ni::MetaSelect<bool(i) >::template Type<T0,T1 >::Res

template<typename B0, typename B1>
struct MetaInherit : B0, B1
{
};

#define niMetaInherit(B0, B1) ni::MetaInherit<B0,B1 >

template<typename B0, typename B1>
class MetaCat
{
  typedef typename niMetaSelect(niMetaIsVoid(B0), B1, B0) T0;
  typedef typename niMetaSelect(niMetaIsVoid(B0), void, B1) T1;

 public:
  typedef typename niMetaInherit(T0, T1) T01;
  typedef typename niMetaSelect(niMetaIsVoid(T1), T0, T01) Res;

 private:
  typedef typename niMetaSelect(niMetaIsVoid(T1), int, T1) CheckedT1;

  niMetaAssert(niMetaIsVoid(T1) || sizeof(Res) == sizeof(T0) + sizeof(CheckedT1));
};

#define niMetaCat(B0, B1) ni::MetaCat<B0,B1 >::Res

template<bool cnd, typename B0, typename B1>
class MetaConditionalInherit
{
  typedef typename niMetaCat(B0, B1) MetaInherit;

 public:
  typedef typename niMetaSelect(cnd, MetaInherit, B0) Res;
};

#define niMetaConditionalInherit(cnd, B0, B1) ni::MetaConditionalInherit<(cnd),B0,B1 >::Res

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __META_75732352_H__
