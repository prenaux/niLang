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

log_info "Compile shaders"
(
  set -x
  cd "$WORK/Playground"
  # hamx :niLang :Playground ni
  # ./_build_shaders.sh
)

log_info "Build and run test case"
(
  set -x
  cd "$WORK/niLang"
  # ham Run_Test_niUI_GDRV FIXTURE=FRay,Triangle A2=-Drenderer=Vulkan BUILD=da
  # ham Run_Test_niUI_GDRV BUILD=da FIXTURE=FRay,RayQueryTriangle
  # ham Run_Test_niUI_GDRV BUILD=da FIXTURE=FRay,RayQueryIntSphere
  # ham Debug_Test_niUI BUILD=da FIXTURE=FRayTracer,Triangle
  # ham Debug_Test_niUI BUILD=da FIXTURE=FRayTracer,Triangle
  ham Run_Test_niUI BUILD=ra FIXTURE=FRayTracer,Triangle
  # ham Run_Test_niUI_GDRV BUILD=ra FIXTURE=FRay,RayQueryTriangle
)
