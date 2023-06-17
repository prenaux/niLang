#ifndef __NIUNITTEST_H_0D4C849D_D811_46D6_9666_27CB0CFFDAD3__
#define __NIUNITTEST_H_0D4C849D_D811_46D6_9666_27CB0CFFDAD3__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include <niLang.h>

/** \addtogroup niUnitTest
 * @{
 */

//--------------------------------------------------------------------------------------------
//
//  Config
//
//--------------------------------------------------------------------------------------------
#if defined _MSC_VER
#  ifndef _DEBUG
#    pragma warning(disable:4702)  // bogus unreachable code in release mode
#  endif
#endif

#if !defined TEST_NO_EXCEPTIONS
#  if defined niBuildType && !defined niNoExceptions
#    define TEST_NO_EXCEPTIONS 0
#  else
#    define TEST_NO_EXCEPTIONS 1
#  endif
#endif

#if TEST_NO_EXCEPTIONS == 0
#  define TEST_NITHROWASSERT
#  include <niLang/STL/exception.h>
//
// niUnitTest catches all exceptions. This has the disavantage of producing
// stack traces that originate in a signal/exception handlers instead of the
// actual origin of the crash so it disabled by default since it makes
// debugging more difficult.
//
// #define TEST_NICATCHALL
#  define TEST_TRY niTry
#  define TEST_CATCH niCatch
#  define TEST_CATCHALL niCatchAll
#else
#  define TEST_TRY
#  define TEST_CATCH(T,X) if(const T EXC = {}; 0)
#  define TEST_CATCHALL() if(0)
#endif

namespace UnitTest {

_HSymExport(unittest_assert);

static inline ni::cString GetTestInputFilePath(const ni::achar* fn) {
  ni::cString dir = ni::GetLang()->GetProperty("ni.dirs.data");
  dir += "tests/";
  if (!fn)
    return dir;
  return dir + fn;
}
static inline ni::cString GetTestOutputFilePath(const ni::achar* fn) {
  ni::cString dir = ni::GetLang()->GetProperty("ni.dirs.home");
  dir += "niApp/Tests/";
  ni::GetRootFS()->FileMakeDir(dir.Chars());
  if (!fn)
    return dir;
  return dir + fn;
}

}

#define TEST_FILEPATH(FILENAME) UnitTest::GetTestOutputFilePath(FILENAME).Chars()

//--------------------------------------------------------------------------------------------
//
//  Test Macros
//
//--------------------------------------------------------------------------------------------

#ifdef TEST
#  error "TEST macro already defined."
#endif
#ifdef TEST_FIXTURE
#  error "TEST_FIXTURE macro already defined."
#endif

//
// Macros to declare the parameters necessary to add to a classe that wants to use
// the CHECK_* macros
// Example :
//    class MyTestRun {
//         TEST_PARAMS_DECL;
//         int _x;
//         cMyTestRun(TEST_PARAMS_FUNC, int x) : TEST_PARAMS_CONS, _x(x) {}
//    }
//
//  TEST(Something) {
//    new MyTestRun(TEST_PARAMS_CALL);
//  }
//
#define TEST_PARAMS_DECL                                            \
  UnitTest::TestResults& testResults_;                              \
  char const* const m_testName;                                     \
  bool& m_timeReport;                                               \
  int& m_numSteps;

#define TEST_PARAMS_FUNC                                          \
  UnitTest::TestResults& testResults_,                            \
    char const* const m_testName,                                 \
    bool& m_timeReport,                                           \
    int& m_numSteps

#define TEST_PARAMS_CONS testResults_(testResults_), m_testName(m_testName), m_timeReport(m_timeReport), m_numSteps(m_numSteps)

#define TEST_PARAMS_CALL testResults_, m_testName, m_timeReport, m_numSteps

#define TEST_PARAMS_LAMBDA  &testResults_, m_testName = m_testName

#define TEST_PARAMS_LAMBDA_UNUSED niUnused(testResults_); niUnused(m_testName)

#define TEST_CONSTRUCTOR(NAME)                  \
  TEST_PARAMS_DECL;                             \
  NAME(TEST_PARAMS_FUNC) : TEST_PARAMS_CONS

#define TEST_CONSTRUCTOR_BASE(NAME,BASE)          \
  NAME(TEST_PARAMS_FUNC) : BASE(TEST_PARAMS_CALL)

#define TEST_DISABLED(NAME)                                     \
  void DisabledTest##NAME(UnitTest::TestResults& testResults_,  \
                          char const* const m_testName,         \
                          char const* const m_filename,         \
                          int const m_lineNumber,               \
                          bool m_timeConstraintExempt,          \
                          ni::tF64& m_timeStart,                \
                          bool m_timeReport,                    \
                          int m_numSteps                        \
                          )

#define TEST_EX_DISABLED(NAME,LIST) TEST_DISABLED(NAME)

#define TEST_EX(Name,List)                                            \
  class Test##Name : public UnitTest::Test                            \
  {                                                                   \
 public:                                                              \
    Test##Name() : Test(#Name, __FILE__, __LINE__) {}                 \
 private:                                                             \
    virtual void RunImpl(UnitTest::TestResults& testResults_) const;  \
  } test##Name##Instance;                                             \
  UnitTest::ListAdder adder##Name (List, &test##Name##Instance);      \
  void Test##Name::RunImpl(UnitTest::TestResults& testResults_) const

