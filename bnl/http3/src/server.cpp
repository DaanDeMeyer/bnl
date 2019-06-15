#include <bnl/http3/server.hpp>

#include <bnl/http3/error.hpp>

#include <bnl/util/error.hpp>

namespace bnl {
namespace http3 {

server::server(const log::api *logger)
    : logger_(logger),
      control_{ endpoint::server::control::sender(logger),
                endpoint::server::control::receiver(logger) }
{}

quic::data server::send(std::error_code &ec) noexcept
{
  endpoint::server::control::sender &control = control_.first;

  quic::data data = control.send(ec);
  if (ec != error::idle) {
    return data;
  }

  for (auto &entry : requests_) {
    uint64_t id = entry.first;
    endpoint::server::request::sender &request = entry.second.first;

    if (request.finished()) {
      continue;
    }

    data = request.send(ec);

    if (request.finished()) {
      requests_.erase(id);
    }

    if (ec != error::idle) {
      return data;
    }
  }

  THROW(error::idle);
}

void server::recv(quic::data data, event::handler handler, std::error_code &ec)
{
  endpoint::server::control::receiver &control = control_.second;

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

    return;
  }

  auto match = requests_.find(data.id);
  if (match == requests_.end()) {
    endpoint::server::request::sender sender(data.id, logger_);
    endpoint::server::request::receiver receiver(data.id, logger_);

    TRY_VOID(receiver.start(ec));

    request request = std::make_pair(std::move(sender), std::move(receiver));
    requests_.insert(std::make_pair(data.id, std::move(request)));
  }

  endpoint::server::request::receiver &request = requests_.at(data.id).second;

  auto request_handler = [&handler](event event, std::error_code &ec) {
    handler(std::move(event), ec);
  };

  request.recv(std::move(data), request_handler, ec);
}

endpoint::handle server::response(uint64_t id, std::error_code &ec) noexcept
{
  if (requests_.find(id) == requests_.end()) {
    THROW(error::stream_closed);
  }

  return requests_.at(id).first.handle();
}

} // namespace http3
} // namespace bnl
