// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niUnitTest.h"
#include "../../niLang/src/API/niLang/Utils/URLFileHandlerManifest.h"

#ifdef niWindows
#  ifdef niMSVC
//#define USE_SEH
#    include <windows.h>
#    include <winbase.h>
#  elif !defined __JSCC__
#    define USE_SIGNALS
#    include <signal.h>
#  endif
#elif defined niUnix
#  if !defined __JSCC__
#    define USE_SIGNALS
#    include <signal.h>
#    include <setjmp.h>
#  endif
#  include <stdio.h>
#endif

//--------------------------------------------------------------------------------------------
//
//  AssertException
//
//--------------------------------------------------------------------------------------------
#ifdef TEST_NITHROWASSERT
namespace UnitTest {

AssertException::AssertException(char const* description, char const* filename, int const lineNumber)
    : m_lineNumber(lineNumber)
{
  strcpy(m_description, description);
  strcpy(m_filename, filename);
}

AssertException::~AssertException() niThrowSpec()
{
}

char const* AssertException::what() const niThrowSpec()
{
  return m_description;
}

char const* AssertException::Filename() const
{
  return m_filename;
}

int AssertException::LineNumber() const
{
  return m_lineNumber;
}

}
#endif

//--------------------------------------------------------------------------------------------
//
//  SEH
//
//--------------------------------------------------------------------------------------------
#ifdef USE_SEH
namespace UnitTest {
class SEHException : public astl::exception
{
 public:
  SEHException(const char* desc) {
    strncpy(m_desc,desc,sizeof(m_desc)/sizeof(char));
  }
  virtual ~SEHException() {
  }
  const char* what() const {
    return m_desc;
  }
  char m_desc[1024];

  static inline int _Handle(DWORD excode)
  {
    switch(excode) {
      case EXCEPTION_ACCESS_VIOLATION:
      case EXCEPTION_DATATYPE_MISALIGNMENT:
      case EXCEPTION_BREAKPOINT:
      case EXCEPTION_SINGLE_STEP:
      case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
      case EXCEPTION_FLT_DENORMAL_OPERAND:
      case EXCEPTION_FLT_DIVIDE_BY_ZERO:
      case EXCEPTION_FLT_INEXACT_RESULT:
      case EXCEPTION_FLT_INVALID_OPERATION:
      case EXCEPTION_FLT_OVERFLOW:
      case EXCEPTION_FLT_STACK_CHECK:
      case EXCEPTION_FLT_UNDERFLOW:
      case EXCEPTION_INT_DIVIDE_BY_ZERO:
      case EXCEPTION_INT_OVERFLOW:
      case EXCEPTION_PRIV_INSTRUCTION:
      case EXCEPTION_IN_PAGE_ERROR:
      case EXCEPTION_ILLEGAL_INSTRUCTION:
      case EXCEPTION_NONCONTINUABLE_EXCEPTION:
      case EXCEPTION_STACK_OVERFLOW:
      case EXCEPTION_INVALID_DISPOSITION:
      case EXCEPTION_GUARD_PAGE:
      case EXCEPTION_INVALID_HANDLE:
        return EXCEPTION_EXECUTE_HANDLER;
    };
    return EXCEPTION_CONTINUE_SEARCH;
  }
  static inline const char* _Desc(DWORD excode)
  {
    switch(excode) {
      case EXCEPTION_ACCESS_VIOLATION: return "EXCEPTION_ACCESS_VIOLATION";
      case EXCEPTION_DATATYPE_MISALIGNMENT: return "EXCEPTION_DATATYPE_MISALIGNMENT";
      case EXCEPTION_BREAKPOINT: return "EXCEPTION_BREAKPOINT";
      case EXCEPTION_SINGLE_STEP: return "EXCEPTION_SINGLE_STEP";
      case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
      case EXCEPTION_FLT_DENORMAL_OPERAND: return "EXCEPTION_FLT_DENORMAL_OPERAND";
      case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
      case EXCEPTION_FLT_INEXACT_RESULT: return "EXCEPTION_FLT_INEXACT_RESULT";
      case EXCEPTION_FLT_INVALID_OPERATION: return "EXCEPTION_FLT_INVALID_OPERATION";
      case EXCEPTION_FLT_OVERFLOW: return "EXCEPTION_FLT_OVERFLOW";
      case EXCEPTION_FLT_STACK_CHECK: return "EXCEPTION_FLT_STACK_CHECK";
      case EXCEPTION_FLT_UNDERFLOW: return "EXCEPTION_FLT_UNDERFLOW";
      case EXCEPTION_INT_DIVIDE_BY_ZERO: return "EXCEPTION_INT_DIVIDE_BY_ZERO";
      case EXCEPTION_INT_OVERFLOW: return "EXCEPTION_INT_OVERFLOW";
      case EXCEPTION_PRIV_INSTRUCTION: return "EXCEPTION_PRIV_INSTRUCTION";
      case EXCEPTION_IN_PAGE_ERROR: return "EXCEPTION_IN_PAGE_ERROR";
      case EXCEPTION_ILLEGAL_INSTRUCTION: return "EXCEPTION_ILLEGAL_INSTRUCTION";
      case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
      case EXCEPTION_STACK_OVERFLOW: return "EXCEPTION_STACK_OVERFLOW";
      case EXCEPTION_INVALID_DISPOSITION: return "EXCEPTION_INVALID_DISPOSITION";
      case EXCEPTION_GUARD_PAGE: return "EXCEPTION_GUARD_PAGE";
      case EXCEPTION_INVALID_HANDLE: return "EXCEPTION_INVALID_HANDLE";
    };
    return "EXCEPTION_UNKNOWN";
  }
};
}
#endif

