#!/bin/bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"
source _env_ci.sh

set -e

echo "I/Building modules..."
ham pass1 modlibs

echo "I/Building documentation..."
ODIR=$WORK/pr-notes/APIs/niLang
./scripts/tools/build_module_doc.sh "$ODIR" niLang niLang
./scripts/tools/build_module_doc.sh "$ODIR" niLang niCURL
./scripts/tools/build_module_doc.sh "$ODIR" niLang niUI

echo "I/Done, see '$ODIR'."
