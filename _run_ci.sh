#!/bin/bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

source ./_env_ci.sh

export BUILD=${BUILD:-ra}
(set -ex ;
 ham NO_MODULEDEF=1 pass1 build_tools ;
 ham pass1 all)

if [[ "${GITHUB_ACTIONS}" == "true" ]]; then
    echo "I/Running in GITHUB_ACTIONS..."
    (set -ex ;
     ham Run_Test_niLang ;
     ham Run_Test_niCURL ;
     ham Run_Test_niScript_ni ;
     ham Run_Test_niScriptCpp ;
     ham Run_Test_niThrift)
else
    echo "I/Running full CI..."
    (set -ex ; time ham -- -j1 Run_ci)
fi
