#include <bnl/http3/codec/frame.hpp>

#include <bnl/base/enum.hpp>
#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>

namespace bnl {
namespace http3 {

result<uint64_t>
payload_size(const frame &frame) noexcept
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
      payload_size += BNL_TRY(varint::encoded_size(prioritized_element_id));

      uint64_t element_dependency_id = frame.priority.element_dependency_id;
      payload_size += BNL_TRY(varint::encoded_size(element_dependency_id));

      payload_size++; // Weight
      break;
    }

    case frame::type::cancel_push: {
      uint64_t push_id = frame.cancel_push.push_id;
      payload_size += BNL_TRY(varint::encoded_size(push_id));
      break;
    }

    case frame::type::settings:
      for (auto setting : frame.settings.array()) {
        if (setting.first == setting::num_placeholders && setting.second == 0) {
          continue;
        }

        payload_size += BNL_TRY(varint::encoded_size(setting.first));
        payload_size += BNL_TRY(varint::encoded_size(setting.second));
      }
      break;

    case frame::type::push_promise: {
      uint64_t push_id = frame.push_promise.push_id;
      payload_size += BNL_TRY(varint::encoded_size(push_id));
      payload_size += frame.push_promise.size;
      break;
    }

    case frame::type::goaway: {
      payload_size += BNL_TRY(varint::encoded_size(frame.goaway.stream_id));
      break;
    }

    case frame::type::max_push_id: {
      uint64_t push_id = frame.max_push_id.push_id;
      payload_size += BNL_TRY(varint::encoded_size(push_id));
      break;
    }

    case frame::type::duplicate_push:
      uint64_t push_id = frame.duplicate_push.push_id;
      payload_size += BNL_TRY(varint::encoded_size(push_id));
      break;
  }

  return payload_size;
}

result<size_t>
frame::encoded_size(const frame &frame) noexcept
{
  uint64_t payload_size = BNL_TRY(http3::payload_size(frame));
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
      payload_encoded_size = BNL_TRY(varint::encoded_size(push_id));
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

  uint64_t type = enumeration::value(frame.type_);
  encoded_size += BNL_TRY(varint::encoded_size(type));
  encoded_size += BNL_TRY(varint::encoded_size(payload_size));
  encoded_size += payload_encoded_size;

  return encoded_size;
}

result<size_t>
frame::encode(uint8_t *dest, const frame &frame) noexcept
{
  assert(dest != nullptr);

  uint8_t *begin = dest;

  uint64_t payload_size = BNL_TRY(http3::payload_size(frame));

  dest += BNL_TRY(varint::encode(dest, enumeration::value(frame.type_)));
  dest += BNL_TRY(varint::encode(dest, payload_size));

  switch (frame) {
    case frame::type::data:
      break;
    case frame::type::headers:
      break;
    case frame::type::priority: {
      uint8_t prioritized_element_type =
        enumeration::value(frame.priority.prioritized_element_type);
      uint8_t element_dependency_type =
        enumeration::value(frame.priority.element_dependency_type);

      uint8_t byte = 0;
      byte = static_cast<uint8_t>(
        byte | static_cast<uint8_t>((prioritized_element_type << 6U)));
      byte = static_cast<uint8_t>(
        byte | static_cast<uint8_t>(element_dependency_type << 4U));
      byte &= 0xf0U;

      *dest++ = byte;

      uint64_t prioritized_element_id = frame.priority.prioritized_element_id;
      uint64_t element_dependency_id = frame.priority.element_dependency_id;

      dest += BNL_TRY(varint::encode(dest, prioritized_element_id));
      dest += BNL_TRY(varint::encode(dest, element_dependency_id));

      *dest++ = frame.priority.weight;
      break;
    }
    case frame::type::cancel_push:
      dest += BNL_TRY(varint::encode(dest, frame.cancel_push.push_id));
      break;
    case frame::type::settings:
      for (auto setting : frame.settings.array()) {
        if (setting.first == setting::num_placeholders && setting.second == 0) {
          continue;
        }

        dest += BNL_TRY(varint::encode(dest, setting.first));
        dest += BNL_TRY(varint::encode(dest, setting.second));
      }
      break;
    case frame::type::push_promise:
      dest += BNL_TRY(varint::encode(dest, frame.push_promise.push_id));
      break;
    case frame::type::goaway:
      dest += BNL_TRY(varint::encode(dest, frame.goaway.stream_id));
      break;
    case frame::type::max_push_id:
      dest += BNL_TRY(varint::encode(dest, frame.max_push_id.push_id));
      break;
    case frame::type::duplicate_push:
      dest += BNL_TRY(varint::encode(dest, frame.duplicate_push.push_id));
      break;
  }

  return static_cast<size_t>(dest - begin);
}

result<base::buffer>
frame::encode(const frame &frame)
{
  size_t encoded_size = BNL_TRY(frame::encoded_size(frame));
  base::buffer encoded(encoded_size);

  BNL_TRY(encode(encoded.data(), frame));

  return encoded;
}

}
}
