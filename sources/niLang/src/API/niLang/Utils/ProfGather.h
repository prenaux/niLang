// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#ifdef __NI_PROFGATHER_H__
#error "ProfGather is already included, you should make sure to include it only once to avoid definition conflicts."
#else
#define __NI_PROFGATHER_H__

#include "../IProf.h"

// takes 'precedence' over niProfiler so that profiling code can be forcefully
// removed during the build process
#ifdef niNoProfiler
#undef niProfiler
#endif

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

#if defined niProfiler

#pragma niNote("# Profiler Enabled")

static ni::tI64 niProf_time;
static ni::sProfilerZoneStack** niProf_stack_ptr;

#define niProf_Begin_Cache(z)                                           \
  /* declare a static cache of the zone stack */                        \
  static ni::sProfilerZoneStack *niProf_cache = ni::GetProf()->GetZoneStackDummy()

#define niProf_Begin_Raw(z)                     \
  niProf_Begin_Cache(z);                        \
  niProf_Begin_Code(z)

#define niProf_Begin_Code(z)                                      \
  (                                                               \
      (ni::niProf_stack_ptr = ni::GetProf()->GetZoneStack()),     \
      /* check the cached Zone_Stack and update if needed */      \
      (niProf_cache->parent != (*ni::niProf_stack_ptr)            \
       ? niProf_cache = ni::GetProf()->StackAppend(&z)            \
       : 0),                                                      \
      ++niProf_cache->total_entry_count,                          \
      ni::GetProf()->GetTimeStamp(&ni::niProf_time),              \
      /* stop the timer on the parent zone stack */               \
      ((*ni::niProf_stack_ptr)->total_self_ticks +=               \
       ni::niProf_time - (*ni::niProf_stack_ptr)->t_self_start),  \
      /* make cached stack current */                             \
      (*ni::niProf_stack_ptr) = niProf_cache,                     \
      /* start the timer on this stack */                         \
      (*ni::niProf_stack_ptr)->t_self_start = ni::niProf_time,    \
      0)

#define niProf_End_Raw()                                      \
  (ni::GetProf()->GetTimeStamp(&ni::niProf_time),             \
   (ni::niProf_stack_ptr = ni::GetProf()->GetZoneStack()),    \
   /* stop timer for current zone stack */                    \
   (*ni::niProf_stack_ptr)->total_self_ticks +=               \
   ni::niProf_time - (*ni::niProf_stack_ptr)->t_self_start,   \
   /* make parent chain current */                            \
   (*ni::niProf_stack_ptr) = (*ni::niProf_stack_ptr)->parent, \
   /* start timer for parent zone stack */                    \
   (*ni::niProf_stack_ptr)->t_self_start = ni::niProf_time)

#define niProf_Declare(z)  ni::sProfilerZone niProf_region_##z
#define niProf_Define(z)   niProf_Declare(z) = { #z }
#define niProf_Region(z)   niProf_Begin_Raw(niProf_region_##z);
#define niProf_End()       niProf_End_Raw();

#define niProf_Begin(z)    static niProf_Define(z); niProf_Region(z)
#define niProf_Counter(z)  niProf_Begin(z) niProf_End

#ifdef __cplusplus

#define niProf_Scope(x)                                                 \
  niProf_Begin_Cache(x);                                                \
  ni::niProf_Scope_Var niProf_scope_var(niProf_region_ ## x, niProf_cache)

#define niProf(x)       static niProf_Define(x); niProf_Scope(x)

struct niProf_Scope_Var {
  inline niProf_Scope_Var(ni::sProfilerZone &zone,
                          ni::sProfilerZoneStack* &niProf_cache);
  inline ~niProf_Scope_Var();
};
inline niProf_Scope_Var::niProf_Scope_Var(ni::sProfilerZone &zone,
                                          ni::sProfilerZoneStack * &niProf_cache)
{
  niProf_Begin_Code(zone);
}
inline niProf_Scope_Var::~niProf_Scope_Var() {
  niProf_End_Raw();
}
#endif

#else

#pragma niNote("# Profiler Disabled")

#ifdef __cplusplus
#define niProf(x)
#define niProf_Scope(x)
#endif

#define niProf_Define(name)
#define niProf_Begin(z)
#define niProf_End()
#define niProf_Region(z)
#define niProf_Counter(z)

#endif

#define niProfileBlock(NAME) niProf(NAME)
#define niProfileEnter(NAME) niProf_Begin(NAME)
#define niProfileLeave(NAME) niProf_End

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __NI_PROFGATHER_H__
