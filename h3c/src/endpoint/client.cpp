#include <h3c/endpoint/client.hpp>

#include <util/endpoint.hpp>
#include <util/error.hpp>
#include <util/stream.hpp>

static constexpr uint64_t CLIENT_STREAM_CONTROL_ID = 0x02;
static constexpr uint64_t SERVER_STREAM_CONTROL_ID = 0x03;

namespace h3c {

quic::data client::control::encoder::encode(std::error_code &ec) noexcept
{
  return stream::control::encoder::encode(ec);
}

event client::control::decoder::decode(quic::data &data,
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
    case frame::type::goaway:
      // TODO: Implement CANCEL_PUSH
      // TODO: Implement GOAWAY
      STREAM_DECODE_THROW(error::not_implemented);
    case frame::type::settings:
    case frame::type::max_push_id:
    case frame::type::priority:
      STREAM_DECODE_THROW(error::unexpected_frame);
    default:
      NOTREACHED();
  }

  NOTREACHED();
}

quic::data client::request::encoder::encode(std::error_code &ec) noexcept
{
  return stream::request::encoder::encode(ec);
}

event client::request::decoder::decode(quic::data &data,
                                       std::error_code &ec) noexcept
{
  event event = stream::request::decoder::decode(data, ec);
  if (ec != error::unknown) {
    return event;
  }

  STREAM_DECODE_START();

  frame frame = STREAM_DECODE_TRY(frame_.decode(data.buffer, ec));

  switch (frame) {
    case frame::type::push_promise:
    case frame::type::duplicate_push:
      // TODO: Implement PUSH_PROMISE
      // TODO: Implement DUPLICATE_PUSH
      STREAM_DECODE_THROW(error::not_implemented);
    case frame::type::headers:
    case frame::type::priority:
      STREAM_DECODE_THROW(error::unexpected_frame);
    default:
      NOTREACHED();
  }

  NOTREACHED();
}

client::client(logger *logger)
    : logger_(logger),
      control_{ client::control::encoder(CLIENT_STREAM_CONTROL_ID, logger),
                client::control::decoder(SERVER_STREAM_CONTROL_ID, logger) }
{}

quic::data client::send(std::error_code &ec) noexcept
{
  client::control::encoder &control = control_.encoder_;

  quic::data data = ENDPOINT_TRY(control.encode(ec));
  if (ec != error::idle) {
    return data;
  }

  for (auto &entry : requests_) {
    client::request::encoder &encoder = entry.second.encoder_;

    if (encoder == client::request::encoder::state::fin) {
      continue;
    }

    data = ENDPOINT_TRY(encoder.encode(ec));
    if (ec != error::idle) {
      client::request::decoder &decoder = entry.second.decoder_;

      if (decoder == client::request::decoder::state::closed) {
        // TODO: Handle error
        decoder.start(ec);
      }

      return data;
    }
  }

  ENDPOINT_THROW(error::idle);
}

event client::recv(quic::data &data, std::error_code &ec) noexcept
{
  if (data.id == SERVER_STREAM_CONTROL_ID) {
    client::control::decoder &control = control_.decoder_;

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
    // TODO: Better error
    ENDPOINT_THROW(error::internal_error);
  }

  client::request::decoder &request = match->second.decoder_;

  event event = ENDPOINT_TRY(request.decode(data, ec));

  switch (request) {
    case client::request::decoder::state::fin:
      requests_.erase(data.id);
      break;
    default:
      break;
  }

  return event;
}

stream::request::handle client::request(std::error_code & /* ec */)
{
  uint64_t id = next_stream_id_;

  client::request::encoder encoder(id, logger_);
  client::request::decoder decoder(id, logger_);

  struct request request = { std::move(encoder), std::move(decoder) };
  requests_.emplace(id, std::move(request));

  stream::request::handle handle = requests_.at(id).encoder_.handle();

  next_stream_id_ += 4;

  return handle;
}

} // namespace h3c
