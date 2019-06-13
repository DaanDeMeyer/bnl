#include <bnl/buffers.hpp>

#include <algorithm>
#include <cassert>

namespace bnl {

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

buffer buffers::slice(size_t size) const
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

  return concat(start, end, left);
}

void buffers::push(buffer buffer)
{
  buffers_.emplace_back(std::move(buffer));
}

void buffers::consume(size_t size) noexcept
{
  assert(size <= this->size());

  for (size_t i = 0; size != 0; i++) {
    size_t to_consume = std::min(size, buffers_[i].size());
    buffers_[i].consume(to_consume);
    size -= to_consume;
  }
}

buffers &buffers::operator+=(size_t size) noexcept
{
  consume(size);
  return *this;
}

size_t buffers::consumed() const noexcept
{
  size_t consumed = 0;

  for (const buffer &buffer : buffers_) {
    consumed += buffer.consumed();
  }

  return consumed;
}

void buffers::undo(size_t size) noexcept
{
  assert(size <= this->consumed());

  for (size_t i = buffers_.size() - 1; size != 0; i--) {
    size_t to_undo = std::min(buffers_[i].consumed(), size);
    buffers_[i].undo(to_undo);
    size -= to_undo;
  }
}

void buffers::discard()
{
  while (!buffers_.empty() && buffers_.front().empty()) {
    buffers_.pop_front();
  }
}

buffer buffers::concat(size_t start, size_t end, size_t left) const
{
  if (start == end) {
    return buffers_[start].slice(left);
  }

  size_t size = 0;
  for (size_t i = start; i < end; i++) {
    size += buffers_[i].size();
  }

  size += left;

  mutable_buffer result(size);
  size_t offset = 0;

  for (size_t i = start; i < end; i++) {
    std::copy_n(buffers_[i].data(), buffers_[i].size(), result.data() + offset);
    offset += buffers_[i].size();
  }

  std::copy_n(buffers_[end].data(), left, result.data() + offset);

  return std::move(result);
}

buffers::anchor::anchor(buffers &buffers) noexcept
    : buffers_(buffers), position_(buffers.consumed())
{}

void buffers::anchor::relocate() noexcept
{
  position_ = buffers_.consumed();
}

void buffers::anchor::release() noexcept
{
  released_ = true;
}

buffers::anchor::~anchor() noexcept
{
  if (!released_) {
    buffers_.undo(buffers_.consumed() - position_);
  }
}

buffers::discarder::discarder(buffers &buffers) noexcept : buffers_(buffers) {}

buffers::discarder::~discarder() noexcept
{
  buffers_.discard();
}

} // namespace bnl
