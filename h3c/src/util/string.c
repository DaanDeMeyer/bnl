#include <util/string.h>

bool is_lowercase(const char *string, size_t length)
{
  for (size_t i = 0; i < length; i++) {
    if (string[i] >= 'A' && string[i] <= 'Z') {
      return false;
    }
  }

  return true;
}
