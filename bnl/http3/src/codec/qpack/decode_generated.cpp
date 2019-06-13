#include <bnl/http3/header.hpp>

#include <cstdint>

namespace bnl {
namespace http3 {
namespace qpack {
namespace static_table {

bool find_header_value(uint8_t index, header *header)
{
  switch(index) {
    case 15:
  *header = { ":method", "CONNECT" };
  return true;case 16:
  *header = { ":method", "DELETE" };
  return true;case 17:
  *header = { ":method", "GET" };
  return true;case 18:
  *header = { ":method", "HEAD" };
  return true;case 19:
  *header = { ":method", "OPTIONS" };
  return true;case 20:
  *header = { ":method", "POST" };
  return true;case 21:
  *header = { ":method", "PUT" };
  return true;case 1:
  *header = { ":path", "/" };
  return true;case 22:
  *header = { ":scheme", "http" };
  return true;case 23:
  *header = { ":scheme", "https" };
  return true;case 24:
  *header = { ":status", "103" };
  return true;case 25:
  *header = { ":status", "200" };
  return true;case 26:
  *header = { ":status", "304" };
  return true;case 27:
  *header = { ":status", "404" };
  return true;case 28:
  *header = { ":status", "503" };
  return true;case 63:
  *header = { ":status", "100" };
  return true;case 64:
  *header = { ":status", "204" };
  return true;case 65:
  *header = { ":status", "206" };
  return true;case 66:
  *header = { ":status", "302" };
  return true;case 67:
  *header = { ":status", "400" };
  return true;case 68:
  *header = { ":status", "403" };
  return true;case 69:
  *header = { ":status", "421" };
  return true;case 70:
  *header = { ":status", "425" };
  return true;case 71:
  *header = { ":status", "500" };
  return true;case 29:
  *header = { "accept", "*/*" };
  return true;case 30:
  *header = { "accept", "application/dns-message" };
  return true;case 31:
  *header = { "accept-encoding", "gzip, deflate, br" };
  return true;case 32:
  *header = { "accept-ranges", "bytes" };
  return true;case 73:
  *header = { "access-control-allow-credentials", "FALSE" };
  return true;case 74:
  *header = { "access-control-allow-credentials", "TRUE" };
  return true;case 33:
  *header = { "access-control-allow-headers", "cache-control" };
  return true;case 34:
  *header = { "access-control-allow-headers", "content-type" };
  return true;case 75:
  *header = { "access-control-allow-headers", "*" };
  return true;case 76:
  *header = { "access-control-allow-methods", "get" };
  return true;case 77:
  *header = { "access-control-allow-methods", "get, post, options" };
  return true;case 78:
  *header = { "access-control-allow-methods", "options" };
  return true;case 35:
  *header = { "access-control-allow-origin", "*" };
  return true;case 79:
  *header = { "access-control-expose-headers", "content-length" };
  return true;case 80:
  *header = { "access-control-request-headers", "content-type" };
  return true;case 81:
  *header = { "access-control-request-method", "get" };
  return true;case 82:
  *header = { "access-control-request-method", "post" };
  return true;case 2:
  *header = { "age", "0" };
  return true;case 83:
  *header = { "alt-svc", "clear" };
  return true;case 36:
  *header = { "cache-control", "max-age=0" };
  return true;case 37:
  *header = { "cache-control", "max-age=2592000" };
  return true;case 38:
  *header = { "cache-control", "max-age=604800" };
  return true;case 39:
  *header = { "cache-control", "no-cache" };
  return true;case 40:
  *header = { "cache-control", "no-store" };
  return true;case 41:
  *header = { "cache-control", "public, max-age=31536000" };
  return true;case 42:
  *header = { "content-encoding", "br" };
  return true;case 43:
  *header = { "content-encoding", "gzip" };
  return true;case 4:
  *header = { "content-length", "0" };
  return true;case 85:
  *header = { "content-security-policy", "script-src 'none'; object-src 'none'; base-uri 'none'" };
  return true;case 44:
  *header = { "content-type", "application/dns-message" };
  return true;case 45:
  *header = { "content-type", "application/javascript" };
  return true;case 46:
  *header = { "content-type", "application/json" };
  return true;case 47:
  *header = { "content-type", "application/x-www-form-urlencoded" };
  return true;case 48:
  *header = { "content-type", "image/gif" };
  return true;case 49:
  *header = { "content-type", "image/jpeg" };
  return true;case 50:
  *header = { "content-type", "image/png" };
  return true;case 51:
  *header = { "content-type", "text/css" };
  return true;case 52:
  *header = { "content-type", "text/html; charset=utf-8" };
  return true;case 53:
  *header = { "content-type", "text/plain" };
  return true;case 54:
  *header = { "content-type", "text/plain;charset=utf-8" };
  return true;case 86:
  *header = { "early-data", "1" };
  return true;case 91:
  *header = { "purpose", "prefetch" };
  return true;case 55:
  *header = { "range", "bytes=0-" };
  return true;case 56:
  *header = { "strict-transport-security", "max-age=31536000" };
  return true;case 57:
  *header = { "strict-transport-security", "max-age=31536000; includesubdomains" };
  return true;case 58:
  *header = { "strict-transport-security", "max-age=31536000; includesubdomains; preload" };
  return true;case 93:
  *header = { "timing-allow-origin", "*" };
  return true;case 94:
  *header = { "upgrade-insecure-requests", "1" };
  return true;case 59:
  *header = { "vary", "accept-encoding" };
  return true;case 60:
  *header = { "vary", "origin" };
  return true;case 61:
  *header = { "x-content-type-options", "nosniff" };
  return true;case 97:
  *header = { "x-frame-options", "deny" };
  return true;case 98:
  *header = { "x-frame-options", "sameorigin" };
  return true;case 62:
  *header = { "x-xss-protection", "1; mode=block" };
  return true;
  }

  return false;
}

bool find_header_only(uint8_t index, header *header)
{
  switch(index) {
    case 0:
  *header = { ":authority", {} };
  return true;case 15:
  *header = { ":method", {} };
  return true;case 16:
  *header = { ":method", {} };
  return true;case 17:
  *header = { ":method", {} };
  return true;case 18:
  *header = { ":method", {} };
  return true;case 19:
  *header = { ":method", {} };
  return true;case 20:
  *header = { ":method", {} };
  return true;case 21:
  *header = { ":method", {} };
  return true;case 1:
  *header = { ":path", {} };
  return true;case 22:
  *header = { ":scheme", {} };
  return true;case 23:
  *header = { ":scheme", {} };
  return true;case 24:
  *header = { ":status", {} };
  return true;case 25:
  *header = { ":status", {} };
  return true;case 26:
  *header = { ":status", {} };
  return true;case 27:
  *header = { ":status", {} };
  return true;case 28:
  *header = { ":status", {} };
  return true;case 63:
  *header = { ":status", {} };
  return true;case 64:
  *header = { ":status", {} };
  return true;case 65:
  *header = { ":status", {} };
  return true;case 66:
  *header = { ":status", {} };
  return true;case 67:
  *header = { ":status", {} };
  return true;case 68:
  *header = { ":status", {} };
  return true;case 69:
  *header = { ":status", {} };
  return true;case 70:
  *header = { ":status", {} };
  return true;case 71:
  *header = { ":status", {} };
  return true;case 29:
  *header = { "accept", {} };
  return true;case 30:
  *header = { "accept", {} };
  return true;case 31:
  *header = { "accept-encoding", {} };
  return true;case 72:
  *header = { "accept-language", {} };
  return true;case 32:
  *header = { "accept-ranges", {} };
  return true;case 73:
  *header = { "access-control-allow-credentials", {} };
  return true;case 74:
  *header = { "access-control-allow-credentials", {} };
  return true;case 33:
  *header = { "access-control-allow-headers", {} };
  return true;case 34:
  *header = { "access-control-allow-headers", {} };
  return true;case 75:
  *header = { "access-control-allow-headers", {} };
  return true;case 76:
  *header = { "access-control-allow-methods", {} };
  return true;case 77:
  *header = { "access-control-allow-methods", {} };
  return true;case 78:
  *header = { "access-control-allow-methods", {} };
  return true;case 35:
  *header = { "access-control-allow-origin", {} };
  return true;case 79:
  *header = { "access-control-expose-headers", {} };
  return true;case 80:
  *header = { "access-control-request-headers", {} };
  return true;case 81:
  *header = { "access-control-request-method", {} };
  return true;case 82:
  *header = { "access-control-request-method", {} };
  return true;case 2:
  *header = { "age", {} };
  return true;case 83:
  *header = { "alt-svc", {} };
  return true;case 84:
  *header = { "authorization", {} };
  return true;case 36:
  *header = { "cache-control", {} };
  return true;case 37:
  *header = { "cache-control", {} };
  return true;case 38:
  *header = { "cache-control", {} };
  return true;case 39:
  *header = { "cache-control", {} };
  return true;case 40:
  *header = { "cache-control", {} };
  return true;case 41:
  *header = { "cache-control", {} };
  return true;case 3:
  *header = { "content-disposition", {} };
  return true;case 42:
  *header = { "content-encoding", {} };
  return true;case 43:
  *header = { "content-encoding", {} };
  return true;case 4:
  *header = { "content-length", {} };
  return true;case 85:
  *header = { "content-security-policy", {} };
  return true;case 44:
  *header = { "content-type", {} };
  return true;case 45:
  *header = { "content-type", {} };
  return true;case 46:
  *header = { "content-type", {} };
  return true;case 47:
  *header = { "content-type", {} };
  return true;case 48:
  *header = { "content-type", {} };
  return true;case 49:
  *header = { "content-type", {} };
  return true;case 50:
  *header = { "content-type", {} };
  return true;case 51:
  *header = { "content-type", {} };
  return true;case 52:
  *header = { "content-type", {} };
  return true;case 53:
  *header = { "content-type", {} };
  return true;case 54:
  *header = { "content-type", {} };
  return true;case 5:
  *header = { "cookie", {} };
  return true;case 6:
  *header = { "date", {} };
  return true;case 86:
  *header = { "early-data", {} };
  return true;case 7:
  *header = { "etag", {} };
  return true;case 87:
  *header = { "expect-ct", {} };
  return true;case 88:
  *header = { "forwarded", {} };
  return true;case 8:
  *header = { "if-modified-since", {} };
  return true;case 9:
  *header = { "if-none-match", {} };
  return true;case 89:
  *header = { "if-range", {} };
  return true;case 10:
  *header = { "last-modified", {} };
  return true;case 11:
  *header = { "link", {} };
  return true;case 12:
  *header = { "location", {} };
  return true;case 90:
  *header = { "origin", {} };
  return true;case 91:
  *header = { "purpose", {} };
  return true;case 55:
  *header = { "range", {} };
  return true;case 13:
  *header = { "referer", {} };
  return true;case 92:
  *header = { "server", {} };
  return true;case 14:
  *header = { "set-cookie", {} };
  return true;case 56:
  *header = { "strict-transport-security", {} };
  return true;case 57:
  *header = { "strict-transport-security", {} };
  return true;case 58:
  *header = { "strict-transport-security", {} };
  return true;case 93:
  *header = { "timing-allow-origin", {} };
  return true;case 94:
  *header = { "upgrade-insecure-requests", {} };
  return true;case 95:
  *header = { "user-agent", {} };
  return true;case 59:
  *header = { "vary", {} };
  return true;case 60:
  *header = { "vary", {} };
  return true;case 61:
  *header = { "x-content-type-options", {} };
  return true;case 96:
  *header = { "x-forwarded-for", {} };
  return true;case 97:
  *header = { "x-frame-options", {} };
  return true;case 98:
  *header = { "x-frame-options", {} };
  return true;case 62:
  *header = { "x-xss-protection", {} };
  return true;
  }

  return false;
}

}
}
}
}
