#!/bin/bash
#===== PRELUDE BEGIN ===========
if [[ -z "$HAM_HOME" ]]; then echo "E/HAM_HOME not set !"; exit 1; fi
# shellcheck disable=SC2034
SCRIPT_NAME=$(basename "$0")
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
. "$HAM_HOME/bin/ham-bash-setenv.sh"
#===== PRELUDE END =============
cd "$SCRIPT_DIR"
. hat

echo "I/Build bootstrap..."
./_build_bootstrap.sh

echo "I/Native build..."
PARAMS=("$@")
if [ "${#PARAMS[@]}" -eq 0 ]; then
  PARAMS=(pass1 build_tools all)
fi
(
  set -ex
  BUILD=ra ham -D niLang "${PARAMS[@]}"
  BUILD=da ham -D niLang "${PARAMS[@]}"
)

echo "I/Done."
