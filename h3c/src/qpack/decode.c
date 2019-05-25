#include <h3c/qpack.h>

#include <h3c/huffman.h>

#include <util/error.h>
#include <util/string.h>

#include <assert.h>
#include <stdlib.h>

#include "decode_generated.c"

H3C_ERROR h3c_qpack_decode_context_init(h3c_qpack_decode_context_t *context,
                                        h3c_log_t *log)
{
  // TODO: Find out what is best for max header and max value size.
  context->huffman_decoded.name.data = malloc(1000);
  context->huffman_decoded.name.size = 1000;
  context->huffman_decoded.value.data = malloc(64000);
  context->huffman_decoded.value.size = 64000;

  if (context->huffman_decoded.name.data == NULL ||
      context->huffman_decoded.value.data == NULL) {
    h3c_qpack_decode_context_destroy(context);
    THROW(H3C_ERROR_OUT_OF_MEMORY);
  }

  return H3C_SUCCESS;
}

void h3c_qpack_decode_context_destroy(h3c_qpack_decode_context_t *context)
{
  free((char *) context->huffman_decoded.name.data);
  free((char *) context->huffman_decoded.value.data);
}

#define QPACK_PREFIX_ENCODED_SIZE 2

H3C_ERROR h3c_qpack_prefix_decode(const uint8_t *src,
                                  size_t size,
                                  size_t *encoded_size,
                                  h3c_log_t *log)
{
  (void) src;
  assert(encoded_size);

  *encoded_size = 0;

  if (size < QPACK_PREFIX_ENCODED_SIZE) {
    THROW(H3C_ERROR_INCOMPLETE);
  }

  *encoded_size = QPACK_PREFIX_ENCODED_SIZE;

  return H3C_SUCCESS;
}

#define TRY_UINT8_DECODE(value)                                                \
  if (size == 0) {                                                             \
    THROW(H3C_ERROR_INCOMPLETE);                                               \
  }                                                                            \
                                                                               \
  (value) = *src;                                                              \
                                                                               \
  src++;                                                                       \
  size--;                                                                      \
  (void) 0

static H3C_ERROR prefix_int_decode(const uint8_t *src,
                                   size_t size,
                                   uint64_t *value,
                                   uint8_t prefix,
                                   size_t *encoded_size,
                                   h3c_log_t *log)
{
  *encoded_size = 0;
  const uint8_t *begin = src;

  uint8_t prefix_max = (uint8_t)((1U << prefix) - 1);

  TRY_UINT8_DECODE(*value);
  *value &= prefix_max;

  if (*value >= prefix_max) {
    uint64_t offset = 0;
    uint8_t byte = 0;
    do {
      TRY_UINT8_DECODE(byte);
      *value += (byte & 127) * (1U << offset);
      offset += 7;
    } while ((byte & 128) == 128);
  }

  *encoded_size = (size_t)(src - begin);

  return H3C_SUCCESS;
}

#define TRY_PREFIX_INT_DECODE(value, type, prefix)                             \
  {                                                                            \
    uint64_t pi = 0;                                                           \
    size_t pi_encoded_size = 0;                                                \
    H3C_ERROR error = prefix_int_decode(src, size, &pi, (prefix),              \
                                        &pi_encoded_size, log);                \
    if (error) {                                                               \
      return error;                                                            \
    }                                                                          \
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
      THROW(H3C_ERROR_INCOMPLETE);                                             \
    }                                                                          \
                                                                               \
    bool is_huffman_encoded = (*src >> (prefix)) & 0x01;                       \
                                                                               \
    /* WARNING: Sizes larger than 32-bit will be truncated on 32-bit           \
     * platforms. */                                                           \
    size_t buffer_encoded_size = 0;                                            \
    TRY_PREFIX_INT_DECODE(buffer_encoded_size, size_t, (prefix));              \
                                                                               \
    if (buffer_encoded_size > size) {                                          \
      THROW(H3C_ERROR_INCOMPLETE);                                             \
    }                                                                          \
                                                                               \
    if (is_huffman_encoded) {                                                  \
      (buffer).size = (huffman_buffer).size;                                   \
      H3C_ERROR error = h3c_huffman_decode(src, buffer_encoded_size,           \
                                           (char *) (huffman_buffer).data,     \
                                           &(buffer).size, log);               \
      if (error) {                                                             \
        return error;                                                          \
      }                                                                        \
                                                                               \
      (buffer).data = (huffman_buffer).data;                                   \
    } else {                                                                   \
      (buffer).data = (const char *) src;                                      \
      (buffer).size = buffer_encoded_size;                                     \
    }                                                                          \
                                                                               \
    src += buffer_encoded_size;                                                \
    size -= buffer_encoded_size;                                               \
  }                                                                            \
  (void) 0

