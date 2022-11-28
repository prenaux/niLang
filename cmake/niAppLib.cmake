###########################################################################
# build niAppLib
###########################################################################
add_target_library(NAME niAppLib STATIC INCS "../niLang/src/API" "../niUI/src/API" LIBS niLang niUI)
