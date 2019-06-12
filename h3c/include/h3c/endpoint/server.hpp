#pragma once

#include <h3c/endpoint/stream/control.hpp>
#include <h3c/endpoint/stream/request.hpp>
#include <h3c/event.hpp>
#include <h3c/export.hpp>
#include <h3c/quic.hpp>
#include <h3c/util/class.hpp>

#include <map>

namespace h3c {

class logger;

class server {
public:
  H3C_EXPORT explicit server(logger *logger);

  H3C_MOVE_ONLY(server);

  ~server() = default;

  H3C_EXPORT quic::data send(std::error_code &ec) noexcept;

  H3C_EXPORT void
  recv(quic::data data, event::handler handler, std::error_code &ec);

  H3C_EXPORT stream::request::handle
  response(uint64_t id, std::error_code &ec) noexcept;

private:
  struct control {
    class sender : public stream::control::sender {
    public:
      H3C_EXPORT explicit sender(logger *logger) noexcept;
    };

    class H3C_EXPORT receiver : public stream::control::receiver {
    public:
      explicit receiver(logger *logger) noexcept;

      H3C_MOVE_ONLY(receiver);

      ~receiver() noexcept final;

    private:
      event process(frame frame, std::error_code &ec) noexcept final;

      logger *logger_;
    };

    sender sender_;
    receiver receiver_;
  };

  struct request {
    class sender : public stream::request::sender {
    public:
      using stream::request::sender::sender;
    };

    class H3C_EXPORT receiver : public stream::request::receiver {
    public:
      receiver(uint64_t id, logger *logger) noexcept;

      H3C_MOVE_ONLY(receiver);

      ~receiver() noexcept final;

    private:
      event process(frame frame, std::error_code &ec) noexcept final;

      logger *logger_;
    };

    sender sender_;
    receiver receiver_;
  };

  logger *logger_;

  struct {
    settings local;
    settings remote;
  } settings_;

  control control_;

  std::map<uint64_t, request> requests_;
  buffer buffered_;
};

} // namespace h3c
