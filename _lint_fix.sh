#!/bin/bash -e
#===== PRELUDE BEGIN ===========
if [[ -z "$HAM_HOME" ]]; then
  echo "E/HAM_HOME not set !"
  exit 1
fi
# shellcheck disable=SC2034
SCRIPT_NAME=$(basename "$0")
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
cd "$SCRIPT_DIR"
#===== PRELUDE END =============
HAM_LINT_COMMAND=ham_fix_sh
export HAM_LINT_COMMAND
. _lint.sh
