#include <h3c/varint.h>

#include <util.h>

#include <assert.h>

static size_t varint_size_(uint64_t varint)
{
  if (varint < 0x40) {
    return 1;
  }

  if (varint < (0x40 << 8)) {
    return 2;
  }

  if (varint < (0x40 << 24)) {
    return 4;
  }

  if (varint < (0x40ULL << 56)) {
    return 8;
  }

  return 0;
}

// All serialize functions convert from host to network byte order (big-endian)
// and insert the varint header.

#define VARINT_UINT8_HEADER 0x00
#define VARINT_UINT16_HEADER 0x40
#define VARINT_UINT32_HEADER 0x80
#define VARINT_UINT64_HEADER 0xc0

static void varint_uint8_serialize(uint8_t *dest, uint8_t number)
{
  assert(dest);

  dest[0] = (uint8_t)(number >> 0);

  dest[0] |= VARINT_UINT8_HEADER;
}

static void varint_uint16_serialize(uint8_t *dest, uint16_t number)
{
  assert(dest);

  dest[0] = (uint8_t)(number >> 8);
  dest[1] = (uint8_t)(number >> 0);

  dest[0] |= VARINT_UINT16_HEADER;
}

static void varint_uint32_serialize(uint8_t *dest, uint32_t number)
{
  assert(dest);

  dest[0] = (uint8_t)(number >> 24);
  dest[1] = (uint8_t)(number >> 16);
  dest[2] = (uint8_t)(number >> 8);
  dest[3] = (uint8_t)(number >> 0);

  dest[0] |= VARINT_UINT32_HEADER;
}

static void varint_uint64_serialize(uint8_t *dest, uint64_t number)
{
  assert(dest);

  dest[0] = (uint8_t)(number >> 56);
  dest[1] = (uint8_t)(number >> 48);
  dest[2] = (uint8_t)(number >> 40);
  dest[3] = (uint8_t)(number >> 32);
  dest[4] = (uint8_t)(number >> 24);
  dest[5] = (uint8_t)(number >> 16);
  dest[6] = (uint8_t)(number >> 8);
  dest[7] = (uint8_t)(number >> 0);

  dest[0] |= VARINT_UINT64_HEADER;
}

H3C_ERROR h3c_varint_serialize(uint8_t *dest,
                               size_t size,
                               uint64_t varint,
                               size_t *varint_size)
{
  assert(varint_size);

  size_t actual_varint_size = varint_size_(varint);

  if (actual_varint_size == 0) {
    return H3C_ERROR_VARINT_OVERFLOW;
  }

  // If the varint's actual size is larger than the user's wanted (fixed) varint
  // size, return overflow as well.
  if (*varint_size != 0 && actual_varint_size > *varint_size) {
    return H3C_ERROR_VARINT_OVERFLOW;
  }

  *varint_size = MAX(actual_varint_size, *varint_size);

  if (dest == NULL) {
    return H3C_SUCCESS;
  }

  if (*varint_size > size) {
    return H3C_ERROR_BUFFER_TOO_SMALL;
  }

  switch (*varint_size) {
    case 1:
      varint_uint8_serialize(dest, (uint8_t) varint);
      break;
    case 2:
      varint_uint16_serialize(dest, (uint16_t) varint);
      break;
    case 4:
      varint_uint32_serialize(dest, (uint32_t) varint);
      break;
    case 8:
      varint_uint64_serialize(dest, varint);
      break;
    default:
      assert(0);
      return 0;
  }

  return H3C_SUCCESS;
}

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
    return H3C_ERROR_INCOMPLETE_VARINT;
  }

  *varint_size = 1;
  uint8_t header = *src >> 6;

  // varint size = 2^header
  *varint_size <<= header; // shift left => x2

  if (*varint_size > size) {
    return H3C_ERROR_INCOMPLETE_VARINT;
  }

  switch (*varint_size) {
    case 1:
      *varint = varint_uint8_parse(src);
      break;
    case 2:
      *varint = varint_uint16_parse(src);
      break;
    case 4:
      *varint = varint_uint32_parse(src);
      break;
    case 8:
      *varint = varint_uint64_parse(src);
      break;
    default:
      assert(0);
      return 0;
  }

  return H3C_SUCCESS;
}
