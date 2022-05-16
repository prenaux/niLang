#!/bin/bash
export HAM_NO_VER_CHECK=1
if [[ -z "$HAM_HOME" ]]; then echo "E/HAM_HOME not set !"; exit 1; fi
. "$HAM_HOME/bin/ham-bash-setenv.sh"

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

if [ -z "$1" ]; then
    FILES=$(find . -name "*.ni" -o -name "*.nip" -o -name "*.niw")
else
    FILES="$@"
fi

for f in $FILES
do
  ni -c "$f"
done