#define TEST(Name) TEST_EX(Name,UnitTest::Test::GetTestList())

#ifdef TEST_NITHROWASSERT
#  define TEST_CATCH_ASSERT_EXCEPTION(MSG)              \
  niCatch(ni::sPanicException,e) {                      \
    testResults_.OnTestFailure(                         \
      __FILE__, __LINE__, m_testName,                   \
      niFmt("sPanicException: %s: %s", MSG, e.what())); \
  }
#else
#  define TEST_CATCH_ASSERT_EXCEPTION(MSG)
#endif

#ifdef TEST_NICATCHALL
#  define TEST_CATCH_ALL_EXCEPTIONS(MSG)                                \
  TEST_CATCHALL() {                                                      \
    testResults_.OnTestFailure(__FILE__, __LINE__, m_testName, MSG);  \
  }
#else
#  define TEST_CATCH_ALL_EXCEPTIONS(MSG)
#endif

#define TEST_FIXTURE_EX(Fixture, Name, List)                            \
  struct Fixture##Name##Helper : public Fixture {                       \
    Fixture##Name##Helper(char const* testName) : m_testName(testName) {} \
    void RunTest(UnitTest::TestResults& testResults_,                   \
                 char const* const& m_testName,                         \
                 char const* const& m_filename,                         \
                 int const& m_lineNumber,                               \
                 bool& m_timeConstraintExempt,                          \
                 ni::tF64& m_timeStart,                                 \
                 bool& m_timeReport,                                    \
                 int& m_numSteps);                                      \
    char const* const m_testName;                                       \
   private:                                                             \
   Fixture##Name##Helper(Fixture##Name##Helper const&);                 \
   Fixture##Name##Helper& operator =(Fixture##Name##Helper const&);     \
  };                                                                    \
  class Test##Fixture##Name : public UnitTest::Test                     \
  {                                                                     \
   public:                                                              \
   Test##Fixture##Name() : Test(#Fixture "-" #Name, __FILE__, __LINE__, #Fixture) {} \
   private:                                                             \
   mutable Fixture##Name##Helper* _mt;                                  \
   virtual void BeforeRunImpl(UnitTest::TestResults& testResults_) const { \
     TEST_TRY {                                                          \
       _mt = new Fixture##Name##Helper(m_testName);                     \
     }                                                                  \
     TEST_CATCH_ASSERT_EXCEPTION("Assert exception in fixture constructor " #Fixture) \
     TEST_CATCH_ALL_EXCEPTIONS("Unhandled exception in fixture constructor " #Fixture)\
   }                                                                    \
   virtual void RunImpl(UnitTest::TestResults& testResults_) const  {   \
     TEST_TRY {                                                          \
       _mt->RunTest(testResults_,m_testName,m_filename,m_lineNumber,m_timeConstraintExempt,m_timeStart,m_timeReport,m_numSteps); \
     }                                                                  \
     TEST_CATCH_ASSERT_EXCEPTION("Assert exception in fixture " #Fixture) \
     TEST_CATCH_ALL_EXCEPTIONS("Unhandled exception in fixture " #Fixture) \
   }                                                                    \
   virtual void AfterRunImpl(UnitTest::TestResults& testResults_) const { \
     TEST_TRY {                                                          \
       delete _mt;                                                      \
       _mt = NULL;                                                      \
     }                                                                  \
     TEST_CATCH_ASSERT_EXCEPTION("Assert exception in fixture destructor " #Fixture) \
     TEST_CATCH_ALL_EXCEPTIONS("Unhandled exception in fixture destructor " #Fixture) \
   }                                                                    \
  } test##Fixture##Name##Instance;                                      \
  UnitTest::ListAdder adder##Fixture##Name (List, &test##Fixture##Name##Instance); \
  void Fixture##Name##Helper::RunTest(UnitTest::TestResults& testResults_, \
                                      char const* const& m_testName,    \
                                      char const* const& m_filename,    \
                                      int const& m_lineNumber,          \
                                      bool& m_timeConstraintExempt,     \
                                      ni::tF64& m_timeStart,            \
                                      bool& m_timeReport,               \
                                      int& m_numSteps)

#define TEST_FIXTURE(Fixture,Name) TEST_FIXTURE_EX(Fixture,Name,UnitTest::Test::GetTestList())


#define TEST_FIXTURE_DISABLED(FIXTURE,NAME)                             \
  void DisabledTest##FIXTURE##NAME(UnitTest::TestResults& testResults_, \
                                   char const* const m_testName,        \
                                   char const* const m_filename,        \
                                   int const m_lineNumber,              \
                                   bool m_timeConstraintExempt,         \
                                   ni::tF64& m_timeStart,               \
                                   bool m_timeReport,                   \
                                   int m_numSteps)

#define TEST_FIXTURE_EX_DISABLED(FIXTURE,NAME,LIST) TEST_FIXTURE_DISABLED(FIXTURE,NAME)

#define TEST_WIDGET_EX(Name, List)                                               \
class Test##Name : public UnitTest::Test                                         \
{                                                                                \
 public:                                                                         \
 Test##Name() : Test(#Name, __FILE__, __LINE__) {}                               \
 private:                                                                        \
 virtual void BeforeRunImpl(UnitTest::TestResults& testResults_) const {         \
   TEST_TRY {                                                                  \
     TEST_STEPS(5);                                                              \
     UnitTest::TestAppSetCurrentTestWidgetSink(                                  \
       niNew Name(TEST_PARAMS_CALL),                                             \
       TEST_IS_INTERACTIVE());                                                   \
   }                                                                             \
   TEST_CATCH_ASSERT_EXCEPTION("Assert exception in widget constructor " #Name)  \
   TEST_CATCH_ALL_EXCEPTIONS("Unhandled exception in widget constructor " #Name) \
 }                                                                               \
 virtual void RunImpl(UnitTest::TestResults& testResults_) const  {              \
   TEST_TRY {                                                                  \
   }                                                                             \
   TEST_CATCH_ASSERT_EXCEPTION("Assert exception in fixture " #Name)             \
   TEST_CATCH_ALL_EXCEPTIONS("Unhandled exception in fixture " #Name)            \
 }                                                                               \
 virtual void AfterRunImpl(UnitTest::TestResults& testResults_) const {          \
   TEST_TRY {                                                                  \
     UnitTest::TestAppSetCurrentTestWidgetSink(NULL,eFalse);                     \
   }                                                                             \
   TEST_CATCH_ASSERT_EXCEPTION("Assert exception in widget destructor " #Name)   \
   TEST_CATCH_ALL_EXCEPTIONS("Unhandled exception in widget destructor " #Name)  \
 }                                                                               \
} test##Name##Instance;                                                          \
UnitTest::ListAdder adder##Name (List, &test##Name##Instance);

#define TEST_WIDGET(Name) TEST_WIDGET_EX(Name,UnitTest::Test::GetTestList())

#define TEST_WIDGET_DISABLED(NAME)
#define TEST_WIDGET_EX_DISABLED(NAME,LIST)

#define TEST_FIXTURE_WIDGET_EX(Fixture, Name, List)                                  \
struct Fixture##Name##Helper : public Fixture {                                      \
  Fixture##Name##Helper(char const* testName) : m_testName(testName) {}              \
  char const* const m_testName;                                                      \
 private:                                                                            \
 Fixture##Name##Helper(Fixture##Name##Helper const&);                                \
 Fixture##Name##Helper& operator =(Fixture##Name##Helper const&);                    \
};                                                                                   \
class Test##Fixture##Name : public UnitTest::Test                                    \
{                                                                                    \
 public:                                                                             \
 Test##Fixture##Name() : Test(#Fixture "-" #Name, __FILE__, __LINE__, #Fixture) {}   \
 private:                                                                            \
 mutable Fixture##Name##Helper* _mt;                                                 \
 virtual void BeforeRunImpl(UnitTest::TestResults& testResults_) const {             \
   TEST_TRY {                                                                      \
     _mt = new Fixture##Name##Helper(m_testName);                                    \
   }                                                                                 \
   TEST_CATCH_ASSERT_EXCEPTION("Assert exception in fixture constructor " #Fixture)  \
   TEST_CATCH_ALL_EXCEPTIONS("Unhandled exception in fixture constructor " #Fixture) \
   TEST_TRY {                                                                      \
     TEST_STEPS(5);                                                                  \
     UnitTest::TestAppSetCurrentTestWidgetSink(                                      \
       niNew Name(TEST_PARAMS_CALL),                                                 \
       TEST_IS_INTERACTIVE());                                                       \
   }                                                                                 \
   TEST_CATCH_ASSERT_EXCEPTION("Assert exception in widget constructor " #Fixture)   \
   TEST_CATCH_ALL_EXCEPTIONS("Unhandled exception in widget constructor " #Fixture)  \
 }                                                                                   \
 virtual void RunImpl(UnitTest::TestResults& testResults_) const  {                  \
   TEST_TRY {                                                                      \
   }                                                                                 \
   TEST_CATCH_ASSERT_EXCEPTION("Assert exception in fixture " #Fixture)              \
   TEST_CATCH_ALL_EXCEPTIONS("Unhandled exception in fixture " #Fixture)             \
 }                                                                                   \
 virtual void AfterRunImpl(UnitTest::TestResults& testResults_) const {              \
   TEST_TRY {                                                                      \
     UnitTest::TestAppSetCurrentTestWidgetSink(NULL,eFalse);                         \
   }                                                                                 \
   TEST_CATCH_ASSERT_EXCEPTION("Assert exception in widget destructor " #Fixture)    \
   TEST_CATCH_ALL_EXCEPTIONS("Unhandled exception in widget destructor " #Fixture)   \
   TEST_TRY {                                                                      \
     delete _mt;                                                                     \
     _mt = NULL;                                                                     \
   }                                                                                 \
   TEST_CATCH_ASSERT_EXCEPTION("Assert exception in fixture destructor " #Fixture)   \
   TEST_CATCH_ALL_EXCEPTIONS("Unhandled exception in fixture destructor " #Fixture)  \
 }                                                                                   \
} test##Fixture##Name##Instance;                                                     \
UnitTest::ListAdder adder##Fixture##Name (List, &test##Fixture##Name##Instance);

#define TEST_FIXTURE_WIDGET(Fixture,Name) TEST_FIXTURE_WIDGET_EX(Fixture,Name,UnitTest::Test::GetTestList())

#define TEST_FIXTURE_WIDGET_DISABLED(FIXTURE,NAME)
#define TEST_FIXTURE_WIDGET_EX_DISABLED(FIXTURE,NAME,LIST)

#ifdef TEST_NITHROWASSERT
#  define TEST_THROW_ASSERT(DESC)  niThrowPanic(UnitTest,unittest_assert,DESC)
#  undef niAssert
#  undef niAssertMsg
#  undef niAssertUnreachable
#  define niAssert(exp)            if (!(exp)) { TEST_THROW_ASSERT("ASSERT: " _A(#exp)); }
#  define niAssertMsg(exp,msg)     if (!(exp)) { TEST_THROW_ASSERT("ASSERT: " _A(#exp) ": " msg); }
#  define niAssertUnreachable(...) TEST_THROW_ASSERT("ASSERT UNREACHABLE: " __VA_ARGS__);
#else
#  define TEST_THROW_ASSERT(DESC)
#endif

#define TEST_LOG_NORMAL   (1<<0)
#define TEST_LOG_ERROR    (1<<1)
#define TEST_LOG_DEBUG    (1<<2)
#define TEST_LOG_WARNING  (1<<3)
#define TEST_LOG_CONSOLE  (1<<4)
#define TEST_LOG_MEMORY   (1<<5)

#define TEST_UNUSED(X)    X;

#define TEST_TIMEREPORT()   m_timeReport = true;

#define TEST_INTERACTIVE_STEPS_COUNT 0xffffff
#define TEST_IS_INTERACTIVE() (m_numSteps == TEST_INTERACTIVE_STEPS_COUNT)

// run interactively when the passed fixture name is the same as the test name
#define TEST_STEPS(COUNT)                                             \
  if (UnitTest::runFixtureName.IEq(m_testName)) {                     \
    m_numSteps = TEST_INTERACTIVE_STEPS_COUNT;                        \
  }                                                                   \
  else {                                                              \
    m_numSteps = COUNT;                                               \
  }

#define TEST_PRINT(FMT)        {niPrintln(FMT);}

#define TEST_DEBUGFMT(STR,...) niDebugFmt(("[%s] " STR, m_testName, __VA_ARGS__))

#ifndef _A
#  define _A(X) X
#endif

#define TEST_TIMING_BLOCK_(NAME,FMTDATA,FMT)                            \
  struct sUnitTestTimingBlock_##NAME {                                  \
    void*             _fmtData;                                         \
    char const* const _testName;                                        \
    char const* const _Name;                                            \
    double        _startTime;                                           \
    sUnitTestTimingBlock_##NAME(char const* const aTestName, char const* const aName, void* aFmtData) : \
    _fmtData(aFmtData),                                                 \
      _testName(aTestName),                                             \
      _Name(aName),                                                     \
      _startTime(ni::TimerInSeconds()) {}                               \
    ~sUnitTestTimingBlock_##NAME() {                                    \
      double timing = ni::TimerInSeconds()-_startTime;                  \
      TEST_PRINT(niFmt FMT);                                            \
    }                                                                   \
  };  sUnitTestTimingBlock_##NAME _timing_##NAME(m_testName,#NAME,(void*)(FMTDATA));

#define TEST_TIMING_BLOCK(NAME) TEST_TIMING_BLOCK_(NAME,NULL,("[Timing-%s] [%s] %.5f secs\n",_testName,_Name,timing))

#define TEST_TIMING_BEGIN_(NAME,FMTDATA,FMT) {  \
  TEST_TIMING_BLOCK_(NAME,FMTDATA,FMT);

#define TEST_TIMING_BEGIN(NAME) TEST_TIMING_BEGIN_(NAME,NULL,("[Timing-%s] [%s] %.5f secs\n",_testName,_Name,timing))

#define TEST_TIMING_END() }
#define TEST_TIMING_END() }

#define ENTER_WARNING_MODE() ++testResults_.m_warningMode
#define LEAVE_WARNING_MODE() --testResults_.m_warningMode

struct sAutoWarningMode {
  int& _warningMode;
  sAutoWarningMode(int& warningMode) : _warningMode(warningMode) {
    ++_warningMode;
  }
  ~sAutoWarningMode() {
    --_warningMode;
  }
};

#define AUTO_WARNING_MODE() sAutoWarningMode __autoWarningMode(testResults_.m_warningMode)

struct UnitTestMemDelta {
  const char* _file;
  const int _line;
  const char* _func;
  ni::sVec4i _memStart;
  UnitTestMemDelta(const char* aFile, int aLine, const char* aFunc)
      : _file(aFile)
      , _line(aLine)
      , _func(aFunc)
  {
    ni_mem_get_stats(&_memStart);
  }
  ~UnitTestMemDelta() {
    EndTracking();
  }
  void EndTracking() {
    if (_memStart == ni::sVec4i::Zero())
      return;
    ni::sVec4i delta;
    ni_mem_get_stats(&delta);
    delta -= _memStart;
    ni::ni_log(
      ni::eLogFlags_Raw,
      niFmt("MemTracking delta: numAlloc: %d, numFree: %d, numObjectAlloc: %d, numObjectFree: %d",
            delta.x, delta.y, delta.z, delta.w),
      _file, _line, _func);
    _memStart = ni::sVec4i::Zero();
  }
};

#define TEST_TRACK_MEMORY_BEGIN() UnitTestMemDelta __memtrack(__FILE__,__LINE__,__FUNCTION__);
#define TEST_TRACK_MEMORY_END() __memtrack.EndTracking();

//--------------------------------------------------------------------------------------------
//
//  Check Macros
//
//--------------------------------------------------------------------------------------------

#ifdef CHECK
#  error UnitTest++ redefines CHECK
#endif

#define CHECK(value)                                                    \
  TEST_TRY {                                                             \
    if (!UnitTest::Check(value))                                        \
      testResults_.OnTestFailure(__FILE__, __LINE__, m_testName, #value); \
  }                                                                     \
  TEST_CATCH_ASSERT_EXCEPTION("Assert exception in CHECK(" #value ")")  \
  TEST_CATCH_ALL_EXCEPTIONS("Unhandled exception in CHECK(" #value ")")

#define CHECK_RETURN_IF_FAILED(value)                                   \
  TEST_TRY {                                                             \
    if (!UnitTest::Check(value)) {                                      \
      testResults_.OnTestFailure(__FILE__, __LINE__, m_testName, #value); \
      return;                                                           \
    }                                                                   \
  }                                                                     \
  TEST_CATCH_ASSERT_EXCEPTION("Assert exception in CHECK(" #value ")")  \
  TEST_CATCH_ALL_EXCEPTIONS("Unhandled exception in CHECK(" #value ")") \

#define CHECK_EQUAL(expected, actual)                                   \
  TEST_TRY {                                                             \
    UnitTest::CheckEqual(testResults_, "CHECK_EQUAL(" #expected ", " #actual ")", expected, actual, m_testName, __FILE__, __LINE__); \
  }                                                                     \
  TEST_CATCH_ASSERT_EXCEPTION("Assert exception in CHECK_EQUAL(" #expected ", " #actual ")") \
  TEST_CATCH_ALL_EXCEPTIONS("Unhandled exception in CHECK_EQUAL(" #expected ", " #actual ")")

#define CHECK_NOT_EQUAL(expected, actual)                               \
  TEST_TRY {                                                             \
    UnitTest::CheckNotEqual(testResults_, "CHECK_NOT_EQUAL(" #expected ", " #actual ")", expected, actual, m_testName, __FILE__, __LINE__); \
  }                                                                     \
  TEST_CATCH_ASSERT_EXCEPTION("Assert exception in CHECK_NOT_EQUAL(" #expected ", " #actual ")") \
  TEST_CATCH_ALL_EXCEPTIONS("Unhandled exception in CHECK_NOT_EQUAL(" #expected ", " #actual ")")

#define CHECK_CLOSE(expected, actual, tolerance)                        \
  TEST_TRY {                                                             \
    UnitTest::CheckClose(testResults_, expected, actual, tolerance, m_testName, __FILE__, __LINE__); \
  }                                                                     \
  TEST_CATCH_ASSERT_EXCEPTION("Assert exception in CHECK_CLOSE(" #expected ", " #actual ")") \
  TEST_CATCH_ALL_EXCEPTIONS("Unhandled exception in CHECK_CLOSE(" #expected ", " #actual ")")

#define CHECK_ARRAY_CLOSE(expected, actual, count, tolerance)           \
  TEST_TRY {                                                             \
    UnitTest::CheckArrayClose(testResults_, expected, actual, count, tolerance, m_testName, __FILE__, __LINE__); \
  }                                                                     \
  TEST_CATCH_ASSERT_EXCEPTION("Assert exception in CHECK_ARRAY_CLOSE(" #expected ", " #actual ")") \
  TEST_CATCH_ALL_EXCEPTIONS("Unhandled exception in CHECK_ARRAY_CLOSE(" #expected ", " #actual ")")

#define CHECK_THROW(expression, ExpectedExceptionType)                  \
  {                                                                     \
    bool caught_ = false;                                               \
    TEST_TRY { expression; }                                             \
    TEST_CATCH(ExpectedExceptionType,_) { caught_ = true; }          \
    if (!caught_) {                                                     \
      testResults_.OnTestFailure(__FILE__, __LINE__, m_testName, "Expected exception: \"" #ExpectedExceptionType "\" not thrown"); \
    }                                                                   \
  }

#define CHECK_THROW_ANY(expression)                                     \
  {                                                                     \
    bool caught_ = false;                                               \
    TEST_TRY { expression; }                                             \
    TEST_CATCHALL() { caught_ = true; }                                   \
    if (!caught_)                                                       \
      testResults_.OnTestFailure(__FILE__, __LINE__, m_testName, "Expected any exception, none thrown"); \
  }

#ifdef TEST_NITHROWASSERT
#  define CHECK_THROW_ASSERT(expression)                \
  CHECK_THROW(expression, const ni::sPanicException);
#else
#  define CHECK_THROW_ASSERT(expression)
#endif

#define CHECK_LOGERROR_BEGIN()  testResults_.PushLogErrors();
#define CHECK_LOGERROR_END(EXPECTED)  CHECK_EQUAL(EXPECTED,testResults_.GetLogErrorsDelta());
#define CHECK_LOGERROR_END_CLOSE(EXPECTED,DELTA)  CHECK_CLOSE(EXPECTED,testResults_.GetLogErrorsDelta(),DELTA);

#define CHECK_LOGWARNING_BEGIN()  testResults_.PushLogWarnings();
#define CHECK_LOGWARNING_END(EXPECTED,DELTA)  CHECK_EQUAL(EXPECTED,testResults_.GetLogWarningsDelta());
#define CHECK_LOGWARNING_END_CLOSE(EXPECTED,DELTA)  CHECK_CLOSE(EXPECTED,testResults_.GetLogWarningsDelta(),DELTA);

#define CHECK_PRED(expected, actual, name, pred)                        \
  TEST_TRY {                                                             \
    UnitTest::CheckPred(testResults_, expected, actual, m_testName, __FILE__, __LINE__, #name, pred); \
  }                                                                     \
  TEST_CATCH_ASSERT_EXCEPTION("Assert exception in CHECK_PRED(" #expected ", " #actual ", " #name ")") \
  TEST_CATCH_ALL_EXCEPTIONS("Unhandled exception in CHECK_PRED(" #expected ", " #actual ", " #name ")")

#define CHECK_LE(expected, actual)                                      \
  TEST_TRY {                                                             \
    UnitTest::CheckPred(testResults_, expected, actual, m_testName, __FILE__, __LINE__, \
                        "<=", [&](auto aLeft, auto aRight) {            \
                          return aLeft <= aRight;                       \
                        });                                             \
  }                                                                     \
  TEST_CATCH_ASSERT_EXCEPTION("Assert exception in CHECK_LE(" #expected ", " #actual ")") \
  TEST_CATCH_ALL_EXCEPTIONS("Unhandled exception in CHECK_LE(" #expected ", " #actual ")")

#define CHECK_LT(expected, actual)                                      \
  TEST_TRY {                                                             \
    UnitTest::CheckPred(testResults_, expected, actual, m_testName, __FILE__, __LINE__, \
                        "<", [&](auto aLeft, auto aRight) {             \
                          return aLeft < aRight;                        \
                        });                                             \
  }                                                                     \
  TEST_CATCH_ASSERT_EXCEPTION("Assert exception in CHECK_LT(" #expected ", " #actual ")") \
  TEST_CATCH_ALL_EXCEPTIONS("Unhandled exception in CHECK_LT(" #expected ", " #actual ")")

#define CHECK_GE(expected, actual)                                      \
  TEST_TRY {                                                             \
    UnitTest::CheckPred(testResults_, expected, actual, m_testName, __FIGE__, __LINE__, \
                        ">=", [&](auto aLeft, auto aRight) {            \
                          return aLeft >= aRight;                       \
                        });                                             \
  }                                                                     \
  TEST_CATCH_ASSERT_EXCEPTION("Assert exception in CHECK_GE(" #expected ", " #actual ")") \
  TEST_CATCH_ALL_EXCEPTIONS("Unhandled exception in CHECK_GE(" #expected ", " #actual ")")

#define CHECK_GT(expected, actual)                                      \
  TEST_TRY {                                                             \
    UnitTest::CheckPred(testResults_, expected, actual, m_testName, __FILE__, __LINE__, \
                        ">", [&](auto aLeft, auto aRight) {             \
                          return aLeft > aRight;                        \
                        });                                             \
  }                                                                     \
  TEST_CATCH_ASSERT_EXCEPTION("Assert exception in CHECK_GT(" #expected ", " #actual ")") \
  TEST_CATCH_ALL_EXCEPTIONS("Unhandled exception in CHECK_GT(" #expected ", " #actual ")")

//--------------------------------------------------------------------------------------------
//
//  TestList
//
//--------------------------------------------------------------------------------------------

namespace UnitTest {

class Test;

class TestList
{
 public:
  TestList();
  void Add (Test* test);

  const Test* GetHead() const;

 private:
  Test* m_head;
  Test* m_tail;
};


class ListAdder
{
 public:
  ListAdder(TestList& list, Test* test);
};

}

//--------------------------------------------------------------------------------------------
//
//  Test
//
//--------------------------------------------------------------------------------------------
namespace UnitTest {

class TestResults;

class Test
{
 public:
  Test(char const* testName, char const* filename = "",
       int lineNumber = 0, char const* fixtureName = "");
  virtual ~Test();
  bool BeforeRun(TestResults& testResults) const;
  bool Run(TestResults& testResults) const;
  bool AfterRun(TestResults& testResults) const;

  Test* next;
  char const* const m_testName;
  char const* const m_filename;
  char const* const m_fixtureName;
  int const m_lineNumber;
  mutable bool m_timeConstraintExempt;
  mutable ni::tF64 m_timeStart;
  mutable bool m_timeReport;
  mutable int m_numSteps;

  static TestList& GetTestList();

 private:
  virtual void BeforeRunImpl(TestResults& testResults_) const {}
  virtual void AfterRunImpl(TestResults& testResults_) const {}
  virtual void RunImpl(TestResults& testResults_) const {}

  // revoked
  Test(Test const&);
  Test& operator =(Test const&);
};

}

//--------------------------------------------------------------------------------------------
//
//  Test results
//
//--------------------------------------------------------------------------------------------

namespace UnitTest {

class TestReporter;

class TestResults
{
 public:
  explicit TestResults(TestReporter* reporter, int numTests);

  void OnTestStart(char const* testName);
  void OnCountLog(int type);
  void OnTestTime(char const* testName, float secondsElapsed);
  void OnTestFailure(char const* file, int line, char const* testName, char const* failure);
  void OnTestFinish(char const* testName, float secondsElapsed);

  int GetTestCount() const;
  int GetTestFailedCount() const;
  int GetTestWarnedCount() const;

  int GetTotalFailureCount() const;
  int GetTotalWarningCount() const;

  int GetLogErrors() const;
  int GetLogWarnings() const;
  void PushLogErrors();
  void PushLogWarnings();
  int GetLogErrorsDelta() const;
  int GetLogWarningsDelta() const;

 public:
  TestReporter* m_testReporter;
  const int m_numTests;
  int m_testCount;
  int m_testFailedCount;
  int m_testWarnedCount;
  int m_logErrors, m_logErrorsPush;
  int m_logWarnings, m_logWarningsPush;
  int m_totalFailureCount, m_totalFailureCountPush;
  int m_totalWarningCount, m_totalWarningCountPush;
  int m_warningMode;

 private:
  // revoked
  TestResults(TestResults const&);
  TestResults& operator =(TestResults const&);
};

}

//--------------------------------------------------------------------------------------------
//
//  Test reporter
//
//--------------------------------------------------------------------------------------------

namespace UnitTest {

class TestReporter
{
 public:
  virtual ~TestReporter() {}

  virtual void ReportTestStart(char const* testName) = 0;
  virtual void ReportTime(char const* testName, float secondsElapsed) = 0;
  virtual void ReportFailure(char const* file, int line, char const* testName, char const* failure) = 0;
  virtual void ReportWarning(char const* file, int line, char const* testName, char const* failure) = 0;
  virtual void ReportTestFinish(char const* testName, float secondsElapsed) = 0;
  virtual void ReportSummary(int testCount,
                             int failedCount, int totalFailedCount,
                             int warnedCount, int totalWarnedCount,
                             float secondsElapsed) = 0;
};

}

//--------------------------------------------------------------------------------------------
//
//  Test Reporter Stdout
//
//--------------------------------------------------------------------------------------------
namespace UnitTest {

class TestReporterStdout : public TestReporter
{
 private:
  virtual void ReportTestStart(char const* testName);
  virtual void ReportTime(char const* testName, float secondsElapsed);
  virtual void ReportFailure(char const* file, int line, char const* testName, char const* failure);
  virtual void ReportWarning(char const* file, int line, char const* testName, char const* failure);
  virtual void ReportTestFinish(char const* testName, float secondsElapsed);
  virtual void ReportSummary(int testCount,
                             int failedCount, int totalFailedCount,
                             int warnedCount, int totalWarnedCount,
                             float secondsElapsed);

  astl::vector<ni::cString> _warnings;
  astl::vector<ni::cString> _failures;
};

}

//--------------------------------------------------------------------------------------------
//
//  Time Constraint
//
//--------------------------------------------------------------------------------------------

namespace UnitTest {

class TestResults;


class TimeConstraint
{
 public:
  TimeConstraint(int ms, TestResults& result, char const* filename, int lineNumber,
                 char const* testName);
  ~TimeConstraint();

 private:
  void operator=(const TimeConstraint&);

  TestResults& m_result;
  int const m_maxMs;
  char const* const m_filename;
  int const m_lineNumber;
  char const* const m_testName;
  ni::tF64 m_timeStart;
};

#define TEST_TIME_CONSTRAINT(ms)    UnitTest::TimeConstraint t__(ms,testResults_,__FILE__,__LINE__,m_testName)
#define TEST_TIME_CONSTRAINT_EXEMPT() m_timeConstraintExempt = true

}

//--------------------------------------------------------------------------------------------
//
//  Checks
//
//--------------------------------------------------------------------------------------------
namespace UnitTest {


template< typename Value >
bool Check(Value const value)
{
  return !!value; // doing double negative to avoid silly VS warnings
}

template< typename Expected, typename Actual >
void CheckEqual(TestResults& results,
                char const* const msg,
                Expected const expected, Actual const actual,
                char const* const testName, char const* const filename, int const line)
{
#ifdef _MSC_VER
  // warning C4389: '==' : signed/unsigned mismatch
#  pragma warning( disable : 4389 )
#endif
  if (!(expected == actual))
  {
    ni::cString stream;
    stream << msg << ": Expected [" << expected << "] == [" << actual << "]";
    results.OnTestFailure(filename, line, testName, stream.c_str());
  }
#ifdef _MSC_VER
#  pragma warning( default : 4389 )
#endif
}

template< typename Expected, typename Actual >
void CheckNotEqual(TestResults& results,
                   char const* const msg,
                   Expected const expected, Actual const actual,
                   char const* const testName, char const* const filename, int const line)
{
#ifdef _MSC_VER
  // warning C4389: '==' : signed/unsigned mismatch
#  pragma warning( disable : 4389 )
#endif
  if (!(expected != actual))
  {
    ni::cString stream;
    stream << "Expected [" << expected << "] != [" << actual << "]";
    results.OnTestFailure(filename, line, testName, stream.c_str());
  }
#ifdef _MSC_VER
#  pragma warning( default : 4389 )
#endif
}

void CheckEqual(TestResults& results,
                char const* msg,
                char const* expected, char const* actual,
                char const* testName, char const* filename, int line);

void CheckNotEqual(TestResults& results,
                   char const* msg,
                   char const* expected, char const* actual,
                   char const* testName, char const* filename, int line);

template< typename Expected, typename Actual, typename Tolerance >
bool AreClose(Expected const expected, Actual const actual, Tolerance const tolerance)
{
  return (actual >= Actual(expected - tolerance)) && (actual <= Actual(expected + tolerance));
}

template< typename Expected, typename Actual, typename Tolerance >
void CheckClose(TestResults& results, Expected const expected, Actual const actual, Tolerance const tolerance,
                char const* const testName, char const* const filename, int const line)
{
  if (!AreClose(expected, actual, tolerance))
  {
    ni::cString stream;
    stream << "Expected [" << expected << "] {+/- " << tolerance << "} but was [" << actual << "]";
    results.OnTestFailure(filename, line, testName, stream.c_str());
  }
}


template< typename Expected, typename Actual >
void CheckArrayEqual(TestResults& results, Expected const expected, Actual const actual,
                     int const count, char const* const testName, char const* const filename, int const line)
{
  bool equal = true;
  for (int i = 0; i < count; ++i)
    equal &= (expected[i] == actual[i]);

  if (!equal)
  {
    ni::cString stream;
    stream << "Expected [ ";
    for (int i = 0; i < count; ++i)
      stream << expected[i] << " ";
    stream << "] but was [ ";
    for (int i = 0; i < count; ++i)
      stream << actual[i] << " ";
    stream << "]";
    results.OnTestFailure(filename, line, testName, stream.c_str());
  }
}

template< typename Expected, typename Actual, typename Tolerance >
void CheckArrayClose(TestResults& results, Expected const expected, Actual const actual,
                     int const count, Tolerance const tolerance, char const* const testName,
                     char const* const filename, int const line)
{
  bool equal = true;
  for (int i = 0; i < count; ++i)
    equal &= AreClose(expected[i], actual[i], tolerance);

  if (!equal)
  {
    ni::cString stream;
    stream << "Expected [ ";
    for (int i = 0; i < count; ++i)
      stream << expected[i] << " ";
    stream << "] +/- " << tolerance << " but was [ ";
    for (int i = 0; i < count; ++i)
      stream << actual[i] << " ";
    stream << "]";
    results.OnTestFailure(filename, line, testName, stream.c_str());
  }
}

template< typename Expected, typename Actual, typename Pred >
void CheckPred(TestResults& results, Expected const expected, Actual const actual,
               char const* const testName, char const* const filename, int const line,
               char const* const predName, Pred&& aPred)
{
#ifdef _MSC_VER
  // warning C4389: '==' : signed/unsigned mismatch
#  pragma warning( disable : 4389 )
#endif
  if (!aPred(expected,actual))
  {
    ni::cString stream;
    stream << "Expected [" << expected << "] " << predName << " [" << actual << "]";
    results.OnTestFailure(filename, line, testName, stream.c_str());
  }
#ifdef _MSC_VER
#  pragma warning( default : 4389 )
#endif
}

}

//--------------------------------------------------------------------------------------------
//
//  TestRunner
//
//--------------------------------------------------------------------------------------------
namespace ni {
struct iWidgetSink;
};
namespace app {
struct AppContext;
};

namespace UnitTest {

extern astl::vector<ni::cString> dataDirs;
extern ni::cString runFixtureName;

class TestReporter;
class TestList;

bool TestRunner_Startup(TestReporter& reporter,
                        TestList const& list,
                        int const maxTestTimeInMs,
                        char const* fixtureName);
bool TestRunner_Startup(char const* fixtureName);
void TestRunner_Shutdown();
void TestRunner_Reset();
bool TestRunner_RunNext();
const char* TestRunner_GetCurrentTestName();
int TestRunner_ReportSummary();

int RunAllTests(char const* fixtureName = NULL);
int RunAllTests(TestReporter& reporter,
                TestList const& list,
                int maxTestTimeInMs = 0,
                char const* fixtureName = NULL);

int TestAppNativeMainLoop(const char* aTitle, const char* aDefaultFixtureName);
int TestAppNativeMainLoop(const char* aTitle, int argc, const char** argv);
void TestAppSetCurrentTestWidgetSink(ni::iWidgetSink* apSink, ni::tBool abInteractive);

void TestLoop(TEST_PARAMS_FUNC, ni::Ptr<ni::iRunnable> aLoop, ni::Ptr<ni::iRunnable> aTestEnd);

astl::non_null<app::AppContext*> GetTestAppContext();

}

/**@}*/

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __NIUNITTEST_H_0D4C849D_D811_46D6_9666_27CB0CFFDAD3__
