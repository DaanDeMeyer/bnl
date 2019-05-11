#include <h3c/header.h>

#include <stdbool.h>
#include <stdint.h>


bool static_table_find_header_value(uint8_t index, h3c_header_t *header)
{
  switch (index) {
    case 15:
      *header = (h3c_header_t){
        .name = { .data = ":method", .length = sizeof(":method") - 1 },
        .value = { .data = "CONNECT", .length = sizeof("CONNECT") - 1 }
      };
      return true;
    case 16:
      *header = (h3c_header_t){
        .name = { .data = ":method", .length = sizeof(":method") - 1 },
        .value = { .data = "DELETE", .length = sizeof("DELETE") - 1 }
      };
      return true;
    case 17:
      *header = (h3c_header_t){
        .name = { .data = ":method", .length = sizeof(":method") - 1 },
        .value = { .data = "GET", .length = sizeof("GET") - 1 }
      };
      return true;
    case 18:
      *header = (h3c_header_t){
        .name = { .data = ":method", .length = sizeof(":method") - 1 },
        .value = { .data = "HEAD", .length = sizeof("HEAD") - 1 }
      };
      return true;
    case 19:
      *header = (h3c_header_t){
        .name = { .data = ":method", .length = sizeof(":method") - 1 },
        .value = { .data = "OPTIONS", .length = sizeof("OPTIONS") - 1 }
      };
      return true;
    case 20:
      *header = (h3c_header_t){
        .name = { .data = ":method", .length = sizeof(":method") - 1 },
        .value = { .data = "POST", .length = sizeof("POST") - 1 }
      };
      return true;
    case 21:
      *header = (h3c_header_t){
        .name = { .data = ":method", .length = sizeof(":method") - 1 },
        .value = { .data = "PUT", .length = sizeof("PUT") - 1 }
      };
      return true;
    case 1:
      *header = (h3c_header_t){
        .name = { .data = ":path", .length = sizeof(":path") - 1 },
        .value = { .data = "/", .length = sizeof("/") - 1 }
      };
      return true;
    case 22:
      *header = (h3c_header_t){
        .name = { .data = ":scheme", .length = sizeof(":scheme") - 1 },
        .value = { .data = "http", .length = sizeof("http") - 1 }
      };
      return true;
    case 23:
      *header = (h3c_header_t){
        .name = { .data = ":scheme", .length = sizeof(":scheme") - 1 },
        .value = { .data = "https", .length = sizeof("https") - 1 }
      };
      return true;
    case 24:
      *header = (h3c_header_t){
        .name = { .data = ":status", .length = sizeof(":status") - 1 },
        .value = { .data = "103", .length = sizeof("103") - 1 }
      };
      return true;
    case 25:
      *header = (h3c_header_t){
        .name = { .data = ":status", .length = sizeof(":status") - 1 },
        .value = { .data = "200", .length = sizeof("200") - 1 }
      };
      return true;
    case 26:
      *header = (h3c_header_t){
        .name = { .data = ":status", .length = sizeof(":status") - 1 },
        .value = { .data = "304", .length = sizeof("304") - 1 }
      };
      return true;
    case 27:
      *header = (h3c_header_t){
        .name = { .data = ":status", .length = sizeof(":status") - 1 },
        .value = { .data = "404", .length = sizeof("404") - 1 }
      };
      return true;
    case 28:
      *header = (h3c_header_t){
        .name = { .data = ":status", .length = sizeof(":status") - 1 },
        .value = { .data = "503", .length = sizeof("503") - 1 }
      };
      return true;
    case 63:
      *header = (h3c_header_t){
        .name = { .data = ":status", .length = sizeof(":status") - 1 },
        .value = { .data = "100", .length = sizeof("100") - 1 }
      };
      return true;
    case 64:
      *header = (h3c_header_t){
        .name = { .data = ":status", .length = sizeof(":status") - 1 },
        .value = { .data = "204", .length = sizeof("204") - 1 }
      };
      return true;
    case 65:
      *header = (h3c_header_t){
        .name = { .data = ":status", .length = sizeof(":status") - 1 },
        .value = { .data = "206", .length = sizeof("206") - 1 }
      };
      return true;
    case 66:
      *header = (h3c_header_t){
        .name = { .data = ":status", .length = sizeof(":status") - 1 },
        .value = { .data = "302", .length = sizeof("302") - 1 }
      };
      return true;
    case 67:
      *header = (h3c_header_t){
        .name = { .data = ":status", .length = sizeof(":status") - 1 },
        .value = { .data = "400", .length = sizeof("400") - 1 }
      };
      return true;
    case 68:
      *header = (h3c_header_t){
        .name = { .data = ":status", .length = sizeof(":status") - 1 },
        .value = { .data = "403", .length = sizeof("403") - 1 }
      };
      return true;
    case 69:
      *header = (h3c_header_t){
        .name = { .data = ":status", .length = sizeof(":status") - 1 },
        .value = { .data = "421", .length = sizeof("421") - 1 }
      };
      return true;
    case 70:
      *header = (h3c_header_t){
        .name = { .data = ":status", .length = sizeof(":status") - 1 },
        .value = { .data = "425", .length = sizeof("425") - 1 }
      };
      return true;
    case 71:
      *header = (h3c_header_t){
        .name = { .data = ":status", .length = sizeof(":status") - 1 },
        .value = { .data = "500", .length = sizeof("500") - 1 }
      };
      return true;
    case 29:
      *header = (h3c_header_t){
        .name = { .data = "accept", .length = sizeof("accept") - 1 },
        .value = { .data = "*/*", .length = sizeof("*/*") - 1 }
      };
      return true;
    case 30:
      *header = (h3c_header_t){
        .name = { .data = "accept", .length = sizeof("accept") - 1 },
        .value = { .data = "application/dns-message",
                   .length = sizeof("application/dns-message") - 1 }
      };
      return true;
    case 31:
      *header = (h3c_header_t){
        .name = { .data = "accept-encoding",
                  .length = sizeof("accept-encoding") - 1 },
        .value = { .data = "gzip, deflate, br",
                   .length = sizeof("gzip, deflate, br") - 1 }
      };
      return true;
    case 32:
      *header = (h3c_header_t){
        .name = { .data = "accept-ranges",
                  .length = sizeof("accept-ranges") - 1 },
        .value = { .data = "bytes", .length = sizeof("bytes") - 1 }
      };
      return true;
    case 73:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-credentials",
                  .length = sizeof("access-control-allow-credentials") - 1 },
        .value = { .data = "FALSE", .length = sizeof("FALSE") - 1 }
      };
      return true;
    case 74:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-credentials",
                  .length = sizeof("access-control-allow-credentials") - 1 },
        .value = { .data = "TRUE", .length = sizeof("TRUE") - 1 }
      };
      return true;
    case 33:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-headers",
                  .length = sizeof("access-control-allow-headers") - 1 },
        .value = { .data = "cache-control",
                   .length = sizeof("cache-control") - 1 }
      };
      return true;
    case 34:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-headers",
                  .length = sizeof("access-control-allow-headers") - 1 },
        .value = { .data = "content-type",
                   .length = sizeof("content-type") - 1 }
      };
      return true;
    case 75:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-headers",
                  .length = sizeof("access-control-allow-headers") - 1 },
        .value = { .data = "*", .length = sizeof("*") - 1 }
      };
      return true;
    case 76:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-methods",
                  .length = sizeof("access-control-allow-methods") - 1 },
        .value = { .data = "get", .length = sizeof("get") - 1 }
      };
      return true;
    case 77:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-methods",
                  .length = sizeof("access-control-allow-methods") - 1 },
        .value = { .data = "get, post, options",
                   .length = sizeof("get, post, options") - 1 }
      };
      return true;
    case 78:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-methods",
                  .length = sizeof("access-control-allow-methods") - 1 },
        .value = { .data = "options", .length = sizeof("options") - 1 }
      };
      return true;
    case 35:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-origin",
                  .length = sizeof("access-control-allow-origin") - 1 },
        .value = { .data = "*", .length = sizeof("*") - 1 }
      };
      return true;
    case 79:
      *header = (h3c_header_t){
        .name = { .data = "access-control-expose-headers",
                  .length = sizeof("access-control-expose-headers") - 1 },
        .value = { .data = "content-length",
                   .length = sizeof("content-length") - 1 }
      };
      return true;
    case 80:
      *header = (h3c_header_t){
        .name = { .data = "access-control-request-headers",
                  .length = sizeof("access-control-request-headers") - 1 },
        .value = { .data = "content-type",
                   .length = sizeof("content-type") - 1 }
      };
      return true;
    case 81:
      *header = (h3c_header_t){
        .name = { .data = "access-control-request-method",
                  .length = sizeof("access-control-request-method") - 1 },
        .value = { .data = "get", .length = sizeof("get") - 1 }
      };
      return true;
    case 82:
      *header = (h3c_header_t){
        .name = { .data = "access-control-request-method",
                  .length = sizeof("access-control-request-method") - 1 },
        .value = { .data = "post", .length = sizeof("post") - 1 }
      };
      return true;
    case 2:
      *header = (h3c_header_t){
        .name = { .data = "age", .length = sizeof("age") - 1 },
        .value = { .data = "0", .length = sizeof("0") - 1 }
      };
      return true;
    case 83:
      *header = (h3c_header_t){
        .name = { .data = "alt-svc", .length = sizeof("alt-svc") - 1 },
        .value = { .data = "clear", .length = sizeof("clear") - 1 }
      };
      return true;
    case 36:
      *header = (h3c_header_t){
        .name = { .data = "cache-control",
                  .length = sizeof("cache-control") - 1 },
        .value = { .data = "max-age=0", .length = sizeof("max-age=0") - 1 }
      };
      return true;
    case 37:
      *header = (h3c_header_t){
        .name = { .data = "cache-control",
                  .length = sizeof("cache-control") - 1 },
        .value = { .data = "max-age=2592000",
                   .length = sizeof("max-age=2592000") - 1 }
      };
      return true;
    case 38:
      *header = (h3c_header_t){
        .name = { .data = "cache-control",
                  .length = sizeof("cache-control") - 1 },
        .value = { .data = "max-age=604800",
                   .length = sizeof("max-age=604800") - 1 }
      };
      return true;
    case 39:
      *header = (h3c_header_t){
        .name = { .data = "cache-control",
                  .length = sizeof("cache-control") - 1 },
        .value = { .data = "no-cache", .length = sizeof("no-cache") - 1 }
      };
      return true;
    case 40:
      *header = (h3c_header_t){
        .name = { .data = "cache-control",
                  .length = sizeof("cache-control") - 1 },
        .value = { .data = "no-store", .length = sizeof("no-store") - 1 }
      };
      return true;
    case 41:
      *header = (h3c_header_t){
        .name = { .data = "cache-control",
                  .length = sizeof("cache-control") - 1 },
        .value = { .data = "public, max-age=31536000",
                   .length = sizeof("public, max-age=31536000") - 1 }
      };
      return true;
    case 42:
      *header = (h3c_header_t){
        .name = { .data = "content-encoding",
                  .length = sizeof("content-encoding") - 1 },
        .value = { .data = "br", .length = sizeof("br") - 1 }
      };
      return true;
    case 43:
      *header = (h3c_header_t){
        .name = { .data = "content-encoding",
                  .length = sizeof("content-encoding") - 1 },
        .value = { .data = "gzip", .length = sizeof("gzip") - 1 }
      };
      return true;
    case 4:
      *header = (h3c_header_t){
        .name = { .data = "content-length",
                  .length = sizeof("content-length") - 1 },
        .value = { .data = "0", .length = sizeof("0") - 1 }
      };
      return true;
    case 85:
      *header = (h3c_header_t){
        .name = { .data = "content-security-policy",
                  .length = sizeof("content-security-policy") - 1 },
        .value = { .data =
                       "script-src 'none'; object-src 'none'; base-uri 'none'",
                   .length = sizeof("script-src 'none'; object-src 'none'; "
                                    "base-uri 'none'") -
                             1 }
      };
      return true;
    case 44:
      *header = (h3c_header_t){
        .name = { .data = "content-type",
                  .length = sizeof("content-type") - 1 },
        .value = { .data = "application/dns-message",
                   .length = sizeof("application/dns-message") - 1 }
      };
      return true;
    case 45:
      *header = (h3c_header_t){
        .name = { .data = "content-type",
                  .length = sizeof("content-type") - 1 },
        .value = { .data = "application/javascript",
                   .length = sizeof("application/javascript") - 1 }
      };
      return true;
    case 46:
      *header = (h3c_header_t){
        .name = { .data = "content-type",
                  .length = sizeof("content-type") - 1 },
        .value = { .data = "application/json",
                   .length = sizeof("application/json") - 1 }
      };
      return true;
    case 47:
      *header = (h3c_header_t){
        .name = { .data = "content-type",
                  .length = sizeof("content-type") - 1 },
        .value = { .data = "application/x-www-form-urlencoded",
                   .length = sizeof("application/x-www-form-urlencoded") - 1 }
      };
      return true;
    case 48:
      *header = (h3c_header_t){
        .name = { .data = "content-type",
                  .length = sizeof("content-type") - 1 },
        .value = { .data = "image/gif", .length = sizeof("image/gif") - 1 }
      };
      return true;
    case 49:
      *header = (h3c_header_t){
        .name = { .data = "content-type",
                  .length = sizeof("content-type") - 1 },
        .value = { .data = "image/jpeg", .length = sizeof("image/jpeg") - 1 }
      };
      return true;
    case 50:
      *header = (h3c_header_t){
        .name = { .data = "content-type",
                  .length = sizeof("content-type") - 1 },
        .value = { .data = "image/png", .length = sizeof("image/png") - 1 }
      };
      return true;
    case 51:
      *header = (h3c_header_t){
        .name = { .data = "content-type",
                  .length = sizeof("content-type") - 1 },
        .value = { .data = "text/css", .length = sizeof("text/css") - 1 }
      };
      return true;
    case 52:
      *header = (h3c_header_t){
        .name = { .data = "content-type",
                  .length = sizeof("content-type") - 1 },
        .value = { .data = "text/html; charset=utf-8",
                   .length = sizeof("text/html; charset=utf-8") - 1 }
      };
      return true;
    case 53:
      *header = (h3c_header_t){
        .name = { .data = "content-type",
                  .length = sizeof("content-type") - 1 },
        .value = { .data = "text/plain", .length = sizeof("text/plain") - 1 }
      };
      return true;
    case 54:
      *header = (h3c_header_t){
        .name = { .data = "content-type",
                  .length = sizeof("content-type") - 1 },
        .value = { .data = "text/plain;charset=utf-8",
                   .length = sizeof("text/plain;charset=utf-8") - 1 }
      };
      return true;
    case 86:
      *header = (h3c_header_t){
        .name = { .data = "early-data", .length = sizeof("early-data") - 1 },
        .value = { .data = "1", .length = sizeof("1") - 1 }
      };
      return true;
    case 91:
      *header = (h3c_header_t){
        .name = { .data = "purpose", .length = sizeof("purpose") - 1 },
        .value = { .data = "prefetch", .length = sizeof("prefetch") - 1 }
      };
      return true;
    case 55:
      *header = (h3c_header_t){
        .name = { .data = "range", .length = sizeof("range") - 1 },
        .value = { .data = "bytes=0-", .length = sizeof("bytes=0-") - 1 }
      };
      return true;
    case 56:
      *header = (h3c_header_t){
        .name = { .data = "strict-transport-security",
                  .length = sizeof("strict-transport-security") - 1 },
        .value = { .data = "max-age=31536000",
                   .length = sizeof("max-age=31536000") - 1 }
      };
      return true;
    case 57:
      *header = (h3c_header_t){
        .name = { .data = "strict-transport-security",
                  .length = sizeof("strict-transport-security") - 1 },
        .value = { .data = "max-age=31536000; includesubdomains",
                   .length = sizeof("max-age=31536000; includesubdomains") - 1 }
      };
      return true;
    case 58:
      *header = (h3c_header_t){
        .name = { .data = "strict-transport-security",
                  .length = sizeof("strict-transport-security") - 1 },
        .value = { .data = "max-age=31536000; includesubdomains; preload",
                   .length =
                       sizeof("max-age=31536000; includesubdomains; preload") -
                       1 }
      };
      return true;
    case 93:
      *header = (h3c_header_t){
        .name = { .data = "timing-allow-origin",
                  .length = sizeof("timing-allow-origin") - 1 },
        .value = { .data = "*", .length = sizeof("*") - 1 }
      };
      return true;
    case 94:
      *header = (h3c_header_t){
        .name = { .data = "upgrade-insecure-requests",
                  .length = sizeof("upgrade-insecure-requests") - 1 },
        .value = { .data = "1", .length = sizeof("1") - 1 }
      };
      return true;
    case 59:
      *header = (h3c_header_t){
        .name = { .data = "vary", .length = sizeof("vary") - 1 },
        .value = { .data = "accept-encoding",
                   .length = sizeof("accept-encoding") - 1 }
      };
      return true;
    case 60:
      *header = (h3c_header_t){
        .name = { .data = "vary", .length = sizeof("vary") - 1 },
        .value = { .data = "origin", .length = sizeof("origin") - 1 }
      };
      return true;
    case 61:
      *header = (h3c_header_t){
        .name = { .data = "x-content-type-options",
                  .length = sizeof("x-content-type-options") - 1 },
        .value = { .data = "nosniff", .length = sizeof("nosniff") - 1 }
      };
      return true;
    case 97:
      *header = (h3c_header_t){
        .name = { .data = "x-frame-options",
                  .length = sizeof("x-frame-options") - 1 },
        .value = { .data = "deny", .length = sizeof("deny") - 1 }
      };
      return true;
    case 98:
      *header = (h3c_header_t){
        .name = { .data = "x-frame-options",
                  .length = sizeof("x-frame-options") - 1 },
        .value = { .data = "sameorigin", .length = sizeof("sameorigin") - 1 }
      };
      return true;
    case 62:
      *header = (h3c_header_t){
        .name = { .data = "x-xss-protection",
                  .length = sizeof("x-xss-protection") - 1 },
        .value = { .data = "1; mode=block",
                   .length = sizeof("1; mode=block") - 1 }
      };
      return true;
  }

  return false;
}

