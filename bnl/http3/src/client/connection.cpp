#include <bnl/http3/client/connection.hpp>

#include <bnl/base/log.hpp>

namespace bnl {
namespace http3 {
namespace client {

result<quic::event>
connection::send() noexcept
{
  client::stream::control::sender &control = control_.first;

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
    client::stream::request::sender &sender = entry.second.first;

    if (sender.finished()) {
      continue;
    }

    result<quic::event> r = sender.send();
    if (r) {
      client::stream::request::receiver &receiver = entry.second.second;

      if (receiver.closed()) {
        BNL_TRY(receiver.start());
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
  BNL_LOG_T("Received QUIC event: {}", event);

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
  uint64_t id = data.id;
  client::stream::control::receiver &control = control_.second;

  if (id == control.id()) {
    return control.recv(std::move(data));
  }

  // TODO: Actually handle unidirectional streams.
  if ((data.id & 0x2U) != 0) {
    return base::success();
  }

  auto match = requests_.find(id);
  // TODO: Better error
  if (match == requests_.end()) {
    return error::internal;
  }

  client::stream::request::receiver &request = match->second.second;

  return request.recv(std::move(data));
}

result<event>
connection::process(uint64_t id)
{
  client::stream::control::receiver &control = control_.second;

  if (id == control.id()) {
    event event = BNL_TRY(control.process());

    switch (event) {
      case event::type::settings:
        settings_.peer = event.settings;
        break;
      default:
        break;
    }

    return event;
  }

  // TODO: Actually handle unidirectional streams.
  if ((id & 0x2U) != 0) {
    return error::incomplete;
  }

  auto match = requests_.find(id);
  // TODO: Better error
  if (match == requests_.end()) {
    return error::internal;
  }

  client::stream::request::receiver &request = match->second.second;

  event event = BNL_TRY(request.process());

  if (event == event::type::finished) {
    requests_.erase(id);
  }

  return event;
}

result<request::handle>
connection::request()
{
  uint64_t id = next_stream_id_;

  client::stream::request::sender sender(id);
  client::stream::request::receiver receiver(id);

  request_t request = std::make_pair(std::move(sender), std::move(receiver));
  requests_.insert(std::make_pair(id, std::move(request)));

  next_stream_id_ += 4;

  return request::handle(&requests_.at(id).first);
}

}
}
}
