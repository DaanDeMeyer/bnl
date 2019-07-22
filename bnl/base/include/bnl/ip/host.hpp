#pragma once

#include <bnl/base/string.hpp>
#include <bnl/base/string_view.hpp>

namespace bnl {
namespace ip {

class BNL_BASE_EXPORT host {
public:
  host() = default;
  host(std::string name) noexcept; // NOLINT

  host(const host &) = default;
  host &operator=(const host &) = default;

  host(host &&) = default;
  host &operator=(host &&) = default;

  base::string_view name() const noexcept;

private:
  base::string name_;
};

}
}
