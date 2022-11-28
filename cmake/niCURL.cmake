###########################################################################
# build niCURL
###########################################################################
add_target_library(NAME niCURL
  SRCS "src" "src/libCURL/src" "src/libCURL/include"
  LIBS niLang)
