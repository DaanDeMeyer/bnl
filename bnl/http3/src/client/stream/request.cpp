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

base::result<event>
receiver::process(frame frame) noexcept
{
  switch (frame) {
    case frame::type::push_promise:
    case frame::type::duplicate_push:
      // TODO: Implement PUSH_PROMISE
      // TODO: Implement DUPLICATE_PUSH
      THROW(base::error::not_implemented);
    case frame::type::headers:
    case frame::type::priority:
      THROW(error::unexpected_frame);
    default:
      THROW(error::internal_error);
  }
}

}
}
}
}
}
