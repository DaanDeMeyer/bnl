#include <bnl/http3/codec/qpack.hpp>

#include <bnl/base/log.hpp>

#include <tuple>

#include "decode_generated.cpp"

namespace bnl {
namespace http3 {
namespace qpack {

uint64_t
decoder::count() const noexcept
{
  return count_;
}

static constexpr size_t QPACK_PREFIX_ENCODED_SIZE = 2;

template<typename Sequence>
result<header>
decoder::decode(Sequence &encoded)
{
  if (state_ == state::prefix) {
    if (encoded.size() < QPACK_PREFIX_ENCODED_SIZE) {
      return error::incomplete;
    }

    encoded.consume(QPACK_PREFIX_ENCODED_SIZE);
    count_ += QPACK_PREFIX_ENCODED_SIZE;
    state_ = state::header;
  }

  header header;
  typename Sequence::lookahead_type lookahead(encoded);

  if (lookahead.empty()) {
    return error::incomplete;
  }

  switch (table::fixed::find_type(*lookahead)) {

    case table::fixed::type::header_value: {
      if ((*lookahead & 0x40U) == 0) {
        BNL_LOG_E("'S' (static table) bit not set in indexed header field");
        return error::qpack_decompression_failed;
      }

      uint8_t index =
        static_cast<uint8_t>(BNL_TRY(prefix_int::decode(lookahead, 6)));

      bool found = false;
      std::tie(found, header) = table::fixed::find_header_value(index);

      if (!found) {
        BNL_LOG_E("Indexed header field ({}) not found in static table", index);
        return error::qpack_decompression_failed;
      }

      break;
    }

    case table::fixed::type::header_only: {
      if ((*lookahead & 0x10U) == 0) {
        BNL_LOG_E(
          "'S' (static table) bit not set in literal with name reference");
        return error::qpack_decompression_failed;
      }

      uint8_t index =
        static_cast<uint8_t>(BNL_TRY(prefix_int::decode(lookahead, 4)));

      bool found = false;
      base::string name;
      std::tie(found, name) = table::fixed::find_header_only(index);

      if (!found) {
        BNL_LOG_E("Header name reference ({}) not found in static table",
                  index);
        return error::qpack_decompression_failed;
      }

      base::string value = BNL_TRY(literal::decode(lookahead, 7));

      header = http3::header(std::move(name), std::move(value));
      break;
    }

    case table::fixed::type::missing: {
      base::string name = BNL_TRY(literal::decode(lookahead, 3));
      base::string value = BNL_TRY(literal::decode(lookahead, 7));

      header = http3::header(std::move(name), std::move(value));

      if (!header_is_lowercase(header)) {
        BNL_LOG_E("Header ({}) is not lowercase", header);
        return error::malformed_header;
      }

      break;
    }

    case table::fixed::type::unknown:
      BNL_LOG_E("Unexpected header block instruction prefix ({})", *lookahead);
      return error::qpack_decompression_failed;
  }

  count_ += lookahead.consumed();

  encoded.consume(lookahead.consumed());

  return header;
}

BNL_BASE_SEQUENCE_IMPL(BNL_HTTP3_QPACK_DECODE_IMPL);

}
}
}
