#include <bnl/http3/server.hpp>

#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/util/error.hpp>

namespace bnl {
namespace http3 {

server::server(const log::api *logger)
  : control_{ endpoint::server::control::sender(logger),
              endpoint::server::control::receiver(logger) }
  , logger_(logger)
{}

base::result<quic::event>
server::send() noexcept
{
  endpoint::server::control::sender &control = control_.first;

  {
    base::result<quic::event> event = control.send();
    if (event != base::error::idle) {
      return event;
    }
  }

  for (auto &entry : requests_) {
    uint64_t id = entry.first;
    endpoint::server::request::sender &request = entry.second.first;

    if (request.finished()) {
      continue;
    }

    base::result<quic::event> event = request.send();

    if (request.finished()) {
      requests_.erase(id);
    }

    if (event != base::error::idle) {
      return event;
    }
  }

  THROW(base::error::idle);
}

std::error_code
server::recv(quic::event event, event::handler handler)
{
  endpoint::server::control::receiver &control = control_.second;

  if (event.id == control.id()) {
    auto control_handler = [this, &handler](http3::event event) {
      switch (event) {
        case event::type::settings:
          settings_.remote = event.settings;
          break;
        default:
          break;
      }

      return handler(std::move(event));
    };

    control.recv(std::move(event), control_handler);

    return {};
  }

  auto match = requests_.find(event.id);
  if (match == requests_.end()) {
    endpoint::server::request::sender sender(event.id, logger_);
    endpoint::server::request::receiver receiver(event.id, logger_);

    TRY(receiver.start());

    request request = std::make_pair(std::move(sender), std::move(receiver));
    requests_.insert(std::make_pair(event.id, std::move(request)));
  }

  endpoint::server::request::receiver &request = requests_.at(event.id).second;

  request.recv(std::move(event), handler);

  return {};
}

std::error_code
server::header(uint64_t id, header_view header)
{
  auto match = requests_.find(id);
  CHECK(match != requests_.end(), error::stream_closed);

  endpoint::server::request::sender &sender = match->second.first;

  return sender.header(header);
}

std::error_code
server::body(uint64_t id, base::buffer body)
{
  auto match = requests_.find(id);
  CHECK(match != requests_.end(), error::stream_closed);

  endpoint::server::request::sender &sender = match->second.first;

  return sender.body(std::move(body));
}

std::error_code
server::start(uint64_t id) noexcept
{
  auto match = requests_.find(id);
  CHECK(match != requests_.end(), error::stream_closed);

  endpoint::server::request::sender &sender = match->second.first;

  return sender.start();
}

std::error_code
server::fin(uint64_t id) noexcept
{
  auto match = requests_.find(id);
  CHECK(match != requests_.end(), error::stream_closed);

  endpoint::server::request::sender &sender = match->second.first;

  return sender.fin();
}

} // namespace http3
} // namespace bnl
