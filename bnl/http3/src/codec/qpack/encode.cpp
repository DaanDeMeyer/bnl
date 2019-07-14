#include <bnl/http3/codec/qpack.hpp>

#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/util/error.hpp>
#include <bnl/util/string.hpp>

#include <algorithm>

#include "encode_generated.cpp"

static constexpr size_t QPACK_PREFIX_ENCODED_SIZE = 2;

namespace bnl {
namespace http3 {
namespace qpack {

encoder::encoder(const log::api* logger) noexcept
  : prefix_int_(logger)
  , literal_(logger)
  , logger_(logger)
{}

uint64_t
encoder::count() const noexcept
{
  return count_;
}

base::result<size_t>
encoder::encoded_size(header_view header) const noexcept
{
  if (!util::is_lowercase(header.name())) {
    fmt::string_view view(header.name().data(), header.name().size());
    LOG_E("Header ({}) is not lowercase", view);
    THROW(error::malformed_header);
  }

  size_t encoded_size = 0;

  if (state_ == state::prefix) {
    encoded_size += QPACK_PREFIX_ENCODED_SIZE;
  }

  table::fixed::type type;
  uint8_t index = 0;
  std::tie(type, index) = table::fixed::find_index(header);

  switch (type) {

    case table::fixed::type::header_value: {
      encoded_size += prefix_int_.encoded_size(index, 6);
      break;
    }

    case table::fixed::type::header_only: {
      encoded_size += prefix_int_.encoded_size(index, 4);

      size_t value_encoded_size = literal_.encoded_size(header.value(), 7);
      encoded_size += value_encoded_size;
      break;
    }

    case table::fixed::type::missing: {
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

base::result<size_t>
encoder::encode(uint8_t* dest, header_view header) noexcept
{
  CHECK(dest != nullptr, base::error::invalid_argument);

  size_t encoded_size = TRY(this->encoded_size(header));
  uint8_t* begin = dest;

  if (state_ == state::prefix) {
    std::fill(dest, dest + QPACK_PREFIX_ENCODED_SIZE, static_cast<uint8_t>(0U));
    dest += QPACK_PREFIX_ENCODED_SIZE;
    state_ = state::header;
  }

  table::fixed::type type;
  uint8_t index = 0;
  std::tie(type, index) = table::fixed::find_index(header);

  switch (type) {

    case table::fixed::type::header_value:
      *dest = INDEXED_HEADER_FIELD_PREFIX;
      dest += prefix_int_.encode(dest, index, 6);
      break;

    case table::fixed::type::header_only:
      *dest = LITERAL_WITH_NAME_REFERENCE_PREFIX;
      dest += prefix_int_.encode(dest, index, 4);

      *dest = LITERAL_NO_PREFIX;
      dest += literal_.encode(dest, header.value(), 7);
      break;

    case table::fixed::type::missing:
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

base::result<base::buffer>
encoder::encode(header_view header)
{
  size_t encoded_size = TRY(this->encoded_size(header));
  base::buffer encoded(encoded_size);

  ASSERT(encoded_size == TRY(encode(encoded.data(), header)));

  return encoded;
}

} // namespace qpack
} // namespace http3
} // namespace bnl
