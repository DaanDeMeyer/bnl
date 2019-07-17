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

std::error_code
connection::header(uint64_t id, header_view header)
{
  auto match = requests_.find(id);
  CHECK(match != requests_.end(), error::stream_closed);

  server::stream::request::sender &sender = match->second.first;

  return sender.header(header);
}

std::error_code
connection::body(uint64_t id, base::buffer body)
{
  auto match = requests_.find(id);
  CHECK(match != requests_.end(), error::stream_closed);

  server::stream::request::sender &sender = match->second.first;

  return sender.body(std::move(body));
}

std::error_code
connection::start(uint64_t id) noexcept
{
  auto match = requests_.find(id);
  CHECK(match != requests_.end(), error::stream_closed);

  server::stream::request::sender &sender = match->second.first;

  return sender.start();
}

std::error_code
connection::fin(uint64_t id) noexcept
{
  auto match = requests_.find(id);
  CHECK(match != requests_.end(), error::stream_closed);

  server::stream::request::sender &sender = match->second.first;

  return sender.fin();
}

}
}
}
