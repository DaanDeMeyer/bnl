#pragma once

#include <bnl/buffer_view.hpp>

#include <bnl/core/export.hpp>

#include <bnl/class/macro.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>

namespace bnl {

class BNL_CORE_EXPORT buffer {
public:
  class anchor;

  buffer() noexcept;

  template <size_t Size>
  buffer(const char (&static_)[Size]) noexcept // NOLINT
      : buffer(static_cast<const char *>(static_), Size - 1)
  {}

  // Use a templated constructor to avoid an ambiguous overload that occurs
  // because `buffer` provides constructors for both `std::unique_ptr` and
  // `std::shared_ptr`.
  template <typename Deleter>
  buffer(std::unique_ptr<uint8_t[], Deleter> data, size_t size) noexcept;

  buffer(std::shared_ptr<uint8_t> data, size_t size) noexcept;

public:
  buffer(const buffer &other) noexcept;
  buffer &operator=(const buffer &other) noexcept;

  BNL_CUSTOM_MOVE(buffer);

  ~buffer() noexcept;

  const uint8_t *data() const noexcept;
  uint8_t operator[](size_t index) const noexcept;
  uint8_t operator*() const noexcept;

  size_t size() const noexcept;
  bool empty() const noexcept;

  const uint8_t *begin() const noexcept;
  const uint8_t *end() const noexcept;

  buffer slice(size_t size) const noexcept;

  void consume(size_t size) noexcept;
  buffer &operator+=(size_t size) noexcept;

  size_t consumed() const noexcept;

  void undo(size_t size) noexcept;

  static buffer concat(const buffer &first, const buffer &second);

  operator buffer_view() const noexcept; // NOLINT

protected:
  explicit buffer(size_t size) noexcept;

  uint8_t *data_mut() noexcept;

private:
  buffer(const char *static_, size_t size) noexcept;
  buffer(const uint8_t *data, size_t size) noexcept;

  void upgrade() const noexcept;

  void destroy() noexcept;

private:
  enum class type { static_, sso, unique, shared };

  static constexpr size_t SSO_THRESHOLD = 10;

  mutable type type_;
  size_t size_ = 0;
  size_t position_ = 0;

  using deleter = std::function<void(uint8_t *)>;

  union {
    const char *static_;
    std::array<uint8_t, SSO_THRESHOLD> sso_;
    // Type erase `std::unique_ptr` deleter so any kind of deleter can be stored
    // in `buffer`.
    mutable std::unique_ptr<uint8_t[], deleter> unique_;
    // `std::shared_ptr<uint8_t[]>` requires C++17.
    mutable std::shared_ptr<uint8_t> shared_;
  };
};

template <typename Deleter>
buffer::buffer(std::unique_ptr<uint8_t[], Deleter> data, // NOLINT
               size_t size) noexcept
    : type_(size <= SSO_THRESHOLD ? type::sso : type::unique), size_(size)
{
  switch (type_) {
    case type::sso:
      new (&sso_) decltype(sso_)();
      for (size_t i = 0; i < size; i++) {
        sso_[i] = data[i];
      }
      break;
    case type::unique:
      new (&unique_) decltype(unique_)(std::move(data));
      break;
    default:
      break;
  }
}

class BNL_CORE_EXPORT buffer::anchor {
public:
  explicit anchor(buffer &buffer) noexcept;

  BNL_NO_COPY(anchor);
  BNL_NO_MOVE(anchor);

  ~anchor() noexcept;

  void relocate() noexcept;

  void release() noexcept;

private:
  buffer &buffer_;

  bool released_ = false;
  size_t position_;
};

class BNL_CORE_EXPORT mutable_buffer : public buffer {
public:
  mutable_buffer() = default;
  explicit mutable_buffer(size_t size);

  BNL_MOVE_ONLY(mutable_buffer);

  template <size_t Size>
  mutable_buffer(const char (&static_)[Size]) noexcept = delete;

  mutable_buffer slice(size_t) = delete;

  uint8_t *data() noexcept;
  uint8_t &operator[](size_t index) noexcept;
  uint8_t &operator*() noexcept;

  uint8_t *end() noexcept;

  operator mutable_buffer_view() noexcept;
};

} // namespace bnl
