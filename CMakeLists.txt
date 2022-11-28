# How To Build
# 1. install dependences
# brew install llvm && brew install cmake && brew install ninja

# 2. generate build config for ninja
# cmake -H. -G Ninja -Bcmake-build -DCMAKE_TOOLCHAIN_FILE=x86_64-msys.cmake  /* building for windows */
# cmake -H. -G Ninja -Bcmake-build   /* building for macos */

# 3. build
# cmake --build cmake-build
cmake_minimum_required(VERSION 3.17.1)
#
#  PROJECT START
#
include($ENV{HAM_TOOLSET_DIR}/tools.cmake)

project(niLang)
# set(CMAKE_VERBOSE_MAKEFILE ON)
project_init()

add_target(niLang)
add_target(niUI)
add_target(niAppLib)
add_target(niScript)
add_target(niCURL)
add_target(niSound)

###########################################################################
# build Executables
###########################################################################
add_target(ni)
add_target(nicgc)
add_target(HelloUI)

###########################################################################
# build TestCases
###########################################################################
add_target(niUnitTest)
add_target_test(NAME niLang)
add_target_test(NAME niUI)
add_target_test(NAME niCURL)

# add_target(niPainter)
# add_target(niPhysics)
# add_target(niScene)
# add_target(sldAssImp)
# add_target(sldCollada)
list_all_targets()
