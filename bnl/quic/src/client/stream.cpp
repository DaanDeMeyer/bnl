#include <bnl/quic/client/stream.hpp>

#include <bnl/base/log.hpp>
#include <bnl/quic/client/ngtcp2/connection.hpp>

namespace bnl {
namespace quic {
namespace client {

stream::stream(uint64_t id, ngtcp2::connection *ngtcp2)
  : id_(id)
  , ngtcp2_(ngtcp2)

{}

result<base::buffer>
stream::send()
{
  if (buffers_.empty()) {
    return error::idle;
  }

  if (!opened()) {
    result<void> r = ngtcp2_->open(id_);
    if (!r) {
      return r.error() == error::stream_id_blocked ? error::idle
                                                   : r.error();
    }

    opened_ = true;
  }

  const base::buffer &first = buffers_.front();
  bool fin = fin_ && (first == buffers_.back());

  base::buffer packet;
  size_t stream_bytes_written = 0;

  result<std::pair<base::buffer, size_t>> r =
    ngtcp2_->write_stream(id_, first, fin);

  if (!r) {
    if (r.error() == error::stream_id_blocked ||
        r.error() == error::stream_data_blocked) {
      return error::idle;
    }

    return r.error();
  }

  std::tie(packet, stream_bytes_written) = std::move(r).value();

  base::buffer sent = buffers_.slice(stream_bytes_written);
  keepalive_.push(std::move(sent));

  return packet;
}

result<void>
stream::add(base::buffer buffer)
{
  assert(!fin_);

  buffers_.push(std::move(buffer));

  return base::success();
}

result<void>
stream::fin()
{
  assert(!fin_);

  fin_ = true;

  return base::success();
}

result<void>
stream::ack(size_t size)
{
  if (size > keepalive_.size()) {
    BNL_LOG_E(
      "ngtcp2's acked stream ({}) data ({}) exceeds remaining data ({})",
      id_,
      size,
      keepalive_.size());
    return error::internal;
  }

  keepalive_.consume(size);

  return base::success();
}

bool
stream::finished() const noexcept
{
  return fin_ && buffers_.empty() && keepalive_.empty();
}

bool
stream::opened() const noexcept
{
  return opened_;
}

}
}
}