bool static_table_find_header_only(uint8_t index, h3c_header_t *header)
{
  switch (index) {
    case 0:
      *header = (h3c_header_t){
        .name = { .data = ":authority", .length = sizeof(":authority") - 1 }
      };
      return true;
    case 15:
      *header = (h3c_header_t){ .name = { .data = ":method",
                                          .length = sizeof(":method") - 1 } };
      return true;
    case 16:
      *header = (h3c_header_t){ .name = { .data = ":method",
                                          .length = sizeof(":method") - 1 } };
      return true;
    case 17:
      *header = (h3c_header_t){ .name = { .data = ":method",
                                          .length = sizeof(":method") - 1 } };
      return true;
    case 18:
      *header = (h3c_header_t){ .name = { .data = ":method",
                                          .length = sizeof(":method") - 1 } };
      return true;
    case 19:
      *header = (h3c_header_t){ .name = { .data = ":method",
                                          .length = sizeof(":method") - 1 } };
      return true;
    case 20:
      *header = (h3c_header_t){ .name = { .data = ":method",
                                          .length = sizeof(":method") - 1 } };
      return true;
    case 21:
      *header = (h3c_header_t){ .name = { .data = ":method",
                                          .length = sizeof(":method") - 1 } };
      return true;
    case 1:
      *header = (h3c_header_t){ .name = { .data = ":path",
                                          .length = sizeof(":path") - 1 } };
      return true;
    case 22:
      *header = (h3c_header_t){ .name = { .data = ":scheme",
                                          .length = sizeof(":scheme") - 1 } };
      return true;
    case 23:
      *header = (h3c_header_t){ .name = { .data = ":scheme",
                                          .length = sizeof(":scheme") - 1 } };
      return true;
    case 24:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .length = sizeof(":status") - 1 } };
      return true;
    case 25:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .length = sizeof(":status") - 1 } };
      return true;
    case 26:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .length = sizeof(":status") - 1 } };
      return true;
    case 27:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .length = sizeof(":status") - 1 } };
      return true;
    case 28:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .length = sizeof(":status") - 1 } };
      return true;
    case 63:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .length = sizeof(":status") - 1 } };
      return true;
    case 64:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .length = sizeof(":status") - 1 } };
      return true;
    case 65:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .length = sizeof(":status") - 1 } };
      return true;
    case 66:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .length = sizeof(":status") - 1 } };
      return true;
    case 67:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .length = sizeof(":status") - 1 } };
      return true;
    case 68:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .length = sizeof(":status") - 1 } };
      return true;
    case 69:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .length = sizeof(":status") - 1 } };
      return true;
    case 70:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .length = sizeof(":status") - 1 } };
      return true;
    case 71:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .length = sizeof(":status") - 1 } };
      return true;
    case 29:
      *header = (h3c_header_t){ .name = { .data = "accept",
                                          .length = sizeof("accept") - 1 } };
      return true;
    case 30:
      *header = (h3c_header_t){ .name = { .data = "accept",
                                          .length = sizeof("accept") - 1 } };
      return true;
    case 31:
      *header = (h3c_header_t){ .name = { .data = "accept-encoding",
                                          .length = sizeof("accept-encoding") -
                                                    1 } };
      return true;
    case 72:
      *header = (h3c_header_t){ .name = { .data = "accept-language",
                                          .length = sizeof("accept-language") -
                                                    1 } };
      return true;
    case 32:
      *header = (h3c_header_t){ .name = { .data = "accept-ranges",
                                          .length = sizeof("accept-ranges") -
                                                    1 } };
      return true;
    case 73:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-credentials",
                  .length = sizeof("access-control-allow-credentials") - 1 }
      };
      return true;
    case 74:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-credentials",
                  .length = sizeof("access-control-allow-credentials") - 1 }
      };
      return true;
    case 33:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-headers",
                  .length = sizeof("access-control-allow-headers") - 1 }
      };
      return true;
    case 34:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-headers",
                  .length = sizeof("access-control-allow-headers") - 1 }
      };
      return true;
    case 75:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-headers",
                  .length = sizeof("access-control-allow-headers") - 1 }
      };
      return true;
    case 76:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-methods",
                  .length = sizeof("access-control-allow-methods") - 1 }
      };
      return true;
    case 77:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-methods",
                  .length = sizeof("access-control-allow-methods") - 1 }
      };
      return true;
    case 78:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-methods",
                  .length = sizeof("access-control-allow-methods") - 1 }
      };
      return true;
    case 35:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-origin",
                  .length = sizeof("access-control-allow-origin") - 1 }
      };
      return true;
    case 79:
      *header = (h3c_header_t){
        .name = { .data = "access-control-expose-headers",
                  .length = sizeof("access-control-expose-headers") - 1 }
      };
      return true;
    case 80:
      *header = (h3c_header_t){
        .name = { .data = "access-control-request-headers",
                  .length = sizeof("access-control-request-headers") - 1 }
      };
      return true;
    case 81:
      *header = (h3c_header_t){
        .name = { .data = "access-control-request-method",
                  .length = sizeof("access-control-request-method") - 1 }
      };
      return true;
    case 82:
      *header = (h3c_header_t){
        .name = { .data = "access-control-request-method",
                  .length = sizeof("access-control-request-method") - 1 }
      };
      return true;
    case 2:
      *header = (h3c_header_t){ .name = { .data = "age",
                                          .length = sizeof("age") - 1 } };
      return true;
    case 83:
      *header = (h3c_header_t){ .name = { .data = "alt-svc",
                                          .length = sizeof("alt-svc") - 1 } };
      return true;
    case 84:
      *header = (h3c_header_t){ .name = { .data = "authorization",
                                          .length = sizeof("authorization") -
                                                    1 } };
      return true;
    case 36:
      *header = (h3c_header_t){ .name = { .data = "cache-control",
                                          .length = sizeof("cache-control") -
                                                    1 } };
      return true;
    case 37:
      *header = (h3c_header_t){ .name = { .data = "cache-control",
                                          .length = sizeof("cache-control") -
                                                    1 } };
      return true;
    case 38:
      *header = (h3c_header_t){ .name = { .data = "cache-control",
                                          .length = sizeof("cache-control") -
                                                    1 } };
      return true;
    case 39:
      *header = (h3c_header_t){ .name = { .data = "cache-control",
                                          .length = sizeof("cache-control") -
                                                    1 } };
      return true;
    case 40:
      *header = (h3c_header_t){ .name = { .data = "cache-control",
                                          .length = sizeof("cache-control") -
                                                    1 } };
      return true;
    case 41:
      *header = (h3c_header_t){ .name = { .data = "cache-control",
                                          .length = sizeof("cache-control") -
                                                    1 } };
      return true;
    case 3:
      *header = (h3c_header_t){
        .name = { .data = "content-disposition",
                  .length = sizeof("content-disposition") - 1 }
      };
      return true;
    case 42:
      *header = (h3c_header_t){ .name = { .data = "content-encoding",
                                          .length = sizeof("content-encoding") -
                                                    1 } };
      return true;
    case 43:
      *header = (h3c_header_t){ .name = { .data = "content-encoding",
                                          .length = sizeof("content-encoding") -
                                                    1 } };
      return true;
    case 4:
      *header = (h3c_header_t){ .name = { .data = "content-length",
                                          .length = sizeof("content-length") -
                                                    1 } };
      return true;
    case 85:
      *header = (h3c_header_t){
        .name = { .data = "content-security-policy",
                  .length = sizeof("content-security-policy") - 1 }
      };
      return true;
    case 44:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .length = sizeof("content-type") - 1 }
      };
      return true;
    case 45:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .length = sizeof("content-type") - 1 }
      };
      return true;
    case 46:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .length = sizeof("content-type") - 1 }
      };
      return true;
    case 47:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .length = sizeof("content-type") - 1 }
      };
      return true;
    case 48:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .length = sizeof("content-type") - 1 }
      };
      return true;
    case 49:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .length = sizeof("content-type") - 1 }
      };
      return true;
    case 50:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .length = sizeof("content-type") - 1 }
      };
      return true;
    case 51:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .length = sizeof("content-type") - 1 }
      };
      return true;
    case 52:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .length = sizeof("content-type") - 1 }
      };
      return true;
    case 53:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .length = sizeof("content-type") - 1 }
      };
      return true;
    case 54:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .length = sizeof("content-type") - 1 }
      };
      return true;
    case 5:
      *header = (h3c_header_t){ .name = { .data = "cookie",
                                          .length = sizeof("cookie") - 1 } };
      return true;
    case 6:
      *header = (h3c_header_t){ .name = { .data = "date",
                                          .length = sizeof("date") - 1 } };
      return true;
    case 86:
      *header = (h3c_header_t){
        .name = { .data = "early-data", .length = sizeof("early-data") - 1 }
      };
      return true;
    case 7:
      *header = (h3c_header_t){ .name = { .data = "etag",
                                          .length = sizeof("etag") - 1 } };
      return true;
    case 87:
      *header = (h3c_header_t){ .name = { .data = "expect-ct",
                                          .length = sizeof("expect-ct") - 1 } };
      return true;
    case 88:
      *header = (h3c_header_t){ .name = { .data = "forwarded",
                                          .length = sizeof("forwarded") - 1 } };
      return true;
    case 8:
      *header = (h3c_header_t){
        .name = { .data = "if-modified-since",
                  .length = sizeof("if-modified-since") - 1 }
      };
      return true;
    case 9:
      *header = (h3c_header_t){ .name = { .data = "if-none-match",
                                          .length = sizeof("if-none-match") -
                                                    1 } };
      return true;
    case 89:
      *header = (h3c_header_t){ .name = { .data = "if-range",
                                          .length = sizeof("if-range") - 1 } };
      return true;
    case 10:
      *header = (h3c_header_t){ .name = { .data = "last-modified",
                                          .length = sizeof("last-modified") -
                                                    1 } };
      return true;
    case 11:
      *header = (h3c_header_t){ .name = { .data = "link",
                                          .length = sizeof("link") - 1 } };
      return true;
    case 12:
      *header = (h3c_header_t){ .name = { .data = "location",
                                          .length = sizeof("location") - 1 } };
      return true;
    case 90:
      *header = (h3c_header_t){ .name = { .data = "origin",
                                          .length = sizeof("origin") - 1 } };
      return true;
    case 91:
      *header = (h3c_header_t){ .name = { .data = "purpose",
                                          .length = sizeof("purpose") - 1 } };
      return true;
    case 55:
      *header = (h3c_header_t){ .name = { .data = "range",
                                          .length = sizeof("range") - 1 } };
      return true;
    case 13:
      *header = (h3c_header_t){ .name = { .data = "referer",
                                          .length = sizeof("referer") - 1 } };
      return true;
    case 92:
      *header = (h3c_header_t){ .name = { .data = "server",
                                          .length = sizeof("server") - 1 } };
      return true;
    case 14:
      *header = (h3c_header_t){
        .name = { .data = "set-cookie", .length = sizeof("set-cookie") - 1 }
      };
      return true;
    case 56:
      *header = (h3c_header_t){
        .name = { .data = "strict-transport-security",
                  .length = sizeof("strict-transport-security") - 1 }
      };
      return true;
    case 57:
      *header = (h3c_header_t){
        .name = { .data = "strict-transport-security",
                  .length = sizeof("strict-transport-security") - 1 }
      };
      return true;
    case 58:
      *header = (h3c_header_t){
        .name = { .data = "strict-transport-security",
                  .length = sizeof("strict-transport-security") - 1 }
      };
      return true;
    case 93:
      *header = (h3c_header_t){
        .name = { .data = "timing-allow-origin",
                  .length = sizeof("timing-allow-origin") - 1 }
      };
      return true;
    case 94:
      *header = (h3c_header_t){
        .name = { .data = "upgrade-insecure-requests",
                  .length = sizeof("upgrade-insecure-requests") - 1 }
      };
      return true;
    case 95:
      *header = (h3c_header_t){
        .name = { .data = "user-agent", .length = sizeof("user-agent") - 1 }
      };
      return true;
    case 59:
      *header = (h3c_header_t){ .name = { .data = "vary",
                                          .length = sizeof("vary") - 1 } };
      return true;
    case 60:
      *header = (h3c_header_t){ .name = { .data = "vary",
                                          .length = sizeof("vary") - 1 } };
      return true;
    case 61:
      *header = (h3c_header_t){
        .name = { .data = "x-content-type-options",
                  .length = sizeof("x-content-type-options") - 1 }
      };
      return true;
    case 96:
      *header = (h3c_header_t){ .name = { .data = "x-forwarded-for",
                                          .length = sizeof("x-forwarded-for") -
                                                    1 } };
      return true;
    case 97:
      *header = (h3c_header_t){ .name = { .data = "x-frame-options",
                                          .length = sizeof("x-frame-options") -
                                                    1 } };
      return true;
    case 98:
      *header = (h3c_header_t){ .name = { .data = "x-frame-options",
                                          .length = sizeof("x-frame-options") -
                                                    1 } };
      return true;
    case 62:
      *header = (h3c_header_t){ .name = { .data = "x-xss-protection",
                                          .length = sizeof("x-xss-protection") -
                                                    1 } };
      return true;
  }

  return false;
}
