#pragma once

#include <bnl/base/macro.hpp>
#include <bnl/http3/endpoint/shared/request.hpp>
#include <bnl/http3/export.hpp>

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace endpoint {
namespace client {
namespace request {

class BNL_HTTP3_EXPORT sender : public shared::request::sender
{
public:
  using shared::request::sender::sender;

  BNL_BASE_MOVE_ONLY(sender);
};

class BNL_HTTP3_EXPORT receiver : public shared::request::receiver
{
public:
  receiver(uint64_t id, const log::api* logger) noexcept;

  BNL_BASE_MOVE_ONLY(receiver);

private:
  base::result<event> process(frame frame) noexcept final;

private:
  const log::api* logger_;
};

} // namespace request
} // namespace client
} // namespace endpoint
} // namespace http3
} // namespace bnl
