#!/bin/bash
source ./_env_ci.sh

set -ex
export BUILD=ra
ham NO_MODULEDEF=1 pass1 build_tools
ham pass1 all
time ham -- -j1 -q Run_ci
