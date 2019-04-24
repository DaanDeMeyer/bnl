#include <h3c/error.h>

const char *h3c_strerror(H3C_ERROR error)
{
  switch (error) {
    case H3C_SUCCESS:
      return "h3c: success";

      // HTTP/3 connection errors

    case H3C_ERROR_MALFORMED_FRAME:
      return "HTTP/3: malformed frame";

      // h3c library errors

    case H3C_ERROR_INTERNAL:
      return "h3c: internal error";
    case H3C_ERROR_BUF_TOO_SMALL:
      return "h3c: buf too small";
    case H3C_ERROR_VARINT_OVERFLOW:
      return "h3c: varint overflow";
    case H3C_ERROR_SETTING_OVERFLOW:
      return "h3c: setting overflow";
    case H3C_ERROR_INCOMPLETE_FRAME:
      return "h3c: frame incomplete";
  }

  return "unknown error";
}

H3C_ERROR_TYPE h3c_error_type(H3C_ERROR error)
{
  if (error == H3C_SUCCESS) {
    return H3C_ERROR_TYPE_SUCCESS;
  }

  if (error > UINT16_MAX) {
    return H3C_ERROR_TYPE_LIBRARY;
  }

  return H3C_ERROR_TYPE_CONNECTION;
}