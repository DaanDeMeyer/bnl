cddm_add_library(h3c CXX 11)

# GCC 8.1.0 has Wconversion false positives.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
    CMAKE_CXX_COMPILER_VERSION STREQUAL "8.1.0")
  target_compile_options(h3c PRIVATE -Wno-conversion)
endif()

target_link_libraries(h3c PUBLIC fmt PRIVATE xxhash)
target_include_directories(h3c PRIVATE src)
target_sources(h3c PRIVATE
  src/frame/decode.cpp
  src/frame/encode.cpp
  src/huffman/encode.cpp
  src/huffman/decode.cpp
  src/qpack/encode.cpp
  src/qpack/decode.cpp
  src/util/string.cpp
  src/varint/decode.cpp
  src/varint/encode.cpp
  src/error.cpp
)

# Make h3c-log available so we can use it in the tests.
add_subdirectory(../h3c-log ../h3c-log)

if(H3C_TEST)
  target_sources(h3c-test PRIVATE
    test/frame.cpp
    test/huffman.cpp
    test/qpack.cpp
    test/varint.cpp
  )

  target_link_libraries(h3c-test PRIVATE h3c h3c-log)
endif()

add_executable(h3c-qpack-encode)

cddm_add_common(h3c-qpack-encode CXX 11 bin)
target_link_libraries(h3c-qpack-encode PRIVATE h3c h3c-log)
target_include_directories(h3c-qpack-encode PRIVATE app)
if(MSVC)
  target_compile_definitions(h3c-qpack-encode PRIVATE CRT_SECURE_NO_WARNINGS)
endif()
target_sources(h3c-qpack-encode PRIVATE app/qpack-encode.cpp)

add_executable(h3c-qpack-decode)

cddm_add_common(h3c-qpack-decode CXX 11 bin)
target_link_libraries(h3c-qpack-decode PRIVATE h3c h3c-log)
target_include_directories(h3c-qpack-decode PRIVATE app)
if(MSVC)
  target_compile_definitions(h3c-qpack-decode PRIVATE CRT_SECURE_NO_WARNINGS)
endif()
target_sources(h3c-qpack-decode PRIVATE app/qpack-decode.cpp)
