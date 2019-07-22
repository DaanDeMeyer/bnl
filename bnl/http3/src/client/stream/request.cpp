#include <bnl/http3/client/stream/request.hpp>

#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/util/error.hpp>

namespace bnl {
namespace http3 {
namespace client {
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
    case frame::type::push_promise:
    case frame::type::duplicate_push:
      // TODO: Implement PUSH_PROMISE
      // TODO: Implement DUPLICATE_PUSH
      THROW(error::not_implemented);
    case frame::type::headers:
    case frame::type::priority:
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
