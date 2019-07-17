#include <bnl/quic/client/stream.hpp>

#include <bnl/base/error.hpp>
#include <bnl/quic/client/ngtcp2/connection.hpp>
#include <bnl/util/enum.hpp>
#include <bnl/util/error.hpp>

namespace bnl {
namespace quic {
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

std::error_code
stream::ack(size_t size)
{
  if (size > buffers_.size()) {
    LOG_E("ngtcp2's acked stream ({}) data ({}) exceeds remaining data ({})",
          id_,
          size,
          buffers_.size());
    THROW(base::error::internal);
  }

  buffers_.consume(size);

  return {};
}

bool
stream::finished() const noexcept
{
  return fin_ && buffers_.empty() && keepalive_.empty();
}

}
}
}
