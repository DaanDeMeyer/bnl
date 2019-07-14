#include <bnl/http3/endpoint/client/control.hpp>

#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/util/error.hpp>

static constexpr uint64_t CLIENT_STREAM_CONTROL_ID = 0x02;
static constexpr uint64_t SERVER_STREAM_CONTROL_ID = 0x03;

namespace bnl {
namespace http3 {
namespace endpoint {
namespace client {
namespace control {

sender::sender(const log::api* logger) noexcept
  : shared::control::sender(CLIENT_STREAM_CONTROL_ID, logger)
{}

receiver::receiver(const log::api* logger) noexcept
  : shared::control::receiver(SERVER_STREAM_CONTROL_ID, logger)
  , logger_(logger)
{}

base::result<event>
receiver::process(frame frame) noexcept
{
  switch (frame) {
    case frame::type::cancel_push:
    case frame::type::goaway:
      // TODO: Implement CANCEL_PUSH
      // TODO: Implement GOAWAY
      THROW(base::error::not_implemented);
    case frame::type::max_push_id:
    case frame::type::priority:
      THROW(error::unexpected_frame);
    default:
      THROW(error::internal_error);
  }
}

} // namespace control
} // namespace client
} // namespace endpoint
} // namespace http3
} // namespace bnl
