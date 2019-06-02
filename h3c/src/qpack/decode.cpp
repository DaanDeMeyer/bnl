#include <h3c/qpack.hpp>

#include <h3c/huffman.hpp>

#include <util/error.hpp>
#include <util/string.hpp>

#include <cassert>
#include <cstdlib>

#include "decode_generated.cpp"

namespace h3c {

// TODO: Find out what is best for max header and max value size.
qpack::decoder::decoder(const class logger *logger)
    : logger(logger), huffman_decoded_name_(1000), huffman_decoded_value_(64000)
{}

static constexpr size_t QPACK_PREFIX_ENCODED_SIZE = 2;

std::error_code qpack::decoder::prefix_decode(const uint8_t *src,
                                              size_t size,
                                              size_t *encoded_size) const
    noexcept
{
  (void) src;
  assert(encoded_size);

  *encoded_size = 0;

  if (size < QPACK_PREFIX_ENCODED_SIZE) {
    THROW(error::incomplete);
  }

  *encoded_size = QPACK_PREFIX_ENCODED_SIZE;

  return {};
}

#define TRY_UINT8_DECODE(value)                                                \
  if (size == 0) {                                                             \
    THROW(error::incomplete);                                                  \
  }                                                                            \
                                                                               \
  (value) = *src;                                                              \
                                                                               \
  src++;                                                                       \
  size--;                                                                      \
  (void) 0

static std::error_code prefix_int_decode(const uint8_t *src,
                                         size_t size,
                                         uint64_t *value,
                                         uint8_t prefix,
                                         size_t *encoded_size,
                                         const logger *logger)
{
  *encoded_size = 0;
  const uint8_t *begin = src;

  uint8_t prefix_max = static_cast<uint8_t>((1U << prefix) - 1);

  TRY_UINT8_DECODE(*value);
  *value &= prefix_max;

  if (*value >= prefix_max) {
    uint64_t offset = 0;
    uint8_t byte = 0;
    do {
      TRY_UINT8_DECODE(byte);
      *value += (byte & 127U) * (1U << offset);
      offset += 7;
    } while ((byte & 128U) == 128);
  }

  *encoded_size = static_cast<size_t>(src - begin);

  return {};
}

#define TRY_PREFIX_INT_DECODE(value, type, prefix)                             \
  {                                                                            \
    uint64_t pi = 0;                                                           \
    size_t pi_encoded_size = 0;                                                \
    TRY(prefix_int_decode(src, size, &pi, (prefix), &pi_encoded_size,          \
                          logger));                                            \
                                                                               \
    /* TODO: Introduce max values */                                           \
    (value) = (type) pi;                                                       \
                                                                               \
    src += pi_encoded_size;                                                    \
    size -= pi_encoded_size;                                                   \
  }                                                                            \
  (void) 0

#define TRY_LITERAL_DECODE(buffer, prefix, huffman_buffer)                     \
  {                                                                            \
    if (size == 0) {                                                           \
      THROW(error::incomplete);                                                \
    }                                                                          \
                                                                               \
    bool is_huffman_encoded = static_cast<uint8_t>(*src >> (prefix)) & 0x01u;  \
                                                                               \
    /* WARNING: Sizes larger than 32-bit will be truncated on 32-bit           \
     * platforms. */                                                           \
    size_t buffer_encoded_size = 0;                                            \
    TRY_PREFIX_INT_DECODE(buffer_encoded_size, size_t, (prefix));              \
                                                                               \
    if (buffer_encoded_size > size) {                                          \
      THROW(error::incomplete);                                                \
    }                                                                          \
                                                                               \
    if (is_huffman_encoded) {                                                  \
      (buffer).size = (huffman_buffer).size;                                   \
      TRY(huffman::decode(src, buffer_encoded_size,                            \
                          (huffman_buffer).data.get(), &(buffer).size,         \
                          logger));                                            \
                                                                               \
      (buffer).data = (huffman_buffer).data.get();                             \
    } else {                                                                   \
      (buffer).data = reinterpret_cast<const char *>(src); /* NOLINT */        \
      (buffer).size = buffer_encoded_size;                                     \
    }                                                                          \
                                                                               \
    src += buffer_encoded_size;                                                \
    size -= buffer_encoded_size;                                               \
  }                                                                            \
  (void) 0

std::error_code
qpack::decoder::indexed_header_field_decode(const uint8_t *src,
                                            size_t size,
                                            header *header,
                                            size_t *encoded_size) noexcept
{
  *encoded_size = 0;
  const uint8_t *begin = src;

  // Ensure the 'S' bit is set which indicates the index is in the static table.
  if ((*src & 0x40U) == 0) {
    H3C_LOG_ERROR(logger, "'S' bit not set in indexed header field");
    THROW(error::qpack_decompression_failed);
  }

  uint8_t index = 0;
  TRY_PREFIX_INT_DECODE(index, uint8_t, 6U);

  if (!qpack::static_table::find_header_value(index, header)) {
    H3C_LOG_ERROR(logger, "Indexed header field ({}) not found in static table",
                  index);
    THROW(error::qpack_decompression_failed);
  }

  *encoded_size = static_cast<size_t>(src - begin);

  return {};
}

std::error_code qpack::decoder::literal_with_name_reference_decode(
    const uint8_t *src,
    size_t size,
    header *header,
    size_t *encoded_size) noexcept
{
  *encoded_size = 0;
  const uint8_t *begin = src;

  // Ensure the 'S' bit is set which indicates the index is in the static table.
  if ((*src & 0x10U) == 0) {
    H3C_LOG_ERROR(logger, "'S' bit not set in literal with name reference");
    THROW(error::qpack_decompression_failed);
  }

  uint8_t index = 0;
  TRY_PREFIX_INT_DECODE(index, uint8_t, 4U);

  if (!static_table::find_header_only(index, header)) {
    H3C_LOG_ERROR(logger,
                  "Header name reference ({}) not found in static table",
                  index);
    THROW(error::qpack_decompression_failed);
  }

  TRY_LITERAL_DECODE(header->value, 7U, huffman_decoded_name_);

  *encoded_size = static_cast<size_t>(src - begin);

  return {};
}

std::error_code qpack::decoder::literal_without_name_reference_decode(
    const uint8_t *src,
    size_t size,
    header *header,
    size_t *encoded_size) noexcept
{
  *encoded_size = 0;
  const uint8_t *begin = src;

  TRY_LITERAL_DECODE(header->name, 3U, huffman_decoded_name_);

  if (!util::is_lowercase(header->name.data, header->name.size)) {
    H3C_LOG_ERROR(logger, "Header ({}) is not lowercase",
                  fmt::string_view(header->name.data, header->name.size));
    THROW(error::malformed_header);
  }

  TRY_LITERAL_DECODE(header->value, 7U, huffman_decoded_value_);

  *encoded_size = static_cast<size_t>(src - begin);

  return {};
}

static constexpr uint8_t INDEXED_HEADER_FIELD_PREFIX = 0x80;
static constexpr uint8_t LITERAL_WITH_NAME_REFERENCE_PREFIX = 0x40;
static constexpr uint8_t LITERAL_WITHOUT_NAME_REFERENCE_PREFIX = 0x20;

std::error_code qpack::decoder::decode(const uint8_t *src,
                                       size_t size,
                                       header *header,
                                       size_t *encoded_size) noexcept
{
  assert(src);
  assert(header);
  assert(encoded_size);

  if (size == 0) {
    THROW(error::incomplete);
  }

  if ((*src & INDEXED_HEADER_FIELD_PREFIX) == INDEXED_HEADER_FIELD_PREFIX) {
    return indexed_header_field_decode(src, size, header, encoded_size);
  }

  if ((*src & LITERAL_WITH_NAME_REFERENCE_PREFIX) ==
      LITERAL_WITH_NAME_REFERENCE_PREFIX) {
    return literal_with_name_reference_decode(src, size, header, encoded_size);
  }

  if ((*src & LITERAL_WITHOUT_NAME_REFERENCE_PREFIX) ==
      LITERAL_WITHOUT_NAME_REFERENCE_PREFIX) {
    return literal_without_name_reference_decode(src, size, header,
                                                 encoded_size);
  }

  H3C_LOG_ERROR(logger, "Unexpected header block instruction prefix ({})",
                *src);
  THROW(error::qpack_decompression_failed);
}

} // namespace h3c
