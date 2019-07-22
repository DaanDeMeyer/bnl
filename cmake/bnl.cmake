### Installation options ###

get_directory_property(BNL_IS_SUBDIRECTORY PARENT_DIRECTORY)

# Don't add libraries to the install target by default if the project is built
# from within another project as a static library.
if(BNL_IS_SUBDIRECTORY AND NOT BUILD_SHARED_LIBS)
  option(BNL_INSTALL "Generate installation rules." OFF)
else()
  option(BNL_INSTALL "Generate installation rules." ON)
endif()

mark_as_advanced(BNL_INSTALL)

include(GNUInstallDirs)

option(BNL_INSTALL_PKGCONFIG "Install pkg-config files." ON)

set(BNL_INSTALL_CMAKECONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake
    CACHE STRING "CMake config files installation directory.")
set(BNL_INSTALL_PKGCONFIGDIR ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    CACHE STRING "pkg-config files installation directory.")

mark_as_advanced(
  BNL_INSTALL
  BNL_INSTALL_PKGCONFIG
  BNL_INSTALL_CMAKECONFIGDIR
  BNL_INSTALL_PKGCONFIGDIR
)

### Developer options ###

option(BNL_TIDY "Run clang-tidy when building.")
option(BNL_SANITIZERS "Build with sanitizers.")
option(BNL_WARNINGS_AS_ERRORS "Add -Werror or equivalent to the compile flags and clang-tidy.")

mark_as_advanced(
  BNL_TIDY
  BNL_SANITIZERS
  BNL_WARNINGS_AS_ERRORS
)

### clang-tidy ###

if(BNL_TIDY)
  find_program(BNL_TIDY_PROGRAM clang-tidy)
  mark_as_advanced(BNL_TIDY_PROGRAM)

  if(BNL_TIDY_PROGRAM)
    if(BNL_WARNINGS_AS_ERRORS)
      set(BNL_TIDY_PROGRAM
          ${BNL_TIDY_PROGRAM} -warnings-as-errors=*)
    endif()
  else()
    message(FATAL_ERROR "clang-tidy not found")
  endif()
endif()

### Global Setup ###

if(MSVC)
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag(/permissive- BNL_HAVE_PERMISSIVE)
endif()

if(BNL_SANITIZERS)
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
function(bnl_add_common TARGET OUTPUT_DIRECTORY)
  target_compile_features(${TARGET} PUBLIC cxx_std_11)

  set_target_properties(${TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
    ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
  )

  if(BNL_TIDY)
    set_target_properties(${TARGET} PROPERTIES
      # BNL_TIDY_PROGRAM is a list so we surround it with quotes to pass it as a
      # single argument.
      CXX_CLANG_TIDY "${BNL_TIDY_PROGRAM}"
    )
  endif()

  ### Common development flags (warnings + sanitizers + colors) ###

  if(MSVC)
    target_compile_options(${TARGET} PRIVATE
      /W4
      /nologo # Silence MSVC compiler version output.
      /wd4068 # Allow unknown pragmas.
      # We assume the library is compiled with the same compiler and runtime
      # used to compile the client code.
      /wd4251
      $<$<BOOL:${BNL_WARNINGS_AS_ERRORS}>:/WX> # -Werror
      $<$<BOOL:${BNL_HAVE_PERMISSIVE}>:/permissive->
    )

    target_link_options(${TARGET} PRIVATE
      /nologo # Silence MSVC linker version output.
    )

    target_compile_definitions(${TARGET} PRIVATE _SCL_SECURE_NO_WARNINGS)
  else()
    target_compile_options(${TARGET} PRIVATE
      -Wall
      -Wextra
      -Wconversion
      -Wno-sign-conversion
      -Wno-unknown-pragmas
      $<$<BOOL:${BNL_WARNINGS_AS_ERRORS}>:-Werror>
    )
  endif()

  if(BNL_SANITIZERS)
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

# Adds a new library with name `bnl-${NAME}` and applies common
# configuration to it.
function(bnl_add_library NAME)
  set(TARGET bnl-${NAME})
  add_library(${TARGET})

  bnl_add_common(${TARGET} lib)
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
    BASE_NAME BNL_${NAME_UPPER}
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/include/bnl/${NAME}/export.hpp
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

  if(BNL_INSTALL)

    ## Config files

    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${NAME}-config.cmake.in)

      # CMake

      ## Headers

      install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/bnl
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
      )

      install(
        DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/bnl
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
        DESTINATION ${BNL_INSTALL_CMAKECONFIGDIR}/${TARGET}
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
          ${BNL_INSTALL_CMAKECONFIGDIR}/${TARGET}
      )

      install(
        FILES
          ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}-config.cmake
          ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}-config-version.cmake
        DESTINATION
          ${BNL_INSTALL_CMAKECONFIGDIR}/${TARGET}
      )
    endif()

    # pkg-config

    if(BNL_INSTALL_PKGCONFIG AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${NAME}.pc.in)
      configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/${NAME}.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.pc
        @ONLY
      )

      install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.pc
        DESTINATION ${BNL_INSTALL_PKGCONFIGDIR}
      )
    endif()
  endif()
endfunction()

# We duplicate `FetchContent_MakeAvailable` here so we can use a CMake minimum
# version of 3.13 (`FetchContent_MakeAvailable` was added in CMake 3.14). We
# don't require CMake 3.14 because Debian Buster doesn't have it in its stable
# repositories.
macro(reproc_MakeAvailable)
  foreach(contentName IN ITEMS ${ARGV})
    string(TOLOWER ${contentName} contentNameLower)
    FetchContent_GetProperties(${contentName})
    if(NOT ${contentNameLower}_POPULATED)
      FetchContent_Populate(${contentName})

      # Only try to call add_subdirectory() if the populated content
      # can be treated that way. Protecting the call with the check
      # allows this function to be used for projects that just want
      # to ensure the content exists, such as to provide content at
      # a known location.
      if(EXISTS ${${contentNameLower}_SOURCE_DIR}/CMakeLists.txt)
        add_subdirectory(${${contentNameLower}_SOURCE_DIR}
                         ${${contentNameLower}_BINARY_DIR})
      endif()
    endif()
  endforeach()
endmacro()
