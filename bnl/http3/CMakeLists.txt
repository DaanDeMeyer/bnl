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
  src/varint/decode.cpp
  src/varint/encode.cpp
  src/error.cpp
  src/event.cpp
  src/header.cpp
  src/literal.cpp
  src/prefix_int.cpp
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
