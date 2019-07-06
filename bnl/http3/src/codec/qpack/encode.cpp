#include <bnl/http3/codec/qpack.hpp>

#include <bnl/http3/error.hpp>

#include <bnl/util/error.hpp>
#include <bnl/util/string.hpp>

#include <algorithm>

#include "encode_generated.cpp"

static constexpr size_t QPACK_PREFIX_ENCODED_SIZE = 2;

namespace bnl {
namespace http3 {

qpack::encoder::encoder(const log::api *logger) noexcept
    : logger_(logger), prefix_int_(logger), literal_(logger)
{}

uint64_t qpack::encoder::count() const noexcept
{
  return count_;
}

size_t qpack::encoder::encoded_size(header_view header,
                                    std::error_code &ec) const noexcept
{
  const char *name = reinterpret_cast<const char *>(header.name().data());
  size_t size = header.name().size();

  CHECK_MSG(util::is_lowercase(name, size), error::malformed_header,
            "Header ({}) is not lowercase", fmt::string_view(name, size));

  size_t encoded_size = 0;

  if (state_ == state::prefix) {
    encoded_size += QPACK_PREFIX_ENCODED_SIZE;
  }

  qpack::table::fixed::type type;
  uint8_t index = 0;
  std::tie(type, index) = qpack::table::fixed::find_index(header);

  switch (type) {

    case qpack::table::fixed::type::header_value: {
      encoded_size += prefix_int_.encoded_size(index, 6);
      break;
    }

    case qpack::table::fixed::type::header_only: {
      encoded_size += prefix_int_.encoded_size(index, 4);

      size_t value_encoded_size = literal_.encoded_size(header.value(), 7);
      encoded_size += value_encoded_size;
      break;
    }

    case qpack::table::fixed::type::missing: {
      size_t name_encoded_size = literal_.encoded_size(header.name(), 3);
      encoded_size += name_encoded_size;

      size_t value_encoded_size = literal_.encoded_size(header.value(), 7);
      encoded_size += value_encoded_size;
      break;
    }

    default:
      NOTREACHED();
  }

  return encoded_size;
}

static constexpr uint8_t INDEXED_HEADER_FIELD_PREFIX = 0xc0;
static constexpr uint8_t LITERAL_WITH_NAME_REFERENCE_PREFIX = 0x50;
static constexpr uint8_t LITERAL_WITHOUT_NAME_REFERENCE_PREFIX = 0x20;
static constexpr uint8_t LITERAL_NO_PREFIX = 0x00;

size_t qpack::encoder::encode(uint8_t *dest,
                              header_view header,
                              std::error_code &ec) noexcept
{
  CHECK(dest != nullptr, error::invalid_argument);

  size_t encoded_size = TRY(this->encoded_size(header, ec));
  uint8_t *begin = dest;

  if (state_ == state::prefix) {
    std::fill(dest, dest + QPACK_PREFIX_ENCODED_SIZE, static_cast<uint8_t>(0U));
    dest += QPACK_PREFIX_ENCODED_SIZE;
    state_ = state::header;
  }

  qpack::table::fixed::type type;
  uint8_t index = 0;
  std::tie(type, index) = qpack::table::fixed::find_index(header);

  switch (type) {

    case qpack::table::fixed::type::header_value:
      *dest = INDEXED_HEADER_FIELD_PREFIX;
      dest += prefix_int_.encode(dest, index, 6);
      break;

    case qpack::table::fixed::type::header_only:
      *dest = LITERAL_WITH_NAME_REFERENCE_PREFIX;
      dest += prefix_int_.encode(dest, index, 4);

      *dest = LITERAL_NO_PREFIX;
      dest += literal_.encode(dest, header.value(), 7);
      break;

    case qpack::table::fixed::type::missing:
      *dest = LITERAL_WITHOUT_NAME_REFERENCE_PREFIX;
      dest += literal_.encode(dest, header.name(), 3);

      *dest = LITERAL_NO_PREFIX;
      dest += literal_.encode(dest, header.value(), 7);
      break;

    default:
      NOTREACHED();
  }

  ASSERT(static_cast<size_t>(dest - begin) == encoded_size);

  count_ += encoded_size;

  return encoded_size;
}

buffer qpack::encoder::encode(header_view header, std::error_code &ec)
{
  size_t encoded_size = TRY(this->encoded_size(header, ec));
  buffer encoded(encoded_size);

  ASSERT(encoded_size == TRY(encode(encoded.data(), header, ec)));

  return encoded;
}

} // namespace http3
} // namespace bnl
