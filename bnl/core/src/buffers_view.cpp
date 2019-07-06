#include <bnl/buffers_view.hpp>

#include <bnl/buffers.hpp>

#include <cassert>

namespace bnl {

buffers_view::buffers_view(const buffers &buffers) noexcept : buffers_(buffers)
{}

size_t buffers_view::size() const noexcept
{
  return buffers_.size() - position_;
}

bool buffers_view::empty() const noexcept
{
  return size() == 0;
}

uint8_t buffers_view::operator[](size_t index) const noexcept
{
  assert(index < size());
  return buffers_[position_ + index];
}

uint8_t buffers_view::operator*() const noexcept
{
  return buffers_[position_];
}

void buffers_view::consume(size_t size) noexcept
{
  assert(size <= this->size());

  position_ += size;
}

size_t buffers_view::consumed() const noexcept
{
  return position_;
}

buffer buffers_view::copy(size_t size) const
{
  assert(size <= this->size());

  buffer result(size);
  for (size_t i = 0; i < size; i++) {
    result[i] = operator[](i);
  }

  return result;
}

} // namespace bnl
