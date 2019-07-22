#pragma once

#include <bnl/quic/export.hpp>
#include <bnl/result.hpp>

namespace bnl {
namespace quic {
namespace client {
namespace ngtcp2 {

enum class error {
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

class domain;
using code = status_code<domain>;

class domain : public status_code_domain {
public:
  using value_type = error;

  constexpr domain() noexcept
    : status_code_domain(0xfa71587d307fb79c)
  {}

  domain(const domain &) = default;
  domain(domain &&) = default;
  domain &operator=(const domain &) = default;
  domain &operator=(domain &&) = default;
  ~domain() = default;

  static inline constexpr const domain &get();

  string_ref name() const noexcept final
  {
    return string_ref("bnl-quic-ngtcp2");
  }

  bool _do_failure(const status_code<void> &sc) const noexcept final
  {
    (void) sc;
    return true;
  }

  bool _do_equivalent(const status_code<void> &first,
                      const status_code<void> &second) const noexcept final
  {
    if (second.domain() == *this) {
      return static_cast<const code &>(first).value() ==
             static_cast<const code &>(second).value();
    }

    return false;
  }

  generic_code _generic_code(const status_code<void> &sc) const noexcept final
  {
    (void) sc;
    return errc::unknown;
  }

  domain::string_ref _do_message(const status_code<void> &sc) const
    noexcept final
  {
    switch (static_cast<const code &>(sc).value()) {
      case error::invalid_argument:
        return string_ref("invalid argument");
      case error::unknown_pkt_type:
        return string_ref("unknown pkt type");
      case error::nobuf:
        return string_ref("nobuf");
      case error::proto:
        return string_ref("proto");
      case error::invalid_state:
        return string_ref("invalid state");
      case error::ack_frame:
        return string_ref("ack frame");
      case error::stream_id_blocked:
        return string_ref("stream id blocked");
      case error::stream_in_use:
        return string_ref("stream in use");
      case error::stream_data_blocked:
        return string_ref("stream data blocked");
      case error::flow_control:
        return string_ref("flow control");
      case error::stream_limit:
        return string_ref("stream limit");
      case error::final_size:
        return string_ref("final size");
      case error::crypto:
        return string_ref("crypto");
      case error::pkt_num_exhausted:
        return string_ref("pkt num exhausted");
      case error::required_transport_param:
        return string_ref("required transport param");
      case error::malformed_transport_param:
        return string_ref("malformed transport param");
      case error::frame_encoding:
        return string_ref("frame encoding");
      case error::tls_decrypt:
        return string_ref("tls decrypt");
      case error::stream_shut_wr:
        return string_ref("stream shut wr");
      case error::stream_not_found:
        return string_ref("stream not found");
      case error::stream_state:
        return string_ref("stream state");
      case error::nokey:
        return string_ref("nokey");
      case error::early_data_rejected:
        return string_ref("early data rejected");
      case error::recv_version_negotiation:
        return string_ref("recv version negotiation");
      case error::closing:
        return string_ref("closing");
      case error::draining:
        return string_ref("draining");
      case error::transport_param:
        return string_ref("transport param");
      case error::discard_pkt:
        return string_ref("discard pkt");
      case error::path_validation_failed:
        return string_ref("path validation failed");
      case error::conn_id_blocked:
        return string_ref("conn id blocked");
      case error::internal:
        return string_ref("internal");
      case error::crypto_buffer_exceeded:
        return string_ref("crypto buffer exceeded");
      case error::write_stream_more:
        return string_ref("write stream more");
      case error::fatal:
        return string_ref("fatal");
      case error::nomem:
        return string_ref("nomem");
      case error::callback_failure:
        return string_ref("callback failure");
    }

    return string_ref("unknown");
  }

#if defined(_CPPUNWIND) || defined(__EXCEPTIONS) || 0
  void _do_throw_exception(const status_code<void> &sc) const final
  {

    throw status_error<domain>(static_cast<const code &>(sc));
  }
#endif
};

constexpr domain instance;

inline constexpr const domain &
domain::get()
{
  return instance;
}

inline code
make_status_code(error error)
{
  return code(in_place, error);
}

}
}
}
}