//--------------------------------------------------------------------------------------------
//
//  Report Assert
//
//--------------------------------------------------------------------------------------------
namespace UnitTest {

void ReportAssert(char const* description, char const* filename, int const lineNumber)
{
#ifdef TEST_NITHROWASSERT
  niThrow(AssertException(description, filename, lineNumber));
#endif
}

}

//--------------------------------------------------------------------------------------------
//
//  MemoryOutStream
//
//--------------------------------------------------------------------------------------------
namespace UnitTest {

template<typename ValueType>
inline void FormatToStream(MemoryOutStream& stream, char const* format, ValueType const& value) {
  stream << niFmt(format, value);
}

inline int RoundUpToMultipleOfPow2Number(int n, int pow2Number) {
  return (n + (pow2Number - 1)) & ~(pow2Number - 1);
}

MemoryOutStream::MemoryOutStream(int const size)
    : m_capacity (0)
    , m_buffer (0)

{
  GrowBuffer(size);
}

MemoryOutStream::~MemoryOutStream()
{
  delete [] m_buffer;
}

char const* MemoryOutStream::GetText() const
{
  return m_buffer;
}

MemoryOutStream& MemoryOutStream::operator << (char const* txt)
{
  if (!txt || !*txt) return *this;
  int const bytesLeft = m_capacity - (int)strlen(m_buffer);
  int const bytesRequired = (int)strlen(txt) + 1;
  if (bytesRequired > bytesLeft) {
    int const requiredCapacity = bytesRequired + m_capacity - bytesLeft;
    GrowBuffer(requiredCapacity);
  }
  strcat(m_buffer, txt);
  return *this;
}

#if defined niTypeIntIsOtherType
MemoryOutStream& MemoryOutStream::operator << (signed int const n)
{
  FormatToStream(*this, "%i", n);
  return *this;
}
MemoryOutStream& MemoryOutStream::operator << (unsigned int const n)
{
  FormatToStream(*this, "%u", n);
  return *this;
}
#endif

#if defined niTypeIntPtrIsOtherType
MemoryOutStream& MemoryOutStream::operator << (ni::tIntPtr const n)
{
  FormatToStream(*this, "%i", n);
  return *this;
}
MemoryOutStream& MemoryOutStream::operator << (ni::tUIntPtr const n)
{
  FormatToStream(*this, "%u", n);
  return *this;
}
#endif

MemoryOutStream& MemoryOutStream::operator << (void const* p)
{
  FormatToStream(*this, "0x%p", p);
  return *this;
}

MemoryOutStream& MemoryOutStream::operator << (ni::tI8 const n) { FormatToStream(*this, "%i", n); return *this; }
MemoryOutStream& MemoryOutStream::operator << (ni::tU8 const n) { FormatToStream(*this, "%i", n); return *this; }
MemoryOutStream& MemoryOutStream::operator << (ni::tI16 const n)  { FormatToStream(*this, "%i", n); return *this; }
MemoryOutStream& MemoryOutStream::operator << (ni::tU16 const n)  { FormatToStream(*this, "%i", n); return *this; }
MemoryOutStream& MemoryOutStream::operator << (ni::tI32 const n)  { FormatToStream(*this, "%d", n); return *this; }
MemoryOutStream& MemoryOutStream::operator << (ni::tU32 const n)  { FormatToStream(*this, "%d", n); return *this; }
MemoryOutStream& MemoryOutStream::operator << (ni::tI64 const n)  { FormatToStream(*this, "%lld", n); return *this; }
MemoryOutStream& MemoryOutStream::operator << (ni::tU64 const n)  { FormatToStream(*this, "%llu", n); return *this; }
MemoryOutStream& MemoryOutStream::operator << (ni::tF32 const n)  { FormatToStream(*this, "%g", n); return *this; }
MemoryOutStream& MemoryOutStream::operator << (ni::tF64 const n)  { FormatToStream(*this, "%g", n); return *this; }
MemoryOutStream& MemoryOutStream::operator << (const ni::cString& n)  { *this << n.Chars(); return *this; }
MemoryOutStream& MemoryOutStream::operator << (const ni::tUUID& n)  { *this << ni::cString(n).Chars(); return *this;  }
#ifdef __REALCAPI_H__
MemoryOutStream& MemoryOutStream::operator << (const tRealNumber& n)  {
  *this << ph::ToString(n);
  return *this;
}
#endif
MemoryOutStream& MemoryOutStream::operator << (ni::uchar const* txt)  { *this << ni::cString(txt).Chars(); return *this; }
MemoryOutStream& MemoryOutStream::operator << (const ni::sVec2f& v)
{ *this << "["; FormatToStream(*this,"%g",v.x);
  *this << ","; FormatToStream(*this,"%g",v.y);
  *this << "]"; return *this; }
MemoryOutStream& MemoryOutStream::operator << (const ni::sVec3f& v)
{ *this << "["; FormatToStream(*this,"%g",v.x);
  *this << ","; FormatToStream(*this,"%g",v.y);
  *this << ","; FormatToStream(*this,"%g",v.z);
  *this << "]"; return *this; }
MemoryOutStream& MemoryOutStream::operator << (const ni::sVec4f& v)
{ *this << "["; FormatToStream(*this,"%g",v.x);
  *this << ","; FormatToStream(*this,"%g",v.y);
  *this << ","; FormatToStream(*this,"%g",v.z);
  *this << ","; FormatToStream(*this,"%g",v.w);
  *this << "]"; return *this; }
MemoryOutStream& MemoryOutStream::operator << (const ni::sVec2i& v)
{ *this << "["; FormatToStream(*this,"%d",v.x);
  *this << ","; FormatToStream(*this,"%d",v.y);
  *this << "]"; return *this; }
MemoryOutStream& MemoryOutStream::operator << (const ni::sVec3i& v)
{ *this << "["; FormatToStream(*this,"%d",v.x);
  *this << ","; FormatToStream(*this,"%d",v.y);
  *this << ","; FormatToStream(*this,"%d",v.z);
  *this << "]"; return *this; }
MemoryOutStream& MemoryOutStream::operator << (const ni::sVec4i& v)
{ *this << "["; FormatToStream(*this,"%d",v.x);
  *this << ","; FormatToStream(*this,"%d",v.y);
  *this << ","; FormatToStream(*this,"%d",v.z);
  *this << ","; FormatToStream(*this,"%d",v.w);
  *this << "]"; return *this; }
MemoryOutStream& MemoryOutStream::operator << (const ni::sMatrixf& v)
{ *this << "[";
  *this << "("; FormatToStream(*this,"%g",v._11);
  *this << ","; FormatToStream(*this,"%g",v._12);
  *this << ","; FormatToStream(*this,"%g",v._13);
  *this << ","; FormatToStream(*this,"%g",v._14);
  *this << ")";
  *this << "("; FormatToStream(*this,"%g",v._21);
  *this << ","; FormatToStream(*this,"%g",v._22);
  *this << ","; FormatToStream(*this,"%g",v._23);
  *this << ","; FormatToStream(*this,"%g",v._24);
  *this << ")";
  *this << "("; FormatToStream(*this,"%g",v._31);
  *this << ","; FormatToStream(*this,"%g",v._32);
  *this << ","; FormatToStream(*this,"%g",v._33);
  *this << ","; FormatToStream(*this,"%g",v._34);
  *this << ")";
  *this << "("; FormatToStream(*this,"%g",v._41);
  *this << ","; FormatToStream(*this,"%g",v._42);
  *this << ","; FormatToStream(*this,"%g",v._43);
  *this << ","; FormatToStream(*this,"%g",v._44);
  *this << ")]";
  return *this; }
MemoryOutStream& MemoryOutStream::operator << (const ni::Var& var)
{
  *this << "Var(";
  *this << var.mType;
  *this << ":";
  *this << var.mInt;
  *this << ")";
  return *this;
}

int MemoryOutStream::GetCapacity() const
{
  return m_capacity;
}


void MemoryOutStream::GrowBuffer(int const desiredCapacity)
{
  int const newCapacity = RoundUpToMultipleOfPow2Number(desiredCapacity, GROW_CHUNK_SIZE);

  char* buffer = new char[newCapacity];
  if (m_buffer)
    strcpy(buffer, m_buffer);
  else
    strcpy(buffer, "");

  delete [] m_buffer;
  m_buffer = buffer;
  m_capacity = newCapacity;
}

}

