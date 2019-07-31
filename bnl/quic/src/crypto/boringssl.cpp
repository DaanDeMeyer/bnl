#include <bnl/quic/crypto.hpp>

#include <openssl/aead.h>
#include <openssl/aes.h>
#include <openssl/chacha.h>
#include <openssl/evp.h>
#include <openssl/hkdf.h>

namespace bnl {
namespace quic {

static const EVP_AEAD *
evp_aead(crypto::aead aead)
{
  switch (aead) {
    case crypto::aead::aes_128_gcm:
      return EVP_aead_aes_128_gcm();
    case crypto::aead::aes_256_gcm:
      return EVP_aead_aes_256_gcm();
    case crypto::aead::chacha20_poly1305:
      return EVP_aead_chacha20_poly1305();
  }

  assert(false);
  return nullptr;
}

static const EVP_MD *
evp_md(crypto::hash hash)
{
  switch (hash) {
    case crypto::hash::sha256:
      return EVP_sha256();
    case crypto::hash::sha384:
      return EVP_sha384();
  }

  assert(false);
  return nullptr;
}

result<base::buffer>
crypto::hkdf_extract(base::buffer_view secret, base::buffer_view salt)
{
  base::buffer prk(hash_size(hash_)); // Pseudo Random Key

  size_t prk_size = prk.size();

  int rv = HKDF_extract(prk.data(),
                        &prk_size,
                        evp_md(hash_),
                        secret.data(),
                        secret.size(),
                        salt.data(),
                        salt.size());
  if (rv == 0) {
    return error::crypto;
  }

  assert(prk.size() == prk_size);

  return prk;
}

result<base::buffer>
crypto::hkdf_expand(base::buffer_view prk, base::buffer_view info, size_t size)
{
  base::buffer okm(size); // Output Keying Material

  int rv = HKDF_expand(okm.data(),
                       okm.size(),
                       evp_md(hash_),
                       prk.data(),
                       prk.size(),
                       info.data(),
                       info.size());
  if (rv == 0) {
    return error::crypto;
  }

  return okm;
}

result<void>
crypto::encrypt(base::buffer_view_mut dest,
                base::buffer_view plaintext,
                base::buffer_view key,
                base::buffer_view nonce,
                base::buffer_view ad)
{
  auto context = EVP_AEAD_CTX_new(
    evp_aead(aead_), key.data(), key.size(), EVP_AEAD_DEFAULT_TAG_LENGTH);
  if (context == nullptr) {
    return error::crypto;
  }

  size_t max_size = plaintext.size() + EVP_AEAD_max_overhead(evp_aead(aead_));
  size_t out_size = dest.size();

  int rv = EVP_AEAD_CTX_seal(context,
                             dest.data(),
                             &out_size,
                             max_size,
                             nonce.data(),
                             nonce.size(),
                             plaintext.data(),
                             plaintext.size(),
                             ad.data(),
                             ad.size());

  EVP_AEAD_CTX_free(context);

  if (rv == 0) {
    return error::crypto;
  }

  assert(out_size == max_size);

  return base::success();
}

result<void>
crypto::decrypt(base::buffer_view_mut dest,
                base::buffer_view ciphertext,
                base::buffer_view key,
                base::buffer_view nonce,
                base::buffer_view ad)
{
  auto context = EVP_AEAD_CTX_new(
    evp_aead(aead_), key.data(), key.size(), EVP_AEAD_DEFAULT_TAG_LENGTH);
  if (context == nullptr) {
    return error::crypto;
  }

  size_t max_size = dest.size();
  size_t out_size = dest.size();

  int rv = EVP_AEAD_CTX_open(context,
                             dest.data(),
                             &out_size,
                             max_size,
                             nonce.data(),
                             nonce.size(),
                             ciphertext.data(),
                             ciphertext.size(),
                             ad.data(),
                             ad.size());

  EVP_AEAD_CTX_free(context);

  if (rv == 0) {
    return error::crypto;
  }

  return base::success();
}

static uint32_t
uint32_LE(const uint8_t *data)
{
  return static_cast<uint32_t>(data[0] << 0U) |
         static_cast<uint32_t>(data[1] << 8U) |
         static_cast<uint32_t>(data[2] << 16U) |
         static_cast<uint32_t>(data[3] << 24U);
}

// https://quicwg.org/base-drafts/draft-ietf-quic-tls.html#rfc.section.5.4
result<void>
crypto::hp_mask(base::buffer_view_mut dest,
                base::buffer_view key,
                base::buffer_view sample)
{
  assert(sample.size() == 16);

  switch (aead_) {
    // https://quicwg.org/base-drafts/draft-ietf-quic-tls.html#hp-aes
    case crypto::aead::aes_128_gcm:
    case crypto::aead::aes_256_gcm: {
      AES_KEY aes_key = {};
      int rv = AES_set_encrypt_key(
        key.data(), static_cast<unsigned int>(key.size() * 8), &aes_key);
      if (rv < 0) {
        return error::crypto;
      }

      AES_ecb_encrypt(sample.data(), dest.data(), &aes_key, AES_ENCRYPT);

      break;
    }
    // https://quicwg.org/base-drafts/draft-ietf-quic-tls.html#hp-chacha
    case crypto::aead::chacha20_poly1305: {
      uint32_t counter = uint32_LE(sample.data());

      uint32_t nonce[3];
      nonce[0] = uint32_LE(sample.data() + 4);
      nonce[1] = uint32_LE(sample.data() + 8);
      nonce[2] = uint32_LE(sample.data() + 12);

      CRYPTO_chacha_20(dest.data(),
                       sample.data(),
                       sample.size(),
                       key.data(),
                       reinterpret_cast<uint8_t *>(nonce),
                       counter);

      break;
    }
  }

  return base::success();
}
}
}
