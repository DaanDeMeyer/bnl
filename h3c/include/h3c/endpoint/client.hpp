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

class client {
public:
  H3C_EXPORT explicit client(logger *logger);

  H3C_MOVE_ONLY(client);

  ~client() = default;

  H3C_EXPORT quic::data send(std::error_code &ec) noexcept;

  H3C_EXPORT event recv(quic::data &data, std::error_code &ec) noexcept;

  H3C_EXPORT stream::request::handle request(std::error_code &ec);

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

  client::control control_;

  std::map<uint64_t, struct request> requests_;

  uint64_t next_stream_id_ = 0;
  buffer buffered_;
};

} // namespace h3c
