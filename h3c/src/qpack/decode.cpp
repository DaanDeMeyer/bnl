#include <h3c/qpack.hpp>

#include <util/error.hpp>
#include <util/string.hpp>

#include "decode_generated.cpp"

namespace h3c {

// TODO: Find out what is best for max header and max value size.
qpack::decoder::decoder(logger *logger)
    : logger_(logger), prefix_int_(logger), literal_(logger)
{}

static constexpr size_t QPACK_PREFIX_ENCODED_SIZE = 2;

void qpack::decoder::prefix_decode(buffer &encoded, std::error_code &ec) const
    noexcept
{
  if (encoded.size() < QPACK_PREFIX_ENCODED_SIZE) {
    THROW_VOID(error::incomplete);
  }

  encoded.advance(QPACK_PREFIX_ENCODED_SIZE);
}

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

header qpack::decoder::decode(buffer &encoded, std::error_code &ec) const
{
  header header;

  DECODE_START();

  if (encoded.empty()) {
    DECODE_THROW(error::incomplete);
  }

  switch (qpack_instruction_type(*encoded)) {

    case instruction::indexed_header_field: {
      // Ensure the 'S' bit is set which indicates the index is in the static
      // table.
      if ((*encoded & 0x40U) == 0) {
        LOG_E("'S' bit not set in indexed header field");
        DECODE_THROW(error::qpack_decompression_failed);
      }

      uint8_t index = DECODE_TRY(
          static_cast<uint8_t>(prefix_int_.decode(encoded, 6, ec)));

      if (!qpack::static_table::find_header_value(index, &header)) {
        LOG_E("Indexed header field ({}) not found in static table", index);
        DECODE_THROW(error::qpack_decompression_failed);
      }
      break;
    }

    case instruction::literal_with_name_reference: {
      // Ensure the 'S' bit is set which indicates the index is in the static
      // table.
      if ((*encoded & 0x10U) == 0) {
        LOG_E("'S' bit not set in literal with name reference");
        DECODE_THROW(error::qpack_decompression_failed);
      }

      uint8_t index = DECODE_TRY(
          static_cast<uint8_t>(prefix_int_.decode(encoded, 4, ec)));

      if (!static_table::find_header_only(index, &header)) {
        LOG_E("Header name reference ({}) not found in static table", index);
        DECODE_THROW(error::qpack_decompression_failed);
      }

      header.value = DECODE_TRY(literal_.decode(encoded, 7, ec));
      break;
    }

    case instruction::literal_without_name_reference: {
      header.name = DECODE_TRY(literal_.decode(encoded, 3, ec));

      auto name = reinterpret_cast<const char *>(header.name.data());
      size_t size = header.name.size();

      if (!util::is_lowercase(name, size)) {
        LOG_E("Header ({}) is not lowercase", fmt::string_view(name, size));
        DECODE_THROW(error::malformed_header);
      }

      header.value = DECODE_TRY(literal_.decode(encoded, 7, ec));
      break;
    }

    case instruction::unknown:
      LOG_E("Unexpected header block instruction prefix ({})", *encoded);
      DECODE_THROW(error::qpack_decompression_failed);
  }

  return header;
}

} // namespace h3c
