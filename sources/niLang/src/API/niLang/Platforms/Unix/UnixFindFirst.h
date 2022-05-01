#ifndef __UNIXFINDFIRST_H_B0CFA78D_E10A_4BD8_995B_59853E193611__
#define __UNIXFINDFIRST_H_B0CFA78D_E10A_4BD8_995B_59853E193611__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "../../Types.h"

/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Unix
 * @{
 */

#ifndef HAVE_FINDFIRST

#include <limits.h>
#if defined niOSX || defined niLinux || defined niQNX
#  include <time.h>
#  include <dirent.h>
#endif

#ifdef ni64
#  if !defined __USE_LARGEFILE64 || __USE_LARGEFILE64 != 1
#    error "__USE_LARGEFILE64 not defined correctly."
#  endif
#endif

#define FF_MAXPATHLEN _MAX_PATH

struct sUnixFFInfo {
  niNamespace(ni,achar) dirname[FF_MAXPATHLEN];
  niNamespace(ni,achar) pattern[FF_MAXPATHLEN];
  niNamespace(ni,achar) filename[FF_MAXPATHLEN];
  DIR* dir;
};

struct sUnixFFBlk {
  sUnixFFInfo info;
  niNamespace(ni,achar) name[FF_MAXPATHLEN];
  time_t      time_write;
  unsigned long size;
  unsigned long attrib;
};

niExportFunc(int) unix_ff_match(const niNamespace(ni,achar) *fn,
                                const niNamespace(ni,achar) *pattern);
niExportFunc(int) unix_findfirst(const niNamespace(ni,achar) *pattern, sUnixFFBlk* aFFBlk);
niExportFunc(int) unix_findnext(sUnixFFBlk* aFFBlk);
niExportFunc(void) unix_findclose(sUnixFFBlk* aFFBlk);
niExportFunc(niNamespace(ni,tBool)) unix_filetime(const niNamespace(ni,achar)* filename, time_t* apATime, time_t* apMTime, time_t* apCTime);

#endif // HAVE_FINDFIRST

/**@}*/
/**@}*/

#endif // __UNIXFINDFIRST_H_B0CFA78D_E10A_4BD8_995B_59853E193611__
