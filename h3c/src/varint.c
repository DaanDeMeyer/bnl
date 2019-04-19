#include <varint.h>

#include <assert.h>

// https://quicwg.org/base-drafts/draft-ietf-quic-transport.html#integer-encoding

size_t varint_size(varint_t varint)
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

// All parse functions convert from network to host byte order and remove the
// varint header before returning a value.

static uint8_t varint_uint8_parse(const uint8_t *src)
{
  return src[0] & 0x3f;
}

static uint16_t varint_uint16_parse(const uint8_t *src)
{
  uint16_t result = (uint16_t)((src[0] & 0x3f) << 8);
  return (uint16_t)(result | src[1]);
}

static uint32_t varint_uint32_parse(const uint8_t *src)
{
  uint32_t result = ((uint32_t)(src[0] & 0x3f)) << 24;
  result |= ((uint32_t) src[1]) << 16;
  result |= ((uint32_t) src[2]) << 8;
  return (uint32_t)(result | src[3]);
}

static uint64_t varint_uint64_parse(const uint8_t *src)
{
  uint64_t result = ((uint64_t)(src[0] & 0x3f)) << 56;
  result |= ((uint64_t) src[1]) << 48;
  result |= ((uint64_t) src[2]) << 40;
  result |= ((uint64_t) src[3]) << 32;
  result |= ((uint64_t) src[4]) << 24;
  result |= ((uint64_t) src[5]) << 16;
  result |= ((uint64_t) src[6]) << 8;
  return (uint64_t)(result | src[7]);
}

size_t varint_parse(const uint8_t *src, size_t size, varint_t *varint)
{
  if (size == 0) {
    return 0;
  }

  size_t varint_size = 1;
  uint8_t header = *src >> 6;

  // varint size = 2^header
  varint_size <<= header; // shift left => x2

  if (varint_size > size) {
    return 0;
  }

  switch (varint_size) {
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

  return varint_size;
}

// All serialize functions convert from host to network byte order (big-endian)
// and insert the varint header.

#define VARINT_UINT8_HEADER 0x00
#define VARINT_UINT16_HEADER 0x40
#define VARINT_UINT32_HEADER 0x80
#define VARINT_UINT64_HEADER 0xc0

static void varint_uint8_serialize(uint8_t *dest, uint8_t number)
{
  dest[0] = number | VARINT_UINT8_HEADER;
}

static void varint_uint16_serialize(uint8_t *dest, uint16_t number)
{
  dest[0] = (uint8_t)((number >> 8) | VARINT_UINT16_HEADER);
  dest[1] = (uint8_t)(number & 0xff);
}

static void varint_uint32_serialize(uint8_t *dest, uint32_t number)
{
  dest[0] = (uint8_t)((number >> 24) | VARINT_UINT32_HEADER);
  dest[1] = number >> 16 & 0xff;
  dest[2] = number >> 8 & 0xff;
  dest[3] = number & 0xff;
}

static void varint_uint64_serialize(uint8_t *dest, uint64_t number)
{
  dest[0] = (uint8_t)((number >> 56) | VARINT_UINT64_HEADER);
  dest[1] = number >> 48 & 0xff;
  dest[2] = number >> 40 & 0xff;
  dest[3] = number >> 32 & 0xff;
  dest[4] = number >> 24 & 0xff;
  dest[5] = number >> 16 & 0xff;
  dest[6] = number >> 8 & 0xff;
  dest[7] = number & 0xff;
}

size_t varint_serialize(uint8_t *dest, size_t size, varint_t varint)
{
  if (size == 0) {
    return 0;
  }

  size_t varint_size_ = varint_size(varint);

  if (varint_size_ == 0 || varint_size_ > size) {
    return 0;
  }

  switch (varint_size_) {
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

  return varint_size_;
}