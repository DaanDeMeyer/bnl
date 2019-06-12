#pragma once

#include <h3c/buffer.hpp>
#include <h3c/header.hpp>
#include <h3c/quic.hpp>
#include <h3c/settings.hpp>

#include <functional>

namespace h3c {

class H3C_EXPORT event {
public:
  using handler = const std::function<void(event, std::error_code &ec)> &;

  enum class type { empty, settings, header, body, error };

  struct payload {
    using settings = h3c::settings;
    using header = h3c::header;
    using body = h3c::buffer;
    using error = quic::error;
  };

  event() noexcept;

  event(uint64_t id, bool fin, payload::settings settings) noexcept;
  event(uint64_t id, bool fin, payload::header header) noexcept;
  event(uint64_t id, bool fin, payload::body body) noexcept;
  event(uint64_t id, bool fin, payload::error error) noexcept;

  event(const event &other) noexcept;
  event(event &&other) noexcept;

  event &operator=(const event &) = delete;
  event &operator=(event &&) = delete;

  ~event() noexcept;

  operator type() const noexcept; // NOLINT

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
