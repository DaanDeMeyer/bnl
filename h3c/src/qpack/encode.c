#include <h3c/qpack.h>

#include <h3c/huffman.h>

#include <util.h>

#include <assert.h>
#include <string.h>

#include "encode_generated.c"

H3C_ERROR h3c_qpack_encode_prefix(uint8_t *dest,
                                  size_t size,
                                  size_t *encoded_size,
                                  h3c_log_t *log)
{
  assert(encoded_size);

  *encoded_size = 0;

  if (size < 2) {
    H3C_ERROR(H3C_ERROR_BUFFER_TOO_SMALL);
  }

  if (dest) {
    memset(dest, 0, 2);
  }

  *encoded_size = 2;

  return H3C_SUCCESS;
}

#define TRY_UINT8_ENCODE(value)                                                \
  if (dest) {                                                                  \
    if (size == 0) {                                                           \
      H3C_ERROR(H3C_ERROR_BUFFER_TOO_SMALL);                                   \
    }                                                                          \
                                                                               \
    *dest = (value);                                                           \
                                                                               \
    dest++;                                                                    \
    size--;                                                                    \
  }                                                                            \
                                                                               \
  (*encoded_size)++;                                                           \
  (void) 0

static H3C_ERROR prefix_int_encode(uint8_t *dest,
                                   size_t size,
                                   uint64_t value,
                                   uint8_t prefix,
                                   size_t *encoded_size,
                                   h3c_log_t *log)
{
  *encoded_size = 0;

  uint8_t prefix_max = (uint8_t)((1U << prefix) - 1);

  if (value < prefix_max) {
    TRY_UINT8_ENCODE((uint8_t)(*dest & ~prefix_max) | (uint8_t) value);
    return H3C_SUCCESS;
  }

  TRY_UINT8_ENCODE(*dest | prefix_max);
  value -= prefix_max;

  while (value >= 128) {
    TRY_UINT8_ENCODE((uint8_t)((value % 128) + 128));
    value /= 128;
  }

  TRY_UINT8_ENCODE((uint8_t) value);

  return H3C_SUCCESS;
}

#define TRY_PREFIX_INT_ENCODE(initial, value, prefix)                          \
  {                                                                            \
    if (dest) {                                                                \
      if (size == 0) {                                                         \
        H3C_ERROR(H3C_ERROR_BUFFER_TOO_SMALL);                                 \
      }                                                                        \
                                                                               \
      *dest = (initial);                                                       \
    }                                                                          \
                                                                               \
    size_t pi_encoded_size = 0;                                                \
    H3C_ERROR error = prefix_int_encode(dest, size, (value), (prefix),         \
                                        &pi_encoded_size, log);                \
    if (error) {                                                               \
      return error;                                                            \
    }                                                                          \
                                                                               \
    if (dest) {                                                                \
      dest += pi_encoded_size;                                                 \
      size -= pi_encoded_size;                                                 \
    }                                                                          \
                                                                               \
    *encoded_size += pi_encoded_size;                                          \
  }                                                                            \
  (void) 0

#define TRY_LITERAL_ENCODE(initial, buffer, prefix)                            \
  {                                                                            \
    size_t literal_encoded_size = 0;                                           \
    size_t huffman_encoded_size = h3c_huffman_encoded_size((buffer).data,      \
                                                           (buffer).length);   \
                                                                               \
    if (huffman_encoded_size < (buffer).length) {                              \
      literal_encoded_size = huffman_encoded_size;                             \
      TRY_PREFIX_INT_ENCODE((initial) | (uint8_t)(1 << (prefix)),              \
                            literal_encoded_size, (prefix));                   \
                                                                               \
      if (dest) {                                                              \
        H3C_ERROR error = h3c_huffman_encode(dest, size, (buffer).data,        \
                                             (buffer).length, log);            \
        if (error) {                                                           \
          return error;                                                        \
        }                                                                      \
      }                                                                        \
    } else {                                                                   \
      literal_encoded_size = (buffer).length;                                  \
      TRY_PREFIX_INT_ENCODE(initial, literal_encoded_size, (prefix));          \
                                                                               \
      if (dest) {                                                              \
        if (literal_encoded_size > size) {                                     \
          H3C_ERROR(H3C_ERROR_BUFFER_TOO_SMALL);                               \
        }                                                                      \
                                                                               \
        memcpy(dest, (buffer).data, (buffer).length);                          \
      }                                                                        \
    }                                                                          \
                                                                               \
    if (dest) {                                                                \
      dest += literal_encoded_size;                                            \
      size -= literal_encoded_size;                                            \
    }                                                                          \
                                                                               \
    *encoded_size += literal_encoded_size;                                     \
  }                                                                            \
  (void) 0

#define INDEXED_HEADER_FIELD_PREFIX 0xc0
#define LITERAL_WITH_NAME_REFERENCE_PREFIX 0x50
#define LITERAL_WITHOUT_NAME_REFERENCE_PREFIX 0x20
#define LITERAL_NO_PREFIX 0x00

static H3C_ERROR indexed_header_field_encode(uint8_t *dest,
                                             size_t size,
                                             uint8_t index,
                                             size_t *encoded_size,
                                             h3c_log_t *log)
{
  TRY_PREFIX_INT_ENCODE(INDEXED_HEADER_FIELD_PREFIX, index, 6);

  return H3C_SUCCESS;
}

static H3C_ERROR literal_with_name_reference_encode(uint8_t *dest,
                                                    size_t size,
                                                    uint8_t index,
                                                    const h3c_header_t *header,
                                                    size_t *encoded_size,
                                                    h3c_log_t *log)
{
  TRY_PREFIX_INT_ENCODE(LITERAL_WITH_NAME_REFERENCE_PREFIX, index, 4);
  TRY_LITERAL_ENCODE(LITERAL_NO_PREFIX, header->value, 7);

  return H3C_SUCCESS;
}

static H3C_ERROR
literal_without_name_reference_encode(uint8_t *dest,
                                      size_t size,
                                      const h3c_header_t *header,
                                      size_t *encoded_size,
                                      h3c_log_t *log)
{
  TRY_LITERAL_ENCODE(LITERAL_WITHOUT_NAME_REFERENCE_PREFIX, header->name, 3);
  TRY_LITERAL_ENCODE(LITERAL_NO_PREFIX, header->value, 7);

  return H3C_SUCCESS;
}

H3C_ERROR h3c_qpack_encode(uint8_t *dest,
                           size_t size,
                           const h3c_header_t *header,
                           size_t *encoded_size,
                           h3c_log_t *log)
{
  assert(header);
  assert(encoded_size);

  if (!is_lowercase(header->name.data, header->name.length)) {
    H3C_LOG_ERROR("Header (%.*s) is not lowercase", header->name.length,
                  header->name.data);
    H3C_ERROR(H3C_ERROR_MALFORMED_HEADER);
  }

  *encoded_size = 0;

  uint8_t index = 0;
  STATIC_TABLE_INDEX_TYPE result = static_table_find_index(header, &index);

  switch (result) {
    case STATIC_TABLE_INDEX_HEADER_VALUE:
      return indexed_header_field_encode(dest, size, index, encoded_size, log);
    case STATIC_TABLE_INDEX_HEADER_ONLY:
      return literal_with_name_reference_encode(dest, size, index, header,
                                                encoded_size, log);
    case STATIC_TABLE_INDEX_MISSING:
      return literal_without_name_reference_encode(dest, size, header,
                                                   encoded_size, log);
  }

  H3C_ERROR(H3C_ERROR_INTERNAL);
}
