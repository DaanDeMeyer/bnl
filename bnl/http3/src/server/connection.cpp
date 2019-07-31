#include <bnl/http3/server/connection.hpp>

namespace bnl {
namespace http3 {
namespace server {

result<quic::event>
connection::send() noexcept
{
  server::stream::control::sender &control = control_.first;

  {
    result<quic::event> r = control.send();
    if (r) {
      return r;
    }

    if (r.error() != error::idle) {
      return r.error();
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

    if (r.error() != error::idle) {
      return r.error();
    }
  }

  return error::idle;
}

result<generator>
connection::recv(quic::event event)
{
  uint64_t id = event.id();

  switch (event) {
    case quic::event::type::data:
      BNL_TRY(recv(std::move(event.data)));
      break;
    case quic::event::type::error:
      return error::not_implemented;
  }

  return generator(id, *this);
}

result<void>
connection::recv(quic::data data)
{
  server::stream::control::receiver &control = control_.second;

  if (data.id == control.id()) {
    return control.recv(std::move(data));
  };

  auto match = requests_.find(data.id);
  if (match == requests_.end()) {
    server::stream::request::sender sender(data.id);
    server::stream::request::receiver receiver(data.id);

    BNL_TRY(receiver.start());

    request request = std::make_pair(std::move(sender), std::move(receiver));
    requests_.insert(std::make_pair(data.id, std::move(request)));
  }

  server::stream::request::receiver &request = requests_.at(data.id).second;

  return request.recv(std::move(data));
}

result<event>
connection::process(uint64_t id)
{
  server::stream::control::receiver &control = control_.second;

  if (id == control.id()) {
    event event = BNL_TRY(control.process());

    switch (event) {
      case event::type::settings:
        settings_.peer = event.settings;
        break;
      default:
        break;
    }

    return base::success(std::move(event));
  }

  server::stream::request::receiver &request = requests_.at(id).second;

  return request.process();
}

result<response::handle>
connection::response(uint64_t id)
{
  auto match = requests_.find(id);
  if (match == requests_.end()) {
    return error::stream_closed;
  }

  stream::request::sender &sender = match->second.first;

  return response::handle(&sender);
}

}
}
}
