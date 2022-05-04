#!/bin/bash
source ./_env_ci.sh

set -ex
ham BUILD=ra NO_MODULEDEF=1 pass1 build_tools
ham BUILD=ra pass1 all
time ham -- -q Run_ci
