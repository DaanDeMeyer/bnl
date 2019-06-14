#include <bnl/http3/endpoint/client.hpp>

#include <bnl/http3/error.hpp>

#include <bnl/util/error.hpp>

static constexpr uint64_t CLIENT_STREAM_CONTROL_ID = 0x02;
static constexpr uint64_t SERVER_STREAM_CONTROL_ID = 0x03;

namespace bnl {
namespace http3 {

client::control::sender::sender(const log::api *logger) noexcept
    : stream::control::sender(CLIENT_STREAM_CONTROL_ID, logger)
{}

client::control::receiver::receiver(const log::api *logger) noexcept
    : stream::control::receiver(SERVER_STREAM_CONTROL_ID, logger),
      logger_(logger)
{}

client::control::receiver::~receiver() noexcept = default;

event client::control::receiver::process(frame frame,
                                         std::error_code &ec) noexcept
{
  switch (frame) {
    case frame::type::cancel_push:
    case frame::type::goaway:
      // TODO: Implement CANCEL_PUSH
      // TODO: Implement GOAWAY
      THROW(error::not_implemented);
    case frame::type::max_push_id:
    case frame::type::priority:
      THROW(error::unexpected_frame);
    default:
      THROW(error::internal_error);
  }
}

client::request::receiver::receiver(uint64_t id,
                                    const log::api *logger) noexcept
    : stream::request::receiver(id, logger), logger_(logger)
{}

client::request::receiver::~receiver() noexcept = default;

event client::request::receiver::process(frame frame,
                                         std::error_code &ec) noexcept
{
  switch (frame) {
    case frame::type::push_promise:
    case frame::type::duplicate_push:
      // TODO: Implement PUSH_PROMISE
      // TODO: Implement DUPLICATE_PUSH
      THROW(error::not_implemented);
    case frame::type::headers:
    case frame::type::priority:
      THROW(error::unexpected_frame);
    default:
      THROW(error::internal_error);
  }
}

client::client(const log::api *logger)
    : logger_(logger),
      control_{ client::control::sender(logger),
                client::control::receiver(logger) }
{}

quic::data client::send(std::error_code &ec) noexcept
{
  client::control::sender &control = control_.sender_;

  quic::data data = control.send(ec);
  if (ec != error::idle) {
    return data;
  }

  for (auto &entry : requests_) {
    client::request::sender &sender = entry.second.sender_;

    if (sender.finished()) {
      continue;
    }

    data = sender.send(ec);
    if (ec != error::idle) {
      if (!ec) {
        client::request::receiver &receiver = entry.second.receiver_;

        if (receiver.closed()) {
          // TODO: Handle error
          receiver.start(ec);
        }
      }

      return data;
    }
  }

  THROW(error::idle);
}

void client::recv(quic::data data, event::handler handler, std::error_code &ec)
{
  if (data.id == SERVER_STREAM_CONTROL_ID) {
    client::control::receiver &control = control_.receiver_;

    auto control_handler = [this, &handler](event event, std::error_code &ec) {
      switch (event) {
        case event::type::settings:
          settings_.remote = event.settings;
          break;
        default:
          break;
      }

      handler(std::move(event), ec);
    };

    control.recv(std::move(data), control_handler, ec);

    return;
  }

  auto match = requests_.find(data.id);
  if (match == requests_.end()) {
    // TODO: Better error
    THROW_VOID(error::internal_error);
  }

  client::request::receiver &request = match->second.receiver_;

  auto request_handler = [&handler](event event, std::error_code &ec) {
    handler(std::move(event), ec);
  };

  uint64_t id = data.id;
  request.recv(std::move(data), request_handler, ec);

  if (request.finished()) {
    requests_.erase(id);
  }
}

stream::request::handle client::request(std::error_code & /* ec */)
{
  uint64_t id = next_stream_id_;

  client::request::sender sender(id, logger_);
  client::request::receiver receiver(id, logger_);

  struct request request = { std::move(sender), std::move(receiver) };
  requests_.emplace(id, std::move(request));

  stream::request::handle handle = requests_.at(id).sender_.handle();

  next_stream_id_ += 4;

  return handle;
}

} // namespace http3
} // namespace bnl
