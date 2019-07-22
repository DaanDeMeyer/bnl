#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/base/export.hpp>
#include <bnl/base/function_view.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/http3/header.hpp>
#include <bnl/http3/settings.hpp>
#include <bnl/quic/event.hpp>

namespace bnl {
namespace http3 {

class BNL_BASE_EXPORT event {
public:
  using handler = base::function_view<result<void>(event)>;

  enum class type { settings, header, body };

  struct payload {
    using settings = http3::settings;

    struct header {
      uint64_t id;
      bool fin;
      http3::header header;
    };

    using body = quic::data;
  };

  event(payload::settings settings) noexcept; // NOLINT
  event(payload::header header) noexcept; // NOLINT
  event(payload::body body) noexcept; // NOLINT

  event(event &&other) noexcept;
  event &operator=(event &&) = delete;

  ~event() noexcept;

  operator type() const noexcept; // NOLINT

private:
  const type type_;

public:
  union {
    payload::settings settings;
    payload::header header;
    payload::body body;
  };
};

}
}
