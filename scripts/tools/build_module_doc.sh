#!/bin/bash
echo "# Building documentation for $1"
mkdir -p "$WORK/_idlgen_doc"
ni -e "$WORK/niLang/scripts/tools/idl_query.ni" doc $1 > "$WORK/_idlgen_doc/$1.md"
