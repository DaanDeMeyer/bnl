#pragma once

#include <h3c/event.hpp>
#include <h3c/frame.hpp>
#include <h3c/quic.hpp>

#include <h3c/util/class.hpp>

namespace h3c {

class logger;

namespace stream {
namespace control {

class encoder {
public:
  H3C_EXPORT encoder(uint64_t id, logger *logger) noexcept;

  H3C_MOVE_ONLY(encoder);

  ~encoder() = default;

  enum class state : uint8_t { settings, idle, error };

  H3C_EXPORT operator state() const noexcept; // NOLINT

  H3C_EXPORT quic::data encode(std::error_code &ec) noexcept;

protected:
  uint64_t id_;
  logger *logger_;

  frame::encoder frame_;

  state state_ = state::settings;
  settings settings_;
};

class decoder {
public:
  H3C_EXPORT decoder(uint64_t id, logger *logger) noexcept;

  H3C_MOVE_ONLY(decoder);

  ~decoder() = default;

  enum class state : uint8_t { settings, active, error };

  H3C_EXPORT operator state() const noexcept; // NOLINT

  H3C_EXPORT event decode(quic::data &data, std::error_code &ec) noexcept;

protected:
  uint64_t id_;
  logger *logger_;

  frame::decoder frame_;

  state state_ = state::settings;

  buffer buffered_;
};

} // namespace control
} // namespace stream
} // namespace h3c