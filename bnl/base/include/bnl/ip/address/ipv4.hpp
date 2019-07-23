#pragma once

#include <bnl/base/buffer_view.hpp>
#include <bnl/base/export.hpp>

#include <array>
#include <cstdint>
#include <iosfwd>

namespace bnl {
namespace ipv4 {

class BNL_BASE_EXPORT address {
public:
  static constexpr size_t size = 4;

  address() = default;
  explicit address(base::buffer_view bytes) noexcept;
  explicit address(const uint8_t *bytes) noexcept;
  explicit address(uint32_t bytes) noexcept;

  address(const address &) = default;
  address &operator=(const address &) = default;

  base::buffer_view bytes() const noexcept;

private:
  std::array<uint8_t, size> bytes_ = {};
};

BNL_BASE_EXPORT
std::ostream &
operator<<(std::ostream &os, const address &address);

}
}
