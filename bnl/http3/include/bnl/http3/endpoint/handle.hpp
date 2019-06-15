#pragma once

#include <bnl/http3/export.hpp>
#include <bnl/http3/header.hpp>

#include <bnl/buffer.hpp>

#include <cstdint>
#include <system_error>

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace endpoint {
namespace shared {
namespace request {
class sender;
}
} // namespace shared

class BNL_HTTP3_EXPORT handle {
public:
  handle() = default;
  explicit handle(uint64_t id,
                  endpoint::shared::request::sender *ref,
                  const log::api *logger) noexcept;

  BNL_NO_COPY(handle);
  BNL_CUSTOM_MOVE(handle);

  ~handle() noexcept;

  bool valid() const noexcept;

  uint64_t id() const noexcept;

  void header(header_view header, std::error_code &ec);
  void body(buffer body, std::error_code &ec);

  void start(std::error_code &ec) noexcept;
  void fin(std::error_code &ec) noexcept;

private:
  friend endpoint::shared::request::sender;

  uint64_t id_ = UINT64_MAX;
  endpoint::shared::request::sender *ref_ = nullptr;
  const log::api *logger_ = nullptr;
};

} // namespace endpoint
} // namespace http3
} // namespace bnl