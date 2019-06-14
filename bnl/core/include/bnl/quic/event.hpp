#pragma once

#include <bnl/core/export.hpp>

#include <bnl/buffer.hpp>

#include <cstdint>
#include <functional>
#include <system_error>

namespace bnl {
namespace quic {

struct BNL_CORE_EXPORT data {
  data() = default;
  data(uint64_t id, bool fin, buffer buffer)
      : id(id), fin(fin), buffer(std::move(buffer))
  {}

  uint64_t id = 0;
  bool fin = false;
  bnl::buffer buffer;
};

struct BNL_CORE_EXPORT error {
  enum class type { connection, stream };
  type type = type::connection;
  uint64_t code = 3; // HTTP_INTERNAL_ERROR
};

class BNL_CORE_EXPORT event {
public:
  using handler = const std::function<void(event, std::error_code &ec)> &;

  enum class type { data, error };

  struct payload {
    using data = quic::data;
    using error = quic::error;
  };

  event() noexcept;

  event(payload::data data) noexcept;   // NOLINT
  event(payload::error error) noexcept; // NOLINT

  event(const event &other) noexcept;
  event(event &&other) noexcept;

  ~event() noexcept;

  event &operator=(const event &) = delete;
  event &operator=(event &&) = delete;

  operator type() const noexcept; // NOLINT

private:
  const type type_;

public:
  union {
    payload::data data;
    payload::error error;
  };
};

} // namespace quic
} // namespace bnl
