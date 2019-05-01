include(FetchContent)

## cddm ##

FetchContent_Declare(
  cddm
  GIT_REPOSITORY https://github.com/DaanDeMeyer/cddm
  GIT_TAG 006392481d2dcd4fb4256d4c236f2546cf511782
)

FetchContent_MakeAvailable(cddm)

include(${cddm_SOURCE_DIR}/cddm.cmake)

## doctest ##

FetchContent_Declare(
  doctest
  GIT_REPOSITORY https://github.com/onqtam/doctest
  GIT_TAG e1181956453d0044f6c0ba547e9a3d131ba85b42
)

option(DOCTEST_WITH_TESTS "" OFF)
option(DOCTEST_WITH_MAIN_IN_STATIC_LIB "" ON)
option(DOCTEST_NO_INSTALL "" ON)

FetchContent_MakeAvailable(doctest)
