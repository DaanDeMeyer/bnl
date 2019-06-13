#pragma once

#include <bnl/http3/endpoint/stream/control.hpp>
#include <bnl/http3/endpoint/stream/request.hpp>
#include <bnl/http3/event.hpp>
#include <bnl/http3/export.hpp>
#include <bnl/http3/quic.hpp>

#include <map>

namespace bnl {

namespace log {
class api;
}

namespace http3 {

class BNL_HTTP3_EXPORT server {
public:
  explicit server(const log::api *logger);

  quic::data send(std::error_code &ec) noexcept;

  void recv(quic::data data, event::handler handler, std::error_code &ec);

  stream::request::handle response(uint64_t id, std::error_code &ec) noexcept;

private:
  struct control {
    class sender : public stream::control::sender {
    public:
      explicit sender(const log::api *logger) noexcept;
    };

    class receiver : public stream::control::receiver {
    public:
      explicit receiver(const log::api *logger) noexcept;

      receiver(const receiver &) = delete;
      receiver &operator=(const receiver &) = delete;

      receiver(receiver &&) = default;
      receiver &operator=(receiver &&) = default;

      ~receiver() noexcept final;

    private:
      event process(frame frame, std::error_code &ec) noexcept final;

      const log::api *logger_;
    };

    sender sender_;
    receiver receiver_;
  };

  struct request {
    class sender : public stream::request::sender {
    public:
      using stream::request::sender::sender;
    };

    class receiver : public stream::request::receiver {
    public:
      receiver(uint64_t id, const log::api *logger) noexcept;

      receiver(const receiver &) = delete;
      receiver &operator=(const receiver &) = delete;

      receiver(receiver &&) = default;
      receiver &operator=(receiver &&) = default;

      ~receiver() noexcept final;

    private:
      event process(frame frame, std::error_code &ec) noexcept final;

      const log::api *logger_;
    };

    sender sender_;
    receiver receiver_;
  };

  const log::api *logger_;

  struct {
    settings local;
    settings remote;
  } settings_;

  control control_;

  std::map<uint64_t, request> requests_;
  buffer buffered_;
};

} // namespace http3
} // namespace bnl
