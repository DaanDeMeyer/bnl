#include <bnl/base/buffer_view.hpp>

#include <algorithm>
#include <cassert>

namespace bnl {
namespace base {

buffer_view::buffer_view(const uint8_t *data, size_t size) noexcept
    : data_(data), size_(size)
{}

const uint8_t *buffer_view::data() const noexcept
{
  return data_;
}

size_t buffer_view::size() const noexcept
{
  return size_;
}

bool buffer_view::empty() const noexcept
{
  return size() == 0;
}

const uint8_t *buffer_view::begin() const noexcept
{
  return data();
}

const uint8_t *buffer_view::end() const noexcept
{
  return data() + size();
}

bool operator==(buffer_view lhs, buffer_view rhs) noexcept
{
  return lhs.size() == rhs.size() &&
         std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

bool operator!=(buffer_view lhs, buffer_view rhs) noexcept
{
  return !(lhs == rhs);
}

buffer_view_mut::buffer_view_mut(uint8_t *data, size_t size) noexcept
    : data_(data), size_(size)
{}

uint8_t *buffer_view_mut::data() noexcept
{
  return data_;
}

size_t buffer_view_mut::size() const noexcept
{
  return size_;
}

bool buffer_view_mut::empty() const noexcept
{
  return size() == 0;
}

uint8_t *buffer_view_mut::begin() noexcept
{
  return data();
}

uint8_t *buffer_view_mut::end() noexcept
{
  return data() + size();
}

buffer_view_mut::operator buffer_view() const noexcept
{
  return { data_, size() };
}

} // namespace base
} // namespace bnl