//--------------------------------------------------------------------------------------------
//
//  Test list
//
//--------------------------------------------------------------------------------------------

namespace UnitTest {

TestList::TestList()
    : m_head(0)
    , m_tail(0)
{
}

void TestList::Add(Test* test)
{
  if (m_tail == 0)
  {
    niAssert(m_head == 0);
    m_head = test;
    m_tail = test;
  }
  else
  {
    m_tail->next = test;
    m_tail = test;
  }
}

const Test* TestList::GetHead() const
{
  return m_head;
}

ListAdder::ListAdder(TestList& list, Test* test)
{
  list.Add(test);
}

}

//--------------------------------------------------------------------------------------------
//
// Unix Signal handling
//
//--------------------------------------------------------------------------------------------
#ifdef USE_SIGNALS
namespace UnitTest {

class SignalTranslator
{
 public:
  SignalTranslator();
  ~SignalTranslator();

  static sigjmp_buf* s_jumpTarget;

 private:
  sigjmp_buf m_currentJumpTarget;
  sigjmp_buf* m_oldJumpTarget;

  struct sigaction m_old_SIGFPE_action;
  struct sigaction m_old_SIGTRAP_action;
  struct sigaction m_old_SIGSEGV_action;
  struct sigaction m_old_SIGBUS_action;
  struct sigaction m_old_SIGABRT_action;
  struct sigaction m_old_SIGALRM_action;
};


#define TEST_THROW_SIGNALS                                \
  SignalTranslator sig;                                       \
  if (sigsetjmp( *SignalTranslator::s_jumpTarget, 1 ) != 0) { \
    niThrow ("Unhandled system exception");                   \
  }

sigjmp_buf* SignalTranslator::s_jumpTarget = 0;

namespace {

void SignalHandler (int sig)
{
  siglongjmp(*SignalTranslator::s_jumpTarget, sig );
}

}


SignalTranslator::SignalTranslator ()
{
  m_oldJumpTarget = s_jumpTarget;
  s_jumpTarget = &m_currentJumpTarget;

  struct sigaction action;
  action.sa_flags = 0;
  action.sa_handler = SignalHandler;
  sigemptyset( &action.sa_mask );

  sigaction( SIGSEGV, &action, &m_old_SIGSEGV_action );
  sigaction( SIGFPE , &action, &m_old_SIGFPE_action  );
  sigaction( SIGTRAP, &action, &m_old_SIGTRAP_action );
  sigaction( SIGBUS , &action, &m_old_SIGBUS_action  );
}

SignalTranslator::~SignalTranslator()
{
  sigaction( SIGBUS , &m_old_SIGBUS_action , 0 );
  sigaction( SIGTRAP, &m_old_SIGTRAP_action, 0 );
  sigaction( SIGFPE , &m_old_SIGFPE_action , 0 );
  sigaction( SIGSEGV, &m_old_SIGSEGV_action, 0 );

  s_jumpTarget = m_oldJumpTarget;
}


}
#endif

