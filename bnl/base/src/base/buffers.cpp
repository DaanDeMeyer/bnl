#include <bnl/base/buffers.hpp>

#include <algorithm>
#include <cassert>

namespace bnl {
namespace base {

static size_t find_first_not_empty(const std::deque<buffer> &buffers)
{
  for (size_t i = 0; i < buffers.size(); i++) {
    if (!buffers[i].empty()) {
      return i;
    }
  }

  return buffers.size() - 1;
}

size_t buffers::size() const noexcept
{
  size_t size = 0;

  for (const buffer &buffer : buffers_) {
    size += buffer.size();
  }

  return size;
}

bool buffers::empty() const noexcept
{
  return size() == 0;
}

uint8_t buffers::operator[](size_t index) const noexcept
{
  assert(index < size());

  size_t i = 0;
  while (index >= buffers_[i].size()) {
    index -= buffers_[i].size();
    i++;
  }

  return buffers_[i][index];
}

uint8_t buffers::operator*() const noexcept
{
  return operator[](0);
}

buffer buffers::slice(size_t size)
{
  assert(size <= this->size());

  size_t start = find_first_not_empty(buffers_);

  assert(start <= buffers_.size());

  size_t end = start;
  size_t left = size;
  while (left > buffers_[end].size()) {
    left -= buffers_[end].size();
    end++;
  }

  if (start == end) {
    return buffers_[start].slice(left);
  }

  buffer result = concat(start, end, left);

  consume(result.size());

  return result;
}

void buffers::push(buffer buffer)
{
  buffers_.emplace_back(std::move(buffer));
}

const buffer &buffers::front() const noexcept
{
  return buffers_.front();
}

const buffer &buffers::back() const noexcept
{
  return buffers_.back();
}

buffer &buffers::front() noexcept
{
  return buffers_.front();
}

buffer &buffers::back() noexcept
{
  return buffers_.back();
}

void buffers::consume(size_t size) noexcept
{
  assert(size <= this->size());

  for (size_t i = 0; size != 0; i++) {
    size_t to_consume = std::min(size, buffers_[i].size());
    buffers_[i].consume(to_consume);
    size -= to_consume;
  }

  discard();
}

size_t buffers::consumed() const noexcept
{
  size_t consumed = 0;

  for (const buffer &buffer : buffers_) {
    consumed += buffer.consumed();
  }

  return consumed;
}

void buffers::discard() noexcept
{
  while (!buffers_.empty() && buffers_.front().empty()) {
    buffers_.pop_front();
  }
}

buffer buffers::concat(size_t start, size_t end, size_t left) const
{
  size_t size = 0;
  for (size_t i = start; i < end; i++) {
    size += buffers_[i].size();
  }

  size += left;

  buffer result(size);
  size_t offset = 0;

  for (size_t i = start; i < end; i++) {
    std::copy_n(buffers_[i].data(), buffers_[i].size(), result.data() + offset);
    offset += buffers_[i].size();
  }

  std::copy_n(buffers_[end].data(), left, result.data() + offset);

  return result;
}

buffers::lookahead::lookahead(const buffers &buffers) noexcept
    : buffers_(buffers)
{}

buffers::lookahead::lookahead(const lookahead &other) noexcept
    : buffers_(other.buffers_), previous_(other.previous_ + other.position_)
{}

size_t buffers::lookahead::size() const noexcept
{
  return buffers_.size() - position_ - previous_;
}

bool buffers::lookahead::empty() const noexcept
{
  return size() == 0;
}

uint8_t buffers::lookahead::operator[](size_t index) const noexcept
{
  assert(index < size());
  return buffers_[previous_ + position_ + index];
}

uint8_t buffers::lookahead::operator*() const noexcept
{
  return buffers_[previous_ + position_];
}

void buffers::lookahead::consume(size_t size) noexcept
{
  assert(size <= this->size());
  position_ += size;
}

size_t buffers::lookahead::consumed() const noexcept
{
  return position_;
}

buffer buffers::lookahead::copy(size_t size) const
{
  assert(size <= this->size());

  buffer result(size);
  for (size_t i = 0; i < size; i++) {
    result[i] = operator[](i);
  }

  return result;
}

} // namespace base
} // namespace bnl
