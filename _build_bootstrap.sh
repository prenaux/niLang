#!/bin/bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
export WORK="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." && pwd )"
if [ -z "$HAM_HOME" ]; then
    HAM_HOME=`pwd`/"../ham"
fi
. "$HAM_HOME/bin/ham-bash-setenv.sh"

cd "$SCRIPT_DIR"

. ham-toolset default
errcheck $? _ "Can't set the Default toolset."

set -e

ham BUILD=ra NO_MODULEDEF=1 pass1 build_tools
errcheck $? _ "Can't build 'build_tools' ra."

ham BUILD=ra pass1
errcheck $? _ "Can't build 'pass1' ra."

ham BUILD=ra all
errcheck $? _ "Can't build 'all' ra."
