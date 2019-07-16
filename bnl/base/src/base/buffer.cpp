#include <bnl/base/buffer.hpp>

#include <algorithm>
#include <cassert>

namespace bnl {
namespace base {

buffer::buffer() noexcept
  : sso_()
{}

buffer::buffer(size_t size) // NOLINT
{
  init(size);
}

buffer::buffer(const uint8_t *data, size_t size)
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
    destroy();
    init(other.size());
    std::copy_n(other.data(), other.size(), begin());
  }

  return *this;
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

    bool sso = other.size() <= SSO_THRESHOLD;

    if (sso) {
      new (&sso_) decltype(sso_)(other.sso_);
      begin_ = sso_.data();
    } else {
      new (&shared_) decltype(shared_)(std::move(other.shared_));
      begin_ = shared_.get();
    }

    end_ = begin() + other.size();

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
  bool sso = this->sso();

  assert(size <= this->size());
  begin_ += size;

  // If we weren't small-size-optimized before but are small enough to be now,
  // we do so.
  if (!sso && this->sso()) {
    std::array<uint8_t, SSO_THRESHOLD> tmp; // NOLINT
    std::copy_n(this->data(), this->size(), tmp.data());
    size_t tmp_size = this->size();

    shared_.~shared_ptr();
    new (&sso_) decltype(sso_)(tmp);

    begin_ = sso_.data();
    end_ = begin_ + tmp_size;
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

  if (sso() || size <= SSO_THRESHOLD) {
    result = buffer(data(), size);
  } else {
    result = buffer(std::shared_ptr<uint8_t>(shared_, data()), size);
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

buffer::buffer(std::shared_ptr<uint8_t> data, size_t size) noexcept // NOLINT
  : begin_(data.get())
  , end_(begin() + size)
  , shared_(std::move(data))
{}

bool
buffer::sso() const noexcept
{
  return size() <= SSO_THRESHOLD;
}

void
buffer::init(size_t size)
{
  if (size <= SSO_THRESHOLD) {
    new (&sso_) decltype(sso_)();
    begin_ = sso_.begin();
  } else {
    new (&shared_) decltype(shared_)(new uint8_t[size]);
    begin_ = shared_.get();
  }

  end_ = begin() + size;
}

void
buffer::destroy() noexcept
{
  if (sso()) {
    sso_.~array();
  } else {
    shared_.~shared_ptr();
  }

  begin_ = nullptr;
  end_ = nullptr;
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
