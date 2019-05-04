#include <h3c/varint.h>

#include <util.h>

#include <assert.h>

static size_t varint_size_(uint64_t varint)
{
  if (varint < 0x40) {
    return H3C_VARINT_UINT8_SIZE;
  }

  if (varint < (0x40 << 8)) {
    return H3C_VARINT_UINT16_SIZE;
  }

  if (varint < (0x40 << 24)) {
    return H3C_VARINT_UINT32_SIZE;
  }

  if (varint < (0x40ULL << 56)) {
    return H3C_VARINT_UINT64_SIZE;
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
    case H3C_VARINT_UINT8_SIZE:
      varint_uint8_serialize(dest, (uint8_t) varint);
      break;
    case H3C_VARINT_UINT16_SIZE:
      varint_uint16_serialize(dest, (uint16_t) varint);
      break;
    case H3C_VARINT_UINT32_SIZE:
      varint_uint32_serialize(dest, (uint32_t) varint);
      break;
    case H3C_VARINT_UINT64_SIZE:
      varint_uint64_serialize(dest, varint);
      break;
    default:
      assert(0);
      return 0;
  }

  return H3C_SUCCESS;
}
