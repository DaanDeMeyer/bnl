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
  H3C_EXPORT explicit encoder(logger *logger) noexcept;

  H3C_MOVE_ONLY(encoder);

  ~encoder() = default;

  H3C_EXPORT void add(buffer body, std::error_code &ec);
  H3C_EXPORT void fin(std::error_code &ec) noexcept;

  bool finished() const noexcept;

  H3C_EXPORT buffer encode(std::error_code &ec) noexcept;

private:
  logger *logger_;

  frame::encoder frame_;

  enum class state : uint8_t { frame, data, fin, error };
  
  state state_ = state::frame;
  bool fin_ = false;
  std::queue<buffer> buffers_;
};

class decoder {
public:
  H3C_EXPORT explicit decoder(logger *logger) noexcept;

  H3C_MOVE_ONLY(decoder);

  ~decoder() = default;

  bool in_progress() const noexcept;

  H3C_EXPORT buffer decode(buffers &encoded, std::error_code &ec) noexcept;

private:
  logger *logger_;

  frame::decoder frame_;

  enum class state : uint8_t { frame, data, error };

  state state_ = state::frame;
  uint64_t remaining_ = 0;
};

} // namespace body
} // namespace stream
} // namespace h3c
