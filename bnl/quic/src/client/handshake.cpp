#include <bnl/quic/client/handshake.hpp>

#include <bnl/base/enum.hpp>
#include <bnl/base/log.hpp>
#include <bnl/quic/client/ngtcp2/connection.hpp>

namespace bnl {
namespace quic {
namespace client {

result<void>
handshake::ack(crypto::level level, size_t size)
{
  base::buffers &keepalive = keepalive_[enumeration::value(level)];

  if (size > keepalive.size()) {
    BNL_LOG_E("ngtcp2's acked crypto data ({}) exceeds remaining data ({})",
              size,
              keepalive.size());
    return error::internal;
  }

  keepalive.consume(size);

  return base::success();
}

bool
handshake::completed() const noexcept
{
  return ngtcp2_->get_handshake_completed();
}

result<void>
handshake::update_keys()
{
  quic::crypto crypto = BNL_TRY(this->negotiated_crypto());

  tx_secret_ = BNL_TRY(crypto.update_secret(tx_secret_));
  crypto::key write_key = BNL_TRY(crypto.packet_protection_key(tx_secret_));
  BNL_TRY(ngtcp2_->update_tx_keys(write_key));

  rx_secret_ = BNL_TRY(crypto.update_secret(rx_secret_));
  crypto::key read_key = BNL_TRY(crypto.packet_protection_key(rx_secret_));
  BNL_TRY(ngtcp2_->update_rx_keys(read_key));

  return base::success();
}

result<void>
handshake::set_encryption_secrets(crypto::level level,
                                  base::buffer_view read_secret,
                                  base::buffer_view write_secret)
{
  quic::crypto crypto = BNL_TRY(this->negotiated_crypto());

  BNL_LOG_T("{}", crypto);

  ngtcp2_->set_aead_overhead(crypto.aead_overhead());

  crypto::key write_key = BNL_TRY(crypto.packet_protection_key(write_secret));
  crypto::key read_key;

  // Data is not received at the 0-RTT level by the client so BoringSSL does not
  // provide the client with a key for receiving early data.
  if (level != crypto::level::early_data) {
    read_key = BNL_TRY(crypto.packet_protection_key(read_secret));
  }

  switch (level) {
    case crypto::level::initial:
      return quic::error::handshake;
    case crypto::level::early_data:
      BNL_TRY(ngtcp2_->install_early_keys(write_key));
      break;
    case crypto::level::handshake:
      BNL_TRY(ngtcp2_->install_handshake_tx_keys(write_key));
      BNL_TRY(ngtcp2_->install_handshake_rx_keys(read_key));
      BNL_LOG_T("handshake: installed handshake keys");
      break;
    case crypto::level::application:
      tx_secret_ = base::buffer(write_secret);
      BNL_TRY(ngtcp2_->install_tx_keys(write_key));

      rx_secret_ = base::buffer(read_secret);
      BNL_TRY(ngtcp2_->install_rx_keys(read_key));

      BNL_LOG_T("handshake: installed application keys");
      break;
  }

  return base::success();
}

result<void>
handshake::add_handshake_data(crypto::level level, base::buffer_view data)
{
  base::buffers &keepalive = keepalive_[enumeration::value(level)];

  keepalive.push(base::buffer(data));
  const base::buffer &buffer = keepalive.back();

  BNL_TRY(ngtcp2_->submit_crypto_data(level, buffer));

  return base::success();
}

}
}
}
