#include <endpoint/shared/stream.hpp>

#include <bnl/base/error.hpp>
#include <bnl/util/enum.hpp>
#include <bnl/util/error.hpp>

namespace bnl {
namespace quic {

stream::stream(uint64_t id, ngtcp2_conn* connection, const log::api* logger)
  : id_(id)
  , connection_(connection)
  , logger_(logger)
{}

std::error_code
stream::add(base::buffer buffer)
{
  CHECK(!fin_, base::error::internal);

  buffers_.push(std::move(buffer));

  return {};
}

std::error_code
stream::fin()
{
  CHECK(!fin_, base::error::internal);

  fin_ = true;

  return {};
}

std::error_code
stream::close(quic::error error)
{
  uint32_t as_int = util::to_underlying(error);

  if (as_int > UINT16_MAX) {
    LOG_E("Stream error ({}) exceeds valid stream error size ({})",
          as_int,
          UINT16_MAX);
    THROW(base::error::internal);
  }

  // int rv = ngtcp2_conn_shutdown_stream(connection_,
  // static_cast<int64_t>(id_),
  //                                      static_cast<uint16_t>(as_int));

  return {};
}

} // namespace quic
} // namespace bnl
