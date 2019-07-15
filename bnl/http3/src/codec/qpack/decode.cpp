#include <bnl/http3/codec/qpack.hpp>

#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/util/error.hpp>
#include <bnl/util/string.hpp>

#include "decode_generated.cpp"

namespace bnl {
namespace http3 {
namespace qpack {

// TODO: Find out what is best for max header and max value size.
decoder::decoder(const log::api *logger)
  : prefix_int_(logger)
  , literal_(logger)
  , logger_(logger)
{
}

uint64_t
decoder::count() const noexcept
{
  return count_;
}

static constexpr size_t QPACK_PREFIX_ENCODED_SIZE = 2;

template<typename Sequence>
base::result<header>
decoder::decode(Sequence &encoded)
{
  if (state_ == state::prefix) {
    CHECK(encoded.size() >= QPACK_PREFIX_ENCODED_SIZE, base::error::incomplete);

    encoded.consume(QPACK_PREFIX_ENCODED_SIZE);
    count_ += QPACK_PREFIX_ENCODED_SIZE;
    state_ = state::header;
  }

  header header;
  typename Sequence::lookahead_type lookahead(encoded);

  CHECK(!lookahead.empty(), base::error::incomplete);

  switch (table::fixed::find_type(*lookahead)) {

    case table::fixed::type::header_value: {
      if ((*lookahead & 0x40U) == 0) {
        LOG_E("'S' (static table) bit not set in indexed header field");
        THROW(error::qpack_decompression_failed);
      }

      uint8_t index =
        static_cast<uint8_t>(TRY(prefix_int_.decode(lookahead, 6)));

      bool found = false;
      std::tie(found, header) = table::fixed::find_header_value(index);

      if (!found) {
        LOG_E("Indexed header field ({}) not found in static table", index);
        THROW(error::qpack_decompression_failed);
      }

      break;
    }

    case table::fixed::type::header_only: {
      if ((*lookahead & 0x10U) == 0) {
        LOG_E("'S' (static table) bit not set in literal with name reference");
        THROW(error::qpack_decompression_failed);
      }

      uint8_t index =
        static_cast<uint8_t>(TRY(prefix_int_.decode(lookahead, 4)));

      bool found = false;
      base::string name;
      std::tie(found, name) = table::fixed::find_header_only(index);

      if (!found) {
        LOG_E("Header name reference ({}) not found in static table", index);
        THROW(error::qpack_decompression_failed);
      }

      base::string value = TRY(literal_.decode(lookahead, 7));

      header = http3::header(std::move(name), std::move(value));
      break;
    }

    case table::fixed::type::missing: {
      base::string name = TRY(literal_.decode(lookahead, 3));

      if (!util::is_lowercase(name)) {
        LOG_E("Header ({}) is not lowercase", name);
        THROW(error::malformed_header);
      }

      base::string value = TRY(literal_.decode(lookahead, 7));

      header = http3::header(std::move(name), std::move(value));
      break;
    }

    case table::fixed::type::unknown:
      LOG_E("Unexpected header block instruction prefix ({})", *lookahead);
      THROW(error::qpack_decompression_failed);
  }

  count_ += lookahead.consumed();

  encoded.consume(lookahead.consumed());

  return header;
}

BNL_BASE_SEQUENCE_IMPL(BNL_HTTP3_QPACK_DECODE_IMPL);

} // namespace qpack
} // namespace http3
} // namespace bnl
