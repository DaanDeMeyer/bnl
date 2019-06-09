#include <h3c/endpoint/server.hpp>

#include <util/endpoint.hpp>
#include <util/error.hpp>
#include <util/stream.hpp>

static constexpr uint64_t CLIENT_STREAM_CONTROL_ID = 0x02;
static constexpr uint64_t SERVER_STREAM_CONTROL_ID = 0x03;

namespace h3c {

quic::data server::control::encoder::encode(std::error_code &ec) noexcept
{
  return stream::control::encoder::encode(ec);
}

event server::control::decoder::decode(quic::data &data,
                                       std::error_code &ec) noexcept
{
  event event = stream::control::decoder::decode(data, ec);
  if (ec != error::unknown) {
    return event;
  }

  STREAM_DECODE_START();

  frame frame = STREAM_DECODE_TRY(frame_.decode(data.buffer, ec));

  switch (frame) {
    case frame::type::cancel_push:
    case frame::type::max_push_id:
    case frame::type::priority:
      // TODO: Implement CANCEL_PUSH
      // TODO: Implement MAX_PUSH_ID
      // TODO: Implement PRIORITY
      STREAM_DECODE_THROW(error::not_implemented);
    case frame::type::goaway:
      STREAM_DECODE_THROW(error::unexpected_frame);
    default:
      NOTREACHED();
  }

  NOTREACHED();
}

quic::data server::request::encoder::encode(std::error_code &ec) noexcept
{
  return stream::request::encoder::encode(ec);
}

event server::request::decoder::decode(quic::data &data,
                                       std::error_code &ec) noexcept
{
  event event = stream::request::decoder::decode(data, ec);
  if (ec != error::unknown) {
    return event;
  }

  STREAM_DECODE_START();

  frame frame = STREAM_DECODE_TRY(frame_.decode(data.buffer, ec));

  switch (frame) {
    case frame::type::priority:
      if (headers_ != stream::headers::decoder::state::frame) {
        STREAM_DECODE_THROW(error::unexpected_frame);
      }

      // TODO: Implement PRIORITY
      STREAM_DECODE_THROW(error::not_implemented);
    case frame::type::headers:
    case frame::type::push_promise:
    case frame::type::duplicate_push:
      STREAM_DECODE_THROW(error::unexpected_frame);
    default:
      NOTREACHED();
  }

  NOTREACHED();
}

server::server(logger *logger)
    : logger_(logger),
      control_{ server::control::encoder(SERVER_STREAM_CONTROL_ID, logger),
                server::control::decoder(CLIENT_STREAM_CONTROL_ID, logger) }
{}

quic::data server::send(std::error_code &ec) noexcept
{
  server::control::encoder &control = control_.encoder_;

  quic::data data = ENDPOINT_TRY(control.encode(ec));
  if (ec != error::idle) {
    return data;
  }

  for (auto &entry : requests_) {
    uint64_t id = entry.first;
    server::request::encoder &request = entry.second.encoder_;

    if (request == server::request::encoder::state::fin) {
      continue;
    }

    data = ENDPOINT_TRY(request.encode(ec));

    switch (request) {
      case server::request::encoder::state::fin:
        requests_.erase(id);
        break;
      default:
        break;
    }

    if (ec != error::idle) {
      return data;
    }
  }

  ENDPOINT_THROW(error::idle);
}

event server::recv(quic::data &data, std::error_code &ec)
{
  if (data.id == CLIENT_STREAM_CONTROL_ID) {
    server::control::decoder &control = control_.decoder_;

    event event = ENDPOINT_TRY(control.decode(data, ec));

    switch (event) {
      case event::type::settings:
        settings_.remote = event.settings;
        break;
      default:
        break;
    }

    return event;
  }

  auto match = requests_.find(data.id);
  if (match == requests_.end()) {
    server::request::encoder encoder(data.id, logger_);
    server::request::decoder decoder(data.id, logger_);

    decoder.start(ec);

    requests_.emplace(data.id,
                      request{ std::move(encoder), std::move(decoder) });
  }

  server::request::decoder &request = requests_.at(data.id).decoder_;

  event event = ENDPOINT_TRY(request.decode(data, ec));

  return event;
}

stream::request::handle
server::response(uint64_t id, std::error_code &ec) noexcept
{
  if (requests_.find(id) == requests_.end()) {
    THROW(error::stream_closed);
  }

  return requests_.at(id).encoder_.handle();
}

} // namespace h3c
