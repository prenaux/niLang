#!/bin/bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ODIR=$SCRIPT_DIR/docs/modules/
cd "$SCRIPT_DIR"
HAM_NO_VER_CHECK=1 source _env_ci.sh

set -e

echo "I/Building modules..."
ham pass1 && ham modlibs

echo "I/Building documentation..."
mkdir -p "$ODIR"
./scripts/tools/build_module_doc.sh "$ODIR" niLang niCURL
./scripts/tools/build_module_doc.sh "$ODIR" niLang niLang
./scripts/tools/build_module_doc.sh "$ODIR" niLang niScript
./scripts/tools/build_module_doc.sh "$ODIR" niLang niSound
./scripts/tools/build_module_doc.sh "$ODIR" niLang niUI

echo "I/Done, see '$ODIR'."
