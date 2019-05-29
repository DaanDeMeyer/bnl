#include <h3c/http.hpp>

#include <cstdint>

namespace h3c {
namespace qpack {
namespace static_table {

bool find_header_value(uint8_t index, header *header)
{
  switch (index) {
    case 15:
      *header = { { ":method", sizeof(":method") - 1 },
                  { "CONNECT", sizeof("CONNECT") - 1 } };
      return true;
    case 16:
      *header = { { ":method", sizeof(":method") - 1 },
                  { "DELETE", sizeof("DELETE") - 1 } };
      return true;
    case 17:
      *header = { { ":method", sizeof(":method") - 1 },
                  { "GET", sizeof("GET") - 1 } };
      return true;
    case 18:
      *header = { { ":method", sizeof(":method") - 1 },
                  { "HEAD", sizeof("HEAD") - 1 } };
      return true;
    case 19:
      *header = { { ":method", sizeof(":method") - 1 },
                  { "OPTIONS", sizeof("OPTIONS") - 1 } };
      return true;
    case 20:
      *header = { { ":method", sizeof(":method") - 1 },
                  { "POST", sizeof("POST") - 1 } };
      return true;
    case 21:
      *header = { { ":method", sizeof(":method") - 1 },
                  { "PUT", sizeof("PUT") - 1 } };
      return true;
    case 1:
      *header = { { ":path", sizeof(":path") - 1 }, { "/", sizeof("/") - 1 } };
      return true;
    case 22:
      *header = { { ":scheme", sizeof(":scheme") - 1 },
                  { "http", sizeof("http") - 1 } };
      return true;
    case 23:
      *header = { { ":scheme", sizeof(":scheme") - 1 },
                  { "https", sizeof("https") - 1 } };
      return true;
    case 24:
      *header = { { ":status", sizeof(":status") - 1 },
                  { "103", sizeof("103") - 1 } };
      return true;
    case 25:
      *header = { { ":status", sizeof(":status") - 1 },
                  { "200", sizeof("200") - 1 } };
      return true;
    case 26:
      *header = { { ":status", sizeof(":status") - 1 },
                  { "304", sizeof("304") - 1 } };
      return true;
    case 27:
      *header = { { ":status", sizeof(":status") - 1 },
                  { "404", sizeof("404") - 1 } };
      return true;
    case 28:
      *header = { { ":status", sizeof(":status") - 1 },
                  { "503", sizeof("503") - 1 } };
      return true;
    case 63:
      *header = { { ":status", sizeof(":status") - 1 },
                  { "100", sizeof("100") - 1 } };
      return true;
    case 64:
      *header = { { ":status", sizeof(":status") - 1 },
                  { "204", sizeof("204") - 1 } };
      return true;
    case 65:
      *header = { { ":status", sizeof(":status") - 1 },
                  { "206", sizeof("206") - 1 } };
      return true;
    case 66:
      *header = { { ":status", sizeof(":status") - 1 },
                  { "302", sizeof("302") - 1 } };
      return true;
    case 67:
      *header = { { ":status", sizeof(":status") - 1 },
                  { "400", sizeof("400") - 1 } };
      return true;
    case 68:
      *header = { { ":status", sizeof(":status") - 1 },
                  { "403", sizeof("403") - 1 } };
      return true;
    case 69:
      *header = { { ":status", sizeof(":status") - 1 },
                  { "421", sizeof("421") - 1 } };
      return true;
    case 70:
      *header = { { ":status", sizeof(":status") - 1 },
                  { "425", sizeof("425") - 1 } };
      return true;
    case 71:
      *header = { { ":status", sizeof(":status") - 1 },
                  { "500", sizeof("500") - 1 } };
      return true;
    case 29:
      *header = { { "accept", sizeof("accept") - 1 },
                  { "*/*", sizeof("*/*") - 1 } };
      return true;
    case 30:
      *header = { { "accept", sizeof("accept") - 1 },
                  { "application/dns-message",
                    sizeof("application/dns-message") - 1 } };
      return true;
    case 31:
      *header = { { "accept-encoding", sizeof("accept-encoding") - 1 },
                  { "gzip, deflate, br", sizeof("gzip, deflate, br") - 1 } };
      return true;
    case 32:
      *header = { { "accept-ranges", sizeof("accept-ranges") - 1 },
                  { "bytes", sizeof("bytes") - 1 } };
      return true;
    case 73:
      *header = { { "access-control-allow-credentials",
                    sizeof("access-control-allow-credentials") - 1 },
                  { "FALSE", sizeof("FALSE") - 1 } };
      return true;
    case 74:
      *header = { { "access-control-allow-credentials",
                    sizeof("access-control-allow-credentials") - 1 },
                  { "TRUE", sizeof("TRUE") - 1 } };
      return true;
    case 33:
      *header = { { "access-control-allow-headers",
                    sizeof("access-control-allow-headers") - 1 },
                  { "cache-control", sizeof("cache-control") - 1 } };
      return true;
    case 34:
      *header = { { "access-control-allow-headers",
                    sizeof("access-control-allow-headers") - 1 },
                  { "content-type", sizeof("content-type") - 1 } };
      return true;
    case 75:
      *header = { { "access-control-allow-headers",
                    sizeof("access-control-allow-headers") - 1 },
                  { "*", sizeof("*") - 1 } };
      return true;
    case 76:
      *header = { { "access-control-allow-methods",
                    sizeof("access-control-allow-methods") - 1 },
                  { "get", sizeof("get") - 1 } };
      return true;
    case 77:
      *header = { { "access-control-allow-methods",
                    sizeof("access-control-allow-methods") - 1 },
                  { "get, post, options", sizeof("get, post, options") - 1 } };
      return true;
    case 78:
      *header = { { "access-control-allow-methods",
                    sizeof("access-control-allow-methods") - 1 },
                  { "options", sizeof("options") - 1 } };
      return true;
    case 35:
      *header = { { "access-control-allow-origin",
                    sizeof("access-control-allow-origin") - 1 },
                  { "*", sizeof("*") - 1 } };
      return true;
    case 79:
      *header = { { "access-control-expose-headers",
                    sizeof("access-control-expose-headers") - 1 },
                  { "content-length", sizeof("content-length") - 1 } };
      return true;
    case 80:
      *header = { { "access-control-request-headers",
                    sizeof("access-control-request-headers") - 1 },
                  { "content-type", sizeof("content-type") - 1 } };
      return true;
    case 81:
      *header = { { "access-control-request-method",
                    sizeof("access-control-request-method") - 1 },
                  { "get", sizeof("get") - 1 } };
      return true;
    case 82:
      *header = { { "access-control-request-method",
                    sizeof("access-control-request-method") - 1 },
                  { "post", sizeof("post") - 1 } };
      return true;
    case 2:
      *header = { { "age", sizeof("age") - 1 }, { "0", sizeof("0") - 1 } };
      return true;
    case 83:
      *header = { { "alt-svc", sizeof("alt-svc") - 1 },
                  { "clear", sizeof("clear") - 1 } };
      return true;
    case 36:
      *header = { { "cache-control", sizeof("cache-control") - 1 },
                  { "max-age=0", sizeof("max-age=0") - 1 } };
      return true;
    case 37:
      *header = { { "cache-control", sizeof("cache-control") - 1 },
                  { "max-age=2592000", sizeof("max-age=2592000") - 1 } };
      return true;
    case 38:
      *header = { { "cache-control", sizeof("cache-control") - 1 },
                  { "max-age=604800", sizeof("max-age=604800") - 1 } };
      return true;
    case 39:
      *header = { { "cache-control", sizeof("cache-control") - 1 },
                  { "no-cache", sizeof("no-cache") - 1 } };
      return true;
    case 40:
      *header = { { "cache-control", sizeof("cache-control") - 1 },
                  { "no-store", sizeof("no-store") - 1 } };
      return true;
    case 41:
      *header = { { "cache-control", sizeof("cache-control") - 1 },
                  { "public, max-age=31536000",
                    sizeof("public, max-age=31536000") - 1 } };
      return true;
    case 42:
      *header = { { "content-encoding", sizeof("content-encoding") - 1 },
                  { "br", sizeof("br") - 1 } };
      return true;
    case 43:
      *header = { { "content-encoding", sizeof("content-encoding") - 1 },
                  { "gzip", sizeof("gzip") - 1 } };
      return true;
    case 4:
      *header = { { "content-length", sizeof("content-length") - 1 },
                  { "0", sizeof("0") - 1 } };
      return true;
    case 85:
      *header = {
        { "content-security-policy", sizeof("content-security-policy") - 1 },
        { "script-src 'none'; object-src 'none'; base-uri 'none'",
          sizeof("script-src 'none'; object-src 'none'; base-uri 'none'") - 1 }
      };
      return true;
    case 44:
      *header = { { "content-type", sizeof("content-type") - 1 },
                  { "application/dns-message",
                    sizeof("application/dns-message") - 1 } };
      return true;
    case 45:
      *header = { { "content-type", sizeof("content-type") - 1 },
                  { "application/javascript",
                    sizeof("application/javascript") - 1 } };
      return true;
    case 46:
      *header = { { "content-type", sizeof("content-type") - 1 },
                  { "application/json", sizeof("application/json") - 1 } };
      return true;
    case 47:
      *header = { { "content-type", sizeof("content-type") - 1 },
                  { "application/x-www-form-urlencoded",
                    sizeof("application/x-www-form-urlencoded") - 1 } };
      return true;
    case 48:
      *header = { { "content-type", sizeof("content-type") - 1 },
                  { "image/gif", sizeof("image/gif") - 1 } };
      return true;
    case 49:
      *header = { { "content-type", sizeof("content-type") - 1 },
                  { "image/jpeg", sizeof("image/jpeg") - 1 } };
      return true;
    case 50:
      *header = { { "content-type", sizeof("content-type") - 1 },
                  { "image/png", sizeof("image/png") - 1 } };
      return true;
    case 51:
      *header = { { "content-type", sizeof("content-type") - 1 },
                  { "text/css", sizeof("text/css") - 1 } };
      return true;
    case 52:
      *header = { { "content-type", sizeof("content-type") - 1 },
                  { "text/html; charset=utf-8",
                    sizeof("text/html; charset=utf-8") - 1 } };
      return true;
    case 53:
      *header = { { "content-type", sizeof("content-type") - 1 },
                  { "text/plain", sizeof("text/plain") - 1 } };
      return true;
    case 54:
      *header = { { "content-type", sizeof("content-type") - 1 },
                  { "text/plain;charset=utf-8",
                    sizeof("text/plain;charset=utf-8") - 1 } };
      return true;
    case 86:
      *header = { { "early-data", sizeof("early-data") - 1 },
                  { "1", sizeof("1") - 1 } };
      return true;
    case 91:
      *header = { { "purpose", sizeof("purpose") - 1 },
                  { "prefetch", sizeof("prefetch") - 1 } };
      return true;
    case 55:
      *header = { { "range", sizeof("range") - 1 },
                  { "bytes=0-", sizeof("bytes=0-") - 1 } };
      return true;
    case 56:
      *header = { { "strict-transport-security",
                    sizeof("strict-transport-security") - 1 },
                  { "max-age=31536000", sizeof("max-age=31536000") - 1 } };
      return true;
    case 57:
      *header = { { "strict-transport-security",
                    sizeof("strict-transport-security") - 1 },
                  { "max-age=31536000; includesubdomains",
                    sizeof("max-age=31536000; includesubdomains") - 1 } };
      return true;
    case 58:
      *header = { { "strict-transport-security",
                    sizeof("strict-transport-security") - 1 },
                  { "max-age=31536000; includesubdomains; preload",
                    sizeof("max-age=31536000; includesubdomains; preload") -
                        1 } };
      return true;
    case 93:
      *header = { { "timing-allow-origin", sizeof("timing-allow-origin") - 1 },
                  { "*", sizeof("*") - 1 } };
      return true;
    case 94:
      *header = { { "upgrade-insecure-requests",
                    sizeof("upgrade-insecure-requests") - 1 },
                  { "1", sizeof("1") - 1 } };
      return true;
    case 59:
      *header = { { "vary", sizeof("vary") - 1 },
                  { "accept-encoding", sizeof("accept-encoding") - 1 } };
      return true;
    case 60:
      *header = { { "vary", sizeof("vary") - 1 },
                  { "origin", sizeof("origin") - 1 } };
      return true;
    case 61:
      *header = { { "x-content-type-options",
                    sizeof("x-content-type-options") - 1 },
                  { "nosniff", sizeof("nosniff") - 1 } };
      return true;
    case 97:
      *header = { { "x-frame-options", sizeof("x-frame-options") - 1 },
                  { "deny", sizeof("deny") - 1 } };
      return true;
    case 98:
      *header = { { "x-frame-options", sizeof("x-frame-options") - 1 },
                  { "sameorigin", sizeof("sameorigin") - 1 } };
      return true;
    case 62:
      *header = { { "x-xss-protection", sizeof("x-xss-protection") - 1 },
                  { "1; mode=block", sizeof("1; mode=block") - 1 } };
      return true;
  }

  return false;
}

bool find_header_only(uint8_t index, header *header)
{
  switch (index) {
    case 0:
      *header = { { ":authority", sizeof(":authority") - 1 }, { nullptr, 0 } };
      return true;
    case 15:
      *header = { { ":method", sizeof(":method") - 1 }, { nullptr, 0 } };
      return true;
    case 16:
      *header = { { ":method", sizeof(":method") - 1 }, { nullptr, 0 } };
      return true;
    case 17:
      *header = { { ":method", sizeof(":method") - 1 }, { nullptr, 0 } };
      return true;
    case 18:
      *header = { { ":method", sizeof(":method") - 1 }, { nullptr, 0 } };
      return true;
    case 19:
      *header = { { ":method", sizeof(":method") - 1 }, { nullptr, 0 } };
      return true;
    case 20:
      *header = { { ":method", sizeof(":method") - 1 }, { nullptr, 0 } };
      return true;
    case 21:
      *header = { { ":method", sizeof(":method") - 1 }, { nullptr, 0 } };
      return true;
    case 1:
      *header = { { ":path", sizeof(":path") - 1 }, { nullptr, 0 } };
      return true;
    case 22:
      *header = { { ":scheme", sizeof(":scheme") - 1 }, { nullptr, 0 } };
      return true;
    case 23:
      *header = { { ":scheme", sizeof(":scheme") - 1 }, { nullptr, 0 } };
      return true;
    case 24:
      *header = { { ":status", sizeof(":status") - 1 }, { nullptr, 0 } };
      return true;
    case 25:
      *header = { { ":status", sizeof(":status") - 1 }, { nullptr, 0 } };
      return true;
    case 26:
      *header = { { ":status", sizeof(":status") - 1 }, { nullptr, 0 } };
      return true;
    case 27:
      *header = { { ":status", sizeof(":status") - 1 }, { nullptr, 0 } };
      return true;
    case 28:
      *header = { { ":status", sizeof(":status") - 1 }, { nullptr, 0 } };
      return true;
    case 63:
      *header = { { ":status", sizeof(":status") - 1 }, { nullptr, 0 } };
      return true;
    case 64:
      *header = { { ":status", sizeof(":status") - 1 }, { nullptr, 0 } };
      return true;
    case 65:
      *header = { { ":status", sizeof(":status") - 1 }, { nullptr, 0 } };
      return true;
    case 66:
      *header = { { ":status", sizeof(":status") - 1 }, { nullptr, 0 } };
      return true;
    case 67:
      *header = { { ":status", sizeof(":status") - 1 }, { nullptr, 0 } };
      return true;
    case 68:
      *header = { { ":status", sizeof(":status") - 1 }, { nullptr, 0 } };
      return true;
    case 69:
      *header = { { ":status", sizeof(":status") - 1 }, { nullptr, 0 } };
      return true;
    case 70:
      *header = { { ":status", sizeof(":status") - 1 }, { nullptr, 0 } };
      return true;
    case 71:
      *header = { { ":status", sizeof(":status") - 1 }, { nullptr, 0 } };
      return true;
    case 29:
      *header = { { "accept", sizeof("accept") - 1 }, { nullptr, 0 } };
      return true;
    case 30:
      *header = { { "accept", sizeof("accept") - 1 }, { nullptr, 0 } };
      return true;
    case 31:
      *header = { { "accept-encoding", sizeof("accept-encoding") - 1 },
                  { nullptr, 0 } };
      return true;
    case 72:
      *header = { { "accept-language", sizeof("accept-language") - 1 },
                  { nullptr, 0 } };
      return true;
    case 32:
      *header = { { "accept-ranges", sizeof("accept-ranges") - 1 },
                  { nullptr, 0 } };
      return true;
    case 73:
      *header = { { "access-control-allow-credentials",
                    sizeof("access-control-allow-credentials") - 1 },
                  { nullptr, 0 } };
      return true;
    case 74:
      *header = { { "access-control-allow-credentials",
                    sizeof("access-control-allow-credentials") - 1 },
                  { nullptr, 0 } };
      return true;
    case 33:
      *header = { { "access-control-allow-headers",
                    sizeof("access-control-allow-headers") - 1 },
                  { nullptr, 0 } };
      return true;
    case 34:
      *header = { { "access-control-allow-headers",
                    sizeof("access-control-allow-headers") - 1 },
                  { nullptr, 0 } };
      return true;
    case 75:
      *header = { { "access-control-allow-headers",
                    sizeof("access-control-allow-headers") - 1 },
                  { nullptr, 0 } };
      return true;
    case 76:
      *header = { { "access-control-allow-methods",
                    sizeof("access-control-allow-methods") - 1 },
                  { nullptr, 0 } };
      return true;
    case 77:
      *header = { { "access-control-allow-methods",
                    sizeof("access-control-allow-methods") - 1 },
                  { nullptr, 0 } };
      return true;
    case 78:
      *header = { { "access-control-allow-methods",
                    sizeof("access-control-allow-methods") - 1 },
                  { nullptr, 0 } };
      return true;
    case 35:
      *header = { { "access-control-allow-origin",
                    sizeof("access-control-allow-origin") - 1 },
                  { nullptr, 0 } };
      return true;
    case 79:
      *header = { { "access-control-expose-headers",
                    sizeof("access-control-expose-headers") - 1 },
                  { nullptr, 0 } };
      return true;
    case 80:
      *header = { { "access-control-request-headers",
                    sizeof("access-control-request-headers") - 1 },
                  { nullptr, 0 } };
      return true;
    case 81:
      *header = { { "access-control-request-method",
                    sizeof("access-control-request-method") - 1 },
                  { nullptr, 0 } };
      return true;
    case 82:
      *header = { { "access-control-request-method",
                    sizeof("access-control-request-method") - 1 },
                  { nullptr, 0 } };
      return true;
    case 2:
      *header = { { "age", sizeof("age") - 1 }, { nullptr, 0 } };
      return true;
    case 83:
      *header = { { "alt-svc", sizeof("alt-svc") - 1 }, { nullptr, 0 } };
      return true;
    case 84:
      *header = { { "authorization", sizeof("authorization") - 1 },
                  { nullptr, 0 } };
      return true;
    case 36:
      *header = { { "cache-control", sizeof("cache-control") - 1 },
                  { nullptr, 0 } };
      return true;
    case 37:
      *header = { { "cache-control", sizeof("cache-control") - 1 },
                  { nullptr, 0 } };
      return true;
    case 38:
      *header = { { "cache-control", sizeof("cache-control") - 1 },
                  { nullptr, 0 } };
      return true;
    case 39:
      *header = { { "cache-control", sizeof("cache-control") - 1 },
                  { nullptr, 0 } };
      return true;
    case 40:
      *header = { { "cache-control", sizeof("cache-control") - 1 },
                  { nullptr, 0 } };
      return true;
    case 41:
      *header = { { "cache-control", sizeof("cache-control") - 1 },
                  { nullptr, 0 } };
      return true;
    case 3:
      *header = { { "content-disposition", sizeof("content-disposition") - 1 },
                  { nullptr, 0 } };
      return true;
    case 42:
      *header = { { "content-encoding", sizeof("content-encoding") - 1 },
                  { nullptr, 0 } };
      return true;
    case 43:
      *header = { { "content-encoding", sizeof("content-encoding") - 1 },
                  { nullptr, 0 } };
      return true;
    case 4:
      *header = { { "content-length", sizeof("content-length") - 1 },
                  { nullptr, 0 } };
      return true;
    case 85:
      *header = { { "content-security-policy",
                    sizeof("content-security-policy") - 1 },
                  { nullptr, 0 } };
      return true;
    case 44:
      *header = { { "content-type", sizeof("content-type") - 1 },
                  { nullptr, 0 } };
      return true;
    case 45:
      *header = { { "content-type", sizeof("content-type") - 1 },
                  { nullptr, 0 } };
      return true;
    case 46:
      *header = { { "content-type", sizeof("content-type") - 1 },
                  { nullptr, 0 } };
      return true;
    case 47:
      *header = { { "content-type", sizeof("content-type") - 1 },
                  { nullptr, 0 } };
      return true;
    case 48:
      *header = { { "content-type", sizeof("content-type") - 1 },
                  { nullptr, 0 } };
      return true;
    case 49:
      *header = { { "content-type", sizeof("content-type") - 1 },
                  { nullptr, 0 } };
      return true;
    case 50:
      *header = { { "content-type", sizeof("content-type") - 1 },
                  { nullptr, 0 } };
      return true;
    case 51:
      *header = { { "content-type", sizeof("content-type") - 1 },
                  { nullptr, 0 } };
      return true;
    case 52:
      *header = { { "content-type", sizeof("content-type") - 1 },
                  { nullptr, 0 } };
      return true;
    case 53:
      *header = { { "content-type", sizeof("content-type") - 1 },
                  { nullptr, 0 } };
      return true;
    case 54:
      *header = { { "content-type", sizeof("content-type") - 1 },
                  { nullptr, 0 } };
      return true;
    case 5:
      *header = { { "cookie", sizeof("cookie") - 1 }, { nullptr, 0 } };
      return true;
    case 6:
      *header = { { "date", sizeof("date") - 1 }, { nullptr, 0 } };
      return true;
    case 86:
      *header = { { "early-data", sizeof("early-data") - 1 }, { nullptr, 0 } };
      return true;
    case 7:
      *header = { { "etag", sizeof("etag") - 1 }, { nullptr, 0 } };
      return true;
    case 87:
      *header = { { "expect-ct", sizeof("expect-ct") - 1 }, { nullptr, 0 } };
      return true;
    case 88:
      *header = { { "forwarded", sizeof("forwarded") - 1 }, { nullptr, 0 } };
      return true;
    case 8:
      *header = { { "if-modified-since", sizeof("if-modified-since") - 1 },
                  { nullptr, 0 } };
      return true;
    case 9:
      *header = { { "if-none-match", sizeof("if-none-match") - 1 },
                  { nullptr, 0 } };
      return true;
    case 89:
      *header = { { "if-range", sizeof("if-range") - 1 }, { nullptr, 0 } };
      return true;
    case 10:
      *header = { { "last-modified", sizeof("last-modified") - 1 },
                  { nullptr, 0 } };
      return true;
    case 11:
      *header = { { "link", sizeof("link") - 1 }, { nullptr, 0 } };
      return true;
    case 12:
      *header = { { "location", sizeof("location") - 1 }, { nullptr, 0 } };
      return true;
    case 90:
      *header = { { "origin", sizeof("origin") - 1 }, { nullptr, 0 } };
      return true;
    case 91:
      *header = { { "purpose", sizeof("purpose") - 1 }, { nullptr, 0 } };
      return true;
    case 55:
      *header = { { "range", sizeof("range") - 1 }, { nullptr, 0 } };
      return true;
    case 13:
      *header = { { "referer", sizeof("referer") - 1 }, { nullptr, 0 } };
      return true;
    case 92:
      *header = { { "server", sizeof("server") - 1 }, { nullptr, 0 } };
      return true;
    case 14:
      *header = { { "set-cookie", sizeof("set-cookie") - 1 }, { nullptr, 0 } };
      return true;
    case 56:
      *header = { { "strict-transport-security",
                    sizeof("strict-transport-security") - 1 },
                  { nullptr, 0 } };
      return true;
    case 57:
      *header = { { "strict-transport-security",
                    sizeof("strict-transport-security") - 1 },
                  { nullptr, 0 } };
      return true;
    case 58:
      *header = { { "strict-transport-security",
                    sizeof("strict-transport-security") - 1 },
                  { nullptr, 0 } };
      return true;
    case 93:
      *header = { { "timing-allow-origin", sizeof("timing-allow-origin") - 1 },
                  { nullptr, 0 } };
      return true;
    case 94:
      *header = { { "upgrade-insecure-requests",
                    sizeof("upgrade-insecure-requests") - 1 },
                  { nullptr, 0 } };
      return true;
    case 95:
      *header = { { "user-agent", sizeof("user-agent") - 1 }, { nullptr, 0 } };
      return true;
    case 59:
      *header = { { "vary", sizeof("vary") - 1 }, { nullptr, 0 } };
      return true;
    case 60:
      *header = { { "vary", sizeof("vary") - 1 }, { nullptr, 0 } };
      return true;
    case 61:
      *header = { { "x-content-type-options",
                    sizeof("x-content-type-options") - 1 },
                  { nullptr, 0 } };
      return true;
    case 96:
      *header = { { "x-forwarded-for", sizeof("x-forwarded-for") - 1 },
                  { nullptr, 0 } };
      return true;
    case 97:
      *header = { { "x-frame-options", sizeof("x-frame-options") - 1 },
                  { nullptr, 0 } };
      return true;
    case 98:
      *header = { { "x-frame-options", sizeof("x-frame-options") - 1 },
                  { nullptr, 0 } };
      return true;
    case 62:
      *header = { { "x-xss-protection", sizeof("x-xss-protection") - 1 },
                  { nullptr, 0 } };
      return true;
  }

  return false;
}

} // namespace static_table
} // namespace qpack
} // namespace h3c
