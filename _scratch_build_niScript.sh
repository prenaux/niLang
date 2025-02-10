#!/bin/bash
#===== PRELUDE BEGIN ===========
if [[ -z "$HAM_HOME" ]]; then echo "E/HAM_HOME not set !"; exit 1; fi
# shellcheck disable=SC2034
SCRIPT_NAME=$(basename "$0")
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
. "$HAM_HOME/bin/ham-bash-setenv.sh"
#===== PRELUDE END =============
set -e
cd "$SCRIPT_DIR"

rm -Rf "k:/Work/TSGames/_ham/obj/nilang-3/nt-msvc_19_x64-x64-da/niScript/"

time ham niScript BUILD=da
