#include <bnl/buffer_view.hpp>

#include <algorithm>
#include <cassert>

namespace bnl {

buffer_view::buffer_view(const uint8_t *data, size_t size) noexcept
    : data_(data), size_(size)
{}

buffer_view::buffer_view(const buffer &buffer) noexcept
    : buffer_view(buffer.data(), buffer.size())
{}

const uint8_t *buffer_view::data() const noexcept
{
  return data_ + position_;
}

size_t buffer_view::size() const noexcept
{
  return size_ - position_;
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

uint8_t buffer_view::operator[](size_t index) const noexcept
{
  assert(index < size());
  return *(data() + index);
}

uint8_t buffer_view::operator*() const noexcept
{
  return *data();
}

void buffer_view::consume(size_t size) noexcept
{
  assert(size <= this->size());
  position_ += size;
}

size_t buffer_view::consumed() const noexcept
{
  return position_;
}

buffer buffer_view::copy(size_t size) const
{
  assert(size <= this->size());
  return buffer(data(), size);
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

} // namespace bnl