//--------------------------------------------------------------------------------------------
//
// 'Generic' Signal handling
//
//--------------------------------------------------------------------------------------------

#ifdef USE_SIGNALS


class SignalException
#ifndef niNoExceptions
    : public astl::exception
#endif
{
 public:
  SignalException(int signal, int subcode) {
    switch (signal) {
      case SIGABRT: strcpy(m_desc,"SIGABRT"); break;
      case SIGFPE: strcpy(m_desc,"SIGFPE"); break;
      case SIGILL: strcpy(m_desc,"SIGILL"); break;
      case SIGINT: strcpy(m_desc,"SIGINT"); break;
      case SIGSEGV: strcpy(m_desc,"SIGSEGV"); break;
      case SIGTERM: strcpy(m_desc,"SIGTERM"); break;
      default:
        sprintf(m_desc,"SIGUNK : %d",signal);
        break;
    }
  }
  virtual ~SignalException() niThrowSpec() {
  }
  const char* what() const niThrowSpec() {
    return m_desc;
  }
  char m_desc[1024];
};

class SignalTranslator
{
 public:
  SignalTranslator() {
    signal(SIGSEGV,_trap);
  }
  ~SignalTranslator() {
  }

  static void __cdecl _trap(int signal) {
    niThrow(SignalException(signal,0));
  }
};

static SignalTranslator _signalTranslator;
#endif

//--------------------------------------------------------------------------------------------
//
//  Test
//
//--------------------------------------------------------------------------------------------

