#include <bnl/http3/client.hpp>

#include <bnl/http3/error.hpp>

#include <bnl/util/error.hpp>

namespace bnl {
namespace http3 {

client::client(const log::api *logger)
    : logger_(logger),
      control_{ endpoint::client::control::sender(logger),
                endpoint::client::control::receiver(logger) }
{}

quic::data client::send(std::error_code &ec) noexcept
{
  endpoint::client::control::sender &control = control_.first;

  quic::data data = control.send(ec);
  if (ec != error::idle) {
    return data;
  }

  for (auto &entry : requests_) {
    endpoint::client::request::sender &sender = entry.second.first;

    if (sender.finished()) {
      continue;
    }

    data = sender.send(ec);
    if (ec != error::idle) {
      if (!ec) {
        endpoint::client::request::receiver &receiver = entry.second.second;

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

nothing
client::recv(quic::data data, event::handler handler, std::error_code &ec)
{
  endpoint::client::control::receiver &control = control_.second;

  if (data.id == control.id()) {
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

    return {};
  }

  auto match = requests_.find(data.id);
  // TODO: Better error
  CHECK(match != requests_.end(), error::internal_error);

  endpoint::client::request::receiver &request = match->second.second;

  auto request_handler = [&handler](event event, std::error_code &ec) {
    handler(std::move(event), ec);
  };

  uint64_t id = data.id;
  request.recv(std::move(data), request_handler, ec);

  if (request.finished()) {
    requests_.erase(id);
  }

  return {};
}

endpoint::handle client::request(std::error_code & /* ec */)
{
  uint64_t id = next_stream_id_;

  endpoint::client::request::sender sender(id, logger_);
  endpoint::client::request::receiver receiver(id, logger_);

  request_t request = std::make_pair(std::move(sender), std::move(receiver));
  requests_.insert(std::make_pair(id, std::move(request)));

  endpoint::handle handle = requests_.at(id).first.handle();

  next_stream_id_ += 4;

  return handle;
}

} // namespace http3
} // namespace bnl
