#pragma once

#include <h3c/export.hpp>
#include <h3c/settings.hpp>
#include <h3c/util/class.hpp>
#include <h3c/varint.hpp>

#include <cstddef>
#include <cstdint>
#include <system_error>

// https://quicwg.org/base-drafts/draft-ietf-quic-http.html#rfc.section.4

namespace h3c {

class logger;

class frame {
public:
  class encoder;
  class decoder;

  enum class type : uint64_t {
    data = 0x0,
    headers = 0x1,
    priority = 0x2,
    cancel_push = 0x3,
    settings = 0x4,
    push_promise = 0x5,
    goaway = 0x7,
    max_push_id = 0xd,
    duplicate_push = 0xe
  };

  struct payload {
    struct data {
      uint64_t size;
    };

    struct headers {
      uint64_t size;
    };

    struct priority {
      enum class type : uint8_t {
        request = 0x0,
        push = 0x1,
        placeholder = 0x2,
        // Only valid for prioritized_element_type.
        current = 0x3,
        // Only valid for element_dependency_type.
        root = 0x3
      };

      priority::type prioritized_element_type;
      priority::type element_dependency_type;
      uint64_t prioritized_element_id;
      uint64_t element_dependency_id;
      uint8_t weight;
    };

    using settings = h3c::settings;

    struct cancel_push {
      uint64_t push_id;
    };

    struct push_promise {
      uint64_t push_id;
      uint64_t size;
    };

    struct goaway {
      uint64_t stream_id;
    };

    struct max_push_id {
      uint64_t push_id;
    };

    struct duplicate_push {
      uint64_t push_id;
    };
  };

  // We allow implicit conversions from a frame payload into a frame.

  // clang-format off
  H3C_EXPORT frame() noexcept;
  H3C_EXPORT frame(frame::payload::data data) noexcept;                     // NOLINT
  H3C_EXPORT frame(frame::payload::headers headers) noexcept;               // NOLINT
  H3C_EXPORT frame(frame::payload::priority priority) noexcept;             // NOLINT
  H3C_EXPORT frame(frame::payload::settings settings) noexcept;             // NOLINT
  H3C_EXPORT frame(frame::payload::cancel_push cancel_push) noexcept;       // NOLINT
  H3C_EXPORT frame(frame::payload::push_promise push_promise) noexcept;     // NOLINT
  H3C_EXPORT frame(frame::payload::goaway goaway) noexcept;                 // NOLINT
  H3C_EXPORT frame(frame::payload::max_push_id max_push_id) noexcept;       // NOLINT
  H3C_EXPORT frame(frame::payload::duplicate_push duplicate_push) noexcept; // NOLINT
  // clang-format on

  H3C_EXPORT operator type() const noexcept; // NOLINT

private:
  const type type_; // NOLINT

public:
  union {
    const payload::data data;
    const payload::headers headers;
    const payload::priority priority;
    const payload::settings settings;
    const payload::cancel_push cancel_push;
    const payload::push_promise push_promise;
    const payload::goaway goaway;
    const payload::max_push_id max_push_id;
    const payload::duplicate_push duplicate_push;
  };
};

class frame::encoder {
public:
  H3C_EXPORT explicit encoder(logger *logger) noexcept;

  H3C_MOVE_ONLY(encoder)

  H3C_EXPORT size_t encoded_size(const frame &frame, std::error_code &ec) const
      noexcept;

  H3C_EXPORT size_t encode(uint8_t *dest,
                           const frame &frame,
                           std::error_code &ec) const noexcept;

  H3C_EXPORT buffer encode(const frame &frame, std::error_code &ec) const;

private:
  logger *logger_;

  varint::encoder varint_;

  uint64_t payload_size(const frame &frame, std::error_code &ec) const noexcept;
};

class frame::decoder {
public:
  H3C_EXPORT explicit decoder(logger *logger) noexcept;

  H3C_MOVE_ONLY(decoder)

  H3C_EXPORT frame decode(buffer &encoded, std::error_code &ec) const noexcept;

private:
  logger *logger_;

  varint::decoder varint_;
};

} // namespace h3c