namespace UnitTest {

TestList& Test::GetTestList()
{
  static TestList s_list;
  return s_list;
}

Test::Test(char const* testName, char const* filename,
           int const lineNumber, char const* fixtureName)
    : next(0)
    , m_testName(testName)
    , m_filename(filename)
    , m_fixtureName(fixtureName)
    , m_lineNumber(lineNumber)
    , m_timeConstraintExempt(false)
    , m_timeReport(true)
    , m_numSteps(1)
{
}

Test::~Test()
{
}

bool Test::BeforeRun(TestResults& testResults) const
{
  niTry() {
#ifdef USE_SIGNALS
    TEST_THROW_SIGNALS;
#endif
#ifndef TEST_DONT_PRINT_TEST_NAMES
    niPrintln(niFmt(_A("## [%d/%d] Test: %s ##\n"),
                            testResults.m_testCount,testResults.m_numTests,m_testName));
#endif
    m_timeStart = ni::TimerInSeconds();
#ifdef USE_SEH
    __try {
#endif
      BeforeRunImpl(testResults);
#ifdef USE_SEH
    }
    __except(SEHException::_Handle(::GetExceptionCode())) {
      throw SEHException(SEHException::_Desc(::GetExceptionCode()));
    }
#endif
  }
#ifdef TEST_NITHROWASSERT
  niCatch (AssertException const& e) {
    testResults.OnTestFailure(e.Filename(), e.LineNumber(), m_testName, e.what());
    return false;
  }
#endif
#ifndef niNoExceptions
  niCatch (astl::exception const& e) {
    MemoryOutStream stream;
    stream << "Unhandled exception: " << e.what();
    testResults.OnTestFailure(m_filename, m_lineNumber, m_testName, stream.GetText());
    return false;
  }
#endif
  niCatchAll() {
    testResults.OnTestFailure(m_filename, m_lineNumber, m_testName, "Unhandled exception: Crash.");
    return false;
  }
  return true;
}

bool Test::Run(TestResults& testResults) const
{
  niTry() {
#ifdef USE_SIGNALS
    TEST_THROW_SIGNALS;
#endif
#ifdef USE_SEH
    __try {
#endif
      RunImpl(testResults);
#ifdef USE_SEH
    }
    __except(SEHException::_Handle(::GetExceptionCode())) {
      throw SEHException(SEHException::_Desc(::GetExceptionCode()));
    }
#endif
  }
#ifdef TEST_NITHROWASSERT
  niCatch (AssertException const& e) {
    testResults.OnTestFailure(e.Filename(), e.LineNumber(), m_testName, e.what());
    return false;
  }
#endif
#ifndef niNoExceptions
  niCatch (astl::exception const& e) {
    MemoryOutStream stream;
    stream << "Unhandled exception: " << e.what();
    testResults.OnTestFailure(m_filename, m_lineNumber, m_testName, stream.GetText());
    return false;
  }
#endif
  niCatchAll() {
    testResults.OnTestFailure(m_filename, m_lineNumber, m_testName, "Unhandled exception: Crash.");
    return false;
  }
  return true;
}

bool Test::AfterRun(TestResults& testResults) const
{
  niTry() {
#ifdef USE_SIGNALS
    TEST_THROW_SIGNALS;
#endif
#ifdef USE_SEH
    __try {
#endif
      AfterRunImpl(testResults);
#ifdef USE_SEH
    }
    __except(SEHException::_Handle(::GetExceptionCode())) {
      throw SEHException(SEHException::_Desc(::GetExceptionCode()));
    }
#endif
    if (m_timeReport) {
      testResults.OnTestTime(m_testName, ni::TimerInSeconds() - m_timeStart);
    }
  }
#ifdef TEST_NITHROWASSERT
  niCatch (AssertException const& e) {
    testResults.OnTestFailure(e.Filename(), e.LineNumber(), m_testName, e.what());
    return false;
  }
#endif
#ifndef niNoExceptions
  niCatch (astl::exception const& e) {
    MemoryOutStream stream;
    stream << "Unhandled exception: " << e.what();
    testResults.OnTestFailure(m_filename, m_lineNumber, m_testName, stream.GetText());
    return false;
  }
#endif
  niCatchAll() {
    testResults.OnTestFailure(m_filename, m_lineNumber, m_testName, "Unhandled exception: Crash.");
    return false;
  }
  return true;
}

}

//--------------------------------------------------------------------------------------------
//
//  Test results
//
//--------------------------------------------------------------------------------------------
namespace UnitTest {

TestResults::TestResults(TestReporter* testReporter, int numTests)
    : m_testReporter(testReporter)
    , m_numTests(numTests)
    , m_testCount(0)
    , m_testFailedCount(0), m_testWarnedCount(0)
    , m_logErrors(0), m_logErrorsPush(0)
    , m_logWarnings(0), m_logWarningsPush(0)
    , m_warningMode(0)
    , m_totalFailureCount(0), m_totalFailureCountPush(0)
    , m_totalWarningCount(0), m_totalWarningCountPush(0)
{

}

void TestResults::OnTestStart(char const* testName)
{
  ++m_testCount;
  m_totalFailureCountPush = m_totalFailureCount;
  m_totalWarningCountPush = m_totalWarningCount;
  if (m_testReporter)
    m_testReporter->ReportTestStart(testName);
}

void TestResults::OnCountLog(int type)
{
  if (m_testReporter) {
    if (type == ni::eLogFlags_Warning) {
      ++m_logWarnings;
    }
    if (type == ni::eLogFlags_Error) {
      ++m_logErrors;
    }
  }
}

void TestResults::OnTestTime(const char* testName, float secondsElapsed)
{
  if (m_testReporter)
    m_testReporter->ReportTime(testName, secondsElapsed);
}

void TestResults::OnTestFailure(char const* file, int const line,
                                char const* testName, char const* failure)
{
  if (m_warningMode) {
    ++m_totalWarningCount;
    if (m_testReporter)
      m_testReporter->ReportWarning(file, line, testName, failure);
  }
  else {
    ++m_totalFailureCount;
    if (m_testReporter)
      m_testReporter->ReportFailure(file, line, testName, failure);
  }
}

void TestResults::OnTestFinish(char const* testName, float secondsElapsed)
{
  if (m_totalFailureCount-m_totalFailureCountPush > 0) {
    ++m_testFailedCount;
  }
  if (m_totalWarningCount-m_totalWarningCountPush > 0) {
    ++m_testWarnedCount;
  }
  if (m_testReporter)
    m_testReporter->ReportTestFinish(testName, secondsElapsed);
}


int TestResults::GetTestCount() const
{
  return m_testCount;
}

int TestResults::GetTestFailedCount() const {
  return m_testFailedCount;
}
int TestResults::GetTestWarnedCount() const {
  return m_testWarnedCount;
}

int TestResults::GetTotalFailureCount() const {
  return m_totalFailureCount;
}
int TestResults::GetTotalWarningCount() const {
  return m_totalWarningCount;
}

int TestResults::GetLogErrors() const
{
  return m_logErrors;
}

int TestResults::GetLogWarnings() const
{
  return m_logWarnings;
}

void TestResults::PushLogErrors()
{
  m_logErrorsPush = m_logErrors;
}

void TestResults::PushLogWarnings()
{
  m_logWarningsPush = m_logWarnings;
}

int TestResults::GetLogErrorsDelta() const
{
  return m_logErrors-m_logErrorsPush;
}

int TestResults::GetLogWarningsDelta() const
{
  return m_logWarnings-m_logWarningsPush;
}


}

