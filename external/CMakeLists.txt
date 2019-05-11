include(FetchContent)

## cddm ##

FetchContent_Declare(
  cddm
  GIT_REPOSITORY https://github.com/DaanDeMeyer/cddm
  GIT_TAG 22024ea82a6b7cc9dcf938f408d08703fc81980c
)

FetchContent_MakeAvailable(cddm)

include(${cddm_SOURCE_DIR}/cddm.cmake)

## doctest ##

FetchContent_Declare(
  doctest
  GIT_REPOSITORY https://github.com/onqtam/doctest
  GIT_TAG 2.3.2
)

option(DOCTEST_WITH_TESTS "" OFF)
option(DOCTEST_WITH_MAIN_IN_STATIC_LIB "" ON)
option(DOCTEST_NO_INSTALL "" ON)

FetchContent_MakeAvailable(doctest)

## xxhash ##

FetchContent_Declare(
  xxhash
  GIT_REPOSITORY https://github.com/Cyan4973/xxHash
  GIT_TAG 22d66e972267852409e8d13565cdfb459e285b70
)

FetchContent_MakeAvailable(xxhash)

add_library(xxhash INTERFACE IMPORTED GLOBAL)
target_compile_definitions(xxhash INTERFACE XXH_INLINE_ALL)
target_include_directories(xxhash INTERFACE ${xxhash_SOURCE_DIR})
