bnl_add_library(base)

target_link_libraries(
  bnl-base
  PUBLIC fmt outcome
)

target_sources(bnl-base PRIVATE
  src/base/buffer_view.cpp
  src/base/buffer.cpp
  src/base/buffers.cpp
  src/base/string_view.cpp
  src/base/system_error.cpp
  src/http3/event.cpp
  src/http3/header.cpp
  src/http3/settings.cpp
  src/ip/address/ipv4.cpp
  src/ip/address/ipv6.cpp
  src/ip/address.cpp
  src/ip/endpoint.cpp
  src/ip/host.cpp
  src/quic/event.cpp
  src/quic/path.cpp
  src/log.cpp
)

if(BNL_TEST)
  target_sources(bnl-test PRIVATE
    test/buffer.cpp
    test/buffers.cpp
  )

  target_link_libraries(bnl-test PRIVATE bnl-base)
endif()
