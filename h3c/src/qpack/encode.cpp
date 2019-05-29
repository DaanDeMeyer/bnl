#include <h3c/qpack.hpp>

#include <h3c/huffman.hpp>

#include <util/error.hpp>
#include <util/string.hpp>

#include <cassert>
#include <cstring>

#include "encode_generated.cpp"

static constexpr size_t QPACK_PREFIX_ENCODED_SIZE = 2;

namespace h3c {

size_t qpack::prefix::encoded_size()
{
  return QPACK_PREFIX_ENCODED_SIZE;
}

std::error_code qpack::prefix::encode(uint8_t *dest,
                                      size_t size,
                                      size_t *encoded_size,
                                      const logger *logger)
{
  assert(dest);
  assert(encoded_size);

  *encoded_size = 0;

  if (size < QPACK_PREFIX_ENCODED_SIZE) {
    THROW(error::buffer_too_small);
  }

  memset(dest, 0, QPACK_PREFIX_ENCODED_SIZE);

  *encoded_size = QPACK_PREFIX_ENCODED_SIZE;

  return {};
}

static size_t prefix_int_encoded_size(uint64_t value, uint8_t prefix)
{
  uint8_t prefix_max = static_cast<uint8_t>((1U << prefix) - 1);

  size_t encoded_size = 0;

  if (value < prefix_max) {
    encoded_size++;
  } else {
    encoded_size++;
    value -= prefix_max;

    while (value >= 128) {
      value /= 128;
      encoded_size++;
    }

    encoded_size++;
  }

  return encoded_size;
}

#define TRY_UINT8_ENCODE(value)                                                \
  if (size == 0) {                                                             \
    THROW(error::buffer_too_small);                                            \
  }                                                                            \
                                                                               \
  *dest = (value);                                                             \
                                                                               \
  dest++;                                                                      \
  size--;                                                                      \
  (void) 0

static std::error_code prefix_int_encode(uint8_t *dest,
                                         size_t size,
                                         uint64_t value,
                                         uint8_t prefix,
                                         size_t *encoded_size,
                                         const logger *logger)
{
  *encoded_size = 0;
  uint8_t *begin = dest;

  uint8_t prefix_max = static_cast<uint8_t>((1U << prefix) - 1);

  if (value < prefix_max) {
    TRY_UINT8_ENCODE(
        static_cast<uint8_t>(*dest & static_cast<uint8_t>(~prefix_max)) |
        static_cast<uint8_t>(value));
  } else {
    TRY_UINT8_ENCODE(*dest | prefix_max);
    value -= prefix_max;

    while (value >= 128) {
      TRY_UINT8_ENCODE((uint8_t)((value % 128) + 128));
      value /= 128;
    }

    TRY_UINT8_ENCODE((uint8_t) value);
  }

  *encoded_size = static_cast<size_t>(dest - begin);

  return {};
}

#define TRY_PREFIX_INT_ENCODE(initial, value, prefix)                          \
  {                                                                            \
    if (size == 0) {                                                           \
      THROW(error::buffer_too_small);                                          \
    }                                                                          \
                                                                               \
    *dest = (initial);                                                         \
                                                                               \
    size_t pi_encoded_size = 0;                                                \
    std::error_code error = prefix_int_encode(dest, size, (value), (prefix),   \
                                              &pi_encoded_size, logger);       \
    if (error) {                                                               \
      return error;                                                            \
    }                                                                          \
                                                                               \
    dest += pi_encoded_size;                                                   \
    size -= pi_encoded_size;                                                   \
  }                                                                            \
  (void) 0

static size_t literal_encoded_size(const char *data, size_t size)
{
  size_t huffman_encoded_size = huffman::encoded_size(data, size);
  return huffman_encoded_size < size ? huffman_encoded_size : size;
}

#define TRY_LITERAL_ENCODE(initial, buffer, prefix)                            \
  {                                                                            \
    size_t literal_encoded_size = 0;                                           \
    size_t huffman_encoded_size = huffman::encoded_size((buffer).data,         \
                                                        (buffer).size);        \
                                                                               \
    if (huffman_encoded_size < (buffer).size) {                                \
      literal_encoded_size = huffman_encoded_size;                             \
      TRY_PREFIX_INT_ENCODE((initial) | static_cast<uint8_t>(1U << (prefix)),  \
                            literal_encoded_size, (prefix));                   \
                                                                               \
      std::error_code error = huffman::encode(dest, size, (buffer).data,       \
                                              (buffer).size, logger);          \
      if (error) {                                                             \
        return error;                                                          \
      }                                                                        \
    } else {                                                                   \
      literal_encoded_size = (buffer).size;                                    \
      TRY_PREFIX_INT_ENCODE(initial, literal_encoded_size, (prefix));          \
                                                                               \
      if (literal_encoded_size > size) {                                       \
        THROW(error::buffer_too_small);                                        \
      }                                                                        \
                                                                               \
      memcpy(dest, (buffer).data, (buffer).size);                              \
    }                                                                          \
                                                                               \
    dest += literal_encoded_size;                                              \
    size -= literal_encoded_size;                                              \
  }                                                                            \
  (void) 0

static constexpr uint8_t INDEXED_HEADER_FIELD_PREFIX = 0xc0;
static constexpr uint8_t LITERAL_WITH_NAME_REFERENCE_PREFIX = 0x50;
static constexpr uint8_t LITERAL_WITHOUT_NAME_REFERENCE_PREFIX = 0x20;
static constexpr uint8_t LITERAL_NO_PREFIX = 0x00;

static size_t indexed_header_field_encoded_size(uint8_t index)
{
  return prefix_int_encoded_size(index, 6);
}

static std::error_code indexed_header_field_encode(uint8_t *dest,
                                                   size_t size,
                                                   uint8_t index,
                                                   size_t *encoded_size,
                                                   const logger *logger)
{
  *encoded_size = 0;
  uint8_t *begin = dest;

  TRY_PREFIX_INT_ENCODE(INDEXED_HEADER_FIELD_PREFIX, index, 6);

  *encoded_size = static_cast<size_t>(dest - begin);

  return {};
}

static size_t literal_with_name_reference_encoded_size(uint8_t index,
                                                       const header &header)
{
  size_t encoded_size = 0;

  encoded_size += prefix_int_encoded_size(index, 4);

  size_t value_encoded_size = literal_encoded_size(header.value.data,
                                                   header.value.size);
  encoded_size += prefix_int_encoded_size(value_encoded_size, 7);
  encoded_size += value_encoded_size;

  return encoded_size;
}

static std::error_code literal_with_name_reference_encode(uint8_t *dest,
                                                          size_t size,
                                                          uint8_t index,
                                                          const header &header,
                                                          size_t *encoded_size,
                                                          const logger *logger)
{
  *encoded_size = 0;
  uint8_t *begin = dest;

  TRY_PREFIX_INT_ENCODE(LITERAL_WITH_NAME_REFERENCE_PREFIX, index, 4U);
  TRY_LITERAL_ENCODE(LITERAL_NO_PREFIX, header.value, 7U);

  *encoded_size = static_cast<size_t>(dest - begin);

  return {};
}

static size_t literal_without_name_reference_encoded_size(const header &header)
{
  size_t encoded_size = 0;

  size_t name_encoded_size = literal_encoded_size(header.name.data,
                                                  header.name.size);
  encoded_size += prefix_int_encoded_size(name_encoded_size, 3);
  encoded_size += name_encoded_size;

  size_t value_encoded_size = literal_encoded_size(header.value.data,
                                                   header.value.size);
  encoded_size += prefix_int_encoded_size(value_encoded_size, 7);
  encoded_size += value_encoded_size;

  return encoded_size;
}

static std::error_code
literal_without_name_reference_encode(uint8_t *dest,
                                      size_t size,
                                      const header &header,
                                      size_t *encoded_size,
                                      const logger *logger)
{
  *encoded_size = 0;
  uint8_t *begin = dest;

  TRY_LITERAL_ENCODE(LITERAL_WITHOUT_NAME_REFERENCE_PREFIX, header.name, 3U);
  TRY_LITERAL_ENCODE(LITERAL_NO_PREFIX, header.value, 7U);

  *encoded_size = static_cast<size_t>(dest - begin);

  return {};
}

size_t qpack::encoded_size(const header &header)
{
  assert(header);

  uint8_t index = 0;
  static_table::index_type result = static_table::find_index(header, &index);

  switch (result) {
    case static_table::index_type::header_value:
      return indexed_header_field_encoded_size(index);
    case static_table::index_type::header_only:
      return literal_with_name_reference_encoded_size(index, header);
    case static_table::index_type::missing:
      return literal_without_name_reference_encoded_size(header);
  }

  assert(0);
  return 0;
}

std::error_code qpack::encode(uint8_t *dest,
                              size_t size,
                              const header &header,
                              size_t *encoded_size,
                              const logger *logger)
{
  assert(dest);
  assert(header);
  assert(encoded_size);

  if (!util::is_lowercase(header.name.data, header.name.size)) {
    H3C_LOG_ERROR(logger, "Header ({}) is not lowercase",
                  fmt::string_view(header.name.data, header.name.size));
    THROW(error::malformed_header);
  }

  uint8_t index = 0;
  qpack::static_table::index_type result =
      qpack::static_table::find_index(header, &index);

  switch (result) {
    case qpack::static_table::index_type::header_value:
      return indexed_header_field_encode(dest, size, index, encoded_size,
                                         logger);
    case qpack::static_table::index_type::header_only:
      return literal_with_name_reference_encode(dest, size, index, header,
                                                encoded_size, logger);
    case qpack::static_table::index_type::missing:
      return literal_without_name_reference_encode(dest, size, header,
                                                   encoded_size, logger);
  }

  THROW(error::internal_error);
}

} // namespace h3c
