#pragma once

#include <bnl/base/string.hpp>
#include <bnl/base/string_view.hpp>

namespace bnl {
namespace ip {

class BNL_BASE_EXPORT host {
public:
  host() = default;
  host(std::string name) noexcept; // NOLINT

  host(const host &other) = default;
  host &operator=(const host &other) = default;

  host(host &&other) = default;
  host &operator=(host &&other) = default;

  base::string_view name() const noexcept;

private:
  base::string name_;
};

}
}
