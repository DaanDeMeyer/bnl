#include <bnl/http3/server/stream/request.hpp>

#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/util/error.hpp>

namespace bnl {
namespace http3 {
namespace server {
namespace stream {
namespace request {

receiver::receiver(uint64_t id, const log::api *logger) noexcept
  : endpoint::stream::request::receiver(id, logger)
  , logger_(logger)
{}

result<event>
receiver::process(frame frame) noexcept
{
  switch (frame) {
    case frame::type::priority:
      if (headers().started()) {
        THROW(http3::connection::error::unexpected_frame);
      }
      // TODO: Implement PRIORITY
      THROW(error::not_implemented);
    case frame::type::headers:
    case frame::type::push_promise:
    case frame::type::duplicate_push:
      THROW(http3::connection::error::unexpected_frame);
    default:
      THROW(http3::connection::error::internal);
  }
}

}
}
}
}
}
