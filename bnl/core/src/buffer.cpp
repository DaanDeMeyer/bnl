#include <bnl/buffer.hpp>

#include <algorithm>
#include <cassert>

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

buffer::buffer() noexcept : type_(type::static_), static_() {} // NOLINT

buffer::buffer(std::shared_ptr<uint8_t> data, size_t size) noexcept // NOLINT
    : type_(size <= SSO_THRESHOLD ? type::sso : type::shared), size_(size)
{
  switch (type_) {
    case type::sso:
      new (&sso_) decltype(sso_)();
      std::copy_n(data.get(), size, sso_.data());
      break;
    case type::shared:
      new (&shared_) decltype(shared_)(std::move(data));
      break;
    default:
      assert(false);
      break;
  }
}

buffer::buffer(const buffer &other) noexcept : buffer()
{
  operator=(other);
}

buffer &buffer::operator=(const buffer &other) noexcept
{
  if (&other != this) {
    destroy();

    switch (other.type_) {
      case type::static_:
        new (&static_) decltype(static_)(other.static_);
        break;
      case type::sso:
        new (&sso_) decltype(sso_)(other.sso_);
        break;
      case type::unique:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
        other.upgrade();
#pragma GCC diagnostic pop
      case type::shared:
        new (&shared_) decltype(shared_)(other.shared_);
        break;
    }

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

    switch (other.type_) {
      case type::static_:
        new (&static_) decltype(static_)(other.static_);
        break;
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

    type_ = other.type_;
    size_ = other.size_;
    position_ = other.position_;

    other.size_ = 0;
    other.position_ = 0;
  }

  return *this;
}

buffer::~buffer() noexcept
{
  destroy();
}

const uint8_t *buffer::data() const noexcept
{
  switch (type_) {
    case type::static_:
      return reinterpret_cast<const uint8_t *>(static_) + position_;
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
  return *(data() + index);
}

uint8_t buffer::operator*() const noexcept
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

buffer buffer::slice(size_t size) const noexcept
{
  assert(size <= this->size());

  if (size == 0) {
    return buffer();
  }

  switch (type_) {
    case type::static_:
      return buffer(data(), size);
    case type::sso:
      return buffer(data(), size);
    case type::unique:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
      if (size <= SSO_THRESHOLD) {
        return buffer(data(), size);
      }

      upgrade();
#pragma GCC diagnostic pop
    case type::shared:
      if (size <= SSO_THRESHOLD) {
        return buffer(data(), size);
      }

      uint8_t *data = shared_.get() + position_;
      return buffer(std::shared_ptr<uint8_t>(shared_, data), size);
  }

  assert(false);
  return {};
}

void buffer::consume(size_t size) noexcept
{
  assert(size <= this->size());
  position_ += size;
}

buffer &buffer::operator+=(size_t size) noexcept
{
  consume(size);
  return *this;
}

size_t buffer::consumed() const noexcept
{
  return position_;
}

void buffer::undo(size_t size) noexcept
{
  assert(size <= consumed());
  position_ -= size;
}

buffer buffer::concat(const buffer &first, const buffer &second)
{
  mutable_buffer result(first.size() + second.size());

  std::copy_n(first.data(), first.size(), result.data());
  std::copy_n(second.data(), second.size(), result.data() + first.size());

  return std::move(result);
}

buffer::operator buffer_view() const noexcept
{
  return { data(), size() };
}

buffer::buffer(size_t size) noexcept // NOLINT
    : type_(size <= SSO_THRESHOLD ? type::sso : type::unique), size_(size)
{
  switch (type_) {
    case type::sso:
      new (&sso_) decltype(sso_)();
      break;
    case type::unique:
      new (&unique_) decltype(unique_)(
          std::unique_ptr<uint8_t[]>(new uint8_t[size]));
      break;
    default:
      assert(false);
      break;
  }
}

uint8_t *buffer::data_mut() noexcept
{
  switch (type_) {
    case type::static_:
      break;
    case type::sso:
      return sso_.data() + position_;
    case type::unique:
      return unique_.get() + position_;
    case type::shared:
      return shared_.get() + position_;
  }

  assert(false);
  return nullptr;
}

buffer::buffer(const char *static_, size_t size) noexcept // NOLINT
    : type_(type::static_), size_(size), static_(static_)
{}

buffer::buffer(const uint8_t *data, size_t size) noexcept // NOLINT
    : type_(type::sso), size_(size), sso_()
{
  assert(size <= SSO_THRESHOLD);
  std::copy_n(data, size, sso_.data());
}

void buffer::upgrade() const noexcept
{
  assert(type_ == type::unique);

  std::unique_ptr<uint8_t[], deleter> temp = std::move(unique_);
  unique_.~unique_ptr();
  new (&shared_) decltype(shared_)(temp.release(), temp.get_deleter());
  type_ = type::shared;
}

void buffer::destroy() noexcept
{
  switch (type_) {
    case type::sso:
      sso_.~array();
    case type::static_:
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

buffer::anchor::anchor(buffer &buffer) noexcept
    : buffer_(buffer), position_(buffer.position_)
{}

void buffer::anchor::relocate() noexcept
{
  position_ = buffer_.position_;
}

void buffer::anchor::release() noexcept
{
  released_ = true;
}

buffer::anchor::~anchor() noexcept
{
  if (!released_) {
    buffer_.position_ = position_;
  }
}

mutable_buffer::mutable_buffer(size_t size) : buffer(size) {}

uint8_t *mutable_buffer::data() noexcept
{
  return data_mut();
}

uint8_t &mutable_buffer::operator[](size_t index) noexcept
{
  return *(data() + index);
}

uint8_t &mutable_buffer::operator*() noexcept
{
  return *data();
}

uint8_t *mutable_buffer::end() noexcept
{
  return data() + size();
}

} // namespace bnl
