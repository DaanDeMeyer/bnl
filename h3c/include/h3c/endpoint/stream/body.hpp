#pragma once

#include <h3c/event.hpp>
#include <h3c/frame.hpp>
#include <h3c/util/class.hpp>

#include <queue>

namespace h3c {

class logger;

namespace stream {
namespace body {

class encoder {
public:
  H3C_EXPORT encoder(uint64_t id, logger *logger) noexcept;

  H3C_MOVE_ONLY(encoder)

  H3C_EXPORT void add(buffer body, std::error_code &ec);
  H3C_EXPORT void fin(std::error_code &ec) noexcept;

  enum class state : uint8_t { frame, data, fin, error };

  H3C_EXPORT operator state() const noexcept; // NOLINT

  H3C_EXPORT quic::data encode(std::error_code &ec) noexcept;

private:
  uint64_t id_;
  logger *logger_;

  frame::encoder frame_;

  state state_ = state::frame;
  bool fin_ = false;
  std::queue<buffer> buffers_;
};

class decoder {
public:
  H3C_EXPORT decoder(uint64_t id, logger *logger) noexcept;

  H3C_MOVE_ONLY(decoder)

  enum class state : uint8_t { frame, data, fin, error };

  H3C_EXPORT operator state() const noexcept; // NOLINT

  H3C_EXPORT event decode(quic::data &data, std::error_code &ec) noexcept;

private:
  uint64_t id_;
  const logger *logger_;

  frame::decoder frame_;

  state state_ = state::frame;
  uint64_t remaining_ = 0;

  buffer buffered_;
};

} // namespace body
} // namespace stream
} // namespace h3c
