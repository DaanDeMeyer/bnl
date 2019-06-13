include(FetchContent)

## doctest ##

FetchContent_Declare(
  doctest
  URL https://github.com/onqtam/doctest/archive/2.3.2.tar.gz
)

option(DOCTEST_WITH_TESTS "" OFF)
option(DOCTEST_WITH_MAIN_IN_STATIC_LIB "" ON)
option(DOCTEST_NO_INSTALL "" ON)

FetchContent_MakeAvailable(doctest)

## fmt ##

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  # https://github.com/fmtlib/fmt/pull/1134
  GIT_TAG cbbee1b38569c6394cc1169b5c49fd2640035012
)

option(FMT_INSTALL "" ON)
FetchContent_MakeAvailable(fmt)

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
