#!/bin/bash -e
export HAM_TOOLSET=syslib_libdw
export HAM_TOOLSET_NAME=syslib_libdw
export HAM_TOOLSET_DIR=""

case "$HAM_OS" in
  LINUX*)
    CHECK_HDR="elfutils/libdw.h"
    HAM_HDRS_SYSLIB_LIBDW=$(ham_os_package_syslib_check_and_install include "$CHECK_HDR" apt:libdw-dev libelf)
    if [ ! -e "$HAM_HDRS_SYSLIB_LIBDW" ]; then
      complain "$HAM_TOOLSET" "Cant find '$CHECK_HDR' that should be installed by the binutils package."
      return 1
    fi
    # we want the include directory, not the file itself
    HAM_HDRS_SYSLIB_LIBDW=$(dirname $(dirname "$HAM_HDRS_SYSLIB_LIBDW"))
    export HAM_HDRS_SYSLIB_LIBDW
    ;;
  *)
    log_error "Toolset: $HAM_TOOLSET_NAME: Unsupported OS '$HAM_OS'."
    return 1
    ;;
esac

VER="--- syslib_libdw ----------------------
HAM_HDRS_SYSLIB_LIBDW: $HAM_HDRS_SYSLIB_LIBDW"

export HAM_TOOLSET_VERSIONS="$HAM_TOOLSET_VERSIONS
$VER"
