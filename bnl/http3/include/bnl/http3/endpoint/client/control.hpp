#pragma once

#include <bnl/http3/endpoint/shared/control.hpp>

#include <bnl/http3/export.hpp>

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace endpoint {
namespace client {
namespace control {

class BNL_HTTP3_EXPORT sender : public endpoint::shared::control::sender {
public:
  explicit sender(const log::api *logger) noexcept;

  BNL_MOVE_ONLY(sender);
};

class BNL_HTTP3_EXPORT receiver : public endpoint::shared::control::receiver {
public:
  explicit receiver(const log::api *logger) noexcept;

  BNL_MOVE_ONLY(receiver);

private:
  event process(frame frame, std::error_code &ec) noexcept final;

  const log::api *logger_;
};

} // namespace control
} // namespace client
} // namespace endpoint
} // namespace http3
} // namespace bnl