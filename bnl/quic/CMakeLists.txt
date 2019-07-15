bnl_add_library(quic)

target_link_libraries(bnl-quic
  PUBLIC bnl-base
  PRIVATE bnl-util ngtcp2 ssl
)

target_include_directories(bnl-quic PRIVATE src)

target_sources(bnl-quic PRIVATE
  src/endpoint/client/handshake.cpp
  src/endpoint/client/impl.cpp
  src/endpoint/client/ngtcp2/connection.cpp
  src/endpoint/client/ngtcp2/error.cpp
  src/endpoint/shared/crypto/boringssl.cpp
  src/endpoint/shared/crypto.cpp
  src/client.cpp
  src/error.cpp
  src/path.cpp
)
