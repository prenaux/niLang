#!/bin/bash
#===== PRELUDE BEGIN ===========
if [[ -z "$HAM_HOME" ]]; then
  echo "E/HAM_HOME not set !"
  exit 1
fi
# shellcheck disable=SC2034
SCRIPT_NAME=$(basename "$0")
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
. "$HAM_HOME/bin/ham-bash-setenv.sh"
#===== PRELUDE END =============
set -e
cd "$SCRIPT_DIR"

(
  set -x
  cd "$WORK/niLang"

  export BUILD=ra
  export A0=-DdrawFPS=1
  export A3=-Dswapinterval=0

  #ham Run_Test_niUI FIXTURE=FRayTracer,LitTexturedCube
  #ham Run_Test_niUI_niui_ListBox
  ham Run_Test_niUI_niui_Text
)
