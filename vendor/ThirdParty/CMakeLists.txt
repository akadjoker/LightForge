cmake_minimum_required(VERSION 3.16)
project(ThirdPartyAgg LANGUAGES C CXX)

# Helpers (se o original já define, isto é inofensivo)
if(NOT COMMAND setup_project_group)
  function(setup_project_group sources base_dir)
    if(sources)
      source_group(TREE "${base_dir}" FILES ${sources})
    endif()
  endfunction()
endif()

if(NOT COMMAND add_source_group)
  function(add_source_group sources prefix)
    if(sources)
      source_group("${prefix}" FILES ${sources})
    endif()
  endfunction()
endif()

# --- toggles úteis ---
option(THIRDPARTY_USE_PNG_ASM "Compilar ficheiros .S do libpng" ON)

set(THIRD_ROOT ${CMAKE_CURRENT_LIST_DIR})

# ===================== ZLIB =====================
file(GLOB_RECURSE zlib_source
  ${THIRD_ROOT}/zlib/*.c
  ${THIRD_ROOT}/zlib/*.h
)
setup_project_group("${zlib_source}" ${CMAKE_CURRENT_SOURCE_DIR})

# ===================== LIBPNG ===================
file(GLOB_RECURSE libpng_source
  ${THIRD_ROOT}/libpng/*.cpp
  ${THIRD_ROOT}/libpng/*.hpp
  ${THIRD_ROOT}/libpng/*.c
  ${THIRD_ROOT}/libpng/*.h
  ${THIRD_ROOT}/libpng/*.S
)
# Em ARM podes desligar ASM do png
if(NOT THIRDPARTY_USE_PNG_ASM)
  list(FILTER libpng_source EXCLUDE REGEX "\\.S$")
endif()
setup_project_group("${libpng_source}" ${CMAKE_CURRENT_SOURCE_DIR})

# ===================== JPEGLIB ==================
file(GLOB_RECURSE jpeglib_source
  ${THIRD_ROOT}/jpeglib/*.cpp
  ${THIRD_ROOT}/jpeglib/*.hpp
  ${THIRD_ROOT}/jpeglib/*.c
  ${THIRD_ROOT}/jpeglib/*.h
)
setup_project_group("${jpeglib_source}" ${CMAKE_CURRENT_SOURCE_DIR})

# ===================== FREETYPE (curado) =========
# Usa a lista EXACTA do original (sem globbing)
set(freetype_source
  ${THIRD_ROOT}/freetype2/src/autofit/autofit.c
  ${THIRD_ROOT}/freetype2/src/base/basepic.c
  ${THIRD_ROOT}/freetype2/src/base/ftapi.c
  ${THIRD_ROOT}/freetype2/src/base/ftbase.c
  ${THIRD_ROOT}/freetype2/src/base/ftbbox.c
  ${THIRD_ROOT}/freetype2/src/base/ftbitmap.c
  ${THIRD_ROOT}/freetype2/src/base/ftdbgmem.c
  ${THIRD_ROOT}/freetype2/src/base/ftdebug.c
  ${THIRD_ROOT}/freetype2/src/base/ftglyph.c
  ${THIRD_ROOT}/freetype2/src/base/ftinit.c
  ${THIRD_ROOT}/freetype2/src/base/ftpic.c
  ${THIRD_ROOT}/freetype2/src/base/ftstroke.c
  ${THIRD_ROOT}/freetype2/src/base/ftsynth.c
  ${THIRD_ROOT}/freetype2/src/base/ftsystem.c
  ${THIRD_ROOT}/freetype2/src/cff/cff.c
  ${THIRD_ROOT}/freetype2/src/pshinter/pshinter.c
  ${THIRD_ROOT}/freetype2/src/psaux/psaux.c
  ${THIRD_ROOT}/freetype2/src/psnames/psnames.c
  ${THIRD_ROOT}/freetype2/src/raster/raster.c
  ${THIRD_ROOT}/freetype2/src/sfnt/sfnt.c
  ${THIRD_ROOT}/freetype2/src/smooth/smooth.c
  ${THIRD_ROOT}/freetype2/src/truetype/truetype.c
)
add_source_group("${freetype_source}" "source\\freetype2")

# ===================== LZMA / BZIP2 (se tiveres) =========
file(GLOB_RECURSE lzma_source  ${THIRD_ROOT}/lzma/*.c  ${THIRD_ROOT}/lzma/*.h)
setup_project_group("${lzma_source}" ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE bzip2_source ${THIRD_ROOT}/bzip2/*.c ${THIRD_ROOT}/bzip2/*.h)
setup_project_group("${bzip2_source}" ${CMAKE_CURRENT_SOURCE_DIR})

# ===================== AES Gladman / KDTREE =================
file(GLOB_RECURSE aes_source   ${THIRD_ROOT}/aesGladman/*.[ch] ${THIRD_ROOT}/aesGladman/*.cpp)
setup_project_group("${aes_source}" ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE kdtree_source ${THIRD_ROOT}/kdtree/*.[ch] ${THIRD_ROOT}/kdtree/*.cpp)
setup_project_group("${kdtree_source}" ${CMAKE_CURRENT_SOURCE_DIR})

# ===================== JSON (HEADERS-ONLY!) =================
# NÃO misturar json nos includes do alvo C; cria alvo INTERFACE separado
file(GLOB_RECURSE json_source
  ${THIRD_ROOT}/json/*.hpp
  ${THIRD_ROOT}/json/*.h
  ${THIRD_ROOT}/json/*.cpp   # se houver utils em C++, ficam para targets C++
)
setup_project_group("${json_source}" ${CMAKE_CURRENT_SOURCE_DIR})

add_library(ThirdPartyJSON INTERFACE)
target_include_directories(ThirdPartyJSON INTERFACE
  ${THIRD_ROOT}/json
)

# ===================== Biblioteca agregada ==================
add_library(ThirdParty STATIC
  ${zlib_source}
  ${libpng_source}
  ${jpeglib_source}
  ${freetype_source}
  ${lzma_source}
  ${bzip2_source}
  ${aes_source}
  ${kdtree_source}
)
set_target_properties(ThirdParty PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Includes **apenas** das libs C/C++ (sem json)
target_include_directories(ThirdParty PUBLIC
  ${THIRD_ROOT}/zlib
  ${THIRD_ROOT}/libpng
  ${THIRD_ROOT}/jpeglib
  ${THIRD_ROOT}/freetype2/include
  ${THIRD_ROOT}/freetype2
  ${THIRD_ROOT}/lzma
  ${THIRD_ROOT}/bzip2
  ${THIRD_ROOT}/aesGladman
  ${THIRD_ROOT}/kdtree
)

# Definições necessárias
target_compile_definitions(ThirdParty PUBLIC
  FT2_BUILD_LIBRARY
  PNG_NO_STDIO=0
)

# Em Linux/Unix, algumas libs C pedem -lm
if(UNIX)
  target_link_libraries(ThirdParty PUBLIC m)
endif()

 