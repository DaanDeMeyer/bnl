include(FetchContent)

if(NOT EXISTS ${FETCHCONTENT_BASE_DIR}/outcome)
  file(
    DOWNLOAD
    https://raw.githubusercontent.com/ned14/outcome/v2.1/single-header/outcome-experimental.hpp
    ${FETCHCONTENT_BASE_DIR}/outcome/outcome-experimental.hpp
  )
endif()

add_library(outcome INTERFACE IMPORTED GLOBAL)
target_compile_features(outcome INTERFACE cxx_std_14)
target_include_directories(outcome INTERFACE ${FETCHCONTENT_BASE_DIR}/outcome)

## doctest ##

if(NOT EXISTS ${FETCHCONTENT_BASE_DIR}/doctest)
  file(
    DOWNLOAD
    https://raw.githubusercontent.com/onqtam/doctest/master/doctest/doctest.h
    ${FETCHCONTENT_BASE_DIR}/doctest/doctest.h
  )
endif()

add_library(doctest INTERFACE IMPORTED GLOBAL)
target_compile_features(doctest INTERFACE cxx_std_11)
target_include_directories(doctest INTERFACE ${FETCHCONTENT_BASE_DIR}/doctest)

## fmt ##

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
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
  GIT_REPOSITORY https://github.com/ngtcp2/ngtcp2.git
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

## systemd ##

find_package(PkgConfig)

if(PKG_CONFIG_FOUND)
  pkg_check_modules(systemd IMPORTED_TARGET GLOBAL systemd)
endif()