static H3C_ERROR indexed_header_field_decode(const uint8_t *src,
                                             size_t size,
                                             h3c_header_t *header,
                                             size_t *encoded_size,
                                             h3c_log_t *log)
{
  *encoded_size = 0;
  const uint8_t *begin = src;

  // Ensure the 'S' bit is set which indicates the index is in the static table.
  if (!(*src & 0x40)) {
    H3C_LOG_ERROR(log, "'S' bit not set in indexed header field");
    THROW(H3C_ERROR_QPACK_DECOMPRESSION_FAILED);
  }

  uint8_t index = 0;
  TRY_PREFIX_INT_DECODE(index, uint8_t, 6);

  if (!static_table_find_header_value(index, header)) {
    H3C_LOG_ERROR(log, "Indexed header field (%i) not found in static table",
                  index);
    THROW(H3C_ERROR_QPACK_DECOMPRESSION_FAILED);
  }

  *encoded_size = (size_t)(src - begin);

  return H3C_SUCCESS;
}

static H3C_ERROR
literal_with_name_reference_decode(h3c_qpack_decode_context_t *context,
                                   const uint8_t *src,
                                   size_t size,
                                   h3c_header_t *header,
                                   size_t *encoded_size,
                                   h3c_log_t *log)
{
  *encoded_size = 0;
  const uint8_t *begin = src;

  // Ensure the 'S' bit is set which indicates the index is in the static table.
  if (!(*src & 0x10)) {
    H3C_LOG_ERROR(log, "'S' bit not set in literal with name reference");
    THROW(H3C_ERROR_QPACK_DECOMPRESSION_FAILED);
  }

  uint8_t index = 0;
  TRY_PREFIX_INT_DECODE(index, uint8_t, 4);

  if (!static_table_find_header_only(index, header)) {
    H3C_LOG_ERROR(log, "Header name reference (%i) not found in static table",
                  index);
    THROW(H3C_ERROR_QPACK_DECOMPRESSION_FAILED);
  }

  TRY_LITERAL_DECODE(header->value, 7, context->huffman_decoded.value);

  *encoded_size = (size_t)(src - begin);

  return H3C_SUCCESS;
}

static H3C_ERROR
literal_without_name_reference_decode(h3c_qpack_decode_context_t *context,
                                      const uint8_t *src,
                                      size_t size,
                                      h3c_header_t *header,
                                      size_t *encoded_size,
                                      h3c_log_t *log)
{
  *encoded_size = 0;
  const uint8_t *begin = src;

  TRY_LITERAL_DECODE(header->name, 3, context->huffman_decoded.name);

  if (!is_lowercase(header->name.data, header->name.size)) {
    H3C_LOG_ERROR(log, "Header (%.*s) is not lowercase", header->name.size,
                  header->name.data);
    THROW(H3C_ERROR_MALFORMED_HEADER);
  }

  TRY_LITERAL_DECODE(header->value, 7, context->huffman_decoded.value);

  *encoded_size = (size_t)(src - begin);

  return H3C_SUCCESS;
}

#define INDEXED_HEADER_FIELD_PREFIX 0x80
#define LITERAL_WITH_NAME_REFERENCE_PREFIX 0x40
#define LITERAL_WITHOUT_NAME_REFERENCE_PREFIX 0x20

H3C_ERROR h3c_qpack_decode(h3c_qpack_decode_context_t *context,
                           const uint8_t *src,
                           size_t size,
                           h3c_header_t *header,
                           size_t *encoded_size,
                           h3c_log_t *log)
{
  assert(src);
  assert(header);
  assert(encoded_size);

  if (size == 0) {
    THROW(H3C_ERROR_INCOMPLETE);
  }

  if ((*src & INDEXED_HEADER_FIELD_PREFIX) == INDEXED_HEADER_FIELD_PREFIX) {
    return indexed_header_field_decode(src, size, header, encoded_size, log);
  }

  if ((*src & LITERAL_WITH_NAME_REFERENCE_PREFIX) ==
      LITERAL_WITH_NAME_REFERENCE_PREFIX) {
    return literal_with_name_reference_decode(context, src, size, header,
                                              encoded_size, log);
  }

  if ((*src & LITERAL_WITHOUT_NAME_REFERENCE_PREFIX) ==
      LITERAL_WITHOUT_NAME_REFERENCE_PREFIX) {
    return literal_without_name_reference_decode(context, src, size, header,
                                                 encoded_size, log);
  }

  H3C_LOG_ERROR(log, "Unexpected header block instruction prefix (%i)", *src);
  THROW(H3C_ERROR_QPACK_DECOMPRESSION_FAILED);
}
