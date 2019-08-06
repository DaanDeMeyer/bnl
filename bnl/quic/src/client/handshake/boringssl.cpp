#include <bnl/quic/client/handshake.hpp>

#include <bnl/base/log.hpp>
#include <bnl/quic/client/ngtcp2/connection.hpp>

#include <openssl/ssl.h>

namespace bnl {
namespace quic {
namespace client {

static SSL *
ssl_new(handshake *handshake)
{
  SSL_CTX *ssl_ctx = SSL_CTX_new(TLS_method());
  assert(ssl_ctx != nullptr);

  SSL_CTX_set_default_verify_paths(ssl_ctx);

  SSL *ssl = SSL_new(ssl_ctx);
  assert(ssl != nullptr);

  SSL_CTX_free(ssl_ctx);

  int rv = SSL_set_ex_data(ssl, 0, handshake);
  // TODO: re-enable exceptions
  (void) rv;
  assert(rv == 1);

  return ssl;
}

static ssl_encryption_level_t
make_crypto_level(crypto::level level)
{
  switch (level) {
    case crypto::level::initial:
      return ssl_encryption_initial;
    case crypto::level::early_data:
      return ssl_encryption_early_data;
    case crypto::level::handshake:
      return ssl_encryption_handshake;
    case crypto::level::application:
      return ssl_encryption_application;
  }

  assert(false);
  return ssl_encryption_initial;
}

static crypto::level
make_crypto_level(ssl_encryption_level_t level)
{
  switch (level) {
    case ssl_encryption_initial:
      return crypto::level::initial;
    case ssl_encryption_early_data:
      return crypto::level::early_data;
    case ssl_encryption_handshake:
      return crypto::level::handshake;
    case ssl_encryption_application:
      return crypto::level::application;
  }

  assert(false);
  return crypto::level::initial;
}

result<crypto::aead>
make_aead(const SSL_CIPHER *cipher) noexcept
{
  switch (SSL_CIPHER_get_id(cipher)) {
    case 0x03001301U: // TLS_AES_128_GCM_SHA256
      return crypto::aead::aes_128_gcm;
    case 0x03001302U: // TLS_AES_256_GCM_SHA384
      return crypto::aead::aes_256_gcm;
    case 0x03001303U: // TLS_CHACHA20_POLY1305_SHA256
      return crypto::aead::chacha20_poly1305;
  }

  BNL_LOG_E("Unsupported Cipher Suite: {}", SSL_CIPHER_standard_name(cipher));
  return quic::error::handshake;
}

result<crypto::hash>
make_hash(const SSL_CIPHER *cipher) noexcept
{
  switch (SSL_CIPHER_get_id(cipher)) {
    case 0x03001301U: // TLS_AES_128_GCM_SHA256
    case 0x03001303U: // TLS_CHACHA20_POLY1305_SHA256
      return crypto::hash::sha256;
    case 0x03001302U: // TLS_AES_256_GCM_SHA384
      return crypto::hash::sha384;
  }

  BNL_LOG_E("Unsupported Cipher Suite: {}", SSL_CIPHER_standard_name(cipher));
  return quic::error::handshake;
}

static int
set_encryption_secrets_cb(SSL *ssl,
                          ssl_encryption_level_t level,
                          const uint8_t *read_secret,
                          const uint8_t *write_secret,
                          size_t size)
{
  auto handshake = static_cast<class handshake *>(SSL_get_app_data(ssl));

  result<void> r =
    handshake->set_encryption_secrets(make_crypto_level(level),
                                      base::buffer_view(read_secret, size),
                                      base::buffer_view(write_secret, size));

  return r ? 1 : 0;
}

static int
add_handshake_data_cb(SSL *ssl,
                      ssl_encryption_level_t level,
                      const uint8_t *data,
                      size_t size)
{
  auto handshake = static_cast<class handshake *>(SSL_get_app_data(ssl));

  result<void> r = handshake->add_handshake_data(make_crypto_level(level),
                                                 base::buffer_view(data, size));

  return r ? 1 : 0;
}

static int
flush_flight_cb(SSL *ssl)
{
  // TODO: Implement
  (void) ssl;
  return 1;
}

static int
send_alert_cb(SSL *ssl, ssl_encryption_level_t level, uint8_t alert)
{
  // TODO: Implement
  (void) ssl;
  (void) level;
  (void) alert;
  return 1;
}

struct handshake::impl {
  explicit impl(handshake *handshake) noexcept;

  std::unique_ptr<SSL, void (*)(SSL *)> ssl_;
  SSL_QUIC_METHOD ssl_quic_method_;

