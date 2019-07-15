#pragma once

#include <bnl/http3/endpoint/shared/control.hpp>
#include <bnl/http3/export.hpp>

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace endpoint {
namespace server {
namespace control {

class BNL_HTTP3_EXPORT sender : public shared::control::sender {
public:
  explicit sender(const log::api *logger) noexcept;
};

class BNL_HTTP3_EXPORT receiver : public shared::control::receiver {
public:
  explicit receiver(const log::api *logger) noexcept;

private:
  base::result<event> process(frame frame) noexcept final;

private:
  const log::api *logger_;
};

} // namespace control
} // namespace server
} // namespace endpoint
} // namespace http3
} // namespace bnl
