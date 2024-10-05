#!/bin/bash -e
export HAM_TOOLSET=syslib_alsa
export HAM_TOOLSET_NAME=syslib_alsa
export HAM_TOOLSET_DIR=""

case "$HAM_OS" in
  LINUX*)
    CHECK_HDR="alsa/asoundlib.h"
    HAM_HDRS_SYSLIB_ALSA=$(ham_os_package_syslib_check_and_install include "$CHECK_HDR" apt:alsa-dev alsa-lib)
    if [ ! -e "$HAM_HDRS_SYSLIB_ALSA" ]; then
      complain "$HAM_TOOLSET" "Cant find '$CHECK_HDR' that should be installed by the binutils package."
      return 1
    fi
    # we want the include directory, not the file itself
    HAM_HDRS_SYSLIB_ALSA=$(dirname $(dirname "$HAM_HDRS_SYSLIB_ALSA"))
    export HAM_HDRS_SYSLIB_ALSA
    ;;
  *)
    log_error "Toolset: $HAM_TOOLSET_NAME: Unsupported OS '$HAM_OS'."
    return 1
    ;;
esac

VER="--- syslib_alsa ----------------------
HAM_HDRS_SYSLIB_ALSA: $HAM_HDRS_SYSLIB_ALSA"

export HAM_TOOLSET_VERSIONS="$HAM_TOOLSET_VERSIONS
$VER"
