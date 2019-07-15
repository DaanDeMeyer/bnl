#pragma once

#include <bnl/quic/export.hpp>

#include <system_error>

namespace bnl {
namespace quic {
namespace endpoint {
namespace client {
namespace ngtcp2 {

enum class error
{
  invalid_argument = -201,
  unknown_pkt_type = -202,
  nobuf = -203,
  proto = -205,
  invalid_state = -206,
  ack_frame = -207,
  stream_id_blocked = -208,
  stream_in_use = -209,
  stream_data_blocked = -210,
  flow_control = -211,
  stream_limit = -213,
  final_size = -214,
  crypto = -215,
  pkt_num_exhausted = -216,
  required_transport_param = -217,
  malformed_transport_param = -218,
  frame_encoding = -219,
  tls_decrypt = -220,
  stream_shut_wr = -221,
  stream_not_found = -222,
  stream_state = -226,
  nokey = -227,
  early_data_rejected = -228,
  recv_version_negotiation = -229,
  closing = -230,
  draining = -231,
  transport_param = -234,
  discard_pkt = -235,
  path_validation_failed = -236,
  conn_id_blocked = -237,
  internal = -238,
  crypto_buffer_exceeded = -239,
  write_stream_more = -240,
  fatal = -500,
  nomem = -501,
  callback_failure = -502
};

BNL_QUIC_EXPORT const std::error_category&
error_category() noexcept;

BNL_QUIC_EXPORT std::error_code
make_error_code(error error) noexcept;

} // namespace ngtcp2
} // namespace client
} // namespace endpoint
} // namespace quic
} // namespace bnl

namespace std {

template<>
struct is_error_code_enum<bnl::quic::endpoint::client::ngtcp2::error>
  : true_type
{};

} // namespace std
