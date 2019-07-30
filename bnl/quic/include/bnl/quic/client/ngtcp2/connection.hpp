#pragma once

#include <bnl/quic/clock.hpp>
#include <bnl/quic/crypto.hpp>
#include <bnl/quic/params.hpp>
#include <bnl/quic/path.hpp>

#include <memory>
#include <random>

using ngtcp2_conn = struct ngtcp2_conn;
using ngtcp2_pkt_stateless_reset = struct ngtcp2_pkt_stateless_reset;
using ngtcp2_pkt_hd = struct ngtcp2_pkt_hd;
using ngtcp2_pkt_retry = struct ngtcp2_pkt_retry;
using ngtcp2_cid = struct ngtcp2_cid;
using ngtcp2_path = struct ngtcp2_path;
using ngtcp2_addr = struct ngtcp2_addr;
using ngtcp2_preferred_addr = struct ngtcp2_preferred_addr;

enum ngtcp2_crypto_level : int;
enum ngtcp2_rand_ctx : int;
enum ngtcp2_path_validation_result : int;

namespace bnl {
namespace quic {
namespace client {

class connection;

namespace ngtcp2 {

class BNL_QUIC_EXPORT connection {
public:
  connection(path path,
             const params &params,
             client::connection *context,
             clock clock,
             std::mt19937 &prng);

  connection(connection &&) = default;
  connection &operator=(connection &&) = default;

  static const base::buffer_view INITIAL_SALT;
  static const base::buffer_view ALPN_H3;

  void set_aead_overhead(size_t overhead);

  result<void> install_initial_tx_keys(crypto::key_view key);
  result<void> install_initial_rx_keys(crypto::key_view key);

  result<void> install_early_keys(crypto::key_view key);

  result<void> install_handshake_tx_keys(crypto::key_view key);
  result<void> install_handshake_rx_keys(crypto::key_view key);

  result<void> install_tx_keys(crypto::key_view key);
  result<void> install_rx_keys(crypto::key_view key);

  result<void> update_tx_keys(crypto::key_view key);
  result<void> update_rx_keys(crypto::key_view key);

  bool get_handshake_completed() const noexcept;
  void handshake_completed() noexcept;

  result<base::buffer> get_local_transport_parameters() noexcept;
  result<void> set_remote_transport_parameters(
    base::buffer_view encoded) noexcept;

  result<void> submit_crypto_data(crypto::level level, base::buffer_view data);

  result<base::buffer> write_pkt();

  result<std::pair<base::buffer, size_t>> write_stream(uint64_t id,
                                                       base::buffer_view data,
                                                       bool fin);

  result<void> read_pkt(base::buffer_view packet);

  base::buffer_view dcid() const noexcept;

  duration timeout() const noexcept;
  duration expiry() const noexcept;

  result<void> expire();

  result<void> open(uint64_t id);

private:
  static int client_initial(ngtcp2_conn *connection, void *context);

  static int recv_crypto_data(ngtcp2_conn *connection,
                              ngtcp2_crypto_level level,
                              uint64_t offset,
                              const uint8_t *data,
                              size_t size,
                              void *context);

  static int handshake_completed(ngtcp2_conn *connection, void *context);

  static ssize_t in_encrypt(ngtcp2_conn *connection,
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
                            void *context);

  static ssize_t in_decrypt(ngtcp2_conn *connection,
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
                            void *context);

  static ssize_t encrypt(ngtcp2_conn *connection,
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
                         void *context);

  static ssize_t decrypt(ngtcp2_conn *connection,
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
                         void *context);

  static ssize_t in_hp_mask(ngtcp2_conn *connection,
                            uint8_t *dest,
                            size_t dest_size,
                            const uint8_t *key,
                            size_t key_size,
                            const uint8_t *sample,
                            size_t sample_size,
                            void *context);

  static ssize_t hp_mask(ngtcp2_conn *connection,
                         uint8_t *dest,
                         size_t dest_size,
                         const uint8_t *key,
                         size_t key_size,
                         const uint8_t *sample,
                         size_t sample_size,
                         void *context);

  static int recv_stream_data(ngtcp2_conn *connection,
                              int64_t id,
                              int fin,
                              uint64_t offset,
                              const uint8_t *data,
                              size_t size,
                              void *context,
                              void *stream_context);

  static int acked_crypto_offset(ngtcp2_conn *connection,
                                 ngtcp2_crypto_level level,
                                 uint64_t offset,
                                 size_t size,
                                 void *context);

  static int acked_stream_data_offset(ngtcp2_conn *connection,
                                      int64_t id,
                                      uint64_t offset,
                                      size_t size,
                                      void *context,
                                      void *stream_context);

  static int stream_open(ngtcp2_conn *connection, int64_t id, void *context);

  static int stream_close(ngtcp2_conn *connection,
                          int64_t id,
                          uint64_t error,
                          void *context,
                          void *stream_context);

  static int recv_stateless_reset(ngtcp2_conn *connection,
                                  const ngtcp2_pkt_stateless_reset *reset,
                                  void *context);

  static int recv_retry(ngtcp2_conn *connection,
                        const ngtcp2_pkt_hd *packet_header,
                        const ngtcp2_pkt_retry *retry,
                        void *context);

  static int extend_max_local_streams_bidi(ngtcp2_conn *connection,
                                           uint64_t max_streams,
                                           void *context);

  static int extend_max_local_streams_uni(ngtcp2_conn *connection,
                                          uint64_t max_streams,
                                          void *context);

  static int rand(ngtcp2_conn *connection,
                  uint8_t *dest,
                  size_t size,
                  ngtcp2_rand_ctx usage,
                  void *context);

  static int get_new_connection_id(ngtcp2_conn *connection,
                                   ngtcp2_cid *cid,
                                   uint8_t *token,
                                   size_t cid_size,
                                   void *context);

  static int remove_connection_id(ngtcp2_conn *connection,
                                  const ngtcp2_cid *cid,
                                  void *context);

  static int update_key(ngtcp2_conn *connection, void *context);

  static int path_validation(ngtcp2_conn *connection,
                             const ngtcp2_path *path,
                             ngtcp2_path_validation_result result,
                             void *context);

  static int select_preferred_addr(ngtcp2_conn *connection,
                                   ngtcp2_addr *dest,
                                   const ngtcp2_preferred_addr *preferred,
                                   void *context);

  static int stream_reset(ngtcp2_conn *connection,
                          int64_t id,
                          uint64_t final_size,
                          uint64_t error,
                          void *context,
                          void *stream_context);

  static int extend_max_remote_streams_bidi(ngtcp2_conn *connection,
                                            uint64_t max_streams,
                                            void *context);

  static int extend_max_remote_streams_uni(ngtcp2_conn *connection,
                                           uint64_t max_streams,
                                           void *context);

  static int extend_max_stream_data(ngtcp2_conn *connection,
                                    int64_t id,
                                    uint64_t max_data,
                                    void *context,
                                    void *stream_context);

  static void log(void *context, const char *format, ...); // NOLINT

private:
  std::unique_ptr<ngtcp2_conn, void (*)(ngtcp2_conn *)> connection_;

  path path_;
  clock clock_;
};

}
}
}
}
