#include <openssl/ssl.h>

#include <endpoint/client/handshake.hpp>
#include <endpoint/client/ngtcp2/connection.hpp>

#include <bnl/base/error.hpp>
#include <bnl/quic/error.hpp>
#include <bnl/util/enum.hpp>
#include <bnl/util/error.hpp>

namespace bnl {
namespace quic {
namespace endpoint {
namespace client {

static SSL_CTX*
ssl_ctx_new()
{
  SSL_CTX* ssl_ctx = SSL_CTX_new(TLS_method());
  assert(ssl_ctx != nullptr);

  SSL_CTX_set_default_verify_paths(ssl_ctx);

  return ssl_ctx;
}

static SSL_CTX* SSL_CONTEXT = ssl_ctx_new(); // NOLINT

static SSL*
ssl_new(handshake* handshake)
{
  SSL* ssl = SSL_new(SSL_CONTEXT);
  assert(ssl != nullptr);

  int rv = SSL_set_ex_data(ssl, 0, handshake);
  // TODO: re-enable exceptions
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

static int
set_encryption_secrets_cb(SSL* ssl,
                          ssl_encryption_level_t level,
                          const uint8_t* read_secret,
                          const uint8_t* write_secret,
                          size_t size)
{
  auto handshake = static_cast<class handshake*>(SSL_get_app_data(ssl));

  std::error_code ec =
    handshake->set_encryption_secrets(make_crypto_level(level),
                                      base::buffer_view(read_secret, size),
                                      base::buffer_view(write_secret, size));

  return ec ? 0 : 1;
}

static int
add_handshake_data_cb(SSL* ssl,
                      ssl_encryption_level_t level,
                      const uint8_t* data,
                      size_t size)
{
  auto handshake = static_cast<class handshake*>(SSL_get_app_data(ssl));

  std::error_code ec = handshake->add_handshake_data(
    make_crypto_level(level), base::buffer_view(data, size));

  return ec ? 0 : 1;
}

static int
flush_flight_cb(SSL* ssl)
{
  // TODO: Implement
  (void)ssl;
  return 1;
}

static int
send_alert_cb(SSL* ssl, ssl_encryption_level_t level, uint8_t alert)
{
  // TODO: Implement
  (void)ssl;
  (void)level;
  (void)alert;
  return 1;
}

handshake::handshake(base::buffer_view dcid,
                     ngtcp2::connection* ngtcp2,
                     const log::api* logger)
  : ssl_(ssl_new(this), SSL_free)
  , ssl_quic_method_(new SSL_QUIC_METHOD{ set_encryption_secrets_cb,
                                          add_handshake_data_cb,
                                          flush_flight_cb,
                                          send_alert_cb })
  , ngtcp2_(ngtcp2)
  , logger_(logger)
{
  init(dcid);
  // TODO: re-enable exceptions
}

handshake::~handshake() = default;

// https://quicwg.org/base-drafts/draft-ietf-quic-tls.html#initial-secrets
std::error_code
handshake::init(base::buffer_view dcid)
{
  // Initial Keys

  quic::crypto crypto(crypto::aead::aes_128_gcm, crypto::hash::sha256, logger_);

  base::buffer initial =
    TRY(crypto.initial_secret(dcid, ngtcp2::connection::INITIAL_SALT));

  base::buffer client_secret = TRY(crypto.client_initial_secret(initial));
  crypto::key write_key = TRY(crypto.packet_protection_key(client_secret));
  TRY(ngtcp2_->install_initial_tx_keys(write_key));

  base::buffer server_secret = TRY(crypto.server_initial_secret(initial));
  crypto::key read_key = TRY(crypto.packet_protection_key(server_secret));
  TRY(ngtcp2_->install_initial_rx_keys(read_key));

  // ALPN

  const uint8_t* alpn = ngtcp2::connection::ALPN_H3.data();
  uint32_t alpn_size =
    static_cast<uint32_t>(ngtcp2::connection::ALPN_H3.size());

  int rv = SSL_set_alpn_protos(ssl_.get(), alpn, alpn_size);
  CHECK(rv == 0, error::handshake);

  // Client mode

  SSL_set_connect_state(ssl_.get());

  // TLS 1.3

  SSL_set_min_proto_version(ssl_.get(), TLS1_3_VERSION);
  SSL_set_max_proto_version(ssl_.get(), TLS1_3_VERSION);

  // SNI

  // TODO: Set actual hostname
  SSL_set_tlsext_host_name(ssl_.get(), "localhost");

  // Transport Parameters

  base::buffer tp = TRY(ngtcp2_->get_local_transport_parameters());
  rv = SSL_set_quic_transport_params(ssl_.get(), tp.data(), tp.size());
  CHECK(rv == 1, error::handshake);

  // QUIC

  rv = SSL_set_quic_method(ssl_.get(), ssl_quic_method_.get());
  CHECK(rv == 1, error::handshake);

  return {};
}

std::error_code
handshake::send()
{
  int rv = SSL_do_handshake(ssl_.get());
  if (rv <= 0) {
    int error = SSL_get_error(ssl_.get(), rv);

    switch (error) {
      case SSL_ERROR_WANT_READ:
      case SSL_ERROR_WANT_WRITE:
        break;
      default:
        log_errors();
        THROW(error::handshake);
    }
  }

  return {};
}

std::error_code
handshake::recv(crypto::level level, base::buffer_view data)
{
  int rv = SSL_provide_quic_data(
    ssl_.get(), make_crypto_level(level), data.data(), data.size());
  if (rv == 0) {
    log_errors();
    THROW(error::handshake);
  }

  if (completed()) {
    rv = SSL_process_quic_post_handshake(ssl_.get());
    if (rv == 0) {
      log_errors();
      THROW(error::handshake);
    }

    return {};
  }

  rv = SSL_do_handshake(ssl_.get());
  if (rv <= 0) {
    int error = SSL_get_error(ssl_.get(), rv);

    switch (error) {
      case SSL_ERROR_WANT_READ:
      case SSL_ERROR_WANT_WRITE:
        THROW(base::error::incomplete);
      default:
        log_errors();
        THROW(error::handshake);
    }
  }

  ngtcp2_->handshake_completed();

  const uint8_t* remote_tp = nullptr;
  size_t remote_tp_len = 0;
  SSL_get_peer_quic_transport_params(ssl_.get(), &remote_tp, &remote_tp_len);

  base::buffer_view view(remote_tp, remote_tp_len);
  TRY(ngtcp2_->set_remote_transport_parameters(view));

  return {};
}

std::error_code
handshake::ack(crypto::level level, size_t size)
{
  base::buffers& keepalive = keepalive_[util::to_underlying(level)];

  if (size > keepalive.size()) {
    LOG_E("ngtcp2's acked crypto data ({}) exceeds remaining data ({})",
          size,
          keepalive.size());
    THROW(base::error::internal);
  }

  keepalive.consume(size);

  return {};
}

bool
handshake::completed() const noexcept
{
  return ngtcp2_->get_handshake_completed();
}

base::result<crypto>
handshake::negotiated_crypto() const noexcept
{
  const SSL_CIPHER* cipher = SSL_get_current_cipher(ssl_.get());

  if (cipher == nullptr) {
    LOG_E("Cipher suite not negotiated yet");
    THROW(error::handshake);
  }

  crypto::aead aead = TRY(make_aead(cipher));
  crypto::hash hash = TRY(make_hash(cipher));

  return quic::crypto(aead, hash, logger_);
}

base::result<crypto::aead>
handshake::make_aead(const SSL_CIPHER* cipher) const noexcept
{
  switch (SSL_CIPHER_get_id(cipher)) {
    case 0x03001301U: // TLS_AES_128_GCM_SHA256
      return crypto::aead::aes_128_gcm;
    case 0x03001302U: // TLS_AES_256_GCM_SHA384
      return crypto::aead::aes_256_gcm;
    case 0x03001303U: // TLS_CHACHA20_POLY1305_SHA256
      return crypto::aead::chacha20_poly1305;
  }

  LOG_E("Unsupported Cipher Suite: {}", SSL_CIPHER_standard_name(cipher));
  THROW(error::handshake);
}

base::result<crypto::hash>
handshake::make_hash(const SSL_CIPHER* cipher) const noexcept
{
  switch (SSL_CIPHER_get_id(cipher)) {
    case 0x03001301U: // TLS_AES_128_GCM_SHA256
    case 0x03001303U: // TLS_CHACHA20_POLY1305_SHA256
      return crypto::hash::sha256;
    case 0x03001302U: // TLS_AES_256_GCM_SHA384
      return crypto::hash::sha384;
  }

  LOG_E("Unsupported Cipher Suite: {}", SSL_CIPHER_standard_name(cipher));
  THROW(error::handshake);
}

std::error_code
handshake::update_keys()
{
  quic::crypto crypto = TRY(this->negotiated_crypto());

  tx_secret_ = TRY(crypto.update_secret(tx_secret_));
  crypto::key write_key = TRY(crypto.packet_protection_key(tx_secret_));
  ngtcp2_->update_tx_keys(write_key);

  rx_secret_ = TRY(crypto.update_secret(rx_secret_));
  crypto::key read_key = TRY(crypto.packet_protection_key(rx_secret_));
  ngtcp2_->update_rx_keys(read_key);

  return {};
}

std::error_code
handshake::set_encryption_secrets(crypto::level level,
                                  base::buffer_view read_secret,
                                  base::buffer_view write_secret)
{
  quic::crypto crypto = TRY(this->negotiated_crypto());

  LOG_T("{}", crypto);

  ngtcp2_->set_aead_overhead(crypto.aead_overhead());

  crypto::key write_key = TRY(crypto.packet_protection_key(write_secret));
  crypto::key read_key;

  // Data is not received at the 0-RTT level by the client so BoringSSL does not
  // provide the client with a key for receiving early data.
  if (level != crypto::level::early_data) {
    read_key = TRY(crypto.packet_protection_key(read_secret));
  }

  switch (level) {
    case crypto::level::initial:
      THROW(error::handshake);
    case crypto::level::early_data:
      TRY(ngtcp2_->install_early_keys(write_key));
      break;
    case crypto::level::handshake:
      TRY(ngtcp2_->install_handshake_tx_keys(write_key));
      TRY(ngtcp2_->install_handshake_rx_keys(read_key));
      LOG_T("handshake: installed handshake keys");
      break;
    case crypto::level::application:
      tx_secret_ = base::buffer(write_secret);
      TRY(ngtcp2_->install_tx_keys(write_key));

      rx_secret_ = base::buffer(read_secret);
      TRY(ngtcp2_->install_rx_keys(read_key));

      LOG_T("handshake: installed application keys");
      break;
  }

  return {};
}

std::error_code
handshake::add_handshake_data(crypto::level level, base::buffer_view data)
{
  base::buffers& keepalive = keepalive_[util::to_underlying(level)];

  keepalive.push(base::buffer(data));
  base::buffer& buffer = keepalive.back();

  TRY(ngtcp2_->submit_crypto_data(level, buffer));

  return {};
}

void
handshake::log_errors()
{
  while (true) {
    uint32_t error = ERR_get_error();
    if (error == 0) {
      break;
    }

    std::array<char, 100> string = {};
    ERR_error_string_n(error, string.data(), string.size());

    LOG_E(string.data());
  }
}

} // namespace client
} // namespace endpoint
} // namespace quic
} // namespace bnl
