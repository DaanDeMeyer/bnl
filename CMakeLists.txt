# 3.13 is required for `target_link_options`.
cmake_minimum_required(VERSION 3.13...3.14)

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
  # bnl uses the doctest testing framework. All tests are grouped inside a
  # single executable. Use `target_sources(bnl-test PRIVATE <file>)` to add
  # new tests. Tests are executed by running the build/test executable.
  add_executable(bnl-test)

  bnl_add_common(bnl-test "")
  target_link_libraries(bnl-test PRIVATE doctest_with_main)
  set_target_properties(bnl-test PROPERTIES OUTPUT_NAME test)
endif()

add_subdirectory(bnl/base)
add_subdirectory(bnl/util)
add_subdirectory(bnl/log)
add_subdirectory(bnl/quic)
add_subdirectory(bnl/http3)
