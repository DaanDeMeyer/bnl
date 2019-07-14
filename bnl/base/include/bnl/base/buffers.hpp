#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/base/export.hpp>
#include <bnl/base/macro.hpp>

#include <deque>

namespace bnl {
namespace base {

class BNL_BASE_EXPORT buffers
{
public:
  class lookahead;

  using lookahead_type = lookahead;

  buffers() = default;

  size_t size() const noexcept;
  bool empty() const noexcept;

  uint8_t operator[](size_t index) const noexcept;
  uint8_t operator*() const noexcept;

  buffer slice(size_t size);

  void push(buffer buffer);

  const buffer& front() const noexcept;
  const buffer& back() const noexcept;

  buffer& front() noexcept;
  buffer& back() noexcept;

  void consume(size_t size) noexcept;
  size_t consumed() const noexcept;

private:
  std::deque<buffer> buffers_;

  void discard() noexcept;

  buffer concat(size_t start, size_t end, size_t left) const;
};

class BNL_BASE_EXPORT buffers::lookahead
{
public:
  using lookahead_type = lookahead;

  lookahead(const buffers& buffers) noexcept; // NOLINT

  lookahead(const lookahead& other) noexcept;
  lookahead& operator=(const lookahead&) = delete;

  BNL_BASE_NO_MOVE(lookahead);

  ~lookahead() = default;

  size_t size() const noexcept;
  bool empty() const noexcept;

  uint8_t operator[](size_t index) const noexcept;
  uint8_t operator*() const noexcept;

  void consume(size_t size) noexcept;
  size_t consumed() const noexcept;

  buffer copy(size_t size) const;

private:
  const buffers& buffers_;
  size_t previous_ = 0;
  size_t position_ = 0;
};

} // namespace base
} // namespace bnl
