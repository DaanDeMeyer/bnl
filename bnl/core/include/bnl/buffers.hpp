#pragma once

#include <bnl/buffer.hpp>

#include <bnl/core/export.hpp>

#include <bnl/class/macro.hpp>

#include <deque>

namespace bnl {

class buffers_view;

class BNL_CORE_EXPORT buffers {
public:
  using view = buffers_view;

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

} // namespace bnl

#include <bnl/buffers_view.hpp>