//--------------------------------------------------------------------------------------------
//
//  Test reporter stdout
//
//--------------------------------------------------------------------------------------------

namespace UnitTest {

void TestReporterStdout::ReportTime(char const* testName, float secondsElapsed)
{
  ni::cString str = niFmt(_A("Test %s took %.5f seconds.\n"),_ASZ(testName),secondsElapsed);
  niPrintln(str.Chars());
}

void TestReporterStdout::ReportFailure(char const* file, int const line,  char const* testName, char const* failure)
{
  ni::cString str;
#ifdef niWindows
  ni::achar const* const errorFormat = _A("%s(%d): error: Failure in %s: %s\n");
#else
  ni::achar const* const errorFormat = _A("%s:%d: error: Failure in %s: %s\n");
#endif
  str.Format(errorFormat, _ASZ(file), line, _ASZ(testName), _ASZ(failure));
  niPrintln(str.Chars());
  _failures.push_back(str);
}

void TestReporterStdout::ReportWarning(char const* file, int const line,  char const* testName, char const* failure)
{
  ni::cString str;
#ifdef niWindows
  ni::achar const* const warningFormat = _A("%s(%d): warning: Failure in %s: %s\n");
#else
  ni::achar const* const warningFormat = _A("%s:%d: warning: Failure in %s: %s\n");
#endif
  str.Format(warningFormat, _ASZ(file), line, _ASZ(testName), _ASZ(failure));
  niPrintln(str.Chars());
  _warnings.push_back(str);
}

void TestReporterStdout::ReportTestStart(char const* test)
{
  niUnused(test);
}

void TestReporterStdout::ReportTestFinish(char const* test, float)
{
  niUnused(test);
}

void TestReporterStdout::ReportSummary(int testCount,
                                       int failedCount, int totalFailedCount,
                                       int warnedCount, int totalWarnedCount,
                                       float secondsElapsed)
{
  ni::cString str;
  if (warnedCount > 0) {
    str += niFmt(_A("WARNING: %d out of %d tests warned (%d warnings).\n"),
                 warnedCount, testCount, totalWarnedCount);
  }
  if (failedCount > 0) {
    str += niFmt(_A("FAILURE: %d out of %d tests failed (%d failures).\n"),
                 failedCount, testCount, totalFailedCount);
  }
  else {
    str += niFmt(_A("Success: %d tests passed.\n"), testCount);
  }
  if (_warnings.size() > 0) {
    str << "WARNINGS:\n";
    niLoop(i,_warnings.size()) {
      str << "- " << _warnings[i];
    }
  }
  if (_failures.size() > 0) {
    str << "FAILURES:\n";
    niLoop(i,_failures.size()) {
      str << "- " << _failures[i];
    }
  }
  str += niFmt(_A("Test time: %g seconds.\n"), secondsElapsed);
  niPrintln(str.Chars());
}

}

//--------------------------------------------------------------------------------------------
//
//  Time constraint
//
//--------------------------------------------------------------------------------------------

namespace UnitTest {


TimeConstraint::TimeConstraint(int ms, TestResults& result, char const* filename, int lineNumber,
                               char const* testName)
    : m_result (result)
    , m_maxMs (ms)
    , m_filename (filename)
    , m_lineNumber (lineNumber)
    , m_testName (testName)
{
  m_timeStart = ni::TimerInSeconds();
}

TimeConstraint::~TimeConstraint()
{
  int const totalTimeInMs = (int)((ni::TimerInSeconds() - m_timeStart)*1000.0);
  if (totalTimeInMs > m_maxMs) {
    MemoryOutStream stream;
    stream << "Time constraint failed. Expected to run test under " << m_maxMs <<
        "ms but took " << totalTimeInMs << "ms.";
    m_result.OnTestFailure(m_filename, m_lineNumber, m_testName, stream.GetText());
  }
}

}

