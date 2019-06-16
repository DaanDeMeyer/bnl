#include <bnl/buffer_view.hpp>

#include <algorithm>

namespace bnl {

buffer_view::buffer_view(const char *data, size_t size) noexcept
    : buffer_view(reinterpret_cast<const uint8_t *>(data), size)
{}

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

const uint8_t *buffer_view::begin() const noexcept
{
  return data_;
}

const uint8_t *buffer_view::end() const noexcept
{
  return data_ + size_;
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

mutable_buffer_view::mutable_buffer_view(uint8_t *data, size_t size) noexcept
    : data_(data), size_(size)
{}

uint8_t *mutable_buffer_view::data() noexcept
{
  return data_;
}

size_t mutable_buffer_view::size() const noexcept
{
  return size_;
}

uint8_t *mutable_buffer_view::begin() noexcept
{
  return data();
}

uint8_t *mutable_buffer_view::end() noexcept
{
  return data() + size();
}

mutable_buffer_view::operator buffer_view() const noexcept
{
  return { data_, size() };
}

} // namespace bnl
