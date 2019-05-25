#include <h3c/http.h>

#include <stdbool.h>
#include <stdint.h>

bool static_table_find_header_value(uint8_t index, h3c_header_t *header)
{
  switch (index) {
    case 15:
      *header = (h3c_header_t){
        .name = { .data = ":method", .size = sizeof(":method") - 1 },
        .value = { .data = "CONNECT", .size = sizeof("CONNECT") - 1 }
      };
      return true;
    case 16:
      *header = (h3c_header_t){
        .name = { .data = ":method", .size = sizeof(":method") - 1 },
        .value = { .data = "DELETE", .size = sizeof("DELETE") - 1 }
      };
      return true;
    case 17:
      *header = (h3c_header_t){
        .name = { .data = ":method", .size = sizeof(":method") - 1 },
        .value = { .data = "GET", .size = sizeof("GET") - 1 }
      };
      return true;
    case 18:
      *header = (h3c_header_t){
        .name = { .data = ":method", .size = sizeof(":method") - 1 },
        .value = { .data = "HEAD", .size = sizeof("HEAD") - 1 }
      };
      return true;
    case 19:
      *header = (h3c_header_t){
        .name = { .data = ":method", .size = sizeof(":method") - 1 },
        .value = { .data = "OPTIONS", .size = sizeof("OPTIONS") - 1 }
      };
      return true;
    case 20:
      *header = (h3c_header_t){
        .name = { .data = ":method", .size = sizeof(":method") - 1 },
        .value = { .data = "POST", .size = sizeof("POST") - 1 }
      };
      return true;
    case 21:
      *header = (h3c_header_t){
        .name = { .data = ":method", .size = sizeof(":method") - 1 },
        .value = { .data = "PUT", .size = sizeof("PUT") - 1 }
      };
      return true;
    case 1:
      *header = (h3c_header_t){
        .name = { .data = ":path", .size = sizeof(":path") - 1 },
        .value = { .data = "/", .size = sizeof("/") - 1 }
      };
      return true;
    case 22:
      *header = (h3c_header_t){
        .name = { .data = ":scheme", .size = sizeof(":scheme") - 1 },
        .value = { .data = "http", .size = sizeof("http") - 1 }
      };
      return true;
    case 23:
      *header = (h3c_header_t){
        .name = { .data = ":scheme", .size = sizeof(":scheme") - 1 },
        .value = { .data = "https", .size = sizeof("https") - 1 }
      };
      return true;
    case 24:
      *header = (h3c_header_t){
        .name = { .data = ":status", .size = sizeof(":status") - 1 },
        .value = { .data = "103", .size = sizeof("103") - 1 }
      };
      return true;
    case 25:
      *header = (h3c_header_t){
        .name = { .data = ":status", .size = sizeof(":status") - 1 },
        .value = { .data = "200", .size = sizeof("200") - 1 }
      };
      return true;
    case 26:
      *header = (h3c_header_t){
        .name = { .data = ":status", .size = sizeof(":status") - 1 },
        .value = { .data = "304", .size = sizeof("304") - 1 }
      };
      return true;
    case 27:
      *header = (h3c_header_t){
        .name = { .data = ":status", .size = sizeof(":status") - 1 },
        .value = { .data = "404", .size = sizeof("404") - 1 }
      };
      return true;
    case 28:
      *header = (h3c_header_t){
        .name = { .data = ":status", .size = sizeof(":status") - 1 },
        .value = { .data = "503", .size = sizeof("503") - 1 }
      };
      return true;
    case 63:
      *header = (h3c_header_t){
        .name = { .data = ":status", .size = sizeof(":status") - 1 },
        .value = { .data = "100", .size = sizeof("100") - 1 }
      };
      return true;
    case 64:
      *header = (h3c_header_t){
        .name = { .data = ":status", .size = sizeof(":status") - 1 },
        .value = { .data = "204", .size = sizeof("204") - 1 }
      };
      return true;
    case 65:
      *header = (h3c_header_t){
        .name = { .data = ":status", .size = sizeof(":status") - 1 },
        .value = { .data = "206", .size = sizeof("206") - 1 }
      };
      return true;
    case 66:
      *header = (h3c_header_t){
        .name = { .data = ":status", .size = sizeof(":status") - 1 },
        .value = { .data = "302", .size = sizeof("302") - 1 }
      };
      return true;
    case 67:
      *header = (h3c_header_t){
        .name = { .data = ":status", .size = sizeof(":status") - 1 },
        .value = { .data = "400", .size = sizeof("400") - 1 }
      };
      return true;
    case 68:
      *header = (h3c_header_t){
        .name = { .data = ":status", .size = sizeof(":status") - 1 },
        .value = { .data = "403", .size = sizeof("403") - 1 }
      };
      return true;
    case 69:
      *header = (h3c_header_t){
        .name = { .data = ":status", .size = sizeof(":status") - 1 },
        .value = { .data = "421", .size = sizeof("421") - 1 }
      };
      return true;
    case 70:
      *header = (h3c_header_t){
        .name = { .data = ":status", .size = sizeof(":status") - 1 },
        .value = { .data = "425", .size = sizeof("425") - 1 }
      };
      return true;
    case 71:
      *header = (h3c_header_t){
        .name = { .data = ":status", .size = sizeof(":status") - 1 },
        .value = { .data = "500", .size = sizeof("500") - 1 }
      };
      return true;
    case 29:
      *header = (h3c_header_t){
        .name = { .data = "accept", .size = sizeof("accept") - 1 },
        .value = { .data = "*/*", .size = sizeof("*/*") - 1 }
      };
      return true;
    case 30:
      *header = (h3c_header_t){
        .name = { .data = "accept", .size = sizeof("accept") - 1 },
        .value = { .data = "application/dns-message",
                   .size = sizeof("application/dns-message") - 1 }
      };
      return true;
    case 31:
      *header = (h3c_header_t){
        .name = { .data = "accept-encoding",
                  .size = sizeof("accept-encoding") - 1 },
        .value = { .data = "gzip, deflate, br",
                   .size = sizeof("gzip, deflate, br") - 1 }
      };
      return true;
    case 32:
      *header = (h3c_header_t){ .name = { .data = "accept-ranges",
                                          .size = sizeof("accept-ranges") - 1 },
                                .value = { .data = "bytes",
                                           .size = sizeof("bytes") - 1 } };
      return true;
    case 73:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-credentials",
                  .size = sizeof("access-control-allow-credentials") - 1 },
        .value = { .data = "FALSE", .size = sizeof("FALSE") - 1 }
      };
      return true;
    case 74:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-credentials",
                  .size = sizeof("access-control-allow-credentials") - 1 },
        .value = { .data = "TRUE", .size = sizeof("TRUE") - 1 }
      };
      return true;
    case 33:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-headers",
                  .size = sizeof("access-control-allow-headers") - 1 },
        .value = { .data = "cache-control",
                   .size = sizeof("cache-control") - 1 }
      };
      return true;
    case 34:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-headers",
                  .size = sizeof("access-control-allow-headers") - 1 },
        .value = { .data = "content-type", .size = sizeof("content-type") - 1 }
      };
      return true;
    case 75:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-headers",
                  .size = sizeof("access-control-allow-headers") - 1 },
        .value = { .data = "*", .size = sizeof("*") - 1 }
      };
      return true;
    case 76:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-methods",
                  .size = sizeof("access-control-allow-methods") - 1 },
        .value = { .data = "get", .size = sizeof("get") - 1 }
      };
      return true;
    case 77:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-methods",
                  .size = sizeof("access-control-allow-methods") - 1 },
        .value = { .data = "get, post, options",
                   .size = sizeof("get, post, options") - 1 }
      };
      return true;
    case 78:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-methods",
                  .size = sizeof("access-control-allow-methods") - 1 },
        .value = { .data = "options", .size = sizeof("options") - 1 }
      };
      return true;
    case 35:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-origin",
                  .size = sizeof("access-control-allow-origin") - 1 },
        .value = { .data = "*", .size = sizeof("*") - 1 }
      };
      return true;
    case 79:
      *header = (h3c_header_t){
        .name = { .data = "access-control-expose-headers",
                  .size = sizeof("access-control-expose-headers") - 1 },
        .value = { .data = "content-length",
                   .size = sizeof("content-length") - 1 }
      };
      return true;
    case 80:
      *header = (h3c_header_t){
        .name = { .data = "access-control-request-headers",
                  .size = sizeof("access-control-request-headers") - 1 },
        .value = { .data = "content-type", .size = sizeof("content-type") - 1 }
      };
      return true;
    case 81:
      *header = (h3c_header_t){
        .name = { .data = "access-control-request-method",
                  .size = sizeof("access-control-request-method") - 1 },
        .value = { .data = "get", .size = sizeof("get") - 1 }
      };
      return true;
    case 82:
      *header = (h3c_header_t){
        .name = { .data = "access-control-request-method",
                  .size = sizeof("access-control-request-method") - 1 },
        .value = { .data = "post", .size = sizeof("post") - 1 }
      };
      return true;
    case 2:
      *header = (h3c_header_t){
        .name = { .data = "age", .size = sizeof("age") - 1 },
        .value = { .data = "0", .size = sizeof("0") - 1 }
      };
      return true;
    case 83:
      *header = (h3c_header_t){
        .name = { .data = "alt-svc", .size = sizeof("alt-svc") - 1 },
        .value = { .data = "clear", .size = sizeof("clear") - 1 }
      };
      return true;
    case 36:
      *header = (h3c_header_t){ .name = { .data = "cache-control",
                                          .size = sizeof("cache-control") - 1 },
                                .value = { .data = "max-age=0",
                                           .size = sizeof("max-age=0") - 1 } };
      return true;
    case 37:
      *header = (h3c_header_t){ .name = { .data = "cache-control",
                                          .size = sizeof("cache-control") - 1 },
                                .value = { .data = "max-age=2592000",
                                           .size = sizeof("max-age=2592000") -
                                                   1 } };
      return true;
    case 38:
      *header = (h3c_header_t){ .name = { .data = "cache-control",
                                          .size = sizeof("cache-control") - 1 },
                                .value = { .data = "max-age=604800",
                                           .size = sizeof("max-age=604800") -
                                                   1 } };
      return true;
    case 39:
      *header = (h3c_header_t){ .name = { .data = "cache-control",
                                          .size = sizeof("cache-control") - 1 },
                                .value = { .data = "no-cache",
                                           .size = sizeof("no-cache") - 1 } };
      return true;
    case 40:
      *header = (h3c_header_t){ .name = { .data = "cache-control",
                                          .size = sizeof("cache-control") - 1 },
                                .value = { .data = "no-store",
                                           .size = sizeof("no-store") - 1 } };
      return true;
    case 41:
      *header = (h3c_header_t){
        .name = { .data = "cache-control",
                  .size = sizeof("cache-control") - 1 },
        .value = { .data = "public, max-age=31536000",
                   .size = sizeof("public, max-age=31536000") - 1 }
      };
      return true;
    case 42:
      *header = (h3c_header_t){
        .name = { .data = "content-encoding",
                  .size = sizeof("content-encoding") - 1 },
        .value = { .data = "br", .size = sizeof("br") - 1 }
      };
      return true;
    case 43:
      *header = (h3c_header_t){
        .name = { .data = "content-encoding",
                  .size = sizeof("content-encoding") - 1 },
        .value = { .data = "gzip", .size = sizeof("gzip") - 1 }
      };
      return true;
    case 4:
      *header = (h3c_header_t){
        .name = { .data = "content-length",
                  .size = sizeof("content-length") - 1 },
        .value = { .data = "0", .size = sizeof("0") - 1 }
      };
      return true;
    case 85:
      *header = (h3c_header_t){
        .name = { .data = "content-security-policy",
                  .size = sizeof("content-security-policy") - 1 },
        .value = { .data =
                       "script-src 'none'; object-src 'none'; base-uri 'none'",
                   .size = sizeof("script-src 'none'; object-src 'none'; "
                                  "base-uri 'none'") -
                           1 }
      };
      return true;
    case 44:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .size = sizeof("content-type") - 1 },
        .value = { .data = "application/dns-message",
                   .size = sizeof("application/dns-message") - 1 }
      };
      return true;
    case 45:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .size = sizeof("content-type") - 1 },
        .value = { .data = "application/javascript",
                   .size = sizeof("application/javascript") - 1 }
      };
      return true;
    case 46:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .size = sizeof("content-type") - 1 },
        .value = { .data = "application/json",
                   .size = sizeof("application/json") - 1 }
      };
      return true;
    case 47:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .size = sizeof("content-type") - 1 },
        .value = { .data = "application/x-www-form-urlencoded",
                   .size = sizeof("application/x-www-form-urlencoded") - 1 }
      };
      return true;
    case 48:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .size = sizeof("content-type") - 1 },
        .value = { .data = "image/gif", .size = sizeof("image/gif") - 1 }
      };
      return true;
    case 49:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .size = sizeof("content-type") - 1 },
        .value = { .data = "image/jpeg", .size = sizeof("image/jpeg") - 1 }
      };
      return true;
    case 50:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .size = sizeof("content-type") - 1 },
        .value = { .data = "image/png", .size = sizeof("image/png") - 1 }
      };
      return true;
    case 51:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .size = sizeof("content-type") - 1 },
        .value = { .data = "text/css", .size = sizeof("text/css") - 1 }
      };
      return true;
    case 52:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .size = sizeof("content-type") - 1 },
        .value = { .data = "text/html; charset=utf-8",
                   .size = sizeof("text/html; charset=utf-8") - 1 }
      };
      return true;
    case 53:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .size = sizeof("content-type") - 1 },
        .value = { .data = "text/plain", .size = sizeof("text/plain") - 1 }
      };
      return true;
    case 54:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .size = sizeof("content-type") - 1 },
        .value = { .data = "text/plain;charset=utf-8",
                   .size = sizeof("text/plain;charset=utf-8") - 1 }
      };
      return true;
    case 86:
      *header = (h3c_header_t){
        .name = { .data = "early-data", .size = sizeof("early-data") - 1 },
        .value = { .data = "1", .size = sizeof("1") - 1 }
      };
      return true;
    case 91:
      *header = (h3c_header_t){
        .name = { .data = "purpose", .size = sizeof("purpose") - 1 },
        .value = { .data = "prefetch", .size = sizeof("prefetch") - 1 }
      };
      return true;
    case 55:
      *header = (h3c_header_t){
        .name = { .data = "range", .size = sizeof("range") - 1 },
        .value = { .data = "bytes=0-", .size = sizeof("bytes=0-") - 1 }
      };
      return true;
    case 56:
      *header = (h3c_header_t){
        .name = { .data = "strict-transport-security",
                  .size = sizeof("strict-transport-security") - 1 },
        .value = { .data = "max-age=31536000",
                   .size = sizeof("max-age=31536000") - 1 }
      };
      return true;
    case 57:
      *header = (h3c_header_t){
        .name = { .data = "strict-transport-security",
                  .size = sizeof("strict-transport-security") - 1 },
        .value = { .data = "max-age=31536000; includesubdomains",
                   .size = sizeof("max-age=31536000; includesubdomains") - 1 }
      };
      return true;
    case 58:
      *header = (h3c_header_t){
        .name = { .data = "strict-transport-security",
                  .size = sizeof("strict-transport-security") - 1 },
        .value = { .data = "max-age=31536000; includesubdomains; preload",
                   .size = sizeof(
                               "max-age=31536000; includesubdomains; preload") -
                           1 }
      };
      return true;
    case 93:
      *header = (h3c_header_t){
        .name = { .data = "timing-allow-origin",
                  .size = sizeof("timing-allow-origin") - 1 },
        .value = { .data = "*", .size = sizeof("*") - 1 }
      };
      return true;
    case 94:
      *header = (h3c_header_t){
        .name = { .data = "upgrade-insecure-requests",
                  .size = sizeof("upgrade-insecure-requests") - 1 },
        .value = { .data = "1", .size = sizeof("1") - 1 }
      };
      return true;
    case 59:
      *header = (h3c_header_t){
        .name = { .data = "vary", .size = sizeof("vary") - 1 },
        .value = { .data = "accept-encoding",
                   .size = sizeof("accept-encoding") - 1 }
      };
      return true;
    case 60:
      *header = (h3c_header_t){
        .name = { .data = "vary", .size = sizeof("vary") - 1 },
        .value = { .data = "origin", .size = sizeof("origin") - 1 }
      };
      return true;
    case 61:
      *header = (h3c_header_t){
        .name = { .data = "x-content-type-options",
                  .size = sizeof("x-content-type-options") - 1 },
        .value = { .data = "nosniff", .size = sizeof("nosniff") - 1 }
      };
      return true;
    case 97:
      *header = (h3c_header_t){
        .name = { .data = "x-frame-options",
                  .size = sizeof("x-frame-options") - 1 },
        .value = { .data = "deny", .size = sizeof("deny") - 1 }
      };
      return true;
    case 98:
      *header = (h3c_header_t){
        .name = { .data = "x-frame-options",
                  .size = sizeof("x-frame-options") - 1 },
        .value = { .data = "sameorigin", .size = sizeof("sameorigin") - 1 }
      };
      return true;
    case 62:
      *header = (h3c_header_t){
        .name = { .data = "x-xss-protection",
                  .size = sizeof("x-xss-protection") - 1 },
        .value = { .data = "1; mode=block",
                   .size = sizeof("1; mode=block") - 1 }
      };
      return true;
  }

  return false;
}

