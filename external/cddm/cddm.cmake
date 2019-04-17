# CDDM (CMake Daan De Meyer)
# Version: v0.0.8
#
# Description: Encapsulates common CMake configuration for cross-platform
# C/C++ libraries.
#
# Features:
# - Warnings
#   - UNIX: -Wall, -Wextra, ...
#   - Windows: /W4
# - Sanitizers (optional, UNIX only)
# - clang-tidy (optional)
# - Automatic installation including pkg-config and CMake config files.
# - Automatic export header generation.
#
# Options:
#
# Every option is prefixed with the upper cased project name. For example, if
# the project is named reproc every option is prefixed with `REPROC_`.
#
# Installation options:
# - `INSTALL`: Generate installation rules (default: `ON` unless
#   `BUILD_SHARED_LIBS` is false and the project is built via
#   `add_subdirectory`).
# - `INSTALL_CMAKECONFIGDIR`: CMake config files installation directory
#   (default: `${CMAKE_INSTALL_LIBDIR}/cmake`).
# - `INSTALL_PKGCONFIG`: Install pkg-config files (default: `ON`)
# - `INSTALL_PKGCONFIGDIR`: pkg-config files installation directory
#   (default: `${CMAKE_INSTALL_LIBDIR}/pkgconfig`).
#
# Developer options:
# - `SANITIZERS`: Build with sanitizers (default: `OFF`).
# - `TIDY`: Run clang-tidy when building (default: `OFF`).
# - `WARNINGS_AS_ERRORS`: Add -Werror or equivalent to the compile flags and
#   clang-tidy (default: `OFF`).
#
# Functions:
# - `cddm_add_common`
# - `cddm_add_library`
#
# See https://github.com/DaanDeMeyer/reproc for an example on how to use cddm.
#
# NOTE: All languages used have to be enabled before including cddm.

# CMake 3.13 added target_link_options.
cmake_minimum_required(VERSION 3.13)

set(PNL ${PROJECT_NAME}) # PROJECT_NAME_LOWER (PNL)
string(TOUPPER ${PROJECT_NAME} PNU) # PROJECT_NAME_UPPER (PNU)

### Installation options ###

get_directory_property(CDDM_IS_SUBDIRECTORY PARENT_DIRECTORY)

# Don't add libraries to the install target by default if the project is built
# from within another project as a static library.
if(CDDM_IS_SUBDIRECTORY AND NOT BUILD_SHARED_LIBS)
  option(${PNU}_INSTALL "Generate installation rules." OFF)
else()
  option(${PNU}_INSTALL "Generate installation rules." ON)
endif()

mark_as_advanced(${PNU}_INSTALL)

include(GNUInstallDirs)

option(${PNU}_INSTALL_PKGCONFIG "Install pkg-config files." ON)

set(${PNU}_INSTALL_CMAKECONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake
    CACHE STRING "CMake config files installation directory.")
set(${PNU}_INSTALL_PKGCONFIGDIR ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    CACHE STRING "pkg-config files installation directory.")

mark_as_advanced(
  ${PNU}_INSTALL
  ${PNU}_INSTALL_PKGCONFIG
  ${PNU}_INSTALL_CMAKECONFIGDIR
  ${PNU}_INSTALL_PKGCONFIGDIR
)

### Developer options ###

option(${PNU}_TIDY "Run clang-tidy when building.")
option(${PNU}_SANITIZERS "Build with sanitizers.")
option(${PNU}_WARNINGS_AS_ERRORS "Add -Werror or equivalent to the compile flags and clang-tidy.")

mark_as_advanced(
  ${PNU}_TIDY
  ${PNU}_SANITIZERS
  ${PNU}_WARNINGS_AS_ERRORS
)

### clang-tidy ###

if(${PNU}_TIDY)
  find_program(CDDM_CLANG_TIDY_PROGRAM clang-tidy)
  mark_as_advanced(CDDM_CLANG_TIDY_PROGRAM)

  if(CDDM_CLANG_TIDY_PROGRAM)
    if(${PNU}_WARNINGS_AS_ERRORS)
      set(CDDM_CLANG_TIDY_PROGRAM
          ${CDDM_CLANG_TIDY_PROGRAM} -warnings-as-errors=*)
    endif()
  else()
    message(FATAL_ERROR "clang-tidy not found")
  endif()
