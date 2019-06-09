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

  H3C_EXPORT event recv(quic::data &data, std::error_code &ec);

  H3C_EXPORT stream::request::handle
  response(uint64_t id, std::error_code &ec) noexcept;

private:
  struct control {
    class encoder : public stream::control::encoder {
    public:
      using stream::control::encoder::encoder;

      quic::data encode(std::error_code &ec) noexcept;
    };

    class decoder : public stream::control::decoder {
    public:
      using stream::control::decoder::decoder;

      event decode(quic::data &data, std::error_code &ec) noexcept;
    };

    encoder encoder_;
    decoder decoder_;
  };

  struct request {
    class encoder : public stream::request::encoder {
    public:
      using stream::request::encoder::encoder;

      quic::data encode(std::error_code &ec) noexcept;
    };

    class decoder : public stream::request::decoder {
    public:
      using stream::request::decoder::decoder;

      event decode(quic::data &data, std::error_code &ec) noexcept;
    };

    encoder encoder_;
    decoder decoder_;
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
