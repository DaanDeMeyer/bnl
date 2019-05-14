#include <h3c/error.h>

const char *h3c_error_string(H3C_ERROR error)
{
  switch (error) {
    case H3C_SUCCESS:
      return "success";

    // HTTP/3 connection errors

    case H3C_ERROR_MALFORMED_FRAME:
      return "malformed frame";
    case H3C_ERROR_QPACK_DECOMPRESSION_FAILED:
      return "qpack decompression failed";

    // h3c library errors

    case H3C_ERROR_INTERNAL:
      return "internal error";
    case H3C_ERROR_OUT_OF_MEMORY:
      return "out of memory";
    case H3C_ERROR_BUFFER_TOO_SMALL:
      return "buffer too small";
    case H3C_ERROR_INCOMPLETE:
      return "incomplete";
    case H3C_ERROR_VARINT_OVERFLOW:
      return "varint overflow";
    case H3C_ERROR_SETTING_OVERFLOW:
      return "setting overflow";
    case H3C_ERROR_MALFORMED_HEADER:
      return "malformed header";
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
