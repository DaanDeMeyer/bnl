#pragma once

#include <h3c/error.hpp>
#include <h3c/event.hpp>

#include <util/enum.hpp>
#include <util/log.hpp>

#define STREAM_ENCODE_THROW(err)                                               \
  ec = err;                                                                    \
                                                                               \
  if (ec != error::idle) {                                                     \
    state_ = state::error;                                                     \
    LOG_E("{}", ec.message());                                                 \
  }                                                                            \
                                                                               \
  return {};                                                                   \
  (void) 0

#define STREAM_ENCODE_TRY(expression)                                          \
  [&]() {                                                                      \
    ec = {};                                                                   \
                                                                               \
    auto result = expression;                                                  \
                                                                               \
    if (ec && ec != error::idle) {                                             \
      state_ = state::error;                                                   \
    }                                                                          \
                                                                               \
    return result;                                                             \
  }();                                                                         \
  if (ec && ec != error::idle) {                                               \
    return {};                                                                 \
  }                                                                            \
  (void) 0

#define STREAM_DECODE_START()                                                  \
  if (!buffered_.empty()) {                                                    \
    data.buffer = buffer::concat(buffered_, data.buffer);                      \
  }                                                                            \
  (void) 0

#define STREAM_DECODE_SHARED()                                                 \
  if (ec == error::incomplete) {                                               \
    if (data.fin) {                                                            \
      ec = error::malformed_frame;                                             \
    } else {                                                                   \
      if (!data.buffer.empty()) {                                              \
        if (buffered_.empty()) {                                               \
          buffered_ = data.buffer.slice(data.buffer.size());                   \
        } else {                                                               \
          buffered_ = buffer::concat(buffered_, data.buffer);                  \
        }                                                                      \
                                                                               \
        data.buffer.advance(data.buffer.size());                               \
      }                                                                        \
    }                                                                          \
  }                                                                            \
                                                                               \
  if (ec && ec != error::unknown && ec != error::incomplete) {                 \
    state_ = state::error;                                                     \
  }                                                                            \
  (void) 0

#define STREAM_DECODE_THROW(err)                                               \
  {                                                                            \
    ec = err;                                                                  \
                                                                               \
    STREAM_DECODE_SHARED();                                                    \
                                                                               \
    if (ec && ec != error::unknown && ec != error::incomplete) {               \
      LOG_E("{}", ec.message());                                               \
    }                                                                          \
                                                                               \
    return {};                                                                 \
  }                                                                            \
  (void) 0

#define STREAM_DECODE_TRY(expression)                                          \
  [&]() {                                                                      \
    ec = {};                                                                   \
                                                                               \
    auto result = expression;                                                  \
                                                                               \
    STREAM_DECODE_SHARED();                                                    \
                                                                               \
    return result;                                                             \
  }();                                                                         \
  if (ec && ec != error::unknown && ec != error::incomplete) {                 \
    return {};                                                                 \
  };                                                                           \
  (void) 0
