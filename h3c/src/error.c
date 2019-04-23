#include <h3c/error.h>

const char *h3c_strerror(H3C_ERROR error)
{
  switch (error) {
  case H3C_SUCCESS:
    return "success";
  case H3C_ERROR_BUF_TOO_SMALL:
    return "buf too small";
  case H3C_ERROR_VARINT_OVERFLOW:
    return "varint overflow";
  case H3C_ERROR_SETTING_OVERFLOW:
    return "setting overflow";
  case H3C_ERROR_FRAME_INCOMPLETE:
    return "frame incomplete";
  case H3C_ERROR_FRAME_MALFORMED:
    return "frame malformed";
  }

  return "unknown error";
}
