#include <bnl/base/buffer.hpp>

#include <algorithm>
#include <cassert>

namespace bnl {
namespace base {

buffer::buffer() noexcept : type_(type::sso), sso_() {} // NOLINT

buffer::buffer(size_t size) // NOLINT
    : type_(size <= SSO_THRESHOLD ? type::sso : type::unique), size_(size)
{
  switch (type_) {
    case type::sso:
      new (&sso_) decltype(sso_)();
      break;
    case type::unique:
      new (&unique_) decltype(unique_)(new uint8_t[size]);
      break;
    default:
      assert(false);
      break;
  }
}

buffer::buffer(const uint8_t *data, size_t size) // NOLINT
    : buffer(size)
{
  std::copy_n(data, size, this->data());
}

buffer::buffer(const buffer &other) : buffer()
{
  operator=(other);
}

buffer &buffer::operator=(const buffer &other)
{
  if (&other != this) {
    destroy();

    type_ = other.size() <= SSO_THRESHOLD ? type::sso : type::unique;
    size_ = other.size();
    position_ = 0;

    switch (type_) {
      case type::sso:
        new (&sso_) decltype(sso_)();
        break;
      default:
        new (&unique_) decltype(unique_)(new uint8_t[size_]);
        break;
    }

    std::copy_n(other.data(), size_, data());

    type_ = other.type_;
    size_ = other.size_;
    position_ = other.position_;
  }

  return *this;
}

buffer::buffer(buffer &&other) noexcept : buffer()
{
  operator=(std::move(other));
}

buffer &buffer::operator=(buffer &&other) noexcept
{
  if (&other != this) {
    destroy();

    type_ = other.type_;
    size_ = other.size_;
    position_ = other.position_;

    switch (type_) {
      case type::sso:
        new (&sso_) decltype(sso_)(other.sso_);
        break;
      case type::unique:
        new (&unique_) decltype(unique_)(std::move(other.unique_));
        break;
      case type::shared:
        new (&shared_) decltype(shared_)(std::move(other.shared_));
        break;
    }

    other.size_ = 0;
    other.position_ = 0;
  }

  return *this;
}

buffer::~buffer() noexcept
{
  destroy();
}

uint8_t *buffer::data() noexcept
{
  switch (type_) {
    case type::sso:
      return sso_.data() + position_;
    case type::unique:
      return unique_.get() + position_;
    case type::shared:
      return static_cast<uint8_t *>(shared_.get()) + position_;
  }

  assert(false);
  return nullptr;
}

const uint8_t *buffer::data() const noexcept
{
  switch (type_) {
    case type::sso:
      return sso_.data() + position_;
    case type::unique:
      return unique_.get() + position_;
    case type::shared:
      return static_cast<uint8_t *>(shared_.get()) + position_;
  }

  assert(false);
  return nullptr;
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

size_t buffer::size() const noexcept
{
  return size_ - position_;
}

bool buffer::empty() const noexcept
{
  return size() == 0;
}

const uint8_t *buffer::begin() const noexcept
{
  return data();
}

const uint8_t *buffer::end() const noexcept
{
  return data() + size();
}

uint8_t *buffer::begin() noexcept
{
  return data();
}

uint8_t *buffer::end() noexcept
{
  return data() + size();
}

void buffer::consume(size_t size) noexcept
{
  assert(size <= this->size());
  position_ += size;
}

size_t buffer::consumed() const noexcept
{
  return position_;
}

buffer buffer::slice(size_t size) noexcept
{
  assert(size <= this->size());

  if (size == 0) {
    return buffer();
  }

  if (size <= SSO_THRESHOLD || type_ == type::sso) {
    buffer result = buffer(data(), size);
    consume(size);
    return result;
  }

  if (type_ == type::unique) {
    upgrade();
  }

  buffer result = buffer(std::shared_ptr<uint8_t>(shared_, data()), size);

  consume(size);

  return result;
}

buffer buffer::copy(size_t size) noexcept
{
  assert(size <= this->size());
  return buffer(data(), size);
}

buffer::operator buffer_view() const noexcept
{
  return { data(), size() };
}

buffer buffer::concat(const buffer &first, const buffer &second)
{
  buffer result(first.size() + second.size());

  std::copy_n(first.data(), first.size(), result.data());
  std::copy_n(second.data(), second.size(), result.data() + first.size());

  return result;
}

buffer::buffer(std::shared_ptr<uint8_t> data, size_t size) noexcept // NOLINT
    : type_(type::shared), size_(size), shared_(std::move(data))
{}

void buffer::upgrade() noexcept
{
  assert(type_ == type::unique);

  std::unique_ptr<uint8_t[]> temp = std::move(unique_);
  unique_.~unique_ptr();
  new (&shared_) decltype(shared_)(temp.release(), temp.get_deleter());
  type_ = type::shared;
}

void buffer::destroy() noexcept
{
  switch (type_) {
    case type::sso:
      sso_.~array();
      break;
    case type::unique:
      unique_.~unique_ptr();
      break;
    case type::shared:
      shared_.~shared_ptr();
      break;
  }

  position_ = 0;
  size_ = 0;
}

buffer::lookahead::lookahead(const buffer &buffer) noexcept : buffer_(buffer) {}

size_t buffer::lookahead::size() const noexcept
{
  return buffer_.size() - position_;
}

bool buffer::lookahead::empty() const noexcept
{
  return size() == 0;
}

uint8_t buffer::lookahead::operator[](size_t index) const noexcept
{
  assert(index < size());
  return buffer_[position_ + index];
}

uint8_t buffer::lookahead::operator*() const noexcept
{
  return buffer_[position_];
}

void buffer::lookahead::consume(size_t size) noexcept
{
  assert(size <= this->size());
  position_ += size;
}

size_t buffer::lookahead::consumed() const noexcept
{
  return position_;
}

buffer buffer::lookahead::copy(size_t size) const
{
  assert(size <= this->size());
  return buffer(buffer_.data() + position_, size);
}

} // namespace base
} // namespace bnl
