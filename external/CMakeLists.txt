include(FetchContent)

## doctest ##

FetchContent_Declare(
  doctest
  URL https://github.com/onqtam/doctest/archive/2.3.2.tar.gz
)

if(BNL_TEST)
  option(DOCTEST_WITH_TESTS "" OFF)
  option(DOCTEST_WITH_MAIN_IN_STATIC_LIB "" ON)
  option(DOCTEST_NO_INSTALL "" ON)

  reproc_MakeAvailable(doctest)
endif()

## fmt ##

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  # https://github.com/fmtlib/fmt/pull/1134
  GIT_TAG e9b9b0aefaf3dacf8554ba93bd404248574945ed
)

option(FMT_INSTALL "" ON)
reproc_MakeAvailable(fmt)

## xxhash ##

FetchContent_Declare(
  xxhash
  GIT_REPOSITORY https://github.com/Cyan4973/xxHash
  GIT_TAG 22d66e972267852409e8d13565cdfb459e285b70
)

reproc_MakeAvailable(xxhash)

add_library(xxhash INTERFACE IMPORTED GLOBAL)
target_compile_definitions(xxhash INTERFACE XXH_INLINE_ALL)
target_include_directories(xxhash INTERFACE ${xxhash_SOURCE_DIR})

## ngtcp2 ##

FetchContent_Declare(
  ngtcp2
  GIT_REPOSITORY https://github.com/DaanDeMeyer/ngtcp2.git
  GIT_TAG draft-22
)

reproc_MakeAvailable(ngtcp2)

## boringssl ##

FetchContent_Declare(
  boringssl
  GIT_REPOSITORY https://boringssl.googlesource.com/boringssl
  GIT_TAG 4a8c05ffe826c61d50fdf13483b35097168faa5c
)

reproc_MakeAvailable(boringssl)

target_include_directories(ssl SYSTEM PUBLIC ${boringssl_SOURCE_DIR}/include)
