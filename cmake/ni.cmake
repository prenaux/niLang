###########################################################################
# build ni/niw
###########################################################################
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(RESOURCE "src/resource.rc")
endif()

add_target_executable(
  NAME ni
  SRCS "."
  FILES ${RESOURCE} "src/ni_console.cpp" "src/ni_repl.cpp"
  LIBS niLang niScript)

add_target_executable(
  NAME niw
  DIR "sources/ni"
  SRCS "."
  FILES ${RESOURCE} "src/ni_windowed.cpp"
  LIBS niLang niScript niAppLib niUI)
