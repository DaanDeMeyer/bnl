#include <endpoint/client/stream.hpp>

#include <endpoint/client/ngtcp2/connection.hpp>

#include <bnl/base/error.hpp>
#include <bnl/util/enum.hpp>
#include <bnl/util/error.hpp>

namespace bnl {
namespace quic {
namespace endpoint {
namespace client {

stream::stream(uint64_t id,
               ngtcp2::connection *connection,
               const log::api *logger)
  : id_(id)
  , connection_(connection)
  , logger_(logger)
{}

base::result<base::buffer>
stream::send()
{
  if (fin_ && buffers_.empty()) {
    return { quic::error::finished };
  }

  if (buffers_.empty()) {
    THROW(base::error::idle);
  }

  base::buffer &first = buffers_.front();
  bool fin = fin_ && (first == buffers_.back());

  base::buffer packet;
  size_t stream_bytes_written = 0;

  std::tie(packet, stream_bytes_written) =
    TRY(connection_->write_stream(id_, first, fin));

  base::buffer sent = buffers_.slice(stream_bytes_written);
  keepalive_.push(std::move(sent));

  return packet;
}

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

} // namespace quic
} // namespace bnl
}
}
