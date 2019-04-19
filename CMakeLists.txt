# 3.13 is needed for to allow using `target_link_libraries` to modify targets
# created outside the current directory.
cmake_minimum_required(VERSION 3.13...3.14)

project(h3c
  VERSION 0.0.1
  DESCRIPTION "A cross-platform C99 HTTP/3 library"
  HOMEPAGE_URL "https://github.com/DaanDeMeyer/h3c"
  LANGUAGES C
)

option(H3C_TESTS "Build tests")

if(H3C_TESTS)
  enable_language(CXX)
endif()

include(external/cddm/cddm.cmake)

if(H3C_TESTS)
  add_subdirectory(external/doctest)

  # h3c uses the doctest testing framework. All tests are grouped inside a
  # single executable. Use `target_sources(h3c-tests PRIVATE <file>)` to add
  # new tests. Tests are executed by running the build/tests executable.
  add_executable(h3c-tests)

  cddm_add_common(h3c-tests CXX 11 "")
  set_target_properties(h3c-tests PROPERTIES OUTPUT_NAME tests)
  target_link_libraries(h3c-tests PRIVATE doctest)
endif()

add_subdirectory(h3c)



