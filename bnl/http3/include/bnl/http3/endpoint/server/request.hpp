#pragma once

#include <bnl/http3/endpoint/shared/request.hpp>
#include <bnl/http3/export.hpp>

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace endpoint {
namespace server {
namespace request {

class BNL_HTTP3_EXPORT sender : public shared::request::sender
{
public:
  using shared::request::sender::sender;
};

class BNL_HTTP3_EXPORT receiver : public shared::request::receiver
{
public:
  receiver(uint64_t id, const log::api* logger) noexcept;

private:
  base::result<event> process(frame frame) noexcept final;

private:
  const log::api* logger_;
};

} // namespace request
} // namespace server
} // namespace endpoint
} // namespace http3
} // namespace bnl
