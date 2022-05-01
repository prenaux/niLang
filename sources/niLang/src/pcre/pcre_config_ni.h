#pragma once
#ifndef __CONFIG_H_5B172048_1634_4A81_95F2_048D8D053E90__
#define __CONFIG_H_5B172048_1634_4A81_95F2_048D8D053E90__

#include <niLang/Types.h>

#ifndef PCRE_EXP_DECL
#  ifdef __cplusplus
#    define PCRE_EXP_DECL  extern "C" __ni_module_export
#  else
#    define PCRE_EXP_DECL  extern __ni_module_export
#  endif
#endif

#define PCRE_EXP_DEFN __ni_module_export
#define PCRE_EXP_DATA_DEFN __ni_module_export

#define SUPPORT_UTF8    1
#define BSR_ANYCRLF     1

#define NEWLINE			-2
#define POSIX_MALLOC_THRESHOLD	10
#define LINK_SIZE		2
#define MATCH_LIMIT		10000000
#define MATCH_LIMIT_RECURSION	MATCH_LIMIT

#define MAX_NAME_SIZE	32
#define MAX_NAME_COUNT	10000

#define PCRE_STATIC     1

#endif // __CONFIG_H_5B172048_1634_4A81_95F2_048D8D053E90__
