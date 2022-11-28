###########################################################################
# build nicgc
###########################################################################
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(glslang_os_dir "src/glslang/glslang/OSDependent/Windows")
else()
  set(glslang_os_dir "src/glslang/glslang/OSDependent/Unix")
endif ()

add_target_executable(
  NAME nicgc
  SRCS
  "src"
  "src/glslang/glslang/GenericCodeGen"
  "src/glslang/glslang/MachineIndependent"
  "src/glslang/glslang/MachineIndependent/preprocessor"
  "src/glslang/hlsl"
  "src/glslang/SPIRV"
  "src/glslang/OGLCompilersDLL"
  "src/glslang/StandAlone"
  "src/spirv-cross"
  "src/spirv-tools/source"
  "src/spirv-tools/source/comp"
  "src/spirv-tools/source/opt"
  "src/spirv-tools/source/util"
  "src/spirv-tools/source/val"
  "src/spirv-tools/tools/dis"
  "src/spirv-tools/tools/util"
  "src/spirv-tools"
  "src/spirv-cross/include"
  "src/spirv-tools/include"
  "src/mojoshader"
  ${glslang_os_dir}
  LIBS niLang niCURL
  FLAGS "-DENABLE_HLSL")

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_link_libraries(nicgc PRIVATE dx9sdk2004)
  target_compile_options(nicgc PRIVATE "-Wno-microsoft-exception-spec")
endif()
