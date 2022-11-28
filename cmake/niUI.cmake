###########################################################################
# build niUI
###########################################################################
add_target_library(
  NAME niUI
  SRCS
  "src"
  "src/agg"
  "src/ogg"
  "src/libDXTn"
  "src/libpng"
  "src/mojoshader"
  "src/libtess2"
  "src/GL"
  "src/GLES"
  "src/ft/include"
  "src/jpeglib"
  "src/libDXTn"
  "src/libpng"
  # "src/theoraplayer"
  "../nicgc/src/mojoshader"
  FILES
  #ft
  "src/ft/autofit/autofit.c"
  "src/ft/base/ftbase.c"
  "src/ft/base/ftbitmap.c"
  "src/ft/base/ftdebug.c"
  "src/ft/base/ftglyph.c"
  "src/ft/base/ftinit.c"
  "src/ft/base/ftmm.c"
  "src/ft/base/ftsystem.c"
  "src/ft/bdf/bdf.c"
  "src/ft/cache/ftcache.c"
  "src/ft/cff/cff.c"
  "src/ft/cid/type1cid.c"
  "src/ft/gzip/ftgzip.c"
  "src/ft/lzw/ftlzw.c"
  "src/ft/otvalid/otvalid.c"
  "src/ft/pcf/pcf.c"
  "src/ft/pfr/pfr.c"
  "src/ft/psaux/psaux.c"
  "src/ft/pshinter/pshinter.c"
  "src/ft/psnames/psnames.c"
  "src/ft/raster/raster.c"
  "src/ft/sfnt/sfnt.c"
  "src/ft/smooth/smooth.c"
  "src/ft/truetype/truetype.c"
  "src/ft/type1/type1.c"
  "src/ft/type42/type42.c"
  "src/ft/winfonts/winfnt.c"
  LIBS "niLang"
  FLAGS "-Dagg_real=float")

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  # add dx9sdk
  add_library(dx9sdk2004 STATIC IMPORTED)

  # "$ENV{HAM_HOME}/toolsets/dx9sdk2004/include"
  set(DX9_DIR "$ENV{HAM_HOME}/toolsets/dx9sdk2004/dist")
  set_target_properties(dx9sdk2004 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${DX9_DIR}/include")
  set_target_properties(dx9sdk2004 PROPERTIES IMPORTED_LOCATION "${DX9_DIR}/lib/x64/d3dx9.lib")

  target_link_libraries(niUI PRIVATE dx9sdk2004 legacy_stdio_definitions)
  target_compile_options(niUI PRIVATE "-Wno-microsoft-cast" "-Wno-non-c-typedef-for-linkage")
endif()
