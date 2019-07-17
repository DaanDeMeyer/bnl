#include <bnl/base/buffer.hpp>

#include <algorithm>
#include <cassert>

namespace bnl {
namespace base {

buffer::buffer(size_t size) // NOLINT
{
  init(size);
}

buffer::buffer(const uint8_t *data, size_t size)
  : buffer(size)
{
  std::copy_n(data, size, begin());
}

buffer::buffer(const char *data, size_t size)
  : buffer(size)
{
  std::copy_n(data, size, begin());
}

buffer::buffer(buffer_view data) noexcept
  : buffer(data.data(), data.size())
{}

buffer::buffer(const buffer &other)
  : buffer(other.data(), other.size())
{}

buffer &
buffer::operator=(const buffer &other)
{
  if (&other != this) {
    if (other.size() > size()) {
      destroy();
      init(other.size());
    } else {
      end_ = begin_ + other.size();
    }

    std::copy(other.begin(), other.end(), begin());
  }

  return *this;
}

void
buffer::init(size_t size)
{
  rc_ = sso(size)
          ? nullptr
          // Allocate 4 extra bytes to store the reference count.
          : reinterpret_cast<uint32_t *>(new uint8_t[sizeof(uint32_t) + size]);
  begin_ = sso(size) ? sso_
                     // The reference count is stored at the begin of the buffer
                     // so the actual data starts at a 4 byte offset.
                     : reinterpret_cast<uint8_t *>(rc_) + sizeof(uint32_t);
  end_ = begin_ + size;

  if (rc_ != nullptr) {
    // Initialize the reference count.
    (*rc_) = 1;
  }
}

buffer::buffer(buffer &&other) noexcept
  : buffer()
{
  operator=(std::move(other));
}

buffer &
buffer::operator=(buffer &&other) noexcept
{
  if (&other != this) {
    destroy();

    if (other.sso()) {
      begin_ = sso_;
      end_ = begin_ + other.size();
      std::copy(other.begin(), other.end(), begin_);
    } else {
      rc_ = other.rc_;
      begin_ = other.begin_;
      end_ = other.end_;
    }

    other.rc_ = nullptr;
    other.begin_ = nullptr;
    other.end_ = nullptr;
  }

  return *this;
}

buffer::~buffer() noexcept
{
  destroy();
}

uint8_t *
buffer::data() noexcept
{
  return begin();
}

const uint8_t *
buffer::data() const noexcept
{
  return begin();
}

uint8_t buffer::operator[](size_t index) const noexcept
{
  assert(index < size());
  return *(data() + index);
}

uint8_t buffer::operator*() const noexcept
{
  return *data();
}

uint8_t &buffer::operator[](size_t index) noexcept
{
  assert(index < size());
  return *(data() + index);
}

uint8_t &buffer::operator*() noexcept
{
  return *data();
}

size_t
buffer::size() const noexcept
{
  return static_cast<size_t>(end() - begin());
}

bool
buffer::empty() const noexcept
{
  return size() == 0;
}

const uint8_t *
buffer::begin() const noexcept
{
  return begin_;
}

const uint8_t *
buffer::end() const noexcept
{
  return end_;
}

uint8_t *
buffer::begin() noexcept
{
  return begin_;
}

uint8_t *
buffer::end() noexcept
{
  return end_;
}

void
buffer::consume(size_t size) noexcept
{
  assert(size <= this->size());
  begin_ += size;
}

void
buffer::destroy() noexcept
{
  if (rc_ == nullptr) {
    return;
  }

  (*rc_)--;
  if (*rc_ == 0) {
    // The reference count pointer conveniently also points to the start of the
    // allocated array.
    delete[] reinterpret_cast<uint8_t *>(rc_); // NOLINT
    rc_ = nullptr;
  }
}

buffer
buffer::slice(size_t size) noexcept
{
  assert(size <= this->size());

  if (size == 0) {
    return buffer();
  }

  buffer result;

  if (sso(size)) {
    result = buffer(data(), size);
  } else {
    result = buffer(rc_, data(), data() + size);
  }

  consume(size);

  return result;
}

buffer::operator buffer_view() const noexcept
{
  return { data(), size() };
}

buffer
buffer::concat(const buffer &first, const buffer &second)
{
  buffer result(first.size() + second.size());

  std::copy_n(first.data(), first.size(), result.data());
  std::copy_n(second.data(), second.size(), result.data() + first.size());

  return result;
}

buffer::buffer(uint32_t *rc, uint8_t *begin, uint8_t *end) noexcept // NOLINT
  : rc_(rc)
  , begin_(begin)
  , end_(end)
{
  // Increment the reference count.
  (*rc_)++;
}

bool
buffer::sso() const noexcept
{
  return rc_ == nullptr;
}

bool
buffer::sso(size_t size) noexcept
{
  return size <= SSO_THRESHOLD;
}

buffer::lookahead::lookahead(const buffer &buffer) noexcept
  : buffer_(buffer)
{}

buffer::lookahead::lookahead(const lookahead &other) noexcept
  : buffer_(other.buffer_)
  , previous_(other.previous_ + other.position_)
{}

size_t
buffer::lookahead::size() const noexcept
{
  return buffer_.size() - position_ - previous_;
}

bool
buffer::lookahead::empty() const noexcept
{
  return size() == 0;
}

uint8_t buffer::lookahead::operator[](size_t index) const noexcept
{
  assert(index < size());
  return buffer_[previous_ + position_ + index];
}

uint8_t buffer::lookahead::operator*() const noexcept
{
  return buffer_[previous_ + position_];
}

void
buffer::lookahead::consume(size_t size) noexcept
{
  assert(size <= this->size());
  position_ += size;
}

size_t
buffer::lookahead::consumed() const noexcept
{
  return position_;
}

} // namespace base
} // namespace bnl
