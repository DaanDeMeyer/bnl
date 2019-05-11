include(FetchContent)

## cddm ##

FetchContent_Declare(
  cddm
  GIT_REPOSITORY https://github.com/DaanDeMeyer/cddm
  GIT_TAG 09d16f48ab53e3e4a8f2bdea5d39288b4206fb3f
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
