#!/bin/bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"
source _env_ci.sh

set -ex
export BUILD=ra
ham NO_MODULEDEF=1 pass1 build_tools
ham pass1
ham all
