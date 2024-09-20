#!/bin/bash -e
export HAM_TOOLSET=syslib_bfd
export HAM_TOOLSET_NAME=syslib_bfd
export HAM_TOOLSET_DIR=""

case "$HAM_OS" in
  LINUX*)
    HAM_HDRS_SYSLIB_BFD=$(ham_os_package_syslib_check_and_install include "bfd.h" apt:binutils-dev binutils)
    if [ ! -e "$HAM_HDRS_SYSLIB_BFD" ]; then
      complain "$HAM_TOOLSET" "Cant find 'bfd.h' that should be installed by the binutils package."
      return 1
    fi
    # we want the include directory, not the file itself
    HAM_HDRS_SYSLIB_BFD=$(dirname "$HAM_HDRS_SYSLIB_BFD")
    export HAM_HDRS_SYSLIB_BFD
    ;;
  *)
    log_error "Toolset: $HAM_TOOLSET_NAME: Unsupported OS '$HAM_OS'."
    return 1
    ;;
esac

VER="--- syslib_bfd ----------------------
HAM_HDRS_SYSLIB_BFD: $HAM_HDRS_SYSLIB_BFD"

export HAM_TOOLSET_VERSIONS="$HAM_TOOLSET_VERSIONS
$VER"
