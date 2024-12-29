#!/bin/bash
#===== PRELUDE BEGIN ===========
if [[ -z "$HAM_HOME" ]]; then echo "E/HAM_HOME not set !"; exit 1; fi
# shellcheck disable=SC2034
SCRIPT_NAME=$(basename "$0")
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
. "$HAM_HOME/bin/ham-bash-setenv.sh"
#===== PRELUDE END =============
cd "$SCRIPT_DIR"

if [ "$1" = "clean" ]; then
  shift
  (set -ex ; ./_clean_deep.sh doit)
fi

source _env_ci.sh

set -ex
export BUILD=ra
NO_BASETOOLS=1 NO_MODULEDEF=1 ham pass1 build_tools
ham pass1
