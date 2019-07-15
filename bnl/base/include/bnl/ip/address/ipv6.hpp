#pragma once

#include <bnl/base/buffer_view.hpp>
#include <bnl/base/export.hpp>

#include <array>
#include <cstdint>

namespace bnl {
namespace ipv6 {

class BNL_BASE_EXPORT address
{
public:
  static constexpr size_t size = 16;

  address() = default;
  explicit address(base::buffer_view bytes) noexcept;
  explicit address(const uint8_t* bytes) noexcept;

  base::buffer_view bytes() const noexcept;

private:
  std::array<uint8_t, size> bytes_ = {};
};

} // namespace ipv6
} // namespace bnl
