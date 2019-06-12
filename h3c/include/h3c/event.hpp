#pragma once

#include <h3c/buffer.hpp>
#include <h3c/header.hpp>
#include <h3c/quic.hpp>
#include <h3c/settings.hpp>

#include <functional>

namespace h3c {

class event {
public:
  using handler = const std::function<void(event, std::error_code &ec)> &;

  enum class type { empty, settings, header, body, error };

  struct payload {
    using settings = h3c::settings;
    using header = h3c::header;
    using body = h3c::buffer;
    using error = quic::error;
  };

  H3C_EXPORT event() noexcept;

  H3C_EXPORT event(uint64_t id, bool fin, payload::settings settings) noexcept;
  H3C_EXPORT event(uint64_t id, bool fin, payload::header header) noexcept;
  H3C_EXPORT event(uint64_t id, bool fin, payload::body body) noexcept;
  H3C_EXPORT event(uint64_t id, bool fin, payload::error error) noexcept;

  H3C_EXPORT event(const event &other) noexcept;
  H3C_EXPORT event(event &&other) noexcept;

  event &operator=(const event &) = delete;
  event &operator=(event &&) = delete;

  H3C_EXPORT ~event() noexcept;

  H3C_EXPORT operator type() const noexcept; // NOLINT

private:
  const type type_;

public:
  const uint64_t id;
  const bool fin;

  union {
    payload::settings settings;
    payload::header header;
    payload::body body;
    payload::error error;
  };
};

} // namespace h3c
