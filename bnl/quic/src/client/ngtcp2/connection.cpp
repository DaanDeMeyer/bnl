#include <bnl/quic/client/ngtcp2/connection.hpp>

#include <bnl/base/error.hpp>
#include <bnl/quic/client/connection.hpp>
#include <bnl/quic/client/ngtcp2/error.hpp>
#include <bnl/quic/error.hpp>
#include <bnl/quic/path.hpp>
#include <bnl/util/error.hpp>

#include <ngtcp2/ngtcp2.h>

#include <algorithm>
#include <cassert>

namespace bnl {
namespace quic {
namespace client {
namespace ngtcp2 {

#define THROW_NGTCP2(function, rv)                                             \
  {                                                                            \
    code code_ = make_status_code(static_cast<error>(rv));                     \
    fmt::string_view view = { code_.message().data(),                          \
                              code_.message().size() };                        \
    LOG_E("{}: {}", #function, view);                                          \
    return code_;                                                              \
  }                                                                            \
  (void) 0

static crypto::level
make_crypto_level(ngtcp2_crypto_level level)
{
  switch (level) {
    case NGTCP2_CRYPTO_LEVEL_INITIAL:
      return crypto::level::initial;
    case NGTCP2_CRYPTO_LEVEL_EARLY:
      return crypto::level::early_data;
    case NGTCP2_CRYPTO_LEVEL_HANDSHAKE:
      return crypto::level::handshake;
    case NGTCP2_CRYPTO_LEVEL_APP:
      return crypto::level::application;
  }

  assert(false);
  return crypto::level::initial;
}

static ngtcp2_crypto_level
make_crypto_level(crypto::level level)
{
  switch (level) {
    case crypto::level::initial:
      return NGTCP2_CRYPTO_LEVEL_INITIAL;
    case crypto::level::early_data:
      return NGTCP2_CRYPTO_LEVEL_EARLY;
    case crypto::level::handshake:
      return NGTCP2_CRYPTO_LEVEL_HANDSHAKE;
    case crypto::level::application:
      return NGTCP2_CRYPTO_LEVEL_APP;
  }

  assert(false);
  return NGTCP2_CRYPTO_LEVEL_INITIAL;
}

static ngtcp2_path_storage
make_path(const path &path)
{
  base::buffer_view local = path.local().address().bytes();
  base::buffer_view peer = path.local().address().bytes();

  ngtcp2_path_storage storage = {};
  ngtcp2_path_storage_init(&storage,
                           local.data(),
                           local.size(),
                           nullptr,
                           peer.data(),
                           peer.size(),
                           nullptr);

  return storage;
}

static ngtcp2_tstamp
make_timestamp(duration timestamp)
{
  return timestamp.count() * NGTCP2_MICROSECONDS;
}

static duration
make_timestamp(ngtcp2_tstamp timestamp)
{
  return duration(timestamp / NGTCP2_MICROSECONDS);
}

int
connection::client_initial(ngtcp2_conn *connection, void *context)
{
  (void) connection;
  auto client = static_cast<client::connection *>(context);

  result<void> r = client->client_initial();

  return r ? 0 : NGTCP2_ERR_CALLBACK_FAILURE;
}

int
connection::recv_crypto_data(ngtcp2_conn *connection,
                             ngtcp2_crypto_level level,
                             uint64_t offset,
                             const uint8_t *data,
                             size_t size,
                             void *context)
{
  (void) connection;
  (void) offset;
  auto client = static_cast<client::connection *>(context);

  result<void> r = client->recv_crypto_data(make_crypto_level(level),
                                            base::buffer_view(data, size));

  if (!r && (r.error() == quic::error::handshake ||
             r.error() == quic::error::crypto)) {
    return NGTCP2_ERR_CRYPTO;
  }

  if (!r) {
    return NGTCP2_ERR_CALLBACK_FAILURE;
  }

  return 0;
}

int
connection::handshake_completed(ngtcp2_conn *connection, void *context)
{
  (void) connection;
  auto client = static_cast<client::connection *>(context);

  client->handshake_completed();

  return 0;
}

ssize_t
connection::in_encrypt(ngtcp2_conn *connection,
                       uint8_t *dest,
                       size_t dest_size,
                       const uint8_t *plaintext,
                       size_t plaintext_size,
                       const uint8_t *key,
                       size_t key_size,
                       const uint8_t *nonce,
                       size_t nonce_size,
                       const uint8_t *ad,
                       size_t ad_size,
                       void *context)
{
  (void) connection;
  auto client = static_cast<client::connection *>(context);

  crypto crypto(
    crypto::aead::aes_128_gcm, crypto::hash::sha256, client->logger());

  result<void> r = crypto.encrypt(base::buffer_view_mut(dest, dest_size),
                                  base::buffer_view(plaintext, plaintext_size),
                                  base::buffer_view(key, key_size),
                                  base::buffer_view(nonce, nonce_size),
                                  base::buffer_view(ad, ad_size));

  // TODO: Fix after https://github.com/ngtcp2/ngtcp2/pull/128
  return r ? static_cast<ssize_t>(plaintext_size + crypto.aead_overhead())
           : NGTCP2_ERR_CALLBACK_FAILURE;
}

ssize_t
connection::in_decrypt(ngtcp2_conn *connection,
                       uint8_t *dest,
                       size_t dest_size,
                       const uint8_t *ciphertext,
                       size_t ciphertext_size,
                       const uint8_t *key,
                       size_t key_size,
                       const uint8_t *nonce,
                       size_t nonce_size,
                       const uint8_t *ad,
                       size_t ad_size,
                       void *context)
{
  (void) connection;
  auto client = static_cast<client::connection *>(context);

  crypto crypto(
    crypto::aead::aes_128_gcm, crypto::hash::sha256, client->logger());

  result<void> r =
    crypto.decrypt(base::buffer_view_mut(dest, dest_size),
                   base::buffer_view(ciphertext, ciphertext_size),
                   base::buffer_view(key, key_size),
                   base::buffer_view(nonce, nonce_size),
                   base::buffer_view(ad, ad_size));

  // TODO: Fix after https://github.com/ngtcp2/ngtcp2/pull/128
  return r ? static_cast<ssize_t>(ciphertext_size - crypto.aead_overhead())
           : static_cast<ssize_t>(NGTCP2_ERR_CALLBACK_FAILURE);
}

ssize_t
connection::encrypt(ngtcp2_conn *connection,
                    uint8_t *dest,
                    size_t dest_size,
                    const uint8_t *plaintext,
                    size_t plaintext_size,
                    const uint8_t *key,
                    size_t key_size,
                    const uint8_t *nonce,
                    size_t nonce_size,
                    const uint8_t *ad,
                    size_t ad_size,
                    void *context)
{
  (void) connection;
  auto client = static_cast<client::connection *>(context);

  crypto crypto = ({
    result<quic::crypto> r = client->crypto();
    if (!r) {
      return NGTCP2_ERR_CALLBACK_FAILURE;
    }

    std::move(r).value();
  });

  result<void> r = crypto.encrypt(base::buffer_view_mut(dest, dest_size),
                                  base::buffer_view(plaintext, plaintext_size),
                                  base::buffer_view(key, key_size),
                                  base::buffer_view(nonce, nonce_size),
                                  base::buffer_view(ad, ad_size));

  // TODO: Fix after https://github.com/ngtcp2/ngtcp2/pull/128
  return r ? static_cast<ssize_t>(plaintext_size + crypto.aead_overhead())
           : static_cast<ssize_t>(NGTCP2_ERR_CALLBACK_FAILURE);
}

ssize_t
connection::decrypt(ngtcp2_conn *connection,
                    uint8_t *dest,
                    size_t dest_size,
                    const uint8_t *ciphertext,
                    size_t ciphertext_size,
                    const uint8_t *key,
                    size_t key_size,
                    const uint8_t *nonce,
                    size_t nonce_size,
                    const uint8_t *ad,
                    size_t ad_size,
                    void *context)
{
  (void) connection;
  auto client = static_cast<client::connection *>(context);

  crypto crypto = ({
    result<quic::crypto> r = client->crypto();
    if (!r) {
      return NGTCP2_ERR_CALLBACK_FAILURE;
    }

    std::move(r).value();
  });

  result<void> r =
    crypto.decrypt(base::buffer_view_mut(dest, dest_size),
                   base::buffer_view(ciphertext, ciphertext_size),
                   base::buffer_view(key, key_size),
                   base::buffer_view(nonce, nonce_size),
                   base::buffer_view(ad, ad_size));

  // TODO: Fix after https://github.com/ngtcp2/ngtcp2/pull/128
  return r ? static_cast<ssize_t>(ciphertext_size - crypto.aead_overhead())
           : static_cast<ssize_t>(NGTCP2_ERR_CALLBACK_FAILURE);
}

ssize_t
connection::in_hp_mask(ngtcp2_conn *connection,
                       uint8_t *dest,
                       size_t dest_size,
                       const uint8_t *key,
                       size_t key_size,
                       const uint8_t *sample,
                       size_t sample_size,
                       void *context)
{
  (void) connection;
  auto client = static_cast<client::connection *>(context);

  crypto crypto(
    crypto::aead::aes_128_gcm, crypto::hash::sha256, client->logger());

  result<void> r = crypto.hp_mask(base::buffer_view_mut(dest, dest_size),
                                  base::buffer_view(key, key_size),
                                  base::buffer_view(sample, sample_size));

  return r ? NGTCP2_HP_MASKLEN : NGTCP2_ERR_CALLBACK_FAILURE;
}

ssize_t
connection::hp_mask(ngtcp2_conn *connection,
                    uint8_t *dest,
                    size_t dest_size,
                    const uint8_t *key,
                    size_t key_size,
                    const uint8_t *sample,
                    size_t sample_size,
                    void *context)
{
  (void) connection;
  auto client = static_cast<client::connection *>(context);

  crypto crypto = ({
    result<quic::crypto> r = client->crypto();
    if (!r) {
      return NGTCP2_ERR_CALLBACK_FAILURE;
    }

    std::move(r).value();
  });

  result<void> r = crypto.hp_mask(base::buffer_view_mut(dest, dest_size),
                                  base::buffer_view(key, key_size),
                                  base::buffer_view(sample, sample_size));

  return r ? NGTCP2_HP_MASKLEN : NGTCP2_ERR_CALLBACK_FAILURE;
}

int
connection::recv_stream_data(ngtcp2_conn *connection,
                             int64_t id,
                             int fin,
                             uint64_t offset,
                             const uint8_t *data,
                             size_t size,
                             void *context,
                             void *stream_context)
{
  (void) connection;
  (void) offset;
  (void) stream_context;
  auto client = static_cast<client::connection *>(context);

  client->recv_stream_data(
    static_cast<uint64_t>(id), fin != 0, base::buffer_view(data, size));

  return 0;
}

int
connection::acked_crypto_offset(ngtcp2_conn *connection,
                                ngtcp2_crypto_level level,
                                uint64_t offset,
                                size_t size,
                                void *context)
{
  (void) connection;
  (void) offset;
  auto client = static_cast<client::connection *>(context);

  result<void> r = client->acked_crypto_offset(make_crypto_level(level), size);

  return r ? 0 : NGTCP2_ERR_CALLBACK_FAILURE;
}

int
connection::acked_stream_data_offset(ngtcp2_conn *connection,
                                     int64_t id,
                                     uint64_t offset,
                                     size_t size,
                                     void *context,
                                     void *stream_context)
{
  (void) connection;
  (void) offset;
  (void) stream_context;
  auto client = static_cast<client::connection *>(context);

  result<void> r =
    client->acked_stream_data_offset(static_cast<uint64_t>(id), size);

  return r ? 0 : NGTCP2_ERR_CALLBACK_FAILURE;
}

int
connection::stream_open(ngtcp2_conn *connection, int64_t id, void *context)
{
  (void) connection;
  auto client = static_cast<client::connection *>(context);

  client->stream_opened(static_cast<uint64_t>(id));

  return 0;
}

int
connection::stream_close(ngtcp2_conn *connection,
                         int64_t id,
                         uint64_t error,
                         void *context,
                         void *stream_context)
{
  (void) connection;
  (void) id;
  (void) stream_context;
  auto client = static_cast<client::connection *>(context);

  client->stream_closed(static_cast<uint64_t>(id), error);

  return 0;
}

int
connection::recv_stateless_reset(ngtcp2_conn *connection,
                                 const ngtcp2_pkt_stateless_reset *reset,
                                 void *context)
{
  (void) connection;
  auto client = static_cast<client::connection *>(context);

  result<void> r = client->recv_stateless_reset(
    base::buffer_view(reset->rand, reset->randlen),
    base::buffer_view(reset->stateless_reset_token,
                      NGTCP2_STATELESS_RESET_TOKENLEN));

  return r ? 0 : NGTCP2_ERR_CALLBACK_FAILURE;
}

int
connection::recv_retry(ngtcp2_conn *connection,
                       const ngtcp2_pkt_hd *packet_header,
                       const ngtcp2_pkt_retry *retry,
                       void *context)
{
  (void) packet_header;
  (void) retry;
  auto client = static_cast<client::connection *>(context);

  const ngtcp2_cid *dcid = ngtcp2_conn_get_dcid(connection);

  result<void> r =
    client->recv_retry(base::buffer_view(dcid->data, dcid->datalen));

  return r ? 0 : NGTCP2_ERR_CALLBACK_FAILURE;
}

int
connection::extend_max_local_streams_bidi(ngtcp2_conn *connection,
                                          uint64_t max_streams,
                                          void *context)
{
  (void) connection;
  auto client = static_cast<client::connection *>(context);

  client->extend_max_local_streams_bidi(max_streams);

  return 0;
}

int
connection::extend_max_local_streams_uni(ngtcp2_conn *connection,
                                         uint64_t max_streams,
                                         void *context)
{
  (void) connection;
  auto client = static_cast<client::connection *>(context);

  client->extend_max_local_streams_bidi(max_streams);

  return 0;
}

int
connection::rand(ngtcp2_conn *connection,
                 uint8_t *dest,
                 size_t size,
                 ngtcp2_rand_ctx usage,
                 void *context)
{
  (void) connection;
  (void) usage;
  auto client = static_cast<client::connection *>(context);

  client->random(base::buffer_view_mut(dest, size));

  return 0;
}

int
connection::get_new_connection_id(ngtcp2_conn *connection,
                                  ngtcp2_cid *cid,
                                  uint8_t *token,
                                  size_t cid_size,
                                  void *context)
{
  (void) connection;
  auto client = static_cast<client::connection *>(context);

  client->new_connection_id(base::buffer_view_mut(cid->data, cid_size));
  cid->datalen = cid_size;
  client->new_stateless_reset_token(
    base::buffer_view_mut(token, NGTCP2_STATELESS_RESET_TOKENLEN));

  return 0;
}

int
connection::remove_connection_id(ngtcp2_conn *connection,
                                 const ngtcp2_cid *cid,
                                 void *context)
{
  (void) connection;
  auto client = static_cast<client::connection *>(context);

  client->remove_connection_id(base::buffer_view(cid->data, cid->datalen));

  return 0;
}

int
connection::update_key(ngtcp2_conn *connection, void *context)
{
  (void) connection;
  auto client = static_cast<client::connection *>(context);

  result<void> r = client->update_key();

  return r ? 0 : NGTCP2_ERR_CALLBACK_FAILURE;
}

int
connection::path_validation(ngtcp2_conn *connection,
                            const ngtcp2_path *path,
                            ngtcp2_path_validation_result pv_result,
                            void *context)
{
  (void) connection;
  auto client = static_cast<client::connection *>(context);

  base::buffer_view local(path->local.addr, path->local.addrlen);
  base::buffer_view peer(path->local.addr, path->local.addrlen);
  bool succeeded = pv_result == NGTCP2_PATH_VALIDATION_RESULT_SUCCESS;

  result<void> r = client->path_validation(local, peer, succeeded);

  return r ? 0 : NGTCP2_ERR_CALLBACK_FAILURE;
}

int
connection::select_preferred_addr(ngtcp2_conn *connection,
                                  ngtcp2_addr *dest,
                                  const ngtcp2_preferred_addr *preferred,
                                  void *context)
{
  (void) connection;
  auto client = static_cast<client::connection *>(context);

  ip::endpoint ipv4(ipv4::address(preferred->ipv4_addr), preferred->ipv4_port);
  ip::endpoint ipv6(ipv6::address(preferred->ipv6_addr), preferred->ipv6_port);

  result<void> r = client->select_preferred_address(
    base::buffer_view_mut(dest->addr, dest->addrlen),
    ipv4,
    ipv6,
    base::buffer_view(preferred->stateless_reset_token,
                      NGTCP2_STATELESS_RESET_TOKENLEN));

  return r ? 0 : NGTCP2_ERR_CALLBACK_FAILURE;
}

int
connection::stream_reset(ngtcp2_conn *connection,
                         int64_t id,
                         uint64_t final_size,
                         uint64_t error,
                         void *context,
                         void *stream_context)
{
  (void) connection;
  (void) stream_context;
  auto client = static_cast<client::connection *>(context);

  client->stream_reset(static_cast<uint64_t>(id), final_size, error);

  return 0;
}

int
connection::extend_max_remote_streams_bidi(ngtcp2_conn *connection,
                                           uint64_t max_streams,
                                           void *context)
{
  (void) connection;
  auto client = static_cast<client::connection *>(context);

  client->extend_max_remote_streams_bidi(max_streams);

  return 0;
}

int
connection::extend_max_remote_streams_uni(ngtcp2_conn *connection,
                                          uint64_t max_streams,
                                          void *context)
{
  (void) connection;
  auto client = static_cast<client::connection *>(context);

  client->extend_max_remote_streams_uni(max_streams);

  return 0;
}

int
connection::extend_max_stream_data(ngtcp2_conn *connection,
                                   int64_t id,
                                   uint64_t max_data,
                                   void *context,
                                   void *stream_context)
{
  (void) connection;
  (void) stream_context;
  auto client = static_cast<client::connection *>(context);

  client->extend_max_stream_data(static_cast<uint64_t>(id), max_data);

  return 0;
}

void
connection::log(void *context, const char *format, ...) // NOLINT
{
  auto client = static_cast<client::connection *>(context);

  // C varargs are a runtime concept, the logging macros expect variadic
  // templates at compile time. As a result, we can't pass C varargs to the
  // logging macros. To get around this, we format the string using `vsnprintf`
  // ourselves and pass the formatted string to the logging macros without any
  // arguments.

  std::array<char, 1024> formatted = {};

  va_list args;
  va_start(args, format); // NOLINT

  vsnprintf(formatted.data(), formatted.size(), format, args);

  va_end(args); // NOLINT

  BNL_LOG_TRACE(client->logger(), formatted.data());
}

static ngtcp2_cid
make_cid(std::mt19937 &prng)
{
  auto dist = std::uniform_int_distribution<uint8_t>();

  ngtcp2_cid connection_id;
  connection_id.datalen = NGTCP2_MAX_CIDLEN;

  uint8_t *begin = connection_id.data;
  uint8_t *end = connection_id.data + connection_id.datalen;

  std::generate(begin, end, [&dist, &prng]() { return dist(prng); });

  return connection_id;
}

static ngtcp2_settings
make_settings(const params &params)
{
  ngtcp2_settings settings;
  ngtcp2_settings_default(&settings);

  settings.max_stream_data_bidi_local = params.max_stream_data_bidi_local;
  settings.max_stream_data_bidi_remote = params.max_stream_data_bidi_remote;
  settings.max_stream_data_uni = params.max_stream_data_uni;
  settings.max_data = params.max_data;
  settings.max_streams_bidi = params.max_streams_bidi;
  settings.max_streams_uni = params.max_streams_uni;
  settings.idle_timeout = params.idle_timeout.count();
  settings.max_packet_size = params.max_packet_size;
  settings.ack_delay_exponent = params.ack_delay_exponent;
  settings.disable_migration = params.disable_migration;
  settings.max_ack_delay = params.max_ack_delay.count();

  return settings;
}

connection::connection(path path,
                       const params &params,
                       client::connection *context,
                       clock clock,
                       std::mt19937 &prng,
                       const log::api *logger)
  : connection_(nullptr, ngtcp2_conn_del)
  , path_(path)
  , clock_(std::move(clock))
  , logger_(logger)
{
  ngtcp2_cid scid = make_cid(prng);
  ngtcp2_cid dcid = make_cid(prng);

  ngtcp2_conn_callbacks callbacks = { client_initial,
                                      nullptr, // recv_client_initial
                                      recv_crypto_data,
                                      handshake_completed,
                                      nullptr, // recv_version_negotiation
                                      in_encrypt,
                                      in_decrypt,
                                      encrypt,
                                      decrypt,
                                      in_hp_mask,
                                      hp_mask,
                                      recv_stream_data,
                                      acked_crypto_offset,
                                      acked_stream_data_offset,
                                      stream_open,
                                      stream_close,
                                      recv_stateless_reset,
                                      recv_retry,
                                      extend_max_local_streams_bidi,
                                      extend_max_local_streams_uni,
                                      rand,
                                      get_new_connection_id,
                                      remove_connection_id,
                                      update_key,
                                      path_validation,
                                      select_preferred_addr,
                                      stream_reset,
                                      extend_max_remote_streams_bidi,
                                      extend_max_remote_streams_uni,
                                      extend_max_stream_data };

  ngtcp2_settings settings = make_settings(params);

  duration initial_ts = clock_().assume_value(); // TODO: Handle error

  settings.initial_ts = make_timestamp(initial_ts);
  settings.log_printf = log;

  ngtcp2_path_storage ngtcp2_path = make_path(path_);

  ngtcp2_conn *connection = nullptr;

  int rv = ngtcp2_conn_client_new(&connection,
                                  &dcid,
                                  &scid,
                                  &ngtcp2_path.path,
                                  NGTCP2_PROTO_VER,
                                  &callbacks,
                                  &settings,
                                  nullptr,
                                  context);
  // TODO: re-enable exceptions
  (void) rv;
  assert(rv == 0);

  connection_ = decltype(connection_)(connection, ngtcp2_conn_del);
}

const base::buffer_view connection::INITIAL_SALT = NGTCP2_INITIAL_SALT;
const base::buffer_view connection::ALPN_H3 = NGTCP2_ALPN_H3;

void
connection::set_aead_overhead(size_t overhead)
{
  ngtcp2_conn_set_aead_overhead(connection_.get(), overhead);
}

bool
connection::get_handshake_completed() const noexcept
{
  return ngtcp2_conn_get_handshake_completed(connection_.get()) != 0;
}

void
connection::handshake_completed() noexcept
{
  ngtcp2_conn_handshake_completed(connection_.get());
}

result<base::buffer>
connection::get_local_transport_parameters() noexcept
{
  ngtcp2_transport_params params;
  ngtcp2_conn_get_local_transport_params(connection_.get(), &params);

  std::array<uint8_t, 64> tp = {};

  ssize_t nwrite = ngtcp2_encode_transport_params(
    tp.data(), tp.size(), NGTCP2_TRANSPORT_PARAMS_TYPE_CLIENT_HELLO, &params);
  if (nwrite < 0) {
    THROW_NGTCP2(ngtcp2_set_remote_transport_params,
                 static_cast<error>(nwrite));
  }

  return base::buffer(tp.data(), static_cast<size_t>(nwrite));
}

result<void>
connection::set_remote_transport_parameters(base::buffer_view encoded) noexcept
{
  ngtcp2_transport_params params;

  int rv = ngtcp2_decode_transport_params(
    &params,
    NGTCP2_TRANSPORT_PARAMS_TYPE_ENCRYPTED_EXTENSIONS,
    encoded.data(),
    encoded.size());
  if (rv != 0) {
    THROW_NGTCP2(ngtcp2_decode_transport_params, static_cast<error>(rv));
  }

  rv = ngtcp2_conn_set_remote_transport_params(connection_.get(), &params);
  if (rv != 0) {
    THROW_NGTCP2(ngtcp2_set_remote_transport_params, static_cast<error>(rv));
  }

  return bnl::success();
}

result<void>
connection::install_initial_tx_keys(crypto::key_view key)
{
  int rv = ngtcp2_conn_install_initial_tx_keys(connection_.get(),
                                               key.data().data(),
                                               key.data().size(),
                                               key.iv().data(),
                                               key.iv().size(),
                                               key.hp().data(),
                                               key.hp().size());
  if (rv != 0) {
    THROW_NGTCP2(ngtcp2_conn_install_initial_tx_keys, rv);
  }

  return bnl::success();
}

result<void>
connection::install_initial_rx_keys(crypto::key_view key)
{
  int rv = ngtcp2_conn_install_initial_rx_keys(connection_.get(),
                                               key.data().data(),
                                               key.data().size(),
                                               key.iv().data(),
                                               key.iv().size(),
                                               key.hp().data(),
                                               key.hp().size());
  if (rv != 0) {
    THROW_NGTCP2(ngtcp2_conn_install_initial_rx_keys, rv);
  }

  return bnl::success();
}

result<void>
connection::install_early_keys(crypto::key_view key)
{
  int rv = ngtcp2_conn_install_early_keys(connection_.get(),
                                          key.data().data(),
                                          key.data().size(),
                                          key.iv().data(),
                                          key.iv().size(),
                                          key.hp().data(),
                                          key.hp().size());
  if (rv != 0) {
    THROW_NGTCP2(ngtcp2_conn_install_early_keys, rv);
  }

  return bnl::success();
}

result<void>
connection::install_handshake_tx_keys(crypto::key_view key)
{
  int rv = ngtcp2_conn_install_handshake_tx_keys(connection_.get(),
                                                 key.data().data(),
                                                 key.data().size(),
                                                 key.iv().data(),
                                                 key.iv().size(),
                                                 key.hp().data(),
                                                 key.hp().size());
  if (rv != 0) {
    THROW_NGTCP2(ngtcp2_conn_install_handshake_tx_keys, rv);
  }

  return bnl::success();
}

result<void>
connection::install_handshake_rx_keys(crypto::key_view key)
{
  int rv = ngtcp2_conn_install_handshake_rx_keys(connection_.get(),
                                                 key.data().data(),
                                                 key.data().size(),
                                                 key.iv().data(),
                                                 key.iv().size(),
                                                 key.hp().data(),
                                                 key.hp().size());
  if (rv != 0) {
    THROW_NGTCP2(ngtcp2_conn_install_handshake_rx_keys, rv);
  }

  return bnl::success();
}

result<void>
connection::install_tx_keys(crypto::key_view key)
{
  int rv = ngtcp2_conn_install_tx_keys(connection_.get(),
                                       key.data().data(),
                                       key.data().size(),
                                       key.iv().data(),
                                       key.iv().size(),
                                       key.hp().data(),
                                       key.hp().size());
  if (rv != 0) {
    THROW_NGTCP2(ngtcp2_conn_install_tx_keys, rv);
  }

  return bnl::success();
}

result<void>
connection::install_rx_keys(crypto::key_view key)
{
  int rv = ngtcp2_conn_install_rx_keys(connection_.get(),
                                       key.data().data(),
                                       key.data().size(),
                                       key.iv().data(),
                                       key.iv().size(),
                                       key.hp().data(),
                                       key.hp().size());
  if (rv != 0) {
    THROW_NGTCP2(ngtcp2_conn_install_rx_keys, rv);
  }

  return bnl::success();
}

result<void>
connection::update_tx_keys(crypto::key_view key)
{
  int rv = ngtcp2_conn_update_tx_key(connection_.get(),
                                     key.data().data(),
                                     key.data().size(),
                                     key.iv().data(),
                                     key.iv().size());
  if (rv != 0) {
    THROW_NGTCP2(ngtcp2_conn_update_tx_key, rv);
  }

  return bnl::success();
}

result<void>
connection::update_rx_keys(crypto::key_view key)
{
  int rv = ngtcp2_conn_update_rx_key(connection_.get(),
                                     key.data().data(),
                                     key.data().size(),
                                     key.iv().data(),
                                     key.iv().size());
  if (rv != 0) {
    THROW_NGTCP2(ngtcp2_conn_update_rx_key, rv);
  }

  return bnl::success();
}

result<void>
connection::submit_crypto_data(crypto::level level, base::buffer_view data)
{
  int rv = ngtcp2_conn_submit_crypto_data(
    connection_.get(), make_crypto_level(level), data.data(), data.size());
  if (rv != 0) {
    THROW_NGTCP2(ngtcp2_submit_crypto_data, static_cast<error>(rv));
  }

  return bnl::success();
}

result<base::buffer>
connection::write_pkt()
{
  LOG_D("write_pkt");
  ngtcp2_path_storage path = make_path(path_);

  // TODO: Handle IPV6
  std::array<uint8_t, NGTCP2_MAX_PKTLEN_IPV4> storage = {};

  duration ts = TRY(clock_());
  ssize_t rv = ngtcp2_conn_write_pkt(connection_.get(),
                                     &path.path,
                                     storage.data(),
                                     storage.size(),
                                     make_timestamp(ts));
  if (rv == 0) {
    return base::error::idle;
  }

  if (rv < 0) {
    THROW_NGTCP2(ngtcp2_conn_write_pkt, static_cast<error>(rv));
  }

  return base::buffer(storage.data(), static_cast<size_t>(rv));
  ;
}

result<std::pair<base::buffer, size_t>>
connection::write_stream(uint64_t id, base::buffer_view data, bool fin)
{
  LOG_D("write_stream");
  std::array<uint8_t, NGTCP2_MAX_PKTLEN_IPV4> storage = {};

  ssize_t stream_data_written = 0;

  duration ts = TRY(clock_());
  ssize_t rv = ngtcp2_conn_write_stream(connection_.get(),
                                        nullptr,
                                        storage.data(),
                                        storage.size(),
                                        &stream_data_written,
                                        NGTCP2_WRITE_STREAM_FLAG_NONE,
                                        static_cast<int64_t>(id),
                                        static_cast<uint8_t>(fin),
                                        data.data(),
                                        data.size(),
                                        make_timestamp(ts));
  if (rv < 0) {
    THROW_NGTCP2(ngtcp2_conn_write_stream, rv);
  }

  stream_data_written = stream_data_written == -1 ? 0 : stream_data_written;

  base::buffer packet(storage.data(), static_cast<size_t>(rv));

  return std::make_pair(std::move(packet),
                        static_cast<size_t>(stream_data_written));
}

result<void>
connection::read_pkt(base::buffer_view packet)
{
  ngtcp2_path_storage path = make_path(path_);

  duration ts = TRY(clock_());
  int rv = ngtcp2_conn_read_pkt(connection_.get(),
                                &path.path,
                                packet.data(),
                                packet.size(),
                                make_timestamp(ts));
  if (rv != 0) {
    THROW_NGTCP2(ngtcp2_conn_read_pkt, static_cast<error>(rv));
  }

  return bnl::success();
}

base::buffer_view
connection::dcid() const noexcept
{
  const ngtcp2_cid *dcid = ngtcp2_conn_get_dcid(connection_.get());
  return { dcid->data, dcid->datalen };
}

duration
connection::timeout() const noexcept
{
  uint64_t timeout = ngtcp2_conn_get_idle_timeout(connection_.get());
  return make_timestamp(timeout);
}

duration
connection::expiry() const noexcept
{
  uint64_t expiry = ngtcp2_conn_get_expiry(connection_.get());
  return make_timestamp(expiry);
}

result<void>
connection::expire()
{
  duration now = TRY(clock_());
  ngtcp2_tstamp ts = make_timestamp(now);

  if (ngtcp2_conn_loss_detection_expiry(connection_.get()) <= ts) {
    int rv = ngtcp2_conn_on_loss_detection_timer(connection_.get(), ts);
    if (rv != 0) {
      THROW_NGTCP2(ngtcp2_conn_on_loss_detection_timer, static_cast<error>(rv));
    }
  }

  if (ngtcp2_conn_ack_delay_expiry(connection_.get()) <= ts) {
    ngtcp2_conn_cancel_expired_ack_delay_timer(connection_.get(), ts);
  }

  return bnl::success();
}

result<void>
connection::open(uint64_t id)
{
  int64_t quic_id = 0;
  int rv = 0;

  if ((id & 0x2U) == 0) {
    rv = ngtcp2_conn_open_bidi_stream(connection_.get(), &quic_id, nullptr);
  } else {
    rv = ngtcp2_conn_open_uni_stream(connection_.get(), &quic_id, nullptr);
  }

  if (rv != 0) {
    THROW_NGTCP2(ngtcp2_conn_open_uni_stream, rv);
  }

  // Users are required to open streams in ascending order.
  assert(static_cast<uint64_t>(quic_id) == id);

  return bnl::success();
}

}
}
}
}
