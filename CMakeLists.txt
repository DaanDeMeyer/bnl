
if(WIN32)
  # 3.15 is required for clang with GNU command line and non-default /W3.
  cmake_minimum_required(VERSION 3.15)
else()
  # 3.13 is required for `target_link_options`.
  cmake_minimum_required(VERSION 3.13...3.15)
endif()

project(bnl
  VERSION 0.0.1
  DESCRIPTION "Binary Networking Library"
  HOMEPAGE_URL "https://github.com/DaanDeMeyer/bnl"
  LANGUAGES CXX
)

option(BNL_TEST "Build tests.")

include(cmake/bnl.cmake)

add_subdirectory(external)

if(BNL_TEST)
  # Generate the doctest implementation file.
  file(
    WRITE
    ${PROJECT_BINARY_DIR}/_deps/doctest/impl.cpp
    "#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN\n#include <doctest.h>"
  )

  # bnl uses the doctest testing framework. All tests are grouped inside a
  # single executable. Use `target_sources(bnl-test PRIVATE <file>)` to add
  # new tests. Tests are executed by running the build/test executable.
  add_executable(bnl-test ${PROJECT_BINARY_DIR}/_deps/doctest/impl.cpp)

  bnl_add_common(bnl-test "")
  target_link_libraries(bnl-test PRIVATE doctest)
  set_target_properties(bnl-test PROPERTIES OUTPUT_NAME test)
endif()

add_subdirectory(bnl/base)
add_subdirectory(bnl/log)
add_subdirectory(bnl/quic)
add_subdirectory(bnl/http3)
