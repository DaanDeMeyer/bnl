#include <bnl/http3/codec/frame.hpp>

#include <bnl/http3/error.hpp>
#include <bnl/util/enum.hpp>
#include <bnl/util/error.hpp>

#include <bnl/base/error.hpp>

namespace bnl {
namespace http3 {

frame::encoder::encoder(const log::api *logger) noexcept
  : varint_(logger)
  , logger_(logger)
{}

base::result<uint64_t>
frame::encoder::payload_size(const frame &frame) const noexcept
{
  uint64_t payload_size = 0;

  switch (frame) {
    case frame::type::data:
      payload_size += frame.data.size;
      break;

    case frame::type::headers:
      payload_size += frame.headers.size;
      break;

    case frame::type::priority: {
      payload_size++; // PT size + DT size + Empty size = 1 byte. See
                      // https://quicwg.org/base-drafts/draft-ietf-quic-http.html#frame-priority

      uint64_t prioritized_element_id = frame.priority.prioritized_element_id;
      payload_size += TRY(varint_.encoded_size(prioritized_element_id));

      uint64_t element_dependency_id = frame.priority.element_dependency_id;
      payload_size += TRY(varint_.encoded_size(element_dependency_id));

      payload_size++; // Weight
      break;
    }

    case frame::type::cancel_push: {
      uint64_t push_id = frame.cancel_push.push_id;
      payload_size += TRY(varint_.encoded_size(push_id));
      break;
    }

    case frame::type::settings:
      for (auto setting : frame.settings.array()) {
        payload_size += TRY(varint_.encoded_size(setting.first));
        payload_size += TRY(varint_.encoded_size(setting.second));
      }
      break;

    case frame::type::push_promise: {
      uint64_t push_id = frame.push_promise.push_id;
      payload_size += TRY(varint_.encoded_size(push_id));
      payload_size += frame.push_promise.size;
      break;
    }

    case frame::type::goaway: {
      payload_size += TRY(varint_.encoded_size(frame.goaway.stream_id));
      break;
    }

    case frame::type::max_push_id: {
      uint64_t push_id = frame.max_push_id.push_id;
      payload_size += TRY(varint_.encoded_size(push_id));
      break;
    }

    case frame::type::duplicate_push:
      uint64_t push_id = frame.duplicate_push.push_id;
      payload_size += TRY(varint_.encoded_size(push_id));
      break;
  }

  return payload_size;
}

base::result<size_t>
frame::encoder::encoded_size(const frame &frame) const noexcept
{
  uint64_t payload_size = TRY(this->payload_size(frame));
  size_t payload_encoded_size = 0;

  // `payload_size` includes the size of the DATA frame payload and
  // HEADERS/PUSH_PROMISE header block but these are not encoded in `encode`
  // (this is left to the user) so we calculate the encoded payload size of
  // HEADERS, DATA and PUSH_PROMISE frames manually and delegate the rest to
  // `payload_size`.
  switch (frame) {
    case frame::type::headers:
    case frame::type::data:
      break;

    case frame::type::push_promise: {
      uint64_t push_id = frame.push_promise.push_id;
      payload_encoded_size = TRY(varint_.encoded_size(push_id));
      break;
    }

    default:
      // HEADERS, DATA and PUSH_PROMISE frames are the only frames that might
      // have a payload size exceeding a `size_t` value and those are handled
      // separately so the cast is safe.
      payload_encoded_size = static_cast<size_t>(payload_size);
      break;
  }

  size_t encoded_size = 0;

  uint64_t type = util::to_underlying(frame.type_);
  encoded_size += TRY(varint_.encoded_size(type));
  encoded_size += TRY(varint_.encoded_size(payload_size));
  encoded_size += payload_encoded_size;

  return encoded_size;
}

base::result<size_t>
frame::encoder::encode(uint8_t *dest, const frame &frame) const noexcept
{
  CHECK(dest != nullptr, base::error::invalid_argument);

  size_t encoded_size = TRY(this->encoded_size(frame));
  uint8_t *begin = dest;

  uint64_t payload_size = TRY(this->payload_size(frame));

  dest += TRY(varint_.encode(dest, util::to_underlying(frame.type_)));
  dest += TRY(varint_.encode(dest, payload_size));

  switch (frame) {
    case frame::type::data:
      break;
    case frame::type::headers:
      break;
    case frame::type::priority: {
      uint8_t prioritized_element_type =
        util::to_underlying(frame.priority.prioritized_element_type);
      uint8_t element_dependency_type =
        util::to_underlying(frame.priority.element_dependency_type);

      uint8_t byte = 0;
      byte = static_cast<uint8_t>(
        byte | static_cast<uint8_t>((prioritized_element_type << 6U)));
      byte = static_cast<uint8_t>(
        byte | static_cast<uint8_t>(element_dependency_type << 4U));
      byte &= 0xf0U;

      *dest++ = byte;

      uint64_t prioritized_element_id = frame.priority.prioritized_element_id;
      uint64_t element_dependency_id = frame.priority.element_dependency_id;

      dest += TRY(varint_.encode(dest, prioritized_element_id));
      dest += TRY(varint_.encode(dest, element_dependency_id));

      *dest++ = frame.priority.weight;
      break;
    }
    case frame::type::cancel_push:
      dest += TRY(varint_.encode(dest, frame.cancel_push.push_id));
      break;
    case frame::type::settings:
      for (auto setting : frame.settings.array()) {
        dest += TRY(varint_.encode(dest, setting.first));
        dest += TRY(varint_.encode(dest, setting.second));
      }
      break;
    case frame::type::push_promise:
      dest += TRY(varint_.encode(dest, frame.push_promise.push_id));
      break;
    case frame::type::goaway:
      dest += TRY(varint_.encode(dest, frame.goaway.stream_id));
      break;
    case frame::type::max_push_id:
      dest += TRY(varint_.encode(dest, frame.max_push_id.push_id));
      break;
    case frame::type::duplicate_push:
      dest += TRY(varint_.encode(dest, frame.duplicate_push.push_id));
      break;
  }

  ASSERT(encoded_size == static_cast<size_t>(dest - begin));

  return encoded_size;
}

base::result<base::buffer>
frame::encoder::encode(const frame &frame) const
{
  size_t encoded_size = TRY(this->encoded_size(frame));
  base::buffer encoded(encoded_size);

  ASSERT(encoded_size == TRY(encode(encoded.data(), frame)));

  return encoded;
}

} // namespace http3
} // namespace bnl
