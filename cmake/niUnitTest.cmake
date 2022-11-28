###########################################################################
# build niUnitTest
###########################################################################
add_target_library(NAME niUnitTest
  STATIC
  INCS "../niLang/src/API" "../niUI/src/API"
  APIS "src/API" "../niAppLib/src/API"
  LIBS niLang niAppLib)
