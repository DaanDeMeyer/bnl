#pragma once

#include <bnl/core/export.hpp>

#include <bnl/buffer.hpp>

#include <cstddef>
#include <cstdint>

namespace bnl {

class BNL_CORE_EXPORT buffer_view {
public:
  template <size_t Size>
  buffer_view(const char (&data)[Size]) noexcept // NOLINT
      : buffer_view(reinterpret_cast<const uint8_t *>(data), Size - 1)
  {}

  buffer_view(const uint8_t *data, size_t size) noexcept;
  buffer_view(const buffer &buffer) noexcept; // NOLINT

  const uint8_t *data() const noexcept;

  size_t size() const noexcept;
  bool empty() const noexcept;

  const uint8_t *begin() const noexcept;
  const uint8_t *end() const noexcept;

  uint8_t operator[](size_t index) const noexcept;
  uint8_t operator*() const noexcept;

  void consume(size_t size) noexcept;
  size_t consumed() const noexcept;

  buffer copy(size_t size) const;

private:
  const uint8_t *data_;
  size_t size_;
  size_t position_ = 0;
};

BNL_CORE_EXPORT bool operator==(buffer_view lhs, buffer_view rhs) noexcept;
BNL_CORE_EXPORT bool operator!=(buffer_view lhs, buffer_view rhs) noexcept;

class BNL_CORE_EXPORT buffer_view_mut {
public:
  buffer_view_mut(uint8_t *data, size_t size) noexcept;

  uint8_t *data() noexcept;

  size_t size() const noexcept;
  bool empty() const noexcept;

  uint8_t *begin() noexcept;
  uint8_t *end() noexcept;

  operator buffer_view() const noexcept; // NOLINT

private:
  uint8_t *data_;
  size_t size_;
};

} // namespace bnl
