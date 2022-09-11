#!/bin/bash -e
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

usage() {
    echo ""
    echo "usage: ${0##*/} OUT_DIR TOOLKIT MODULE_NAME"
    echo ""
    exit 1
}

if [ -z "$1" ]; then
  echo "E/OUT_DIR not specified."
  usage
fi
OUT_DIR="$1"
shift

if [ -z "$1" ]; then
  echo "E/TOOLKIT not specified."
  usage
fi
TOOLKIT="$1"
shift

if [ -z "$1" ]; then
  echo "E/MODULE_NAME not specified."
  usage
fi
MODULE_NAME="$1"
shift

echo "# Building documentation for $TOOLKIT :: $MODULE_NAME"
mkdir -p "$OUT_DIR"

(set -x ; ni -e "$WORK/niLang/scripts/tools/idl_query.ni" doc $MODULE_NAME > "$OUT_DIR/$MODULE_NAME.md")