//--------------------------------------------------------------------------------------------
//
//  Checks
//
//--------------------------------------------------------------------------------------------
namespace UnitTest {

void CheckStringsEqual(TestResults& results, char const* const expected, char const* const actual,
                       char const* const testName, char const* const filename, int const line)
{
  if (ni::StrCmp(expected, actual) != 0) {
    UnitTest::MemoryOutStream stream;
    stream << "Expected [" << expected << "] == [" << actual << "]";
    results.OnTestFailure(filename, line, testName, stream.GetText());
  }
}

void CheckStringsNotEqual(TestResults& results, char const* const expected, char const* const actual,
                       char const* const testName, char const* const filename, int const line)
{
  if (ni::StrCmp(expected, actual) == 0) {
    UnitTest::MemoryOutStream stream;
    stream << "Expected [" << expected << "] != [" << actual << "]";
    results.OnTestFailure(filename, line, testName, stream.GetText());
  }
}

void CheckEqual(TestResults& results, char const* const expected, char const* const actual,
                char const* const testName, char const* const filename, int const line)
{
  CheckStringsEqual(results, expected, actual, testName, filename, line);
}

void CheckNotEqual(TestResults& results, char const* const expected, char const* const actual,
                   char const* const testName, char const* const filename, int const line)
{
  CheckStringsNotEqual(results, expected, actual, testName, filename, line);
}

}

//--------------------------------------------------------------------------------------------
//
//  Test Runner
//
//--------------------------------------------------------------------------------------------

namespace UnitTest {

inline bool shouldSkipFixture(char const* filter, char const* fixtureName, char const* testName) {
  if (filter && *filter) {
    if (ni::StrICmp(filter,"*") == 0)
      return false;
    if (ni::StrICmp(filter,"all") == 0)
      return false;

    return
        (!fixtureName || ni::StrICmp(fixtureName,filter) != 0) &&
        ni::StrICmp(testName,filter) != 0;
  }
  return false;
}

inline void normalizeFixtureName(ni::cString& fixtureName) {
  ni::tI32 commaPos = fixtureName.find(',');
  if (commaPos >= 0) {
    fixtureName.data()[commaPos] = '-';
  }
}

struct TestRunner {
  ni::tF64 startTime;
  TestReporter& reporter;
  TestList const& list;
  int const maxTestTimeInMs;
  ni::cString fixtureName;
  int shouldRun;
  Test const* curTest;
  int curTestSteps;
  int numTestRun;
  TestResults* result;

  TestRunner(TestReporter& aReporter,
             TestList const& aList,
             int const aMaxTestTimeInMs,
             char const* aFixtureName)
      : startTime(ni::TimerInSeconds())
      , reporter(aReporter)
      , list(aList)
      , maxTestTimeInMs(aMaxTestTimeInMs)
      , fixtureName(aFixtureName)
      , shouldRun(0)
      , curTest(NULL)
      , curTestSteps(0)
      , numTestRun(0)
      , result(NULL)
  {
#ifdef niWindows
    if (!IsDebuggerPresent())
#endif
    {
      // don't popup the assert dialogs, only print them to the console
      ni_debug_set_print_asserts(ni::eTrue);
    }

    normalizeFixtureName(fixtureName);

    shouldRun = 0;
    curTest = list.GetHead();
    while (curTest != 0) {
      Test const* nextTest = curTest->next;
      if (shouldSkipFixture(fixtureName.c_str(),curTest->m_fixtureName,curTest->m_testName)) {
        curTest = nextTest;
        continue;
      }
      ++shouldRun;
      curTest = nextTest;
    }

    result = new TestResults(&reporter,shouldRun);
    Reset();
  }

  ~TestRunner() {
    if (result)
      delete result;
  }

  void Reset() {
    numTestRun = 0;
    curTest = list.GetHead();
    curTestSteps = 0;
  }

  ni::tF64 testTimeStart;
  Test const* nextTest;

  bool ShouldRun() {
    if (!curTest)
      return false;

    nextTest = curTest->next;
    if (shouldSkipFixture(fixtureName.c_str(),curTest->m_fixtureName,curTest->m_testName)) {
      curTest = nextTest;
      curTestSteps = 0;
      return false;
    }

    return true;
  }

  bool BeforeRun() {
    testTimeStart = ni::TimerInSeconds();
    result->OnTestStart(curTest->m_testName);
    return curTest->BeforeRun(*result);
  }

  bool RunStep() {
    return curTest->Run(*result);
  }

  bool AfterRun() {
    curTest->AfterRun(*result);

    int const testTimeInMs = ni::TimerInSeconds() - testTimeStart;
    if (maxTestTimeInMs > 0 && testTimeInMs > maxTestTimeInMs && !curTest->m_timeConstraintExempt)
    {
      MemoryOutStream stream;
      stream << "Global time constraint failed. Expected under " << maxTestTimeInMs <<
          "ms but took " << testTimeInMs << "ms.";
      result->OnTestFailure(curTest->m_filename, curTest->m_lineNumber,
                           curTest->m_testName, stream.GetText());
    }
    result->OnTestFinish(curTest->m_testName, testTimeInMs/1000.0f);

    curTest = nextTest;
    curTestSteps = 0;
    ++numTestRun;

    return curTest != NULL;
  }

