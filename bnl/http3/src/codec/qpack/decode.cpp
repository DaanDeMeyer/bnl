#include <bnl/http3/codec/qpack.hpp>

#include <bnl/http3/error.hpp>

#include <bnl/util/error.hpp>
#include <bnl/util/string.hpp>

#include "decode_generated.cpp"

namespace bnl {
namespace http3 {

// TODO: Find out what is best for max header and max value size.
qpack::decoder::decoder(const log::api *logger)
    : logger_(logger), prefix_int_(logger), literal_(logger)
{}

uint64_t qpack::decoder::count() const noexcept
{
  return count_;
}

static constexpr size_t QPACK_PREFIX_ENCODED_SIZE = 2;

static constexpr uint8_t INDEXED_HEADER_FIELD_PREFIX = 0x80;
static constexpr uint8_t LITERAL_WITH_NAME_REFERENCE_PREFIX = 0x40;
static constexpr uint8_t LITERAL_WITHOUT_NAME_REFERENCE_PREFIX = 0x20;

enum class instruction {
  indexed_header_field,
  literal_with_name_reference,
  literal_without_name_reference,
  unknown
};

static instruction qpack_instruction_type(uint8_t byte)
{
  if ((byte & INDEXED_HEADER_FIELD_PREFIX) == INDEXED_HEADER_FIELD_PREFIX) {
    return instruction::indexed_header_field;
  }

  if ((byte & LITERAL_WITH_NAME_REFERENCE_PREFIX) ==
      LITERAL_WITH_NAME_REFERENCE_PREFIX) {
    return instruction::literal_with_name_reference;
  }

  if ((byte & LITERAL_WITHOUT_NAME_REFERENCE_PREFIX) ==
      LITERAL_WITHOUT_NAME_REFERENCE_PREFIX) {
    return instruction::literal_without_name_reference;
  }

  return instruction::unknown;
}

header qpack::decoder::decode(buffer &encoded, std::error_code &ec)
{
  return decode<buffer>(encoded, ec);
}

header qpack::decoder::decode(buffers &encoded, std::error_code &ec)
{
  return decode<buffers>(encoded, ec);
}

template <typename Sequence>
header qpack::decoder::decode(Sequence &encoded, std::error_code &ec)
{
  header header;

  typename Sequence::anchor anchor(encoded);

  CHECK(!encoded.empty(), error::incomplete);

  if (state_ == state::prefix) {
    CHECK(encoded.size() >= QPACK_PREFIX_ENCODED_SIZE, error::incomplete);

    encoded += QPACK_PREFIX_ENCODED_SIZE;
    count_ += QPACK_PREFIX_ENCODED_SIZE;
    state_ = state::header;

    anchor.relocate();
  }

  size_t before = encoded.consumed();

  switch (qpack_instruction_type(*encoded)) {

    case instruction::indexed_header_field: {
      CHECK_MSG((*encoded & 0x40U) != 0, error::qpack_decompression_failed,
                "'S' (static table) bit not set in indexed header field");

      uint8_t index = TRY(
          static_cast<uint8_t>(prefix_int_.decode(encoded, 6, ec)));

      bool found = qpack::static_table::find_header_value(index, &header);
      CHECK_MSG(found, error::qpack_decompression_failed,
                "Indexed header field ({}) not found in static table", index);

      break;
    }

    case instruction::literal_with_name_reference: {
      CHECK_MSG(
          (*encoded & 0x10U) != 0, error::qpack_decompression_failed,
          "'S' (static table) bit not set in literal with name reference");

      uint8_t index = TRY(
          static_cast<uint8_t>(prefix_int_.decode(encoded, 4, ec)));

      bool found = qpack::static_table::find_header_only(index, &header);
      CHECK_MSG(found, error::qpack_decompression_failed,
                "Header name reference ({}) not found in static table", index);

      header.value = TRY(literal_.decode(encoded, 7, ec));
      break;
    }

    case instruction::literal_without_name_reference: {
      header.name = TRY(literal_.decode(encoded, 3, ec));

      const char *name = reinterpret_cast<const char *>(header.name.data());
      size_t size = header.name.size();

      CHECK_MSG(util::is_lowercase(name, size), error::malformed_header,
                "Header ({}) is not lowercase", fmt::string_view(name, size));

      header.value = TRY(literal_.decode(encoded, 7, ec));
      break;
    }

    case instruction::unknown:
      LOG_E("Unexpected header block instruction prefix ({})", *encoded);
      THROW(error::qpack_decompression_failed);
  }

  count_ += encoded.consumed() - before;

  anchor.release();

  return header;
}

} // namespace http3
} // namespace bnl
