#!/bin/bash -e
#===== PRELUDE BEGIN ===========
if [[ -z "$HAM_HOME" ]]; then echo "E/HAM_HOME not set !"; exit 1; fi
# shellcheck disable=SC2034
SCRIPT_NAME=$(basename "$0")
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
. "$HAM_HOME/bin/ham-bash-setenv.sh"
#===== PRELUDE TOOLSETS ========
toolset_import_once swift
#===== PRELUDE END =============
cd "$SCRIPT_DIR"

if [ "$#" -ne 1 ]; then
    log_error "usage: $0 [Release/Debug]"
    exit 1
fi

if [ "$1" = "Debug" ]; then
    OUTPUT_FILE="HelloSwift_da"
    COMPILE_FLAGS=(-Onone -g)
elif [ "$1" = "Release" ]; then
    OUTPUT_FILE="HelloSwift_ra"
    COMPILE_FLAGS=(-O)
else
    log_error "Invalid build mode: $1"
    exit 2
fi

(
    set -x
    swiftc "${COMPILE_FLAGS[@]}" -c HelloSwift.swift -o HelloSwift.o
    swiftc -o ${OUTPUT_FILE} HelloSwift.o
)

log_info "Done."