endif()

### Global Setup ###

foreach(LANGUAGE IN ITEMS C CXX)
  if(NOT LANGUAGE IN_LIST ENABLED_LANGUAGES)
    continue()
  endif()

  if(MSVC)
    # CMake adds /W3 to CMAKE_C_FLAGS and CMAKE_CXX_FLAGS by default which
    # results in cl.exe warnings if we add /W4 as well. To avoid these
    # warnings we replace /W3 with /W4 instead.
    string(REGEX REPLACE
      "[-/]W[1-4]" ""
      CMAKE_${LANGUAGE}_FLAGS
      "${CMAKE_${LANGUAGE}_FLAGS}"
    )
    set(CMAKE_${LANGUAGE}_FLAGS "${CMAKE_${LANGUAGE}_FLAGS} /W4")

    if(LANGUAGE STREQUAL "C")
      include(CheckCCompilerFlag)
      check_c_compiler_flag(/permissive- CDDM_${LANGUAGE}_HAVE_PERMISSIVE)
    else()
      include(CheckCXXCompilerFlag)
      check_cxx_compiler_flag(/permissive- CDDM_${LANGUAGE}_HAVE_PERMISSIVE)
    endif()
  endif()

  if(${PNU}_SANITIZERS)
    if(MSVC)
      message(FATAL_ERROR "Building with sanitizers is not supported when using the Visual C++ toolchain.")
    endif()

    if(NOT ${CMAKE_${LANGUAGE}_COMPILER_ID} MATCHES GNU|Clang)
      message(FATAL_ERROR "Building with sanitizers is not supported when using the ${CMAKE_${LANGUAGE}_COMPILER_ID} compiler.")
    endif()
  endif()
endforeach()

### Includes ###

include(GenerateExportHeader)
include(CMakePackageConfigHelpers)

### Functions ###

# Applies common configuration to `TARGET`. `LANGUAGE` (C or CXX) is used to
# indicate the language of the target. `STANDARD` indicates the standard of the
# language to use and `OUTPUT_DIRECTORY` defines where to put the resulting
# files.
function(cddm_add_common TARGET LANGUAGE STANDARD OUTPUT_DIRECTORY)
  if(LANGUAGE STREQUAL "C")
    target_compile_features(${TARGET} PUBLIC c_std_${STANDARD})
  else()
    target_compile_features(${TARGET} PUBLIC cxx_std_${STANDARD})
  endif()

  set_target_properties(${TARGET} PROPERTIES
    ${LANGUAGE}_EXTENSIONS OFF

    # Only one of these is actually used per target but instead of passing the
    # type of target to the function and setting only the appropriate property
    # we just set all of them to avoid lots of if checks and an extra function
    # parameter.
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
    ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
  )

  if(${PNU}_TIDY AND CDDM_CLANG_TIDY_PROGRAM)
    set_target_properties(${TARGET} PROPERTIES
      # CLANG_TIDY_PROGRAM is a list so we surround it with quotes to pass it as
      # a single argument.
      ${LANGUAGE}_CLANG_TIDY "${CDDM_CLANG_TIDY_PROGRAM}"
    )
  endif()

  ### Common development flags (warnings + sanitizers + colors) ###

  if(MSVC)
    target_compile_options(${TARGET} PRIVATE
      /nologo # Silence MSVC compiler version output.
      /wd4068
      $<$<BOOL:${${PNU}_WARNINGS_AS_ERRORS}>:/WX> # -Werror
      $<$<BOOL:${CDDM_${LANGUAGE}_HAVE_PERMISSIVE}>:/permissive->
    )

    if(NOT STANDARD STREQUAL "90")
      # MSVC reports non-constant initializers as a nonstandard extension but
      # they've been standardized in C99 so we disable it if we're targeting at
      # least C99.
      target_compile_options(${TARGET} PRIVATE /wd4204)
    endif()

    target_link_options(${TARGET} PRIVATE
      /nologo # Silence MSVC linker version output.
    )
  else()
    target_compile_options(${TARGET} PRIVATE
      -Wall
      -Wextra
      -pedantic-errors
      -Wshadow
      -Wconversion
      -Wsign-conversion
      $<$<BOOL:${${PNU}_WARNINGS_AS_ERRORS}>:-Werror>
    )
  endif()

  if(${PNU}_SANITIZERS)
    target_compile_options(${TARGET} PRIVATE
      -fsanitize=address,undefined
    )
    target_link_options(${TARGET} PRIVATE
      -fsanitize=address,undefined
      # GCC sanitizers only work when using the gold linker.
      $<$<${LANGUAGE}_COMPILER_ID:GNU>:-fuse-ld=gold>
    )
  endif()

  target_compile_options(${TARGET} PRIVATE
    $<$<${LANGUAGE}_COMPILER_ID:GNU>:-fdiagnostics-color>
    $<$<${LANGUAGE}_COMPILER_ID:Clang>:-fcolor-diagnostics>
  )
