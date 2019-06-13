cddm_add_library(quic)

target_link_libraries(bnl-quic
  PUBLIC bnl-core
  PRIVATE bnl-util ngtcp2
)

target_sources(bnl-quic PRIVATE
  src/endpoint/client.cpp
)