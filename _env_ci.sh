#!/bin/bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

export WORK="$( cd "$SCRIPT_DIR/.." && pwd )"
export GRADLE_USER_HOME="$WORK/_gradle"
export TEMPDIR="$WORK/_ham"
mkdir -p "$TEMPDIR"

if [ -z "$HAM_HOME" ]; then
    export HAM_HOME="$WORK/ham"
fi

# Get ham if its not already on our system
if [ ! -d "$HAM_HOME" ]; then
    echo "I/HAM_HOME ('$HAM_HOME') folder not found, cloning ham from git..."
    set -ex
    pushd $WORK
    git clone https://github.com/prenaux/ham.git
    popd
    set +ex
fi

if [ -z "$BASH_START_PATH" ]; then
   export BASH_START_PATH="$PATH"
fi

# Set the ham environment
. "$HAM_HOME/bin/ham-bash-setenv.sh"

# Setup the toolsets
. hat
