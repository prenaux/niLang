#ifndef __UNIXCONFIG_H_26C93CBA_7B6D_443E_AF3B_FFECC3785BFA__
#define __UNIXCONFIG_H_26C93CBA_7B6D_443E_AF3B_FFECC3785BFA__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

/* Define to empty if the keyword does not work.  */
#undef const

/* Define if you don't have vprintf but do have _doprnt.  */
#undef HAVE_DOPRNT

/* Define if you have a working `mmap' system call.  */
#undef HAVE_MMAP

/* Define if you have the vprintf function.  */
#undef HAVE_VPRINTF

/* Define as __inline if that's what the C compiler calls it.  */
#undef inline

/* Define as the return type of signal handlers (int or void).  */
#undef RETSIGTYPE

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
#undef size_t

/* Define if you have the ANSI C header files.  */
#undef STDC_HEADERS

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#undef TIME_WITH_SYS_TIME

/* Define if your <sys/time.h> declares struct tm.  */
#undef TM_IN_SYS_TIME

/* Define if your processor stores words with the most significant
   byte first (like Motorola and SPARC, unlike Intel and VAX).  */
#undef WORDS_BIGENDIAN

/* Define if target platform is linux.  */
#undef AGLUNIX_LINUX

/* Define to enable Linux console VGA driver */
#undef AGLUNIX_LINUX_VGA

/* Define to enable Linux console SVGAlib driver */
#undef AGLUNIX_LINUX_SVGALIB

/* Define if target machine is little endian.  */
#undef AGLUNIX_LITTLE_ENDIAN

/* Define if target machine is big endian.  */
#undef AGLUNIX_BIG_ENDIAN

/* Define if you need support for X-Windows.  */
#undef AGLUNIX_WITH_XWINDOWS

/* Define if MIT-SHM extension is supported.  */
#undef AGLUNIX_XWINDOWS_WITH_SHM

/* Define if XF86DGA extension is supported.  */
#undef AGLUNIX_XWINDOWS_WITH_XF86DGA

/* Define if OSS DIGI driver is supported.  */
#undef AGLUNIX_WITH_OSSDIGI

/* Define if ALSA DIGI driver is supported.  */
#undef AGLUNIX_WITH_ALSADIGI

/* Define if ESD DIGI driver is supported.  */
#undef AGLUNIX_WITH_ESDDIGI

/* Define to (void *)-1, if MAP_FAILED is not defined.  */
#undef MAP_FAILED

/* Define if constructor attribute is supported. */
#undef AGLUNIX_USE_CONSTRUCTOR

/* Define if you have the findfirst function.  */
#undef HAVE_FINDFIRST

/* Define if you have the getpagesize function.  */
#undef HAVE_GETPAGESIZE

/* Define if you have the memcmp function.  */
#undef HAVE_MEMCMP

/* Define if you have the stricmp function.  */
#undef HAVE_STRICMP

/* Define if you have the strlwr function.  */
#undef HAVE_STRLWR

/* Define if you have the strupr function.  */
#undef HAVE_STRUPR

/* Define if you have the <dirent.h> header file.  */
#undef HAVE_DIRENT_H

/* Define if you have the <fcntl.h> header file.  */
#undef HAVE_FCNTL_H

/* Define if you have the <limits.h> header file.  */
#undef HAVE_LIMITS_H

/* Define if you have the <linux/joystick.h> header file.  */
#undef HAVE_LINUX_JOYSTICK_H

/* Define if you have the <linux/soundcard.h> header file.  */
#undef HAVE_LINUX_SOUNDCARD_H

/* Define if you have the <machine/soundcard.h> header file.  */
#undef HAVE_MACHINE_SOUNDCARD_H

/* Define if you have the <ndir.h> header file.  */
#undef HAVE_NDIR_H

/* Define if you have the <soundcard.h> header file.  */
#undef HAVE_SOUNDCARD_H

/* Define if you have the <sys/asoundlib.h> header file.  */
#undef HAVE_SYS_ASOUNDLIB_H

/* Define if you have the <sys/dir.h> header file.  */
#undef HAVE_SYS_DIR_H

/* Define if you have the <sys/io.h> header file.  */
#undef HAVE_SYS_IO_H

/* Define if you have the <sys/ndir.h> header file.  */
#undef HAVE_SYS_NDIR_H

/* Define if you have the <sys/soundcard.h> header file.  */
#undef HAVE_SYS_SOUNDCARD_H

/* Define if you have the <sys/time.h> header file.  */
#undef HAVE_SYS_TIME_H

/* Define if you have the <sys/utsname.h> header file.  */
#undef HAVE_SYS_UTSNAME_H

/* Define if you have the <unistd.h> header file.  */
#undef HAVE_UNISTD_H

//////////////////////////////////////////////////////////////////////
#define HAVE_DIRENT_H

#ifdef HAVE_DIRENT_H
#include <sys/types.h>
#include <dirent.h>
#define NAMELEN(dirent) (strlen((dirent)->d_name))
#else
#define dirent direct
#define NAMELEN(dirent) ((dirent)->d_namlen)
#ifdef HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#endif
#ifdef HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif
#ifdef HAVE_NDIR_H
#include <ndir.h>
#endif
#endif

#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

#endif // __UNIXCONFIG_H_26C93CBA_7B6D_443E_AF3B_FFECC3785BFA__
