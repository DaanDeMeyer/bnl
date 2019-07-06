#include <bnl/util/string.hpp>

namespace bnl {
namespace util {

bool is_lowercase(string_view string)
{
  for (char character : string) {
    if (character >= 'A' && character <= 'Z') {
      return false;
    }
  }

  return true;
}

} // namespace util
} // namespace bnl
