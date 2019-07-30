#include <bnl/quic/crypto.hpp>

#include <bnl/log.hpp>
#include <bnl/quic/error.hpp>

#include <algorithm>
#include <array>

namespace bnl {
namespace quic {

crypto::key_view::key_view(base::buffer_view data,
                           base::buffer_view iv,
                           base::buffer_view hp) noexcept
  : data_(data)
  , iv_(iv)
  , hp_(hp)
{}

base::buffer_view
crypto::key_view::data() const noexcept
{
  return data_;
}

base::buffer_view
crypto::key_view::iv() const noexcept
{
  return iv_;
}

base::buffer_view
crypto::key_view::hp() const noexcept
{
  return hp_;
}

crypto::key::key(base::buffer data, base::buffer iv, base::buffer hp) noexcept
  : data_(std::move(data))
  , iv_(std::move(iv))
  , hp_(std::move(hp))
{}

base::buffer_view
crypto::key::data() const noexcept
{
  return data_;
}

base::buffer_view
crypto::key::iv() const noexcept
{
  return iv_;
}

base::buffer_view
crypto::key::hp() const noexcept
{
  return hp_;
}

crypto::key::operator key_view() const noexcept
{
  return { data_, iv_, hp_ };
}

crypto::crypto(aead aead, hash hash)
  : aead_(aead)
  , hash_(hash)

{}

// https://quicwg.org/base-drafts/draft-ietf-quic-tls.html#rfc.section.5.2

result<base::buffer>
crypto::initial_secret(base::buffer_view dcid, base::buffer_view salt)
{
  return hkdf_extract(dcid, salt);
}

result<base::buffer>
crypto::update_secret(base::buffer_view secret)
{
  return hkdf_expand_label(secret, "traffic upd", secret.size());
}

result<base::buffer>
crypto::client_initial_secret(base::buffer_view secret)
{
  return hkdf_expand_label(secret, "client in", hash_size(hash_));
}

result<base::buffer>
crypto::server_initial_secret(base::buffer_view secret)
{
  return hkdf_expand_label(secret, "server in", hash_size(hash_));
}

result<crypto::key>
crypto::packet_protection_key(base::buffer_view secret)
{
  return crypto::key{ BNL_TRY(crypto::packet_protection_data(secret)),
                      BNL_TRY(crypto::packet_protection_iv(secret)),
                      BNL_TRY(crypto::packet_protection_hp(secret)) };
}

// https://tools.ietf.org/html/rfc5116#section-5 (authentication tag length)
// https://tools.ietf.org/html/rfc7539#section-2.8 (The output from the AEAD is
// twofold ...)
size_t
crypto::aead_overhead() const noexcept
{
  switch (aead_) {
    case crypto::aead::aes_128_gcm:
    case crypto::aead::aes_256_gcm:
    case crypto::aead::chacha20_poly1305:
      return 16;
  }

  return 0;
}

// https://quicwg.org/base-drafts/draft-ietf-quic-tls.html#rfc.section.5.1

result<base::buffer>
crypto::packet_protection_data(base::buffer_view secret)
{
  return hkdf_expand_label(secret, "quic key", aead_key_size(aead_));
}

result<base::buffer>
crypto::packet_protection_iv(base::buffer_view secret)
{
  // https://tools.ietf.org/html/rfc8446#section-5.3
  size_t iv_size = std::max(static_cast<size_t>(8), aead_nonce_min_size(aead_));

  return hkdf_expand_label(secret, "quic iv", iv_size);
}

result<base::buffer>
crypto::packet_protection_hp(base::buffer_view secret)
{
  return hkdf_expand_label(secret, "quic hp", aead_key_size(aead_));
}

// See https://tools.ietf.org/html/rfc8446#section-7.1 for definition of
// hkfd_expand_label.
result<base::buffer>
crypto::hkdf_expand_label(base::buffer_view secret,
                          base::buffer_view label,
                          size_t size)
{
  std::array<uint8_t, 256> info = {};
  uint8_t *it = std::begin(info);

  // HkdfLabel

  // length (uint16_t)
  *it++ = static_cast<uint8_t>(size / UINT8_MAX); // MSB
  *it++ = static_cast<uint8_t>(size % UINT8_MAX); // LSB

  // "tls 13 " + Label
  static const base::buffer_view LABEL = "tls13 ";
  *it++ = static_cast<uint8_t>(LABEL.size() + label.size()); // Length
  it = std::copy_n(LABEL.data(), LABEL.size(), it);
  it = std::copy_n(label.data(), label.size(), it);

  // Context
  *it++ = 0;

  size_t info_size = static_cast<size_t>(it - std::begin(info));

  return hkdf_expand(secret, base::buffer_view(info.data(), info_size), size);
}

// https://tools.ietf.org/html/rfc5116#section-5
// https://tools.ietf.org/html/rfc7539#section-2.8 (end of section)
size_t
crypto::aead_key_size(aead aead) const noexcept
{
  switch (aead) {
    case crypto::aead::aes_128_gcm:
      return 16;
    case crypto::aead::aes_256_gcm:
    case crypto::aead::chacha20_poly1305:
      return 32;
  }

  assert(false);
  return 0;
}

// https://tools.ietf.org/html/rfc5116#section-5
// https://tools.ietf.org/html/rfc7539#section-2.8 (end of section)
size_t
crypto::aead_nonce_min_size(aead aead) const noexcept
{
  switch (aead) {
    case crypto::aead::aes_128_gcm:
    case crypto::aead::aes_256_gcm:
    case crypto::aead::chacha20_poly1305:
      return 12;
  }

  assert(false);
  return 0;
}

// Size in bytes
size_t
crypto::hash_size(hash hash) const noexcept
{
  switch (hash) {
    case crypto::hash::sha256:
      return 32;
    case crypto::hash::sha384:
      return 48;
  }

  assert(false);
  return 0;
}

std::ostream &
operator<<(std::ostream &os, const crypto &crypto)
{
  os << "AEAD: " << crypto.aead_ << ", ";
  os << "HASH: " << crypto.hash_;
  return os;
}

std::ostream &
operator<<(std::ostream &os, crypto::level level)
{
  switch (level) {
    case crypto::level::initial:
      return os << "initial";
    case crypto::level::early_data:
      return os << "early data";
    case crypto::level::handshake:
      return os << "handshake";
    case crypto::level::application:
      return os << "application";
  }

  assert(false);
  return os;
}

std::ostream &
operator<<(std::ostream &os, crypto::aead aead)
{
  switch (aead) {
    case crypto::aead::aes_128_gcm:
      return os << "AES_128_GCM";
    case crypto::aead::aes_256_gcm:
      return os << "AES_256_GCM";
    case crypto::aead::chacha20_poly1305:
      return os << "CHACHA_POLY1305";
  }

  assert(false);
  return os;
}

std::ostream &
operator<<(std::ostream &os, crypto::hash hash)
{
  switch (hash) {
    case crypto::hash::sha256:
      return os << "SHA256";
    case crypto::hash::sha384:
      return os << "SHA384";
  }

  assert(false);
  return os;
}

}
}
