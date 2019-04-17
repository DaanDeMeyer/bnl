#include "varint.h"

#include "endianness.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// https://quicwg.org/base-drafts/draft-ietf-quic-transport.html#integer-encoding

size_t varint_size(varint_t varint)
{
  if (varint < 64) {
    return 1;
  }

  if (varint < 16383) {
    return 2;
  }

  if (varint < 1073741823) {
    return 4;
  }

  // WARNING: Values that use more than 62 bits are truncated!
  return 8;
}

// All parse functions convert from network to host byte order before returning
// a value.

static uint8_t uint8_parse(const uint8_t *src)
{
  uint8_t h = 0;
  memcpy(&h, src, sizeof(h));
  return h;
}

static uint16_t uint16_parse(const uint8_t *src)
{
  uint16_t h = 0;
  memcpy(&h, src, sizeof(h));
  return ntoh16(h);
}

static uint32_t uint32_parse(const uint8_t *src)
{
  uint32_t h = 0;
  memcpy(&h, src, sizeof(h));
  return ntoh32(h);
}

static uint64_t uint64_parse(const uint8_t *src)
{
  uint64_t h = 0;
  memcpy(&h, src, sizeof(h));
  return ntoh64(h);
}

size_t varint_parse(const uint8_t *src, size_t size, varint_t *varint)
{
  if (size == 0) {
    return 0;
  }

  // header: 00 = 0 => varint size: 1 = 2^0
  // header: 01 = 1 => varint size: 2 = 2^1
  // header: 10 = 2 => varint size: 4 = 2^2
  // header: 11 = 3 => varint size: 8 = 2^3
  // => varint size = 2^header
  uint8_t header = *src & 0xc0;
  size_t varint_size = 1U << header; // shift left => x2

  if (size < varint_size) {
    return 0;
  }

  switch (varint_size) {
  case 1:
    *varint = uint8_parse(src);
    break;
  case 2:
    *varint = uint16_parse(src);
    break;
  case 4:
    *varint = uint32_parse(src);
    break;
  case 8:
    *varint = uint64_parse(src);
    break;
  default:
    assert(0);
    return 0;
  }

  // Remove varint header from result. The varint header consists of the two
  // most significant bits so depending on the platform endianness we have to
  // zero the first two or the last two bits.
#if defined(H3C_LITTLE_ENDIAN)
  *varint &= 0xfffffffffffffffc;
#elif defined(H3C_BIG_ENDIAN)
  *varint &= 0x3fffffffffffffff;
#else
#  error "Endianness not defined"
#endif

  return varint_size;
}

// All serialize functions convert from host to network byte order before
// serializing a value.

static void uint8_serialize(uint8_t *dest, uint8_t number)
{
  *dest = number;
}

static void uint16_serialize(uint8_t *dest, uint16_t number)
{
  uint16_t n = hton16(number);
  memcpy(dest, &n, sizeof(number));
}

static void uint32_serialize(uint8_t *dest, uint32_t number)
{
  uint32_t n = hton32(number);
  memcpy(dest, &n, sizeof(number));
}

static void uint64_serialize(uint8_t *dest, uint64_t number)
{
  uint64_t n = hton64(number);
  memcpy(dest, &n, sizeof(number));
}

size_t varint_serialize(uint8_t *dest, size_t size, varint_t varint)
{
  if (size == 0) {
    return 0;
  }

  size_t varint_size_ = varint_size(varint);

  if (varint_size_ > size) {
    return 0;
  }

  // The variable integer limits are chosen so that after serializing the number
  // the two leftmost bits are always zero. This makes it possible to serialize
  // the number first and add the variable integer header second.
  switch (varint_size_) {
  case 1:
    uint8_serialize(dest, (uint8_t) varint);
    *dest |= 0x00;
    break;
  case 2:
    uint16_serialize(dest, (uint16_t) varint);
    *dest |= 0x40;
    break;
  case 4:
    uint32_serialize(dest, (uint32_t) varint);
    *dest |= 0x80;
    break;
  case 8:
    uint64_serialize(dest, varint);
    *dest |= 0xc0;
    break;
  default:
    assert(0);
    return 0;
  }

  return varint_size_;
}
