#include <bnl/http3/server/stream/request.hpp>

namespace bnl {
namespace http3 {
namespace server {
namespace stream {
namespace request {

receiver::receiver(uint64_t id) noexcept
  : endpoint::stream::request::receiver(id)
{}

result<event>
receiver::process(frame frame) noexcept
{
  switch (frame) {
    case frame::type::priority:
      if (headers().started()) {
        return error::unexpected_frame;
      }
      // TODO: Implement PRIORITY
      return error::not_implemented;
    case frame::type::headers:
    case frame::type::push_promise:
    case frame::type::duplicate_push:
      return error::unexpected_frame;
    default:
      return error::internal;
  }
}

}
}
}
}
}
