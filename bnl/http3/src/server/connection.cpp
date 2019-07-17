#include <bnl/http3/server/connection.hpp>

#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/util/error.hpp>

namespace bnl {
namespace http3 {
namespace server {

connection::connection(const log::api *logger)
  : control_{ server::stream::control::sender(logger),
              server::stream::control::receiver(logger) }
  , logger_(logger)
{}

base::result<quic::event>
connection::send() noexcept
{
  server::stream::control::sender &control = control_.first;

  {
    base::result<quic::event> event = control.send();
    if (event != base::error::idle) {
      return event;
    }
  }

  for (auto &entry : requests_) {
    uint64_t id = entry.first;
    server::stream::request::sender &request = entry.second.first;

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
connection::recv(quic::event event, event::handler handler)
{
  server::stream::control::receiver &control = control_.second;

  if (event.id == control.id()) {
    auto control_handler = [this, &handler](http3::event event) {
      switch (event) {
        case event::type::settings:
          settings_.peer = event.settings;
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
    server::stream::request::sender sender(event.id, logger_);
    server::stream::request::receiver receiver(event.id, logger_);

    TRY(receiver.start());

    request request = std::make_pair(std::move(sender), std::move(receiver));
    requests_.insert(std::make_pair(event.id, std::move(request)));
  }

  server::stream::request::receiver &request = requests_.at(event.id).second;

  request.recv(std::move(event), handler);

  return {};
}

base::result<response::handle>
connection::response(uint64_t id)
{
  auto match = requests_.find(id);
  if (match == requests_.end()) {
    THROW(error::stream_closed);
  }

  stream::request::sender &sender = match->second.first;

  return response::handle(&sender);
}

}
}
}