  void log_errors();
};

handshake::impl::impl(handshake *handshake) noexcept
  : ssl_(ssl_new(handshake), SSL_free)
  , ssl_quic_method_({ set_encryption_secrets_cb,
                       add_handshake_data_cb,
                       flush_flight_cb,
                       send_alert_cb })
{}

handshake::handshake(const ip::host &host,
                     base::buffer_view dcid,
                     ngtcp2::connection *ngtcp2)
  : impl_(new impl(this))
  , ngtcp2_(ngtcp2)
{
  // TODO: re-enable exceptions
  init(host, dcid).assume_value();
}

handshake::handshake(handshake &&) = default; // NOLINT

handshake &
handshake::operator=(handshake &&) = default; // NOLINT

handshake::~handshake() noexcept = default;

// https://quicwg.org/base-drafts/draft-ietf-quic-tls.html#initial-secrets
result<void>
handshake::init(const ip::host &host, base::buffer_view dcid)
{
  // Initial Keys

  quic::crypto crypto(crypto::aead::aes_128_gcm, crypto::hash::sha256);

  base::buffer initial =
    BNL_TRY(crypto.initial_secret(dcid, ngtcp2::connection::INITIAL_SALT));

  base::buffer client_secret = BNL_TRY(crypto.client_initial_secret(initial));
  crypto::key write_key = BNL_TRY(crypto.packet_protection_key(client_secret));
  BNL_TRY(ngtcp2_->install_initial_tx_keys(write_key));

  base::buffer server_secret = BNL_TRY(crypto.server_initial_secret(initial));
  crypto::key read_key = BNL_TRY(crypto.packet_protection_key(server_secret));
  BNL_TRY(ngtcp2_->install_initial_rx_keys(read_key));

  // ALPN

  const uint8_t *alpn = ngtcp2::connection::ALPN_H3.data();
  uint32_t alpn_size =
    static_cast<uint32_t>(ngtcp2::connection::ALPN_H3.size());

  int rv = SSL_set_alpn_protos(impl_->ssl_.get(), alpn, alpn_size);
  if (rv == 1) {
    return quic::error::handshake;
  }

  // Client mode

  SSL_set_connect_state(impl_->ssl_.get());

  // TLS 1.3

  SSL_set_min_proto_version(impl_->ssl_.get(), TLS1_3_VERSION);
  SSL_set_max_proto_version(impl_->ssl_.get(), TLS1_3_VERSION);

  // SNI

  base::string hostname(host.name().data(), host.name().size());
  SSL_set_tlsext_host_name(impl_->ssl_.get(), hostname.c_str());

  // Transport Parameters

  base::buffer tp = BNL_TRY(ngtcp2_->get_local_transport_parameters());
  rv = SSL_set_quic_transport_params(impl_->ssl_.get(), tp.data(), tp.size());
  if (rv == 0) {
    return quic::error::handshake;
  }

  // QUIC

  rv = SSL_set_quic_method(impl_->ssl_.get(), &impl_->ssl_quic_method_);
  if (rv == 0) {
    return quic::error::handshake;
  }

  return base::success();
}

result<void>
handshake::send()
{
  int rv = SSL_do_handshake(impl_->ssl_.get());
  if (rv <= 0) {
    int error = SSL_get_error(impl_->ssl_.get(), rv);

    switch (error) {
      case SSL_ERROR_WANT_READ:
      case SSL_ERROR_WANT_WRITE:
        break;
      default:
        impl_->log_errors();
        return quic::error::handshake;
    }
  }

  return base::success();
}

result<void>
handshake::recv(crypto::level level, base::buffer_view data)
{
  int rv = SSL_provide_quic_data(
    impl_->ssl_.get(), make_crypto_level(level), data.data(), data.size());
  if (rv == 0) {
    impl_->log_errors();
    return quic::error::handshake;
  }

  if (completed()) {
    rv = SSL_process_quic_post_handshake(impl_->ssl_.get());
    if (rv == 0) {
      impl_->log_errors();
      return quic::error::handshake;
    }

    return base::success();
  }

  rv = SSL_do_handshake(impl_->ssl_.get());
  if (rv <= 0) {
    int error = SSL_get_error(impl_->ssl_.get(), rv);

    switch (error) {
      case SSL_ERROR_WANT_READ:
      case SSL_ERROR_WANT_WRITE:
        return error::incomplete;
      default:
        impl_->log_errors();
        return error::handshake;
    }
  }

  ngtcp2_->handshake_completed();

  const uint8_t *peer_tp = nullptr;
  size_t peer_tp_len = 0;
  SSL_get_peer_quic_transport_params(impl_->ssl_.get(), &peer_tp, &peer_tp_len);

  base::buffer_view view(peer_tp, peer_tp_len);
  BNL_TRY(ngtcp2_->set_remote_transport_parameters(view));

  return base::success();
}

result<crypto>
handshake::negotiated_crypto() const noexcept
{
  const SSL_CIPHER *cipher = SSL_get_current_cipher(impl_->ssl_.get());

  if (cipher == nullptr) {
    BNL_LOG_E("Cipher suite not negotiated yet");
    return quic::error::handshake;
  }

  crypto::aead aead = BNL_TRY(make_aead(cipher));
  crypto::hash hash = BNL_TRY(make_hash(cipher));

  return quic::crypto(aead, hash);
}

void
handshake::impl::log_errors()
{
  while (true) {
    uint32_t error = ERR_get_error();
    if (error == 0) {
      break;
    }

    std::array<char, 100> string = {};
    ERR_error_string_n(error, string.data(), string.size());

    BNL_LOG_E(string.data());
  }
}

}
}
}
