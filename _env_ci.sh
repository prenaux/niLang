#!/bin/bash -e
SCRIPT_SOURCED=$((return 0 2>/dev/null) && echo yes || echo "")
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

export WORK="$( cd "$SCRIPT_DIR/.." && pwd )"
if [ -z "$HAM_HOME" ]; then
    export HAM_HOME="$WORK/ham"
fi

function source_ham_env() {
    if [ -e "$HAM_HOME/bin/ham-bash-setenv.sh" ]; then
        source "$HAM_HOME/bin/ham-bash-setenv.sh"
    else
        echo "E/Can't find '$HAM_HOME/bin/ham-bash-setenv.sh'"
        return 1
    fi
}

# Get ham if its not already on our system
if [ ! -d "$HAM_HOME" ]; then
    echo "I/HAM_HOME ('$HAM_HOME') folder not found, cloning ham from git..."
    (set -ex ;
     pushd $WORK
     git clone --depth 1 https://github.com/prenaux/ham.git
     popd)
    source_ham_env || return 1
fi

if [ -z "$BASH_START_PATH" ]; then
   export BASH_START_PATH="$PATH"
fi

# Setup the toolsets
. hat
