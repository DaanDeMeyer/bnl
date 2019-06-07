#include <h3c/buffer.hpp>

#include <algorithm>
#include <cassert>

namespace h3c {

buffer::buffer() noexcept : type_(type::static_), static_() {} // NOLINT

buffer::buffer(std::unique_ptr<uint8_t[]> data, size_t size) noexcept // NOLINT
    : type_(size <= SSO_THRESHOLD ? type::sso : type::unique), size_(size)
{
  switch (type_) {
    case type::sso:
      new (&sso_) decltype(sso_)();
      std::copy_n(data.get(), size, sso_.begin());
      break;
    case type::unique:
      new (&unique_) decltype(unique_)(std::move(data));
      break;
    default:
      assert(false);
      break;
  }
}

buffer::buffer(std::shared_ptr<uint8_t> data, size_t size) noexcept // NOLINT
    : type_(size <= SSO_THRESHOLD ? type::sso : type::shared), size_(size)
{
  switch (type_) {
    case type::sso:
      new (&sso_) decltype(sso_)();
      std::copy_n(data.get(), size, sso_.begin());
      break;
    case type::shared:
      new (&shared_) decltype(shared_)(std::move(data));
      break;
    default:
      assert(0);
      break;
  }
}

buffer::buffer(const char *static_, size_t size) noexcept // NOLINT
    : type_(type::static_), static_(static_), size_(size)
{}

buffer::buffer(const uint8_t *data, size_t size) noexcept // NOLINT
    : type_(type::sso), sso_(), size_(size)
{
  assert(size < SSO_THRESHOLD);
  std::copy_n(data, size, sso_.begin());
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
      upgrade();
#pragma GCC diagnostic pop
    case type::shared:
      uint8_t *data = shared_.get() + position_;
      return buffer(std::shared_ptr<uint8_t>(shared_, data), size);
  }

  assert(false);
  return {};
}

void buffer::advance(size_t size) noexcept
{
  assert(size <= this->size());
  position_ += size;
}

void buffer::reset(const uint8_t *position) noexcept
{
  const uint8_t *data = this->data() - position_;
  assert(position >= data);
  assert(position <= data + size_);
  position_ = position - data;
}

void buffer::upgrade() const noexcept
{
  assert(type_ == type::unique);

  auto temp = std::move(unique_);
  unique_.~unique_ptr();
  new (&shared_) decltype(shared_)(temp.release(),
                                   std::default_delete<uint8_t[]>());
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

mutable_buffer::mutable_buffer(size_t size) // NOLINT
    : buffer(std::unique_ptr<uint8_t[]>(new uint8_t[size]), size)
{}

uint8_t *mutable_buffer::data() noexcept
{
  switch (type_) {
    case type::static_:
      assert(false);
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

uint8_t &mutable_buffer::operator[](size_t index) noexcept
{
  return *(data() + index);
}

uint8_t &mutable_buffer::operator*() noexcept
{
  return *data();
}

uint8_t *mutable_buffer::begin() noexcept
{
  return data();
}

uint8_t *mutable_buffer::end() noexcept
{
  return data() + size();
}

} // namespace h3c
