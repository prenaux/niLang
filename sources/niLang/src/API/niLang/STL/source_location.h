// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#ifndef __SOURCE_LOCATION_H_13513EAF_F158_F84C_A5AE_12166C2E44E6__
#define __SOURCE_LOCATION_H_13513EAF_F158_F84C_A5AE_12166C2E44E6__

#include <niLang/Types.h>

#if defined niLinuxDesktop || defined niSourceLocationUseStd

#include <source_location>

namespace astl {
using source_location = std::source_location;
}

#elif defined niSourceLocationUseExperimental

#include <experimental/source_location>
namespace astl {
using source_location = std::experimental::source_location;
}

#else

#if !defined(__has_builtin)
#  define __has_builtin(builtin) OL
#endif

namespace astl {

struct source_location
{
 private:
  const int m_line;
  const int m_column;
  const char* m_file_name;
  const char* m_function_name;
  explicit niConstExpr source_location(
    const int line=0,
    const int column=0,
    const char* file_name="not_initialized",
    const char* function_name="not_initialized") EA_NOEXCEPT
      : m_line(line)
      , m_column(column)
      , m_file_name(file_name)
      , m_function_name(function_name)
  {}

 public:
#if __has_builtin(__builtin_COLUMN) || (defined(_MSC_VER) && _MSC_VER > 1925)
  inline static niConstExpr source_location current(
    const int line=(int)__builtin_LINE(),
    const int column=(int)__builtin_COLUMN(),
    const char* file_name=__builtin_FILE(),
    const char* function_name=__builtin_FUNCTION()) EA_NOEXCEPT
#elif defined(__GNUC__) and (__GNUC__ > 4 or (__GNUC__ == 4 and __GNUC_MINOR__ >= 8))
  inline static niConstExpr source_location current(
    const int line=(int)__builtin_LINE(),
    const int column=0,
    const char* file_name=__builtin_FILE(),
    const char* function_name=__builtin_FUNCTION()) EA_NOEXCEPT
#else
#error "source_location is not supported on this platform."
  inline static niConstExpr source_location current(
    const int line=0,
    const int column=0,
    const char* file_name="unsupported",
    const char* function_name="unsupported") EA_NOEXCEPT
#endif
  {
    return source_location(line, column, file_name, function_name);
  }

  niConstExpr int line() const EA_NOEXCEPT {
    return m_line;
  }

  niConstExpr int column() const EA_NOEXCEPT {
    return m_column;
  }

  niConstExpr const char* file_name() const EA_NOEXCEPT {
    return m_file_name;
  }

  niConstExpr const char* function_name() const EA_NOEXCEPT {
    return m_function_name;
  }
};

}
#endif

#define ASTL_SOURCE_LOCATION_PARAM              astl::source_location aSrcLoc
#define ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT astl::source_location aSrcLoc = astl::source_location::current()
#define ASTL_SOURCE_LOCATION_PARAM_SOLO         astl::source_location aSrcLoc
#define ASTL_SOURCE_LOCATION_ARG_SOLO           aSrcLoc
#define ASTL_SOURCE_LOCATION_ARG_CALL           aSrcLoc.file_name(), aSrcLoc.line(), aSrcLoc.function_name()

#endif // __SOURCE_LOCATION_H_13513EAF_F158_F84C_A5AE_12166C2E44E6__
