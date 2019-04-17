# 3.13 is needed for to allow using `target_link_libraries` to modify targets
# created outside the current directory.
cmake_minimum_required(VERSION 3.13...3.14)

project(h3c
  VERSION 0.0.1
  DESCRIPTION "A cross-platform C99 HTTP/3 library"
  HOMEPAGE_URL "https://github.com/DaanDeMeyer/h3c"
  LANGUAGES C
)

include(external/cddm/cddm.cmake)

add_subdirectory(h3c)