  bool RunNext() {
    if (curTest) {
      // First time the current test is ran
      if (curTestSteps == 0) {
        if (!ShouldRun())
          return curTest != NULL;
        BeforeRun();
      }
      RunStep();
      if (++curTestSteps >= curTest->m_numSteps) {
        AfterRun();
      }
    }
    return curTest != NULL;
  }

  int ReportSummary() {
    reporter.ReportSummary(result->GetTestCount(),
                           result->GetTestFailedCount(),
                           result->GetTotalFailureCount(),
                           result->GetTestWarnedCount(),
                           result->GetTotalWarningCount(),
                           (float)(ni::TimerInSeconds()-startTime));

    if (!(numTestRun == shouldRun)) {
      niPrintln("Not all tests have been ran, it is very likely that the program's memory has been corrupted.");
    }

    return result->GetTestFailedCount();
  }
};

astl::vector<ni::cString> dataDirs;

using namespace ni;

// #define TRACE_FILEHANDLER(X) niDebugFmt(X)
#define TRACE_FILEHANDLER(X)

struct URLFileHandler_Tests : public URLFileHandler_Manifest {
  URLFileHandler_Tests() {
    dataDirs.push_back(ni::GetToolkitDir("niLang","data"));
  }

  tBool _GetExistingPath(const achar* aURL, cString& aPath) {
    aPath = this->GetPathFromURL(aURL);

    niLoop(i,dataDirs.size()) {
      // Test with the data dir first, this should be the most common since the
      // default URL handler indicates relative paths.
      const cString dataDir = dataDirs[i];
      if (!dataDir.empty()) {
        const cString testPath = dataDir + aPath;
        TRACE_FILEHANDLER(("URLFileHandler.Tests.default.GetExisting: %s", testPath));
        if (ni::GetRootFS()->FileExists(testPath.Chars(), eFileAttrFlags_AllFiles)) {
          aPath = testPath;
          return eTrue;
        }
      }
    }

    // Check the path as-is.
    TRACE_FILEHANDLER(("URLFileHandler.Tests.default.GetExisting.asis: %s", aPath));
    if (ni::GetRootFS()->FileExists(aPath.Chars(), eFileAttrFlags_AllFiles)) {
      return eTrue;
    }

    return eFalse;
  }

  virtual iFile* __stdcall URLOpen(const achar* aURL) {
    cString path;
    if (!_GetExistingPath(aURL,path)) {
      TRACE_FILEHANDLER(("URLFileHandler.Tests.default.URLOpen.noexisting: %s", aURL));
      return NULL;
    }
    TRACE_FILEHANDLER(("URLFileHandler.Tests.default.URLOpen: %s", path));

    return ni::GetRootFS()->FileOpen(path.Chars(),eFileOpenMode_Read);
  }

  virtual tBool __stdcall URLExists(const achar* aURL) {
    cString path;
    _GetExistingPath(aURL,path);
    return ni::GetRootFS()->FileExists(path.Chars(),eFileAttrFlags_AllFiles);
  }
};

static TestReporterStdout _defaultReporterStdout;
static TestRunner* _defaultTestRunner = NULL;

bool TestRunner_Startup(TestReporter& reporter,
                      TestList const& list,
                      int const maxTestTimeInMs,
                      char const* fixtureName)
{
  TestRunner_Shutdown();
  ni::GetLang()->SetGlobalInstance("URLFileHandler.default",niNew URLFileHandler_Tests());
  ni::GetLang()->SetGlobalInstance("URLFileHandler.script",niNew URLFileHandler_Tests());
  runFixtureName = fixtureName;
  normalizeFixtureName(runFixtureName);
  _defaultTestRunner = new TestRunner(reporter,list,maxTestTimeInMs,fixtureName);
  return true;
};

bool TestRunner_Startup(char const* fixtureName)
{
  return TestRunner_Startup(_defaultReporterStdout, Test::GetTestList(), 0, fixtureName);
}

void TestRunner_Shutdown() {
  if (_defaultTestRunner) {
    delete _defaultTestRunner;
    _defaultTestRunner = NULL;
  }
}

bool TestRunner_RunNext() {
  if (!_defaultTestRunner) return false;
  return _defaultTestRunner->RunNext();
}

int TestRunner_ReportSummary() {
  if (!_defaultTestRunner) return ni::eInvalidHandle;
  return _defaultTestRunner->ReportSummary();
}

const char* TestRunner_GetCurrentTestName() {
  if (!_defaultTestRunner) return "";
  return _defaultTestRunner->curTest->m_testName;
}

ni::cString runFixtureName;

int RunAllTests(TestReporter& reporter,
                TestList const& list,
                int const maxTestTimeInMs,
                char const* fixtureName)
{
  TestRunner_Startup(reporter,list,maxTestTimeInMs,fixtureName);
  while (TestRunner_RunNext()) {
  }
  int r = TestRunner_ReportSummary();
  TestRunner_Shutdown();
  return r;
}

int RunAllTests(char const* fixtureName)
{
  TestReporterStdout reporter;
  return RunAllTests(reporter, Test::GetTestList(), 0, fixtureName);
}

}
