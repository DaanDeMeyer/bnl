#include <h3c/qpack.hpp>

#include <h3c/huffman.hpp>

#include <util/error.hpp>
#include <util/string.hpp>

#include <cassert>
#include <cstring>

#include "encode_generated.cpp"

static constexpr size_t QPACK_PREFIX_ENCODED_SIZE = 2;

namespace h3c {

qpack::encoder::encoder(logger *logger) noexcept
    : logger_(logger), huffman_(logger)
{}

size_t qpack::encoder::prefix_encoded_size() const noexcept
{
  return QPACK_PREFIX_ENCODED_SIZE;
}

std::error_code
qpack::encoder::prefix_encode(uint8_t *dest,
                              size_t size,
                              size_t *encoded_size) const noexcept
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

static size_t prefix_int_encoded_size(uint64_t value, uint8_t prefix) noexcept
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
                                         const logger *logger_) noexcept
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
    TRY(prefix_int_encode(dest, size, (value), (prefix), &pi_encoded_size,     \
                          logger_));                                           \
                                                                               \
    dest += pi_encoded_size;                                                   \
    size -= pi_encoded_size;                                                   \
  }                                                                            \
  (void) 0

size_t qpack::encoder::literal_encoded_size(const char *data, size_t size) const
    noexcept
{
  size_t huffman_encoded_size = huffman_.encoded_size(data, size);
  return huffman_encoded_size < size ? huffman_encoded_size : size;
}

#define TRY_LITERAL_ENCODE(initial, buffer, prefix)                            \
  {                                                                            \
    size_t literal_encoded_size = 0;                                           \
    size_t huffman_encoded_size = huffman_.encoded_size((buffer).data,         \
                                                        (buffer).size);        \
                                                                               \
    if (huffman_encoded_size < (buffer).size) {                                \
      literal_encoded_size = huffman_encoded_size;                             \
      TRY_PREFIX_INT_ENCODE((initial) | static_cast<uint8_t>(1U << (prefix)),  \
                            literal_encoded_size, (prefix));                   \
                                                                               \
      TRY(huffman_.encode(dest, size, (buffer).data, (buffer).size));          \
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

size_t qpack::encoder::encoded_size(const header &header) const noexcept
{
  assert(header);

  uint8_t index = 0;
  static_table::index_type result = static_table::find_index(header, &index);

  size_t encoded_size = 0;

  switch (result) {

    case static_table::index_type::header_value: {
      encoded_size += prefix_int_encoded_size(index, 6);
      break;
    }

    case static_table::index_type::header_only: {
      encoded_size += prefix_int_encoded_size(index, 4);

      size_t value_encoded_size = literal_encoded_size(header.value.data,
                                                       header.value.size);
      encoded_size += prefix_int_encoded_size(value_encoded_size, 7);
      encoded_size += value_encoded_size;
      break;
    }

    case static_table::index_type::missing: {
      size_t name_encoded_size = literal_encoded_size(header.name.data,
                                                      header.name.size);
      encoded_size += prefix_int_encoded_size(name_encoded_size, 3);
      encoded_size += name_encoded_size;

      size_t value_encoded_size = literal_encoded_size(header.value.data,
                                                       header.value.size);
      encoded_size += prefix_int_encoded_size(value_encoded_size, 7);
      encoded_size += value_encoded_size;
      break;
    }

    default:
      assert(false);
  }

  return encoded_size;
}

static constexpr uint8_t INDEXED_HEADER_FIELD_PREFIX = 0xc0;
static constexpr uint8_t LITERAL_WITH_NAME_REFERENCE_PREFIX = 0x50;
static constexpr uint8_t LITERAL_WITHOUT_NAME_REFERENCE_PREFIX = 0x20;
static constexpr uint8_t LITERAL_NO_PREFIX = 0x00;

std::error_code qpack::encoder::encode(uint8_t *dest,
                                       size_t size,
                                       const header &header,
                                       size_t *encoded_size) const noexcept
{
  assert(dest);
  assert(header);
  assert(encoded_size);

  if (!util::is_lowercase(header.name.data, header.name.size)) {
    H3C_LOG_ERROR(logger_, "Header ({}) is not lowercase",
                  fmt::string_view(header.name.data, header.name.size));
    THROW(error::malformed_header);
  }

  uint8_t index = 0;
  qpack::static_table::index_type result =
      qpack::static_table::find_index(header, &index);

  *encoded_size = 0;
  uint8_t *begin = dest;

  switch (result) {

    case qpack::static_table::index_type::header_value:
      TRY_PREFIX_INT_ENCODE(INDEXED_HEADER_FIELD_PREFIX, index, 6);
      break;

    case qpack::static_table::index_type::header_only:
      TRY_PREFIX_INT_ENCODE(LITERAL_WITH_NAME_REFERENCE_PREFIX, index, 4U);
      TRY_LITERAL_ENCODE(LITERAL_NO_PREFIX, header.value, 7U);
      break;

    case qpack::static_table::index_type::missing:
      TRY_LITERAL_ENCODE(LITERAL_WITHOUT_NAME_REFERENCE_PREFIX, header.name,
                         3U);
      TRY_LITERAL_ENCODE(LITERAL_NO_PREFIX, header.value, 7U);
      break;

    default:
      THROW(error::internal_error);
  }

  *encoded_size = static_cast<size_t>(dest - begin);

  return {};
}

} // namespace h3c
