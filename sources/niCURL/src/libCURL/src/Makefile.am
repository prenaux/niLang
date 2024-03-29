#***************************************************************************
#                                  _   _ ____  _
#  Project                     ___| | | |  _ \| |
#                             / __| | | | |_) | |
#                            | (__| |_| |  _ <| |___
#                             \___|\___/|_| \_\_____|
#
# Copyright (C) 1998 - 2013, Daniel Stenberg, <daniel@haxx.se>, et al.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://curl.haxx.se/docs/copyright.html.
#
# You may opt to use, copy, modify, merge, publish, distribute and/or sell
# copies of the Software, and permit persons to whom the Software is
# furnished to do so, under the terms of the COPYING file.
#
# This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
# KIND, either express or implied.
#
###########################################################################
AUTOMAKE_OPTIONS = foreign nostdinc

DSP = vc6libcurl.dsp
VCPROJ = libcurl.vcproj

DOCS = README.encoding README.memoryleak README.ares README.curlx	\
 README.hostip README.multi_socket README.httpauth README.pipelining    \
 README.curl_off_t README.pingpong

CMAKE_DIST = CMakeLists.txt curl_config.h.cmake

EXTRA_DIST = Makefile.b32 Makefile.m32 Makefile.vc6 $(DSP)		\
 vc6libcurl.dsw config-win32.h config-win32ce.h config-riscos.h		\
 config-mac.h curl_config.h.in makefile.dj config-dos.h libcurl.plist	\
 libcurl.rc config-amigaos.h makefile.amiga Makefile.netware nwlib.c	\
 nwos.c msvcproj.head msvcproj.foot config-win32ce.h config-os400.h	\
 setup-os400.h config-symbian.h Makefile.Watcom config-tpf.h $(DOCS)	\
 $(VCPROJ) mk-ca-bundle.pl mk-ca-bundle.vbs firefox-db2pem.sh		\
 $(CMAKE_DIST) config-vxworks.h Makefile.vxworks checksrc.pl		\
 objnames-test08.sh objnames-test10.sh objnames.inc

CLEANFILES = $(DSP) $(VCPROJ)

lib_LTLIBRARIES = libcurl.la
LIBCURL_LIBS = @LIBCURL_LIBS@

# This might hold -Werror
CFLAGS += @CURL_CFLAG_EXTRAS@

CFLAG_CURL_SYMBOL_HIDING = @CFLAG_CURL_SYMBOL_HIDING@

# Specify our include paths here, and do it relative to $(top_srcdir) and
# $(top_builddir), to ensure that these paths which belong to the library
# being currently built and tested are searched before the library which
# might possibly already be installed in the system.
#
# $(top_builddir)/include/curl for generated curlbuild.h included from curl.h
# $(top_builddir)/include for generated curlbuild.h inc. from lib/curl_setup.h
# $(top_srcdir)/include is for libcurl's external include files
# $(top_builddir)/lib is for libcurl's generated lib/curl_config.h file
# $(top_srcdir)/lib for libcurl's lib/curl_setup.h and other "private" files
# $(top_builddir)/ares is for in-tree c-ares's generated ares_build.h file
# $(top_srcdir)/ares is for in-tree c-ares's external include files

if USE_EMBEDDED_ARES
AM_CPPFLAGS = -I$(top_builddir)/include/curl \
              -I$(top_builddir)/include      \
              -I$(top_srcdir)/include        \
              -I$(top_builddir)/lib          \
              -I$(top_srcdir)/lib            \
              -I$(top_builddir)/ares         \
              -I$(top_srcdir)/ares
else
AM_CPPFLAGS = -I$(top_builddir)/include/curl \
              -I$(top_builddir)/include      \
              -I$(top_srcdir)/include        \
              -I$(top_builddir)/lib          \
              -I$(top_srcdir)/lib
endif

# Mostly for Windows build targets, when building libcurl library
if USE_CPPFLAG_BUILDING_LIBCURL
AM_CPPFLAGS += -DBUILDING_LIBCURL
endif

# Mostly for Windows build targets, when building static libcurl
if USE_CPPFLAG_CURL_STATICLIB
AM_CPPFLAGS += -DCURL_STATICLIB
endif

if SONAME_BUMP
#
# Bumping of SONAME conditionally may seem like a weird thing to do, and yeah
# it is. The problem is that we try to avoid the bump as hard as possible, but
# yet it is still necessary for a few rare situations. The configure script will
# attempt to figure out these situations, and it can be forced to consider this
# to be such a case! See README.curl_off_t for further details.
#
# This conditional soname bump SHOULD be removed at next "proper" bump.
#
VERSIONINFO=-version-info 8:0:3
else
VERSIONINFO=-version-info 7:0:3
endif

# This flag accepts an argument of the form current[:revision[:age]]. So,
# passing -version-info 3:12:1 sets current to 3, revision to 12, and age to
# 1.
#
# Here's the simplified rule guide on how to change -version-info:
# (current version is C:R:A)
#
# 1. if there are only source changes, use C:R+1:A
# 2. if interfaces were added use C+1:0:A+1
# 3. if interfaces were removed, then use C+1:0:0
#
# For the full guide on libcurl ABI rules, see docs/libcurl/ABI

