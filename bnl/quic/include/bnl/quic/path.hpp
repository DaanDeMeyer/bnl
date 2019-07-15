#pragma once

#include <bnl/base/string.hpp>
#include <bnl/base/string_view.hpp>
#include <bnl/ip/endpoint.hpp>
#include <bnl/quic/export.hpp>

namespace bnl {
namespace quic {

class BNL_QUIC_EXPORT path {
public:
  path() = default;
  path(ip::endpoint local, ip::endpoint remote) noexcept;

  path(const path &other) = default;
  path &operator=(const path &other) = default;

  ip::endpoint local() const noexcept;
  ip::endpoint remote() const noexcept;

private:
  ip::endpoint local_;
  ip::endpoint remote_;
};

} // namespace quic
} // namespace bnl
