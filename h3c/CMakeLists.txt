cddm_add_library(h3c CXX 11)

# GCC 8.1.0 has Wconversion false positives.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
    CMAKE_CXX_COMPILER_VERSION STREQUAL "8.1.0")
  target_compile_options(h3c PRIVATE -Wno-conversion)
endif()

target_link_libraries(h3c PUBLIC fmt PRIVATE xxhash)
target_include_directories(h3c PRIVATE src)
target_sources(h3c PRIVATE
  src/endpoint/stream/body.cpp
  src/endpoint/stream/control.cpp
  src/endpoint/stream/headers.cpp
  src/endpoint/stream/request.cpp
  src/endpoint/client.cpp
  src/endpoint/server.cpp
  src/frame/decode.cpp
  src/frame/encode.cpp
  src/frame/frame.cpp
  src/huffman/decode.cpp
  src/huffman/encode.cpp
  src/qpack/decode.cpp
  src/qpack/encode.cpp
  src/util/string.cpp
  src/varint/decode.cpp
  src/varint/encode.cpp
  src/buffer.cpp
  src/buffers.cpp
  src/error.cpp
  src/event.cpp
  src/header.cpp
  src/literal.cpp
  src/prefix_int.cpp
)

# Make h3c-log available so we can use it in the tests.
add_subdirectory(../h3c-log ../h3c-log)

if(H3C_TEST)
  target_sources(h3c-test PRIVATE
    test/buffer.cpp
    test/buffers.cpp
    test/endpoint.cpp
    test/frame.cpp
    test/handle.cpp
    test/huffman.cpp
    test/qpack.cpp
    test/varint.cpp
  )

  target_link_libraries(h3c-test PRIVATE h3c h3c-log)
endif()

add_executable(h3c-qpack-interop-encode)

cddm_add_common(h3c-qpack-interop-encode CXX 11 bin)
target_link_libraries(h3c-qpack-interop-encode PRIVATE h3c h3c-log)
target_include_directories(h3c-qpack-interop-encode PRIVATE app)
target_sources(h3c-qpack-interop-encode PRIVATE app/qpack-interop-encode.cpp)

add_executable(h3c-qpack-interop-decode)

cddm_add_common(h3c-qpack-interop-decode CXX 11 bin)
target_link_libraries(h3c-qpack-interop-decode PRIVATE h3c h3c-log)
target_include_directories(h3c-qpack-interop-decode PRIVATE app)
target_sources(h3c-qpack-interop-decode PRIVATE app/qpack-interop-decode.cpp)
