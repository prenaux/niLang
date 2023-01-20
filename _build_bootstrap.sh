#!/bin/bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
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
ham all
