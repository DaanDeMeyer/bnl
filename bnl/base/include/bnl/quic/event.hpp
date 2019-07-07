#pragma once

#include <bnl/base/export.hpp>

#include <bnl/base/buffer.hpp>
#include <bnl/base/function_view.hpp>

#include <cstdint>
#include <system_error>

namespace bnl {
namespace quic {

class BNL_BASE_EXPORT event {
public:
  using handler = base::function_view<void(event, std::error_code &ec)>;

  enum class type { data, error };

  struct payload {
    using data = base::buffer;
    using error = std::error_code;
  };

  event() noexcept;

  event(uint64_t id, bool fin, payload::data data) noexcept;   // NOLINT
  event(uint64_t id, bool fin, payload::error error) noexcept; // NOLINT

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
