#pragma once

#include <bnl/buffer.hpp>

#include <bnl/base/export.hpp>

#include <bnl/class/macro.hpp>

#include <deque>

namespace bnl {

class BNL_BASE_EXPORT buffers {
public:
  class lookahead;

  buffers() = default;

  size_t size() const noexcept;
  bool empty() const noexcept;

  uint8_t operator[](size_t index) const noexcept;
  uint8_t operator*() const noexcept;

  buffer slice(size_t size);

  void push(buffer buffer);

  const buffer &front() const noexcept;
  const buffer &back() const noexcept;

  buffer &front() noexcept;
  buffer &back() noexcept;

  void consume(size_t size) noexcept;
  size_t consumed() const noexcept;

private:
  std::deque<buffer> buffers_;

  void discard() noexcept;

  buffer concat(size_t start, size_t end, size_t left) const;
};

class BNL_BASE_EXPORT buffers::lookahead {
public:
  lookahead(const buffers &buffers) noexcept; // NOLINT

  size_t size() const noexcept;
  bool empty() const noexcept;

  uint8_t operator[](size_t index) const noexcept;
  uint8_t operator*() const noexcept;

  void consume(size_t size) noexcept;
  size_t consumed() const noexcept;

  buffer copy(size_t size) const;

private:
  const buffers &buffers_;
  size_t position_ = 0;
};

} // namespace bnl
