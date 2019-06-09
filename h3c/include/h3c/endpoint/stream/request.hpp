#pragma once

#include <h3c/endpoint/stream/body.hpp>
#include <h3c/endpoint/stream/headers.hpp>
#include <h3c/event.hpp>
#include <h3c/export.hpp>
#include <h3c/frame.hpp>
#include <h3c/header.hpp>

#include <h3c/util/class.hpp>

namespace h3c {

class logger;

namespace qpack {
class decoder;
} // namespace qpack

namespace stream {
namespace request {

class encoder;

class handle {
public:
  handle() = default;
  H3C_EXPORT explicit handle(uint64_t id,
                             stream::request::encoder *ref,
                             logger *logger) noexcept;

  handle(const handle &) = delete;
  handle &operator=(const handle &) = delete;

  H3C_EXPORT handle(handle &&other) noexcept;
  H3C_EXPORT handle &operator=(handle &&other) noexcept;

  H3C_EXPORT ~handle() noexcept;

  H3C_EXPORT bool valid() const noexcept;

  H3C_EXPORT uint64_t id() const noexcept;

  H3C_EXPORT void header(header_view header, std::error_code &ec);
  H3C_EXPORT void body(buffer body, std::error_code &ec);

  H3C_EXPORT void start(std::error_code &ec) noexcept;
  H3C_EXPORT void fin(std::error_code &ec) noexcept;

private:
  friend request::encoder;

  uint64_t id_ = UINT64_MAX;
  request::encoder *ref_ = nullptr;
  logger *logger_ = nullptr;
};

class encoder {
public:
  H3C_EXPORT encoder(uint64_t id, logger *logger) noexcept;

  encoder(const encoder &) = delete;
  encoder &operator=(const encoder &) = delete;

  encoder(encoder &&) noexcept;
  encoder &operator=(encoder &&) noexcept;

  H3C_EXPORT ~encoder() noexcept;

  enum state : uint8_t { headers, body, fin, error };

  H3C_EXPORT operator state() const noexcept; // NOLINT

  H3C_EXPORT request::handle handle() noexcept;

  H3C_EXPORT quic::data encode(std::error_code &ec) noexcept;

protected:
  friend request::handle;

  uint64_t id_;
  logger *logger_;

  stream::headers::encoder headers_;
  stream::body::encoder body_;

  state state_ = state::headers;
  request::handle *handle_ = nullptr;
};

class decoder {
public:
  H3C_EXPORT decoder(uint64_t id, logger *logger) noexcept;

  H3C_MOVE_ONLY(decoder);

  ~decoder() = default;

  enum class state : uint8_t { closed, headers, body, fin, error };

  H3C_EXPORT operator state() const noexcept; // NOLINT

  H3C_EXPORT void start(std::error_code &ec) noexcept;

  H3C_EXPORT event decode(quic::data &data, std::error_code &ec) noexcept;

protected:
  uint64_t id_;
  logger *logger_;

  frame::decoder frame_;
  stream::headers::decoder headers_;
  stream::body::decoder body_;

  state state_ = state::closed;

  buffer buffered_;
};

} // namespace request
} // namespace stream
} // namespace h3c
