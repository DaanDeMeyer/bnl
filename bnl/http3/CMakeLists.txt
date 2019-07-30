bnl_add_library(http3)

# GCC 8.1.0 has Wconversion false positives.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
    CMAKE_CXX_COMPILER_VERSION STREQUAL "8.1.0")
  target_compile_options(bnl-http3 PRIVATE -Wno-conversion)
endif()

target_link_libraries(bnl-http3
  PUBLIC bnl-base
  PRIVATE xxhash
)

target_sources(bnl-http3 PRIVATE
  src/codec/frame/decode.cpp
  src/codec/frame/encode.cpp
  src/codec/frame/frame.cpp
  src/codec/qpack/huffman/decode.cpp
  src/codec/qpack/huffman/encode.cpp
  src/codec/qpack/literal/encode.cpp
  src/codec/qpack/literal/decode.cpp
  src/codec/qpack/prefix_int/encode.cpp
  src/codec/qpack/prefix_int/decode.cpp
  src/codec/qpack/decode.cpp
  src/codec/qpack/encode.cpp
  src/codec/varint/decode.cpp
  src/codec/varint/encode.cpp
  src/codec/body.cpp
  src/codec/headers.cpp
  src/client/connection.cpp
  src/client/stream/control.cpp
  src/client/stream/request.cpp
  src/endpoint/stream/control.cpp
  src/endpoint/stream/request.cpp
  src/server/connection.cpp
  src/server/stream/control.cpp
  src/server/stream/request.cpp
)

if(BNL_TEST)
  target_sources(bnl-test PRIVATE
    test/connection.cpp
    test/frame.cpp
    test/huffman.cpp
    test/qpack.cpp
    test/varint.cpp
  )

  target_link_libraries(bnl-test PRIVATE bnl-http3)
endif()

foreach(ACTION encode decode)
  add_executable(bnl-http3-qpack-interop-${ACTION})

  bnl_add_common(bnl-http3-qpack-interop-${ACTION} bin)
  target_link_libraries(bnl-http3-qpack-interop-${ACTION} PRIVATE
    bnl-http3
    bnl-log
  )
  set_target_properties(bnl-http3-qpack-interop-${ACTION} PROPERTIES
    OUTPUT_NAME qpack-interop-${ACTION}
  )

  target_sources(bnl-http3-qpack-interop-${ACTION} PRIVATE
    app/qpack/qpack-interop-${ACTION}.cpp
  )
endforeach()

if(systemd_FOUND)
  add_executable(bnl-http3-client)

  bnl_add_common(bnl-http3-client bin)
  target_include_directories(bnl-http3-client PRIVATE app/endpoint)
  target_link_libraries(bnl-http3-client PRIVATE
    bnl-http3
    bnl-quic
    bnl-log
    systemd
  )

  target_sources(bnl-http3-client PRIVATE
    app/endpoint/client.cpp
    app/endpoint/os/socket/udp.cpp
    app/endpoint/os/dns.cpp
    app/endpoint/os/fd.cpp
    app/endpoint/os/ip/address.cpp
    app/endpoint/sd/event/loop.cpp
    app/endpoint/sd/event/source.cpp
  )
else()
  message(STATUS "bnl: Not building HTTP/3 client because systemd dependency is missing.")
endif()
