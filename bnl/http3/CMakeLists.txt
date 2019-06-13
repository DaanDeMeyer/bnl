cddm_add_library(http3)

# GCC 8.1.0 has Wconversion false positives.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
    CMAKE_CXX_COMPILER_VERSION STREQUAL "8.1.0")
  target_compile_options(bnl-http3 PRIVATE -Wno-conversion)
endif()

target_link_libraries(bnl-http3
  PUBLIC bnl-core
  PRIVATE bnl-util xxhash
)

target_include_directories(bnl-http3 PRIVATE src)
target_sources(bnl-http3 PRIVATE
  src/codec/frame/decode.cpp
  src/codec/frame/encode.cpp
  src/codec/frame/frame.cpp
  src/codec/huffman/decode.cpp
  src/codec/huffman/encode.cpp
  src/codec/qpack/decode.cpp
  src/codec/qpack/encode.cpp
  src/codec/varint/decode.cpp
  src/codec/varint/encode.cpp
  src/codec/body.cpp
  src/codec/headers.cpp
  src/codec/literal.cpp
  src/codec/prefix_int.cpp
  src/endpoint/stream/control.cpp
  src/endpoint/stream/request.cpp
  src/endpoint/client.cpp
  src/endpoint/server.cpp
  src/event.cpp
  src/error.cpp
  src/header.cpp
)

if(BNL_TEST)
  target_sources(bnl-test PRIVATE
    test/endpoint.cpp
    test/frame.cpp
    test/handle.cpp
    test/huffman.cpp
    test/qpack.cpp
    test/varint.cpp
  )

  target_link_libraries(bnl-test PRIVATE bnl-http3)
endif()

foreach(ACTION encode decode)
  add_executable(bnl-http3-qpack-interop-${ACTION})

  cddm_add_common(bnl-http3-qpack-interop-${ACTION} bin)
  target_link_libraries(bnl-http3-qpack-interop-${ACTION} PRIVATE
    bnl-http3
    bnl-log
    bnl-util
  )
  set_target_properties(bnl-http3-qpack-interop-${ACTION} PROPERTIES
    OUTPUT_NAME qpack-interop-${ACTION}
  )

  target_include_directories(bnl-http3-qpack-interop-${ACTION} PRIVATE app)
  target_sources(bnl-http3-qpack-interop-${ACTION} PRIVATE
    app/qpack-interop-${ACTION}.cpp
  )
endforeach()
