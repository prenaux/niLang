#!/bin/bash
#===== PRELUDE BEGIN ===========
if [[ -z "$HAM_HOME" ]]; then echo "E/HAM_HOME not set !"; exit 1; fi
# shellcheck disable=SC2034
SCRIPT_NAME=$(basename "$0")
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
. "$HAM_HOME/bin/ham-bash-setenv.sh"
#===== PRELUDE END =============
cd "$SCRIPT_DIR"

set -ex

# log_info "Build rgen from glsl"
# $WORK/Playground/playground_nish @glslang -Od -S rgen --target-env vulkan1.2 -V -o $WORK/niLang/data/test/rayfunc/triangle_rgen.gpufunc.spv_vk12 $WORK/niLang/data/test/rayfunc/triangle_rgen.glsl @nish -S rgen -n triangle_rgen -t glsl_vk46 $WORK/niLang/data/test/rayfunc/triangle_rgen.gpufunc.spv_vk12 $WORK/niLang/data/test/rayfunc/triangle_rgen.gpufunc.xml

# log_info "Build rmiss from glsl"
# $WORK/Playground/playground_nish @glslang -Od -S rmiss --target-env vulkan1.2 -V -o $WORK/niLang/data/test/rayfunc/triangle_rmiss.gpufunc.spv_vk12 $WORK/niLang/data/test/rayfunc/triangle_rmiss.glsl @nish -S rmiss -n triangle_rmiss -t glsl_vk46 $WORK/niLang/data/test/rayfunc/triangle_rmiss.gpufunc.spv_vk12 $WORK/niLang/data/test/rayfunc/triangle_rmiss.gpufunc.xml

# log_info "Build rchit from glsl"
# $WORK/Playground/playground_nish @glslang -Od -S rchit --target-env vulkan1.2 -V -o $WORK/niLang/data/test/rayfunc/triangle_rchit.gpufunc.spv_vk12 $WORK/niLang/data/test/rayfunc/triangle_rchit.glsl @nish -S rchit -n triangle_rchit -t glsl_vk46 $WORK/niLang/data/test/rayfunc/triangle_rchit.gpufunc.spv_vk12 $WORK/niLang/data/test/rayfunc/triangle_rchit.gpufunc.xml

$WORK/Playground/_build_shaders.sh

log_info "Build and run test case"
ham pass1
ham Run_Test_niUI_GDRV FIXTURE=FRayGpu,Triangle A2=-Drenderer=Vulkan BUILD=da
