#pragma once

#include <bnl/http3/endpoint/shared/request.hpp>

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace endpoint {
namespace server {
namespace request {

class BNL_HTTP3_EXPORT sender : public shared::request::sender {
public:
  using shared::request::sender::sender;

  BNL_BASE_MOVE_ONLY(sender);
};

class BNL_HTTP3_EXPORT receiver : public shared::request::receiver {
public:
  receiver(uint64_t id, const log::api *logger) noexcept;

  BNL_BASE_MOVE_ONLY(receiver);

private:
  event process(frame frame, std::error_code &ec) noexcept final;

  const log::api *logger_;
};

} // namespace request
} // namespace server
} // namespace endpoint
} // namespace http3
} // namespace bnl
