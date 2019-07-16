#pragma once

#include <bnl/quic/clock.hpp>
#include <bnl/quic/params.hpp>
#include <bnl/quic/path.hpp>

#include <endpoint/shared/crypto.hpp>

#include <random>

using ngtcp2_conn = struct ngtcp2_conn;

namespace bnl {
namespace quic {
namespace endpoint {
namespace client {

class impl;

namespace ngtcp2 {

class connection {
public:
  connection(path path,
             const params &params,
             endpoint::client::impl *context,
             clock clock,
             std::mt19937 &prng,
             const log::api *logger);

  connection(connection &&other) = default;
  connection &operator=(connection &&other) = default;

  static const base::buffer_view INITIAL_SALT;
  static const base::buffer_view ALPN_H3;

  void set_aead_overhead(size_t overhead);

  std::error_code install_initial_tx_keys(crypto::key_view key);
  std::error_code install_initial_rx_keys(crypto::key_view key);

  std::error_code install_early_keys(crypto::key_view key);

  std::error_code install_handshake_tx_keys(crypto::key_view key);
  std::error_code install_handshake_rx_keys(crypto::key_view key);

  std::error_code install_tx_keys(crypto::key_view key);
  std::error_code install_rx_keys(crypto::key_view key);

  std::error_code update_tx_keys(crypto::key_view key);
  std::error_code update_rx_keys(crypto::key_view key);

  bool get_handshake_completed() const noexcept;
  void handshake_completed() noexcept;

  base::result<base::buffer> get_local_transport_parameters() noexcept;
  std::error_code set_remote_transport_parameters(
    base::buffer_view encoded) noexcept;

  std::error_code submit_crypto_data(crypto::level level,
                                     base::buffer_view data);

  base::result<base::buffer> write_pkt();

  base::result<std::pair<base::buffer, size_t>>
  write_stream(uint64_t id, base::buffer_view data, bool fin);

  std::error_code read_pkt(base::buffer_view packet);

  base::buffer_view dcid() const noexcept;

  duration timeout() const noexcept;
  duration expiry() const noexcept;

  std::error_code expire();

private:
  std::unique_ptr<ngtcp2_conn, void (*)(ngtcp2_conn *)> connection_;

  path path_;
  clock clock_;
  const log::api *logger_;
};

} // namespace ngtcp2
} // namespace client
} // namespace endpoint
} // namespace quic
} // namespace bnl
