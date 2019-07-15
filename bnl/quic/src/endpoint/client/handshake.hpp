#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>

#include <endpoint/shared/crypto.hpp>

#include <memory>
#include <queue>
#include <system_error>
#include <utility>

typedef struct ssl_st SSL;
typedef struct ssl_cipher_st SSL_CIPHER;
using SSL_QUIC_METHOD = struct ssl_quic_method_st;

namespace bnl {

namespace log {
class api;
}

namespace quic {
namespace endpoint {
namespace client {

namespace ngtcp2 {
class connection;
}

class handshake
{
public:
  handshake(base::buffer_view dcid,
            ngtcp2::connection* ngtcp2,
            const log::api* logger);

  BNL_BASE_NO_COPY(handshake);
  BNL_BASE_DEFAULT_MOVE(handshake);

  ~handshake();

  // Data is passed directly to ngtcp2 so no buffer is returned.
  std::error_code send();

  std::error_code recv(crypto::level level, base::buffer_view data);

  std::error_code ack(crypto::level level, size_t size);

  bool completed() const noexcept;

  base::result<crypto> negotiated_crypto() const noexcept;

  std::error_code update_keys();

  std::error_code set_encryption_secrets(crypto::level level,
                                         base::buffer_view read_secret,
                                         base::buffer_view write_secret);

  std::error_code add_handshake_data(crypto::level level,
                                     base::buffer_view data);

private:
  std::error_code init(base::buffer_view dcid);
  
  void log_errors();

  base::result<crypto::aead> make_aead(const SSL_CIPHER* cipher) const noexcept;
  base::result<crypto::hash> make_hash(const SSL_CIPHER* cipher) const noexcept;

private:
  std::unique_ptr<SSL, void (*)(SSL*)> ssl_;
  std::unique_ptr<SSL_QUIC_METHOD> ssl_quic_method_;

  base::buffers keepalive_[4];

  base::buffer tx_secret_;
  base::buffer rx_secret_;

  ngtcp2::connection* ngtcp2_;
  const log::api* logger_;
};

} // namespace client
} // namespace endpoint
} // namespace quic
} // namespace bnl
