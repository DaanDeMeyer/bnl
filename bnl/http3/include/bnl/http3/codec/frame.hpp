#pragma once

#include <bnl/http3/export.hpp>
#include <bnl/http3/settings.hpp>

#include <bnl/http3/codec/varint.hpp>

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/base/macro.hpp>

#include <cstddef>
#include <cstdint>
#include <system_error>

// https://quicwg.org/base-drafts/draft-ietf-quic-http.html#rfc.section.4

namespace bnl {

namespace log {
class api;
}

namespace http3 {

class BNL_HTTP3_EXPORT frame {
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

    using settings = http3::settings;

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

  frame() noexcept;
  frame(frame::payload::data data) noexcept;                     // NOLINT
  frame(frame::payload::headers headers) noexcept;               // NOLINT
  frame(frame::payload::priority priority) noexcept;             // NOLINT
  frame(frame::payload::settings settings) noexcept;             // NOLINT
  frame(frame::payload::cancel_push cancel_push) noexcept;       // NOLINT
  frame(frame::payload::push_promise push_promise) noexcept;     // NOLINT
  frame(frame::payload::goaway goaway) noexcept;                 // NOLINT
  frame(frame::payload::max_push_id max_push_id) noexcept;       // NOLINT
  frame(frame::payload::duplicate_push duplicate_push) noexcept; // NOLINT

  operator type() const noexcept; // NOLINT

private:
  const type type_;

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

class BNL_HTTP3_EXPORT frame::encoder {
public:
  explicit encoder(const log::api *logger) noexcept;

  BNL_BASE_MOVE_ONLY(encoder);

  size_t encoded_size(const frame &frame, std::error_code &ec) const noexcept;

  size_t encode(uint8_t *dest, const frame &frame, std::error_code &ec) const
      noexcept;

  base::buffer encode(const frame &frame, std::error_code &ec) const;

private:
  const log::api *logger_;

  varint::encoder varint_;

  uint64_t payload_size(const frame &frame, std::error_code &ec) const noexcept;
};

class BNL_HTTP3_EXPORT frame::decoder {
public:
  explicit decoder(const log::api *logger) noexcept;

  BNL_BASE_MOVE_ONLY(decoder);

  frame::type peek(const base::buffer &encoded, std::error_code &ec) const
      noexcept;

  frame::type peek(const base::buffers &encoded, std::error_code &ec) const
      noexcept;

  frame decode(base::buffer &encoded, std::error_code &ec) const noexcept;

  frame decode(base::buffers &encoded, std::error_code &ec) const noexcept;

private:
  const log::api *logger_;

  varint::decoder varint_;

  template <typename Sequence>
  frame::type peek(const Sequence &encoded, std::error_code &ec) const noexcept;

  template <typename Sequence>
  frame decode(Sequence &encoded, std::error_code &ec) const noexcept;

  template <typename Lookahead>
  frame decode(Lookahead &encoded,
               bool *is_unknown_frame_type,
               std::error_code &ec) const noexcept;

  template <typename Lookahead>
  uint8_t uint8_decode(Lookahead &encoded, std::error_code &ec) const noexcept;
};

} // namespace http3
} // namespace bnl
