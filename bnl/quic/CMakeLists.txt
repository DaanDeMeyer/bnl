bnl_add_library(quic)

target_link_libraries(bnl-quic
  PUBLIC bnl-base
  PRIVATE ngtcp2 ssl
)

target_include_directories(bnl-quic PRIVATE src)

target_sources(bnl-quic PRIVATE
  src/client/ngtcp2/connection.cpp
  src/client/connection.cpp
  src/client/handshake.cpp
  src/client/stream.cpp
  src/crypto.cpp
)

target_sources(bnl-quic PRIVATE
  src/client/handshake/boringssl.cpp
  src/crypto/boringssl.cpp
)
