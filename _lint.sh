#!/bin/bash
export HAM_NO_VER_CHECK=1
if [[ -z "$HAM_HOME" ]]; then echo "E/HAM_HOME not set !"; exit 1; fi
. "$HAM_HOME/bin/ham-bash-setenv.sh"

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

function lint_ni_script() {
  if [ -d "$1" ]; then
    FILES=$(rg -g '*.ni' -g '*.nip' -g '*.niw' -g '!**/niScript/tsrc_ni/linter_*.ni' --files "$1")
    for f in $FILES
    do
      (
        set -x
        ni -c "$f"
      )
    done
  else
    (
      set -x
      ni -c "$1"
    )
  fi
}

if [ -z "$1" ]; then
  lint_ni_script "$(pwd)"
else
  for arg in "$@"; do
    lint_ni_script "${arg}"
  done
fi
