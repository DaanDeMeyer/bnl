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

// All parse functions convert from network to host byte order and remove the
// varint header before returning a value.

static uint8_t varint_uint8_parse(const uint8_t *src)
{
  uint8_t h = 0;
  memcpy(&h, src, sizeof(h));
  h &= 0x3f;
  return h;
}

static uint16_t varint_uint16_parse(const uint8_t *src)
{
  uint16_t h = 0;
  memcpy(&h, src, sizeof(h));
  h &= 0x3fff;
  return ntoh16(h);
}

static uint32_t varint_uint32_parse(const uint8_t *src)
{
  uint32_t h = 0;
  memcpy(&h, src, sizeof(h));
  h &= 0x3fffffff;
  return ntoh32(h);
}

static uint64_t varint_uint64_parse(const uint8_t *src)
{
  uint64_t h = 0;
  memcpy(&h, src, sizeof(h));
  h &= 0x3fffffffffffffff;
  return ntoh64(h);
}

size_t varint_parse(const uint8_t *src, size_t size, varint_t *varint)
{
  if (size == 0) {
    return 0;
  }

  size_t varint_size = 1;
  uint8_t header = *src & 0xc0;

  // varint size = 2^header
  varint_size <<= header; // shift left => x2

  if (size < varint_size) {
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

// All serialize functions convert from host to network byte order and insert
// the varint header before serializing a value.

// The variable integer limits are chosen so that after serializing the number
// the two leftmost bits are always zero. This makes it possible to serialize
// the number first and add the variable integer header second.

static void varint_uint8_serialize(uint8_t *dest, uint8_t number)
{
  *dest = number;
  *dest |= 0x00;
}

static void varint_uint16_serialize(uint8_t *dest, uint16_t number)
{
  uint16_t n = hton16(number);
  memcpy(dest, &n, sizeof(number));
  *dest |= 0x40;
}

static void varint_uint32_serialize(uint8_t *dest, uint32_t number)
{
  uint32_t n = hton32(number);
  memcpy(dest, &n, sizeof(number));
  *dest |= 0x80;
}

static void varint_uint64_serialize(uint8_t *dest, uint64_t number)
{
  uint64_t n = hton64(number);
  memcpy(dest, &n, sizeof(number));
  *dest |= 0xc0;
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
