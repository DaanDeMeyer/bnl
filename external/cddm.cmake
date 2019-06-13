# CDDM (CMake Daan De Meyer)
# Version: v0.0.20
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

if(MSVC)
  # CMake adds /W3 to CMAKE_C_FLAGS and CMAKE_CXX_FLAGS by default which
  # results in cl.exe warnings if we add /W4 as well. To avoid these
  # warnings we replace /W3 with /W4 instead.
  string(REGEX REPLACE
    "[-/]W[1-4]" ""
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS}"
  )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")

  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag(/permissive- CDDM_HAVE_PERMISSIVE)
endif()

if(${PNU}_SANITIZERS)
  if(MSVC)
    message(FATAL_ERROR "Building with sanitizers is not supported when using the Visual C++ toolchain.")
  endif()

  if(NOT ${CMAKE_CXX_COMPILER_ID} MATCHES GNU|Clang)
    message(FATAL_ERROR "Building with sanitizers is not supported when using the ${CMAKE_CXX_COMPILER_ID} compiler.")
  endif()
endif()

include(GenerateExportHeader)

### Includes ###

include(CMakePackageConfigHelpers)

### Functions ###

# Applies common configuration to `TARGET`. `OUTPUT_DIRECTORY` specifies the
# target's output directory.
function(cddm_add_common TARGET OUTPUT_DIRECTORY)
  target_compile_features(${TARGET} PUBLIC cxx_std_11)

  set_target_properties(${TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
    ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
  )

  if(${PNU}_TIDY AND CDDM_CLANG_TIDY_PROGRAM)
    set_target_properties(${TARGET} PROPERTIES
      # CLANG_TIDY_PROGRAM is a list so we surround it with quotes to pass it as
      # a single argument.
      CXX_CLANG_TIDY "${CDDM_CLANG_TIDY_PROGRAM}"
    )
  endif()

  ### Common development flags (warnings + sanitizers + colors) ###

  if(MSVC)
    target_compile_options(${TARGET} PRIVATE
      /nologo # Silence MSVC compiler version output.
      /wd4068 # Allow unknown pragmas.
      # We assume the cddm library is compiled with the same compiler and
      # runtime used to compile the client code.
      /wd4251
      $<$<BOOL:${${PNU}_WARNINGS_AS_ERRORS}>:/WX> # -Werror
      $<$<BOOL:${CDDM_HAVE_PERMISSIVE}>:/permissive->
    )

    target_link_options(${TARGET} PRIVATE
      /nologo # Silence MSVC linker version output.
    )

    target_compile_definitions(${TARGET} PRIVATE _SCL_SECURE_NO_WARNINGS)
  else()
    target_compile_options(${TARGET} PRIVATE
      -Wall
      -Wextra
      -pedantic
      -Wconversion
      -Wsign-conversion
      -Wno-unknown-pragmas
      $<$<BOOL:${${PNU}_WARNINGS_AS_ERRORS}>:-Werror>
      $<$<BOOL:${${PNU}_WARNINGS_AS_ERRORS}>:-pedantic-errors>
    )
  endif()

  if(${PNU}_SANITIZERS)
    target_compile_options(${TARGET} PRIVATE
      -fsanitize=address,undefined
    )
    target_link_options(${TARGET} PRIVATE
      -fsanitize=address,undefined
      # GCC sanitizers only work when using the gold linker.
      $<$<CXX_COMPILER_ID:GNU>:-fuse-ld=gold>
    )
  endif()

  target_compile_options(${TARGET} PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-fdiagnostics-color>
    $<$<CXX_COMPILER_ID:Clang>:-fcolor-diagnostics>
  )
endfunction()

# Adds a new library with name `${PNL}-${NAME}` and applies common
# configuration to it.
function(cddm_add_library NAME)
  set(TARGET ${PNL}-${NAME})
  add_library(${TARGET})

  cddm_add_common(${TARGET} lib)
  # Enable -fvisibility=hidden and -fvisibility-inlines-hidden (if applicable).
  set_target_properties(${TARGET} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN true
  )

  string(TOUPPER ${NAME} NAME_UPPER)

  # Generate the export header. We generate the export header using CMake since
  # different export files are required depending on whether a library is shared
  # or static and we can't determine whether a library is shared or static from
  # the export header without requiring the user to add a #define which we want
  # to avoid.
  generate_export_header(${TARGET}
    BASE_NAME ${PNU}_${NAME_UPPER}
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/include/${PNL}/${NAME}/export.hpp
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

    ## Config files

    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${NAME}-config.cmake.in)

      # CMake

      ## Headers

      install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/${PNL}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
      )

      install(
        DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/${PNL}
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
          ${CMAKE_CURRENT_SOURCE_DIR}/${NAME}-config.cmake.in
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
    endif()

    # pkg-config

    if(${PNU}_INSTALL_PKGCONFIG AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${NAME}.pc.in)
      configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/${NAME}.pc.in
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
