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
  if (state_ == state::prefix) {
    CHECK(encoded.size() >= QPACK_PREFIX_ENCODED_SIZE, error::incomplete);

    encoded.consume(QPACK_PREFIX_ENCODED_SIZE);
    count_ += QPACK_PREFIX_ENCODED_SIZE;
    state_ = state::header;
  }

  header header;
  typename Sequence::view view(encoded);

  CHECK(!view.empty(), error::incomplete);

  switch (qpack::table::fixed::find_type(*view)) {

    case qpack::table::fixed::type::header_value: {
      CHECK_MSG((*view & 0x40U) != 0, error::qpack_decompression_failed,
                "'S' (static table) bit not set in indexed header field");

      uint8_t index = TRY(
          static_cast<uint8_t>(prefix_int_.decode(view, 6, ec)));

      bool found = false;
      std::tie(found, header) = qpack::table::fixed::find_header_value(index);
      CHECK_MSG(found, error::qpack_decompression_failed,
                "Indexed header field ({}) not found in static table", index);

      break;
    }

    case qpack::table::fixed::type::header_only: {
      CHECK_MSG(
          (*view & 0x10U) != 0, error::qpack_decompression_failed,
          "'S' (static table) bit not set in literal with name reference");

      uint8_t index = TRY(
          static_cast<uint8_t>(prefix_int_.decode(view, 4, ec)));

      bool found = false;
      std::tie(found, header) = qpack::table::fixed::find_header_only(index);
      CHECK_MSG(found, error::qpack_decompression_failed,
                "Header name reference ({}) not found in static table", index);

      header.value = TRY(literal_.decode(view, 7, ec));
      break;
    }

    case qpack::table::fixed::type::missing: {
      header.name = TRY(literal_.decode(view, 3, ec));

      const char *name = reinterpret_cast<const char *>(header.name.data());
      size_t size = header.name.size();

      CHECK_MSG(util::is_lowercase(name, size), error::malformed_header,
                "Header ({}) is not lowercase", fmt::string_view(name, size));

      header.value = TRY(literal_.decode(view, 7, ec));
      break;
    }

    case qpack::table::fixed::type::unknown:
      LOG_E("Unexpected header block instruction prefix ({})", *view);
      THROW(error::qpack_decompression_failed);
  }

  count_ += view.consumed();

  encoded.consume(view.consumed());

  return header;
}

} // namespace http3
} // namespace bnl
