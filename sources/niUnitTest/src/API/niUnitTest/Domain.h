#ifndef __DOMAIN_H_6B9CD619_D897_447F_BAC8_CE028C78AB0D__
#define __DOMAIN_H_6B9CD619_D897_447F_BAC8_CE028C78AB0D__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "UnitTest.h"
#include <niLang/Utils/Random.h>
#include <time.h>

namespace UnitTest {
namespace Domain {
/** \addtogroup niUnitTest
 * @{
 */

template <typename T, typename S>
struct IsEqual {
  bool operator () (const T& left, const S& right) const {
    return left == right;
  }
};

struct Generator_RandLargeI32 {
  Generator_RandLargeI32(ni::tI32 seed = clock()) {
    ni::RandSeed(seed);
  }
  ni::tI32 operator () (ni::tI32 i) const {
    return ni::RandInt();
  }
};

struct Generator_RandLargeU32 {
  Generator_RandLargeU32(ni::tI32 seed = clock()) {
    ni::RandSeed(seed);
  }
  ni::tU32 operator () (ni::tI32 i) const {
    return (ni::tU32)ni::RandInt();
  }
};

template <typename T, typename GEN, typename PREDICATE>
bool Check(ni::tI32 aStart, ni::tI32 aEnd,
           GEN& aGenerator, PREDICATE& aPredicate,
           TestResults& results,
           char const* const testName, char const* const filename, int const line) {
  for (ni::tI32 i = aStart; i <= aEnd; ++i) {
    T a = aGenerator(i);
    if (!aPredicate(a)) {
      UnitTest::MemoryOutStream stream;
      stream << "Domain::Check[" << aStart << "," << aEnd << "] at [" << i << "] predicate failed with [" << a << "]";
      results.OnTestFailure(filename, line, testName, stream.GetText());
      return false;
    }
  }
  return true;
}

template <typename T, typename GEN, typename PREDICATE>
bool Check2(ni::tI32 aStart, ni::tI32 aEnd,
            GEN& aGenerator, PREDICATE& aPredicate,
            TestResults& results,
            char const* const testName, char const* const filename, int const line) {
  for (ni::tI32 i = aStart; i <= aEnd; ++i) {
    T a = aGenerator(i);
    T b = aGenerator(i);
    if (!aPredicate(a,b)) {
      UnitTest::MemoryOutStream stream;
      stream << "Domain::Check2[" << aStart << "," << aEnd << "] at [" << i << "] predicate failed with [" << a << "] & [" << b << "]";
      results.OnTestFailure(filename, line, testName, stream.GetText());
      return false;
    }
  }
  return true;
}

#define CHECK_DOMAIN_(CHECK,START,END,TYPE,GENERATOR,PREDICATE)         \
  niTry() {                                                             \
    UnitTest::Domain::CHECK<TYPE>(                                      \
        START, END, GENERATOR, PREDICATE,                               \
        testResults_, m_testName, __FILE__, __LINE__);                  \
  }                                                                     \
  TEST_CATCH_ASSERT_EXCEPTION("Assert exception in CHECK_DOMAIN(" #GENERATOR ", " #PREDICATE ")") \
  TEST_CATCH_ALL_EXCEPTIONS("Unhandled exception in CHECK_DOMAIN(" #GENERATOR ", " #PREDICATE ")")

#define CHECK_DOMAIN(START,END,TYPE,GENERATOR,PREDICATE)  \
  CHECK_DOMAIN_(Check,START,END,TYPE,GENERATOR,PREDICATE)

#define CHECK2_DOMAIN(START,END,TYPE,GENERATOR,PREDICATE)   \
  CHECK_DOMAIN_(Check2,START,END,TYPE,GENERATOR,PREDICATE)

/**@}*/
}}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __DOMAIN_H_6B9CD619_D897_447F_BAC8_CE028C78AB0D__
