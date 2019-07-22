#pragma once

#include <bnl/base/export.hpp>
#include <bnl/base/string.hpp>
#include <bnl/base/string_view.hpp>
#include <bnl/ip/endpoint.hpp>

namespace bnl {
namespace quic {

class BNL_BASE_EXPORT path {
public:
  path() = default;
  path(ip::endpoint local, ip::endpoint peer) noexcept;

  path(const path &) = default;
  path &operator=(const path &) = default;

  ip::endpoint local() const noexcept;
  ip::endpoint peer() const noexcept;

private:
  ip::endpoint local_;
  ip::endpoint peer_;
};

}
}
