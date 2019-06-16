#pragma once

#include <bnl/core/export.hpp>

#include <bnl/http3/header.hpp>
#include <bnl/http3/settings.hpp>

#include <bnl/quic/event.hpp>

#include <bnl/buffer.hpp>
#include <bnl/function_view.hpp>

#include <system_error>

namespace bnl {
namespace http3 {

class BNL_CORE_EXPORT event {
public:
  using handler = function_view<void(event, std::error_code &)>;

  enum class type { settings, header, body, error };

  struct payload {
    using settings = http3::settings;
    using header = http3::header;
    using body = buffer;
    using error = quic::event::payload::error;
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

} // namespace http3
} // namespace bnl
