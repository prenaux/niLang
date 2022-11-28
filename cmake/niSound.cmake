###########################################################################
# build niSound
###########################################################################
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(SOUND_LIB "src/win32")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(SOUND_LIB "src/osx")
elseif(CMAKE_SYSTEM_NAME STREQUAL "JSCC")
  set(SOUND_LIB "src/sdl")
endif()

add_target_library(
  NAME niSound
  SRCS "src" ${SOUND_LIB}
  LIBS niLang
  )
