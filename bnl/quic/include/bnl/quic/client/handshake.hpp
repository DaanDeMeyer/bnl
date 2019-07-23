#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/quic/crypto.hpp>
#include <bnl/quic/export.hpp>
#include <bnl/ip/host.hpp>

#include <memory>

using SSL = struct ssl_st;
using SSL_CIPHER = struct ssl_cipher_st;
using SSL_QUIC_METHOD = struct ssl_quic_method_st;

namespace bnl {

namespace log {
class api;
}

namespace quic {
namespace client {

namespace ngtcp2 {
class connection;
}

class BNL_QUIC_EXPORT handshake {
public:
  handshake(const ip::host &host, base::buffer_view dcid,
            ngtcp2::connection *ngtcp2,
            const log::api *logger);

  handshake(handshake &&other);            // NOLINT
  handshake &operator=(handshake &&other); // NOLINT

  ~handshake() noexcept;

  // Data is passed directly to ngtcp2 so no buffer is returned.
  result<void> send();

  result<void> recv(crypto::level level, base::buffer_view data);

  result<void> ack(crypto::level level, size_t size);

  bool completed() const noexcept;

  result<crypto> negotiated_crypto() const noexcept;

  result<void> update_keys();

  result<void> set_encryption_secrets(crypto::level level,
                                      base::buffer_view read_secret,
                                      base::buffer_view write_secret);

  result<void> add_handshake_data(crypto::level level, base::buffer_view data);

private:
  result<void> init(const ip::host &host, base::buffer_view dcid);

  void log_errors();

  result<crypto::aead> make_aead(const SSL_CIPHER *cipher) const noexcept;
  result<crypto::hash> make_hash(const SSL_CIPHER *cipher) const noexcept;

private:
  std::unique_ptr<SSL, void (*)(SSL *)> ssl_;
  std::unique_ptr<SSL_QUIC_METHOD> ssl_quic_method_;

  base::buffers keepalive_[4];

  base::buffer tx_secret_;
  base::buffer rx_secret_;

  ngtcp2::connection *ngtcp2_;
  const log::api *logger_;
};

}
}
}
