#include <bnl/http3/header.hpp>

#include <bnl/string.hpp>

#include <cstdint>
#include <utility>

namespace bnl {
namespace http3 {
namespace qpack {
namespace table {
namespace fixed {

enum class type { header_value, header_only, missing, unknown };

static constexpr uint8_t INDEXED_HEADER_FIELD_PREFIX = 0x80;
static constexpr uint8_t LITERAL_WITH_NAME_REFERENCE_PREFIX = 0x40;
static constexpr uint8_t LITERAL_WITHOUT_NAME_REFERENCE_PREFIX = 0x20;

static type find_type(uint8_t byte)
{
  if ((byte & INDEXED_HEADER_FIELD_PREFIX) == INDEXED_HEADER_FIELD_PREFIX) {
    return type::header_value;
  }

  if ((byte & LITERAL_WITH_NAME_REFERENCE_PREFIX) ==
      LITERAL_WITH_NAME_REFERENCE_PREFIX) {
    return type::header_only;
  }

  if ((byte & LITERAL_WITHOUT_NAME_REFERENCE_PREFIX) ==
      LITERAL_WITHOUT_NAME_REFERENCE_PREFIX) {
    return type::missing;
  }

  return type::unknown;
}

std::pair<bool, header> find_header_value(uint8_t index)
{
  switch (index) {
    case 15:
      return { true, { ":method", "CONNECT" } };
    case 16:
      return { true, { ":method", "DELETE" } };
    case 17:
      return { true, { ":method", "GET" } };
    case 18:
      return { true, { ":method", "HEAD" } };
    case 19:
      return { true, { ":method", "OPTIONS" } };
    case 20:
      return { true, { ":method", "POST" } };
    case 21:
      return { true, { ":method", "PUT" } };
    case 1:
      return { true, { ":path", "/" } };
    case 22:
      return { true, { ":scheme", "http" } };
    case 23:
      return { true, { ":scheme", "https" } };
    case 24:
      return { true, { ":status", "103" } };
    case 25:
      return { true, { ":status", "200" } };
    case 26:
      return { true, { ":status", "304" } };
    case 27:
      return { true, { ":status", "404" } };
    case 28:
      return { true, { ":status", "503" } };
    case 63:
      return { true, { ":status", "100" } };
    case 64:
      return { true, { ":status", "204" } };
    case 65:
      return { true, { ":status", "206" } };
    case 66:
      return { true, { ":status", "302" } };
    case 67:
      return { true, { ":status", "400" } };
    case 68:
      return { true, { ":status", "403" } };
    case 69:
      return { true, { ":status", "421" } };
    case 70:
      return { true, { ":status", "425" } };
    case 71:
      return { true, { ":status", "500" } };
    case 29:
      return { true, { "accept", "*/*" } };
    case 30:
      return { true, { "accept", "application/dns-message" } };
    case 31:
      return { true, { "accept-encoding", "gzip, deflate, br" } };
    case 32:
      return { true, { "accept-ranges", "bytes" } };
    case 73:
      return { true, { "access-control-allow-credentials", "FALSE" } };
    case 74:
      return { true, { "access-control-allow-credentials", "TRUE" } };
    case 33:
      return { true, { "access-control-allow-headers", "cache-control" } };
    case 34:
      return { true, { "access-control-allow-headers", "content-type" } };
    case 75:
      return { true, { "access-control-allow-headers", "*" } };
    case 76:
      return { true, { "access-control-allow-methods", "get" } };
    case 77:
      return { true, { "access-control-allow-methods", "get, post, options" } };
    case 78:
      return { true, { "access-control-allow-methods", "options" } };
    case 35:
      return { true, { "access-control-allow-origin", "*" } };
    case 79:
      return { true, { "access-control-expose-headers", "content-length" } };
    case 80:
      return { true, { "access-control-request-headers", "content-type" } };
    case 81:
      return { true, { "access-control-request-method", "get" } };
    case 82:
      return { true, { "access-control-request-method", "post" } };
    case 2:
      return { true, { "age", "0" } };
    case 83:
      return { true, { "alt-svc", "clear" } };
    case 36:
      return { true, { "cache-control", "max-age=0" } };
    case 37:
      return { true, { "cache-control", "max-age=2592000" } };
    case 38:
      return { true, { "cache-control", "max-age=604800" } };
    case 39:
      return { true, { "cache-control", "no-cache" } };
    case 40:
      return { true, { "cache-control", "no-store" } };
    case 41:
      return { true, { "cache-control", "public, max-age=31536000" } };
    case 42:
      return { true, { "content-encoding", "br" } };
    case 43:
      return { true, { "content-encoding", "gzip" } };
    case 4:
      return { true, { "content-length", "0" } };
    case 85:
      return { true,
               { "content-security-policy",
                 "script-src 'none'; object-src 'none'; base-uri 'none'" } };
    case 44:
      return { true, { "content-type", "application/dns-message" } };
    case 45:
      return { true, { "content-type", "application/javascript" } };
    case 46:
      return { true, { "content-type", "application/json" } };
    case 47:
      return { true, { "content-type", "application/x-www-form-urlencoded" } };
    case 48:
      return { true, { "content-type", "image/gif" } };
    case 49:
      return { true, { "content-type", "image/jpeg" } };
    case 50:
      return { true, { "content-type", "image/png" } };
    case 51:
      return { true, { "content-type", "text/css" } };
    case 52:
      return { true, { "content-type", "text/html; charset=utf-8" } };
    case 53:
      return { true, { "content-type", "text/plain" } };
    case 54:
      return { true, { "content-type", "text/plain;charset=utf-8" } };
    case 86:
      return { true, { "early-data", "1" } };
    case 91:
      return { true, { "purpose", "prefetch" } };
    case 55:
      return { true, { "range", "bytes=0-" } };
    case 56:
      return { true, { "strict-transport-security", "max-age=31536000" } };
    case 57:
      return { true,
               { "strict-transport-security",
                 "max-age=31536000; includesubdomains" } };
    case 58:
      return { true,
               { "strict-transport-security",
                 "max-age=31536000; includesubdomains; preload" } };
    case 93:
      return { true, { "timing-allow-origin", "*" } };
    case 94:
      return { true, { "upgrade-insecure-requests", "1" } };
    case 59:
      return { true, { "vary", "accept-encoding" } };
    case 60:
      return { true, { "vary", "origin" } };
    case 61:
      return { true, { "x-content-type-options", "nosniff" } };
    case 97:
      return { true, { "x-frame-options", "deny" } };
    case 98:
      return { true, { "x-frame-options", "sameorigin" } };
    case 62:
      return { true, { "x-xss-protection", "1; mode=block" } };
  }

  return { false, {} };
}

std::pair<bool, string> find_header_only(uint8_t index)
{
  switch (index) {
    case 0:
      return { true, ":authority" };
    case 15:
      return { true, ":method" };
    case 16:
      return { true, ":method" };
    case 17:
      return { true, ":method" };
    case 18:
      return { true, ":method" };
    case 19:
      return { true, ":method" };
    case 20:
      return { true, ":method" };
    case 21:
      return { true, ":method" };
    case 1:
      return { true, ":path" };
    case 22:
      return { true, ":scheme" };
    case 23:
      return { true, ":scheme" };
    case 24:
      return { true, ":status" };
    case 25:
      return { true, ":status" };
    case 26:
      return { true, ":status" };
    case 27:
      return { true, ":status" };
    case 28:
      return { true, ":status" };
    case 63:
      return { true, ":status" };
    case 64:
      return { true, ":status" };
    case 65:
      return { true, ":status" };
    case 66:
      return { true, ":status" };
    case 67:
      return { true, ":status" };
    case 68:
      return { true, ":status" };
    case 69:
      return { true, ":status" };
    case 70:
      return { true, ":status" };
    case 71:
      return { true, ":status" };
    case 29:
      return { true, "accept" };
    case 30:
      return { true, "accept" };
    case 31:
      return { true, "accept-encoding" };
    case 72:
      return { true, "accept-language" };
    case 32:
      return { true, "accept-ranges" };
    case 73:
      return { true, "access-control-allow-credentials" };
    case 74:
      return { true, "access-control-allow-credentials" };
    case 33:
      return { true, "access-control-allow-headers" };
    case 34:
      return { true, "access-control-allow-headers" };
    case 75:
      return { true, "access-control-allow-headers" };
    case 76:
      return { true, "access-control-allow-methods" };
    case 77:
      return { true, "access-control-allow-methods" };
    case 78:
      return { true, "access-control-allow-methods" };
    case 35:
      return { true, "access-control-allow-origin" };
    case 79:
      return { true, "access-control-expose-headers" };
    case 80:
      return { true, "access-control-request-headers" };
    case 81:
      return { true, "access-control-request-method" };
    case 82:
      return { true, "access-control-request-method" };
    case 2:
      return { true, "age" };
    case 83:
      return { true, "alt-svc" };
    case 84:
      return { true, "authorization" };
    case 36:
      return { true, "cache-control" };
    case 37:
      return { true, "cache-control" };
    case 38:
      return { true, "cache-control" };
    case 39:
      return { true, "cache-control" };
    case 40:
      return { true, "cache-control" };
    case 41:
      return { true, "cache-control" };
    case 3:
      return { true, "content-disposition" };
    case 42:
      return { true, "content-encoding" };
    case 43:
      return { true, "content-encoding" };
    case 4:
      return { true, "content-length" };
    case 85:
      return { true, "content-security-policy" };
    case 44:
      return { true, "content-type" };
    case 45:
      return { true, "content-type" };
    case 46:
      return { true, "content-type" };
    case 47:
      return { true, "content-type" };
    case 48:
      return { true, "content-type" };
    case 49:
      return { true, "content-type" };
    case 50:
      return { true, "content-type" };
    case 51:
      return { true, "content-type" };
    case 52:
      return { true, "content-type" };
    case 53:
      return { true, "content-type" };
    case 54:
      return { true, "content-type" };
    case 5:
      return { true, "cookie" };
    case 6:
      return { true, "date" };
    case 86:
      return { true, "early-data" };
    case 7:
      return { true, "etag" };
    case 87:
      return { true, "expect-ct" };
    case 88:
      return { true, "forwarded" };
    case 8:
      return { true, "if-modified-since" };
    case 9:
      return { true, "if-none-match" };
    case 89:
      return { true, "if-range" };
    case 10:
      return { true, "last-modified" };
    case 11:
      return { true, "link" };
    case 12:
      return { true, "location" };
    case 90:
      return { true, "origin" };
    case 91:
      return { true, "purpose" };
    case 55:
      return { true, "range" };
    case 13:
      return { true, "referer" };
    case 92:
      return { true, "server" };
    case 14:
      return { true, "set-cookie" };
    case 56:
      return { true, "strict-transport-security" };
    case 57:
      return { true, "strict-transport-security" };
    case 58:
      return { true, "strict-transport-security" };
    case 93:
      return { true, "timing-allow-origin" };
    case 94:
      return { true, "upgrade-insecure-requests" };
    case 95:
      return { true, "user-agent" };
    case 59:
      return { true, "vary" };
    case 60:
      return { true, "vary" };
    case 61:
      return { true, "x-content-type-options" };
    case 96:
      return { true, "x-forwarded-for" };
    case 97:
      return { true, "x-frame-options" };
    case 98:
      return { true, "x-frame-options" };
    case 62:
      return { true, "x-xss-protection" };
  }

  return { false, {} };
}

} // namespace fixed
} // namespace table
} // namespace qpack
} // namespace http3
} // namespace bnl
