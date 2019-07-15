#pragma once

#include <bnl/quic/clock.hpp>
#include <bnl/quic/export.hpp>

#include <cstdint>

namespace bnl {
namespace quic {

// https://quicwg.org/base-drafts/draft-ietf-quic-transport.html#transport-parameter-definitions
struct BNL_QUIC_EXPORT params {
  params() = default;

  uint64_t max_stream_data_bidi_local = 0;
  uint64_t max_stream_data_bidi_remote = 0;
  uint64_t max_stream_data_uni = 0;
  uint64_t max_data = 0;
  uint64_t max_streams_bidi = 0;
  uint64_t max_streams_uni = 0;
  milliseconds idle_timeout = milliseconds(0);
  uint64_t max_packet_size = 65527;
  uint64_t ack_delay_exponent = 3;
  uint8_t disable_migration = 0;
  milliseconds max_ack_delay = milliseconds(25);
};

} // namespace quic
} // namespace bnl
