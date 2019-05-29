#pragma once

#include <h3c/export.hpp>
#include <h3c/http.hpp>

#include <memory>
#include <system_error>

// https://quicwg.org/base-drafts/draft-ietf-quic-qpack.html

// This QPACK implementation only uses the static table to keep the
// implementation and usage simple. Users should advertise
// `SETTINGS_QPACK_MAX_TABLE_CAPACITY` as zero when using this implementation.

namespace h3c {

class logger;

namespace qpack {

namespace prefix {

H3C_EXPORT size_t encoded_size();

H3C_EXPORT std::error_code
encode(uint8_t *dest, size_t size, size_t *encoded_size, const logger *logger);

H3C_EXPORT std::error_code decode(const uint8_t *src,
                                  size_t size,
                                  size_t *encoded_size,
                                  const logger *logger);

}; // namespace prefix

H3C_EXPORT size_t encoded_size(const header &header);

H3C_EXPORT std::error_code encode(uint8_t *dest,
                                  size_t size,
                                  const header &header,
                                  size_t *encoded_size,
                                  const logger *logger);

class decoder {
public:
  H3C_EXPORT std::error_code init(const logger *logger);

  H3C_EXPORT std::error_code decode(const uint8_t *src,
                                    size_t size,
                                    header *header,
                                    size_t *encoded_size,
                                    const logger *logger);

private:
  struct {
    struct {
      std::unique_ptr<char[]> data; // NOLINT
      size_t size = 0;
    } name;

    struct {
      std::unique_ptr<char[]> data; // NOLINT
      size_t size = 0;
    } value;
  } huffman_decoded;

  std::error_code literal_with_name_reference_decode(const uint8_t *src,
                                                     size_t size,
                                                     header *header,
                                                     size_t *encoded_size,
                                                     const logger *logger);

  std::error_code literal_without_name_reference_decode(const uint8_t *src,
                                                        size_t size,
                                                        header *header,
                                                        size_t *encoded_size,
                                                        const logger *logger);
};

} // namespace qpack
} // namespace h3c
