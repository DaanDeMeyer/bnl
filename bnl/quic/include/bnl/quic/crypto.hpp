#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/quic/export.hpp>
#include <bnl/quic/result.hpp>

#include <cstddef>

namespace bnl {
namespace quic {

class BNL_QUIC_EXPORT crypto {
public:
  enum class level { initial, early_data, handshake, application };

  enum class aead { aes_128_gcm, aes_256_gcm, chacha20_poly1305 };

  enum class hash { sha256, sha384 };

  class key_view {
  public:
    key_view() = default;
    key_view(base::buffer_view data,
             base::buffer_view iv,
             base::buffer_view hp) noexcept;

    base::buffer_view data() const noexcept;
    base::buffer_view iv() const noexcept;
    base::buffer_view hp() const noexcept;

  private:
    base::buffer_view data_;
    base::buffer_view iv_;
    base::buffer_view hp_;
  };

  class key {
  public:
    key() = default;
    key(base::buffer data, base::buffer iv, base::buffer hp) noexcept;

    base::buffer_view data() const noexcept;
    base::buffer_view iv() const noexcept;
    base::buffer_view hp() const noexcept;

    operator key_view() const noexcept; // NOLINT

  private:
    base::buffer data_;
    base::buffer iv_;
    base::buffer hp_;
  };

  crypto() = default;
  crypto(aead aead, hash hash);

  crypto(crypto &&) = default;
  crypto &operator=(crypto &&) = default;

  result<base::buffer> initial_secret(base::buffer_view dcid,
                                      base::buffer_view salt);
  result<base::buffer> update_secret(base::buffer_view secret);

  result<base::buffer> client_initial_secret(base::buffer_view secret);
  result<base::buffer> server_initial_secret(base::buffer_view secret);

  result<crypto::key> packet_protection_key(base::buffer_view secret);

  size_t aead_overhead() const noexcept;

  result<void> encrypt(base::buffer_view_mut dest,
                       base::buffer_view plaintext,
                       base::buffer_view key,
                       base::buffer_view nonce,
                       base::buffer_view ad);

  result<void> decrypt(base::buffer_view_mut dest,
                       base::buffer_view ciphertext,
                       base::buffer_view key,
                       base::buffer_view nonce,
                       base::buffer_view ad);

  result<void> hp_mask(base::buffer_view_mut dest,
                       base::buffer_view key,
                       base::buffer_view sample);

private:
  result<base::buffer> packet_protection_data(base::buffer_view secret);
  result<base::buffer> packet_protection_iv(base::buffer_view secret);
  result<base::buffer> packet_protection_hp(base::buffer_view secret);

  size_t aead_key_size(aead aead) const noexcept;
  size_t aead_nonce_min_size(aead aead) const noexcept;

  size_t hash_size(hash hash) const noexcept;

  result<base::buffer> hkdf_expand_label(base::buffer_view secret,
                                         base::buffer_view label,
                                         size_t size);

  result<base::buffer> hkdf_extract(base::buffer_view secret,
                                    base::buffer_view salt);

  result<base::buffer> hkdf_expand(base::buffer_view prk,
                                   base::buffer_view info,
                                   size_t size);

  friend std::ostream &operator<<(std::ostream &os, const crypto &crypto);

private:
  aead aead_ = aead::aes_128_gcm;
  hash hash_ = hash::sha256;
};

std::ostream &
operator<<(std::ostream &os, crypto::level level);

std::ostream &
operator<<(std::ostream &os, crypto::aead aead);

std::ostream &
operator<<(std::ostream &os, crypto::hash hash);

}
}
