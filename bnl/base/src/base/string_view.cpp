#include <bnl/base/string_view.hpp>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <ostream>

namespace bnl {
namespace base {

string_view::string_view(const char *data) noexcept
    : data_(data), size_(strlen(data))
{}

string_view::string_view(const char *data, size_t size) noexcept
    : data_(data), size_(size)
{}

string_view::string_view(const string &string) noexcept
    : string_view(string.data(), string.size())
{}

const char *string_view::data() const noexcept
{
  return data_;
}

size_t string_view::size() const noexcept
{
  return size_;
}

bool string_view::empty() const noexcept
{
  return size() == 0;
}

const char *string_view::begin() const noexcept
{
  return data();
}

const char *string_view::end() const noexcept
{
  return data() + size();
}

char string_view::operator[](size_t index) const noexcept
{
  assert(index < size());
  return *(data() + index);
}

char string_view::operator*() const noexcept
{
  return *data();
}

bool operator==(string_view lhs, string_view rhs) noexcept
{
  return lhs.size() == rhs.size() &&
         std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

bool operator!=(string_view lhs, string_view rhs) noexcept
{
  return !(lhs == rhs);
}

std::ostream &operator<<(std::ostream &out, string_view string)
{
  out.write(string.data(), static_cast<std::streamsize>(string.size()));
  return out;
}

} // namespace base
} // namespace bnl
