#!/bin/bash
#===== PRELUDE BEGIN ===========
if [[ -z "$HAM_HOME" ]]; then echo "E/HAM_HOME not set !"; exit 1; fi
# shellcheck disable=SC2034
SCRIPT_NAME=$(basename "$0")
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
. "$HAM_HOME/bin/ham-bash-setenv.sh"
#===== PRELUDE END =============
set -e
cd "$SCRIPT_DIR"

log_info "Build and run test case"
(
  set -x
  cd "$WORK/niLang"

  export BUILD=da
  export A3=-Dswapinterval=0

  ham Run_Test_niUI FIXTURE=FRayTracer,LitTexturedCube
  # ham Run_Test_niUI FIXTURE=FRayTracer,Triangle
  # ham Run_Test_niUI_GDRV FIXTURE=FGpu,Triangle
  # ham Run_Test_niUI_GDRV FIXTURE=FRayPipeline,Quad
  # ham Run_Test_niUI_GDRV FIXTURE=FRayPipeline,Triangle
  # ham Run_Test_niUI_GDRV FIXTURE=FRayQuery,IntSphere
)