bool static_table_find_header_only(uint8_t index, h3c_header_t *header)
{
  switch (index) {
    case 0:
      *header = (h3c_header_t){ .name = { .data = ":authority",
                                          .size = sizeof(":authority") - 1 } };
      return true;
    case 15:
      *header = (h3c_header_t){ .name = { .data = ":method",
                                          .size = sizeof(":method") - 1 } };
      return true;
    case 16:
      *header = (h3c_header_t){ .name = { .data = ":method",
                                          .size = sizeof(":method") - 1 } };
      return true;
    case 17:
      *header = (h3c_header_t){ .name = { .data = ":method",
                                          .size = sizeof(":method") - 1 } };
      return true;
    case 18:
      *header = (h3c_header_t){ .name = { .data = ":method",
                                          .size = sizeof(":method") - 1 } };
      return true;
    case 19:
      *header = (h3c_header_t){ .name = { .data = ":method",
                                          .size = sizeof(":method") - 1 } };
      return true;
    case 20:
      *header = (h3c_header_t){ .name = { .data = ":method",
                                          .size = sizeof(":method") - 1 } };
      return true;
    case 21:
      *header = (h3c_header_t){ .name = { .data = ":method",
                                          .size = sizeof(":method") - 1 } };
      return true;
    case 1:
      *header = (h3c_header_t){ .name = { .data = ":path",
                                          .size = sizeof(":path") - 1 } };
      return true;
    case 22:
      *header = (h3c_header_t){ .name = { .data = ":scheme",
                                          .size = sizeof(":scheme") - 1 } };
      return true;
    case 23:
      *header = (h3c_header_t){ .name = { .data = ":scheme",
                                          .size = sizeof(":scheme") - 1 } };
      return true;
    case 24:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .size = sizeof(":status") - 1 } };
      return true;
    case 25:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .size = sizeof(":status") - 1 } };
      return true;
    case 26:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .size = sizeof(":status") - 1 } };
      return true;
    case 27:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .size = sizeof(":status") - 1 } };
      return true;
    case 28:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .size = sizeof(":status") - 1 } };
      return true;
    case 63:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .size = sizeof(":status") - 1 } };
      return true;
    case 64:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .size = sizeof(":status") - 1 } };
      return true;
    case 65:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .size = sizeof(":status") - 1 } };
      return true;
    case 66:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .size = sizeof(":status") - 1 } };
      return true;
    case 67:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .size = sizeof(":status") - 1 } };
      return true;
    case 68:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .size = sizeof(":status") - 1 } };
      return true;
    case 69:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .size = sizeof(":status") - 1 } };
      return true;
    case 70:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .size = sizeof(":status") - 1 } };
      return true;
    case 71:
      *header = (h3c_header_t){ .name = { .data = ":status",
                                          .size = sizeof(":status") - 1 } };
      return true;
    case 29:
      *header = (h3c_header_t){ .name = { .data = "accept",
                                          .size = sizeof("accept") - 1 } };
      return true;
    case 30:
      *header = (h3c_header_t){ .name = { .data = "accept",
                                          .size = sizeof("accept") - 1 } };
      return true;
    case 31:
      *header = (h3c_header_t){ .name = { .data = "accept-encoding",
                                          .size = sizeof("accept-encoding") -
                                                  1 } };
      return true;
    case 72:
      *header = (h3c_header_t){ .name = { .data = "accept-language",
                                          .size = sizeof("accept-language") -
                                                  1 } };
      return true;
    case 32:
      *header = (h3c_header_t){
        .name = { .data = "accept-ranges", .size = sizeof("accept-ranges") - 1 }
      };
      return true;
    case 73:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-credentials",
                  .size = sizeof("access-control-allow-credentials") - 1 }
      };
      return true;
    case 74:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-credentials",
                  .size = sizeof("access-control-allow-credentials") - 1 }
      };
      return true;
    case 33:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-headers",
                  .size = sizeof("access-control-allow-headers") - 1 }
      };
      return true;
    case 34:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-headers",
                  .size = sizeof("access-control-allow-headers") - 1 }
      };
      return true;
    case 75:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-headers",
                  .size = sizeof("access-control-allow-headers") - 1 }
      };
      return true;
    case 76:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-methods",
                  .size = sizeof("access-control-allow-methods") - 1 }
      };
      return true;
    case 77:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-methods",
                  .size = sizeof("access-control-allow-methods") - 1 }
      };
      return true;
    case 78:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-methods",
                  .size = sizeof("access-control-allow-methods") - 1 }
      };
      return true;
    case 35:
      *header = (h3c_header_t){
        .name = { .data = "access-control-allow-origin",
                  .size = sizeof("access-control-allow-origin") - 1 }
      };
      return true;
    case 79:
      *header = (h3c_header_t){
        .name = { .data = "access-control-expose-headers",
                  .size = sizeof("access-control-expose-headers") - 1 }
      };
      return true;
    case 80:
      *header = (h3c_header_t){
        .name = { .data = "access-control-request-headers",
                  .size = sizeof("access-control-request-headers") - 1 }
      };
      return true;
    case 81:
      *header = (h3c_header_t){
        .name = { .data = "access-control-request-method",
                  .size = sizeof("access-control-request-method") - 1 }
      };
      return true;
    case 82:
      *header = (h3c_header_t){
        .name = { .data = "access-control-request-method",
                  .size = sizeof("access-control-request-method") - 1 }
      };
      return true;
    case 2:
      *header = (h3c_header_t){ .name = { .data = "age",
                                          .size = sizeof("age") - 1 } };
      return true;
    case 83:
      *header = (h3c_header_t){ .name = { .data = "alt-svc",
                                          .size = sizeof("alt-svc") - 1 } };
      return true;
    case 84:
      *header = (h3c_header_t){
        .name = { .data = "authorization", .size = sizeof("authorization") - 1 }
      };
      return true;
    case 36:
      *header = (h3c_header_t){
        .name = { .data = "cache-control", .size = sizeof("cache-control") - 1 }
      };
      return true;
    case 37:
      *header = (h3c_header_t){
        .name = { .data = "cache-control", .size = sizeof("cache-control") - 1 }
      };
      return true;
    case 38:
      *header = (h3c_header_t){
        .name = { .data = "cache-control", .size = sizeof("cache-control") - 1 }
      };
      return true;
    case 39:
      *header = (h3c_header_t){
        .name = { .data = "cache-control", .size = sizeof("cache-control") - 1 }
      };
      return true;
    case 40:
      *header = (h3c_header_t){
        .name = { .data = "cache-control", .size = sizeof("cache-control") - 1 }
      };
      return true;
    case 41:
      *header = (h3c_header_t){
        .name = { .data = "cache-control", .size = sizeof("cache-control") - 1 }
      };
      return true;
    case 3:
      *header = (h3c_header_t){
        .name = { .data = "content-disposition",
                  .size = sizeof("content-disposition") - 1 }
      };
      return true;
    case 42:
      *header = (h3c_header_t){ .name = { .data = "content-encoding",
                                          .size = sizeof("content-encoding") -
                                                  1 } };
      return true;
    case 43:
      *header = (h3c_header_t){ .name = { .data = "content-encoding",
                                          .size = sizeof("content-encoding") -
                                                  1 } };
      return true;
    case 4:
      *header = (h3c_header_t){ .name = { .data = "content-length",
                                          .size = sizeof("content-length") -
                                                  1 } };
      return true;
    case 85:
      *header = (h3c_header_t){
        .name = { .data = "content-security-policy",
                  .size = sizeof("content-security-policy") - 1 }
      };
      return true;
    case 44:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .size = sizeof("content-type") - 1 }
      };
      return true;
    case 45:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .size = sizeof("content-type") - 1 }
      };
      return true;
    case 46:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .size = sizeof("content-type") - 1 }
      };
      return true;
    case 47:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .size = sizeof("content-type") - 1 }
      };
      return true;
    case 48:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .size = sizeof("content-type") - 1 }
      };
      return true;
    case 49:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .size = sizeof("content-type") - 1 }
      };
      return true;
    case 50:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .size = sizeof("content-type") - 1 }
      };
      return true;
    case 51:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .size = sizeof("content-type") - 1 }
      };
      return true;
    case 52:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .size = sizeof("content-type") - 1 }
      };
      return true;
    case 53:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .size = sizeof("content-type") - 1 }
      };
      return true;
    case 54:
      *header = (h3c_header_t){
        .name = { .data = "content-type", .size = sizeof("content-type") - 1 }
      };
      return true;
    case 5:
      *header = (h3c_header_t){ .name = { .data = "cookie",
                                          .size = sizeof("cookie") - 1 } };
      return true;
    case 6:
      *header = (h3c_header_t){ .name = { .data = "date",
                                          .size = sizeof("date") - 1 } };
      return true;
    case 86:
      *header = (h3c_header_t){ .name = { .data = "early-data",
                                          .size = sizeof("early-data") - 1 } };
      return true;
    case 7:
      *header = (h3c_header_t){ .name = { .data = "etag",
                                          .size = sizeof("etag") - 1 } };
      return true;
    case 87:
      *header = (h3c_header_t){ .name = { .data = "expect-ct",
                                          .size = sizeof("expect-ct") - 1 } };
      return true;
    case 88:
      *header = (h3c_header_t){ .name = { .data = "forwarded",
                                          .size = sizeof("forwarded") - 1 } };
      return true;
    case 8:
      *header = (h3c_header_t){ .name = { .data = "if-modified-since",
                                          .size = sizeof("if-modified-since") -
                                                  1 } };
      return true;
    case 9:
      *header = (h3c_header_t){
        .name = { .data = "if-none-match", .size = sizeof("if-none-match") - 1 }
      };
      return true;
    case 89:
      *header = (h3c_header_t){ .name = { .data = "if-range",
                                          .size = sizeof("if-range") - 1 } };
      return true;
    case 10:
      *header = (h3c_header_t){
        .name = { .data = "last-modified", .size = sizeof("last-modified") - 1 }
      };
      return true;
    case 11:
      *header = (h3c_header_t){ .name = { .data = "link",
                                          .size = sizeof("link") - 1 } };
      return true;
    case 12:
      *header = (h3c_header_t){ .name = { .data = "location",
                                          .size = sizeof("location") - 1 } };
      return true;
    case 90:
      *header = (h3c_header_t){ .name = { .data = "origin",
                                          .size = sizeof("origin") - 1 } };
      return true;
    case 91:
      *header = (h3c_header_t){ .name = { .data = "purpose",
                                          .size = sizeof("purpose") - 1 } };
      return true;
    case 55:
      *header = (h3c_header_t){ .name = { .data = "range",
                                          .size = sizeof("range") - 1 } };
      return true;
    case 13:
      *header = (h3c_header_t){ .name = { .data = "referer",
                                          .size = sizeof("referer") - 1 } };
      return true;
    case 92:
      *header = (h3c_header_t){ .name = { .data = "server",
                                          .size = sizeof("server") - 1 } };
      return true;
    case 14:
      *header = (h3c_header_t){ .name = { .data = "set-cookie",
                                          .size = sizeof("set-cookie") - 1 } };
      return true;
    case 56:
      *header = (h3c_header_t){
        .name = { .data = "strict-transport-security",
                  .size = sizeof("strict-transport-security") - 1 }
      };
      return true;
    case 57:
      *header = (h3c_header_t){
        .name = { .data = "strict-transport-security",
                  .size = sizeof("strict-transport-security") - 1 }
      };
      return true;
    case 58:
      *header = (h3c_header_t){
        .name = { .data = "strict-transport-security",
                  .size = sizeof("strict-transport-security") - 1 }
      };
      return true;
    case 93:
      *header = (h3c_header_t){
        .name = { .data = "timing-allow-origin",
                  .size = sizeof("timing-allow-origin") - 1 }
      };
      return true;
    case 94:
      *header = (h3c_header_t){
        .name = { .data = "upgrade-insecure-requests",
                  .size = sizeof("upgrade-insecure-requests") - 1 }
      };
      return true;
    case 95:
      *header = (h3c_header_t){ .name = { .data = "user-agent",
                                          .size = sizeof("user-agent") - 1 } };
      return true;
    case 59:
      *header = (h3c_header_t){ .name = { .data = "vary",
                                          .size = sizeof("vary") - 1 } };
      return true;
    case 60:
      *header = (h3c_header_t){ .name = { .data = "vary",
                                          .size = sizeof("vary") - 1 } };
      return true;
    case 61:
      *header = (h3c_header_t){
        .name = { .data = "x-content-type-options",
                  .size = sizeof("x-content-type-options") - 1 }
      };
      return true;
    case 96:
      *header = (h3c_header_t){ .name = { .data = "x-forwarded-for",
                                          .size = sizeof("x-forwarded-for") -
                                                  1 } };
      return true;
    case 97:
      *header = (h3c_header_t){ .name = { .data = "x-frame-options",
                                          .size = sizeof("x-frame-options") -
                                                  1 } };
      return true;
    case 98:
      *header = (h3c_header_t){ .name = { .data = "x-frame-options",
                                          .size = sizeof("x-frame-options") -
                                                  1 } };
      return true;
    case 62:
      *header = (h3c_header_t){ .name = { .data = "x-xss-protection",
                                          .size = sizeof("x-xss-protection") -
                                                  1 } };
      return true;
  }

  return false;
}
