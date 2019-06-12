#pragma once

#include <h3c/export.hpp>
#include <h3c/util/class.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace h3c {

class buffer_view {
public:
  template <size_t Size>
  buffer_view(const char (&data)[Size]) noexcept // NOLINT
      : buffer_view(static_cast<const char *>(data), Size - 1)
  {}

  H3C_EXPORT buffer_view(const uint8_t *data, size_t size) noexcept;
  H3C_EXPORT buffer_view(const char *data, size_t size) noexcept;

  H3C_EXPORT const uint8_t *data() const noexcept;
  H3C_EXPORT size_t size() const noexcept;

  H3C_EXPORT const uint8_t *begin() const noexcept;
  H3C_EXPORT const uint8_t *end() const noexcept;

private:
  const uint8_t *data_;
  size_t size_;
};

H3C_EXPORT bool operator==(buffer_view lhs, buffer_view rhs) noexcept;
H3C_EXPORT bool operator!=(buffer_view lhs, buffer_view rhs) noexcept;

class buffer {
public:
  H3C_EXPORT buffer() noexcept;

  class anchor;

  template <size_t Size>
  buffer(const char (&static_)[Size]) noexcept // NOLINT
      : buffer(static_cast<const char *>(static_), Size - 1)
  {}

  H3C_EXPORT buffer(std::unique_ptr<uint8_t[]> data, size_t size) noexcept;
  H3C_EXPORT buffer(std::shared_ptr<uint8_t> data, size_t size) noexcept;

  H3C_EXPORT buffer(const buffer &other) noexcept;
  H3C_EXPORT buffer &operator=(const buffer &other) noexcept;

  H3C_EXPORT buffer(buffer &&other) noexcept;
  H3C_EXPORT buffer &operator=(buffer &&other) noexcept;

  H3C_EXPORT ~buffer() noexcept;

  H3C_EXPORT const uint8_t *data() const noexcept;
  H3C_EXPORT uint8_t operator[](size_t index) const noexcept;
  H3C_EXPORT uint8_t operator*() const noexcept;

  H3C_EXPORT size_t size() const noexcept;
  H3C_EXPORT bool empty() const noexcept;

  H3C_EXPORT const uint8_t *begin() const noexcept;
  H3C_EXPORT const uint8_t *end() const noexcept;

  H3C_EXPORT buffer slice(size_t size) const noexcept;

  H3C_EXPORT void consume(size_t size) noexcept;
  H3C_EXPORT buffer &operator+=(size_t size) noexcept;

  H3C_EXPORT size_t consumed() const noexcept;

  H3C_EXPORT void undo(size_t size) noexcept;

  H3C_EXPORT static buffer concat(const buffer &first, const buffer &second);

  H3C_EXPORT operator buffer_view() const noexcept; // NOLINT

protected:
  H3C_EXPORT explicit buffer(size_t size) noexcept;

  H3C_EXPORT uint8_t *data_mut() noexcept;

private:
  H3C_EXPORT buffer(const char *static_, size_t size) noexcept;
  H3C_EXPORT buffer(const uint8_t *data, size_t size) noexcept;

  void upgrade() const noexcept;

  void destroy() noexcept;

private:
  enum class type { static_, sso, unique, shared };

  static constexpr size_t SSO_THRESHOLD = 10;

  mutable type type_;
  size_t size_ = 0;
  size_t position_ = 0;

  union {
    const char *static_;
    std::array<uint8_t, SSO_THRESHOLD> sso_;
    mutable std::unique_ptr<uint8_t[]> unique_;
    // `std::shared_ptr<uint8_t[]>` requires C++17.
    mutable std::shared_ptr<uint8_t> shared_;
  };
};

class buffer::anchor {
public:
  H3C_EXPORT explicit anchor(buffer &buffer) noexcept;

  H3C_NO_COPY(anchor);
  H3C_NO_MOVE(anchor);

  H3C_EXPORT ~anchor() noexcept;

  H3C_EXPORT void relocate() noexcept;

  H3C_EXPORT void release() noexcept;

private:
  buffer &buffer_;

  bool released_ = false;
  size_t position_;
};


class mutable_buffer : public buffer {
public:
  mutable_buffer() = default;
  H3C_EXPORT explicit mutable_buffer(size_t size);

  H3C_MOVE_ONLY(mutable_buffer);

  ~mutable_buffer() = default;

  template <size_t Size>
  mutable_buffer(const char (&static_)[Size]) noexcept = delete;

  mutable_buffer slice(size_t) = delete;

  H3C_EXPORT uint8_t *data() noexcept;
  H3C_EXPORT uint8_t &operator[](size_t index) noexcept;
  H3C_EXPORT uint8_t &operator*() noexcept;

  H3C_EXPORT uint8_t *end() noexcept;
};

} // namespace h3c
