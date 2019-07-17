#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/base/result.hpp>
#include <bnl/base/template.hpp>
#include <bnl/http3/codec/varint.hpp>
#include <bnl/http3/export.hpp>
#include <bnl/http3/settings.hpp>

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

  frame(const frame &other) = default;
  frame &operator=(const frame &other) = delete;

  operator type() const noexcept; // NOLINT

private:
  const type type_;

  BNL_HTTP3_EXPORT friend bool operator==(const frame &lhs, const frame &rhs);

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

BNL_HTTP3_EXPORT bool
operator==(const frame &lhs, const frame &rhs);
BNL_HTTP3_EXPORT bool
operator!=(const frame &lhs, const frame &rhs);

class BNL_HTTP3_EXPORT frame::encoder {
public:
  explicit encoder(const log::api *logger) noexcept;

  encoder(encoder &&other) = default;
  encoder &operator=(encoder &&other) = default;

  base::result<size_t> encoded_size(const frame &frame) const noexcept;

  base::result<size_t> encode(uint8_t *dest, const frame &frame) const noexcept;

  base::result<base::buffer> encode(const frame &frame) const;

private:
  base::result<uint64_t> payload_size(const frame &frame) const noexcept;

private:
  varint::encoder varint_;

  const log::api *logger_;
};

class BNL_HTTP3_EXPORT frame::decoder {
public:
  explicit decoder(const log::api *logger) noexcept;

  decoder(decoder &&other) = default;
  decoder &operator=(decoder &&other) = default;

  template<typename Sequence>
  base::result<frame::type> peek(const Sequence &encoded) const noexcept;

  template<typename Sequence>
  base::result<frame> decode(Sequence &encoded) const noexcept;

private:
  template<typename Lookahead>
  base::result<frame> decode_single(Lookahead &lookahead) const noexcept;

  template<typename Lookahead>
  base::result<uint8_t> uint8_decode(Lookahead &lookahead) const noexcept;

private:
  varint::decoder varint_;

  const log::api *logger_;
};

#define BNL_HTTP3_FRAME_PEEK_IMPL(T)                                           \
  template BNL_HTTP3_EXPORT base::result<frame::type> frame::decoder::peek<T>( \
    const T &) const noexcept // NOLINT

#define BNL_HTTP3_FRAME_DECODE_IMPL(T)                                         \
  template BNL_HTTP3_EXPORT base::result<frame> frame::decoder::decode<T>(     \
    T &) /* NOLINT */ const noexcept

BNL_BASE_SEQUENCE_DECL(BNL_HTTP3_FRAME_PEEK_IMPL);
BNL_BASE_LOOKAHEAD_DECL(BNL_HTTP3_FRAME_PEEK_IMPL);

BNL_BASE_SEQUENCE_DECL(BNL_HTTP3_FRAME_DECODE_IMPL);
BNL_BASE_LOOKAHEAD_DECL(BNL_HTTP3_FRAME_DECODE_IMPL);

}
}
