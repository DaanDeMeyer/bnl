#include <bnl/http3/client/stream/request.hpp>

#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>

namespace bnl {
namespace http3 {
namespace client {
namespace stream {
namespace request {

receiver::receiver(uint64_t id) noexcept
  : endpoint::stream::request::receiver(id)
{}

result<event>
receiver::process(frame frame) noexcept
{
  switch (frame) {
    case frame::type::push_promise:
    case frame::type::duplicate_push:
      // TODO: Implement PUSH_PROMISE
      // TODO: Implement DUPLICATE_PUSH
      return error::not_implemented;
    case frame::type::headers:
    case frame::type::priority:
      return http3::connection::error::unexpected_frame;
    default:
      return http3::connection::error::internal;
  }
}

}
}
}
}
}
