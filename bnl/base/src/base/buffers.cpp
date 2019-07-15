#include <bnl/base/buffers.hpp>

#include <algorithm>
#include <cassert>

namespace bnl {
namespace base {

static std::list<buffer>::iterator
find_first_not_empty(std::list<buffer> &buffers)
{
  for (auto it = buffers.begin(); it != buffers.end(); it++) {
    if (!it->empty()) {
      return it;
    }
  }

  return buffers.end();
}

size_t
buffers::size() const noexcept
{
  size_t size = 0;

  for (const buffer &buffer : buffers_) {
    size += buffer.size();
  }

  return size;
}

bool
buffers::empty() const noexcept
{
  return size() == 0;
}

uint8_t buffers::operator[](size_t index) const noexcept
{
  assert(index < size());

  auto it = buffers_.begin();
  while (it != buffers_.end() && index >= it->size()) {
    index -= it->size();
    it++;
  }

  return (*it)[index];
}

uint8_t buffers::operator*() const noexcept
{
  return operator[](0);
}

buffer
buffers::slice(size_t size)
{
  assert(size <= this->size());

  auto begin = find_first_not_empty(buffers_);
  assert(begin != buffers_.end());

  auto end = begin;
  size_t left = size;
  while (end != buffers_.end() && left > end->size()) {
    left -= end->size();
    end++;
  }

  assert(end != buffers_.end());

  if (begin == end) {
    return begin->slice(left);
  }

  buffer result = concat(begin, end, left);

  consume(result.size());

  return result;
}

void
buffers::push(buffer buffer)
{
  buffers_.emplace_back(std::move(buffer));
}

const buffer &
buffers::front() const noexcept
{
  return buffers_.front();
}

const buffer &
buffers::back() const noexcept
{
  return buffers_.back();
}

buffer &
buffers::front() noexcept
{
  return buffers_.front();
}

buffer &
buffers::back() noexcept
{
  return buffers_.back();
}

void
buffers::consume(size_t size) noexcept
{
  assert(size <= this->size());

  for (auto it = buffers_.begin(); size != 0; it++) {
    size_t to_consume = std::min(size, it->size());
    it->consume(to_consume);
    size -= to_consume;
  }

  auto it = buffers_.begin();
  while (it != buffers_.end() && it->empty()) {
    it = buffers_.erase(it);
  }
}

size_t
buffers::consumed() const noexcept
{
  size_t consumed = 0;

  for (const buffer &buffer : buffers_) {
    consumed += buffer.consumed();
  }

  return consumed;
}

buffer
buffers::concat(std::list<buffer>::iterator start,
                std::list<buffer>::iterator end,
                size_t left) const
{
  size_t size = 0;
  for (auto it = start; it != end; it++) {
    size += it->size();
  }

  size += left;

  buffer result(size);
  size_t offset = 0;

  for (auto it = start; it != end; it++) {
    std::copy_n(it->data(), it->size(), result.data() + offset);
    offset += it->size();
  }

  std::copy_n(end->data(), left, result.data() + offset);

  return result;
}

buffers::lookahead::lookahead(const buffers &buffers) noexcept
  : buffers_(buffers)
{
}

buffers::lookahead::lookahead(const lookahead &other) noexcept
  : buffers_(other.buffers_)
  , previous_(other.previous_ + other.position_)
{
}

size_t
buffers::lookahead::size() const noexcept
{
  return buffers_.size() - position_ - previous_;
}

bool
buffers::lookahead::empty() const noexcept
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

void
buffers::lookahead::consume(size_t size) noexcept
{
  assert(size <= this->size());
  position_ += size;
}

size_t
buffers::lookahead::consumed() const noexcept
{
  return position_;
}

buffer
buffers::lookahead::copy(size_t size) const
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