if NO_UNDEFINED
# The -no-undefined flag is crucial to build fine on some platforms
UNDEF = -no-undefined
endif

if MIMPURE
# This is for gcc on Solaris (8+ ?) to avoid "relocations remain against
# allocatable but non-writable sections" problems.
MIMPURE = -mimpure-text
endif

if VERSIONED_SYMBOLS
VERSIONED_SYMBOLS = -Wl,--version-script=libcurl.vers
endif

# Prevent LIBS from being used for all link targets
LIBS = $(BLANK_AT_MAKETIME)

libcurl_la_LDFLAGS = $(UNDEF) $(VERSIONINFO) $(MIMPURE) $(VERSIONED_SYMBOLS) $(LIBCURL_LIBS)

if DOING_CURL_SYMBOL_HIDING
libcurl_la_CPPFLAGS = $(AM_CPPFLAGS) -DCURL_HIDDEN_SYMBOLS
libcurl_la_CFLAGS = $(AM_CFLAGS) $(CFLAG_CURL_SYMBOL_HIDING)
else
libcurl_la_CPPFLAGS = $(AM_CPPFLAGS)
libcurl_la_CFLAGS = $(AM_CFLAGS)
endif

# unit testing static library built only along with unit tests
if BUILD_UNITTESTS
noinst_LTLIBRARIES = libcurlu.la
else
noinst_LTLIBRARIES =
endif

libcurlu_la_CPPFLAGS = $(AM_CPPFLAGS) -DUNITTESTS
libcurlu_la_LDFLAGS = -static $(LIBCURL_LIBS)
libcurlu_la_CFLAGS = $(AM_CFLAGS)

# Makefile.inc provides the CSOURCES and HHEADERS defines
include Makefile.inc

libcurl_la_SOURCES = $(CSOURCES) $(HHEADERS)
libcurlu_la_SOURCES = $(CSOURCES) $(HHEADERS)

WIN32SOURCES = $(CSOURCES)
WIN32HEADERS = $(HHEADERS) config-win32.h

DSPOUT = | awk '{printf("%s\r\n", $$0)}' >> $(DSP)
VCPROJOUT = | awk '{printf("%s\r\n", $$0)}' >> $(VCPROJ)

$(DSP): msvcproj.head msvcproj.foot Makefile.am
	echo "creating $(DSP)"
	@(cp $(srcdir)/msvcproj.head $(DSP); \
	echo "# Begin Group \"Source Files\"" $(DSPOUT); \
        echo "" $(DSPOUT); \
        echo "# PROP Default_Filter \"\"" $(DSPOUT); \
        win32_srcs='$(WIN32SOURCES)'; \
        sorted_srcs=`for file in $$win32_srcs; do echo $$file; done | sort`; \
        for file in $$sorted_srcs; do \
	echo "# Begin Source File" $(DSPOUT); \
	echo "" $(DSPOUT); \
	echo "SOURCE=.\\"$$file $(DSPOUT); \
	echo "# End Source File" $(DSPOUT); \
	done; \
	echo "# End Group" $(DSPOUT); \
	echo "# Begin Group \"Header Files\"" $(DSPOUT); \
        echo "" $(DSPOUT); \
        echo "# PROP Default_Filter \"\"" $(DSPOUT); \
        win32_hdrs='$(WIN32HEADERS)'; \
        sorted_hdrs=`for file in $$win32_hdrs; do echo $$file; done | sort`; \
        for file in $$sorted_hdrs; do \
	echo "# Begin Source File" $(DSPOUT); \
	echo "" $(DSPOUT); \
	echo "SOURCE=.\\"$$file $(DSPOUT); \
	echo "# End Source File" $(DSPOUT); \
	done; \
	echo "# End Group" $(DSPOUT); \
	cat $(srcdir)/msvcproj.foot $(DSPOUT) )

$(VCPROJ): vc8proj.head vc8proj.foot Makefile.am
	echo "creating $(VCPROJ)"
	@(cp $(srcdir)/vc8proj.head $(VCPROJ); \
        win32_srcs='$(WIN32SOURCES)'; \
        sorted_srcs=`for file in $$win32_srcs; do echo $$file; done | sort`; \
        for file in $$sorted_srcs; do \
	echo "<File RelativePath=\""$$file"\"></File>" $(VCPROJOUT); \
	done; \
	echo "</Filter><Filter	Name=\"Header Files\">" $(VCPROJOUT); \
        win32_hdrs='$(WIN32HEADERS)'; \
        sorted_hdrs=`for file in $$win32_hdrs; do echo $$file; done | sort`; \
        for file in $$sorted_hdrs; do \
	echo "<File RelativePath=\""$$file"\"></File>" $(VCPROJOUT); \
	done; \
	cat $(srcdir)/vc8proj.foot $(VCPROJOUT) )


checksrc:
	@@PERL@ $(top_srcdir)/lib/checksrc.pl -D$(top_srcdir)/lib $(CSOURCES) $(HHEADERS)

if CURLDEBUG
# for debug builds, we scan the sources on all regular make invokes
all-local: checksrc
endif
