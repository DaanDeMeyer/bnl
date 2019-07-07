#include <bnl/http3/client.hpp>

#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/util/error.hpp>

namespace bnl {
namespace http3 {

client::client(const log::api *logger)
    : logger_(logger),
      control_{ endpoint::client::control::sender(logger),
                endpoint::client::control::receiver(logger) }
{}

quic::event client::send(std::error_code &ec) noexcept
{
  endpoint::client::control::sender &control = control_.first;

  {
    quic::event event = control.send(ec);
    if (ec != base::error::idle) {
      return event;
    }
  }

  for (auto &entry : requests_) {
    endpoint::client::request::sender &sender = entry.second.first;

    if (sender.finished()) {
      continue;
    }

    quic::event event = sender.send(ec);
    if (ec != base::error::idle) {
      if (!ec) {
        endpoint::client::request::receiver &receiver = entry.second.second;

        if (receiver.closed()) {
          // TODO: Handle error
          receiver.start(ec);
        }
      }

      return event;
    }
  }

  THROW(base::error::idle);
}

base::nothing client::recv(quic::event event,
                           event::handler handler,
                           std::error_code &ec)
{
  endpoint::client::control::receiver &control = control_.second;

  if (event.id == control.id()) {
    auto control_handler = [this, &handler](http3::event event,
                                            std::error_code &ec) {
      switch (event) {
        case event::type::settings:
          settings_.remote = event.settings;
          break;
        default:
          break;
      }

      handler(std::move(event), ec);
    };

    control.recv(std::move(event), control_handler, ec);

    return {};
  }

  auto match = requests_.find(event.id);
  // TODO: Better error
  CHECK(match != requests_.end(), error::internal_error);

  endpoint::client::request::receiver &request = match->second.second;

  auto request_handler = [&handler](http3::event event, std::error_code &ec) {
    handler(std::move(event), ec);
  };

  uint64_t id = event.id;
  request.recv(std::move(event), request_handler, ec);

  if (request.finished()) {
    requests_.erase(id);
  }

  return {};
}

uint64_t client::request(std::error_code & /* ec */)
{
  uint64_t id = next_stream_id_;

  endpoint::client::request::sender sender(id, logger_);
  endpoint::client::request::receiver receiver(id, logger_);

  request_t request = std::make_pair(std::move(sender), std::move(receiver));
  requests_.insert(std::make_pair(id, std::move(request)));

  next_stream_id_ += 4;

  return id;
}

base::nothing client::header(uint64_t id,
                             header_view header,
                             std::error_code &ec)
{
  auto match = requests_.find(id);
  CHECK(match != requests_.end(), error::stream_closed);

  endpoint::client::request::sender &sender = match->second.first;

  return sender.header(header, ec);
}

base::nothing client::body(uint64_t id, base::buffer body, std::error_code &ec)
{
  auto match = requests_.find(id);
  CHECK(match != requests_.end(), error::stream_closed);

  endpoint::client::request::sender &sender = match->second.first;

  return sender.body(std::move(body), ec);
}

base::nothing client::start(uint64_t id, std::error_code &ec) noexcept
{
  auto match = requests_.find(id);
  CHECK(match != requests_.end(), error::stream_closed);

  endpoint::client::request::sender &sender = match->second.first;

  return sender.start(ec);
}

base::nothing client::fin(uint64_t id, std::error_code &ec) noexcept
{
  auto match = requests_.find(id);
  CHECK(match != requests_.end(), error::stream_closed);

  endpoint::client::request::sender &sender = match->second.first;

  return sender.fin(ec);
}

} // namespace http3
} // namespace bnl
