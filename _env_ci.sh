#!/bin/bash -e
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

export WORK="$( cd "$SCRIPT_DIR/.." && pwd )"
if [ -z "$HAM_HOME" ]; then
    export HAM_HOME="$WORK/ham"
fi
. "$HAM_HOME/bin/ham-bash-setenv.sh"

# Get ham if its not already on our system
if [ ! -d "$HAM_HOME" ]; then
    echo "I/HAM_HOME ('$HAM_HOME') folder not found, cloning ham from git..."
    (set -ex ;
     pushd $WORK
     git clone --depth 1 https://github.com/prenaux/ham.git
     popd)
else
    (set -ex ;
     pushd "$HAM_HOME" ;
     PATH="$HAM_HOME/toolsets/repos/":$PATH git-update .
     popd)
fi

if [ -z "$BASH_START_PATH" ]; then
   export BASH_START_PATH="$PATH"
fi

# Setup the toolsets
. hat
