#include <bnl/http3/client/connection.hpp>

#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/util/error.hpp>

namespace bnl {
namespace http3 {
namespace client {

connection::connection(const log::api *logger)
  : control_{ client::stream::control::sender(logger),
              client::stream::control::receiver(logger) }
  , logger_(logger)
{}

result<quic::event>
connection::send() noexcept
{
  client::stream::control::sender &control = control_.first;

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
    client::stream::request::sender &sender = entry.second.first;

    if (sender.finished()) {
      continue;
    }

    result<quic::event> r = sender.send();
    if (r) {
      client::stream::request::receiver &receiver = entry.second.second;

      if (receiver.closed()) {
        TRY(receiver.start());
      }

      return r;
    }

    if (r.error() != base::error::idle) {
      return std::move(r).error();
    }
  }

  return base::error::idle;
}

result<void>
connection::recv(quic::event event, event::handler handler)
{
  LOG_T("Received QUIC event: {}", event);

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
  uint64_t id = data.id;
  client::stream::control::receiver &control = control_.second;

  if (id == control.id()) {
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

  // TODO: Actually handle unidirectional streams.
  if ((data.id & 0x2U) != 0) {
    return success();
  }

  auto match = requests_.find(id);
  // TODO: Better error
  if (match == requests_.end()) {
    THROW(http3::connection::error::internal);
  }

  client::stream::request::receiver &request = match->second.second;

  TRY(request.recv(std::move(data), handler));

  if (request.finished()) {
    requests_.erase(id);
  }

  return success();
}

result<request::handle>
connection::request()
{
  uint64_t id = next_stream_id_;

  client::stream::request::sender sender(id, logger_);
  client::stream::request::receiver receiver(id, logger_);

  request_t request = std::make_pair(std::move(sender), std::move(receiver));
  requests_.insert(std::make_pair(id, std::move(request)));

  next_stream_id_ += 4;

  return request::handle(&requests_.at(id).first);
}

}
}
}
