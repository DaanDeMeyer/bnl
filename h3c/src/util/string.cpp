#include <util/string.hpp>

namespace h3c {
namespace util {

bool is_lowercase(const char *string, size_t size)
{
  for (size_t i = 0; i < size; i++) {
    if (string[i] >= 'A' && string[i] <= 'Z') {
      return false;
    }
  }

  return true;
}

} // namespace util
} // namespace h3c
