#pragma once

#include <h3c/export.hpp>
#include <h3c/settings.hpp>

#include <cstddef>
#include <cstdint>
#include <system_error>

// https://quicwg.org/base-drafts/draft-ietf-quic-http.html#rfc.section.4

namespace h3c {

class logger;

struct frame {
  // We don't use an enum class because of the possibility of unknown frame
  // types which also have to be handled. We don't use a normal enum because
  // then the type identifiers would leak into `frame`'s scope.

  struct type {
    static constexpr uint64_t data = 0x0;
    static constexpr uint64_t headers = 0x1;
    static constexpr uint64_t priority = 0x2;
    static constexpr uint64_t cancel_push = 0x3;
    static constexpr uint64_t settings = 0x4;
    static constexpr uint64_t push_promise = 0x5;
    static constexpr uint64_t goaway = 0x7;
    static constexpr uint64_t max_push_id = 0xd;
    static constexpr uint64_t duplicate_push = 0xe;
  };

  uint64_t type;

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

      type prioritized_element_type;
      type element_dependency_type;
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
      payload::headers headers;
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

  // We don't store the frame length since (for now) it's easier to calculate it
  // when needed. This also prevents it from getting stale.

  union {
    payload::data data;
    payload::headers headers;
    payload::priority priority;
    payload::settings settings;
    payload::cancel_push cancel_push;
    payload::push_promise push_promise;
    payload::goaway goaway;
    payload::max_push_id max_push_id;
    payload::duplicate_push duplicate_push;
  };

  H3C_EXPORT static size_t encoded_size(const frame &frame);

  H3C_EXPORT static std::error_code encode(uint8_t *dest,
                                           size_t size,
                                           const frame &frame,
                                           size_t *encoded_size,
                                           const logger *logger);

  H3C_EXPORT static std::error_code decode(const uint8_t *src,
                                           size_t size,
                                           frame *frame,
                                           size_t *encoded_size,
                                           const logger *logger);
};

} // namespace h3c
