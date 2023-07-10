#ifndef __SOURCE_LOCATION_H_13513EAF_F158_F84C_A5AE_12166C2E44E6__
#define __SOURCE_LOCATION_H_13513EAF_F158_F84C_A5AE_12166C2E44E6__
/*

  Original code found at:
  https://github.com/flagarde/source_location

  MIT License

  Copyright (c) 2021 flagarde

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include <niLang/STL/EASTL/EABase/config/eacompilertraits.h>
#include <cstdint>

#if !defined(__has_include)
#  define __has_include(include) 0L
#endif

#if (__cplusplus >= 202002L && __has_include(<source_location>) && !defined(__clang__)) || (_MSVC_LANG > 202002L) || (_MSC_VER > 1928 && _MSVC_LANG > 201703L)

#include <source_location>

namespace astl {
using source_location = std::source_location;
}

#elif __cplusplus >= 201402L && __has_include(<experimental/source_location>) && !defined(__clang__)

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
#if __cplusplus >= 201103L || _MSVC_LANG >= 201103L
  using intType = std::uint_least32_t;
#else
  typedef int intType;
#endif
  const intType m_line;
  const intType m_column;
  const char* m_file_name;
  const char* m_function_name;
  explicit EA_CONSTEXPR source_location(
    const intType& line=0,
    const intType& column=0,
    const char* file_name="not_initialized",
    const char* function_name="not_initialized") EA_NOEXCEPT
      : m_line(line)
      , m_column(column)
      , m_file_name(file_name)
      , m_function_name(function_name)
  {}

 public:
#if __has_builtin(__builtin_COLUMN) || (defined(_MSC_VER) && _MSC_VER > 1925)
  inline static EA_CONSTEXPR source_location current(
    const intType& line=__builtin_LINE(),
    const intType& column=__builtin_COLUMN(),
    const char* file_name=__builtin_FILE(),
    const char* function_name=__builtin_FUNCTION()) EA_NOEXCEPT
#elif defined(__GNUC__) and (__GNUC__ > 4 or (__GNUC__ == 4 and __GNUC_MINOR__ >= 8))
  inline static EA_CONSTEXPR source_location current(
    const intType& line=__builtin_LINE(),
    const intType& column=0,
    const char* file_name=__builtin_FILE(),
    const char* function_name=__builtin_FUNCTION()) EA_NOEXCEPT
#else
#error "source_location is not supported on this platform."
  inline static EA_CONSTEXPR source_location current(
    const intType& line=0,
    const intType& column=0,
    const char* file_name="unsupported",
    const char* function_name="unsupported") EA_NOEXCEPT
#endif
  {
    return source_location(line, column, file_name, function_name);
  }

  EA_CONSTEXPR intType line() const EA_NOEXCEPT
  {
    return m_line;
  }

  EA_CONSTEXPR intType column() const EA_NOEXCEPT
  {
    return m_column;
  }

  EA_CONSTEXPR const char* file_name() const EA_NOEXCEPT
  {
    return m_file_name;
  }

  EA_CONSTEXPR const char* function_name() const EA_NOEXCEPT
  {
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
