###########################################################################
# build niLang
###########################################################################
add_target_library(
  PKG niSDK
  NAME niLang
  SRCS
  "src"
  "src/zlib"
  "src/sq"
  "src/pcre"
  "src/double-conversion"
  "src/mbedtls/library"
  "src/mbedtls/include"
  )

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_link_libraries(niLang PUBLIC "-lwinmm" "-lgdi32" "-luser32" "-lshell32")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_link_libraries(niLang PUBLIC
    "-framework Foundation"
    "-framework Cocoa"
    "-framework IOKit"
    "-framework OpenGL"
    "-framework Metal"
    "-framework MetalKit"
    "-framework QuartzCore"
    "-framework CoreAudio"
    "-framework AudioUnit"
    "-framework Security"
    )
endif()
