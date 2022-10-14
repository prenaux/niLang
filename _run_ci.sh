#!/bin/bash
source ./_env_ci.sh

set -ex
export BUILD=${BUILD:-ra}
ham NO_MODULEDEF=1 pass1 build_tools
ham pass1 all
time ham -- -j1 Run_ci
