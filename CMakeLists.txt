# 3.14 introduced `FetchContent_MakeAvailable` which is used to download
# dependencies at configure time.
cmake_minimum_required(VERSION 3.14)

project(h3c
  VERSION 0.0.1
  DESCRIPTION "A cross-platform C99 HTTP/3 library"
  HOMEPAGE_URL "https://github.com/DaanDeMeyer/h3c"
  LANGUAGES C
)

option(H3C_TEST "Build tests.")

if(H3C_TEST)
  enable_language(CXX)
endif()

include(external/CMakeLists.txt)

if(H3C_TEST)
  # h3c uses the doctest testing framework. All tests are grouped inside a
  # single executable. Use `target_sources(h3c-test PRIVATE <file>)` to add
  # new tests. Tests are executed by running the build/test executable.
  add_executable(h3c-test)

  cddm_add_common(h3c-test CXX 11 "")
  target_link_libraries(h3c-test PRIVATE doctest_with_main)
  set_target_properties(h3c-test PROPERTIES OUTPUT_NAME test)
endif()

add_subdirectory(h3c)
