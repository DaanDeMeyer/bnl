#pragma once

#include <h3c/event.hpp>
#include <h3c/frame.hpp>
#include <h3c/header.hpp>
#include <h3c/qpack.hpp>
#include <h3c/util/class.hpp>

#include <queue>

namespace h3c {

class logger;

namespace stream {
namespace headers {

class encoder {
public:
  H3C_EXPORT encoder(uint64_t id, logger *logger) noexcept;

  H3C_MOVE_ONLY(encoder)

  H3C_EXPORT void add(header_view header, std::error_code &ec);
  H3C_EXPORT void fin(std::error_code &ec) noexcept;

  enum class state : uint8_t { idle, frame, qpack, fin, error };

  H3C_EXPORT operator state() const noexcept; // NOLINT

  H3C_EXPORT quic::data encode(std::error_code &ec) noexcept;

private:
  uint64_t id_;
  logger *logger_;

  frame::encoder frame_;
  qpack::encoder qpack_;

  state state_ = state::idle;
  std::queue<buffer> buffers_;
};

class decoder {
public:
  H3C_EXPORT decoder(uint64_t id, logger *logger) noexcept;

  H3C_MOVE_ONLY(decoder)

  enum class state : uint8_t { frame, qpack, fin, error };

  H3C_EXPORT operator state() const noexcept; // NOLINT

  H3C_EXPORT event decode(quic::data &data, std::error_code &ec) noexcept;

private:
  uint64_t id_;
  logger *logger_;

  frame::decoder frame_;
  qpack::decoder qpack_;

  state state_ = state::frame;
  uint64_t headers_size_ = 0;

  buffer buffered_;
};

} // namespace headers
} // namespace stream
} // namespace h3c
