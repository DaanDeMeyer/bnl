#include <bnl/http3/endpoint/server/request.hpp>

#include <bnl/http3/error.hpp>

#include <bnl/util/error.hpp>

namespace bnl {
namespace http3 {
namespace endpoint {

server::request::receiver::receiver(uint64_t id,
                                    const log::api *logger) noexcept
    : endpoint::shared::request::receiver(id, logger), logger_(logger)
{}

event server::request::receiver::process(frame frame,
                                         std::error_code &ec) noexcept
{
  switch (frame) {
    case frame::type::priority:
      if (headers().started()) {
        THROW(error::unexpected_frame);
      }

      // TODO: Implement PRIORITY
      THROW(error::not_implemented);
    case frame::type::headers:
    case frame::type::push_promise:
    case frame::type::duplicate_push:
      THROW(error::unexpected_frame);
    default:
      THROW(error::internal_error);
  }
}

} // namespace endpoint
} // namespace http3
} // namespace bnl