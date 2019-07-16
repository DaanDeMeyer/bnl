#pragma once

#include <bnl/base/buffer_view.hpp>
#include <bnl/base/export.hpp>

#include <array>
#include <cstdint>
#include <memory>

namespace bnl {
namespace base {

class BNL_BASE_EXPORT buffer {
public:
  class lookahead;

  using lookahead_type = lookahead;

  buffer() noexcept;
  buffer(const uint8_t *data, size_t size);

  explicit buffer(size_t size);
  explicit buffer(buffer_view data) noexcept;

  template<size_t Size>
  buffer(const char (&data)[Size]) noexcept // NOLINT
    : buffer(reinterpret_cast<const uint8_t *>(data), Size - 1)
  {}

  buffer(const buffer &other);
  buffer &operator=(const buffer &other);

  buffer(buffer &&other) noexcept;
  buffer &operator=(buffer &&other) noexcept;

  ~buffer() noexcept;

  uint8_t *data() noexcept;
  const uint8_t *data() const noexcept;

  const uint8_t *begin() const noexcept;
  const uint8_t *end() const noexcept;

  uint8_t *begin() noexcept;
  uint8_t *end() noexcept;

  uint8_t operator[](size_t index) const noexcept;
  uint8_t operator*() const noexcept;

  uint8_t &operator[](size_t index) noexcept;
  uint8_t &operator*() noexcept;

  size_t size() const noexcept;
  bool empty() const noexcept;

  void consume(size_t size) noexcept;

  // Returns a buffer to the next `size` bytes of this buffer and consumes
  // `size` bytes from this buffer. Both this buffer and the returned buffer
  // will manage different slices of the same memory block.
  //
  // Conceptually, this method transfers ownership of a slice of the memory
  // block to a new buffer. Because this method consumes `size` bytes, the
  // returned buffer will be the sole owner of the slice of memory.
  buffer slice(size_t size) noexcept;

  // Copies the next `size` bytes to a new buffer and returns it. In contrast to
  // `slice`, this method does not consume any bytes of this buffer.
  buffer copy(size_t size) noexcept;

  operator buffer_view() const noexcept; // NOLINT

  static buffer concat(const buffer &first, const buffer &second);

private:
  buffer(std::shared_ptr<uint8_t> data, size_t size) noexcept;

  bool sso() const noexcept;

  void init(size_t size);

  void destroy() noexcept;

private:
  static constexpr size_t SSO_THRESHOLD = 20;

  uint8_t *begin_ = nullptr;
  uint8_t *end_ = nullptr;

  union {
    std::array<uint8_t, SSO_THRESHOLD> sso_;
    std::shared_ptr<uint8_t> shared_;
  };
};

class BNL_BASE_EXPORT buffer::lookahead {
public:
  using lookahead_type = lookahead;

  lookahead(const buffer &buffer) noexcept; // NOLINT

  lookahead(const lookahead &other) noexcept;
  const lookahead &operator=(const lookahead &other) = delete;

  lookahead(lookahead &&other) = delete;
  lookahead &operator=(lookahead &&other) = delete;

  ~lookahead() = default;

  size_t size() const noexcept;
  bool empty() const noexcept;

  uint8_t operator[](size_t index) const noexcept;
  uint8_t operator*() const noexcept;

  void consume(size_t size) noexcept;
  size_t consumed() const noexcept;

  buffer copy(size_t size) const;

private:
  const buffer &buffer_;
  size_t previous_ = 0;
  size_t position_ = 0;
};

} // namespace base
} // namespace bnl
