#pragma once

#include <bnl/core/export.hpp>

#include <bnl/buffer.hpp>

#include <cstddef>
#include <cstdint>

namespace bnl {

class buffers;

class BNL_CORE_EXPORT buffers_view {
public:
  buffers_view(const buffers &buffers) noexcept; // NOLINT

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
