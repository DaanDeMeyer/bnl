#include <bnl/http3/server/stream/control.hpp>

#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/util/error.hpp>

static constexpr uint64_t CLIENT_STREAM_CONTROL_ID = 0x02;
static constexpr uint64_t SERVER_STREAM_CONTROL_ID = 0x03;

namespace bnl {
namespace http3 {
namespace server {
namespace stream {
namespace control {

sender::sender(const log::api *logger) noexcept
  : endpoint::stream::control::sender(SERVER_STREAM_CONTROL_ID, logger)
{}

receiver::receiver(const log::api *logger) noexcept
  : endpoint::stream::control::receiver(CLIENT_STREAM_CONTROL_ID, logger)
  , logger_(logger)
{}

base::result<event>
receiver::process(frame frame) noexcept
{
  switch (frame) {
    case frame::type::cancel_push:
    case frame::type::max_push_id:
    case frame::type::priority:
      // TODO: Implement CANCEL_PUSH
      // TODO: Implement MAX_PUSH_ID
      // TODO: Implement PRIORITY
      THROW(base::error::not_implemented);
    case frame::type::goaway:
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
