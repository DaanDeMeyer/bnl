#include <h3c/varint.h>

#include <assert.h>

// All parse functions convert from network to host byte order and remove the
// varint header (first two bits) before returning a value.

static uint8_t varint_uint8_parse(const uint8_t *src)
{
  assert(src);

  return src[0] & 0x3f;
}

static uint16_t varint_uint16_parse(const uint8_t *src)
{
  assert(src);

  uint16_t result = (uint16_t)((uint16_t) src[0] << 8 | (uint16_t) src[1] << 0);
  return result & 0x3fff;
}

static uint32_t varint_uint32_parse(const uint8_t *src)
{
  assert(src);

  // clang-format off
  uint32_t result = (uint32_t) src[0] << 24 | (uint32_t) src[1] << 16 |
                    (uint32_t) src[2] << 8  | (uint32_t) src[3] << 0;
  // clang-format on
  return result & 0x3fffffff;
}

static uint64_t varint_uint64_parse(const uint8_t *src)
{
  assert(src);

  // clang-format off
  uint64_t result = (uint64_t) src[0] << 56 | (uint64_t) src[1] << 48 |
                    (uint64_t) src[2] << 40 | (uint64_t) src[3] << 32 |
                    (uint64_t) src[4] << 24 | (uint64_t) src[5] << 16 |
                    (uint64_t) src[6] << 8  | (uint64_t) src[7] << 0;
  // clang-format on
  return result & 0x3fffffffffffffff;
}

H3C_ERROR h3c_varint_parse(const uint8_t *src,
                           size_t size,
                           uint64_t *varint,
                           size_t *varint_size)
{
  assert(src);
  assert(varint);
  assert(varint_size);

  *varint_size = 0;

  if (size == 0) {
    return H3C_ERROR_INCOMPLETE;
  }

  *varint_size = 1;
  uint8_t header = *src >> 6;

  // varint size = 2^header
  *varint_size <<= header; // shift left => x2

  if (*varint_size > size) {
    return H3C_ERROR_INCOMPLETE;
  }

  switch (*varint_size) {
    case H3C_VARINT_UINT8_SIZE:
      *varint = varint_uint8_parse(src);
      break;
    case H3C_VARINT_UINT16_SIZE:
      *varint = varint_uint16_parse(src);
      break;
    case H3C_VARINT_UINT32_SIZE:
      *varint = varint_uint32_parse(src);
      break;
    case H3C_VARINT_UINT64_SIZE:
      *varint = varint_uint64_parse(src);
      break;
    default:
      assert(0);
      return 0;
  }

  return H3C_SUCCESS;
}
