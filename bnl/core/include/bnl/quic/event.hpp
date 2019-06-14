#pragma once

#include <bnl/core/export.hpp>

#include <bnl/buffer.hpp>

#include <cstdint>
#include <functional>
#include <system_error>

namespace bnl {
namespace quic {

class BNL_CORE_EXPORT event {
public:
  using handler = const std::function<void(event, std::error_code &ec)> &;

  enum class type { data, error };

  struct payload {
    using data = buffer;

    struct error {
      enum class type { connection, stream };
      type type;
      uint64_t code;
    };
  };

  event() noexcept;

  event(uint64_t id, bool fin, payload::data data) noexcept;
  event(uint64_t id, bool fin, payload::error error) noexcept;

  event(const event &other) noexcept;
  event(event &&other) noexcept;

  ~event() noexcept;

  event &operator=(const event &) = delete;
  event &operator=(event &&) = delete;

  operator type() const noexcept; // NOLINT

private:
  const type type_;

public:
  const uint64_t id;
  const bool fin;

  union {
    payload::data data;
    payload::error error;
  };
};

} // namespace quic
} // namespace bnl