endfunction()

# Adds a new library with name `TARGET` and applies common configuration to it.
# `LANGUAGE` and `STANDARD` define the language and corresponding standard used
# by the target.
#
# An export header is generated and made available as follows (assuming the
# target is named reproc):
# - `LANGUAGE == C` => `#include <reproc/export.h>`
# - `LANGUAGE == CXX` => `#include <reproc/export.hpp>`
#
# The export header for reproc includes the `REPROC_EXPORT` macro which can be
# applied to any public API functions.
function(cddm_add_library TARGET LANGUAGE STANDARD)
  add_library(${TARGET})

  cddm_add_common(${TARGET} ${LANGUAGE} ${STANDARD} lib)
  # Enable -fvisibility=hidden and -fvisibility-inlines-hidden (if applicable).
  set_target_properties(${TARGET} PROPERTIES
    ${LANGUAGE}_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN true
  )

  # A preprocesor macro cannot contain + so we replace it with x.
  string(REPLACE + x EXPORT_MACRO ${TARGET})
  string(TOUPPER ${EXPORT_MACRO} EXPORT_MACRO_UPPER)

  if(LANGUAGE STREQUAL "C")
    set(HEADER_EXT h)
  else()
    set(HEADER_EXT hpp)
  endif()

  # Generate export headers. We generate export headers using CMake since
  # different export files are required depending on whether a library is shared
  # or static and we can't determine whether a library is shared or static from
  # the export header without requiring the user to add a #define which we want
  # to avoid.
  generate_export_header(${TARGET}
    BASE_NAME ${EXPORT_MACRO_UPPER}
    EXPORT_FILE_NAME
      ${CMAKE_CURRENT_BINARY_DIR}/include/${TARGET}/export.${HEADER_EXT}
  )

  # Make sure we follow the popular naming convention for shared libraries on
  # UNIX systems.
  set_target_properties(${TARGET} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
  )

  # Only use the headers from the repository when building. When installing we
  # want to use the install location of the headers (e.g. /usr/include) as the
  # include directory instead.
  target_include_directories(${TARGET} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  )

  # Adapted from https://codingnest.com/basic-cmake-part-2/.
  # Each library is installed separately (with separate config files).

  if(${PNU}_INSTALL)

    ## Headers

    install(
      DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/${TARGET}
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(
      DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/${TARGET}
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    target_include_directories(${TARGET} PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    ## Libraries

    install(
      TARGETS ${TARGET}
      EXPORT ${TARGET}-targets
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    ## Config files

    # CMake

    install(
      EXPORT ${TARGET}-targets
      FILE ${TARGET}-targets.cmake
      DESTINATION ${${PNU}_INSTALL_CMAKECONFIGDIR}/${TARGET}
    )

    write_basic_package_version_file(
      ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}-config-version.cmake
      VERSION ${PROJECT_VERSION}
      COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}-config.cmake
      INSTALL_DESTINATION
        ${${PNU}_INSTALL_CMAKECONFIGDIR}/${TARGET}
    )

    install(
      FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}-config-version.cmake
      DESTINATION
        ${${PNU}_INSTALL_CMAKECONFIGDIR}/${TARGET}
    )

    # pkg-config

    if(${PNU}_INSTALL_PKGCONFIG)
      configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.pc
        @ONLY
      )

      install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.pc
        DESTINATION ${${PNU}_INSTALL_PKGCONFIGDIR}
      )
    endif()
  endif()
endfunction()
