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

result<quic::event>
connection::send() noexcept
{
  server::stream::control::sender &control = control_.first;

  {
    result<quic::event> r = control.send();
    if (r) {
      return r;
    }

    if (r.error() != base::error::idle) {
      return std::move(r).error();
    }
  }

  for (auto &entry : requests_) {
    uint64_t id = entry.first;
    server::stream::request::sender &request = entry.second.first;

    if (request.finished()) {
      continue;
    }

    result<quic::event> r = request.send();
    if (r) {
      if (request.finished()) {
        requests_.erase(id);
      }

      return r;
    }

    if (r.error() != base::error::idle) {
      return std::move(r).error();
    }
  }

  return base::error::idle;
}

result<void> connection::recv(quic::event event, event::handler handler)
{
  switch (event) {
    case quic::event::type::data:
      return recv(std::move(event.data), handler);
    case quic::event::type::error:
      THROW(error::not_implemented);
  }

  NOTREACHED();
}

result<void>
connection::recv(quic::data data, event::handler handler)
{
  server::stream::control::receiver &control = control_.second;

  if (data.id == control.id()) {
    auto control_handler = [this, handler](http3::event event) {
      switch (event) {
        case event::type::settings:
          settings_.peer = event.settings;
          break;
        default:
          break;
      }

      return handler(std::move(event));
    };

    return control.recv(std::move(data), control_handler);
  }

  auto match = requests_.find(data.id);
  if (match == requests_.end()) {
    server::stream::request::sender sender(data.id, logger_);
    server::stream::request::receiver receiver(data.id, logger_);

    TRY(receiver.start());

    request request = std::make_pair(std::move(sender), std::move(receiver));
    requests_.insert(std::make_pair(data.id, std::move(request)));
  }

  server::stream::request::receiver &request = requests_.at(data.id).second;

  return request.recv(std::move(data), handler);
}

result<response::handle>
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
