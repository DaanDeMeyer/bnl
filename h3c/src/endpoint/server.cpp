#include <h3c/endpoint/server.hpp>

#include <util/error.hpp>

static constexpr uint64_t CLIENT_STREAM_CONTROL_ID = 0x02;
static constexpr uint64_t SERVER_STREAM_CONTROL_ID = 0x03;

namespace h3c {

server::control::sender::sender(const logger *logger) noexcept
    : stream::control::sender(SERVER_STREAM_CONTROL_ID, logger)
{}

server::control::receiver::receiver(const logger *logger) noexcept
    : stream::control::receiver(CLIENT_STREAM_CONTROL_ID, logger),
      logger_(logger)
{}

server::control::receiver::~receiver() noexcept = default;

event server::control::receiver::process(frame frame,
                                         std::error_code &ec) noexcept
{
  switch (frame) {
    case frame::type::cancel_push:
    case frame::type::max_push_id:
    case frame::type::priority:
      // TODO: Implement CANCEL_PUSH
      // TODO: Implement MAX_PUSH_ID
      // TODO: Implement PRIORITY
      THROW(error::not_implemented);
    case frame::type::goaway:
      THROW(error::unexpected_frame);
    default:
      THROW(error::internal_error);
  }

  NOTREACHED();
}

server::request::receiver::receiver(uint64_t id, const logger *logger) noexcept
    : stream::request::receiver(id, logger), logger_(logger)
{}

server::request::receiver::~receiver() noexcept = default;

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

  NOTREACHED();
}

server::server(const logger *logger)
    : logger_(logger),
      control_{ server::control::sender(logger),
                server::control::receiver(logger) }
{}

quic::data server::send(std::error_code &ec) noexcept
{
  server::control::sender &control = control_.sender_;

  quic::data data = control.send(ec);
  if (ec != error::idle) {
    return data;
  }

  for (auto &entry : requests_) {
    uint64_t id = entry.first;
    server::request::sender &request = entry.second.sender_;

    if (request.finished()) {
      continue;
    }

    data = request.send(ec);

    if (request.finished()) {
      requests_.erase(id);
    }

    if (ec != error::idle) {
      return data;
    }
  }

  THROW(error::idle);
}

void server::recv(quic::data data, event::handler handler, std::error_code &ec)
{
  if (data.id == CLIENT_STREAM_CONTROL_ID) {
    server::control::receiver &control = control_.receiver_;

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
    server::request::sender sender(data.id, logger_);
    server::request::receiver receiver(data.id, logger_);

    TRY_VOID(receiver.start(ec));

    requests_.emplace(data.id,
                      request{ std::move(sender), std::move(receiver) });
  }

  server::request::receiver &request = requests_.at(data.id).receiver_;

  auto request_handler = [&handler](event event, std::error_code &ec) {
    handler(std::move(event), ec);
  };

  request.recv(std::move(data), request_handler, ec);
}

stream::request::handle
server::response(uint64_t id, std::error_code &ec) noexcept
{
  if (requests_.find(id) == requests_.end()) {
    THROW(error::stream_closed);
  }

  return requests_.at(id).sender_.handle();
}

} // namespace h3c
