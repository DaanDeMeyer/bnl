#pragma once

#include <h3c/export.hpp>
#include <h3c/http.hpp>
#include <h3c/huffman.hpp>

#include <memory>
#include <system_error>

// https://quicwg.org/base-drafts/draft-ietf-quic-qpack.html

// This QPACK implementation only uses the static table to keep the
// implementation and usage simple. Users should advertise
// `SETTINGS_QPACK_MAX_TABLE_CAPACITY` as zero when using this implementation.

namespace h3c {

class logger;

namespace qpack {

class encoder {
public:
  H3C_EXPORT explicit encoder(const logger *logger) noexcept;

  encoder(const encoder &) = delete;
  encoder &operator=(const encoder &) = delete;

  encoder(encoder &&) = default;
  encoder &operator=(encoder &&) = default;

  ~encoder() = default;

  H3C_EXPORT size_t prefix_encoded_size() const noexcept;

  H3C_EXPORT std::error_code
  prefix_encode(uint8_t *dest, size_t size, size_t *encoded_size) const
      noexcept;

  H3C_EXPORT size_t encoded_size(const header &header) const noexcept;

  H3C_EXPORT std::error_code encode(uint8_t *dest,
                                    size_t size,
                                    const header &header,
                                    size_t *encoded_size) const noexcept;

private:
  const logger *logger;

  huffman::encoder huffman_;

  size_t literal_encoded_size(const char *data, size_t size) const noexcept;
};

class decoder {
public:
  H3C_EXPORT explicit decoder(const logger *logger);

  decoder(const decoder &) = delete;
  decoder &operator=(const decoder &) = delete;

  decoder(decoder &&) = default;
  decoder &operator=(decoder &&) = default;

  ~decoder() = default;

  H3C_EXPORT std::error_code
  prefix_decode(const uint8_t *src, size_t size, size_t *encoded_size) const
      noexcept;

  H3C_EXPORT std::error_code decode(const uint8_t *src,
                                    size_t size,
                                    header *header,
                                    size_t *encoded_size) noexcept;

private:
  const logger *logger;

  huffman::decoder huffman_;

  struct decoded {
    explicit decoded(size_t size)
        : data(std::unique_ptr<char[]>(new char[size])), size(size)
    {}

    std::unique_ptr<char[]> data;
    size_t size;
  };

  decoded huffman_decoded_name_;
  decoded huffman_decoded_value_;
};

} // namespace qpack
} // namespace h3c
